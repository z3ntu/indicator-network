/*
 * Copyright © 2015 Canonical Ltd.
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

#include <QDBusConnection>
#include <QObject>
#include <memory>

class QPowerd
{
public:
    class QSysStateRequest;

    typedef std::shared_ptr<QPowerd> UPtr;
    typedef std::shared_ptr<QPowerd> SPtr;
    typedef std::shared_ptr<QSysStateRequest> RequestSPtr;

    enum class SysPowerState
    {
        suspend = 0,
        //The Active state will prevent system suspend
        active,
        //Substate of Active with disabled proximity based blanking
        active_blank_on_proximity,
        num_power_states
    };

    QPowerd(const QDBusConnection& connection);

    ~QPowerd();

    RequestSPtr requestSysState(const QString &name, SysPowerState state);

protected:
    class Priv;

    friend Priv;
    friend QSysStateRequest;

    std::shared_ptr<Priv> d;
};
