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

#pragma once

#include <connectivityqt/internal/dbus-property-cache.h>

#include <QAbstractItemModel>
#include <unity/util/DefinesPtrs.h>

namespace connectivityqt
{
namespace internal
{
class DBusPropertyCache;
}

class ConnectionsListModel : public QAbstractListModel
{
    Q_ENUMS(Roles)

public:
    UNITY_DEFINES_PTRS(ConnectionsListModel);

    enum Roles
    {
        RoleId,
        RoleActive
    };

    ConnectionsListModel(std::shared_ptr<internal::DBusPropertyCache> propertyCache);

    ~ConnectionsListModel();

    int rowCount(const QModelIndex &parent) const override;

    QVariant data(const QModelIndex &index, int role) const override;

    bool setData(const QModelIndex &index, const QVariant &value, int role) override;

    QHash<int, QByteArray> roleNames() const override
    {
        QHash<int, QByteArray> roles;
        roles[RoleId] = "id";
        roles[RoleActive] = "active";
        return roles;
    }

protected:
    class Priv;
    std::shared_ptr<Priv> d;
};

}
