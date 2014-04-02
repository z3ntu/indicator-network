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

#include "wwan-section.h"

#include "menuitems/text-item.h"

#include "menumodel-cpp/action-group-merger.h"
#include "menumodel-cpp/menu-merger.h"

#include "modem-manager.h"

#include "url-dispatcher-cpp/url-dispatcher.h"

class WwanSection::Private
{
public:

    ActionGroupMerger::Ptr m_actionGroupMerger;
    Menu::Ptr m_menu;

    ModemManager::Ptr m_modemManager;

    TextItem::Ptr m_openCellularSettings;

    Private() = delete;
    Private(ModemManager::Ptr modemManager);
};

WwanSection::Private::Private(ModemManager::Ptr modemManager)
    : m_modemManager{modemManager}
{
    m_actionGroupMerger = std::make_shared<ActionGroupMerger>();
    m_menu = std::make_shared<Menu>();

    /// @todo support more than one sim
    /// @todo support sims().changed()
    /// @todo add Item::visible
    /// @todo support isLocked().changed()
    if (m_modemManager->modems()->size() == 1 &&
        m_modemManager->modems()->begin()->get()->simStatus().get() == Modem::SimStatus::locked) {
            ; // show the unlock thingy.
    }

    m_openCellularSettings = std::make_shared<TextItem>(_("Cellular settings…"), "cellular", "settings");
    m_openCellularSettings->activated().connect([](){
        UrlDispatcher::send("settings:///system/cellular", [](std::string url, bool success){
            if (!success)
                std::cerr << "URL Dispatcher failed on " << url << std::endl;
        });
    });

    m_menu->append(*m_openCellularSettings);
    m_actionGroupMerger->add(*m_openCellularSettings);
}

WwanSection::WwanSection(ModemManager::Ptr modemManager)
{
    d.reset(new Private(modemManager));
}

WwanSection::~WwanSection()
{

}

ActionGroup::Ptr
WwanSection::actionGroup()
{
    return *d->m_actionGroupMerger;
}

MenuModel::Ptr
WwanSection::menuModel()
{
    return d->m_menu;
}
