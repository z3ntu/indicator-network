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

#pragma once

#include <nmofono/vpn/vpn-connection.h>

#include <QDBusConnection>
#include <QDBusContext>
#include <QDBusObjectPath>
#include <QObject>
#include <QString>

#include <lomiri/util/DefinesPtrs.h>

class VpnConnectionAdaptor;

namespace connectivity_service
{

class DBusVpnConnection: public QObject, protected QDBusContext
{
    Q_OBJECT

    friend VpnConnectionAdaptor;

public:
    LOMIRI_DEFINES_PTRS(DBusVpnConnection);

    DBusVpnConnection(nmofono::vpn::VpnConnection::SPtr vpnConnection, const QDBusConnection& connection);

    virtual ~DBusVpnConnection();

    Q_PROPERTY(QString id READ id WRITE setId)
    QString id() const;

    Q_PROPERTY(bool neverDefault READ neverDefault WRITE setNeverDefault)
    bool neverDefault() const;

    virtual nmofono::vpn::VpnConnection::Type type() const = 0;

    Q_PROPERTY(bool active READ active WRITE setActive)
    bool active() const;

    Q_PROPERTY(bool activatable READ activatable)
    bool activatable() const;

    QDBusObjectPath path() const;

    void remove();

Q_SIGNALS:
    void setActive(bool active);

    void setId(const QString& id);

    void setNeverDefault(bool neverDefault);

    void UpdateSecrets();

protected Q_SLOTS:
    void activeUpdated(bool active);

    void activatableUpdated(bool activatable);

    void idUpdated(const QString& id);

    void neverDefaultUpdated(bool neverDefault);

private:
    void notifyProperties(const QStringList& propertyNames);

protected:
    void registerDBusObject();

    Q_PROPERTY(int type READ intType)
    int intType() const;

    nmofono::vpn::VpnConnection::SPtr m_vpnConnection;

    QDBusConnection m_connection;

    QDBusObjectPath m_path;
};

}
