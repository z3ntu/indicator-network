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
namespace URfkill = org::freedesktop::URfkill;

struct KillSwitch::Private
{
    std::shared_ptr<URfkill::Interface::URfkill> urfkill;
    std::shared_ptr<URfkill::Interface::Killswitch> killSwitch;
    core::Property<KillSwitch::State> state;

    Private(std::shared_ptr<URfkill::Interface::URfkill> urfkill,
            std::shared_ptr<URfkill::Interface::Killswitch> killSwitch)
        : urfkill(urfkill),
          killSwitch(killSwitch)
    {}

    void stateChanged(URfkill::Interface::Killswitch::State value)
    {
        switch(value) {
        case URfkill::Interface::Killswitch::State::not_available:
        case URfkill::Interface::Killswitch::State::unblocked:
            state.set(KillSwitch::State::unblocked);
            break;
        case URfkill::Interface::Killswitch::State::hard_blocked:
            state.set(KillSwitch::State::hard_blocked);
            break;
        case URfkill::Interface::Killswitch::State::soft_blocked:
            state.set(KillSwitch::State::soft_blocked);
            break;
        }
    }
};


KillSwitch::KillSwitch(std::shared_ptr<URfkill::Interface::URfkill> urfkill,
                       std::shared_ptr<URfkill::Interface::Killswitch> killSwitch)
{
    d.reset(new Private(urfkill, killSwitch));
    killSwitch->state.changed().connect(std::bind(&Private::stateChanged, d.get(), std::placeholders::_1));
    d->stateChanged(killSwitch->state.get());
}

KillSwitch::~KillSwitch()
{}

void
KillSwitch::block()
{
    if (d->state.get() == KillSwitch::State::hard_blocked)
        throw KillSwitch::exception::HardBlocked();

    try {
        if (!d->urfkill->block(d->killSwitch->type, true))
            throw KillSwitch::exception::Failed("");
    } catch (std::exception &e) {
        throw KillSwitch::exception::Failed(e.what());
    }
}

void
KillSwitch::unblock()
{
    try {
        if (!d->urfkill->block(d->killSwitch->type, false))
            throw KillSwitch::exception::Failed("");
    } catch (std::exception &e) {
        throw KillSwitch::exception::Failed(e.what());
    }
}

const core::Property<KillSwitch::State> &
KillSwitch::state() const
{
    return d->state;
}

