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

#include <menuharness/MatchUtils.h>
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
enum class Secure
{
    secure,
    insecure
};

enum class ApMode
{
    infra,
    adhoc
};

enum class ConnectionStatus
{
    connected,
    disconnected
};

class TestIndicatorNetworkService : public Test
{
protected:
    TestIndicatorNetworkService() :
            dbusMock(dbusTestRunner)
    {
    }

    void SetUp() override
    {
//        dbusTestRunner.registerService(
//                DBusServicePtr(
//                        new QProcessDBusService(
//                                "", QDBusConnection::SessionBus,
//                                "/usr/bin/bustle-pcap",
//                                QStringList{"-e", "/tmp/bustle-session.log"})));
//        dbusTestRunner.registerService(
//                DBusServicePtr(
//                        new QProcessDBusService(
//                                "", QDBusConnection::SystemBus,
//                                "/usr/bin/bustle-pcap",
//                                QStringList{"-y", "/tmp/bustle-system.log"})));

        dbusMock.registerNetworkManager();
        // By default the ofono mock starts with one modem
        dbusMock.registerOfono();
        dbusMock.registerURfkill();

        dbusTestRunner.startServices();
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

    QString createWiFiDevice(int state, const QString& id = "0")
    {
        auto& networkManager(dbusMock.networkManagerInterface());
        auto reply = networkManager.AddWiFiDevice(id, "eth1", state);
        reply.waitForFinished();
        return reply;
    }

    QString createAccessPoint(const QString& id, const QString& ssid, const QString& device)
    {
        auto& networkManager(dbusMock.networkManagerInterface());
        auto reply = networkManager.AddAccessPoint(
                            device, id, ssid,
                            "11:22:33:44:55:66", NM_802_11_MODE_INFRA, 0, 0, 's',
                            NM_802_11_AP_SEC_KEY_MGMT_PSK);
        reply.waitForFinished();
        return reply;
    }

    QString createAccessPointConnection(const QString& id, const QString& ssid, const QString& device)
    {
        auto& networkManager(dbusMock.networkManagerInterface());
        auto reply = networkManager.AddWiFiConnection(device, id, ssid,
                                                      "");
        reply.waitForFinished();
        return reply;
    }

    QString createActiveConnection(const QString& id, const QString& device, const QString& connection, const QString& ap)
    {
        auto& nm = dbusMock.networkManagerInterface();
        auto reply = nm.AddActiveConnection(QStringList() << device,
                               connection,
                               ap,
                               id,
                               NM_ACTIVE_CONNECTION_STATE_ACTIVATED);
        reply.waitForFinished();
        return reply;
    }

    void setGlobalConnectedState(int state)
    {
        auto& nm = dbusMock.networkManagerInterface();
        nm.SetGlobalConnectionState(state).waitForFinished();
    }

    QString createModem(const QString& id)
    {
        auto& ofono(dbusMock.ofonoInterface());
        QVariantMap modemProperties {{ "Powered", false } };
        return ofono.AddModem(id, modemProperties);
    }

    template<typename T>
    void setSimManagerProperty(int modemIndex, const QString& propertyName, const T& value)
    {
        auto& ofono(dbusMock.ofonoSimManagerInterface(modemIndex));
        ofono.SetProperty(propertyName, QDBusVariant(value));
    }

    template<typename T>
    void setNetworkRegistrationProperty(int modemIndex, const QString& propertyName, const T& value)
    {
        auto& ofono(dbusMock.ofonoNetworkRegistrationInterface(modemIndex));
        ofono.SetProperty(propertyName, QDBusVariant(value));
    }

    void setNetworkRegistrationProperty(int modemIndex, const QString& propertyName, const uchar& value)
    {
        auto& ofono(dbusMock.ofonoNetworkRegistrationInterface(modemIndex));
        ofono.SetProperty(propertyName, value);
    }

    static mh::MenuItemMatcher flightModeSwitch(bool toggled = false)
    {
        return mh::MenuItemMatcher::checkbox()
            .label("Flight Mode")
            .action("indicator.airplane.enabled")
            .toggled(toggled);
    }

    static mh::MenuItemMatcher accessPoint(const string& ssid, unsigned int id, Secure secure,
                ApMode apMode, ConnectionStatus connectionStatus)
    {
        return mh::MenuItemMatcher::checkbox()
            .label(ssid)
            .widget("unity.widgets.systemsettings.tablet.accesspoint")
            .action("indicator.accesspoint." + to_string(id))
            .toggled(connectionStatus == ConnectionStatus::connected)
            .pass_through_attribute(
                "x-canonical-wifi-ap-strength-action",
                shared_ptr<GVariant>(g_variant_new_byte(0x73), &mh::gvariant_deleter))
            .boolean_attribute("x-canonical-wifi-ap-is-secure", secure == Secure::secure)
            .boolean_attribute("x-canonical-wifi-ap-is-adhoc", apMode == ApMode::adhoc);
    }

    static mh::MenuItemMatcher wifiEnableSwitch(bool toggled = true)
    {
        return mh::MenuItemMatcher::checkbox()
            .label("Wi-Fi")
            .action("indicator.wifi.enable") // This action is accessed by system-settings-ui, do not change it
            .toggled(toggled);
    }

    static mh::MenuItemMatcher wifiSettings()
    {
        return mh::MenuItemMatcher()
            .label("Wi-Fi settings…")
            .action("indicator.wifi.settings");
    }

    static mh::MenuItemMatcher modemInfo(const string& simIdentifier, const string& label, const string& statusIcon, bool locked = false)
    {
        return mh::MenuItemMatcher()
            .widget("com.canonical.indicator.network.modeminfoitem")
            .pass_through_string_attribute("x-canonical-modem-sim-identifier-label-action", simIdentifier)
            .pass_through_string_attribute("x-canonical-modem-connectivity-icon-action", "")
            .pass_through_string_attribute("x-canonical-modem-status-label-action", label)
            .pass_through_string_attribute("x-canonical-modem-status-icon-action", statusIcon)
            .pass_through_boolean_attribute("x-canonical-modem-roaming-action", false)
            .pass_through_boolean_attribute("x-canonical-modem-locked-action", locked);
    }

    static mh::MenuItemMatcher cellularSettings()
    {
        return mh::MenuItemMatcher()
            .label("Cellular settings…")
            .action("indicator.cellular.settings");
    }

    DBusTestRunner dbusTestRunner;

    DBusMock dbusMock;

    DBusServicePtr indicator;
};

TEST_F(TestIndicatorNetworkService, BasicMenuContents)
{
    setGlobalConnectedState(NM_STATE_DISCONNECTED);
    ASSERT_NO_THROW(startIndicator());

    EXPECT_MATCHRESULT(mh::MenuMatcher(phoneParameters())
        .item(mh::MenuItemMatcher()
            .action("indicator.phone.network-status")
            .state_icons({"gsm-3g-full", "nm-no-connection"})
            .mode(mh::MenuItemMatcher::Mode::all)
            .submenu()
            .item(flightModeSwitch())
            .item(mh::MenuItemMatcher()
                .section()
                .item(modemInfo("", "fake.tel", "gsm-3g-full"))
                .item(cellularSettings())
            )
            .item(wifiEnableSwitch())
            .item(wifiSettings())
        ).match());
}

TEST_F(TestIndicatorNetworkService, OneDisconnectedAccessPointAtStartup)
{
    setGlobalConnectedState(NM_STATE_DISCONNECTED);
    auto device = createWiFiDevice(NM_DEVICE_STATE_DISCONNECTED);
    auto ap = createAccessPoint("0", "the ssid", device);
    auto connection = createAccessPointConnection("0", "the ssid", device);

    ASSERT_NO_THROW(startIndicator());

    EXPECT_MATCHRESULT(mh::MenuMatcher(phoneParameters())
        .item(mh::MenuItemMatcher()
            .state_icons({"gsm-3g-full", "nm-no-connection"})
            .submenu()
            .item(flightModeSwitch())
            .item(mh::MenuItemMatcher()) // <-- modems are under here
            .item(wifiEnableSwitch())
            .item(mh::MenuItemMatcher()
                .section()
                .item(accessPoint("the ssid", 1,
                      Secure::secure,
                      ApMode::infra,
                      ConnectionStatus::disconnected)
                )
            )
            .item(wifiSettings())
        ).match());
}

TEST_F(TestIndicatorNetworkService, OneConnectedAccessPointAtStartup)
{
    setGlobalConnectedState(NM_STATE_CONNECTED_GLOBAL);
    auto device = createWiFiDevice(NM_DEVICE_STATE_ACTIVATED);
    auto ap = createAccessPoint("0", "the ssid", device);
    auto connection = createAccessPointConnection("0", "the ssid", device);
    createActiveConnection("0", device, connection, ap);

    ASSERT_NO_THROW(startIndicator());

    EXPECT_MATCHRESULT(mh::MenuMatcher(phoneParameters())
        .item(mh::MenuItemMatcher()
            .state_icons({"gsm-3g-full", "nm-signal-100-secure"})
            .submenu()
            .item(flightModeSwitch())
            .item(mh::MenuItemMatcher()) // <-- modems are under here
            .item(wifiEnableSwitch())
            .item(mh::MenuItemMatcher()
                .section()
                .item(accessPoint("the ssid", 1,
                      Secure::secure,
                      ApMode::infra,
                      ConnectionStatus::connected)
                )
            )
            .item(wifiSettings())
        ).match());
}

TEST_F(TestIndicatorNetworkService, AddOneDisconnectedAccessPointAfterStartup)
{
    setGlobalConnectedState(NM_STATE_DISCONNECTED);
    auto device = createWiFiDevice(NM_DEVICE_STATE_DISCONNECTED);

    ASSERT_NO_THROW(startIndicator());
    auto ap = createAccessPoint("0", "the ssid", device);
    auto connection = createAccessPointConnection("0", "the ssid", device);

    EXPECT_MATCHRESULT(mh::MenuMatcher(phoneParameters())
        .item(mh::MenuItemMatcher()
            .submenu()
            .item(flightModeSwitch())
            .item(mh::MenuItemMatcher())
            .item(wifiEnableSwitch())
            .item(mh::MenuItemMatcher()
                .section()
                .item(accessPoint("the ssid", 1,
                      Secure::secure,
                      ApMode::infra,
                      ConnectionStatus::disconnected)
                )
            )
            .item(wifiSettings())
        ).match());
}

TEST_F(TestIndicatorNetworkService, AddOneConnectedAccessPointAfterStartup)
{
    setGlobalConnectedState(NM_STATE_DISCONNECTED);
    auto device = createWiFiDevice(NM_DEVICE_STATE_DISCONNECTED);

    ASSERT_NO_THROW(startIndicator());

    auto ap = createAccessPoint("0", "the ssid", device);
    auto connection = createAccessPointConnection("0", "the ssid", device);
    createActiveConnection("0", device, connection, ap);
    setGlobalConnectedState(NM_STATE_CONNECTED_GLOBAL);

    EXPECT_MATCHRESULT(mh::MenuMatcher(phoneParameters())
        .item(mh::MenuItemMatcher()
            .submenu()
            .item(flightModeSwitch())
            .item(mh::MenuItemMatcher())
            .item(wifiEnableSwitch())
            .item(mh::MenuItemMatcher()
                .section()
                .item(accessPoint("the ssid", 1,
                      Secure::secure,
                      ApMode::infra,
                      ConnectionStatus::connected)
                )
            )
            .item(wifiSettings())
        ).match());
}

TEST_F(TestIndicatorNetworkService, SecondModem)
{
    createModem("ril_1"); // ril_0 already exists
    ASSERT_NO_THROW(startIndicator());

    EXPECT_MATCHRESULT(mh::MenuMatcher(phoneParameters())
        .item(mh::MenuItemMatcher()
            .submenu()
            .item(flightModeSwitch())
            .item(mh::MenuItemMatcher()
                .section()
                .item(modemInfo("SIM 1", "fake.tel", "gsm-3g-full"))
                .item(modemInfo("SIM 2", "fake.tel", "gsm-3g-full"))
                .item(cellularSettings())
            )
            .item(wifiEnableSwitch())
            .item(wifiSettings())
        ).match());
}

TEST_F(TestIndicatorNetworkService, ModemSignalStrength)
{
    ASSERT_NO_THROW(startIndicator());

    // start at full strength
    EXPECT_MATCHRESULT(mh::MenuMatcher(phoneParameters())
        .item(mh::MenuItemMatcher()
            .mode(mh::MenuItemMatcher::Mode::starts_with)
            .submenu()
            .item(flightModeSwitch())
            .item(mh::MenuItemMatcher()
                .section()
                .item(modemInfo("", "fake.tel", "gsm-3g-full"))
                .item(cellularSettings())
            )
        ).match());


    EXPECT_MATCHRESULT(mh::MenuMatcher(phoneParameters())
        .item(mh::MenuItemMatcher()
            .mode(mh::MenuItemMatcher::Mode::starts_with)
            .submenu()
            .item(flightModeSwitch())
            .item(mh::MenuItemMatcher()
                .section()
                .item(modemInfo("", "fake.tel", "gsm-3g-full"))
                .item(cellularSettings())
            )
        ).match());
}

TEST_F(TestIndicatorNetworkService, FlightModeTalksToURfkill)
{
    ASSERT_NO_THROW(startIndicator());

    auto& urfkillInterface = dbusMock.urfkillInterface();
    QSignalSpy urfkillSpy(&urfkillInterface, SIGNAL(FlightModeChanged(bool)));

    EXPECT_MATCHRESULT(mh::MenuMatcher(phoneParameters())
        .item(mh::MenuItemMatcher()
            .mode(mh::MenuItemMatcher::Mode::starts_with)
            .submenu()
            .item(flightModeSwitch(false)
                .activate() // <--- Activate the action now
            )
        ).match());

    // Wait to be notified that flight mode was enabled
    if (urfkillSpy.empty())
    {
        ASSERT_TRUE(urfkillSpy.wait());
    }
    EXPECT_EQ(urfkillSpy.first(), QVariantList() << QVariant(true));
}

TEST_F(TestIndicatorNetworkService, IndicatorListensToURfkill)
{
    setGlobalConnectedState(NM_STATE_CONNECTED_GLOBAL);
    auto device = createWiFiDevice(NM_DEVICE_STATE_ACTIVATED);
    auto ap = createAccessPoint("0", "the ssid", device);
    auto connection = createAccessPointConnection("0", "the ssid", device);
    createActiveConnection("0", device, connection, ap);

    ASSERT_NO_THROW(startIndicator());

    EXPECT_MATCHRESULT(mh::MenuMatcher(phoneParameters())
        .item(mh::MenuItemMatcher()
            .mode(mh::MenuItemMatcher::Mode::starts_with)
            .item(flightModeSwitch(false))
            .item(mh::MenuItemMatcher())
            .item(wifiEnableSwitch())
            .item(mh::MenuItemMatcher()
                .has_exactly(1) // <-- has one access point
            )
        ).match());

    ASSERT_TRUE(dbusMock.urfkillInterface().FlightMode(true));

    auto& nm = dbusMock.networkManagerInterface();
    nm.RemoveWifiConnection(device, connection).waitForFinished();
    nm.RemoveAccessPoint(device, ap).waitForFinished();

    EXPECT_MATCHRESULT(mh::MenuMatcher(phoneParameters())
        .item(mh::MenuItemMatcher()
            .mode(mh::MenuItemMatcher::Mode::starts_with)
            .item(flightModeSwitch(true))
            .item(mh::MenuItemMatcher())
            .item(wifiEnableSwitch(false))
            .item(mh::MenuItemMatcher()
                .is_empty() // <-- no access points
            )
        ).match());
}

TEST_F(TestIndicatorNetworkService, SimStates_NoSIM)
{
    setGlobalConnectedState(NM_STATE_DISCONNECTED);
    setSimManagerProperty(0, "Present", false);

    ASSERT_NO_THROW(startIndicator());

    EXPECT_MATCHRESULT(mh::MenuMatcher(phoneParameters())
        .item(mh::MenuItemMatcher()
            .state_icons({"nm-no-connection"})
            .mode(mh::MenuItemMatcher::Mode::starts_with)
            .item(flightModeSwitch())
            .item(mh::MenuItemMatcher()
                .item(modemInfo("", "No SIM", "no-simcard"))
                .item(cellularSettings())
            )
        ).match());
}

TEST_F(TestIndicatorNetworkService, SimStates_NoSIM2)
{
    setGlobalConnectedState(NM_STATE_DISCONNECTED);
    createModem("ril_1");
    setSimManagerProperty(1, "Present", false);

    ASSERT_NO_THROW(startIndicator());

    EXPECT_MATCHRESULT(mh::MenuMatcher(phoneParameters())
        .item(mh::MenuItemMatcher()
            .state_icons({"gsm-3g-full", "nm-no-connection"})
            .mode(mh::MenuItemMatcher::Mode::starts_with)
            .item(flightModeSwitch())
            .item(mh::MenuItemMatcher()
                .item(modemInfo("SIM 1", "fake.tel", "gsm-3g-full"))
                .item(modemInfo("SIM 2", "No SIM", "no-simcard"))
                .item(cellularSettings())
            )
        ).match());
}

TEST_F(TestIndicatorNetworkService, SimStates_LockedSIM)
{
    setGlobalConnectedState(NM_STATE_DISCONNECTED);
    setSimManagerProperty(0, "PinRequired", "pin");

    ASSERT_NO_THROW(startIndicator());

    EXPECT_MATCHRESULT(mh::MenuMatcher(phoneParameters())
        .item(mh::MenuItemMatcher()
            .state_icons({"simcard-locked", "nm-no-connection"})
            .mode(mh::MenuItemMatcher::Mode::starts_with)
            .item(flightModeSwitch())
            .item(mh::MenuItemMatcher()
                .item(modemInfo("", "SIM Locked", "simcard-locked", true)
                      .string_attribute("x-canonical-modem-locked-action", "indicator.modem.1::locked")
                )
                .item(cellularSettings())
            )
        ).match());

    setSimManagerProperty(0, "PinRequired", "none");

    EXPECT_MATCHRESULT(mh::MenuMatcher(phoneParameters())
        .item(mh::MenuItemMatcher()
            .state_icons({"gsm-3g-full", "nm-no-connection"})
            .mode(mh::MenuItemMatcher::Mode::starts_with)
            .item(flightModeSwitch())
            .item(mh::MenuItemMatcher()
                .item(modemInfo("", "fake.tel", "gsm-3g-full"))
                .item(cellularSettings())
            )
        ).match());
}

TEST_F(TestIndicatorNetworkService, SimStates_LockedSIM2)
{
    setGlobalConnectedState(NM_STATE_DISCONNECTED);
    createModem("ril_1");
    setSimManagerProperty(1, "PinRequired", "pin");

    ASSERT_NO_THROW(startIndicator());

    EXPECT_MATCHRESULT(mh::MenuMatcher(phoneParameters())
        .item(mh::MenuItemMatcher()
            .state_icons({"gsm-3g-full", "simcard-locked", "nm-no-connection"})
            .mode(mh::MenuItemMatcher::Mode::starts_with)
            .item(flightModeSwitch())
            .item(mh::MenuItemMatcher()
                .item(modemInfo("SIM 1", "fake.tel", "gsm-3g-full"))
                .item(modemInfo("SIM 2", "SIM Locked", "simcard-locked", true)
                      .string_attribute("x-canonical-modem-locked-action", "indicator.modem.2::locked")
                )
                .item(cellularSettings())
            )
        ).match());

    setSimManagerProperty(1, "PinRequired", "none");

    EXPECT_MATCHRESULT(mh::MenuMatcher(phoneParameters())
        .item(mh::MenuItemMatcher()
            .state_icons({"gsm-3g-full", "gsm-3g-full", "nm-no-connection"})
            .mode(mh::MenuItemMatcher::Mode::starts_with)
            .item(flightModeSwitch())
            .item(mh::MenuItemMatcher()
                .item(modemInfo("SIM 1", "fake.tel", "gsm-3g-full"))
                .item(modemInfo("SIM 2", "fake.tel", "gsm-3g-full"))
                .item(cellularSettings())
            )
        ).match());
}

TEST_F(TestIndicatorNetworkService, SimStates_UnlockedSIM)
{
    setGlobalConnectedState(NM_STATE_DISCONNECTED);
    setNetworkRegistrationProperty(0, "Strength", uchar(0));

    ASSERT_NO_THROW(startIndicator());

    EXPECT_MATCHRESULT(mh::MenuMatcher(phoneParameters())
        .item(mh::MenuItemMatcher()
            .state_icons({"gsm-3g-no-service", "nm-no-connection"})
            .mode(mh::MenuItemMatcher::Mode::starts_with)
            .item(flightModeSwitch())
            .item(mh::MenuItemMatcher()
                .item(modemInfo("", "No Signal", "gsm-3g-no-service"))
                .item(cellularSettings())
            )
        ).match());

    setNetworkRegistrationProperty(0, "Status", "searching");

    EXPECT_MATCHRESULT(mh::MenuMatcher(phoneParameters())
        .item(mh::MenuItemMatcher()
            .state_icons({"gsm-3g-disabled", "nm-no-connection"})
            .mode(mh::MenuItemMatcher::Mode::starts_with)
            .item(flightModeSwitch())
            .item(mh::MenuItemMatcher()
                .item(modemInfo("", "Searching", "gsm-3g-disabled"))
                .item(cellularSettings())
            )
        ).match());

    setNetworkRegistrationProperty(0, "Status", "registered");
    setNetworkRegistrationProperty(0, "Strength", uchar(1));

    EXPECT_MATCHRESULT(mh::MenuMatcher(phoneParameters())
        .item(mh::MenuItemMatcher()
            .state_icons({"gsm-3g-none", "nm-no-connection"})
            .mode(mh::MenuItemMatcher::Mode::starts_with)
            .item(flightModeSwitch())
            .item(mh::MenuItemMatcher()
                .item(modemInfo("", "fake.tel", "gsm-3g-none"))
                .item(cellularSettings())
            )
        ).match());

    setNetworkRegistrationProperty(0, "Strength", uchar(6));

    EXPECT_MATCHRESULT(mh::MenuMatcher(phoneParameters())
        .item(mh::MenuItemMatcher()
            .state_icons({"gsm-3g-low", "nm-no-connection"})
            .mode(mh::MenuItemMatcher::Mode::starts_with)
            .item(flightModeSwitch())
            .item(mh::MenuItemMatcher()
                .item(modemInfo("", "fake.tel", "gsm-3g-low"))
                .item(cellularSettings())
            )
        ).match());

    setNetworkRegistrationProperty(0, "Strength", uchar(16));

    EXPECT_MATCHRESULT(mh::MenuMatcher(phoneParameters())
        .item(mh::MenuItemMatcher()
            .state_icons({"gsm-3g-medium", "nm-no-connection"})
            .mode(mh::MenuItemMatcher::Mode::starts_with)
            .item(flightModeSwitch())
            .item(mh::MenuItemMatcher()
                .item(modemInfo("", "fake.tel", "gsm-3g-medium"))
                .item(cellularSettings())
            )
        ).match());

    setNetworkRegistrationProperty(0, "Strength", uchar(26));

    EXPECT_MATCHRESULT(mh::MenuMatcher(phoneParameters())
        .item(mh::MenuItemMatcher()
            .state_icons({"gsm-3g-high", "nm-no-connection"})
            .mode(mh::MenuItemMatcher::Mode::starts_with)
            .item(flightModeSwitch())
            .item(mh::MenuItemMatcher()
                .item(modemInfo("", "fake.tel", "gsm-3g-high"))
                .item(cellularSettings())
            )
        ).match());

    setNetworkRegistrationProperty(0, "Strength", uchar(39));

    EXPECT_MATCHRESULT(mh::MenuMatcher(phoneParameters())
        .item(mh::MenuItemMatcher()
            .state_icons({"gsm-3g-full", "nm-no-connection"})
            .mode(mh::MenuItemMatcher::Mode::starts_with)
            .item(flightModeSwitch())
            .item(mh::MenuItemMatcher()
                .item(modemInfo("", "fake.tel", "gsm-3g-full"))
                .item(cellularSettings())
            )
        ).match());
}

TEST_F(TestIndicatorNetworkService, SimStates_UnlockedSIM2)
{
    setGlobalConnectedState(NM_STATE_DISCONNECTED);
    createModem("ril_1");
    setNetworkRegistrationProperty(1, "Strength", uchar(0));

    ASSERT_NO_THROW(startIndicator());

    EXPECT_MATCHRESULT(mh::MenuMatcher(phoneParameters())
        .item(mh::MenuItemMatcher()
            .state_icons({"gsm-3g-full", "gsm-3g-no-service", "nm-no-connection"})
            .mode(mh::MenuItemMatcher::Mode::starts_with)
            .item(flightModeSwitch())
            .item(mh::MenuItemMatcher()
                .item(modemInfo("SIM 1", "fake.tel", "gsm-3g-full"))
                .item(modemInfo("SIM 2", "No Signal", "gsm-3g-no-service"))
                .item(cellularSettings())
            )
        ).match());

    setNetworkRegistrationProperty(1, "Status", "searching");

    EXPECT_MATCHRESULT(mh::MenuMatcher(phoneParameters())
        .item(mh::MenuItemMatcher()
            .state_icons({"gsm-3g-full", "gsm-3g-disabled", "nm-no-connection"})
            .mode(mh::MenuItemMatcher::Mode::starts_with)
            .item(flightModeSwitch())
            .item(mh::MenuItemMatcher()
                .item(modemInfo("SIM 1", "fake.tel", "gsm-3g-full"))
                .item(modemInfo("SIM 2", "Searching", "gsm-3g-disabled"))
                .item(cellularSettings())
            )
        ).match());

    setNetworkRegistrationProperty(1, "Status", "registered");
    setNetworkRegistrationProperty(1, "Strength", uchar(1));

    EXPECT_MATCHRESULT(mh::MenuMatcher(phoneParameters())
        .item(mh::MenuItemMatcher()
            .state_icons({"gsm-3g-full", "gsm-3g-none", "nm-no-connection"})
            .mode(mh::MenuItemMatcher::Mode::starts_with)
            .item(flightModeSwitch())
            .item(mh::MenuItemMatcher()
                .item(modemInfo("SIM 1", "fake.tel", "gsm-3g-full"))
                .item(modemInfo("SIM 2", "fake.tel", "gsm-3g-none"))
                .item(cellularSettings())
            )
        ).match());

    setNetworkRegistrationProperty(1, "Strength", uchar(6));

    EXPECT_MATCHRESULT(mh::MenuMatcher(phoneParameters())
        .item(mh::MenuItemMatcher()
            .state_icons({"gsm-3g-full", "gsm-3g-low", "nm-no-connection"})
            .mode(mh::MenuItemMatcher::Mode::starts_with)
            .item(flightModeSwitch())
            .item(mh::MenuItemMatcher()
                .item(modemInfo("SIM 1", "fake.tel", "gsm-3g-full"))
                .item(modemInfo("SIM 2", "fake.tel", "gsm-3g-low"))
                .item(cellularSettings())
            )
        ).match());

    setNetworkRegistrationProperty(1, "Strength", uchar(16));

    EXPECT_MATCHRESULT(mh::MenuMatcher(phoneParameters())
        .item(mh::MenuItemMatcher()
            .state_icons({"gsm-3g-full", "gsm-3g-medium", "nm-no-connection"})
            .mode(mh::MenuItemMatcher::Mode::starts_with)
            .item(flightModeSwitch())
            .item(mh::MenuItemMatcher()
                .item(modemInfo("SIM 1", "fake.tel", "gsm-3g-full"))
                .item(modemInfo("SIM 2", "fake.tel", "gsm-3g-medium"))
                .item(cellularSettings())
            )
        ).match());

    setNetworkRegistrationProperty(1, "Strength", uchar(26));

    EXPECT_MATCHRESULT(mh::MenuMatcher(phoneParameters())
        .item(mh::MenuItemMatcher()
            .state_icons({"gsm-3g-full", "gsm-3g-high", "nm-no-connection"})
            .mode(mh::MenuItemMatcher::Mode::starts_with)
            .item(flightModeSwitch())
            .item(mh::MenuItemMatcher()
                .item(modemInfo("SIM 1", "fake.tel", "gsm-3g-full"))
                .item(modemInfo("SIM 2", "fake.tel", "gsm-3g-high"))
                .item(cellularSettings())
            )
        ).match());

    setNetworkRegistrationProperty(1, "Strength", uchar(39));

    EXPECT_MATCHRESULT(mh::MenuMatcher(phoneParameters())
        .item(mh::MenuItemMatcher()
            .state_icons({"gsm-3g-full", "gsm-3g-full", "nm-no-connection"})
            .mode(mh::MenuItemMatcher::Mode::starts_with)
            .item(flightModeSwitch())
            .item(mh::MenuItemMatcher()
                .item(modemInfo("SIM 1", "fake.tel", "gsm-3g-full"))
                .item(modemInfo("SIM 2", "fake.tel", "gsm-3g-full"))
                .item(cellularSettings())
            )
        ).match());
}

TEST_F(TestIndicatorNetworkService, FlightMode_NoSIM)
{
    setGlobalConnectedState(NM_STATE_CONNECTED_GLOBAL);
    auto device = createWiFiDevice(NM_DEVICE_STATE_ACTIVATED);
    auto ap = createAccessPoint("0", "the ssid", device);
    auto connection = createAccessPointConnection("0", "the ssid", device);
    createActiveConnection("0", device, connection, ap);

    ASSERT_NO_THROW(startIndicator());

    setSimManagerProperty(0, "Present", false);

    EXPECT_MATCHRESULT(mh::MenuMatcher(phoneParameters())
        .item(mh::MenuItemMatcher()
            .state_icons({"nm-signal-100-secure"})
            .mode(mh::MenuItemMatcher::Mode::starts_with)
            .item(flightModeSwitch())
            .item(mh::MenuItemMatcher()
                .item(modemInfo("", "No SIM", "no-simcard"))
                .item(cellularSettings())
            )
            .item(wifiEnableSwitch())
            .item(mh::MenuItemMatcher()
                .section()
                .item(accessPoint("the ssid", 1,
                      Secure::secure,
                      ApMode::infra,
                      ConnectionStatus::connected)
                )
            )
        ).match());

    EXPECT_MATCHRESULT(mh::MenuMatcher(phoneParameters())
        .item(mh::MenuItemMatcher()
            .mode(mh::MenuItemMatcher::Mode::starts_with)
            .item(flightModeSwitch()
                  .activate() // <--- Activate the action now
            )
        ).match());

    setGlobalConnectedState(NM_STATE_DISCONNECTED);
    auto& nm = dbusMock.networkManagerInterface();
    nm.RemoveAccessPoint(device, ap).waitForFinished();

    EXPECT_MATCHRESULT(mh::MenuMatcher(phoneParameters())
        .item(mh::MenuItemMatcher()
            .state_icons({"airplane-mode", "nm-no-connection"})
            .mode(mh::MenuItemMatcher::Mode::starts_with)
            .item(flightModeSwitch(true))
            .item(mh::MenuItemMatcher()
                .item(modemInfo("", "No SIM", "no-simcard"))
                .item(cellularSettings())
            )
            .item(wifiEnableSwitch(false))
            .item(mh::MenuItemMatcher()
                  .is_empty()
            )
        ).match());

    EXPECT_MATCHRESULT(mh::MenuMatcher(phoneParameters())
        .item(mh::MenuItemMatcher()
            .mode(mh::MenuItemMatcher::Mode::starts_with)
            .item(flightModeSwitch(true)
                  .activate() // <--- Activate the action now
            )
        ).match());

    setGlobalConnectedState(NM_STATE_CONNECTED_GLOBAL);

    EXPECT_MATCHRESULT(mh::MenuMatcher(phoneParameters())
        .item(mh::MenuItemMatcher()
            .state_icons({"nm-signal-100-secure"})
            .mode(mh::MenuItemMatcher::Mode::starts_with)
            .item(flightModeSwitch())
            .item(mh::MenuItemMatcher()
                .item(modemInfo("", "No SIM", "no-simcard"))
                .item(cellularSettings())
            )
            .item(wifiEnableSwitch())
        ).match());
}

} // namespace
