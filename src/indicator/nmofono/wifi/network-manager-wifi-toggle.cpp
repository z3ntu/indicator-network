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
 *     Pete Woods <pete.woods@canonical.com>
 */

#include <NetworkManager.h>

#include <dbus-types.h>
#include <nmofono/wifi/network-manager-wifi-toggle.h>

#include <NetworkManagerInterface.h>

using namespace std;

namespace nmofono
{
namespace wifi
{

class NetworkManagerWifiToggle::Private : public QObject
{
Q_OBJECT

public:
    NetworkManagerWifiToggle& p;

    shared_ptr<OrgFreedesktopNetworkManagerInterface> m_networkManager;

    bool m_enabled = false;

    Private(NetworkManagerWifiToggle& parent) :
            p(parent)
    {
    }

public Q_SLOTS:
    void
    networkManagerPropertiesChanged(const QVariantMap &properties)
    {
        auto it = properties.constFind("WirelessEnabled");
        if (it != properties.constEnd())
        {
            m_enabled = it->toBool();
        }

        Q_EMIT p.stateChanged(p.state());
        Q_EMIT p.enabledChanged(m_enabled);
    }
};

NetworkManagerWifiToggle::NetworkManagerWifiToggle(const QDBusConnection& systemConnection) :
        d(new Private(*this))
{
    d->m_networkManager = make_shared<OrgFreedesktopNetworkManagerInterface>(NM_DBUS_SERVICE, NM_DBUS_PATH, systemConnection);
    d->networkManagerPropertiesChanged({{"WirelessEnabled", d->m_networkManager->wirelessEnabled()}});
    connect(d->m_networkManager.get(), &OrgFreedesktopNetworkManagerInterface::PropertiesChanged, d.get(), &Private::networkManagerPropertiesChanged);
}

NetworkManagerWifiToggle::~NetworkManagerWifiToggle()
{
}

void
NetworkManagerWifiToggle::setEnabled(bool enabled)
{
    d->m_networkManager->setWirelessEnabled(enabled);
}

bool
NetworkManagerWifiToggle::isEnabled() const
{
    return d->m_enabled;
}

WifiToggle::State
NetworkManagerWifiToggle::state() const
{
    return d->m_enabled ? State::unblocked : State::soft_blocked;
}

}
}

#include "network-manager-wifi-toggle.moc"
