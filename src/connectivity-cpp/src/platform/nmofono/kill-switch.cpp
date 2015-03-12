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

class KillSwitch::Private : public QObject
{
    Q_OBJECT
public:
    std::shared_ptr<OrgFreedesktopURfkillInterface> urfkill;
    std::shared_ptr<OrgFreedesktopURfkillKillswitchInterface> killSwitch;

    core::Property<KillSwitch::State> state;

    Private(std::shared_ptr<OrgFreedesktopURfkillInterface> urfkill,
            std::shared_ptr<OrgFreedesktopURfkillKillswitchInterface> killSwitch)
        : urfkill(urfkill),
          killSwitch(killSwitch)
    {
        connect(killSwitch.get(), SIGNAL(StateChanged()), this, SLOT(stateChanged()));
        stateChanged(killSwitch->state());
    }

    void stateChanged(int stateIndex)
    {
        if (stateIndex >= static_cast<int>(State::first_) &&
            stateIndex <= static_cast<int>(State::last_))
        {
            state.set(static_cast<State>(stateIndex));
        }
    }

public Q_SLOTS:
    void stateChanged()
    {
        stateChanged(killSwitch->state());
    }
};


KillSwitch::KillSwitch()
{
    auto urfkill = std::make_shared<OrgFreedesktopURfkillInterface>("org.freedesktop.URfkill",
                                                                  "/org/freedesktop/URfkill",
                                                                  QDBusConnection::systemBus());

    auto urfkillKillswitch = std::make_shared<OrgFreedesktopURfkillKillswitchInterface>("org.freedesktop.URfkill",
                                                                                         "/org/freedesktop/URfkill/WLAN",
                                                                                         QDBusConnection::systemBus());

    connect(urfkill.get(), SIGNAL(FlightModeChanged(bool)), this, SIGNAL(flightModeChanged(bool)));

    d.reset(new Private(urfkill, urfkillKillswitch));
}

KillSwitch::~KillSwitch()
{}

void
KillSwitch::block()
{
    if (d->state.get() == KillSwitch::State::hard_blocked)
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

bool KillSwitch::flightMode(bool enable)
{
    return d->urfkill->FlightMode(enable);
}

bool KillSwitch::isFlightMode()
{
    return d->urfkill->IsFlightMode();
}

const core::Property<KillSwitch::State> &
KillSwitch::state() const
{
    return d->state;
}

#include "kill-switch.moc"
