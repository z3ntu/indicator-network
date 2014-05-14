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

#include "wifi-section.h"

#include "wifi-link-item.h"

#include "url-dispatcher-cpp/url-dispatcher.h"

class WifiSection::Private
{
public:
    std::shared_ptr<connectivity::networking::Manager> m_manager;

    ActionGroupMerger::Ptr m_actionGroupMerger;
    Menu::Ptr m_menu;
    Menu::Ptr m_settingsMenu;

    WifiLinkItem::Ptr m_wifiLink;
    TextItem::Ptr m_openWifiSettings;

    Private() = delete;
    Private(std::shared_ptr<connectivity::networking::Manager> manager)
        : m_manager{manager}
    {
        m_actionGroupMerger = std::make_shared<ActionGroupMerger>();
        m_menu = std::make_shared<Menu>();
        m_settingsMenu = std::make_shared<Menu>();


        for (auto link : m_manager->links().get()) {
            auto wifi_link = std::dynamic_pointer_cast<networking::wifi::Link>(link);
            m_wifiLink = std::make_shared<WifiLinkItem>(wifi_link);

            m_actionGroupMerger->add(*m_wifiLink);
            m_menu->append(*m_wifiLink);
            m_settingsMenu->append(*m_wifiLink);

            // just take the first one now.
            /// @todo multiple links and links()->changed()
            break;
        }

        m_openWifiSettings = std::make_shared<TextItem>(_("Wi-Fi settings…"), "wifi", "settings");
        m_openWifiSettings->activated().connect([](){
            UrlDispatcher::send("settings:///system/wifi", [](std::string url, bool success){
                if (!success)
                    std::cerr << "URL Dispatcher failed on " << url << std::endl;
            });
        });

        m_actionGroupMerger->add(*m_openWifiSettings);
        m_menu->append(*m_openWifiSettings);
    }

public:
};

WifiSection::WifiSection(std::shared_ptr<connectivity::networking::Manager> manager)
{
    d.reset(new Private(manager));
}

WifiSection::~WifiSection()
{

}

ActionGroup::Ptr
WifiSection::actionGroup()
{
    return d->m_actionGroupMerger->actionGroup();
}

MenuModel::Ptr
WifiSection::menuModel()
{
    return d->m_menu;
}

MenuModel::Ptr
WifiSection::settingsModel()
{
    return d->m_settingsMenu;
}
