/*
 * Copyright (C) 2014 Canonical, Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3, as published
 * by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranties of
 * MERCHANTABILITY, SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors:
 *     Antti Kaijanmäki <antti.kaijanmaki@canonical.com>
 */

#include <connectivity-service/connectivity-service.h>
#include <NetworkingStatusAdaptor.h>
#include <NetworkingStatusPrivateAdaptor.h>
#include <dbus-types.h>

using namespace nmofono;
using namespace std;

namespace connectivity_service
{

class ConnectivityService::Private : public QObject
{
    Q_OBJECT
public:
    ConnectivityService& p;

    QDBusConnection m_connection;

    Manager::Ptr m_manager;

    shared_ptr<PrivateService> m_privateService;

    QStringList m_limitations;

    QString m_status;

    Private(ConnectivityService& parent, const QDBusConnection& connection) :
        p(parent), m_connection(connection)
    {
    }

    void notifyPropertyChanged( const QObject& o,
                                const QString& path,
                                const QString& interface,
                                const QStringList& propertyNames )
    {
        QDBusMessage signal = QDBusMessage::createSignal(
            path,
            "org.freedesktop.DBus.Properties",
            "PropertiesChanged");
        signal << interface;
        QVariantMap changedProps;
        for(const auto& propertyName: propertyNames)
        {
            changedProps.insert(propertyName, o.property(qPrintable(propertyName)));
        }
        signal << changedProps;
        signal << QStringList();
        m_connection.send(signal);
    }

public Q_SLOTS:
    void flightModeUpdated()
    {
        notifyPropertyChanged(p,
                              DBusTypes::SERVICE_PATH,
                              DBusTypes::SERVICE_INTERFACE,
                              {
                                  "FlightMode",
                                  "HotspotSwitchEnabled",
                              });
    }

    void wifiEnabledUpdated()
    {
        notifyPropertyChanged(p,
                              DBusTypes::SERVICE_PATH,
                              DBusTypes::SERVICE_INTERFACE,
                              {
                                  "WifiEnabled",
                                  "HotspotSwitchEnabled"
                              });
    }

    void unstoppableOperationHappeningUpdated()
    {
        notifyPropertyChanged(p,
                              DBusTypes::SERVICE_PATH,
                              DBusTypes::SERVICE_INTERFACE,
                              {
                                   "FlightModeSwitchEnabled",
                                   "WifiSwitchEnabled",
                                   "HotspotSwitchEnabled"
                              });
    }

    void hotspotSsidUpdated()
    {
        notifyPropertyChanged(p,
                              DBusTypes::SERVICE_PATH,
                              DBusTypes::SERVICE_INTERFACE,
                              { "HotspotSsid" });
    }

    void hotspotEnabledUpdated()
    {
        notifyPropertyChanged(p,
                              DBusTypes::SERVICE_PATH,
                              DBusTypes::SERVICE_INTERFACE,
                              { "HotspotEnabled" });
    }

    void hotspotPasswordUpdated()
    {
        // Note that this is on the private object
        notifyPropertyChanged(*m_privateService,
                              DBusTypes::PRIVATE_PATH,
                              DBusTypes::PRIVATE_INTERFACE,
                              { "HotspotPassword" });
    }

    void hotspotModeUpdated()
    {
        notifyPropertyChanged(p,
                              DBusTypes::SERVICE_PATH,
                              DBusTypes::SERVICE_INTERFACE,
                              { "HotspotMode" });
    }

    // Note that this is on the private object
    void hotspotAuthUpdated()
    {
        notifyPropertyChanged(*m_privateService,
                              DBusTypes::SERVICE_PATH,
                              DBusTypes::SERVICE_INTERFACE,
                              { "HotspotAuth" });
    }

    void hotspotStoredUpdated()
    {
        notifyPropertyChanged(p,
                              DBusTypes::SERVICE_PATH,
                              DBusTypes::SERVICE_INTERFACE,
                              { "HotspotStored" });
    }

