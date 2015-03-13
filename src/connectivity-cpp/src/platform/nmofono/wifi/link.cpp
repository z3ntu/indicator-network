/*
 * Copyright © 2013 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License version 3,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors:
 *     Antti Kaijanmäki <antti.kaijanmaki@canonical.com>
 */

#include "link.h"
#include "access-point-impl.h"
#include "grouped-access-point.h"
#include <cassert>

#include <NetworkManagerActiveConnectionInterface.h>
#include <NetworkManagerDeviceWirelessInterface.h>
#include <NetworkManagerSettingsConnectionInterface.h>

#include <NetworkManager.h>

using namespace platform::nmofono::wifi;
namespace networking = connectivity::networking;

using core::Property;


struct Link::Private
{
    Private(std::shared_ptr<OrgFreedesktopNetworkManagerDeviceInterface> dev,
            std::shared_ptr<OrgFreedesktopNetworkManagerInterface> nm,
            KillSwitch::Ptr killSwitch);

    core::Property<std::uint32_t> characteristics_;
    core::Property<connectivity::networking::Link::Status> status_;
    core::Property<std::set<std::shared_ptr<connectivity::networking::wifi::AccessPoint>>> rawAccessPoints;
    core::Property<std::set<std::shared_ptr<connectivity::networking::wifi::AccessPoint>>> groupedAccessPoints;
    core::Property<std::shared_ptr<connectivity::networking::wifi::AccessPoint>> activeAccessPoint;

    std::shared_ptr<OrgFreedesktopNetworkManagerDeviceInterface> dev;
    OrgFreedesktopNetworkManagerDeviceWirelessInterface wireless;
    std::shared_ptr<OrgFreedesktopNetworkManagerInterface> nm;

    KillSwitch::Ptr killSwitch;

    std::map<AccessPoint::Key, std::shared_ptr<GroupedAccessPoint>> grouper;
    bool disabled;
    std::uint32_t lastState;
    QString name;
    std::shared_ptr<OrgFreedesktopNetworkManagerConnectionActiveInterface> activeConnection;
    bool connecting;
};

Link::Private::Private(std::shared_ptr<OrgFreedesktopNetworkManagerDeviceInterface> dev,
                       std::shared_ptr<OrgFreedesktopNetworkManagerInterface> nm,
                       KillSwitch::Ptr killSwitch)
    : dev(dev),
      wireless(NM_DBUS_SERVICE, dev->path(), dev->connection()),
      nm(nm),
      killSwitch(killSwitch),
      disabled(true),
      lastState(NM_STATE_UNKNOWN),
      connecting(false)
{}

Link::Link(std::shared_ptr<OrgFreedesktopNetworkManagerDeviceInterface> dev,
           std::shared_ptr<OrgFreedesktopNetworkManagerInterface> nm,
           platform::nmofono::KillSwitch::Ptr killSwitch)
    : p(new Private(dev, nm, killSwitch)) {
    p->characteristics_.set(Link::Characteristics::empty);
    p->status_.set(Status::disabled);

    p->name = p->dev->interface();

    connect(&p->wireless, &OrgFreedesktopNetworkManagerDeviceWirelessInterface::AccessPointAdded, this, &Link::ap_added);
    connect(&p->wireless, &OrgFreedesktopNetworkManagerDeviceWirelessInterface::AccessPointRemoved, this, &Link::ap_removed);
    QList<QDBusObjectPath> aps = p->wireless.GetAccessPoints();
    for (const auto& path : aps) {
        ap_added(path);
    }

    connect(p->dev.get(), &OrgFreedesktopNetworkManagerDeviceInterface::StateChanged, this, &Link::state_changed);
    updateDeviceState(p->dev->state());

    QObject::connect(p->killSwitch.get(), &KillSwitch::stateChanged, [this]()
    {
        updateDeviceState(p->lastState);
    });
}

Link::~Link()
{}

void Link::state_changed(uint new_state, uint, uint)
{
    updateDeviceState(new_state);
}

