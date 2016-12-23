/*
 * Copyright (C) 2016 Canonical, Ltd.
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
 * Author: Pete Woods <pete.woods@canonical.com>
 */

#include <indicator-network-test-base-desktop.h>
#include <util/dbus-property-cache.h>

#include <QDebug>
#include <QSignalSpy>

using namespace std;
using namespace testing;
namespace mh = unity::gmenuharness;

namespace
{

class TestIndicatorDesktop: public IndicatorNetworkTestBaseDesktop
{
};

TEST_F(TestIndicatorDesktop, BasicContentsWifiEthernet)
{
    setGlobalConnectedState(NM_STATE_CONNECTED_GLOBAL);
    ASSERT_NO_THROW(startIndicator());
    auto device = createWiFiDevice(NM_DEVICE_STATE_DISCONNECTED);
    auto ap = createAccessPoint("0", "the ssid", device);
    auto eth0 = createEthernetDevice(NM_DEVICE_STATE_ACTIVATED, "1");
    auto eth0connection0 = createEthernetConnection("Home", eth0);
    auto eth0connection1 = createEthernetConnection("Work", eth0);
    createActiveConnection("0", eth0, eth0connection0);

    EXPECT_MATCHRESULT(mh::MenuMatcher(phoneParameters())
        .item(mh::MenuItemMatcher()
            .state_icons({"network-wired-connected"})
            .mode(mh::MenuItemMatcher::Mode::starts_with)
            .submenu()
            .item(mh::MenuItemMatcher())
            .item(mh::MenuItemMatcher()
                .section()
                .item(ethernetInfo("Ethernet",
                      "Connected",
                      Toggle::enabled)
                )
                .item(radio("Home", Toggle::enabled))
                .item(radio("Work", Toggle::disabled))
            )
            .item(ethernetSettings())
            .item(wifiEnableSwitch())
            .item(mh::MenuItemMatcher()
                .section()
                .item(accessPoint("the ssid",
                    Secure::wpa,
                    ApMode::infra,
                    ConnectionStatus::disconnected)
                )
            )
        ).match());
}

TEST_F(TestIndicatorDesktop, WifiToggleTalksToNetworkManager)
{
    setGlobalConnectedState(NM_STATE_CONNECTED_GLOBAL);
    auto device = createWiFiDevice(NM_DEVICE_STATE_DISCONNECTED);

    ASSERT_NO_THROW(startIndicator());

    util::DBusPropertyCache networkManager(NM_DBUS_SERVICE, NM_DBUS_INTERFACE, NM_DBUS_PATH, dbusTestRunner.systemConnection());
    QSignalSpy networkManagerSpy(&networkManager, SIGNAL(propertyChanged(const QString&, const QVariant&)));

    EXPECT_TRUE(networkManager.get("WirelessEnabled").toBool());

    EXPECT_MATCHRESULT(mh::MenuMatcher(phoneParameters())
        .item(mh::MenuItemMatcher()
            .mode(mh::MenuItemMatcher::Mode::starts_with)
            .submenu()
            .item(mh::MenuItemMatcher())
            .item(wifiEnableSwitch().activate())
        ).match());

    if (networkManagerSpy.isEmpty())
    {
        ASSERT_TRUE(networkManagerSpy.wait());
    }
    EXPECT_FALSE(networkManager.get("WirelessEnabled").toBool());
}

TEST_F(TestIndicatorDesktop, WifiToggleFollowsNetworkManager)
{
    setGlobalConnectedState(NM_STATE_CONNECTED_GLOBAL);
    auto device = createWiFiDevice(NM_DEVICE_STATE_DISCONNECTED);

    ASSERT_NO_THROW(startIndicator());

    // WiFi should start enabled
    EXPECT_MATCHRESULT(mh::MenuMatcher(phoneParameters())
        .item(mh::MenuItemMatcher()
            .mode(mh::MenuItemMatcher::Mode::starts_with)
            .submenu()
            .item(mh::MenuItemMatcher())
            .item(wifiEnableSwitch())
        ).match());

    auto& nm = dbusMock.networkManagerInterface();
    nm.SetProperty(NM_DBUS_PATH, NM_DBUS_INTERFACE, "WirelessEnabled", QDBusVariant(false));

    // WiFi should now be disabled
    EXPECT_MATCHRESULT(mh::MenuMatcher(phoneParameters())
        .item(mh::MenuItemMatcher()
            .mode(mh::MenuItemMatcher::Mode::starts_with)
            .submenu()
            .item(mh::MenuItemMatcher())
            .item(wifiEnableSwitch(false))
        ).match());
}

} // namespace
