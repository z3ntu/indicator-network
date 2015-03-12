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
 *     Marcus Tomlinson <marcus.tomlinson@canonical.com>
 */

#include "manager.h"
#include "wifi/link.h"
#include "set_name_for_thread.h"
#include <NetworkManagerInterface.h>
#include <NetworkManagerDeviceInterface.h>

#include <NetworkManager.h>
#include <core/dbus/asio/executor.h>
#include <thread>
#include <QDebug>

using namespace platform::nmofono;
namespace networking = connectivity::networking;
namespace dbus = core::dbus;

namespace platform {
namespace nmofono {

class Manager::Private : public QObject
{
    Q_OBJECT
public:
    std::shared_ptr<OrgFreedesktopNetworkManagerInterface> nm;

    core::Property<connectivity::networking::Manager::FlightModeStatus> m_flightMode;
    core::Property<std::set<std::shared_ptr<connectivity::networking::Link>>> m_links;
    core::Property<std::set<std::shared_ptr<connectivity::networking::Service>>> m_services;
    core::Property<connectivity::networking::Manager::NetworkingStatus> m_status;
    core::Property<std::uint32_t> m_characteristics;

    core::Property<bool> m_hasWifi;
    core::Property<bool> m_wifiEnabled  ;
    std::shared_ptr<KillSwitch> m_wifiKillSwitch;

public Q_SLOTS:
    void updateHasWifi()
    {
        if (m_wifiKillSwitch->state() != KillSwitch::State::not_available) {
            m_hasWifi.set(true);
            if (m_wifiKillSwitch->state() == KillSwitch::State::unblocked)
                m_wifiEnabled.set(true);
            else
                m_wifiEnabled.set(false);
            return;
        }

        // ok, killswitch not supported, but we still might have wifi devices
        bool haswifi = false;
        for (auto link : m_links.get()) {
            if (link->type() == networking::Link::Type::wifi)
                haswifi = true;
        }
        m_hasWifi.set(haswifi);
        m_wifiEnabled.set(haswifi);
    }

    void flightModeChanged(bool flightMode)
    {
        if (flightMode)
            m_flightMode.set(networking::Manager::FlightModeStatus::on);
        else
            m_flightMode.set(networking::Manager::FlightModeStatus::off);
    }
};
}
}

void
Manager::updateNetworkingStatus(uint status)
{
    switch(status) {
    case NM_STATE_UNKNOWN:
    case NM_STATE_ASLEEP:
    case NM_STATE_DISCONNECTED:
    case NM_STATE_DISCONNECTING:
    {
        p->m_status.set(networking::Manager::NetworkingStatus::offline);
        break;
    }
    case NM_STATE_CONNECTING:
    {
        p->m_status.set(networking::Manager::NetworkingStatus::connecting);
        break;
    }
    case NM_STATE_CONNECTED_LOCAL:
    case NM_STATE_CONNECTED_SITE:
    case NM_STATE_CONNECTED_GLOBAL:
    {
        p->m_status.set(networking::Manager::NetworkingStatus::online);
        break;
    }
    }
}

Manager::Manager(const QDBusConnection& systemConnection) : p(new Manager::Private())
{
    p->nm = std::make_shared<OrgFreedesktopNetworkManagerInterface>(NM_DBUS_SERVICE, NM_DBUS_PATH, systemConnection);

    /// @todo add a watcher for the service
    /// @todo exceptions
    /// @todo offload the initialization to a thread or something
    /// @todo those Id() thingies

    p->m_wifiKillSwitch = std::make_shared<KillSwitch>(systemConnection);
    QObject::connect(p->m_wifiKillSwitch.get(), SIGNAL(stateChanged()), p.get(), SLOT(updateHasWifi()));

    connect(p->nm.get(), &OrgFreedesktopNetworkManagerInterface::DeviceAdded, this, &Manager::device_added);
    QList<QDBusObjectPath> devices(p->nm->GetDevices());
    for(const auto &path : devices) {
        device_added(path);
    }

    connect(p->nm.get(), &OrgFreedesktopNetworkManagerInterface::DeviceRemoved, this, &Manager::device_removed);
    updateNetworkingStatus(p->nm->state());
    connect(p->nm.get(), &OrgFreedesktopNetworkManagerInterface::PropertiesChanged, this, &Manager::nm_properties_changed);

    QObject::connect(p->m_wifiKillSwitch.get(), SIGNAL(flightModeChanged(bool)), p.get(), SLOT(flightModeChanged(bool)));
    try
    {
        p->flightModeChanged(p->m_wifiKillSwitch->isFlightMode());
    }
    catch (std::exception const& e)
    {
        std::cerr << __PRETTY_FUNCTION__ << ": " << e.what() << std::endl;
        std::cerr << "Failed to retrieve initial flight mode state, assuming state is false." << std::endl;
        p->flightModeChanged(false);
    }

    /// @todo set by the default connections.
    p->m_characteristics.set(networking::Link::Characteristics::empty);

    p->updateHasWifi();
}