void Link::ap_added(const QDBusObjectPath &path)
{
    try {
        for (auto ap : p->rawAccessPoints.get()) {
            if (std::dynamic_pointer_cast<AccessPoint>(ap)->object_path() == path) {
                // already in the list
                return;
            }
        }

        platform::nmofono::wifi::AccessPoint::Ptr shap;
        try {
            auto ap = std::make_shared<
                    OrgFreedesktopNetworkManagerAccessPointInterface>(
                    NM_DBUS_SERVICE, path.path(), p->dev->connection());
            shap = std::make_shared<platform::nmofono::wifi::AccessPoint>(ap);
        } catch(const std::exception &e) {
            qWarning() << __PRETTY_FUNCTION__ << ": failed to create AccessPoint proxy for "<< path.path() << ": ";
            qWarning() << "\t" << QString::fromStdString(e.what());
            qWarning() << "\tIgnoring.";
            return;
        }

        auto list = p->rawAccessPoints.get();
        list.insert(shap);
        p->rawAccessPoints.set(list);

        auto k = AccessPoint::Key(shap);
        if(p->grouper.find(k) != p->grouper.end()) {
            p->grouper[k]->add_ap(shap);
        } else {
            p->grouper[k] = std::make_shared<GroupedAccessPoint>(shap);
        }
        update_grouped();
    } catch(const std::exception &e) {
        /// @bug dbus-cpp internal logic exploded
        // If this happens, indicator-network is in an unknown state with no clear way of
        // recovering. The only reasonable way out is a graceful exit.
        std::cerr << __PRETTY_FUNCTION__ << " Failed to run dbus service: " << e.what() << std::endl;
        std::quick_exit(0);
    }
}

void Link::ap_removed(const QDBusObjectPath &path)
{
    platform::nmofono::wifi::AccessPoint::Ptr shap;

    auto list = p->rawAccessPoints.get();
    for (const auto &ap : list) {
        if (std::dynamic_pointer_cast<AccessPoint>(ap)->object_path() == path) {
            shap = std::dynamic_pointer_cast<platform::nmofono::wifi::AccessPoint>(ap);
            list.erase(ap);
            break;
        }
    }
    if (!shap) {
        qWarning() << __PRETTY_FUNCTION__ << ": Tried to remove access point " << path.path() << " that has not been added.";
        return;
    }
    p->rawAccessPoints.set(list);

    for (auto &e : p->grouper) {
        if (!e.second->has_object(path))
            continue;

        if(e.second->num_aps() == 1) {
            p->grouper.erase(e.first);
        } else {
            e.second->remove_ap(shap);
        }
        break;
    }
    update_grouped();
}

void Link::update_grouped() {
    std::set<std::shared_ptr<connectivity::networking::wifi::AccessPoint>> new_grouped;
    for(auto &i : p->grouper) {
        new_grouped.insert(i.second);
    }
    p->groupedAccessPoints.set(new_grouped);
}

void
Link::enable()
{
#ifdef INDICATOR_NETWORK_TRACE_MESSAGES
    qDebug() << "Link::enable()";
#endif

    try {
        if (p->killSwitch->state() != KillSwitch::State::unblocked) {
            // try to unblock. throws if fails.
            p->killSwitch->unblock();
        }
        p->nm->setWirelessEnabled(true);
        p->dev->setAutoconnect(true);
    } catch(std::runtime_error &e) {
        /// @todo when toggling enable()/disable() rapidly the default timeout of
        ///       1 second in dbus::core::Property is not long enough..
        ///       just ignore for now and get dbus-cpp to have larger timeout.
        std::cerr << __PRETTY_FUNCTION__ << ": " << e.what() << std::endl;
    }

#if 0
    /** @todo:
     * NetworkManager 0.9.9 allows doing this below and letting NM to figure out the best connection:
     *
     * m_nm.activate_connection(dbus::types::ObjectPath("/"),
     *                          m_dev.object->path(),
     *                          core::dbus::typ es::ObjectPath("/"));
     *
     * for now we get somewhat similar result by toggling the Autoconnect property,
     * but this breaks the possible setting we will have for autoconnecting later.
     */
    dev.autoconnect->set(true);
    disabled = false;
#endif
}

