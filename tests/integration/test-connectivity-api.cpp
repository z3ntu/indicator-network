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
        return make_unique<Connectivity>(dbusTestRunner.sessionConnection());
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

    // Check that flight mode gets updated
    if (!connectivity->flightMode())
    {
        QSignalSpy spy(connectivity.get(), SIGNAL(flightModeUpdated(bool)));
        WAIT_FOR_SIGNALS(spy, 1);
    }
    // Check flight mode is enabled
    EXPECT_TRUE(connectivity->flightMode());

    // Now disable flight mode
    ASSERT_TRUE(dbusMock.urfkillInterface().FlightMode(false));

    // Check that flight mode gets updated
    {
        QSignalSpy spy(connectivity.get(), SIGNAL(flightModeUpdated(bool)));
        WAIT_FOR_SIGNALS(spy, 1);
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

    // Enable flight mode
    connectivity->setFlightMode(true);

    // Wait to be notified that flight mode was enabled
    WAIT_FOR_SIGNALS(urfkillSpy, 1);
    EXPECT_EQ(urfkillSpy.first(), QVariantList() << QVariant(true));

    // Start again
    urfkillSpy.clear();

    // Disable flight mode
    connectivity->setFlightMode(false);

    // Wait to be notified that flight mode was disabled
    WAIT_FOR_SIGNALS(urfkillSpy, 1);
    EXPECT_EQ(urfkillSpy.first(), QVariantList() << QVariant(false));
}

}
