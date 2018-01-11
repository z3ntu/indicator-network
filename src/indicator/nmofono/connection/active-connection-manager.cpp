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
 * Author: Pete Woods <pete.woods@canonical.com>
 */

#include <nmofono/connection/active-connection-manager.h>
#include <NetworkManagerInterface.h>
#include <util/qhash-sharedptr.h>

#include <NetworkManager.h>

using namespace std;

namespace nmofono
{
namespace connection
{

class ActiveConnectionManager::Priv: public QObject
{
    Q_OBJECT

public:
    Priv(ActiveConnectionManager& parent) :
        p(parent)
    {
    }

    void updateConnections(const QList<QDBusObjectPath>& connectionsList)
    {
        auto current(m_connections.keys().toSet());
        auto connections(connectionsList.toSet());

        auto toRemove(current);
        toRemove.subtract(connections);

        auto toAdd(connections);
        toAdd.subtract(current);

        for (const auto& path: toRemove)
        {
            m_connections.remove(path);
        }

        for (const auto& path: toAdd)
        {
            m_connections[path] = make_shared<ActiveConnection>(path, m_manager->connection());
        }

        if (!toRemove.isEmpty() || !toAdd.isEmpty())
        {
            Q_EMIT p.connectionsChanged(m_connections.values().toSet());
            Q_EMIT p.connectionsUpdated();
        }
    }

    ActiveConnection::SPtr addConnection(const QDBusObjectPath& path)
    {
        ActiveConnection::SPtr connection;

        auto it = m_connections.constFind(path);
        if (it == m_connections.constEnd())
        {
            connection = make_shared<ActiveConnection>(path, m_manager->connection());
            m_connections.insert(path, connection);
            Q_EMIT p.connectionsChanged(m_connections.values().toSet());
            Q_EMIT p.connectionsUpdated();
        }
        else
        {
            connection = *it;
        }

        return connection;
    }

public Q_SLOTS:
    void propertiesChanged(const QVariantMap &properties)
    {
        QMapIterator<QString, QVariant> it(properties);
        while (it.hasNext())
        {
            it.next();
            QString property = it.key();
            QVariant value = it.value();

            if (property == "ActiveConnections")
            {
                QList<QDBusObjectPath> activeConnections;
                value.value<QDBusArgument>() >> activeConnections;
                updateConnections(activeConnections);
            }
        }
    }

public:
    ActiveConnectionManager& p;

    shared_ptr<OrgFreedesktopNetworkManagerInterface> m_manager;

    QMap<QDBusObjectPath, ActiveConnection::SPtr> m_connections;
};

ActiveConnectionManager::ActiveConnectionManager(const QDBusConnection& systemConnection) :
        d(new Priv(*this))
{
    d->m_manager = make_shared<OrgFreedesktopNetworkManagerInterface>(NM_DBUS_SERVICE, NM_DBUS_PATH, systemConnection);

    d->updateConnections(d->m_manager->activeConnections());

    connect(d->m_manager.get(), &OrgFreedesktopNetworkManagerInterface::PropertiesChanged, d.get(), &Priv::propertiesChanged);
}

QSet<ActiveConnection::SPtr> ActiveConnectionManager::connections() const
{
    return d->m_connections.values().toSet();
}

ActiveConnection::SPtr ActiveConnectionManager::connection(const QDBusObjectPath& path) const
{
    auto connection = d->m_connections.value(path);
    if (path.path() != "/" && !connection)
    {
        d->updateConnections(d->m_manager->activeConnections());
        connection = d->m_connections.value(path);
        if (!connection)
        {
            qWarning() << "Could not find connection at path" << path.path();
        }
    }
    return connection;
}

bool ActiveConnectionManager::deactivate(ActiveConnection::SPtr activeConnection)
{
    auto reply = d->m_manager->DeactivateConnection(activeConnection->path());
    reply.waitForFinished();
    if (reply.isError())
    {
        qWarning() << reply.error().message();
        return false;
    }
    return true;
}

ActiveConnection::SPtr ActiveConnectionManager::activate(const QDBusObjectPath& connection, const QDBusObjectPath& device, const QDBusObjectPath& specificObject)
{
    auto reply = d->m_manager->ActivateConnection(connection, device, specificObject);
    reply.waitForFinished();
    if (reply.isError())
    {
        qWarning() << reply.error().message();
        return ActiveConnection::SPtr();
    }
    return d->addConnection(reply);
}


ActiveConnection::SPtr ActiveConnectionManager::addAndActivate(const QVariantDictMap &connection, const QDBusObjectPath &device, const QDBusObjectPath &specificObject)
{
    auto reply = d->m_manager->AddAndActivateConnection(connection, device, specificObject);
    reply.waitForFinished();
    if (reply.isError())
    {
        qWarning() << reply.error().message();
        return ActiveConnection::SPtr();
    }
    return d->addConnection(reply.argumentAt<1>());
}

}
}

#include "active-connection-manager.moc"