void
Link::disable()
{
#ifdef INDICATOR_NETWORK_TRACE_MESSAGES
    std::cout << "Link::disable()" << std::endl;
#endif

    /// @todo for now just disable wireless completely.
    ///        this only works properly when there is one wifi adapter on the system
    try {
        if (p->killSwitch->state() == KillSwitch::State::unblocked) {
            // block the device. that will disable it also
            p->killSwitch->block();
            return;
        }
        p->nm->setWirelessEnabled(false);
        p->dev->setAutoconnect(false);
    } catch(std::runtime_error &e) {
        /// @todo when toggling enable()/disable() rapidly the default timeout of
        ///       1 second in dbus::core::Property is not long enough..
        ///       just ignore for now and get dbus-cpp to have larger timeout.
        qWarning() << __PRETTY_FUNCTION__ << ": " << QString::fromStdString(e.what());
    }

#if 0
    /** @todo remove this after NM 0.9.9 is available. check comment in enable() */
    dev.disconnect();
#endif
}

networking::Link::Type
Link::type() const
{
    return Type::wifi;
}

const Property<std::uint32_t>&
Link::characteristics() const
{
    return p->characteristics_;
}

const Property<networking::Link::Status>&
Link::status() const
{
    return p->status_;
}

networking::Link::Id
Link::id() const
{
    return 0;
}

std::string
Link::name() const
{
    return p->name.toStdString();
}

const Property<std::set<std::shared_ptr<networking::wifi::AccessPoint> > >&
Link::rawAccessPoints() const
{
    return p->rawAccessPoints;
}

const core::Property<std::set<std::shared_ptr<connectivity::networking::wifi::AccessPoint>>>&
Link::accessPoints() const {
    return p->groupedAccessPoints;
}

void
Link::connect_to(std::shared_ptr<networking::wifi::AccessPoint> accessPoint)
{
    try {
        p->connecting = true;
        QByteArray ssid;
        std::shared_ptr<GroupedAccessPoint> ap = std::dynamic_pointer_cast<GroupedAccessPoint>(accessPoint);
        // The accesspoint interface does not provide this property so we need to coax it out of
        // derived classes.
        if(ap) {
            ssid = ap->raw_ssid();
        } else {
            std::shared_ptr<AccessPoint> bap = std::dynamic_pointer_cast<AccessPoint>(accessPoint);
            assert(bap);
            ssid = bap->raw_ssid();
        }
        std::shared_ptr<OrgFreedesktopNetworkManagerSettingsConnectionInterface> found;
        QList<QDBusObjectPath> connections = p->dev->availableConnections();
        for (auto &path : connections) {
            auto con = std::make_shared<OrgFreedesktopNetworkManagerSettingsConnectionInterface>(
                    NM_DBUS_SERVICE, path.path(), p->dev->connection());
            QVariantDictMap settings = con->GetSettings();
            auto wirelessIt = settings.find("802-11-wireless");
            if (wirelessIt != settings.cend())
            {
                auto ssidIt = wirelessIt->find("ssid");
                if (ssidIt != wirelessIt->cend())
                {
                    QByteArray value = ssidIt->toByteArray();
                    if (value != ap->raw_ssid())
                    {
                        found = con;
                                break;
                            }
                        }
                    }
                }

        /// @todo check the timestamps as there might be multiple ones that are suitable.
        /// @todo oh, and check more parameters than just the ssid

        QDBusObjectPath ac("/");
        if (found) {
            ac = p->nm->ActivateConnection(QDBusObjectPath(found->path()),
                                           QDBusObjectPath(p->dev->path()),
                    ap->object_path());
        } else {
            QVariantDictMap conf;

            /// @todo getting the ssid multiple times over dbus is stupid.

            QVariantMap wireless_conf;
            wireless_conf["ssid"] = ap->raw_ssid();

            conf["802-11-wireless"] = wireless_conf;
            auto ret = p->nm->AddAndActivateConnection(
                    conf, QDBusObjectPath(p->dev->path()), ap->object_path());
            ret.waitForFinished();
            ac = ret.argumentAt<1>();
        }
        updateActiveConnection(ac);
        p->connecting = false;
    } catch(const std::exception &e) {
        // @bug default timeout expired: LP(#1361642)
        // If this happens, indicator-network is in an unknown state with no clear way of
        // recovering. The only reasonable way out is a graceful exit.
        std::cerr << __PRETTY_FUNCTION__ << " Failed to activate connection: " << e.what() << std::endl;
        std::quick_exit(0);
    }
}

