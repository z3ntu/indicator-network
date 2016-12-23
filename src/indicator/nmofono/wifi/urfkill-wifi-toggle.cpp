/*
 * Copyright © 2014-2016 Canonical Ltd.
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
 *     Marcus Tomlinson <marcus.tomlinson@canonical.com>
 *     Pete Woods <pete.woods@canonical.com>
 */

#include <NetworkManager.h>

#include <backend-utils.h>
#include <dbus-types.h>
#include <nmofono/wifi/urfkill-wifi-toggle.h>

#include <NetworkManagerInterface.h>
#include <URfkillInterface.h>
#include <URfkillKillswitchInterface.h>

using namespace std;

namespace nmofono
{
namespace wifi
{

class UrfkillWifiToggle::Private : public QObject
{
Q_OBJECT

public:
    enum class DeviceType
    {
        wlan = 1,
        bluetooth = 2,
        uwb = 3,
        wimax = 4,
        wwan = 5,
        gps = 6,
        fm = 7,
        nfc = 8
    };

    UrfkillWifiToggle& p;

    shared_ptr<OrgFreedesktopNetworkManagerInterface> m_networkManager;

    shared_ptr<OrgFreedesktopURfkillInterface> m_urfkill;

    shared_ptr<OrgFreedesktopURfkillKillswitchInterface> m_wifiUrfkillWifiToggle;

    State m_state = State::not_available;

    Private(UrfkillWifiToggle& parent) :
            p(parent)
    {
    }

public Q_SLOTS:
    void
    stateChanged()
    {
        int stateIndex = m_wifiUrfkillWifiToggle->state();
        if (stateIndex >= static_cast<int>(State::first_) && stateIndex <= static_cast<int>(State::last_))
        {
            m_state = static_cast<State>(stateIndex);
        }
        else
        {
            m_state = State::not_available;
        }

        Q_EMIT p.stateChanged(m_state);
        Q_EMIT p.enabledChanged(p.isEnabled());
    }
};

UrfkillWifiToggle::UrfkillWifiToggle(const QDBusConnection& systemConnection) :
        d(new Private(*this))
{
    d->m_networkManager = make_shared<OrgFreedesktopNetworkManagerInterface>(NM_DBUS_SERVICE, NM_DBUS_PATH, systemConnection);
    d->m_urfkill = make_shared<OrgFreedesktopURfkillInterface>(DBusTypes::URFKILL_BUS_NAME, DBusTypes::URFKILL_OBJ_PATH, systemConnection);
    d->m_wifiUrfkillWifiToggle = make_shared<OrgFreedesktopURfkillKillswitchInterface>(DBusTypes::URFKILL_BUS_NAME, DBusTypes::URFKILL_WIFI_OBJ_PATH, systemConnection);

    d->stateChanged();
    connect(d->m_wifiUrfkillWifiToggle.get(), &OrgFreedesktopURfkillKillswitchInterface::StateChanged, d.get(), &Private::stateChanged);
}

UrfkillWifiToggle::~UrfkillWifiToggle()
{
}

void
UrfkillWifiToggle::setEnabled(bool enabled)
{

    if (enabled && state() == State::hard_blocked)
    {
        qCritical() << "Killswitch is hard blocked.";
        return;
    }

    if (enabled && state() != State::soft_blocked)
    {
        return;
    }

    if (!enabled && state() != State::unblocked)
    {
        return;
    }

    try
    {
        if (!utils::getOrThrow(d->m_urfkill->Block(static_cast<uint>(Private::DeviceType::wlan), !enabled)))
        {
            throw std::runtime_error("Failed to block WiFi");
        }
    }
    catch (std::exception &e)
    {
        qCritical() << e.what();
    }

    d->m_networkManager->setWirelessEnabled(enabled);
}

bool
UrfkillWifiToggle::isEnabled() const
{
    return d->m_state == State::unblocked;
}

WifiToggle::State
UrfkillWifiToggle::state() const
{
    return d->m_state;
}

}
}

#include "urfkill-wifi-toggle.moc"
