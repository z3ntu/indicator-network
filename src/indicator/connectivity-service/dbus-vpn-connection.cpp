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

#include <connectivity-service/dbus-vpn-connection.h>
#include <VpnConnectionAdaptor.h>
#include <dbus-types.h>

using namespace std;
using namespace nmofono::vpn;

namespace connectivity_service
{

DBusVpnConnection::DBusVpnConnection(VpnConnection::SPtr vpnConnection,
                                     const QDBusConnection& connection) :
        m_vpnConnection(vpnConnection), m_connection(connection)
{
    m_path.setPath(DBusTypes::vpnConnectionPath());

    new VpnConnectionAdaptor(this);

    connect(m_vpnConnection.get(), &VpnConnection::activeChanged, this, &DBusVpnConnection::activeUpdated);
}

DBusVpnConnection::~DBusVpnConnection()
{
}

void DBusVpnConnection::registerDBusObject()
{
    if (!m_connection.registerObject(m_path.path(), this))
    {
        throw logic_error(
                "Unable to register VpnConnection object on DBus");
    }
}

void DBusVpnConnection::setActive(bool active)
{
    m_vpnConnection->setActive(active);
}

void DBusVpnConnection::setId(const QString& id)
{
    m_vpnConnection->setId(id);
}

void DBusVpnConnection::idUpdated(const QString& id)
{
    notifyProperties({"id"});
}

void DBusVpnConnection::activeUpdated(bool)
{
    notifyProperties({"active"});
}

void DBusVpnConnection::notifyProperties(const QStringList& propertyNames)
{
    DBusTypes::notifyPropertyChanged(
        m_connection,
        *this,
        m_path.path(),
        VpnConnectionAdaptor::staticMetaObject.classInfo(VpnConnectionAdaptor::staticMetaObject.indexOfClassInfo("D-Bus Interface")).value(),
        propertyNames
    );
}

QString DBusVpnConnection::id() const
{
    return m_vpnConnection->id();
}

bool DBusVpnConnection::active() const
{
    return m_vpnConnection->isActive();
}

QDBusObjectPath DBusVpnConnection::path() const
{
    return m_path;
}

int DBusVpnConnection::intType() const
{
    return static_cast<int>(type());
}

}