const Property<std::shared_ptr<networking::wifi::AccessPoint> >&
Link::activeAccessPoint()
{
    return p->activeAccessPoint;
}

void
Link::updateDeviceState(uint new_state)
{
    p->lastState = new_state;
    switch (new_state){
    case NM_DEVICE_STATE_DISCONNECTED:
    case NM_DEVICE_STATE_DEACTIVATING:
        if (p->connecting) {
            // ignore these while doing connect_to()
            break;
        }
        /* fallthrough */
    case NM_DEVICE_STATE_UNKNOWN:
    case NM_DEVICE_STATE_UNMANAGED:
    case NM_DEVICE_STATE_UNAVAILABLE:
    case NM_DEVICE_STATE_FAILED:
    {
        // make sure to set activeConnection before changing the status
        updateActiveConnection(QDBusObjectPath("/"));

        switch(p->killSwitch->state()) {
        case KillSwitch::State::hard_blocked:
        case KillSwitch::State::soft_blocked:
            p->status_.set(Status::disabled);
            break;
        case KillSwitch::State::not_available:
        case KillSwitch::State::unblocked:
            p->status_.set(Status::offline);
        }
        break;
    }
    case NM_DEVICE_STATE_PREPARE:
    case NM_DEVICE_STATE_CONFIG:
    case NM_DEVICE_STATE_NEED_AUTH:
    case NM_DEVICE_STATE_IP_CONFIG:
    case NM_DEVICE_STATE_IP_CHECK:
    {
        // make sure to set activeConnection before changing the status
        QDBusObjectPath path = p->dev->activeConnection();
        // for some reason the path is not always set on these
        // states. Let's not clear the active connection as obviously
        // we have one.
        if (path != QDBusObjectPath("/"))
        {
            updateActiveConnection(path);
        }
        p->status_.set(Status::connecting);
        break;
    }
    case NM_DEVICE_STATE_SECONDARIES:
    {
        // make sure to set activeConnection before changing the status
        updateActiveConnection(p->dev->activeConnection());
        p->status_.set(Status::connected);
        break;
    }
    case NM_DEVICE_STATE_ACTIVATED:
    {
        // make sure to set activeConnection before changing the status
        updateActiveConnection(p->dev->activeConnection());
        p->status_.set(Status::online);
        break;
    }}

}

/// '/' path means invalid.
void
Link::updateActiveConnection(const QDBusObjectPath &path)
{
    // clear the one we have.
    if (path == QDBusObjectPath("/")) {
        p->activeAccessPoint.set(connectivity::networking::wifi::AccessPoint::Ptr());
        p->activeConnection.reset();
        return;
    }

    // already up-to-date
    if (p->activeConnection && p->activeConnection->path() == path.path())
        return;

    try {
        p->activeConnection = std::make_shared<
                OrgFreedesktopNetworkManagerConnectionActiveInterface>(
                NM_DBUS_SERVICE, path.path(), p->dev->connection());
        uint state = p->activeConnection->state();
        switch (state) {
        case NM_ACTIVE_CONNECTION_STATE_UNKNOWN:
        case NM_ACTIVE_CONNECTION_STATE_ACTIVATING:
        case NM_ACTIVE_CONNECTION_STATE_ACTIVATED:
        case NM_ACTIVE_CONNECTION_STATE_DEACTIVATING:
        case NM_ACTIVE_CONNECTION_STATE_DEACTIVATED:
            ;

            // for Wi-Fi devices specific_object is the AccessPoint object.
            QDBusObjectPath ap_path = p->activeConnection->specificObject();
            for (auto &ap : p->groupedAccessPoints.get()) {
                auto shap =  std::dynamic_pointer_cast<GroupedAccessPoint>(ap);
                if (shap->has_object(ap_path)) {
                    p->activeAccessPoint.set(ap);
                    break;
                }
            }
        }
    } catch (std::exception &e) {
        qWarning() << "failed to get active connection:";
        qWarning() << "\tpath: " << path.path();
        qWarning() << "\t" << QString::fromStdString(e.what());
    }
}


QDBusObjectPath Link::device_path() const {
    return QDBusObjectPath(p->dev->path());
}
