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
#include <dbus-types.h>
#include <NetworkManagerSettingsInterface.h>

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
        Connectivity::registerMetaTypes();
        auto connectivity = make_unique<Connectivity>(dbusTestRunner.sessionConnection());

        if (!connectivity->isInitialized())
        {
            QSignalSpy initSpy(connectivity.get(), SIGNAL(initialized()));
            initSpy.wait();
        }

        return connectivity;
    }

    QVariantList getMethodCall(const QSignalSpy& spy, const QString& method)
    {
        for(const auto& call: spy)
        {
            if (call.first().toString() == method)
            {
                return call.at(1).toList();
            }
        }
        throw domain_error(qPrintable("No method call [" + method + "] could be found"));
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

    // Follow the flightMode property
    QSignalSpy flightModeSpy(connectivity.get(), SIGNAL(flightModeUpdated(bool)));

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

    // Wait for flight mode property change
    if (flightModeSpy.size() != 1)
    {
        ASSERT_TRUE(flightModeSpy.wait());
    }
    ASSERT_EQ(1, flightModeSpy.size());
    EXPECT_EQ(flightModeSpy.first(), QVariantList() << QVariant(true));

    // Check that nothing is happening again
    EXPECT_FALSE(connectivity->unstoppableOperationHappening());

    // The icing on the cake
    EXPECT_TRUE(connectivity->flightMode());

    // Start again
    urfkillSpy.clear();
    operationSpy.clear();
    flightModeSpy.clear();

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

    // Wait for flight mode property change
    if (flightModeSpy.size() != 1)
    {
        ASSERT_TRUE(flightModeSpy.wait());
    }
    ASSERT_EQ(1, flightModeSpy.size());
    EXPECT_EQ(flightModeSpy.first(), QVariantList() << QVariant(false));

    // Check that nothing is happening again
    EXPECT_FALSE(connectivity->unstoppableOperationHappening());

    // The icing on the cake
    EXPECT_FALSE(connectivity->flightMode());
}

TEST_F(TestConnectivityApi, WifiToggleTalksToUrfkill)
{
    // Start the indicator
    ASSERT_NO_THROW(startIndicator());

    // Connect to the UrfKill mock
    OrgFreedesktopURfkillKillswitchInterface wifiKillswitchInterface(
            DBusTypes::URFKILL_BUS_NAME, DBusTypes::URFKILL_WIFI_OBJ_PATH,
            dbusTestRunner.systemConnection());
    QSignalSpy urfkillSpy(&wifiKillswitchInterface, SIGNAL(StateChanged()));

    // Connect the the service
    auto connectivity(newConnectivity());

    // Follow the wifiEnabled property
    QSignalSpy wifiEnabledSpy(connectivity.get(), SIGNAL(wifiEnabledUpdated(bool)));

    // Follow the unstoppableOperationHappening property
    QSignalSpy operationSpy(connectivity.get(), SIGNAL(unstoppableOperationHappeningUpdated(bool)));

    // Check that nothing is happening yet
    EXPECT_FALSE(connectivity->unstoppableOperationHappening());
    EXPECT_TRUE(connectivity->wifiEnabled());
    EXPECT_EQ(0, wifiKillswitchInterface.state());

    // Disable WiFi
    connectivity->setwifiEnabled(false);

    // We should first get the unstoppable operation change
    ASSERT_TRUE(operationSpy.wait());
    ASSERT_EQ(1, operationSpy.size());
    EXPECT_EQ(operationSpy.first(), QVariantList() << QVariant(true));

    // Wait to be notified that wifi was toggled
    if (urfkillSpy.size() != 1)
    {
        ASSERT_TRUE(urfkillSpy.wait());
    }
    ASSERT_EQ(1, urfkillSpy.size());

    // The unstoppable operation should complete
    if (operationSpy.size() != 2)
    {
        ASSERT_TRUE(operationSpy.wait());
    }
    ASSERT_EQ(2, operationSpy.size());
    EXPECT_EQ(operationSpy.last(), QVariantList() << QVariant(false));

    // Wait for wifi enabled property change
    if (wifiEnabledSpy.size() != 1)
    {
        ASSERT_TRUE(wifiEnabledSpy.wait());
    }
    ASSERT_EQ(1, wifiEnabledSpy.size());
    EXPECT_EQ(wifiEnabledSpy.first(), QVariantList() << QVariant(false));

    // Check that nothing is happening again
    EXPECT_FALSE(connectivity->unstoppableOperationHappening());

    // The icing on the cake
    EXPECT_FALSE(connectivity->wifiEnabled());
    EXPECT_EQ(1, wifiKillswitchInterface.state());

    // Start again
    urfkillSpy.clear();
    operationSpy.clear();
    wifiEnabledSpy.clear();

    // Disable flight mode
    connectivity->setwifiEnabled(true);

    // We should first get the unstoppable operation change
    ASSERT_TRUE(operationSpy.wait());
    ASSERT_EQ(1, operationSpy.size());
    EXPECT_EQ(operationSpy.first(), QVariantList() << QVariant(true));

    // Wait to be notified that wifi was toggled
    if (urfkillSpy.size() != 1)
    {
        ASSERT_TRUE(urfkillSpy.wait());
    }
    ASSERT_EQ(1, urfkillSpy.size());

    // The unstoppable operation should complete
    if (operationSpy.size() != 2)
    {
        ASSERT_TRUE(operationSpy.wait());
    }
    ASSERT_EQ(2, operationSpy.size());
    EXPECT_EQ(operationSpy.last(), QVariantList() << QVariant(false));

    // Wait for wifi enabled property change
    if (wifiEnabledSpy.size() != 1)
    {
        ASSERT_TRUE(wifiEnabledSpy.wait());
    }
    ASSERT_EQ(1, wifiEnabledSpy.size());
    EXPECT_EQ(wifiEnabledSpy.first(), QVariantList() << QVariant(true));

    // Check that nothing is happening again
    EXPECT_FALSE(connectivity->unstoppableOperationHappening());

    // The icing on the cake
    EXPECT_TRUE(connectivity->wifiEnabled());
    EXPECT_EQ(0, wifiKillswitchInterface.state());
}

