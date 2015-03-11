# Copyright © 2015 Canonical Ltd.
#
# This program is free software: you can redistribute it and/or modify it
# under the terms of the GNU Lesser General Public License version 3,
# as published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
# Authors:
#     Marcus Tomlinson <marcus.tomlinson@canonical.com>

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set_source_files_properties(
    "${DATA_DIR}/nm-device.xml"
    "${DATA_DIR}/nm-device-wifi.xml"
    "${DATA_DIR}/nm-access-point.xml"
    "/usr/share/dbus-1/interfaces/org.freedesktop.URfkill.xml"
    "/usr/share/dbus-1/interfaces/org.freedesktop.URfkill.Device.xml"
    "/usr/share/dbus-1/interfaces/org.freedesktop.URfkill.Killswitch.xml"
    PROPERTIES
    NO_NAMESPACE YES
)

set_source_files_properties(
    "${DATA_DIR}/nm-manager.xml"
    PROPERTIES
    NO_NAMESPACE YES
    INCLUDE "DBusTypes.h"
)

qt5_add_dbus_interface(
    CONNECTIVITY_BACKEND_SRC
    "/usr/share/dbus-1/interfaces/org.freedesktop.URfkill.xml"
    services/URfkillInterface
)

qt5_add_dbus_interface(
    CONNECTIVITY_BACKEND_SRC
    "/usr/share/dbus-1/interfaces/org.freedesktop.URfkill.Device.xml"
    services/URfkillDeviceInterface
)

qt5_add_dbus_interface(
    CONNECTIVITY_BACKEND_SRC
    "/usr/share/dbus-1/interfaces/org.freedesktop.URfkill.Killswitch.xml"
    URfkillKillswitchInterface
)

qt5_add_dbus_interface(
    CONNECTIVITY_BACKEND_SRC
    "${DATA_DIR}/nm-manager.xml"
    NetworkManagerInterface
)

qt5_add_dbus_interface(
    CONNECTIVITY_BACKEND_SRC
    "${DATA_DIR}/nm-device.xml"
    NetworkManagerDeviceInterface
)

qt5_add_dbus_interface(
    CONNECTIVITY_BACKEND_SRC
    "${DATA_DIR}/nm-device-wifi.xml"
    NetworkManagerDeviceWirelessInterface
)

qt5_add_dbus_interface(
    CONNECTIVITY_BACKEND_SRC
    "${DATA_DIR}/nm-access-point.xml"
    NetworkManagerAccessPointInterface
)

add_library(
    connectivity-backend
    STATIC
    ${CONNECTIVITY_BACKEND_SRC}
)

qt5_use_modules(
    connectivity-backend
    DBus
)