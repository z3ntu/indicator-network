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
        dbusMock.registerOfono();
        dbusMock.registerURfkill();

        dbusTestRunner.registerService(
                DBusServicePtr(
                        new QProcessDBusService(
                                "com.canonical.indicator.network",
                                QDBusConnection::SessionBus,
                                NETWORK_SERVICE_BIN,
                                QStringList())));

        dbusTestRunner.startServices();
    }

    static mh::MenuMatcher::Parameters phoneParameters()
    {
        return mh::MenuMatcher::Parameters(
                "com.canonical.indicator.network",
                { { "indicator", "/com/canonical/indicator/network" } },
                "/com/canonical/indicator/network/phone");
    }

    DBusTestRunner dbusTestRunner;

    DBusMock dbusMock;
};

TEST_F(TestIndicatorNetworkService, BasicMenuContents)
{
    EXPECT_MATCHRESULT(mh::MenuMatcher(phoneParameters())
        .item(mh::MenuItemMatcher()
            .label("")
            .action("indicator.phone.network-status")
            .mode(mh::MenuItemMatcher::Mode::starts_with)
            .item(mh::MenuItemMatcher::checkbox()
                .label("Flight Mode")
                .action("indicator.airplane.enabled")
                .icon("")
                .toggled(false)
            )
            .item(mh::MenuItemMatcher::separator())
            .item(mh::MenuItemMatcher()
                .widget("com.canonical.indicator.network.modeminfoitem")
                .action("")
                .icon("")
            )
            .item(mh::MenuItemMatcher()
                .label("Cellular settings…")
                .action("indicator.cellular.settings")
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

TEST_F(TestIndicatorNetworkService, FlightModeTalksToURfkill)
{
    auto& urfkillInterface = dbusMock.urfkillInterface();
    QSignalSpy urfkillSpy(&urfkillInterface, SIGNAL(FlightModeChanged(bool)));

    EXPECT_MATCHRESULT(mh::MenuMatcher(phoneParameters())
        .item(mh::MenuItemMatcher()
            .mode(mh::MenuItemMatcher::Mode::starts_with)
            .item(mh::MenuItemMatcher::checkbox()
                .action("indicator.airplane.enabled")
                .toggled(false)
            )
        ).match());

    EXPECT_MATCHRESULT(mh::MenuMatcher(phoneParameters())
        .item(mh::MenuItemMatcher()
            .mode(mh::MenuItemMatcher::Mode::starts_with)
            .item(mh::MenuItemMatcher::checkbox()
                .action("indicator.airplane.enabled")
                .activate() // <--- Activate the action now
            )
        ).match());

    // Wait to be notified that flight mode was enabled
    ASSERT_TRUE(urfkillSpy.wait());
    EXPECT_EQ(urfkillSpy.first(), QVariantList() << QVariant(true));
}

TEST_F(TestIndicatorNetworkService, IndicatorListensToURfkill)
{
    EXPECT_MATCHRESULT(mh::MenuMatcher(phoneParameters())
        .item(mh::MenuItemMatcher()
            .label("")
            .action("indicator.phone.network-status")
            .mode(mh::MenuItemMatcher::Mode::starts_with)
            .item(mh::MenuItemMatcher::checkbox()
                .action("indicator.airplane.enabled")
                .toggled(false)
            )
        ).match());

    ASSERT_TRUE(dbusMock.urfkillInterface().FlightMode(true));

    EXPECT_MATCHRESULT(mh::MenuMatcher(phoneParameters())
        .item(mh::MenuItemMatcher()
            .label("")
            .action("indicator.phone.network-status")
            .mode(mh::MenuItemMatcher::Mode::starts_with)
            .item(mh::MenuItemMatcher::checkbox()
                .action("indicator.airplane.enabled")
                .toggled(true)
            )
        ).match());
}

} // namespace

#include "TestIndicatorNetworkService.moc"
