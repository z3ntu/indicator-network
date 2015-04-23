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
 * Author: Pete Woods <pete.woods@canonical.com>
 */

#include <indicator-network-test-base.h>
#include <connectivityqt/connectivity.h>

#include <QDebug>
#include <QTestEventLoop>
#include <QSignalSpy>

using namespace std;
using namespace testing;
using namespace connectivityqt;

namespace
{

class TestConnectivityApi: public IndicatorNetworkTestBase
{
protected:
    Connectivity::UPtr newConnectivity()
    {
        auto connectivity = make_unique<Connectivity>(dbusTestRunner.sessionConnection());

        if (!connectivity->isInitialized())
        {
            QSignalSpy initSpy(connectivity.get(), SIGNAL(initialized()));
            initSpy.wait();
        }

        return connectivity;
    }
};

TEST_F(TestConnectivityApi, FollowsFlightMode)
{
    // Set up disconnected with flight mode on
    setGlobalConnectedState(NM_STATE_DISCONNECTED);
    ASSERT_TRUE(dbusMock.urfkillInterface().FlightMode(true));

    // Start the indicator
    ASSERT_NO_THROW(startIndicator());

    // Connect to the service
    auto connectivity(newConnectivity());

    // Check flight mode is enabled
    EXPECT_TRUE(connectivity->flightMode());

    // Now disable flight mode
    ASSERT_TRUE(dbusMock.urfkillInterface().FlightMode(false));

    // Check that flight mode gets updated
    {
        QSignalSpy spy(connectivity.get(), SIGNAL(flightModeUpdated(bool)));
        ASSERT_TRUE(spy.wait());
        ASSERT_EQ(1, spy.size());
    }
    // Check that flight mode is disabled
    EXPECT_FALSE(connectivity->flightMode());
}

TEST_F(TestConnectivityApi, FlightModeTalksToURfkill)
{
    // Start the indicator
    ASSERT_NO_THROW(startIndicator());

    // Connect to the UrfKill mock
    auto& urfkillInterface = dbusMock.urfkillInterface();
    QSignalSpy urfkillSpy(&urfkillInterface, SIGNAL(FlightModeChanged(bool)));

    // Connect the the service
    auto connectivity(newConnectivity());

    // Follow the unstoppableOperationHappening property
    QSignalSpy operationSpy(connectivity.get(), SIGNAL(unstoppableOperationHappeningUpdated(bool)));

    // Check that nothing is happening yet
    EXPECT_FALSE(connectivity->unstoppableOperationHappening());

    // Enable flight mode
    connectivity->setFlightMode(true);

    // We should first get the unstoppable operation change
    ASSERT_TRUE(operationSpy.wait());
    ASSERT_EQ(1, operationSpy.size());
    EXPECT_EQ(operationSpy.first(), QVariantList() << QVariant(true));

    // Wait to be notified that flight mode was enabled
    if (urfkillSpy.size() != 1)
    {
        ASSERT_TRUE(urfkillSpy.wait());
    }
    ASSERT_EQ(1, urfkillSpy.size());
    EXPECT_EQ(urfkillSpy.first(), QVariantList() << QVariant(true));

    // The unstoppable operation should complete
    if (operationSpy.size() != 2)
    {
        ASSERT_TRUE(operationSpy.wait());
    }
    ASSERT_EQ(2, operationSpy.size());
    EXPECT_EQ(operationSpy.last(), QVariantList() << QVariant(false));

    // Start again
    urfkillSpy.clear();
    operationSpy.clear();

    // Check that nothing is happening again
    EXPECT_FALSE(connectivity->unstoppableOperationHappening());

    // Disable flight mode
    connectivity->setFlightMode(false);

    // We should first get the unstoppable operation change
    ASSERT_TRUE(operationSpy.wait());
    ASSERT_EQ(1, operationSpy.size());
    EXPECT_EQ(operationSpy.first(), QVariantList() << QVariant(true));

    // Wait to be notified that flight mode was disabled
    if (urfkillSpy.size() != 1)
    {
        ASSERT_TRUE(urfkillSpy.wait());
    }
    ASSERT_EQ(1, urfkillSpy.size());
    EXPECT_EQ(urfkillSpy.first(), QVariantList() << QVariant(false));

    // The unstoppable operation should complete
    if (operationSpy.size() != 2)
    {
        ASSERT_TRUE(operationSpy.wait());
    }
    ASSERT_EQ(2, operationSpy.size());
    EXPECT_EQ(operationSpy.last(), QVariantList() << QVariant(false));

    // Check that nothing is happening again
    EXPECT_FALSE(connectivity->unstoppableOperationHappening());
}

}
