/*
 * Copyright © 2016 Canonical Ltd.
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

#pragma once

#include <QAbstractItemModel>
#include <QDBusConnection>
#include <QDBusObjectPath>
#include <connectivityqt/sim.h>

#include <lomiri/util/DefinesPtrs.h>

#include <memory>

namespace connectivityqt
{
namespace internal
{
struct SimsListModelParameters;
}

class Q_DECL_EXPORT SimsListModel : public QAbstractListModel
{
    Q_OBJECT

    Q_ENUMS(Roles)

public:

    LOMIRI_DEFINES_PTRS(SimsListModel);

    enum Roles
    {
        RoleIccid = Qt::UserRole + 1,
        RoleImsi,
        RolePrimaryPhoneNumber,
        RoleLocked,
        RolePresent,
        RoleMcc,
        RoleMnc,
        RolePreferredLanguages,
        RoleDataRoamingEnabled,
        RoleSim
    };

    SimsListModel(const internal::SimsListModelParameters& parameters);

    ~SimsListModel();

    int columnCount(const QModelIndex &parent) const override;

    int rowCount(const QModelIndex &parent) const override;

    QVariant data(const QModelIndex &index, int role) const override;

    QHash<int, QByteArray> roleNames() const override
    {
        QHash<int, QByteArray> roles;
        roles[RoleIccid] = "Iccid";
        roles[RoleImsi] = "Imsi";
        roles[RolePrimaryPhoneNumber] = "PrimaryPhoneNumber";
        roles[RoleLocked] = "Locked";
        roles[RolePresent] = "Present";
        roles[RoleMcc] = "Mcc";
        roles[RoleMnc] = "Mnc";
        roles[RolePreferredLanguages] = "PreferredLanguages";
        roles[RoleDataRoamingEnabled] = "DataRoamingEnabled";
        roles[RoleSim] = "Sim";
        return roles;
    }

    Sim::SPtr getSimByPath(const QDBusObjectPath &path) const;

Q_SIGNALS:
    void simsUpdated();

protected:
    class Priv;
    std::shared_ptr<Priv> d;
};

}
