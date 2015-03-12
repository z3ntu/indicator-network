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
 *     Antti Kaijanmäki <antti.kaijanmaki@canonical.com>
 */

#include "kill-switch.h"

using namespace platform::nmofono;

class KillSwitch::Private
{
public:
    std::shared_ptr<OrgFreedesktopURfkillInterface> urfkill;
    std::shared_ptr<OrgFreedesktopURfkillKillswitchInterface> killSwitch;

    Private(std::shared_ptr<OrgFreedesktopURfkillInterface> urfkill,
            std::shared_ptr<OrgFreedesktopURfkillKillswitchInterface> killSwitch)
        : urfkill(urfkill),
          killSwitch(killSwitch) {}
};


KillSwitch::KillSwitch()
{
    auto urfkill = std::make_shared<OrgFreedesktopURfkillInterface>("org.freedesktop.URfkill",
                                                                    "/org/freedesktop/URfkill",
                                                                    QDBusConnection::systemBus());

    auto killSwitch = std::make_shared<OrgFreedesktopURfkillKillswitchInterface>("org.freedesktop.URfkill",
                                                                                 "/org/freedesktop/URfkill/WLAN",
                                                                                 QDBusConnection::systemBus());

    connect(urfkill.get(), SIGNAL(FlightModeChanged(bool)), this, SIGNAL(flightModeChanged(bool)));
    connect(killSwitch.get(), SIGNAL(StateChanged()), this, SIGNAL(stateChanged()));

    d.reset(new Private(urfkill, killSwitch));
}

KillSwitch::~KillSwitch()
{}

void
KillSwitch::block()
{
    if (state() == KillSwitch::State::hard_blocked)
        throw KillSwitch::exception::HardBlocked();

    try {
        if (!d->urfkill->Block(1, true)) ///!
            throw KillSwitch::exception::Failed("Failed to block killswitch");
    } catch (std::exception &e) {
        throw KillSwitch::exception::Failed(e.what());
    }
}

void
KillSwitch::unblock()
{
    try {
        if (!d->urfkill->Block(1, false)) ///!
            throw KillSwitch::exception::Failed("Failed to unblock killswitch");
    } catch (std::exception &e) {
        throw KillSwitch::exception::Failed(e.what());
    }
}

KillSwitch::State KillSwitch::state() const
{
    int stateIndex = d->killSwitch->state();
    if (stateIndex >= static_cast<int>(State::first_) &&
        stateIndex <= static_cast<int>(State::last_))
    {
        return static_cast<KillSwitch::State>(stateIndex);
    }
    return KillSwitch::State::not_available;
}

bool KillSwitch::flightMode(bool enable)
{
    return d->urfkill->FlightMode(enable);
}

bool KillSwitch::isFlightMode()
{
    return d->urfkill->IsFlightMode();
}
