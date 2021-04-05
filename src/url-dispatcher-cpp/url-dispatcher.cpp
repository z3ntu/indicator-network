/*
 * Copyright © 2014 Canonical Ltd.
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

#include "url-dispatcher.h"

#include <liblomiri-url-dispatcher/lomiri-url-dispatcher.h>

namespace
{
void url_dispatcher_cb(const gchar * url, gboolean success, gpointer userdata)
{
    std::function<void(std::string, bool)> *cb = static_cast<std::function<void(std::string, bool)> *>(userdata);
    cb->operator()(url, success);
    delete cb;
}
}

void
UrlDispatcher::send(std::string url, std::function<void(std::string, bool)> cb)
{
    if (cb) {
        lomiri_url_dispatch_send(url.c_str(),
                                 url_dispatcher_cb,
                                 new std::function<void(std::string, bool)>{cb});
    } else {
        lomiri_url_dispatch_send(url.c_str(), url_dispatcher_cb, nullptr);
    }
}
