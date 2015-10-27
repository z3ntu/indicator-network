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

#include <nmofono/vpn/vpn-manager.h>
#include <NetworkManager.h>
#include <QMap>

#include <NetworkManagerInterface.h>
#include <NetworkManagerSettingsInterface.h>

using namespace std;

namespace nmofono
{
namespace vpn
{

class VpnManager::Priv: public QObject
{
    Q_OBJECT

public:
    Priv(VpnManager& parent) :
        p(parent)
    {
    }

    void _newConnection(const QDBusObjectPath &path, bool shouldUpdateActiveAndBusy)
    {
        auto connection = make_shared<VpnConnection>(path, m_activeConnectionManager, m_settingsInterface->connection());
        if (connection->isValid())
        {
            m_connections[path.path()] = connection;
            connection->setOtherConnectionIsBusy(m_busy);
            connection->setActiveConnectionPath(m_activeConnectionPath);
            connect(connection.get(), &VpnConnection::activateConnection, this, &Priv::activateConnection);
            connect(connection.get(), &VpnConnection::deactivateConnection, m_nmInterface.get(), &OrgFreedesktopNetworkManagerInterface::DeactivateConnection);
            connect(connection.get(), &VpnConnection::activeChanged, this, &Priv::updateActiveAndBusy);
            connect(connection.get(), &VpnConnection::busyChanged, this, &Priv::updateActiveAndBusy);
            connect(this, &Priv::busyChanged, connection.get(), &VpnConnection::setOtherConnectionIsBusy);
            connect(this, &Priv::activeConnectionPathChanged, connection.get(), &VpnConnection::setActiveConnectionPath);
            Q_EMIT p.connectionsChanged();
            if (shouldUpdateActiveAndBusy)
            {
                updateActiveAndBusy();
            }
        }
    }

Q_SIGNALS:
    void busyChanged(bool busy);

    void activeConnectionPathChanged(const QDBusObjectPath& path);

public Q_SLOTS:
    void connectionRemoved(const QDBusObjectPath &path)
    {
        auto connection = m_connections.take(path.path());
        if (connection)
        {
            Q_EMIT p.connectionsChanged();
            updateActiveAndBusy();
        }
    }

    void newConnection(const QDBusObjectPath &path)
    {
        _newConnection(path, true);
    }

    void activateConnection(const QDBusObjectPath& connection)
    {
        m_nmInterface->ActivateConnection(connection, QDBusObjectPath("/"), QDBusObjectPath("/"));
    }

    void updateActiveAndBusy()
    {
        bool busy = false;
        for (const auto& connection: m_connections)
        {
            if (connection->isBusy())
            {
                busy = true;
                break;
            }
        }
        setBusy(busy);

        QDBusObjectPath activeConnectionPath;
        for (const auto& connection: m_connections)
        {
            if (connection->isActive())
            {
                activeConnectionPath = connection->path();
                break;
            }
        }
        setActiveConnectionPath(activeConnectionPath);
    }

    void setBusy(bool busy)
    {
        if (m_busy == busy)
        {
            return;
        }

        m_busy = busy;
        Q_EMIT busyChanged(m_busy);
    }

    void setActiveConnectionPath(const QDBusObjectPath& activeConnectionPath)
    {
        if (m_activeConnectionPath == activeConnectionPath)
        {
            return;
        }

        m_activeConnectionPath = activeConnectionPath;
        Q_EMIT activeConnectionPathChanged(m_activeConnectionPath);
    }

public:
    VpnManager& p;

    connection::ActiveConnectionManager::SPtr m_activeConnectionManager;

    shared_ptr<OrgFreedesktopNetworkManagerInterface> m_nmInterface;

    shared_ptr<OrgFreedesktopNetworkManagerSettingsInterface> m_settingsInterface;

    QMap<QString, VpnConnection::SPtr> m_connections;

    bool m_busy = false;

    QDBusObjectPath m_activeConnectionPath;
};

VpnManager::VpnManager(connection::ActiveConnectionManager::SPtr activeConnectionManager, const QDBusConnection& systemConnection) :
        d(new Priv(*this))
{
    d->m_activeConnectionManager = activeConnectionManager;
    d->m_nmInterface = make_shared<OrgFreedesktopNetworkManagerInterface>(
                NM_DBUS_SERVICE, NM_DBUS_PATH, systemConnection);
    d->m_settingsInterface = make_shared<OrgFreedesktopNetworkManagerSettingsInterface>(
                NM_DBUS_SERVICE, NM_DBUS_PATH_SETTINGS, systemConnection);

    for (const auto& path : d->m_settingsInterface->connections())
    {
        d->_newConnection(path, false);
    }
    d->updateActiveAndBusy();
    connect(d->m_settingsInterface.get(), &OrgFreedesktopNetworkManagerSettingsInterface::NewConnection, d.get(), &Priv::newConnection);
    connect(d->m_settingsInterface.get(), &OrgFreedesktopNetworkManagerSettingsInterface::ConnectionRemoved, d.get(), &Priv::connectionRemoved);
}

QList<VpnConnection::SPtr> VpnManager::connections() const
{
    return d->m_connections.values();
}

}
}

#include "vpn-manager.moc"
