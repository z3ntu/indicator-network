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

#include "quick-access-section.h"

#include "menuitems/switch-item.h"

#include "menumodel-cpp/action-group-merger.h"
#include "menumodel-cpp/menu-merger.h"

#include <util/localisation.h>

#include <QDebug>

using namespace std;
using namespace nmofono;

class QuickAccessSection::Private : public QObject
{
    Q_OBJECT

public:
    ActionGroupMerger::Ptr m_actionGroupMerger;
    Menu::Ptr m_menu;

    Manager::Ptr m_manager;

    SwitchItem::Ptr m_flightModeSwitch;

    Private() = delete;
    Private(Manager::Ptr manager, SwitchItem::Ptr flightModeSwitch);
};

QuickAccessSection::Private::Private(Manager::Ptr manager, SwitchItem::Ptr flightModeSwitch)
    : m_manager{manager}, m_flightModeSwitch{flightModeSwitch}
{
    m_actionGroupMerger = std::make_shared<ActionGroupMerger>();
    m_menu = std::make_shared<Menu>();

    m_menu->append(m_flightModeSwitch->menuItem());
}

QuickAccessSection::QuickAccessSection(Manager::Ptr manager, SwitchItem::Ptr flightModeSwitch)
    : d{new Private(manager, flightModeSwitch)}
{
}

QuickAccessSection::~QuickAccessSection()
{
}

ActionGroup::Ptr
QuickAccessSection::actionGroup()
{
    return d->m_actionGroupMerger->actionGroup();
}

MenuModel::Ptr
QuickAccessSection::menuModel()
{
    return d->m_menu;
}

#include "quick-access-section.moc"
