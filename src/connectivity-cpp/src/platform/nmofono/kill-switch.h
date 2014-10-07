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

#ifndef PLATFORM_NMOFONO_KILLSWITCH
#define PLATFORM_NMOFONO_KILLSWITCH

#include <exception>
#include <memory>

#include <core/property.h>

#include <services/urfkill.h>

namespace platform {
namespace nmofono {
    class KillSwitch;
}
}

class platform::nmofono::KillSwitch
{
    struct Private;
    std::unique_ptr<Private> d;

public:
    typedef std::shared_ptr<KillSwitch> Ptr;

    struct exception
    {
        struct HardBlocked : public std::runtime_error
        {
            HardBlocked()
                : std::runtime_error("Killswitch is hard blocked.")
            {}
        };

        struct Failed : public std::runtime_error
        {
            Failed() = delete;
            Failed(std::string what)
                : std::runtime_error(what)
            {}
        };
    };

    enum class State
    {
        not_available,
        unblocked,
        soft_blocked,
        hard_blocked
    };

    KillSwitch() = delete;
    KillSwitch(std::shared_ptr<org::freedesktop::URfkill::Interface::URfkill> urfkill,
               std::shared_ptr<org::freedesktop::URfkill::Interface::Killswitch> killSwitch);
    ~KillSwitch();

    /// @throws exception::Failed if the switch fails to block
    void block();

    /// @throws exception::HardBlocked if trying to unblock when switch is hard blocked
    /// @throws exception::Failed if the switch fails to unblock
    void unblock();

    const core::Property<State> &state() const;
};

#endif
