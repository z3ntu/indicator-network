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

#include <NetworkManager.h>

#include <nmofono/vpn/vpn-connection.h>
#include <NetworkManagerSettingsConnectionInterface.h>

using namespace std;

namespace nmofono
{

using namespace connection;

namespace vpn
{

class VpnConnection::Priv: public QObject
{
    Q_OBJECT

public:
    Priv(VpnConnection& parent) :
        p(parent), m_type(Type::openvpn)
    {
    }

    void _connectionUpdated(const connection::ActiveConnection& activeConnection)
    {
        if (activeConnection.type() != "vpn")
        {
            return;
        }

        if (activeConnection.connectionPath().path() != m_connection->path())
        {
            return;
        }

        m_activeConnectionPath = activeConnection.path();

        static QMap<ActiveConnection::State, bool> STATE_ACTIVE_MAP
        {
            {ActiveConnection::State::unknown, false},
            {ActiveConnection::State::activating, true},
            {ActiveConnection::State::activated, true},
            {ActiveConnection::State::deactivating, false},
            {ActiveConnection::State::deactivated, false}
        };

        static QMap<ActiveConnection::State, bool> STATE_BUSY_MAP
        {
            {ActiveConnection::State::unknown, false},
            {ActiveConnection::State::activating, true},
            {ActiveConnection::State::activated, false},
            {ActiveConnection::State::deactivating, true},
            {ActiveConnection::State::deactivated, true}
        };

        auto state = activeConnection.state();
        setActive(STATE_ACTIVE_MAP[state]);
        setBusy(STATE_BUSY_MAP[state]);
    }

    void setActive(bool active)
    {
        if (m_active == active)
        {
            return;
        }

        m_active = active;
        Q_EMIT p.activeChanged(m_active);
    }

    void setBusy(bool busy)
    {
        if (m_busy == busy)
        {
            return;
        }

        m_busy = busy;
        Q_EMIT p.busyChanged(m_busy);
    }

    void updateUuid()
    {
        m_uuid = m_settings["connection"]["uuid"].toString();
    }

    void updateId()
    {
        QString id = m_settings["connection"]["id"].toString();

        if (id == m_id)
        {
            return;
        }

        m_id = id;
        Q_EMIT p.idChanged(m_id);
    }

    void updateValid()
    {
        QString type = m_settings["connection"]["type"].toString();
        bool valid(type == "vpn");

        if (valid == m_valid)
        {
            return;
        }

        m_valid = valid;
        Q_EMIT p.validChanged(m_valid);
    }

    void updateNeverDefault()
    {
    	auto ipv4 = m_settings.value("ipv4");
        bool neverDefault = false;
        if (ipv4.contains("never-default") && ipv4.value("never-default").toBool()) {
        	neverDefault = true;
        }

        if (neverDefault == m_neverDefault)
        {
            return;
        }

        m_neverDefault = neverDefault;
        Q_EMIT p.neverDefaultChanged(m_neverDefault);
    }

    void updateType()
    {
        static const QMap<QString, Type> typeMap
        {
            {"org.freedesktop.NetworkManager.openvpn", Type::openvpn},
            {"org.freedesktop.NetworkManager.pptp", Type::pptp}
        };

        QString serviceType = m_settings["vpn"]["service-type"].toString();
        m_type = typeMap.value(serviceType, Type::openvpn);
    }

    void updateActivatable()
    {
        QString otherPath = m_otherActiveConnectionPath.path();

        // If no connection is busy and there is an active connection and this is the active connection
        setActivatable(!m_otherConnectionIsBusy && (otherPath.isEmpty() || (otherPath == m_connection->path())));
    }

    void setActivatable(bool activatable)
    {
        if (m_activatable == activatable)
        {
            return;
        }

        m_activatable = activatable;
        Q_EMIT p.activatableChanged(m_activatable);
    }

    void setDirty()
    {
        if (m_dirty)
        {
            return;
        }

        m_dirty = true;
        m_pendingSettings = m_settings;

        m_dispatchPendingSettingsTimer.start();
    }

Q_SIGNALS:
    void updateData(const QStringMap& data);
    void updateSecrets(const QStringMap& secrets);

public Q_SLOTS:
    void dispatchPendingSettings()
    {
        auto reply = m_connection->Update(m_pendingSettings);
        reply.waitForFinished();
        if (reply.isError())
        {
            qWarning() << __PRETTY_FUNCTION__ << reply.error().message() << m_pendingSettings;
        }

        m_dirty = false;
    }

    void updateVpnData(const QStringMap &vpnData)
    {
        setDirty();
        m_pendingSettings["vpn"]["data"] = QVariant::fromValue(vpnData);
    }

