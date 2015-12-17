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
 * Authors:
 *     Pete Woods <pete.woods@canonical.com>
 */

#pragma once

#include <nmofono/connection/active-connection-manager.h>
#include <notify-cpp/notification-manager.h>

#include <unity/util/DefinesPtrs.h>

class VpnStatusNotifier
{
public:
    UNITY_DEFINES_PTRS(VpnStatusNotifier);

    VpnStatusNotifier(nmofono::connection::ActiveConnectionManager::SPtr activeConnectionManager,
            notify::NotificationManager::SPtr notificationManager);

    ~VpnStatusNotifier();

protected:
    class Priv;
    std::shared_ptr<Priv> d;
};
