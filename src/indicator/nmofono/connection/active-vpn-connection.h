/*
 * Copyright (C) 2015 Canonical, Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3, as published
 * by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranties of
 * MERCHANTABILITY, SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Author: Pete Woods <pete.woods@canonical.com>
 */

#pragma once

#include <QDBusConnection>
#include <QDBusObjectPath>
#include <QObject>

#include <NetworkManager.h>

#include <unity/util/DefinesPtrs.h>

namespace nmofono
{
namespace connection
{

class ActiveVpnConnection: public QObject
{
    Q_OBJECT

public:
    UNITY_DEFINES_PTRS(ActiveVpnConnection);

    enum class State
    {
        UNKNOWN = 0,
        PREPARE,
        NEED_AUTH,
        CONNECT,
        IP_CONFIG_GET,
        ACTIVATED,
        FAILED,
        DISCONNECTED
    };

    enum class Reason
    {
        UNKNOWN = 0,
        NONE,
        DISCONNECTED,
        DEVICE_DISCONNECTED,
        SERVICE_STOPPED,
        IP_CONFIG_INVALID,
        CONNECT_TIMEOUT,
        SERVICE_START_TIMEOUT,
        SERVICE_START_FAILED,
        NO_SECRETS,
        LOGIN_FAILED,
        CONNECTION_REMOVED
    };

    ActiveVpnConnection(const QDBusObjectPath& path, const QDBusConnection& connection);

    ~ActiveVpnConnection();

    State vpnState() const;

Q_SIGNALS:
    void stateChanged(State state, Reason reason);

protected:
    class Priv;
    std::shared_ptr<Priv> d;
};

}
}