    void updateVpnSecrets(const QStringMap &secrets)
    {
        setDirty();
        m_pendingSettings["vpn"]["secrets"] = QVariant::fromValue(secrets);
    }

    void secretsUpdated()
    {
        if (!m_valid)
        {
            return;
        }

        auto reply = m_connection->GetSecrets("vpn");
        reply.waitForFinished();
        if (reply.isError())
        {
            qWarning() << __PRETTY_FUNCTION__ << reply.error().message();
            return;
        }

        QVariantDictMap vpnSecrets = reply;

        QStringMap secrets;
        QVariant v = vpnSecrets.value("vpn").value("secrets");
        if (v.isValid())
        {
            auto dbusArgument = qvariant_cast<QDBusArgument>(v);
            dbusArgument >> secrets;
        }

        Q_EMIT updateSecrets(secrets);
    }

    void settingsUpdated()
    {
        auto reply = m_connection->GetSettings();

        reply.waitForFinished();
        if (reply.isError())
        {
            qWarning() << __PRETTY_FUNCTION__ << reply.error().message();
            return;
        }

        m_settings = reply;

        QStringMap vpnData;
        // Encourage Qt to decode the nested map
        auto v = m_settings.value("vpn").value("data");
        if (v.isValid())
        {
            auto dbusArgument = qvariant_cast<QDBusArgument>(v);
            dbusArgument >> vpnData;
            m_settings["vpn"]["data"] = QVariant::fromValue(vpnData);
        }

        updateId();
        updateNeverDefault();
        updateValid();
        updateType();

        if (m_valid)
        {
            Q_EMIT updateData(vpnData);
        }
    }

    void activeConnectionsUpdated(const QSet<connection::ActiveConnection::SPtr>& activeConnections)
    {
        connection::ActiveConnection::SPtr activeConnection;

        QDBusObjectPath path(m_connection->path());

        // Search for the active connection
        for (const auto& con : activeConnections)
        {
            // Connection to all signals on all active connections
            connect(con.get(), &connection::ActiveConnection::connectionPathChanged, this, &Priv::connectionUpdated);
            connect(con.get(), &connection::ActiveConnection::stateChanged, this, &Priv::connectionUpdated);
            connect(con.get(), &connection::ActiveConnection::typeChanged, this, &Priv::connectionUpdated);

            if (con->connectionPath() == path)
            {
                activeConnection = con;
                break;
            }
        }

        if (activeConnection)
        {
            _connectionUpdated(*activeConnection);
        }
        else
        {
            setActive(false);
            setBusy(false);
            m_activeConnectionPath.setPath("/");
        }
    }

    void connectionUpdated()
    {
        auto activeConnection = qobject_cast<connection::ActiveConnection*>(sender());
        _connectionUpdated(*activeConnection);
    }

public:
    VpnConnection& p;

    shared_ptr<OrgFreedesktopNetworkManagerSettingsConnectionInterface> m_connection;

    connection::ActiveConnectionManager::SPtr m_activeConnectionManager;

    QVariantDictMap m_settings;

    bool m_dirty = false;

    QVariantDictMap m_pendingSettings;

    QTimer m_dispatchPendingSettingsTimer;

    QString m_uuid;

    QString m_id;

    bool m_neverDefault = false;

    bool m_valid = false;

    Type m_type;

    bool m_active = false;

    bool m_busy = false;

    bool m_activatable = false;

    QDBusObjectPath m_activeConnectionPath;

    bool m_otherConnectionIsBusy = false;

    QDBusObjectPath m_otherActiveConnectionPath;

    OpenvpnConnection::SPtr m_openvpnConnection;

