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

#ifndef SERVICE_H
#define SERVICE_H

#include <com/ubuntu/connectivity/networking/manager.h>
#include <com/ubuntu/connectivity/networking/wifi/link.h>
namespace networking = com::ubuntu::connectivity::networking;

#include <functional>
#include <iostream>

#include <cassert>

#include "menuitems/switch-item.h"
#include "indicator-menu.h"
#include "menumodel-cpp/menu-exporter.h"
#include "menumodel-cpp/menu.h"
#include "wifi-link-item.h"

class Service
{
    IndicatorMenu::Ptr m_desktopMenu;
    IndicatorMenu::Ptr m_desktopGreeterMenu;
    IndicatorMenu::Ptr m_desktopWifiSettingsMenu;

    IndicatorMenu::Ptr m_tabletMenu;
    IndicatorMenu::Ptr m_tabletGreeterMenu;
    IndicatorMenu::Ptr m_tabletWifiSettingsMenu;

    IndicatorMenu::Ptr m_phoneMenu;
    IndicatorMenu::Ptr m_phoneGreeterMenu;
    IndicatorMenu::Ptr m_phoneWifiSettingsMenu;

    IndicatorMenu::Ptr m_ubiquityMenu;

    std::shared_ptr<networking::Manager> m_manager;


    SwitchItem::Ptr m_flightModeSwitch;

    WifiLinkItem::Ptr m_wifiLink;


    TextItem::Ptr m_openWifiSettings;
    TextItem::Ptr m_openCellularSettings;

    std::unique_ptr<MenuExporter> m_desktopMenuExporter;
    std::unique_ptr<MenuExporter> m_desktopGreeterMenuExporter;
    std::unique_ptr<MenuExporter> m_desktopWifiSettingsMenuExporter;

    std::unique_ptr<MenuExporter> m_phoneMenuExporter;
    std::unique_ptr<MenuExporter> m_phoneGreeterMenuExporter;
    std::unique_ptr<MenuExporter> m_phoneWifiSettingsMenuExporter;

    std::unique_ptr<MenuExporter> m_tabletpMenuExporter;
    std::unique_ptr<MenuExporter> m_tabletGreeterMenuExporter;
    std::unique_ptr<MenuExporter> m_tabletWifiSettingsMenuExporter;

    std::unique_ptr<MenuExporter> m_ubiquityMenuExporter;

    std::unique_ptr<ActionGroupExporter> m_actionGroupExporter;
    std::unique_ptr<ActionGroupMerger> m_actionGroupMerger;

    std::shared_ptr<SessionBus> m_sessionBus;
    std::unique_ptr<BusName> m_busName;

