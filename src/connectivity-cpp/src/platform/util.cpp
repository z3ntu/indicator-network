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
 *     Jussi Pakkanen <jussi.pakkanen@canonical.com>
 */

#include<services/util.h>
#include<core/dbus/error.h>
#include<stdexcept>

namespace connectivity {

void throw_dbus_exception(const core::dbus::Error &e) {
    // At this point we would examine what kind of error we got
    // and would throw a corresponding exception. Currently just a
    // placeholder.
    throw std::runtime_error(e.message());
}

}
