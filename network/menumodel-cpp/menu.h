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

#ifndef MENU_H
#define MENU_H

#include <memory>
#include <list>
#include <vector>
#include <map>
#include <algorithm>
#include <mutex>

#include <gio/gio.h>

#include "util.h"
#include "menu-model.h"
#include "menu-item.h"

class Menu : public MenuModel
{    
    GMenuPtr m_gmenu;
    std::list<MenuItem::Ptr> m_items;
    std::recursive_mutex m_mutex;

public:
    typedef std::shared_ptr<Menu> Ptr;
    typedef std::list<MenuItem::Ptr>::iterator iterator;

    Menu()
    {
        m_gmenu = make_gmenu_ptr();
    }

    ~Menu()
    {
        std::lock_guard<std::recursive_mutex> lg(m_mutex);
    }

    void append(MenuItem::Ptr item)
    {
        std::lock_guard<std::recursive_mutex> lg(m_mutex);
        m_items.push_back(item);
        GMainLoopDispatch([=](){
            g_menu_append_item(m_gmenu.get(), item->gmenuitem());
        });
        if (std::count(m_items.begin(), m_items.end(), item) == 1) {
            /// @todo disconenct
            item->changed().connect([this, item](){
                int index = 0;
                for (auto iter = m_items.begin(); iter != m_items.end(); ++iter) {
                    if (*iter == item) {
                        GMainLoopDispatch([=](){
                            g_menu_remove(m_gmenu.get(), index);
                            g_menu_insert_item(m_gmenu.get(), index, item->gmenuitem());
                        });
                    }
                    ++index;
                }
            });
        }
    }

    void insert(MenuItem::Ptr item, iterator position)
    {
        std::lock_guard<std::recursive_mutex> lg(m_mutex);
        int index = 0;
        auto iter = m_items.begin();
        while (iter != m_items.end()) {
            if (iter == position)
                break;
            ++index;
            ++iter;
        }
        GMainLoopDispatch([=](){
            g_menu_insert_item(m_gmenu.get(), index, item->gmenuitem());
        });
        m_items.insert(position, item);
    }

    /* Binary function that accepts two elements in the range as arguments,
     * and returns a value convertible to bool. The value returned indicates
     * whether the element passed as first argument is considered to go before
     * the second in the specific strict weak ordering it defines.
     * The function shall not modify any of its arguments.
     * This can either be a function pointer or a function object.
     */
    void insert(MenuItem::Ptr item, std::function<bool(MenuItem::Ptr a, MenuItem::Ptr b)> compare)
    {
        std::lock_guard<std::recursive_mutex> lg(m_mutex);
        for (auto iter = m_items.begin(); iter != m_items.end(); iter++) {
            if (compare(item, *iter)) {
                insert(item, iter);
                return;
            }
        }
        // end of list reached.
        append(item);
    }

    //iterator insertAbove(MenuItem::Ptr item, iterator position);
    //iterator insertBelow(MenuItem::Ptr item, iterator position);

    void remove(iterator item)
    {
        std::lock_guard<std::recursive_mutex> lg(m_mutex);
        /// @todo check that the item actually is part of the menu
        /// @todo disconnect the changed signal
        int index = 0;
        auto iter = m_items.begin();
        while (iter != item && iter != m_items.end()) {
            ++index;
            ++iter;
        }

        if (iter == m_items.end())
         return;

        GMainLoopDispatch([=](){
            g_menu_remove(m_gmenu.get(), index);
        });
        m_items.erase(item);
    }

    void removeAll(MenuItem::Ptr item)
    {
        std::lock_guard<std::recursive_mutex> lg(m_mutex);
        /// @todo check that item has been added before.
        /// @todo disconnect the changed signal

        // work reversed so that GMenu positions match tbe m_items positions
        int index = m_items.size()-1;
        for (auto iter = m_items.rbegin(); iter != m_items.rend(); ++iter) {
            if (*iter == item) {
                GMainLoopDispatch([=](){
                    g_menu_remove(m_gmenu.get(), index);
                });
            }
            --index;
        }
        // after the loop finishes we should be at index -1
        // which is one above the first g_menu index (=0)
        assert(index == -1);
        auto iter = std::remove(m_items.begin(), m_items.end(), item);
        m_items.erase(iter, m_items.end());
    }

    //void removeRange(iterator first, iterator last);

    void moveTo(iterator item, iterator position)
    {
        std::lock_guard<std::recursive_mutex> lg(m_mutex);
        /// @todo assert that iter is part of the group
        /// @todo assert that position is valid

        if (item == position)
            return;

        remove(item);
        insert(*item, position);
    }

    //void moveRangeTo(iterator first, iterator last, iterator position);

    /// finds the first occurence of item
    iterator find(MenuItem::Ptr item)
    {
        std::lock_guard<std::recursive_mutex> lg(m_mutex);
        return std::find(m_items.begin(), m_items.end(), item);
    }

    iterator begin()
    {
        std::lock_guard<std::recursive_mutex> lg(m_mutex);
        return m_items.begin();
    }

    iterator end()
    {
        std::lock_guard<std::recursive_mutex> lg(m_mutex);
        return m_items.end();
    }

    // clear the whole menu
    void clear()
    {
        std::lock_guard<std::recursive_mutex> lg(m_mutex);
        GMainLoopDispatch([=](){
            g_menu_remove_all(m_gmenu.get());
        });
        m_items.clear();
    }

    operator GMenuModel*() { return G_MENU_MODEL(m_gmenu.get()); }
};

#endif