    static void url_dispatcher_cb(const gchar * url, gboolean success, gpointer user_data)
    {
        if (!success) {
            std::cerr << "URLDispatch failed on " << url << std::endl;
        }
    }

public:
    Service()
    {
        m_sessionBus.reset(new SessionBus());

        m_manager = networking::Manager::createInstance();

        m_desktopMenu = std::make_shared<IndicatorMenu>("desktop");
        m_desktopGreeterMenu = std::make_shared<IndicatorMenu>("desktop.greeter");
        m_desktopWifiSettingsMenu = std::make_shared<IndicatorMenu>("desktop.wifi.settings");

        m_tabletMenu = std::make_shared<IndicatorMenu>("tablet");
        m_tabletGreeterMenu = std::make_shared<IndicatorMenu>("tablet.greeter");
        m_tabletWifiSettingsMenu = std::make_shared<IndicatorMenu>("tablet.wifi.settings");

        m_phoneMenu = std::make_shared<IndicatorMenu>("phone");
        m_phoneGreeterMenu = std::make_shared<IndicatorMenu>("phone.greeter");
        m_phoneWifiSettingsMenu = std::make_shared<IndicatorMenu>("phone.wifi.settings");

        m_ubiquityMenu = std::make_shared<IndicatorMenu>("ubiquity");


        m_desktopMenu->setIcons({"nm-signal-75", "nm-signal-25"});
        m_desktopMenu->setIcon("nm-signal-75-secure");
        m_phoneMenu->setIcons({"nm-signal-75", "nm-signal-25"});
        m_phoneMenu->setIcon("nm-signal-75-secure");

        m_flightModeSwitch = std::make_shared<SwitchItem>(_("Flight Mode"), "airplane", "enabled");
        switch (m_manager->flightMode().get()) {
        case networking::Manager::FlightModeStatus::off:
            m_flightModeSwitch->state().set(false);
            break;
        case networking::Manager::FlightModeStatus::on:
            m_flightModeSwitch->state().set(true);
            break;
        }
        m_manager->flightMode().changed().connect([this](networking::Manager::FlightModeStatus value){
            switch (value) {
            case networking::Manager::FlightModeStatus::off:
                m_flightModeSwitch->state().set(false);
                break;
            case networking::Manager::FlightModeStatus::on:
                m_flightModeSwitch->state().set(true);
                break;
            }
        });
        /// @todo handle icons properly
        m_flightModeSwitch->state().changed().connect([this](bool value){
            if (value) {
                //m_manager->enableFlightMode();
                m_desktopMenu->setIcon("airplane-mode");
                m_phoneMenu->setIcon("airplane-mode");
            } else {
                //m_manager->disableFlightMode();
                m_desktopMenu->setIcon("nm-signal-75-secure");
                m_phoneMenu->setIcon("nm-signal-75-secure");
            }
        });

        m_desktopMenu->addItem(m_flightModeSwitch);
        m_phoneMenu->addItem(m_flightModeSwitch);

        for (auto link : m_manager->links().get()) {
            auto wifi_link = std::dynamic_pointer_cast<networking::wifi::Link>(link);
            m_wifiLink = std::make_shared<WifiLinkItem>(wifi_link);
            m_desktopMenu->addItem(m_wifiLink);
            m_phoneMenu->addItem(m_wifiLink);
            // just take the first one now.
            /// @todo multiple links and links()->changed()
            break;
        }

        m_openWifiSettings = std::make_shared<TextItem>(_("Wi-Fi settings…"), "wifi", "settings");
        m_openWifiSettings->activated().connect([this](){
            url_dispatch_send("settings:///system/wifi", Service::url_dispatcher_cb, this);
        });
        m_desktopMenu->addItem(m_openWifiSettings);
        m_phoneMenu->addItem(m_openWifiSettings);

        auto unlockSim = std::make_shared<TextItem>(_("Unlock SIM…"), "sim", "unlock");
        m_desktopMenu->addItem(unlockSim);
        m_phoneMenu->addItem(unlockSim);

        m_openCellularSettings = std::make_shared<TextItem>(_("Cellular settings…"), "cellular", "settings");
        m_openCellularSettings->activated().connect([this](){
            url_dispatch_send("settings:///system/cellular", Service::url_dispatcher_cb, this);
        });
        m_desktopMenu->addItem(m_openCellularSettings);
        m_phoneMenu->addItem(m_openCellularSettings);


        m_desktopMenuExporter.reset(new MenuExporter(m_sessionBus, "/com/canonical/indicator/network/desktop", m_desktopMenu->menu()));
        m_desktopGreeterMenuExporter.reset(new MenuExporter(m_sessionBus, "/com/canonical/indicator/network/desktop_greeter", m_desktopGreeterMenu->menu()));
        m_desktopWifiSettingsMenuExporter.reset(new MenuExporter(m_sessionBus, "/com/canonical/indicator/network/desktop_wifi_settings", m_desktopWifiSettingsMenu->menu()));

        m_tabletpMenuExporter.reset(new MenuExporter(m_sessionBus, "/com/canonical/indicator/network/tablet", m_tabletMenu->menu()));
        m_tabletGreeterMenuExporter.reset(new MenuExporter(m_sessionBus, "/com/canonical/indicator/network/tablet_greeter", m_tabletGreeterMenu->menu()));
        m_tabletWifiSettingsMenuExporter.reset(new MenuExporter(m_sessionBus, "/com/canonical/indicator/network/tablet_wifi_settings", m_tabletWifiSettingsMenu->menu()));

        m_phoneMenuExporter.reset(new MenuExporter(m_sessionBus, "/com/canonical/indicator/network/phone", m_phoneMenu->menu()));
        m_phoneGreeterMenuExporter.reset(new MenuExporter(m_sessionBus, "/com/canonical/indicator/network/phone_greeter", m_phoneGreeterMenu->menu()));
        m_phoneWifiSettingsMenuExporter.reset(new MenuExporter(m_sessionBus, "/com/canonical/indicator/network/phone_wifi_settings", m_phoneWifiSettingsMenu->menu()));

        m_ubiquityMenuExporter.reset(new MenuExporter(m_sessionBus, "/com/canonical/indicator/network/ubiquity", m_ubiquityMenu->menu()));

        // we have a single actiongroup for all the menus.
        m_actionGroupMerger.reset(new ActionGroupMerger());
        m_actionGroupMerger->add(m_desktopMenu->actionGroup());
        m_actionGroupMerger->add(m_phoneMenu->actionGroup());
        m_actionGroupExporter.reset(new ActionGroupExporter(m_sessionBus, m_actionGroupMerger->actionGroup(),
                                                            "/com/canonical/indicator/network",
                                                            "indicator"));

        m_busName.reset(new BusName("com.canonical.indicator.network",
                                    [](std::string) { std::cout << "acquired" << std::endl;
                                    },
                                    [](std::string) { std::cout << "lost" << std::endl; },
                        m_sessionBus));
    }
};

#endif
