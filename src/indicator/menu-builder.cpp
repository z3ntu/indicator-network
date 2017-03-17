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

#include <menu-builder.h>
#include <factory.h>

using namespace std;

struct MenuBuilder::Priv: public QObject
{
    Q_OBJECT

public:
    nmofono::Manager::Ptr m_manager;

    IndicatorMenu::Ptr m_mainMenu;
    IndicatorMenu::Ptr m_greeterMenu;

    IndicatorMenu::Ptr m_ubiquityMenu;

    RootState::Ptr m_rootState;

    SwitchItem::Ptr m_flightModeSwitch;
    SwitchItem::Ptr m_mobileDataSwitch;
    SwitchItem::Ptr m_hotspotSwitch;
    SwitchItem::Ptr m_wifiSwitch;

    QuickAccessSection::SPtr m_quickAccessSection;
    EthernetSection::SPtr m_ethernetSection;
    WifiSection::SPtr m_wifiSection;
    WwanSection::SPtr m_wwanSection;
    VpnSection::SPtr m_vpnSection;

    MenuExporter::UPtr m_menuExporter;
    MenuExporter::UPtr m_greeterMenuExporter;
    MenuExporter::UPtr m_wifiSettingsMenuExporter;

    MenuExporter::UPtr m_ubiquityMenuExporter;

    ActionGroupExporter::UPtr m_actionGroupExporter;
    ActionGroupMerger::UPtr m_actionGroupMerger;

    BusName::UPtr m_busName;

public Q_SLOTS:
    void unstoppableOperationHappeningUpdated(bool happening)
    {
        m_flightModeSwitch->setEnabled(!happening);
        m_wifiSwitch->setEnabled(!happening);
        updateHotspotSwitch();
        updateMobileDataSwitch();

        if (happening)
        {
            // Give the GActionGroup a chance to emit its Changed signal
            runGMainloop();
        }
    }

    void updateHotspotSwitch()
    {
        m_hotspotSwitch->setEnabled(
            !m_manager->unstoppableOperationHappening()
            && !m_manager->flightMode());
    }

    void updateMobileDataSwitch()
    {
        m_mobileDataSwitch->setEnabled(!m_manager->flightMode() &&
                                        m_manager->simForMobileData() &&
                                        m_manager->simForMobileData()->present() &&
                                       !m_manager->unstoppableOperationHappening());
    }

    void updateSimForMobileData()
    {
        auto sim = m_manager->simForMobileData();
        if (sim)
        {
            QObject::connect(sim.get(), &nmofono::wwan::Sim::presentChanged,
                             this, &Priv::updateMobileDataSwitch);
        }
        updateMobileDataSwitch();
    }
};

MenuBuilder::MenuBuilder(nmofono::Manager::Ptr manager, Factory& factory) :
        d(new Priv)
{
    d->m_manager = manager;

    d->m_rootState = factory.newRootState();

    d->m_mainMenu = factory.newIndicatorMenu(d->m_rootState, "phone");
    d->m_greeterMenu = factory.newIndicatorMenu(d->m_rootState, "phone.greeter");

    d->m_ubiquityMenu = factory.newIndicatorMenu(d->m_rootState, "ubiquity");

    d->m_flightModeSwitch = factory.newFlightModeSwitch();
    d->m_mobileDataSwitch = factory.newMobileDataSwitch();
    d->m_hotspotSwitch = factory.newHotspotSwitch();
    d->m_wifiSwitch = factory.newWifiSwitch();

    // Connect the unstoppable operation property to the toggle enabled properties
    connect(d->m_manager.get(), &nmofono::Manager::unstoppableOperationHappeningUpdated,
            d.get(), &Priv::unstoppableOperationHappeningUpdated);

    // Hotspot enabled toggle is also controlled by flight and WiFi status
    connect(d->m_manager.get(), &nmofono::Manager::flightModeUpdated,
            d.get(), &Priv::updateHotspotSwitch);
    connect(d->m_manager.get(), &nmofono::Manager::wifiEnabledUpdated,
            d.get(), &Priv::updateHotspotSwitch);

    // mobile data enabled depend on these properties
    connect(d->m_manager.get(), &nmofono::Manager::flightModeUpdated,
            d.get(), &Priv::updateMobileDataSwitch);
    connect(d->m_manager.get(), &nmofono::Manager::simForMobileDataChanged,
            d.get(), &Priv::updateSimForMobileData);
    // call updateSimForMobileData to also connect to the Sim::presentChanged()
    d->updateSimForMobileData();

    d->m_quickAccessSection = factory.newQuickAccessSection(d->m_flightModeSwitch);
    d->m_wwanSection = factory.newWwanSection(d->m_mobileDataSwitch, d->m_hotspotSwitch);
    d->m_ethernetSection = factory.newEthernetSection();
    d->m_wifiSection = factory.newWiFiSection(d->m_wifiSwitch);
    d->m_vpnSection = factory.newVpnSection();

    d->m_mainMenu->addSection(d->m_quickAccessSection);
    d->m_greeterMenu->addSection(d->m_quickAccessSection);

    d->m_mainMenu->addSection(d->m_wwanSection);
    d->m_greeterMenu->addSection(d->m_wwanSection);

    d->m_mainMenu->addSection(d->m_ethernetSection);
    d->m_greeterMenu->addSection(d->m_ethernetSection);

    d->m_mainMenu->addSection(d->m_wifiSection);
    d->m_greeterMenu->addSection(d->m_wifiSection);

    d->m_mainMenu->addSection(d->m_vpnSection);
    d->m_greeterMenu->addSection(d->m_vpnSection);

    // we have a single actiongroup for all the menus.
    d->m_actionGroupMerger = factory.newActionGroupMerger();
    d->m_actionGroupMerger->add(d->m_flightModeSwitch->actionGroup());
    d->m_actionGroupMerger->add(d->m_wifiSwitch->actionGroup());
    d->m_actionGroupMerger->add(d->m_hotspotSwitch->actionGroup());
    d->m_actionGroupMerger->add(d->m_mainMenu->actionGroup());
    d->m_actionGroupMerger->add(d->m_greeterMenu->actionGroup());
    d->m_actionGroupExporter = factory.newActionGroupExporter(d->m_actionGroupMerger->actionGroup(),
                                                        "/com/canonical/indicator/network");

    d->m_menuExporter = factory.newMenuExporter("/com/canonical/indicator/network/phone", d->m_mainMenu->menu());
    d->m_greeterMenuExporter = factory.newMenuExporter("/com/canonical/indicator/network/phone_greeter", d->m_greeterMenu->menu());
    d->m_wifiSettingsMenuExporter = factory.newMenuExporter("/com/canonical/indicator/network/phone_wifi_settings", d->m_wifiSection->settingsModel());

    d->m_ubiquityMenuExporter = factory.newMenuExporter("/com/canonical/indicator/network/ubiquity", d->m_ubiquityMenu->menu());

    d->m_busName = factory.newBusName("com.canonical.indicator.network",
                                [](std::string) {
#ifdef INDICATOR_NETWORK_TRACE_MESSAGES
        std::cout << "acquired" << std::endl;
#endif
                                },
                                [](std::string) {
#ifdef INDICATOR_NETWORK_TRACE_MESSAGES
                                    std::cout << "lost" << std::endl;
#endif
                                });
}

#include "menu-builder.moc"
