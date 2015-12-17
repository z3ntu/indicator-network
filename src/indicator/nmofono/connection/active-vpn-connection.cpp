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

#include <nmofono/connection/active-connection.h>
#include <nmofono/connection/active-vpn-connection.h>
#include <NetworkManagerVpnConnectionInterface.h>

using namespace std;

namespace nmofono
{
namespace connection
{

class ActiveVpnConnection::Priv: public QObject
{
    Q_OBJECT

public Q_SLOTS:
    void vpnStateChanged(uint state, uint reason)
    {
        m_state = static_cast<State>(state);
        Q_EMIT p.stateChanged(m_state, static_cast<Reason>(reason));
    }

public:
    Priv(ActiveVpnConnection& parent, ActiveConnection& activeConnection) :
        p(parent), m_activeConnection(activeConnection)
    {
    }

    ActiveVpnConnection& p;

    ActiveConnection& m_activeConnection;

    shared_ptr<OrgFreedesktopNetworkManagerVPNConnectionInterface> m_interface;

    State m_state = State::UNKNOWN;
};

ActiveVpnConnection::ActiveVpnConnection(const QDBusObjectPath& path, const QDBusConnection& connection, ActiveConnection& activeConnection) :
        d(new Priv(*this, activeConnection))
{
    d->m_interface = make_shared<OrgFreedesktopNetworkManagerVPNConnectionInterface>(NM_DBUS_SERVICE, path.path(), connection);
    connect(d->m_interface.get(), &OrgFreedesktopNetworkManagerVPNConnectionInterface::VpnStateChanged, d.get(), &Priv::vpnStateChanged);

    d->vpnStateChanged(d->m_interface->vpnState(), static_cast<int>(Reason::UNKNOWN));
}

ActiveVpnConnection::~ActiveVpnConnection()
{
}

ActiveVpnConnection::State ActiveVpnConnection::vpnState() const
{
    return d->m_state;
}

ActiveConnection& ActiveVpnConnection::activeConnection() const
{
    return d->m_activeConnection;
}

}
}

#include "active-vpn-connection.moc"
