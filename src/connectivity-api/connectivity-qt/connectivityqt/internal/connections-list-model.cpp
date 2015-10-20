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

#include <connectivityqt/internal/connections-list-model.h>
#include <connectivityqt/vpn-connection.h>

#include <QDBusArgument>
#include <QDBusObjectPath>
#include <QDebug>
#include <QList>

using namespace std;

namespace connectivityqt
{
namespace internal
{

class ConnectionsListModel::Priv: public QObject
{
    Q_OBJECT

public:
    Priv(ConnectionsListModel& parent) :
        p(parent)
    {
    }

    void updatePaths(const QVariant& value)
    {
        QList<QDBusObjectPath> tmp;
        qvariant_cast<QDBusArgument>(value) >> tmp;
        auto paths = tmp.toSet();

        QSet<QDBusObjectPath> current;
        for (const auto& connection: m_vpnConnections)
        {
            current << connection->path();
        }

        auto toRemove(current);
        toRemove.subtract(paths);

        auto toAdd(paths);
        toAdd.subtract(current);

        QMutableListIterator<VpnConnection::SPtr> i(m_vpnConnections);
        int idx = 0;
        while (i.hasNext())
        {
            auto vpnConnection(i.next());
            if (toRemove.contains(vpnConnection->path()))
            {
                p.beginRemoveRows(QModelIndex(), idx, idx);
                i.remove();
                p.endRemoveRows();
            }
            else
            {
                ++idx;
            }
        }

        if (!toAdd.isEmpty())
        {
            p.beginInsertRows(QModelIndex(), m_vpnConnections.size(), m_vpnConnections.size() + toAdd.size() - 1);
            for (const auto& path: toAdd)
            {
                auto vpnConnection = make_shared<VpnConnection>(path, m_propertyCache->connection());
                m_vpnConnections << vpnConnection;
                connect(vpnConnection.get(), &VpnConnection::idChanged, this, &Priv::connectionIdChanged);
                connect(vpnConnection.get(), &VpnConnection::activeChanged, this, &Priv::connectionActiveChanged);
            }
            p.endInsertRows();
        }
    }

    QModelIndex findVpnConnection(QObject* o)
    {
        auto vpnConnection = qobject_cast<VpnConnection*>(o);
        for (auto i = m_vpnConnections.constBegin(); i != m_vpnConnections.constEnd(); ++i)
        {
            if (i->get() == vpnConnection)
            {
                return p.index(std::distance(m_vpnConnections.constBegin(), i));
            }
        }
        return QModelIndex();
    }

public Q_SLOTS:
    void connectionIdChanged(const QString&)
    {
        auto idx = findVpnConnection(sender());
        p.dataChanged(idx, idx, {internal::ConnectionsListModel::Roles::RoleId});
    }

    void connectionActiveChanged(bool)
    {
        auto idx = findVpnConnection(sender());
        p.dataChanged(idx, idx, {internal::ConnectionsListModel::Roles::RoleActive});
    }

    void propertyChanged(const QString& name, const QVariant& value)
    {
        if (name == "VpnConnections")
        {
            updatePaths(value);
        }
    }

public:
    ConnectionsListModel& p;

    DBusPropertyCache::SPtr m_propertyCache;

    QList<VpnConnection::SPtr> m_vpnConnections;
};

ConnectionsListModel::ConnectionsListModel(DBusPropertyCache::SPtr propertyCache) :
        d(new Priv(*this))
{
    d->m_propertyCache = propertyCache;
    d->updatePaths(d->m_propertyCache->get("VpnConnections"));
    connect(d->m_propertyCache.get(), &DBusPropertyCache::propertyChanged, d.get(), &Priv::propertyChanged);
}

ConnectionsListModel::~ConnectionsListModel()
{
}

int ConnectionsListModel::rowCount(const QModelIndex &) const
{
    return d->m_vpnConnections.size();
}

QVariant ConnectionsListModel::data(const QModelIndex &index, int role) const
{
    int row(index.row());
    if (row < 0 || row >= d->m_vpnConnections.size())
    {
        return QVariant();
    }

    auto vpnConnection = d->m_vpnConnections.value(row);

    switch (role)
    {
        case Roles::RoleId:
            return vpnConnection->id();
            break;
        case Roles::RoleActive:
            return vpnConnection->active();
            break;
        default:
            break;
    }

    return QVariant();
}

}
}

#include "connections-list-model.moc"
