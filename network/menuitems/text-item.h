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

#ifndef TEXT_ITEM_H
#define TEXT_ITEM_H

#include "item.h"

#include "menumodel-cpp/util.h"
#include "menumodel-cpp/menu-item.h"
#include "menumodel-cpp/action.h"

#include <core/signal.h>

class TextItem : public Item
{
public:
    typedef std::shared_ptr<TextItem> Ptr;

    TextItem() = delete;
    TextItem(const std::string &label, const std::string &prefix, const std::string &name)
    {
        std::string action_name = prefix + "." + name;

        m_action = std::make_shared<Action>(action_name, nullptr);
        m_actionGroup->add(m_action);
        m_action->activated().connect([this](Variant parameter){
            m_activated();
        });
        m_item = std::make_shared<MenuItem>(label, std::string("indicator.") + action_name);
    }

    void
    setLabel(const std::string &label)
    {
        m_item->setLabel(label);
    }

    virtual MenuItem::Ptr
    menuItem() {
        return m_item;
    }

    core::Signal<void> &
    activated()
    {
        return m_activated;
    }

private:
    core::Signal<void> m_activated;

    Action::Ptr m_action;
    MenuItem::Ptr m_item;
};

#endif // TEXT_ITEM_H