    void updateNetworkingStatus()
    {
        QStringList changed;

        QStringList old_limitations = m_limitations;
        QString old_status = m_status;

        switch (m_manager->status())
        {
            case Manager::NetworkingStatus::offline:
                m_status = "offline";
                break;
            case Manager::NetworkingStatus::connecting:
                m_status = "connecting";
                break;
            case Manager::NetworkingStatus::online:
                m_status = "online";
        }
        if (old_status != m_status)
        {
            changed << "Status";
        }

        QStringList limitations;
        auto characteristics = m_manager->characteristics();
        if ((characteristics
                & Link::Characteristics::is_bandwidth_limited) != 0)
        {
            // FIXME KNOWN TYPO
            limitations.push_back("bandwith");
        }
        m_limitations = limitations;
        if (old_limitations != m_limitations)
        {
            changed << "Limitations";
        }

        if (!changed.empty())
        {
            notifyPropertyChanged(p,
                                  DBusTypes::SERVICE_PATH,
                                  DBusTypes::SERVICE_INTERFACE,
                                  changed);
        }
    }
};

ConnectivityService::ConnectivityService(nmofono::Manager::Ptr manager, const QDBusConnection& connection)
    : d{new Private(*this, connection)}
{
    d->m_manager = manager;
    d->m_privateService = make_shared<PrivateService>(*this);

    // Memory is managed by Qt parent ownership
    new NetworkingStatusAdaptor(this);

    connect(d->m_manager.get(), &Manager::characteristicsUpdated, d.get(), &Private::updateNetworkingStatus);
    connect(d->m_manager.get(), &Manager::statusUpdated, d.get(), &Private::updateNetworkingStatus);
    connect(d->m_manager.get(), &Manager::flightModeUpdated, d.get(), &Private::flightModeUpdated);
    connect(d->m_manager.get(), &Manager::wifiEnabledUpdated, d.get(), &Private::wifiEnabledUpdated);
    connect(d->m_manager.get(), &Manager::unstoppableOperationHappeningUpdated, d.get(), &Private::unstoppableOperationHappeningUpdated);

    connect(d->m_manager.get(), &Manager::hotspotEnabledChanged, d.get(), &Private::hotspotEnabledUpdated);
    connect(d->m_manager.get(), &Manager::hotspotSsidChanged, d.get(), &Private::hotspotSsidUpdated);
    connect(d->m_manager.get(), &Manager::hotspotPasswordChanged, d.get(), &Private::hotspotPasswordUpdated);
    connect(d->m_manager.get(), &Manager::hotspotModeChanged, d.get(), &Private::hotspotModeUpdated);
    connect(d->m_manager.get(), &Manager::hotspotAuthChanged, d.get(), &Private::hotspotAuthUpdated);
    connect(d->m_manager.get(), &Manager::hotspotStoredChanged, d.get(), &Private::hotspotStoredUpdated);

    connect(d->m_manager.get(), &Manager::reportError, d->m_privateService.get(), &PrivateService::ReportError);

    d->updateNetworkingStatus();

    if (!d->m_connection.registerObject(DBusTypes::SERVICE_PATH, this))
    {
        throw logic_error(
                "Unable to register NetworkingStatus object on DBus");
    }
    if (!d->m_connection.registerObject(DBusTypes::PRIVATE_PATH, d->m_privateService.get()))
    {
        throw logic_error(
                "Unable to register NetworkingStatus private object on DBus");
    }
    if (!d->m_connection.registerService(DBusTypes::DBUS_NAME))
    {
        throw logic_error(
                "Unable to register Connectivity service on DBus");
    }
}

ConnectivityService::~ConnectivityService()
{
    d->m_connection.unregisterService(DBusTypes::DBUS_NAME);
}

QStringList ConnectivityService::limitations() const
{
    return d->m_limitations;
}

QString ConnectivityService::status() const
{
    return d->m_status;
}

bool ConnectivityService::wifiEnabled() const
{
    return d->m_manager->wifiEnabled();
}

bool ConnectivityService::flightMode() const
{
    return d->m_manager->flightMode();
}

bool ConnectivityService::flightModeSwitchEnabled() const
{
    return !d->m_manager->unstoppableOperationHappening();
}

bool ConnectivityService::wifiSwitchEnabled() const
{
    return !d->m_manager->unstoppableOperationHappening();
}

bool ConnectivityService::hotspotSwitchEnabled() const
{
    return !d->m_manager->unstoppableOperationHappening()
            && !d->m_manager->flightMode();
}

bool ConnectivityService::hotspotEnabled() const
{
    return d->m_manager->hotspotEnabled();
}

QByteArray ConnectivityService::hotspotSsid() const
{
    return d->m_manager->hotspotSsid();
}

QString ConnectivityService::hotspotMode() const
{
    return d->m_manager->hotspotMode();
}

bool ConnectivityService::hotspotStored() const
{
    return d->m_manager->hotspotStored();
}

PrivateService::PrivateService(ConnectivityService& parent) :
        p(parent)
{
    // Memory is managed by Qt parent ownership
    new PrivateAdaptor(this);
}

void PrivateService::UnlockAllModems()
{
    p.d->m_manager->unlockAllModems();
}

void PrivateService::UnlockModem(const QString &modem)
{
    p.d->m_manager->unlockModemByName(modem);
}

void PrivateService::SetFlightMode(bool enabled)
{
    p.d->m_manager->setFlightMode(enabled);
}

void PrivateService::SetWifiEnabled(bool enabled)
{
    p.d->m_manager->setWifiEnabled(enabled);
}

void PrivateService::SetHotspotEnabled(bool enabled)
{
    p.d->m_manager->setHotspotEnabled(enabled);
}

void PrivateService::SetHotspotSsid(const QByteArray &ssid)
{
    p.d->m_manager->setHotspotSsid(ssid);
}

void PrivateService::SetHotspotPassword(const QString &password)
{
    p.d->m_manager->setHotspotPassword(password);
}

void PrivateService::SetHotspotMode(const QString &mode)
{
    p.d->m_manager->setHotspotMode(mode);
}

void PrivateService::SetHotspotAuth(const QString &auth)
{
    p.d->m_manager->setHotspotAuth(auth);
}

QString PrivateService::hotspotPassword() const
{
    return p.d->m_manager->hotspotPassword();
}

QString PrivateService::hotspotAuth() const
{
    return p.d->m_manager->hotspotAuth();
}

}

#include "connectivity-service.moc"
