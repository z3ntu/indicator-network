# Copyright © 2013 Canonical Ltd.
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
#     Antti Kaijanmäki <antti.kaijanmaki@canonical.com>

add_library(connectivity-cpp STATIC
    manager.cpp
)
set_target_properties(connectivity-cpp PROPERTIES
  VERSION ${CONNECTIVITY_CPP_VERSION_MAJOR}.${CONNECTIVITY_CPP_VERSION_MINOR}.${CONNECTIVITY_CPP_VERSION_PATCH}
  SOVERSION ${CONNECTIVITY_CPP_VERSION_MAJOR}
  OUTPUT_NAME "connectivity-cpp"
)
target_link_libraries(connectivity-cpp platform_nmofono)

add_subdirectory(platform/nmofono)


