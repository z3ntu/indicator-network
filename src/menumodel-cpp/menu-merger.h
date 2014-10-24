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

#ifndef MENU_MERGER_H
#define MENU_MERGER_H

#include <algorithm>
#include <memory>
#include <vector>
#include <map>

#include <gio/gio.h>

#include "gio-helpers/util.h"
#include "menu-model.h"
#include "menu.h"

class MenuMerger : public MenuModel
{
    GMenuPtr m_gmenu;
    std::vector<MenuModel::Ptr> m_menus;

    std::map<GMenuModel*, MenuModel::Ptr> m_gmodelToMenu;
    std::map<GMenuModel*, int> m_startPositions;
    std::map<GMenuModel*, gulong> m_handlerId;

    static void items_changed_cb(GMenuModel *model,
                                 gint        position,
                                 gint        removed,
                                 gint        added,
                                 gpointer    user_data)
    {
        MenuMerger *that = static_cast<MenuMerger*>(user_data);
        that->itemsChanged(model, position, removed, added);
    }

    void itemsChanged(GMenuModel *model,
                      gint        position,
                      gint        removed,
                      gint        added)
    {
        int offset = m_startPositions[model] + position;
        auto menu = m_gmenu;

        GMainLoopDispatch([=](){

            std::cout << "itemsChanged : " << "offset " << offset << ", removed " << removed << ", added " << added << std::endl;

            for (int i = 0; i < removed; ++i) {
                std::cout << "\tRemoving: " << std::string(g_variant_get_string(g_menu_model_get_item_attribute_value(G_MENU_MODEL(menu.get()), offset, "label", NULL), NULL)) << std::endl;
                g_menu_remove(menu.get(), offset);
            }
            if (added > 0)
                std::cout << "\t=== Reversed ===" << std::endl;
            for (int i = added-1; i >= 0; --i) {
                auto item = g_menu_item_new_from_model(model, position + i);

                std::cout << ": " << "\tAdding " << std::string(g_variant_get_string(g_menu_item_get_attribute_value(item, "label", NULL), NULL)) << std::endl;

                g_menu_insert_item(menu.get(), offset, item);
                g_object_unref(item);
            }
            if (added > 0)
                std::cout << "\t=== Reversed ===" << std::endl;
        });

        int delta = added - removed;
        bool update = false;
        for (auto iter : m_menus) {
            if (update) {
                m_startPositions[*iter] += delta;
                continue;
            }
            if (m_gmodelToMenu[model] == iter) {
                // the remaining positions need updating
                update = true;
                continue;
            }
        }
    }

public:
    typedef std::shared_ptr<MenuMerger> Ptr;

    MenuMerger()
    {
        m_gmenu = make_gmenu_ptr();
    }

    ~MenuMerger()
    {
        clear();
        GMainLoopSync([]{});
    }

    void append(MenuModel::Ptr menu)
    {
        // calculate the start position for the items for the new menu
        int start_position;
        if (m_menus.empty()) {
            start_position = 0;
        } else {
            start_position = m_startPositions[*m_menus.back()];
            start_position += m_menus.back()->size();
        }

        m_menus.push_back(menu);
        m_gmodelToMenu[*menu] = menu;
        m_startPositions[*menu] = start_position;

        // add all items
        itemsChanged(*menu, 0, 0, menu->size());
        m_handlerId[*menu] = g_signal_connect(menu->operator GMenuModel *(),
                                              "items-changed",
                                              G_CALLBACK(MenuMerger::items_changed_cb),
                                              this);
    }

    void remove(MenuModel::Ptr menu)
    {
        /// @todo menu might have been added multiple times
        assert(std::find(m_menus.begin(), m_menus.end(), menu) != m_menus.end());

        g_signal_handler_disconnect(menu->operator GMenuModel *(), m_handlerId[*menu]);
        m_handlerId.erase(*menu);

        // remove all items
        itemsChanged(*menu, 0, menu->size(), 0);

        m_startPositions.erase(*menu);
        m_gmodelToMenu.erase(*menu);
        m_menus.erase(std::remove(m_menus.begin(), m_menus.end(), menu), m_menus.end());
    }

    void clear()
    {
        std::vector<MenuModel::Ptr> tmp = m_menus;
        for (auto menu : tmp)
            remove(menu);
    }

    size_t size() const
    {
        size_t tmp = 0;
        for (const auto &menu : m_menus) {
            tmp += menu->size();
        }
        return tmp;
    }

    operator GMenuModel*() { return G_MENU_MODEL(m_gmenu.get()); }
};

#endif
