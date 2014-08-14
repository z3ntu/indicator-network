/*
 * Copyright (C) 2014 Canonical, Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3, as published
 * by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranties of
 * MERCHANTABILITY, SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors:
 *     Antti Kaijanmäki <antti.kaijanmaki@canonical.com>
 */

#ifndef ACTION_H
#define ACTION_H

#include <memory>
#include <functional>
#include <mutex>

#include <core/signal.h>
#include <gio/gio.h>

#include "gio-helpers/util.h"
#include "gio-helpers/variant.h"


class Action
{
    typedef std::shared_ptr<GAction> GActionPtr;
    GActionPtr make_gaction_ptr(GSimpleAction *action) { return std::shared_ptr<GAction>(G_ACTION(action), GObjectDeleter()); }

    GActionPtr m_gaction;
    std::string m_name;
    Variant m_state;
    core::Signal<Variant> m_activated;
    gulong m_activateHandlerId;
    gulong m_changeStateHandlerId;
    std::function<void(Variant)> m_changeStateHandler;
    std::recursive_mutex m_mutex;

    static void activate_cb(GSimpleAction *,
                            GVariant      *parameter,
                            gpointer       user_data);

    static void change_state_cb(GSimpleAction *,
                                GVariant      *value,
                                gpointer       user_data);
public:
    typedef std::shared_ptr<Action> Ptr;

    Action(const std::string &name, const
           GVariantType *parameterType = nullptr,
           const Variant &state = Variant(),
           std::function<void(Variant)> changeStateHandler = std::function<void(Variant)>());

    ~Action();

    std::string name();

    void setState(const Variant &value);

    Variant state();

    GActionPtr gaction();

    core::Signal<Variant> &activated();
};

#endif // ACTION_H
