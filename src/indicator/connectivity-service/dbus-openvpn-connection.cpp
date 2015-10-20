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

#include <connectivity-service/dbus-openvpn-connection.h>
#include <OpenVpnAdaptor.h>

using namespace std;
using namespace nmofono::vpn;

namespace connectivity_service
{

DBusOpenvpnConnection::DBusOpenvpnConnection(VpnConnection::SPtr vpnConnection,
                                             const QDBusConnection& connection) :
        DBusVpnConnection(vpnConnection, connection)
{
    new OpenVpnAdaptor(this);
    registerDBusObject();
}

DBusOpenvpnConnection::~DBusOpenvpnConnection()
{
}

nmofono::vpn::VpnConnection::Type DBusOpenvpnConnection::type() const
{
    return nmofono::vpn::VpnConnection::Type::openvpn;
}

}
