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

#include <menuitems/switch-item.h>

SwitchItem::SwitchItem(const std::string &label, const std::string &prefix, const std::string &name)
{
    setState(false);

    std::string action_name = prefix + "." + name;
    m_action = std::make_shared<Action>(action_name, nullptr, TypedVariant<bool>(m_state));
    m_actionGroup->add(m_action);
    connect(m_action.get(), &Action::activated, this, &SwitchItem::actionActivated);

    m_item = std::make_shared<MenuItem>(label,
                                        std::string("indicator.") + action_name);
    m_item->setAttribute("x-canonical-type", TypedVariant<std::string>("com.canonical.indicator.switch"));

    /// @todo state changes don't work properly
}

void
SwitchItem::actionActivated(const Variant&)
{
    bool value = m_action->state().as<bool>();
    ///@ todo something weird is happening as the indicator side is not changing the state..
    value = !value;
    Q_EMIT stateUpdated(m_state);
    Q_EMIT activated();
}

MenuItem::Ptr
SwitchItem::menuItem() {
    return m_item;
}

// state of the switch
bool
SwitchItem::state()
{
    return m_state;
}

void
SwitchItem::setState(bool state)
{
    if (m_state == state)
    {
        return;
    }

    m_state = state;
    Q_EMIT stateUpdated(m_state);

    if (m_action) {
        Variant variant = TypedVariant<bool>(m_state);
        m_action->setState(variant);
    }
}
