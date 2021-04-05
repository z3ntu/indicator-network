/*
 * Copyright © 2015 Canonical Ltd.
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

#include <QDBusConnection>
#include <QDBusObjectPath>
#include <QObject>

#include <lomiri/util/DefinesPtrs.h>

namespace connectivityqt
{

class Q_DECL_EXPORT VpnConnection : public QObject
{
    Q_OBJECT

public:
    LOMIRI_DEFINES_PTRS(VpnConnection);

    Q_ENUMS(Type)

    enum Type
    {
        OPENVPN,
        PPTP
    };

    VpnConnection(const QDBusObjectPath& path, const QDBusConnection& connection, QObject* parent = 0);

    virtual ~VpnConnection();

    Q_PROPERTY(QDBusObjectPath path READ path)
    QDBusObjectPath path() const;

    Q_PROPERTY(QString id READ id WRITE setId NOTIFY idChanged)
    QString id() const;

    Q_PROPERTY(bool neverDefault READ neverDefault WRITE setNeverDefault NOTIFY neverDefaultChanged)
    bool neverDefault() const;

    Q_PROPERTY(bool active READ active WRITE setActive NOTIFY activeChanged)
    bool active() const;

    Q_PROPERTY(bool activatable READ activatable NOTIFY activatableChanged)
    bool activatable() const;

    Q_PROPERTY(Type type READ type)
    virtual Type type() const = 0;

public Q_SLOTS:
    void setId(const QString& id) const;

    void setNeverDefault(bool neverDefault) const;

    void setActive(bool active) const;

    void updateSecrets() const;

Q_SIGNALS:
    void idChanged(const QString& id);

    void neverDefaultChanged(bool neverDefault);

    void activeChanged(bool active);

    void activatableChanged(bool active);

    void remove() const;

protected:
    class Priv;
    std::shared_ptr<Priv> d;
};

}

Q_DECLARE_METATYPE(connectivityqt::VpnConnection::Type)
