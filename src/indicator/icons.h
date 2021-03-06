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

#include <nmofono/wifi/wifi-link.h>
#include <nmofono/wwan/modem.h>

class Icons
{
public:
    Icons() = delete;

    ~Icons() = delete;

    static QString strengthIcon(int8_t strength);

    static QString bearerIcon(nmofono::wwan::Modem::Bearer bearer);

    static QString wifiIcon(nmofono::wifi::WifiLink::Signal signal);
};
