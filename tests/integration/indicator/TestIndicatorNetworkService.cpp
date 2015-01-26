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

    DBusTestRunner dbusTestRunner;

    DBusMock dbusMock;
};

TEST_F(TestIndicatorNetworkService, Foo)
{
    auto& urfkillInterface = dbusMock.mockInterface("org.freedesktop.URfkill",
                                                   "/org/freedesktop/URfkill",
                                                   "org.freedesktop.URfkill",
                                                   QDBusConnection::SystemBus);

    QSignalSpy urfkillSpy(&urfkillInterface, SIGNAL(MethodCalled(const QString &, const QVariantList &)));

    mh::MenuMatcher::Parameters parameters("com.canonical.indicator.network",
                                           {{ "indicator", "/com/canonical/indicator/network" }},
                                           "/com/canonical/indicator/network/phone");

    EXPECT_MATCHRESULT(mh::MenuMatcher(parameters)
        .item(mh::MenuItemMatcher()
            .label("")
            .action("indicator.phone.network-status")
            .mode(mh::MenuItemMatcher::Mode::starts_with)
            .item(mh::MenuItemMatcher::checkbox()
                .label("Flight Mode")
                .action("indicator.airplane.enabled")
                .icon("")
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
            )
            .item(mh::MenuItemMatcher()
                .label("Wi-Fi settings…")
                .action("indicator.wifi.settings")
            )
        ).match());

    EXPECT_MATCHRESULT(mh::MenuMatcher(parameters)
        .item(mh::MenuItemMatcher()
            .mode(mh::MenuItemMatcher::Mode::starts_with)
            .item(mh::MenuItemMatcher::checkbox()
                .action("indicator.airplane.enabled")
                .activate()
            )
        ).match());

    ASSERT_TRUE(urfkillSpy.wait());

    QList<QtDBusMock::MethodCall> calls = urfkillInterface.GetMethodCalls("FlightMode");
    ASSERT_EQ(calls.size(), 1);
    EXPECT_EQ(calls.first().args(), QVariantList() << QVariant(true));
}

} // namespace

#include "TestIndicatorNetworkService.moc"
