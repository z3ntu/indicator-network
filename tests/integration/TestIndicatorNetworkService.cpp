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
#include <NetworkManager.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace std;
using namespace testing;
using namespace QtDBusTest;
using namespace QtDBusMock;

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

    void TearDown() override
    {
        sleep(1); // FIXME delete this line when the indicator shuts down stably
    }

    DBusTestRunner dbusTestRunner;

    DBusMock dbusMock;
};

TEST_F(TestIndicatorNetworkService, Foo)
{
}

} // namespace