TEST_F(TestConnectivityApi, Status)
{
    // Start the indicator
    ASSERT_NO_THROW(startIndicator());

    // Connect the the service
    auto connectivity(newConnectivity());

    // Check we are online
    EXPECT_TRUE(connectivity->online());
    EXPECT_EQ(Connectivity::Status::Online, connectivity->status());

    // Listen to property changes
    QSignalSpy onlineSpy(connectivity.get(), SIGNAL(onlineUpdated(bool)));
    QSignalSpy statusSpy(connectivity.get(), SIGNAL(statusUpdated(connectivityqt::Connectivity::Status)));

    // Set up disconnected with flight mode on
    setGlobalConnectedState(NM_STATE_DISCONNECTED);

    // Wait for changes
    ASSERT_TRUE(onlineSpy.wait());
    if (statusSpy.isEmpty())
    {
        ASSERT_TRUE(statusSpy.wait());
    }

    // Check we are online
    EXPECT_FALSE(connectivity->online());
    EXPECT_EQ(Connectivity::Status::Offline, connectivity->status());

    // Check we have just one signal
    ASSERT_EQ(1, onlineSpy.size());
    ASSERT_EQ(1, statusSpy.size());

    EXPECT_EQ(QVariantList() << QVariant(false), onlineSpy.first());
    EXPECT_EQ(Connectivity::Status::Offline, qvariant_cast<Connectivity::Status>(statusSpy.first().first()));
}

TEST_F(TestConnectivityApi, Limitations)
{
    // Start the indicator
    ASSERT_NO_THROW(startIndicator());

    // Connect the the service
    auto connectivity(newConnectivity());

    // These methods will always return these values - i.e. are not implemented
    EXPECT_EQ(QVector<Connectivity::Limitations>(), connectivity->limitations());
    EXPECT_FALSE(connectivity->limitedBandwith());
}

