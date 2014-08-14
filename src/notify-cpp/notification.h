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

#ifndef NOTIFY_NOTIFICATION_H
#define NOTIFY_NOTIFICATION_H

#include <memory>

#include <core/property.h>

#include "menumodel-cpp/gio-helpers/variant.h"

namespace notify {

class Notification
{
    class Private;
    std::unique_ptr<Private> d;

public:

    typedef std::shared_ptr<Notification> Ptr;

    Notification() = delete;
    Notification(const std::string &summary,
                 const std::string &body,
                 const std::string &icon);
    virtual ~Notification();

    /// @todo remember show() after set().
    core::Property<std::string> &summary();

    /// @todo remember show() after set().
    core::Property<std::string> &body();

    /// @todo remember show() after set().
    core::Property<std::string> &icon();

    void setHint(const std::string &key, Variant value);
    void setHintString(const std::string &key, const std::string &value);

    void show();
    void close();

    core::Signal<void> &closed();
};
}

#endif // NOTIFY_NOTIFICATION_H
