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

#ifndef SWITCH_ITEM_H
#define SWITCH_ITEM_H

#include "item.h"
#include "menumodel-cpp/menu-item.h"

#include <core/property.h>

class SwitchItem : public Item
{
public:
    typedef std::shared_ptr<SwitchItem> Ptr;

    SwitchItem() = delete;
    SwitchItem(const std::string &label, const std::string &prefix, const std::string &name)
    {
        m_state.changed().connect([this](bool value){
            if (m_action) {
                Variant variant = TypedVariant<bool>(value);
                m_action->setState(variant);
            }
        });
        m_state.set(false);

        std::string action_name = prefix + "." + name;
        m_action = std::make_shared<Action>(action_name, nullptr, TypedVariant<bool>(m_state.get()));
        m_actionGroup->add(m_action);
        m_action->activated().connect([this](Variant){
            bool value = m_action->state().as<bool>();
            ///@ todo something weird is happening as the indicator side is not changing the state..
            value = !value;
            m_state.set(value);
            m_activated();
        });

        m_item = std::make_shared<MenuItem>(label,
                                            std::string("indicator.") + action_name);
        m_item->setAttribute("x-canonical-type", TypedVariant<std::string>("com.canonical.indicator.switch"));

        /// @todo state changes don't work properly
    }

    virtual MenuItem::Ptr
    menuItem() {
        return m_item;
    }

    // state of the switch
    core::Property<bool> &
    state()
    {
        return m_state;
    }

    // triggered when the user activates a state change
    core::Signal<void> &
    activated()
    {
        return m_activated;
    }

private:
    core::Property<bool> m_state;
    core::Signal<void> m_activated;

    Action::Ptr m_action;
    MenuItem::Ptr m_item;
};

#endif // SWITCH_ITEM_H
