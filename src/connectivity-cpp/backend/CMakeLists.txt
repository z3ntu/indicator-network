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

set_source_files_properties(
    "/usr/share/dbus-1/interfaces/org.freedesktop.URfkill.xml"
    PROPERTIES
    NO_NAMESPACE YES
)

qt5_add_dbus_interface(
    CONNECTIVITY_BACKEND_SRC
    "/usr/share/dbus-1/interfaces/org.freedesktop.URfkill.xml"
    services/URfkillInterface
)

set_source_files_properties(
    "/usr/share/dbus-1/interfaces/org.freedesktop.URfkill.Killswitch.xml"
    PROPERTIES
    NO_NAMESPACE YES
)

qt5_add_dbus_interface(
    CONNECTIVITY_BACKEND_SRC
    "/usr/share/dbus-1/interfaces/org.freedesktop.URfkill.Killswitch.xml"
    services/URfkillKillswitchInterface
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