    PptpConnection::SPtr m_pptpConnection;
};

VpnConnection::VpnConnection(
        const QDBusObjectPath& path,
        connection::ActiveConnectionManager::SPtr activeConnectionManager,
        const QDBusConnection& systemConnection) :
        d(new Priv(*this))
{
    d->m_dispatchPendingSettingsTimer.setSingleShot(true);
    d->m_dispatchPendingSettingsTimer.setInterval(200);
    d->m_dispatchPendingSettingsTimer.setTimerType(Qt::CoarseTimer);
    connect(&d->m_dispatchPendingSettingsTimer, &QTimer::timeout, d.get(), &Priv::dispatchPendingSettings);

    d->m_connection = make_shared<OrgFreedesktopNetworkManagerSettingsConnectionInterface>(NM_DBUS_SERVICE, path.path(), systemConnection);

    d->m_activeConnectionManager = activeConnectionManager;

    d->settingsUpdated();
    d->updateUuid();
    connect(d->m_connection.get(), &OrgFreedesktopNetworkManagerSettingsConnectionInterface::Updated, d.get(), &Priv::settingsUpdated);

    if (!isValid())
    {
        return;
    }

    d->activeConnectionsUpdated(d->m_activeConnectionManager->connections());
    connect(d->m_activeConnectionManager.get(), &connection::ActiveConnectionManager::connectionsChanged, d.get(), &Priv::activeConnectionsUpdated);

    switch (d->m_type)
    {
        case Type::openvpn:
            d->m_openvpnConnection = make_shared<OpenvpnConnection>();
            d->m_openvpnConnection->updateData(d->m_settings["vpn"]["data"].value<QStringMap>());
            connect(d.get(), &Priv::updateData, d->m_openvpnConnection.get(), &OpenvpnConnection::updateData);
            connect(d.get(), &Priv::updateSecrets, d->m_openvpnConnection.get(), &OpenvpnConnection::updateSecrets);
            connect(d->m_openvpnConnection.get(), &OpenvpnConnection::updateVpnData, d.get(), &Priv::updateVpnData);
            connect(d->m_openvpnConnection.get(), &OpenvpnConnection::updateVpnSecrets, d.get(), &Priv::updateVpnSecrets);
            break;
        case Type::pptp:
            d->m_pptpConnection = make_shared<PptpConnection>();
            d->m_pptpConnection->updateData(d->m_settings["vpn"]["data"].value<QStringMap>());
            connect(d.get(), &Priv::updateData, d->m_pptpConnection.get(), &PptpConnection::updateData);
            connect(d.get(), &Priv::updateSecrets, d->m_pptpConnection.get(), &PptpConnection::updateSecrets);
            connect(d->m_pptpConnection.get(), &PptpConnection::updateVpnData, d.get(), &Priv::updateVpnData);
            connect(d->m_pptpConnection.get(), &PptpConnection::updateVpnSecrets, d.get(), &Priv::updateVpnSecrets);
            break;
        default:
            break;
    }
}

QString VpnConnection::uuid() const
{
    return d->m_uuid;
}

QString VpnConnection::id() const
{
    return d->m_id;
}

bool VpnConnection::neverDefault() const
{
    return d->m_neverDefault;
}

QDBusObjectPath VpnConnection::path() const
{
    return QDBusObjectPath(d->m_connection->path());
}

void VpnConnection::setActive(bool active)
{
    if (active == d->m_active)
    {
        return;
    }

    if (active)
    {
        // Set active properly early as we are not always quick enough
        // to see a temporarily active ActiveConnection object
        d->setActive(true);
        Q_EMIT activateConnection(QDBusObjectPath(d->m_connection->path()));
    }
    else
    {
        Q_EMIT deactivateConnection(d->m_activeConnectionPath);
    }
}

void VpnConnection::setId(const QString& id)
{
    if (d->m_id == id)
    {
        return;
    }

    d->setDirty();
    d->m_pendingSettings["connection"]["id"] = id;
}

void VpnConnection::setNeverDefault(bool neverDefault)
{
    if (d->m_neverDefault == neverDefault)
    {
        return;
    }

    d->setDirty();
    d->m_pendingSettings["ipv4"]["never-default"] = neverDefault;
    d->m_pendingSettings["ipv6"]["never-default"] = neverDefault;
}

void VpnConnection::setOtherConnectionIsBusy(bool otherConnectionIsBusy)
{
    d->m_otherConnectionIsBusy = otherConnectionIsBusy;
    d->updateActivatable();
}

void VpnConnection::setActiveConnectionPath(const QDBusObjectPath& path)
{
    d->m_otherActiveConnectionPath = path;
    d->updateActivatable();
}

void VpnConnection::updateSecrets()
{
    d->secretsUpdated();
}

void VpnConnection::remove()
{
    d->m_connection->Delete();
}

bool VpnConnection::isActive() const
{
    return d->m_active;
}

bool VpnConnection::isValid() const
{
    return d->m_valid;
}

bool VpnConnection::isBusy() const
{
    return d->m_busy;
}

bool VpnConnection::isActivatable() const
{
    return d->m_activatable;
}

VpnConnection::Type VpnConnection::type() const
{
    return d->m_type;
}

OpenvpnConnection::SPtr VpnConnection::openvpnConnection() const
{
    return d->m_openvpnConnection;
}

PptpConnection::SPtr VpnConnection::pptpConnection() const
{
    return d->m_pptpConnection;
}

}
}

#include "vpn-connection.moc"
