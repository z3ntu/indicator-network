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

#include <exception>
#include <lomiri/util/DefinesPtrs.h>

#include <QDBusConnection>

namespace nmofono
{

class FlightModeToggle : public QObject
{
Q_OBJECT
public:
    LOMIRI_DEFINES_PTRS(FlightModeToggle);

    FlightModeToggle() = default;

    virtual ~FlightModeToggle() = default;

    virtual bool isFlightMode() const = 0;

    virtual bool isValid() const = 0;

public Q_SLOTS:
    virtual bool setFlightMode(bool) = 0;

Q_SIGNALS:
    void flightModeChanged(bool);
};

}

