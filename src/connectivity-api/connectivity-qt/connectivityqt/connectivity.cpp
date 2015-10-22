/*
 * Copyright © 2014 Canonical Ltd.
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
 *     Pete Woods <pete.woods@canonical.com>
 */

#include <connectivityqt/connectivity.h>
#include <connectivityqt/connections-list-model.h>
#include <connectivityqt/internal/dbus-property-cache.h>
#include <dbus-types.h>
#include <NetworkingStatusInterface.h>
#include <NetworkingStatusPrivateInterface.h>

#include <QDebug>

using namespace std;

namespace connectivityqt
{

class Connectivity::Priv: public QObject
{
    Q_OBJECT
public:
    Priv(Connectivity& parent, const QDBusConnection& sessionConnection) :
            p(parent), m_sessionConnection(sessionConnection)
    {
    }

    Connectivity& p;

    QDBusConnection m_sessionConnection;

    internal::DBusPropertyCache::SPtr m_propertyCache;

    internal::DBusPropertyCache::SPtr m_writePropertyCache;

    shared_ptr<ComUbuntuConnectivity1NetworkingStatusInterface> m_readInterface;

    shared_ptr<ComUbuntuConnectivity1PrivateInterface> m_writeInterface;

    ConnectionsListModel::SPtr m_connectionsModel;

    static QVector<Limitations> toLimitations(const QVariant& value)
    {
        auto l = value.toStringList();
        QVector<Limitations> result;
        for (const auto& limit : l)
        {
            // FIXME KNOWN TYPO
            if (limit == "bandwith")
            {
                result.push_back(Limitations::Bandwith);
            }
        }
        return result;
    }

    static Status toStatus(const QVariant& value)
    {
        auto s = value.toString();

        if (s == "offline")
        {
            return Status::Offline;
        }
        else if (s == "connecting")
        {
            return Status::Connecting;
        }
        else if (s == "online")
        {
            return Status::Online;
        }

        return Status::Offline;
    }

public Q_SLOTS:
    void interfaceInitialized()
    {
        // If both interfaces have been initialized then we're good to go
        if (p.isInitialized())
        {
            Q_EMIT p.initialized();
        }
    }

