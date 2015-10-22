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

#include <unity/util/DefinesPtrs.h>

namespace connectivityqt
{

class VpnConnection : public QObject
{
    Q_OBJECT

public:
    UNITY_DEFINES_PTRS(VpnConnection);

    VpnConnection(const QDBusObjectPath& path, const QDBusConnection& connection);

    ~VpnConnection();

    Q_PROPERTY(QDBusObjectPath path READ path)
    QDBusObjectPath path() const;

    Q_PROPERTY(QString id READ id WRITE setId NOTIFY idChanged)
    QString id() const;

    Q_PROPERTY(bool active READ active WRITE setActive NOTIFY activeChanged)
    bool active() const;

public Q_SLOTS:
    void setId(const QString& id);

    void setActive(bool active);

Q_SIGNALS:
    void idChanged(const QString& id);

    void activeChanged(bool active);

protected:
    class Priv;
    std::shared_ptr<Priv> d;
};

}
