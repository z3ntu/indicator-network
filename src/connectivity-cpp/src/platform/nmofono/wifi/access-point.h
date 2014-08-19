/*
 * Copyright © 2013 Canonical Ltd.
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

#ifndef PLATFORM_NMOFONO_WIFI_ACCESS_POINT
#define PLATFORM_NMOFONO_WIFI_ACCESS_POINT

#include <connectivity/networking/wifi/access-point.h>

#include <services/nm.h>

#include <chrono>

namespace core {
namespace dbus {
namespace types {
class ObjectPath;
}
}
}

namespace platform {
namespace nmofono {
namespace wifi {

class AccessPoint : public connectivity::networking::wifi::AccessPoint
{

public:
    typedef std::shared_ptr<AccessPoint> Ptr;

    AccessPoint(const org::freedesktop::NetworkManager::Interface::AccessPoint &ap);
    const core::Property<double>& strength() const;
    virtual ~AccessPoint() = default;

    // time when last connected to this access point
    // for APs that have never been connected the
    // lastConnected->time_since_epoch().count() is 0
    const core::Property<std::chrono::system_clock::time_point>& lastConnected() const;

    const std::string& ssid() const override;

    bool secured() const override;

    bool adhoc() const override;

    const core::dbus::types::ObjectPath object_path() const;

    const org::freedesktop::NetworkManager::Interface::AccessPoint& get_ap() const { return m_ap; }

    bool operator==(const platform::nmofono::wifi::AccessPoint &other) const;
    bool operator!=(const platform::nmofono::wifi::AccessPoint &other) const { return !(*this == other); };

private:
    core::Property<double> m_strength;
    core::Property<std::chrono::system_clock::time_point> m_lastConnected;
    org::freedesktop::NetworkManager::Interface::AccessPoint m_ap;
    std::string m_ssid;
    bool m_secured;
    bool m_adhoc;
};

}
}
}

#endif
