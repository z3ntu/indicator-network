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

#include "util.h"

#include "variant.h"


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
                            gpointer       user_data)
    {
        Variant param;
        if (parameter != nullptr) {
            param = Variant::fromGVariant(g_variant_ref(parameter));
        }
        Action *that = static_cast<Action *>(user_data);
        that->m_activated(param);
    }

    static void change_state_cb(GSimpleAction *,
                                GVariant      *value,
                                gpointer       user_data)
    {
        Variant new_value = Variant::fromGVariant(g_variant_ref(value));

        Action *that =  static_cast<Action *>(user_data);
        if (that->m_changeStateHandler)
            that->m_changeStateHandler(new_value);
    }

public:
    typedef std::shared_ptr<Action> Ptr;

    Action(const std::string &name, const
           GVariantType *parameterType = nullptr,
           const Variant &state = Variant(),
           std::function<void(Variant)> changeStateHandler = std::function<void(Variant)>())
        : m_name {name},
          m_state {state},
          m_changeStateHandler {changeStateHandler}
    {
        /// @todo validate that name is valid.

        if (state) {
            m_gaction = make_gaction_ptr(g_simple_action_new_stateful(name.c_str(),
                                                                      parameterType,
                                                                      state));
        } else {
            m_gaction = make_gaction_ptr(g_simple_action_new(name.c_str(),
                                                             parameterType));
        }

        m_activateHandlerId = g_signal_connect(m_gaction.get(),
                                               "activate",
                                               G_CALLBACK(Action::activate_cb),
                                               this);
        m_changeStateHandlerId = 0;
        if (m_changeStateHandler) {
            m_changeStateHandlerId = g_signal_connect(m_gaction.get(),
                                                      "change-state",
                                                      G_CALLBACK(Action::change_state_cb),
                                                      this);
        }
    }
    ~Action()
    {
        std::lock_guard<std::recursive_mutex> lg(m_mutex);
        GMainLoopDispatch([=](){
            g_signal_handler_disconnect(m_gaction.get(), m_activateHandlerId);
            if (m_changeStateHandlerId)
                g_signal_handler_disconnect(m_gaction.get(), m_changeStateHandlerId);
        });
    }

    std::string
    name()
    {
        std::lock_guard<std::recursive_mutex> lg(m_mutex);
        return m_name;
    }

    void setState(const Variant &value)
    {
        std::lock_guard<std::recursive_mutex> lg(m_mutex);
        m_state = value;
        GMainLoopDispatch([=](){
          g_simple_action_set_state(G_SIMPLE_ACTION(m_gaction.get()), value);
        });
    }

    Variant state()
    {
        std::lock_guard<std::recursive_mutex> lg(m_mutex);
        return m_state;
    }

    GActionPtr
    gaction()
    {
        std::lock_guard<std::recursive_mutex> lg(m_mutex);
        return m_gaction;
    }

    core::Signal<Variant> &activated()
    {
        std::lock_guard<std::recursive_mutex> lg(m_mutex);
        return m_activated;
    }
};

#endif // ACTION_H
