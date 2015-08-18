/*
 * Copyright (C) 2015 Canonical, Ltd.
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
 *     Pete Woods <pete.woods@canonical.com>
 */

#include <config.h>
#include <factory.h>

#include <util/localisation.h>
#include <nmofono/manager-impl.h>
#include <notify-cpp/notification-manager.h>

using namespace std;

struct Factory::Private
{
    shared_ptr<nmofono::Manager> m_nmofono;

    SessionBus::Ptr m_sessionBus;

    notify::NotificationManager::SPtr m_notificationManager;

    notify::NotificationManager::SPtr singletonNotificationManager()
    {
        if (!m_notificationManager)
        {
            m_notificationManager = make_shared<notify::NotificationManager>(GETTEXT_PACKAGE);
        }
        return m_notificationManager;
    }

    SessionBus::Ptr singletonSessionBus()
    {
        if (!m_sessionBus)
        {
            m_sessionBus = make_shared<SessionBus>();
        }
        return m_sessionBus;
    }

    shared_ptr<nmofono::Manager> singletonNmofono()
    {
        if (!m_nmofono)
        {
            m_nmofono = make_shared<nmofono::ManagerImpl>(
                    singletonNotificationManager(),
                    QDBusConnection::systemBus());
        }
        return m_nmofono;
    }
};

Factory::Factory() :
        d(new Private)
{
}

Factory::~Factory()
{
}

unique_ptr<MenuBuilder> Factory::newMenuBuilder()
{
    return make_unique<MenuBuilder>(d->singletonNmofono(), *this);
}

unique_ptr<connectivity_service::ConnectivityService> Factory::newConnectivityService()
{
    return make_unique<connectivity_service::ConnectivityService>(d->singletonNmofono(), QDBusConnection::sessionBus());
}

unique_ptr<RootState> Factory::newRootState()
{
    return make_unique<RootState>(d->singletonNmofono());
}

unique_ptr<IndicatorMenu> Factory::newIndicatorMenu(RootState::Ptr rootState, const string &prefix)
{
    return make_unique<IndicatorMenu>(rootState, prefix);
}

unique_ptr<MenuExporter> Factory::newMenuExporter(const string &path, MenuModel::Ptr menuModel)
{
    return make_unique<MenuExporter>(d->singletonSessionBus(), path, menuModel);
}

unique_ptr<QuickAccessSection> Factory::newQuickAccessSection(SwitchItem::Ptr flightModeSwitch)
{
    return make_unique<QuickAccessSection>(d->singletonNmofono(), flightModeSwitch);
}

unique_ptr<WwanSection> Factory::newWwanSection(SwitchItem::Ptr hotspotSwitch)
{
    return make_unique<WwanSection>(d->singletonNmofono(), hotspotSwitch);
}

unique_ptr<WifiSection> Factory::newWiFiSection(SwitchItem::Ptr wifiSwitch)
{
    return make_unique<WifiSection>(d->singletonNmofono(), wifiSwitch);
}

ActionGroupMerger::UPtr Factory::newActionGroupMerger()
{
    return make_unique<ActionGroupMerger>();
}

ActionGroupExporter::UPtr Factory::newActionGroupExporter(ActionGroup::Ptr actionGroup,
                                const string &path)
{
    return make_unique<ActionGroupExporter>(d->singletonSessionBus(), actionGroup, path);
}

SwitchItem::UPtr Factory::newWifiSwitch()
{
    // TODO Move this into a new class
    auto wifiSwitch = make_unique<SwitchItem>(_("Wi-Fi"), "wifi", "enable");
    auto manager = d->singletonNmofono();
    wifiSwitch->setState(manager->wifiEnabled());
    QObject::connect(manager.get(), &nmofono::Manager::wifiEnabledUpdated, wifiSwitch.get(), &SwitchItem::setState);
    QObject::connect(wifiSwitch.get(), &SwitchItem::stateUpdated, manager.get(), &nmofono::Manager::setWifiEnabled);
    return wifiSwitch;
}

SwitchItem::UPtr Factory::newFlightModeSwitch()
{
    // TODO Move this into a new class
    auto flightModeSwitch = make_unique<SwitchItem>(_("Flight Mode"), "airplane", "enabled");
    auto manager = d->singletonNmofono();
    flightModeSwitch->setState(manager->flightMode());
    QObject::connect(manager.get(), &nmofono::Manager::flightModeUpdated, flightModeSwitch.get(), &SwitchItem::setState);
    QObject::connect(flightModeSwitch.get(), &SwitchItem::stateUpdated, manager.get(), &nmofono::Manager::setFlightMode);
    return flightModeSwitch;
}

SwitchItem::UPtr Factory::newHotspotSwitch()
{
    // TODO Move this into a new class
    auto hotspotSwitch = make_unique<SwitchItem>(dgettext("ubuntu-system-settings", "Hotspot"), "hotspot", "enable");
    auto manager = d->singletonNmofono();
    hotspotSwitch->setState(manager->hotspotEnabled());
    QObject::connect(manager.get(), &nmofono::Manager::hotspotEnabledChanged, hotspotSwitch.get(), &SwitchItem::setState);
    QObject::connect(hotspotSwitch.get(), &SwitchItem::stateUpdated, manager.get(), &nmofono::Manager::setHotspotEnabled);
    return hotspotSwitch;
}

BusName::UPtr Factory::newBusName(string name,
                                  function<void(string)> acquired,
                                  function<void(string)> lost)
{
    return make_unique<BusName>(name, acquired, lost, d->singletonSessionBus());
}

agent::SecretAgent::UPtr Factory::newSecretAgent()
{
    return make_unique<agent::SecretAgent>(d->singletonNotificationManager(),
                                           QDBusConnection::systemBus(),
                                           QDBusConnection::sessionBus());
}

