/*
 * Copyright (C) 2013 Canonical, Ltd.
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

#include <libqtdbustest/DBusTestRunner.h>
#include <libqtdbustest/QProcessDBusService.h>
#include <libqtdbusmock/DBusMock.h>

#include <menuharness/MenuMatcher.h>

#include <NetworkManager.h>

#include <QDebug>
#include <QTestEventLoop>
#include <QSignalSpy>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace std;
using namespace testing;
using namespace QtDBusTest;
using namespace QtDBusMock;

namespace mh = menuharness;

namespace
{

class TestIndicatorNetworkService : public Test
{
protected:
    TestIndicatorNetworkService() :
            dbusMock(dbusTestRunner)
    {
    }

    void SetUp() override
    {
        dbusMock.registerNetworkManager();
        // By default the ofono mock starts with one modem
        dbusMock.registerOfono();
        dbusMock.registerURfkill();

        dbusMock.networkManagerInterface();

        dbusTestRunner.startServices();
    }

    void TearDown() override
    {
        QTestEventLoop::instance().enterLoopMSecs(200);
    }

    static mh::MenuMatcher::Parameters phoneParameters()
    {
        return mh::MenuMatcher::Parameters(
                "com.canonical.indicator.network",
                { { "indicator", "/com/canonical/indicator/network" } },
                "/com/canonical/indicator/network/phone");
    }

    void startIndicator()
    {
        indicator.reset(
                new QProcessDBusService("com.canonical.indicator.network",
                                        QDBusConnection::SessionBus,
                                        NETWORK_SERVICE_BIN,
                                        QStringList()));
        indicator->start(dbusTestRunner.sessionConnection());
    }

    DBusTestRunner dbusTestRunner;

    DBusMock dbusMock;

    DBusServicePtr indicator;
};

TEST_F(TestIndicatorNetworkService, BasicMenuContents)
{
    startIndicator();

    EXPECT_MATCHRESULT(mh::MenuMatcher(phoneParameters())
        .item(mh::MenuItemMatcher()
            .action("indicator.phone.network-status")
            .mode(mh::MenuItemMatcher::Mode::all)
            .submenu()
            .item(mh::MenuItemMatcher::checkbox()
                .label("Flight Mode")
                .action("indicator.airplane.enabled")
                .toggled(false)
            )
            .item(mh::MenuItemMatcher()
                .section()
                .item(mh::MenuItemMatcher()
                    .widget("com.canonical.indicator.network.modeminfoitem")
                )
                .item(mh::MenuItemMatcher()
                    .label("Cellular settings…")
                    .action("indicator.cellular.settings")
                )
            )
            .item(mh::MenuItemMatcher::checkbox()
                .label("Wi-Fi")
                .action("indicator.wifi.enable")
                .toggled(true)
            )
            .item(mh::MenuItemMatcher()
                .label("Wi-Fi settings…")
                .action("indicator.wifi.settings")
            )
        ).match());
}

TEST_F(TestIndicatorNetworkService, OneAccessPoint)
{
    auto& networkManager(dbusMock.networkManagerInterface());
    networkManager.AddWiFiDevice("device", "eth1", NM_DEVICE_STATE_DISCONNECTED).waitForFinished();
    networkManager.AddAccessPoint(
            "/org/freedesktop/NetworkManager/Devices/device", "ap", "ssid",
            "11:22:33:44:55:66", 0, 0, 0, 's', 0).waitForFinished();
    networkManager.AddWiFiConnection(
            "/org/freedesktop/NetworkManager/Devices/device", "connection",
            "the ssid", "wpa-psk").waitForFinished();

    startIndicator();

    EXPECT_MATCHRESULT(mh::MenuMatcher(phoneParameters())
        .item(mh::MenuItemMatcher()
            .label("")
            .action("indicator.phone.network-status")
            .mode(mh::MenuItemMatcher::Mode::starts_with)
            .submenu()
            .item(mh::MenuItemMatcher::checkbox()
                .label("Flight Mode")
                .action("indicator.airplane.enabled")
                .icon("")
                .toggled(false)
            )
            .item(mh::MenuItemMatcher()
                .section()
                .item(mh::MenuItemMatcher()
                    .widget("com.canonical.indicator.network.modeminfoitem")
                    .label("")
                    .action("")
                )
                .item(mh::MenuItemMatcher()
                    .label("Cellular settings…")
                    .action("indicator.cellular.settings")
                )
            )
            .item(mh::MenuItemMatcher::checkbox()
                .label("Wi-Fi")
                .action("indicator.wifi.enable")
                .toggled(true)
            )
            .item(mh::MenuItemMatcher()
                .section()
                .item(mh::MenuItemMatcher::checkbox()
                    .label("ssid")
                    .icon("")
                    .action("indicator.accesspoint.1")
                    .toggled(false)
                )
            )
            .item(mh::MenuItemMatcher()
                .label("Wi-Fi settings…")
                .action("indicator.wifi.settings")
            )
        ).match());
}

TEST_F(TestIndicatorNetworkService, SecondModem)
{
    auto& ofono(dbusMock.ofonoInterface());
    {
        QVariantMap modemProperties {{ "Powered", false } };
        ofono.AddModem("ril_1", modemProperties).waitForFinished();
    }

    startIndicator();

    EXPECT_MATCHRESULT(mh::MenuMatcher(phoneParameters())
        .item(mh::MenuItemMatcher()
            .label("")
            .action("indicator.phone.network-status")
            .mode(mh::MenuItemMatcher::Mode::starts_with)
            .submenu()
            .item(mh::MenuItemMatcher::checkbox()
                .label("Flight Mode")
                .action("indicator.airplane.enabled")
                .icon("")
                .toggled(false)
            )
            .item(mh::MenuItemMatcher()
                .section()
                .item(mh::MenuItemMatcher()
                    .widget("com.canonical.indicator.network.modeminfoitem")
                )
                .item(mh::MenuItemMatcher()
                    .widget("com.canonical.indicator.network.modeminfoitem")
                )
                .item(mh::MenuItemMatcher()
                    .label("Cellular settings…")
                    .action("indicator.cellular.settings")
                )
            )
            .item(mh::MenuItemMatcher::checkbox()
                .label("Wi-Fi")
                .action("indicator.wifi.enable")
                .toggled(true)
            )
            .item(mh::MenuItemMatcher()
                .label("Wi-Fi settings…")
                .action("indicator.wifi.settings")
            )
        ).match());
}

//TEST_F(TestIndicatorNetworkService, FlightModeTalksToURfkill)
//{
//    startIndicator();
//
//    auto& urfkillInterface = dbusMock.urfkillInterface();
//    QSignalSpy urfkillSpy(&urfkillInterface, SIGNAL(FlightModeChanged(bool)));
//
//    EXPECT_MATCHRESULT(mh::MenuMatcher(phoneParameters())
//        .item(mh::MenuItemMatcher()
//            .mode(mh::MenuItemMatcher::Mode::starts_with)
//            .item(mh::MenuItemMatcher::checkbox()
//                .action("indicator.airplane.enabled")
//                .toggled(false)
//                .activate() // <--- Activate the action now
//            )
//        ).match());
//
//    // Wait to be notified that flight mode was enabled
//    ASSERT_TRUE(urfkillSpy.wait());
//    EXPECT_EQ(urfkillSpy.first(), QVariantList() << QVariant(true));
//}

//TEST_F(TestIndicatorNetworkService, IndicatorListensToURfkill)
//{
//    startIndicator();
//
//    EXPECT_MATCHRESULT(mh::MenuMatcher(phoneParameters())
//        .item(mh::MenuItemMatcher()
//            .label("")
//            .action("indicator.phone.network-status")
//            .mode(mh::MenuItemMatcher::Mode::starts_with)
//            .item(mh::MenuItemMatcher::checkbox()
//                .action("indicator.airplane.enabled")
//                .toggled(false)
//            )
//        ).match());
//
//    ASSERT_TRUE(dbusMock.urfkillInterface().FlightMode(true));
//
//    EXPECT_MATCHRESULT(mh::MenuMatcher(phoneParameters())
//        .item(mh::MenuItemMatcher()
//            .label("")
//            .action("indicator.phone.network-status")
//            .mode(mh::MenuItemMatcher::Mode::starts_with)
//            .item(mh::MenuItemMatcher::checkbox()
//                .action("indicator.airplane.enabled")
//                .toggled(true)
//            )
//        ).match());
//}

} // namespace
