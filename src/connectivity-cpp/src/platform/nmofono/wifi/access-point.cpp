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

#include "access-point.h"
#include <glib.h>

namespace platform {
namespace nmofono {
namespace wifi {


AccessPoint::AccessPoint(const org::freedesktop::NetworkManager::Interface::AccessPoint &ap)
        : m_ap(ap)
{
    m_secured = m_ap.flags->get() == NM_802_11_AP_FLAGS_PRIVACY;
    /// @todo check for the other modes also..
    m_adhoc = m_ap.mode->get() != NM_802_11_MODE_INFRA;

    std::string ssid;
    const std::vector<std::int8_t> &raw_ssid = m_ap.ssid->get();
    if(g_utf8_validate((const char*)(&raw_ssid[0]), raw_ssid.size(), nullptr)) {
        m_ssid = std::string(raw_ssid.begin(), raw_ssid.end());
    } else {
        for (auto c : m_ap.ssid->get()) {
            if (isprint(c)) {
                ssid += (char)c;
            } else {
                // contains unprintable characters
                /// @todo do something more elegant
                ssid += u8"�";
            }
        }
    }

    m_strength.set(m_ap.strength->get());
    m_ap.properties_changed->connect([this](org::freedesktop::NetworkManager::Interface::AccessPoint::Signal::PropertiesChanged::ArgumentType map){
        for (const auto &entry : map) {
            if (entry.first == "Strength") {
                m_strength.set(entry.second.as<std::int8_t>());
            }
        }
    });
}

const core::dbus::types::ObjectPath AccessPoint::object_path() const {
    return m_ap.object->path();
}

const core::Property<double>& AccessPoint::strength() const
{
    return m_strength;
}

const core::Property<std::chrono::system_clock::time_point>& AccessPoint::lastConnected() const
{
    return m_lastConnected;
}

const std::string& AccessPoint::ssid() const
{
    return m_ssid;
}

bool AccessPoint::secured() const
{
    return m_secured;
}

bool AccessPoint::adhoc() const
{
    return m_adhoc;
}

bool AccessPoint::operator==(const platform::nmofono::wifi::AccessPoint &other) const {
    if(this == &other)
        return true;
    return m_ssid == other.m_ssid &&
            m_ap.flags->get() == other.m_ap.flags->get() &&
            /* NetworkManager seems to set the wpa and rns flags
             * for AccessPoints on the same network in a total random manner.
             * Sometimes only wpa_flags or rns_flags is set and sometimes
             * they both are set but always to the same value
             */
            (m_ap.wpa_flags->get()|m_ap.rsn_flags->get()) == (other.m_ap.wpa_flags->get()|other.m_ap.rsn_flags->get()) &&
            m_ap.mode->get() == other.m_ap.mode->get();
}

}
}
}
