/*
 * Copyright (C) 2015 Canonical, Ltd.
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
 *     Pete Woods <pete.woods@canonical.com>
 */

#include <connectivityqt/connectivity.h>
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

    shared_ptr<internal::DBusPropertyCache> m_propertyCache;

    shared_ptr<internal::DBusPropertyCache> m_writePropertyCache;

    shared_ptr<ComUbuntuConnectivity1NetworkingStatusInterface> m_readInterface;

    shared_ptr<ComUbuntuConnectivity1PrivateInterface> m_writeInterface;

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
        else if (name == "UnstoppableOperationHappening")
        {
            Q_EMIT p.unstoppableOperationHappeningUpdated(value.toBool());
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
        else if (name == "HotspotActive")
        {
            Q_EMIT p.hotspotActiveUpdated(value.toBool());
        }
        else if (name == "HotspotName")
        {
            Q_EMIT p.hotspotNameUpdated(value.toByteArray());
        }
        else if (name == "HotspotPassword")
        {
            Q_EMIT p.hotspotPasswordUpdated(value.toString());
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

    d->m_propertyCache = make_shared<internal::DBusPropertyCache>(
            DBusTypes::DBUS_NAME, DBusTypes::SERVICE_INTERFACE,
            DBusTypes::SERVICE_PATH, sessionConnection);
    connect(d->m_propertyCache.get(),
            &internal::DBusPropertyCache::propertyChanged, d.get(),
            &Priv::propertyChanged);
    connect(d->m_propertyCache.get(),
            &internal::DBusPropertyCache::initialized, this,
            &Connectivity::initialized);
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
    return d->m_propertyCache->get("UnstoppableOperationHappening").toBool();
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
    return d->m_propertyCache->isInitialized();
}

void Connectivity::setFlightMode(bool enabled)
{
    d->m_writeInterface->SetFlightMode(enabled);
}

void Connectivity::setwifiEnabled(bool enabled)
{
    d->m_writeInterface->SetWifiEnabled(enabled);
}

QByteArray Connectivity::hotspotName() const
{
    return d->m_propertyCache->get("HotspotName").toByteArray();
}

QString Connectivity::hotspotPassword() const
{
    return d->m_writePropertyCache->get("HotspotPassword").toString();
}

bool Connectivity::hotspotActive() const
{
    return d->m_propertyCache->get("HotspotActive").toBool();
}

void Connectivity::setupHotspot(const QByteArray& ssid, const QString& password)
{
    d->m_writeInterface->SetupHotspot(ssid, password);
}

void Connectivity::setHotspotActive(bool active)
{
    d->m_writeInterface->SetHotspotActive(active);
}

}

#include "connectivity.moc"
