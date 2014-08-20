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

#include "notification.h"

#include <libnotify/notify.h>

using namespace notify;

class notify::Notification::Private
{
public:
    core::Property<std::string> summary;
    core::Property<std::string> body;
    core::Property<std::string> icon;

    core::Signal<void> closed;

    std::unique_ptr<NotifyNotification, GObjectDeleter> notification;

    static void closed_cb(NotifyNotification *,
                          gpointer user_data)
    {
        Private *that = static_cast<Private *>(user_data);
        that->closed();
    }

    gulong disconnectId;
};

Notification::Notification(const std::string &summary,
                           const std::string &body,
                           const std::string &icon)
{
    d.reset(new Private);
    d->summary.set(summary);
    d->body.set(body);
    d->icon.set(icon);

    d->notification.reset(notify_notification_new(summary.c_str(), body.c_str(), icon.c_str()));
    d->disconnectId = g_signal_connect(d->notification.get(), "closed", G_CALLBACK(Private::closed_cb), d.get());

    d->summary.changed().connect([this](const std::string &value){
        g_object_set(d->notification.get(), "summary", value.c_str(), nullptr);
    });
    d->body.changed().connect([this](const std::string &value){
        g_object_set(d->notification.get(), "body", value.c_str(), nullptr);
    });
    d->icon.changed().connect([this](const std::string &value){
        g_object_set(d->notification.get(), "icon", value.c_str(), nullptr);
    });

}

Notification::~Notification()
{
    g_signal_handler_disconnect(d->notification.get(), d->disconnectId);
}

core::Property<std::string> &
Notification::summary()
{
    return d->summary;
}

core::Property<std::string> &
Notification::body()
{
    return d->body;
}

core::Property<std::string> &
Notification::icon()
{
    return d->icon;
}

void
Notification::setHint(const std::string &key, Variant value)
{
    notify_notification_set_hint(d->notification.get(), key.c_str(), value);
}

void
Notification::setHintString(const std::string &key, const std::string &value)
{
    notify_notification_set_hint_string(d->notification.get(), key.c_str(), value.c_str());
}

void
Notification::show()
{
    GError *error = nullptr;
    notify_notification_show(d->notification.get(), &error);
    if (error) {
        std::string message {error->message};
        g_error_free(error);
        throw std::runtime_error(message);
    }
}

void
Notification::close()
{
    GError *error = nullptr;
    notify_notification_close(d->notification.get(), &error);
    if (error) {
        std::string message {error->message};
        g_error_free(error);
        throw std::runtime_error(message);
    }
}


core::Signal<void> &
Notification::closed()
{
    return d->closed;
}
