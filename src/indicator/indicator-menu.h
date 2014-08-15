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

#ifndef INDICATOR_MENU_H
#define INDICATOR_MENU_H

#include "menuitems/section.h"
#include "menumodel-cpp/menu-merger.h"
#include "menumodel-cpp/action-group-merger.h"
#include "root-state.h"

#include <vector>

class IndicatorMenu
{
public:
    typedef std::shared_ptr<IndicatorMenu> Ptr;

    IndicatorMenu() = delete;
    virtual ~IndicatorMenu() = default;

    IndicatorMenu(RootState::Ptr rootState, const std::string &prefix)
        : m_rootState{rootState},
          m_prefix{prefix}
    {
        m_actionGroupMerger = std::make_shared<ActionGroupMerger>();
        m_actionGroup = std::make_shared<ActionGroup>();

        m_actionGroupMerger->add(m_actionGroup);

        m_rootAction = std::make_shared<Action>(prefix + ".network-status",
                                                nullptr,
                                                rootState->state().get());
        rootState->state().changed().connect([this](const Variant &state){
            m_rootAction->setState(state);
        });
        m_actionGroup->add(m_rootAction);

        m_rootMenu = std::make_shared<Menu>();
        m_subMenuMerger = std::make_shared<MenuMerger>();

        m_rootItem = MenuItem::newSubmenu(m_subMenuMerger);

        m_rootItem->setAction("indicator." + prefix + ".network-status");
        m_rootItem->setAttribute("x-canonical-type", TypedVariant<std::string>("com.canonical.indicator.root"));
        m_rootMenu->append(m_rootItem);
    }

    virtual void
    addSection(Section::Ptr section)
    {
        m_sections.push_back(section);
        m_actionGroupMerger->add(*section);
        m_subMenuMerger->append(*section);
    }

    Menu::Ptr
    menu() const
    {
        return m_rootMenu;
    }

    ActionGroup::Ptr
    actionGroup() const
    {
        return m_actionGroupMerger->actionGroup();
    }

private:
    RootState::Ptr m_rootState;
    std::string m_prefix;

    Action::Ptr m_rootAction;
    MenuItem::Ptr m_rootItem;

    Menu::Ptr m_rootMenu;
    MenuMerger::Ptr m_subMenuMerger;

    ActionGroupMerger::Ptr m_actionGroupMerger;
    ActionGroup::Ptr m_actionGroup;

    std::vector<Section::Ptr> m_sections;
};

#endif // INDICATOR_MENU_H