void
Manager::nm_properties_changed(const QVariantMap &properties)
{
    auto stateIt = properties.find("State");
    if (stateIt != properties.cend())
    {
        updateNetworkingStatus(stateIt->toUInt());
    }
}

void
Manager::device_removed(const QDBusObjectPath &path)
{
#ifdef INDICATOR_NETWORK_TRACE_MESSAGES
        qDebug() << "Device Removed:" << path.path();
#endif
    auto links = p->m_links.get();
    for (const auto &dev : links) {
        if (std::dynamic_pointer_cast<wifi::Link>(dev)->device_path() == path) {
            links.erase(dev);
            break;
        }
    }
    p->m_links.set(links);

    p->updateHasWifi();
}

void
Manager::device_added(const QDBusObjectPath &path)
{
#ifdef INDICATOR_NETWORK_TRACE_MESSAGES
    qDebug() << "Device Added:" << path.path();
#endif
    auto links = p->m_links.get();

    for (const auto &dev : links) {
        if (std::dynamic_pointer_cast<wifi::Link>(dev)->device_path() == path) {
            // already in the list
            return;
        }
    }

    connectivity::networking::Link::Ptr link;
    try {
        auto dev = std::make_shared<OrgFreedesktopNetworkManagerDeviceInterface>(
            NM_DBUS_SERVICE, path.path(), p->nm->connection());
        if (dev->deviceType() == NM_DEVICE_TYPE_WIFI) {
            link = std::make_shared<wifi::Link>(dev,
                                                p->nm,
                                                p->m_wifiKillSwitch);
        }
    } catch (const std::exception &e) {
        qDebug() << __PRETTY_FUNCTION__ << ": failed to create Device proxy for "<< path.path() << ": ";
        qDebug() << "\t" << e.what();
        qDebug() << "\tIgnoring.";
        return;
    }

    if (link) {
        links.insert(link);
        p->m_links.set(links);
    }

    p->updateHasWifi();
}


void
Manager::enableFlightMode()
{
#ifdef INDICATOR_NETWORK_TRACE_MESSAGES
    std::cout << __PRETTY_FUNCTION__ << std::endl;
#endif
    if (!p->m_wifiKillSwitch->flightMode(true))
        throw std::runtime_error("Failed to enable flightmode.");
}

void
Manager::disableFlightMode()
{
#ifdef INDICATOR_NETWORK_TRACE_MESSAGES
    std::cout << __PRETTY_FUNCTION__ << std::endl;
#endif
    if (!p->m_wifiKillSwitch->flightMode(false))
        throw std::runtime_error("Failed to disable flightmode");
}

const core::Property<networking::Manager::FlightModeStatus>&
Manager::flightMode() const
{
    // - connect to each individual URfkill.Killswitch interface
    // - make this property to reflect their combined state
    /// @todo implement flightmode status properly when URfkill gets the flightmode API
    return p->m_flightMode;
}

const core::Property<std::set<std::shared_ptr<networking::Link> > >&
Manager::links() const
{
    return p->m_links;
}

const core::Property<std::set<std::shared_ptr<networking::Service>>>&
Manager::services() const
{
    return p->m_services;
}

const core::Property<networking::Manager::NetworkingStatus> &
Manager::status() const
{
    return p->m_status;
}

const core::Property<std::uint32_t>&
Manager::characteristics() const
{
    return p->m_characteristics;
}

const core::Property<bool>&
Manager::hasWifi() const
{
    return p->m_hasWifi;
}

const core::Property<bool>&
Manager::wifiEnabled() const
{
    return p->m_wifiEnabled;
}


bool
Manager::enableWifi()
{
    if (!p->m_hasWifi.get())
        return false;

    try {
        if (p->m_wifiKillSwitch->state() == KillSwitch::State::soft_blocked) {
            // try to unblock. throws if fails.
            p->m_wifiKillSwitch->unblock();
        }
        p->nm->setWimaxEnabled(true);
    } catch(std::runtime_error &e) {
        std::cerr << __PRETTY_FUNCTION__ << ": " << e.what() << std::endl;
        return false;
    }
    return true;
}

bool
Manager::disableWifi()
{
    if (!p->m_hasWifi.get())
        return false;

    try {
        if (p->m_wifiKillSwitch->state() == KillSwitch::State::unblocked) {
            // block the device. that will disable it also
            p->m_wifiKillSwitch->block();
        }
        p->nm->setWirelessEnabled(false);
    } catch(std::runtime_error &e) {
        std::cerr << __PRETTY_FUNCTION__ << ": " << e.what() << std::endl;
        return false;
    }
    return true;
}

#include "manager.moc"
