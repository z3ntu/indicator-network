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

#ifndef MENU_ITEM_H
#define MENU_ITEM_H

#include <memory>
#include <map>
#include <mutex>

#include <core/signal.h>

#include <gio/gio.h>

#include "gio-helpers/variant.h"
#include "gio-helpers/util.h"

#include "menu-model.h"

class MenuItem
{
    std::recursive_mutex m_mutex;
    GMenuItemPtr m_gmenuitem;

    std::string m_label;
    std::string m_action;

    std::map<std::string, Variant> m_attributes;

    core::Signal<> m_changed;

public:
    typedef std::shared_ptr<MenuItem> Ptr;

    static MenuItem::Ptr newSubmenu(MenuModel::Ptr submenu,
                                    const std::string &label = "")
    {
        auto menu = std::make_shared<MenuItem>(label);
        g_menu_item_set_submenu(menu->m_gmenuitem.get(), *submenu);
        return menu;
    }

    static MenuItem::Ptr newSection(MenuModel::Ptr submenu,
                                    const std::string &label = "")
    {
        auto menu = std::make_shared<MenuItem>(label);
        g_menu_item_set_section(menu->m_gmenuitem.get(), *submenu);
        return menu;
    }

    explicit MenuItem(const std::string &label  = "",
             const std::string &action = "")
        : m_label{label},
          m_action{action}
    {
        /// @todo validate that action is valid
        m_gmenuitem = make_gmenuitem_ptr(g_menu_item_new(nullptr, nullptr));
        if (!label.empty()) {
            g_menu_item_set_label(m_gmenuitem.get(), label.c_str());
        }
        if (!action.empty()) {
            g_menu_item_set_detailed_action(m_gmenuitem.get(), action.c_str());
        }
    }

    ~MenuItem()
    {
        std::lock_guard<std::recursive_mutex> lg(m_mutex);
    }

    std::string label()
    {
        std::lock_guard<std::recursive_mutex> lg(m_mutex);
        return m_label;
    }

    void setLabel(const std::string &value)
    {
        std::lock_guard<std::recursive_mutex> lg(m_mutex);
        if (m_label == value)
            return;
        m_label = value;
        g_menu_item_set_label(m_gmenuitem.get(), m_label.c_str());
        m_changed();
    }

    void setAction(const std::string &value)
    {
        /// @todo validate that action is valid
        std::lock_guard<std::recursive_mutex> lg(m_mutex);
        if (m_action == value)
            return;
        m_action = value;
        g_menu_item_set_detailed_action(m_gmenuitem.get(), m_action.c_str());
        m_changed();
    }

    void setAttribute(const std::string &attribute,
                      Variant value)
    {
        /// @todo validate that attribute is valid.
        assert(value);
        assert(!attribute.empty());
        std::lock_guard<std::recursive_mutex> lg(m_mutex);

        auto iter = m_attributes.find(attribute);
        if (iter != m_attributes.end()) {
            if (iter->second == value)
                return;
            iter->second = value;
        } else {
            m_attributes[attribute] = value;
        }
        g_menu_item_set_attribute_value(m_gmenuitem.get(), attribute.c_str(), value);
        m_changed();
    }

    void clearAttribute(const std::string &attribute)
    {
        assert(!attribute.empty());
        std::lock_guard<std::recursive_mutex> lg(m_mutex);
        m_attributes.erase(attribute);
        g_menu_item_set_attribute(m_gmenuitem.get(), attribute.c_str(), nullptr);
        m_changed();
    }

    GMenuItem *gmenuitem()
    {
        std::lock_guard<std::recursive_mutex> lg(m_mutex);
        return m_gmenuitem.get();
    }

    core::Signal<> &changed()
    {
        std::lock_guard<std::recursive_mutex> lg(m_mutex);
        return m_changed;
    }

    const std::string &
    action () const
    {
        return m_action;
    }
};

#endif // MENU_ITEM_H
