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

#pragma once

#include <QObject>

#include <exception>
#include <unity/util/DefinesPtrs.h>

namespace nmofono
{
namespace wifi
{

class WifiToggle : public QObject
{
Q_OBJECT
public:
    UNITY_DEFINES_PTRS(WifiToggle);

    enum class State
    {
        not_available = -1,
        first_ = not_available,
        unblocked = 0,
        soft_blocked = 1,
        hard_blocked = 2,
        last_ = hard_blocked
    };

    WifiToggle() = default;

    virtual ~WifiToggle() = default;

    virtual State state() const = 0;

    virtual bool isEnabled() const = 0;

public Q_SLOTS:
    virtual void setEnabled(bool) = 0;

Q_SIGNALS:
    void stateChanged(State);

    void enabledChanged(bool);
};

}
}
