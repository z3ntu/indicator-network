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
 *     Marcus Tomlinson <marcus.tomlinson@canonical.com>
 */

#pragma once

#include <exception>
#include <memory>

#include <QDBusConnection>

namespace nmofono {

class KillSwitch : public QObject
{
    Q_OBJECT

    class Private;
    std::unique_ptr<Private> d;

public:
    typedef std::shared_ptr<KillSwitch> Ptr;

    enum class State
    {
        not_available = -1,
        first_ = not_available,
        unblocked = 0,
        soft_blocked = 1,
        hard_blocked = 2,
        last_ = hard_blocked
    };

    KillSwitch(const QDBusConnection& systemBus);
    ~KillSwitch();

    void setBlock(bool block);

    State state() const;
    bool flightMode(bool enable);
    bool isFlightMode();

Q_SIGNALS:
    void stateChanged(State);
    void flightModeChanged(bool);

};

}
