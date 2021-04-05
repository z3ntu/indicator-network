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

#include <nmofono/wifi/wifi-toggle.h>

#include <QDBusConnection>

namespace nmofono
{
namespace wifi
{

class NetworkManagerWifiToggle : public WifiToggle
{
public:
    LOMIRI_DEFINES_PTRS(NetworkManagerWifiToggle);

    NetworkManagerWifiToggle(const QDBusConnection& systemConnection);

    ~NetworkManagerWifiToggle();

    State state() const override;

    bool isEnabled() const override;

    void setEnabled(bool block) override;

protected:
    class Private;
    std::unique_ptr<Private> d;
};

}
}