    void propertyChanged(const QString& name, const QVariant& value)
    {
        if (name == "FlightMode")
        {
            Q_EMIT p.flightModeUpdated(value.toBool());
        }
        else if (name == "WifiEnabled")
        {
            Q_EMIT p.wifiEnabledUpdated(value.toBool());
        }
        else if (name == "FlightModeSwitchEnabled")
        {
            Q_EMIT p.flightModeSwitchEnabledUpdated(value.toBool());
        }
        else if (name == "WifiSwitchEnabled")
        {
            Q_EMIT p.wifiSwitchEnabledUpdated(value.toBool());
        }
        else if (name == "HotspotSwitchEnabled")
        {
            Q_EMIT p.hotspotSwitchEnabledUpdated(value.toBool());
        }
        else if (name == "Limitations")
        {
            auto limitations = toLimitations(value);
            Q_EMIT p.limitationsUpdated(limitations);
            Q_EMIT p.limitedBandwithUpdated(limitations.contains(Limitations::Bandwith));
        }
        else if (name == "Status")
        {
            auto status = toStatus(value);
            Q_EMIT p.statusUpdated(status);
            Q_EMIT p.onlineUpdated(status == Status::Online);
        }
        else if (name == "ModemAvailable")
        {
            Q_EMIT p.modemAvailableUpdated(value.toBool());
        }
        else if (name == "HotspotEnabled")
        {
            Q_EMIT p.hotspotEnabledUpdated(value.toBool());
        }
        else if (name == "HotspotSsid")
        {
            Q_EMIT p.hotspotSsidUpdated(value.toByteArray());
        }
        else if (name == "HotspotPassword")
        {
            Q_EMIT p.hotspotPasswordUpdated(value.toString());
        }
        else if (name == "HotspotMode")
        {
            Q_EMIT p.hotspotModeUpdated(value.toString());
        }
        else if (name == "HotspotAuth")
        {
            Q_EMIT p.hotspotAuthUpdated(value.toString());
        }
        else if (name == "HotspotStored")
        {
            Q_EMIT p.hotspotStoredUpdated(value.toBool());
        }
    }
};

void Connectivity::registerMetaTypes()
{
    DBusTypes::registerMetaTypes();

    qRegisterMetaType<connectivityqt::Connectivity::Limitations>("connectivityqt::Connectivity::Limitations");
    qRegisterMetaType<QVector<connectivityqt::Connectivity::Limitations>>("QVector<connectivityqt::Connectivity::Limitations>");
    qRegisterMetaType<connectivityqt::Connectivity::Status>("connectivityqt::Connectivity::Status");
}

Connectivity::Connectivity(const QDBusConnection& sessionConnection, QObject* parent) :
        QObject(parent), d(new Priv(*this, sessionConnection))
{
    d->m_readInterface = make_shared<
            ComUbuntuConnectivity1NetworkingStatusInterface>(
            DBusTypes::DBUS_NAME, DBusTypes::SERVICE_PATH,
            d->m_sessionConnection);

    d->m_writeInterface = make_shared<ComUbuntuConnectivity1PrivateInterface>(
            DBusTypes::DBUS_NAME, DBusTypes::PRIVATE_PATH,
            d->m_sessionConnection);

    d->m_writePropertyCache = make_shared<internal::DBusPropertyCache>(
                DBusTypes::DBUS_NAME, DBusTypes::PRIVATE_INTERFACE,
                DBusTypes::PRIVATE_PATH, sessionConnection);
    connect(d->m_writePropertyCache.get(),
            &internal::DBusPropertyCache::propertyChanged, d.get(),
            &Priv::propertyChanged);
    connect(d->m_writePropertyCache.get(),
            &internal::DBusPropertyCache::initialized, d.get(),
            &Connectivity::Priv::interfaceInitialized);

    d->m_propertyCache = make_shared<internal::DBusPropertyCache>(
            DBusTypes::DBUS_NAME, DBusTypes::SERVICE_INTERFACE,
            DBusTypes::SERVICE_PATH, sessionConnection);
    connect(d->m_propertyCache.get(),
            &internal::DBusPropertyCache::propertyChanged, d.get(),
            &Priv::propertyChanged);
    connect(d->m_propertyCache.get(),
            &internal::DBusPropertyCache::initialized, d.get(),
            &Connectivity::Priv::interfaceInitialized);

    connect(d->m_writeInterface.get(),
            &ComUbuntuConnectivity1PrivateInterface::ReportError, this,
            &Connectivity::reportError);

    d->m_connectionsModel = make_shared<ConnectionsListModel>(d->m_writePropertyCache);
}

Connectivity::~Connectivity()
{
}

bool Connectivity::flightMode() const
{
    return d->m_propertyCache->get("FlightMode").toBool();
}

bool Connectivity::wifiEnabled() const
{
    return d->m_propertyCache->get("WifiEnabled").toBool();
}

bool Connectivity::unstoppableOperationHappening() const
{
    return false;
}

bool Connectivity::flightModeSwitchEnabled() const
{
    return d->m_propertyCache->get("FlightModeSwitchEnabled").toBool();
}

bool Connectivity::wifiSwitchEnabled() const
{
    return d->m_propertyCache->get("WifiSwitchEnabled").toBool();
}

bool Connectivity::hotspotSwitchEnabled() const
{
    return d->m_propertyCache->get("HotspotSwitchEnabled").toBool();
}

QVector<Connectivity::Limitations> Connectivity::limitations() const
{
    return d->toLimitations(d->m_propertyCache->get("Limitations"));
}

Connectivity::Status Connectivity::status() const
{
    return d->toStatus(d->m_propertyCache->get("Status"));
}

bool Connectivity::online() const
{
    return (status() ==  Status::Online);
}

bool Connectivity::limitedBandwith() const
{
    return limitations().contains(Limitations::Bandwith);
}

bool Connectivity::isInitialized() const
{
    return d->m_propertyCache->isInitialized()
            && d->m_writePropertyCache->isInitialized();
}

void Connectivity::setFlightMode(bool enabled)
{
    d->m_writeInterface->SetFlightMode(enabled);

    // TODO Remove this when SDK switch widget isn't broken
    d->m_propertyCache->setProperty("FlightMode", enabled);
}

void Connectivity::setwifiEnabled(bool enabled)
{
    d->m_writeInterface->SetWifiEnabled(enabled);

    // TODO Remove this when SDK switch widget isn't broken
    d->m_propertyCache->setProperty("WifiEnabled", enabled);
}

QByteArray Connectivity::hotspotSsid() const
{
    return d->m_propertyCache->get("HotspotSsid").toByteArray();
}

QString Connectivity::hotspotPassword() const
{
    return d->m_writePropertyCache->get("HotspotPassword").toString();
}

bool Connectivity::modemAvailable() const
{
    return d->m_propertyCache->get("ModemAvailable").toBool();
}

bool Connectivity::hotspotEnabled() const
{
    return d->m_propertyCache->get("HotspotEnabled").toBool();
}

QString Connectivity::hotspotMode() const
{
    return d->m_propertyCache->get("HotspotMode").toString();
}

QString Connectivity::hotspotAuth() const
{
    return d->m_writePropertyCache->get("HotspotAuth").toString();
}

bool Connectivity::hotspotStored() const
{
    return d->m_propertyCache->get("HotspotStored").toBool();
}

void Connectivity::setHotspotSsid(const QByteArray& ssid)
{
    d->m_writeInterface->SetHotspotSsid(ssid);
}

void Connectivity::setHotspotPassword(const QString& password)
{
    d->m_writeInterface->SetHotspotPassword(password);
}

void Connectivity::setHotspotEnabled(bool enabled)
{
    d->m_writeInterface->SetHotspotEnabled(enabled);

    // TODO Remove this when SDK switch widget isn't broken
    d->m_propertyCache->setProperty("HotspotEnabled", enabled);
}

void Connectivity::setHotspotMode(const QString& mode)
{
    d->m_writeInterface->SetHotspotMode(mode);
}

void Connectivity::setHotspotAuth(const QString& auth)
{
    d->m_writeInterface->SetHotspotAuth(auth);
}

QAbstractListModel* Connectivity::vpnConnections() const
{
    return d->m_connectionsModel.get();
}

}

#include "connectivity.moc"