TEST_F(TestConnectivityApi, HotspotConfig)
{
    setGlobalConnectedState(NM_STATE_DISCONNECTED);
    auto device = createWiFiDevice(NM_DEVICE_STATE_DISCONNECTED);

    // Start the indicator
    ASSERT_NO_THROW(startIndicator());

    auto& powerdMock = dbusMock.mockInterface(DBusTypes::POWERD_DBUS_NAME,
                           DBusTypes::POWERD_DBUS_PATH,
                           DBusTypes::POWERD_DBUS_INTERFACE,
                           QDBusConnection::SystemBus);
    QSignalSpy powerdMockCallSpy(
                           &powerdMock,
                           SIGNAL(MethodCalled(const QString &, const QVariantList &)));

    auto& nmSettingsMock = dbusMock.mockInterface(NM_DBUS_SERVICE,
                           NM_DBUS_PATH_SETTINGS,
                           NM_DBUS_IFACE_SETTINGS,
                           QDBusConnection::SystemBus);
    QSignalSpy nmSettingsMockCallSpy(
                           &nmSettingsMock,
                           SIGNAL(MethodCalled(const QString &, const QVariantList &)));

    OrgFreedesktopNetworkManagerSettingsInterface settings(
            NM_DBUS_SERVICE, NM_DBUS_PATH_SETTINGS, dbusTestRunner.systemConnection());
    QSignalSpy settingsNewConnectionSpy(
                           &settings,
                           SIGNAL(NewConnection(const QDBusObjectPath &)));

    // Connect the the service
    auto connectivity(newConnectivity());

    QSignalSpy storedSpy(connectivity.get(), SIGNAL(hotspotStoredUpdated(bool)));
    QSignalSpy enabledSpy(connectivity.get(), SIGNAL(hotspotEnabledUpdated(bool)));
    QSignalSpy passwordSpy(connectivity.get(), SIGNAL(hotspotPasswordUpdated(const QString&)));

    EXPECT_EQ("Ubuntu", connectivity->hotspotSsid().toStdString());
    EXPECT_FALSE(connectivity->hotspotStored());
    EXPECT_FALSE(connectivity->hotspotEnabled());

    connectivity->setHotspotPassword("the password");

    if (passwordSpy.empty())
    {
        ASSERT_TRUE(passwordSpy.wait());
    }
    EXPECT_FALSE(passwordSpy.empty());
    EXPECT_EQ("the password", connectivity->hotspotPassword().toStdString());

    connectivity->setHotspotEnabled(true);

    if (enabledSpy.empty())
    {
        ASSERT_TRUE(enabledSpy.wait());
    }
    EXPECT_FALSE(enabledSpy.empty());

    if (storedSpy.empty())
    {
        ASSERT_TRUE(storedSpy.wait());
    }
    EXPECT_FALSE(storedSpy.empty());

    if (powerdMockCallSpy.empty())
    {
        ASSERT_TRUE(powerdMockCallSpy.wait());
    }
    EXPECT_FALSE(powerdMockCallSpy.empty());

    if (nmSettingsMockCallSpy.empty())
    {
        ASSERT_TRUE(nmSettingsMockCallSpy.wait());
    }
    EXPECT_FALSE(nmSettingsMockCallSpy.empty());

    if (settingsNewConnectionSpy.empty())
    {
        ASSERT_TRUE(settingsNewConnectionSpy.wait());
    }
    EXPECT_FALSE(settingsNewConnectionSpy.empty());

    EXPECT_TRUE(connectivity->hotspotEnabled());
    EXPECT_TRUE(connectivity->hotspotStored());

    // Connect to method calls on the newly added connection
    auto connectionPath = qvariant_cast<QDBusObjectPath>(settingsNewConnectionSpy.first().first());
    auto& connectionSettingsMock = dbusMock.mockInterface(NM_DBUS_SERVICE, connectionPath.path(),
                           NM_DBUS_IFACE_SETTINGS_CONNECTION,
                           QDBusConnection::SystemBus);
    QSignalSpy connectionSettingsMockCallSpy(
                           &connectionSettingsMock,
                           SIGNAL(MethodCalled(const QString &, const QVariantList &)));

    // Expect a wakelock request
    EXPECT_EQ(
        QVariantList({"connectivity-service", int(1)}),
        getMethodCall(powerdMockCallSpy, "requestSysState"));

    {
        auto call = getMethodCall(nmSettingsMockCallSpy, "AddConnection");
        // Decode the QDBusArgument
        QVariantDictMap connection;
        qvariant_cast<QDBusArgument>(call.first()) >> connection;
        EXPECT_EQ(QVariantMap({
            {"mode", "ap"},
            {"security", "802-11-wireless-security"},
            {"ssid", "Ubuntu"}
        }), connection["802-11-wireless"]);
        EXPECT_EQ(QVariantMap({
            {"group", "ccmp"},
            {"key-mgmt", "wpa-psk"},
            {"pairwise" ,  QStringList{"ccmp"}},
            {"proto" ,  QStringList{"rsn"}},
            {"psk", "the password"}
        }), connection["802-11-wireless-security"]);
        EXPECT_TRUE(connection["connection"]["autoconnect"].toBool());
    }

    // Next we'll disable the hotspot
    storedSpy.clear();
    enabledSpy.clear();
    passwordSpy.clear();
    powerdMockCallSpy.clear();
    nmSettingsMockCallSpy.clear();

    connectivity->setHotspotEnabled(false);

    if (powerdMockCallSpy.empty())
    {
        ASSERT_TRUE(powerdMockCallSpy.wait());
    }
    EXPECT_FALSE(powerdMockCallSpy.empty());

    if (connectionSettingsMockCallSpy.empty())
    {
        ASSERT_TRUE(connectionSettingsMockCallSpy.wait());
    }
    EXPECT_FALSE(connectionSettingsMockCallSpy.empty());

    if (enabledSpy.empty())
    {
        ASSERT_TRUE(enabledSpy.wait());
    }
    EXPECT_FALSE(enabledSpy.empty());

    EXPECT_FALSE(connectivity->hotspotEnabled());
    EXPECT_TRUE(connectivity->hotspotStored());

    // Expect a wakelock cancel
    EXPECT_EQ(
        QVariantList{"dummy_cookie"},
        getMethodCall(powerdMockCallSpy, "clearSysState"));

    // The connection should no-longer auto-connect
    {
        auto call = getMethodCall(connectionSettingsMockCallSpy, "Update");
        // Decode the QDBusArgument
        QVariantDictMap connection;
        qvariant_cast<QDBusArgument>(call.first()) >> connection;
        EXPECT_FALSE(connection["connection"]["autoconnect"].toBool());
    }
}

}
