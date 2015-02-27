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

    static QString randomMac()
    {
        int high = 254;
        int low = 1;
        QString hardwareAddress;
        bool first = true;

        for (unsigned int i = 0; i < 6; ++i)
        {
            if (!first)
            {
                hardwareAddress.append(":");
            }
            int r = qrand() % ((high + 1) - low) + low;
            hardwareAddress.append(QString::number(r, 16));
            first = false;
        }
        return hardwareAddress;
    }


    QString createAccessPoint(const QString& id, const QString& ssid, const QString& device, int strength = 100,
                              Secure secure = Secure::secure, ApMode apMode = ApMode::infra)
    {

        auto& networkManager(dbusMock.networkManagerInterface());
        auto reply = networkManager.AddAccessPoint(
                            device, id, ssid,
                            randomMac(),
                            apMode == ApMode::adhoc ? NM_802_11_MODE_ADHOC : NM_802_11_MODE_INFRA,
                            0, 0, strength,
                            secure == Secure::secure ? NM_802_11_AP_SEC_KEY_MGMT_PSK : NM_802_11_AP_SEC_NONE,
                            secure == Secure::secure ? NM_802_11_AP_FLAGS_PRIVACY : NM_802_11_AP_FLAGS_NONE);
        reply.waitForFinished();
        return reply;
    }

    void removeAccessPoint(const QString& device, const QString& ap)
    {
        auto& nm = dbusMock.networkManagerInterface();
        nm.RemoveAccessPoint(device, ap).waitForFinished();
    }

    QString createAccessPointConnection(const QString& id, const QString& ssid, const QString& device)
    {
        auto& networkManager(dbusMock.networkManagerInterface());
        auto reply = networkManager.AddWiFiConnection(device, id, ssid,
                                                      "");
        reply.waitForFinished();
        return reply;
    }

    void removeWifiConnection(const QString& device, const QString& connection)
    {
        auto& nm = dbusMock.networkManagerInterface();
        nm.RemoveWifiConnection(device, connection).waitForFinished();
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

    void removeActiveConnection(const QString& device, const QString& active_connection)
    {
        auto& nm = dbusMock.networkManagerInterface();
        nm.RemoveActiveConnection(device, active_connection).waitForFinished();
    }

    void setGlobalConnectedState(int state)
    {
        auto& nm = dbusMock.networkManagerInterface();
        nm.SetGlobalConnectionState(state).waitForFinished();
    }

    void setNmProperty(const QString& path, const QString& iface, const QString& name, const QVariant& value)
    {
        auto& nm = dbusMock.networkManagerInterface();
        nm.SetProperty(path, iface, name, QDBusVariant(value)).waitForFinished();
    }

    QString createModem(const QString& id)
    {
        auto& ofono(dbusMock.ofonoInterface());
        QVariantMap modemProperties {{ "Powered", false } };
        return ofono.AddModem(id, modemProperties);
    }

    template<typename T>
    void setModemProperty(int modemIndex, const QString& propertyName, const T& value)
    {
        auto& ofono(dbusMock.ofonoModemInterface(modemIndex));
        ofono.SetProperty(propertyName, QDBusVariant(value)).waitForFinished();
    }

    template<typename T>
    void setSimManagerProperty(int modemIndex, const QString& propertyName, const T& value)
    {
        auto& ofono(dbusMock.ofonoSimManagerInterface(modemIndex));
        ofono.SetProperty(propertyName, QDBusVariant(value)).waitForFinished();
    }

    template<typename T>
    void setNetworkRegistrationProperty(int modemIndex, const QString& propertyName, const T& value)
    {
        auto& ofono(dbusMock.ofonoNetworkRegistrationInterface(modemIndex));
        ofono.SetProperty(propertyName, QDBusVariant(value)).waitForFinished();
    }

    static mh::MenuItemMatcher flightModeSwitch(bool toggled = false)
    {
        return mh::MenuItemMatcher::checkbox()
            .label("Flight Mode")
            .action("indicator.airplane.enabled")
            .toggled(toggled);
    }

    static mh::MenuItemMatcher accessPoint(const string& ssid, Secure secure,
                ApMode apMode, ConnectionStatus connectionStatus, int strength = 100)
    {
        return mh::MenuItemMatcher::checkbox()
            .label(ssid)
            .widget("unity.widgets.systemsettings.tablet.accesspoint")
            .toggled(connectionStatus == ConnectionStatus::connected)
            .pass_through_attribute(
                "x-canonical-wifi-ap-strength-action",
                shared_ptr<GVariant>(g_variant_new_byte(strength), &mh::gvariant_deleter))
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
                .item(accessPoint("the ssid",
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
                .item(accessPoint("the ssid",
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
                .item(accessPoint("the ssid",
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
                .item(accessPoint("the ssid",
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

    removeWifiConnection(device, connection);
    removeAccessPoint(device, ap);

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
    // set flight mode off, wifi off, and cell data off
    setGlobalConnectedState(NM_STATE_DISCONNECTED);

    // set no sim
    setSimManagerProperty(0, "Present", false);

    // start the indicator
    ASSERT_NO_THROW(startIndicator());

    // check indicator is just a 0-bar wifi icon
    // check sim status shows “No SIM” with crossed sim card icon
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
    // set flight mode off, wifi off, and cell data off
    setGlobalConnectedState(NM_STATE_DISCONNECTED);

    // set no sim 2
    createModem("ril_1");
    setSimManagerProperty(1, "Present", false);

    // start the indicator
    ASSERT_NO_THROW(startIndicator());

    // check indicator is a 4-bar signal icon and a 0-bar wifi icon
    // check sim 2 status shows “No SIM” with crossed sim card icon
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
    // set flight mode off, wifi off, and cell data off, and sim in
    setGlobalConnectedState(NM_STATE_DISCONNECTED);

    // set sim locked
    setSimManagerProperty(0, "PinRequired", "pin");

    // start the indicator
    ASSERT_NO_THROW(startIndicator());

    // check indicator is a locked sim card and a 0-bar wifi icon.
    // check sim status shows “SIM Locked”, with locked sim card icon and a “Unlock SIM” button beneath.
    // check that the “Unlock SIM” button has the correct action name.
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

    // set sim unlocked
    setSimManagerProperty(0, "PinRequired", "none");

    // check indicator is a 4-bar signal icon and a 0-bar wifi icon
    // check sim status shows correct carrier name with 4-bar signal icon.
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
    // set flight mode off, wifi off, and cell data off, and sim in
    setGlobalConnectedState(NM_STATE_DISCONNECTED);

    // set sim 2 locked
    createModem("ril_1");
    setSimManagerProperty(1, "PinRequired", "pin");

    // start the indicator
    ASSERT_NO_THROW(startIndicator());

    // check indicator is a 4-bar signal icon, a locked sim card and a 0-bar wifi icon.
    // check sim 2 status shows “SIM Locked”, with locked sim card icon and a “Unlock SIM” button beneath.
    // check that the “Unlock SIM” button has the correct action name.
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

    // set sim 2 unlocked
    setSimManagerProperty(1, "PinRequired", "none");

    // check indicator is 4-bar signal icon, a 4-bar signal icon and a 0-bar wifi icon
    // check sim statuses show correct carrier names with 4-bar signal icons.
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
    // set flight mode off, wifi off, cell data off, sim in, and sim unlocked
    setGlobalConnectedState(NM_STATE_DISCONNECTED);

    // set no signal
    setNetworkRegistrationProperty(0, "Strength", QVariant::fromValue(uchar(0)));

    // start the indicator
    ASSERT_NO_THROW(startIndicator());

    // check indicator is a crossed signal icon and a 0-bar wifi icon.
    // check sim status shows “No Signal” with crossed signal icon.
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

    // set sim searching
    setNetworkRegistrationProperty(0, "Status", "searching");

    // check indicator is a disabled signal icon and a 0-bar wifi icon.
    // check sim status shows “Searching” with disabled signal icon.
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

    // set sim registered
    setNetworkRegistrationProperty(0, "Status", "registered");

    // set signal strength to 1
    setNetworkRegistrationProperty(0, "Strength", QVariant::fromValue(uchar(1)));

    // check indicator is a 0-bar signal icon and a 0-bar wifi icon.
    // check sim status shows correct carrier name with 0-bar signal icon.
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

    // set signal strength to 6
    setNetworkRegistrationProperty(0, "Strength", QVariant::fromValue(uchar(6)));

    // check indicator is a 1-bar signal icon and a 0-bar wifi icon.
    // check sim status shows correct carrier name with 1-bar signal icon.
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

    // set signal strength to 16
    setNetworkRegistrationProperty(0, "Strength", QVariant::fromValue(uchar(16)));

    // check indicator is a 2-bar signal icon and a 0-bar wifi icon.
    // check sim status shows correct carrier name with 2-bar signal icon.
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

    // set signal strength to 26
    setNetworkRegistrationProperty(0, "Strength", QVariant::fromValue(uchar(26)));

    // check indicator is a 3-bar signal icon and a 0-bar wifi icon.
    // check sim status shows correct carrier name with 3-bar signal icon.
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

    // set signal strength to 39
    setNetworkRegistrationProperty(0, "Strength", QVariant::fromValue(uchar(39)));

    // check indicator is a 4-bar signal icon and a 0-bar wifi icon.
    // check sim status shows correct carrier name with 4-bar signal icon.
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
    // set flight mode off, wifi off, cell data off, sim in, and sim unlocked
    setGlobalConnectedState(NM_STATE_DISCONNECTED);

    // set no signal on sim 2
    createModem("ril_1");
    setNetworkRegistrationProperty(1, "Strength", QVariant::fromValue(uchar(0)));

    // start the indicator
    ASSERT_NO_THROW(startIndicator());

    // check indicator is a 4-bar signal icon, a crossed signal icon and a 0-bar wifi icon.
    // check sim 2 status shows “No Signal” with crossed signal icon.
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

    // set sim searching
    setNetworkRegistrationProperty(1, "Status", "searching");

    // check indicator is a 4-bar signal icon, a disabled signal icon and a 0-bar wifi icon.
    // check sim 2 status shows “Searching” with disabled signal icon.
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

    // set sim registered
    setNetworkRegistrationProperty(1, "Status", "registered");

    // set signal strength to 1
    setNetworkRegistrationProperty(1, "Strength", QVariant::fromValue(uchar(1)));

    // check indicator is a 4-bar signal icon, a 0-bar signal icon and a 0-bar wifi icon.
    // check sim 2 status shows correct carrier name with 0-bar signal icon.
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

    // set signal strength to 6
    setNetworkRegistrationProperty(1, "Strength", QVariant::fromValue(uchar(6)));

    // check indicator is a 4-bar signal icon, a 1-bar signal icon and a 0-bar wifi icon.
    // check sim 2 status shows correct carrier name with 1-bar signal icon.
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

    // set signal strength to 16
    setNetworkRegistrationProperty(1, "Strength", QVariant::fromValue(uchar(16)));

    // check indicator is a 4-bar signal icon, a 2-bar signal icon and a 0-bar wifi icon.
    // check sim 2 status shows correct carrier name with 2-bar signal icon.
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

    // set signal strength to 26
    setNetworkRegistrationProperty(1, "Strength", QVariant::fromValue(uchar(26)));

    // check indicator is a 4-bar signal icon, a 3-bar signal icon and a 0-bar wifi icon.
    // check sim 2 status shows correct carrier name with 3-bar signal icon.
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

    // set signal strength to 39
    setNetworkRegistrationProperty(1, "Strength", QVariant::fromValue(uchar(39)));

    // check indicator is a 4-bar signal icon, a 4-bar signal icon and a 0-bar wifi icon.
    // check sim 2 status shows correct carrier name with 4-bar signal icon.
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
    // set wifi on, flight mode off
    setGlobalConnectedState(NM_STATE_CONNECTED_GLOBAL);

    // add and connect to 2-bar unsecure AP
    auto device = createWiFiDevice(NM_DEVICE_STATE_ACTIVATED);
    auto ap = createAccessPoint("0", "the ssid", device, 40, Secure::insecure);
    auto connection = createAccessPointConnection("0", "the ssid", device);
    createActiveConnection("0", device, connection, ap);

    // start the indicator
    ASSERT_NO_THROW(startIndicator());

    // set no sim
    setSimManagerProperty(0, "Present", false);

    // check indicator is just a 2-bar wifi icon
    // check sim status shows “No SIM” with crossed sim card icon.
    EXPECT_MATCHRESULT(mh::MenuMatcher(phoneParameters())
        .item(mh::MenuItemMatcher()
            .state_icons({"nm-signal-50"})
            .mode(mh::MenuItemMatcher::Mode::starts_with)
            .item(flightModeSwitch(false))
            .item(mh::MenuItemMatcher()
                .item(modemInfo("", "No SIM", "no-simcard"))
                .item(cellularSettings())
            )
            .item(wifiEnableSwitch(true))
            .item(mh::MenuItemMatcher()
                .section()
                .item(accessPoint("the ssid",
                      Secure::insecure,
                      ApMode::infra,
                      ConnectionStatus::connected,
                      40)
                )
            )
        ).match());

    // set flight mode on
    EXPECT_MATCHRESULT(mh::MenuMatcher(phoneParameters())
        .item(mh::MenuItemMatcher()
            .mode(mh::MenuItemMatcher::Mode::starts_with)
            .item(flightModeSwitch(false)
                  .activate()
            )
        ).match());

    setGlobalConnectedState(NM_STATE_DISCONNECTED);
    removeWifiConnection(device, connection);
    removeAccessPoint(device, ap);

    // check that the wifi switch turns off
    // check indicator is a plane icon and a 0-bar wifi icon
    // check sim status shows “No SIM” with crossed sim card icon (unchanged).
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

    // set flight mode off
    EXPECT_MATCHRESULT(mh::MenuMatcher(phoneParameters())
        .item(mh::MenuItemMatcher()
            .mode(mh::MenuItemMatcher::Mode::starts_with)
            .item(flightModeSwitch(true)
                  .activate()
            )
        ).match());

    setGlobalConnectedState(NM_STATE_CONNECTED_GLOBAL);

    // check that the wifi switch turns back on
    // check indicator is just a 2-bar wifi icon
    // check sim status shows “No SIM” with crossed sim card icon.
    EXPECT_MATCHRESULT(mh::MenuMatcher(phoneParameters())
        .item(mh::MenuItemMatcher()
            .state_icons({"nm-signal-50"})
            .mode(mh::MenuItemMatcher::Mode::starts_with)
            .item(flightModeSwitch(false))
            .item(mh::MenuItemMatcher()
                .item(modemInfo("", "No SIM", "no-simcard"))
                .item(cellularSettings())
            )
            .item(wifiEnableSwitch(true))
        ).match());
}

TEST_F(TestIndicatorNetworkService, FlightMode_LockedSIM)
{
    // set wifi on, flight mode off
    setGlobalConnectedState(NM_STATE_CONNECTED_GLOBAL);

    // add and connect to 1-bar secure AP
    auto device = createWiFiDevice(NM_DEVICE_STATE_ACTIVATED);
    auto ap = createAccessPoint("0", "the ssid", device, 20, Secure::secure);
    auto connection = createAccessPointConnection("0", "the ssid", device);
    createActiveConnection("0", device, connection, ap);

    // start the indicator
    ASSERT_NO_THROW(startIndicator());

    // set sim locked
    setSimManagerProperty(0, "PinRequired", "pin");

    // check indicator is a locked sim card and a 1-bar locked wifi icon
    // check sim status shows “SIM Locked”, with locked sim card icon and a “Unlock SIM” button beneath.
    EXPECT_MATCHRESULT(mh::MenuMatcher(phoneParameters())
        .item(mh::MenuItemMatcher()
            .state_icons({"simcard-locked", "nm-signal-25-secure"})
            .mode(mh::MenuItemMatcher::Mode::starts_with)
            .item(flightModeSwitch(false))
            .item(mh::MenuItemMatcher()
                .item(modemInfo("", "SIM Locked", "simcard-locked", true))
                .item(cellularSettings())
            )
            .item(wifiEnableSwitch(true))
            .item(mh::MenuItemMatcher()
                .section()
                .item(accessPoint("the ssid",
                      Secure::secure,
                      ApMode::infra,
                      ConnectionStatus::connected,
                      20)
                )
            )
        ).match());

    // set flight mode on
    EXPECT_MATCHRESULT(mh::MenuMatcher(phoneParameters())
        .item(mh::MenuItemMatcher()
            .mode(mh::MenuItemMatcher::Mode::starts_with)
            .item(flightModeSwitch(false)
                  .activate()
            )
        ).match());

    setGlobalConnectedState(NM_STATE_DISCONNECTED);
    removeWifiConnection(device, connection);
    removeAccessPoint(device, ap);

    // check that the wifi switch turns off
    // check indicator is a plane icon, a locked sim card and a 0-bar wifi icon
    // check sim status shows “SIM Locked”, with locked sim card icon and a “Unlock SIM” button beneath (unchanged).
    EXPECT_MATCHRESULT(mh::MenuMatcher(phoneParameters())
        .item(mh::MenuItemMatcher()
            .state_icons({"airplane-mode", "simcard-locked", "nm-no-connection"})
            .mode(mh::MenuItemMatcher::Mode::starts_with)
            .item(flightModeSwitch(true))
            .item(mh::MenuItemMatcher()
                .item(modemInfo("", "SIM Locked", "simcard-locked", true))
                .item(cellularSettings())
            )
            .item(wifiEnableSwitch(false))
            .item(mh::MenuItemMatcher()
                .is_empty()
            )
        ).match());

    // set flight mode off
    EXPECT_MATCHRESULT(mh::MenuMatcher(phoneParameters())
        .item(mh::MenuItemMatcher()
            .mode(mh::MenuItemMatcher::Mode::starts_with)
            .item(flightModeSwitch(true)
                  .activate()
            )
        ).match());

    setGlobalConnectedState(NM_STATE_CONNECTED_GLOBAL);

    // check that the wifi switch turns back on
    // check indicator is a locked sim card and a 1-bar locked wifi icon
    // check sim status shows “SIM Locked”, with locked sim card icon and a “Unlock SIM” button beneath.
    EXPECT_MATCHRESULT(mh::MenuMatcher(phoneParameters())
        .item(mh::MenuItemMatcher()
            .state_icons({"simcard-locked", "nm-signal-25-secure"})
            .mode(mh::MenuItemMatcher::Mode::starts_with)
            .item(flightModeSwitch(false))
            .item(mh::MenuItemMatcher()
                .item(modemInfo("", "SIM Locked", "simcard-locked", true))
                .item(cellularSettings())
            )
            .item(wifiEnableSwitch(true))
        ).match());
}

TEST_F(TestIndicatorNetworkService, FlightMode_WifiOff)
{
    // set wifi on, flight mode off
    setGlobalConnectedState(NM_STATE_CONNECTED_GLOBAL);

    // add some APs (secure / unsecure / adhoc / varied strength)
    auto device = createWiFiDevice(NM_DEVICE_STATE_ACTIVATED);
    auto ap1 = createAccessPoint("1", "NSD", device, 0, Secure::secure, ApMode::infra);
    auto ap2 = createAccessPoint("2", "JDR", device, 20, Secure::secure, ApMode::adhoc);
    auto ap3 = createAccessPoint("3", "DGN", device, 40, Secure::secure, ApMode::infra);
    auto ap4 = createAccessPoint("4", "JDY", device, 60, Secure::secure, ApMode::adhoc);
    auto ap5 = createAccessPoint("5", "SCE", device, 20, Secure::insecure, ApMode::infra);
    auto ap6 = createAccessPoint("6", "ADS", device, 40, Secure::insecure, ApMode::adhoc);
    auto ap7 = createAccessPoint("7", "CFT", device, 60, Secure::insecure, ApMode::infra);
    auto ap8 = createAccessPoint("8", "GDF", device, 80, Secure::insecure, ApMode::adhoc);

    // connect to 2-bar secure AP
    auto connection = createAccessPointConnection("3", "DGN", device);
    auto active_connection = createActiveConnection("3", device, connection, ap3);

    // start the indicator
    ASSERT_NO_THROW(startIndicator());

    // set sim unlocked on carrier with 3-bar signal
    setNetworkRegistrationProperty(0, "Strength", QVariant::fromValue(uchar(26)));

    // check that the wifi switch is on
    // check indicator is a 3-bar signal icon and 2-bar locked wifi icon
    // check sim status shows correct carrier name with 3-bar signal icon.
    // check that AP list contains the connected AP at top then other APs underneath in alphabetical order.
    EXPECT_MATCHRESULT(mh::MenuMatcher(phoneParameters())
        .item(mh::MenuItemMatcher()
            .state_icons({"gsm-3g-high", "nm-signal-50-secure"})
            .mode(mh::MenuItemMatcher::Mode::starts_with)
            .item(flightModeSwitch(false))
            .item(mh::MenuItemMatcher()
                .item(modemInfo("", "fake.tel", "gsm-3g-high"))
                .item(cellularSettings())
            )
            .item(wifiEnableSwitch(true))
            .item(mh::MenuItemMatcher()
                .section()
                .item(accessPoint("DGN", Secure::secure, ApMode::infra, ConnectionStatus::connected, 40))
                .item(accessPoint("ADS", Secure::insecure, ApMode::adhoc, ConnectionStatus::disconnected, 40))
                .item(accessPoint("CFT", Secure::insecure, ApMode::infra, ConnectionStatus::disconnected, 60))
                .item(accessPoint("GDF", Secure::insecure, ApMode::adhoc, ConnectionStatus::disconnected, 80))
                .item(accessPoint("JDR", Secure::secure, ApMode::adhoc, ConnectionStatus::disconnected, 20))
                .item(accessPoint("JDY", Secure::secure, ApMode::adhoc, ConnectionStatus::disconnected, 60))
                .item(accessPoint("NSD", Secure::secure, ApMode::infra, ConnectionStatus::disconnected, 0))
                .item(accessPoint("SCE", Secure::insecure, ApMode::infra, ConnectionStatus::disconnected, 20))
            )
        ).match());

    // set wifi off
    EXPECT_MATCHRESULT(mh::MenuMatcher(phoneParameters())
        .item(mh::MenuItemMatcher()
            .mode(mh::MenuItemMatcher::Mode::starts_with)
              .item(flightModeSwitch(false))
              .item(mh::MenuItemMatcher())
              .item(wifiEnableSwitch(true)
                  .activate()
              )
        ).match());

    setGlobalConnectedState(NM_STATE_DISCONNECTED);
    removeActiveConnection(device, active_connection);
    removeWifiConnection(device, connection);
    removeAccessPoint(device, ap1);
    removeAccessPoint(device, ap2);
    removeAccessPoint(device, ap3);
    removeAccessPoint(device, ap4);
    removeAccessPoint(device, ap5);
    removeAccessPoint(device, ap6);
    removeAccessPoint(device, ap7);
    removeAccessPoint(device, ap8);

    // check that the flight mode switch is still off
    // check that the wifi switch is off
    // check indicator is a 3-bar signal icon and 0-bar wifi icon
    // check sim status shows correct carrier name with 3-bar signal icon.
    // check that AP list is empty
    EXPECT_MATCHRESULT(mh::MenuMatcher(phoneParameters())
        .item(mh::MenuItemMatcher()
            .state_icons({"gsm-3g-high", "nm-no-connection"})
            .mode(mh::MenuItemMatcher::Mode::starts_with)
            .item(flightModeSwitch(false))
            .item(mh::MenuItemMatcher()
                .item(modemInfo("", "fake.tel", "gsm-3g-high"))
                .item(cellularSettings())
            )
            .item(wifiEnableSwitch(false))
            .item(mh::MenuItemMatcher()
                .is_empty()
            )
        ).match());

    // set flight mode on
    EXPECT_MATCHRESULT(mh::MenuMatcher(phoneParameters())
        .item(mh::MenuItemMatcher()
            .mode(mh::MenuItemMatcher::Mode::starts_with)
            .item(flightModeSwitch(false)
                  .activate()
            )
        ).match());

    setModemProperty(0, "Online", false);

    // check indicator is a plane icon and a 0-bar wifi icon
    // check sim status shows “Offline” with 0-bar signal icon.
    // check that AP list is empty
    EXPECT_MATCHRESULT(mh::MenuMatcher(phoneParameters())
        .item(mh::MenuItemMatcher()
            .state_icons({"airplane-mode", "nm-no-connection"})
            .mode(mh::MenuItemMatcher::Mode::starts_with)
            .item(flightModeSwitch(true))
            .item(mh::MenuItemMatcher()
                .item(modemInfo("", "Offline", "gsm-3g-disabled"))
                .item(cellularSettings())
            )
            .item(wifiEnableSwitch(false))
            .item(mh::MenuItemMatcher()
                .is_empty()
            )
        ).match());

    // set flight mode off
    EXPECT_MATCHRESULT(mh::MenuMatcher(phoneParameters())
        .item(mh::MenuItemMatcher()
            .mode(mh::MenuItemMatcher::Mode::starts_with)
            .item(flightModeSwitch(true)
                  .activate()
            )
        ).match());

    setModemProperty(0, "Online", true);
    setGlobalConnectedState(NM_STATE_CONNECTED_GLOBAL);

    // check that the wifi switch is still off
    // check indicator is a 3-bar signal icon and 0-bar wifi icon
    // check sim status shows correct carrier name with 3-bar signal icon.
    // check that AP list is empty
    EXPECT_MATCHRESULT(mh::MenuMatcher(phoneParameters())
        .item(mh::MenuItemMatcher()
            .state_icons({"gsm-3g-high", "nm-signal-0"})
            .mode(mh::MenuItemMatcher::Mode::starts_with)
            .item(flightModeSwitch(false))
            .item(mh::MenuItemMatcher()
                .item(modemInfo("", "fake.tel", "gsm-3g-high"))
                .item(cellularSettings())
            )
            .item(wifiEnableSwitch(false))
            .item(mh::MenuItemMatcher()
                .is_empty()
            )
        ).match());
}

TEST_F(TestIndicatorNetworkService, FlightMode_WifiOn)
{
    // set wifi on, flight mode off
    setGlobalConnectedState(NM_STATE_CONNECTED_GLOBAL);

    // add some APs (secure / unsecure / adhoc / varied strength)
    auto device = createWiFiDevice(NM_DEVICE_STATE_ACTIVATED);
    auto ap1 = createAccessPoint("1", "NSD", device, 0, Secure::secure, ApMode::infra);
    auto ap2 = createAccessPoint("2", "JDR", device, 20, Secure::secure, ApMode::adhoc);
    auto ap3 = createAccessPoint("3", "DGN", device, 40, Secure::secure, ApMode::infra);
    auto ap4 = createAccessPoint("4", "JDY", device, 60, Secure::secure, ApMode::adhoc);
    auto ap5 = createAccessPoint("5", "SCE", device, 20, Secure::insecure, ApMode::infra);
    auto ap6 = createAccessPoint("6", "ADS", device, 40, Secure::insecure, ApMode::adhoc);
    auto ap7 = createAccessPoint("7", "CFT", device, 60, Secure::insecure, ApMode::infra);
    auto ap8 = createAccessPoint("8", "GDF", device, 80, Secure::insecure, ApMode::adhoc);

    // connect to 4-bar insecure AP
    auto connection = createAccessPointConnection("8", "GDF", device);
    auto active_connection = createActiveConnection("8", device, connection, ap8);

    // start the indicator
    ASSERT_NO_THROW(startIndicator());

    // set sim unlocked on carrier with 1-bar signal
    setNetworkRegistrationProperty(0, "Strength", QVariant::fromValue(uchar(6)));

    // check that the wifi switch is on
    // check indicator is a 1-bar signal icon and 4-bar wifi icon
    // check sim status shows correct carrier name with 1-bar signal icon.
    // check that AP list contains the connected AP at top then other APs underneath in alphabetical order.
    EXPECT_MATCHRESULT(mh::MenuMatcher(phoneParameters())
        .item(mh::MenuItemMatcher()
            .state_icons({"gsm-3g-low", "nm-signal-100"})
            .mode(mh::MenuItemMatcher::Mode::starts_with)
            .item(flightModeSwitch(false))
            .item(mh::MenuItemMatcher()
                .item(modemInfo("", "fake.tel", "gsm-3g-low"))
                .item(cellularSettings())
            )
            .item(wifiEnableSwitch(true))
            .item(mh::MenuItemMatcher()
                .item(accessPoint("GDF", Secure::insecure, ApMode::adhoc, ConnectionStatus::connected, 80))
                .item(accessPoint("ADS", Secure::insecure, ApMode::adhoc, ConnectionStatus::disconnected, 40))
                .item(accessPoint("CFT", Secure::insecure, ApMode::infra, ConnectionStatus::disconnected, 60))
                .item(accessPoint("DGN", Secure::secure, ApMode::infra, ConnectionStatus::disconnected, 40))
                .item(accessPoint("JDR", Secure::secure, ApMode::adhoc, ConnectionStatus::disconnected, 20))
                .item(accessPoint("JDY", Secure::secure, ApMode::adhoc, ConnectionStatus::disconnected, 60))
                .item(accessPoint("NSD", Secure::secure, ApMode::infra, ConnectionStatus::disconnected, 0))
                .item(accessPoint("SCE", Secure::insecure, ApMode::infra, ConnectionStatus::disconnected, 20))
            )
        ).match());

    // set flight mode on
    EXPECT_MATCHRESULT(mh::MenuMatcher(phoneParameters())
        .item(mh::MenuItemMatcher()
            .mode(mh::MenuItemMatcher::Mode::starts_with)
            .item(flightModeSwitch(false)
                  .activate()
            )
        ).match());

    setModemProperty(0, "Online", false);
    setGlobalConnectedState(NM_STATE_DISCONNECTED);
    removeActiveConnection(device, active_connection);
    removeWifiConnection(device, connection);
    removeAccessPoint(device, ap1);
    removeAccessPoint(device, ap2);
    removeAccessPoint(device, ap3);
    removeAccessPoint(device, ap4);
    removeAccessPoint(device, ap5);
    removeAccessPoint(device, ap6);
    removeAccessPoint(device, ap7);
    removeAccessPoint(device, ap8);

    // check that the wifi switch turns off
    // check indicator is a plane icon and a 0-bar wifi icon
    // check sim status shows “Offline” with 0-bar signal icon.
    // check that AP list is empty
    EXPECT_MATCHRESULT(mh::MenuMatcher(phoneParameters())
        .item(mh::MenuItemMatcher()
            .state_icons({"airplane-mode", "nm-no-connection"})
            .mode(mh::MenuItemMatcher::Mode::starts_with)
            .item(flightModeSwitch(true))
            .item(mh::MenuItemMatcher()
                .item(modemInfo("", "Offline", "gsm-3g-disabled"))
                .item(cellularSettings())
            )
            .item(wifiEnableSwitch(false))
            .item(mh::MenuItemMatcher()
                .is_empty()
            )
        ).match());


    // set wifi on
    auto& urfkillInterface = dbusMock.urfkillInterface();
    urfkillInterface.Block(1, true);
    EXPECT_MATCHRESULT(mh::MenuMatcher(phoneParameters())
        .item(mh::MenuItemMatcher()
            .mode(mh::MenuItemMatcher::Mode::starts_with)
              .item(flightModeSwitch(true))
              .item(mh::MenuItemMatcher())
              .item(wifiEnableSwitch(false)
                  .activate()
              )
        ).match());

    // NOTE: every newly created access point increments AP index (see: AccessPointItem::Private::ConstructL())
    //       so here we need to start at index 1+8 as we've had 8 APs previously.
    ap1 = createAccessPoint("9", "NSD", device, 0, Secure::secure, ApMode::infra);
    ap2 = createAccessPoint("10", "JDR", device, 20, Secure::secure, ApMode::adhoc);
    ap3 = createAccessPoint("11", "DGN", device, 40, Secure::secure, ApMode::infra);
    ap4 = createAccessPoint("12", "JDY", device, 60, Secure::secure, ApMode::adhoc);
    ap5 = createAccessPoint("13", "SCE", device, 20, Secure::insecure, ApMode::infra);
    ap6 = createAccessPoint("14", "ADS", device, 40, Secure::insecure, ApMode::adhoc);
    ap7 = createAccessPoint("15", "CFT", device, 60, Secure::insecure, ApMode::infra);
    ap8 = createAccessPoint("16", "GDF", device, 80, Secure::insecure, ApMode::adhoc);
    connection = createAccessPointConnection("16", "GDF", device);
    active_connection = createActiveConnection("16", device, connection, ap8);
    setGlobalConnectedState(NM_STATE_CONNECTED_GLOBAL);

    // check that the flight mode switch is still on
    // check that the wifi switch is on
    // check indicator is a plane icon and a 4-bar wifi icon
    // check sim status shows “Offline” with 0-bar signal icon.
    // check that AP list contains the connected AP highlighted at top then other APs underneath in alphabetical order.
    EXPECT_MATCHRESULT(mh::MenuMatcher(phoneParameters())
        .item(mh::MenuItemMatcher()
            .state_icons({"airplane-mode", "nm-signal-100"})
            .mode(mh::MenuItemMatcher::Mode::starts_with)
            .item(flightModeSwitch(true))
            .item(mh::MenuItemMatcher()
                .item(modemInfo("", "Offline", "gsm-3g-disabled"))
                .item(cellularSettings())
            )
            .item(wifiEnableSwitch(true))
            .item(mh::MenuItemMatcher()
                .item(accessPoint("GDF", Secure::insecure, ApMode::adhoc, ConnectionStatus::connected, 80))
                .item(accessPoint("ADS", Secure::insecure, ApMode::adhoc, ConnectionStatus::disconnected, 40))
                .item(accessPoint("CFT", Secure::insecure, ApMode::infra, ConnectionStatus::disconnected, 60))
                .item(accessPoint("DGN", Secure::secure, ApMode::infra, ConnectionStatus::disconnected, 40))
                .item(accessPoint("JDR", Secure::secure, ApMode::adhoc, ConnectionStatus::disconnected, 20))
                .item(accessPoint("JDY", Secure::secure, ApMode::adhoc, ConnectionStatus::disconnected, 60))
                .item(accessPoint("NSD", Secure::secure, ApMode::infra, ConnectionStatus::disconnected, 0))
                .item(accessPoint("SCE", Secure::insecure, ApMode::infra, ConnectionStatus::disconnected, 20))
            )
        ).match());

    // set flight mode off
    EXPECT_MATCHRESULT(mh::MenuMatcher(phoneParameters())
        .item(mh::MenuItemMatcher()
            .mode(mh::MenuItemMatcher::Mode::starts_with)
            .item(flightModeSwitch(true)
                  .activate()
            )
        ).match());

    setModemProperty(0, "Online", true);

    // check that the wifi switch remains on
    // check indicator is a 1-bar signal icon and 4-bar wifi icon
    // check sim status shows correct carrier name with 1-bar signal icon.
    // check that AP list contains the connected AP highlighted at top then other APs underneath in alphabetical order.
    EXPECT_MATCHRESULT(mh::MenuMatcher(phoneParameters())
        .item(mh::MenuItemMatcher()
              .state_icons({"gsm-3g-low", "nm-signal-100"})
              .mode(mh::MenuItemMatcher::Mode::starts_with)
              .item(flightModeSwitch(false))
              .item(mh::MenuItemMatcher()
                  .item(modemInfo("", "fake.tel", "gsm-3g-low"))
                  .item(cellularSettings())
            )
            .item(wifiEnableSwitch(true))
            .item(mh::MenuItemMatcher()
                .item(accessPoint("GDF", Secure::insecure, ApMode::adhoc, ConnectionStatus::connected, 80))
                .item(accessPoint("ADS", Secure::insecure, ApMode::adhoc, ConnectionStatus::disconnected, 40))
                .item(accessPoint("CFT", Secure::insecure, ApMode::infra, ConnectionStatus::disconnected, 60))
                .item(accessPoint("DGN", Secure::secure, ApMode::infra, ConnectionStatus::disconnected, 40))
                .item(accessPoint("JDR", Secure::secure, ApMode::adhoc, ConnectionStatus::disconnected, 20))
                .item(accessPoint("JDY", Secure::secure, ApMode::adhoc, ConnectionStatus::disconnected, 60))
                .item(accessPoint("NSD", Secure::secure, ApMode::infra, ConnectionStatus::disconnected, 0))
                .item(accessPoint("SCE", Secure::insecure, ApMode::infra, ConnectionStatus::disconnected, 20))
            )
        ).match());
}

TEST_F(TestIndicatorNetworkService, GroupedWiFiAccessPoints)
{
    // set wifi on, flight mode off
    setGlobalConnectedState(NM_STATE_DISCONNECTED);

    // create the wifi device
    auto device = createWiFiDevice(NM_DEVICE_STATE_ACTIVATED);

    // start the indicator
    ASSERT_NO_THROW(startIndicator());

    // add a single AP
    auto ap1 = createAccessPoint("1", "groupA", device, 40, Secure::secure, ApMode::infra);

    EXPECT_MATCHRESULT(mh::MenuMatcher(phoneParameters())
        .item(mh::MenuItemMatcher()
            .mode(mh::MenuItemMatcher::Mode::starts_with)
            .item(flightModeSwitch())
            .item(mh::MenuItemMatcher())
            .item(wifiEnableSwitch())
            .item(mh::MenuItemMatcher()
                .section()
                .item(accessPoint("groupA", Secure::secure, ApMode::infra, ConnectionStatus::disconnected, 40))
            )
        ).match());

    // add a second AP with the same SSID
    auto ap2 = createAccessPoint("2", "groupA", device, 60, Secure::secure, ApMode::infra);

    // check that we see a single AP with the higher strength
    EXPECT_MATCHRESULT(mh::MenuMatcher(phoneParameters())
        .item(mh::MenuItemMatcher()
            .mode(mh::MenuItemMatcher::Mode::starts_with)
            .item(flightModeSwitch())
            .item(mh::MenuItemMatcher())
            .item(wifiEnableSwitch())
            .item(mh::MenuItemMatcher()
                .section()
                .item(accessPoint("groupA", Secure::secure, ApMode::infra, ConnectionStatus::disconnected, 60))
            )
        ).match());

    // up the strength of the first AP
    setNmProperty(ap1, NM_DBUS_INTERFACE_ACCESS_POINT, "Strength", QVariant::fromValue(uchar(80)));

    // check that we see a single AP with the higher strength
    EXPECT_MATCHRESULT(mh::MenuMatcher(phoneParameters())
        .item(mh::MenuItemMatcher()
            .mode(mh::MenuItemMatcher::Mode::starts_with)
            .item(flightModeSwitch())
            .item(mh::MenuItemMatcher())
            .item(wifiEnableSwitch())
            .item(mh::MenuItemMatcher()
                .section()
                .item(accessPoint("groupA", Secure::secure, ApMode::infra, ConnectionStatus::disconnected, 80))
            )
        ).match());

    // add another AP with a different SSID
    auto ap3 = createAccessPoint("3", "groupB", device, 75, Secure::secure, ApMode::infra);

    // check that we see a single AP with the higher strength
    EXPECT_MATCHRESULT(mh::MenuMatcher(phoneParameters())
        .item(mh::MenuItemMatcher()
            .mode(mh::MenuItemMatcher::Mode::starts_with)
            .item(flightModeSwitch())
            .item(mh::MenuItemMatcher())
            .item(wifiEnableSwitch())
            .item(mh::MenuItemMatcher()
                .section()
                .item(accessPoint("groupA", Secure::secure, ApMode::infra, ConnectionStatus::disconnected, 80))
                .item(accessPoint("groupB", Secure::secure, ApMode::infra, ConnectionStatus::disconnected, 75))
            )
        ).match());

    // remove the first access point
    removeAccessPoint(device, ap1);

    // verify the list has the old combined access point and the strength matches the second ap initial strength
    EXPECT_MATCHRESULT(mh::MenuMatcher(phoneParameters())
        .item(mh::MenuItemMatcher()
            .mode(mh::MenuItemMatcher::Mode::starts_with)
            .item(flightModeSwitch())
            .item(mh::MenuItemMatcher())
            .item(wifiEnableSwitch())
            .item(mh::MenuItemMatcher()
                .section()
                .item(accessPoint("groupA", Secure::secure, ApMode::infra, ConnectionStatus::disconnected, 60))
                .item(accessPoint("groupB", Secure::secure, ApMode::infra, ConnectionStatus::disconnected, 75))
            )
        ).match());
}

TEST_F(TestIndicatorNetworkService, WifiStates_Connect1AP)
{
    // create a wifi device
    auto device = createWiFiDevice(NM_DEVICE_STATE_ACTIVATED);

    // start the indicator
    ASSERT_NO_THROW(startIndicator());

    // set wifi off
    EXPECT_MATCHRESULT(mh::MenuMatcher(phoneParameters())
        .item(mh::MenuItemMatcher()
            .mode(mh::MenuItemMatcher::Mode::starts_with)
              .item(flightModeSwitch(false))
              .item(mh::MenuItemMatcher())
              .item(wifiEnableSwitch(true)
                  .activate()
              )
        ).match());

    setGlobalConnectedState(NM_STATE_DISCONNECTED);

    // set no sim
    setSimManagerProperty(0, "Present", false);

    // check indicator is just a 0-bar wifi icon
    // check that AP list is empty
    EXPECT_MATCHRESULT(mh::MenuMatcher(phoneParameters())
        .item(mh::MenuItemMatcher()
            .state_icons({"nm-no-connection"})
            .mode(mh::MenuItemMatcher::Mode::starts_with)
            .item(flightModeSwitch(false))
            .item(mh::MenuItemMatcher()
                .item(modemInfo("", "No SIM", "no-simcard"))
                .item(cellularSettings())
            )
            .item(wifiEnableSwitch(false))
            .item(mh::MenuItemMatcher()
                .is_empty()
            )
        ).match());

    // set wifi switch on and add some APs (secure/unsecure/adhoc/varied strength)
    EXPECT_MATCHRESULT(mh::MenuMatcher(phoneParameters())
        .item(mh::MenuItemMatcher()
            .mode(mh::MenuItemMatcher::Mode::starts_with)
              .item(flightModeSwitch(false))
              .item(mh::MenuItemMatcher())
              .item(wifiEnableSwitch(false)
                  .activate()
              )
        ).match());

    auto ap1 = createAccessPoint("1", "NSD", device, 20, Secure::secure, ApMode::infra);
    auto ap2 = createAccessPoint("2", "JDR", device, 40, Secure::secure, ApMode::adhoc);
    auto ap3 = createAccessPoint("3", "DGN", device, 60, Secure::secure, ApMode::infra);
    auto ap4 = createAccessPoint("4", "JDY", device, 80, Secure::secure, ApMode::adhoc);
    auto ap5 = createAccessPoint("5", "SCE", device, 0, Secure::insecure, ApMode::infra);
    auto ap6 = createAccessPoint("6", "ADS", device, 20, Secure::insecure, ApMode::adhoc);
    auto ap7 = createAccessPoint("7", "CFT", device, 40, Secure::insecure, ApMode::infra);
    auto ap8 = createAccessPoint("8", "GDF", device, 60, Secure::insecure, ApMode::adhoc);
    setGlobalConnectedState(NM_STATE_CONNECTED_GLOBAL);

    // check indicator is still a 0-bar wifi icon
    // check that AP list contains available APs in alphabetical order (with correct signal and security icons).
    // check AP items have the correct associated action names.
    EXPECT_MATCHRESULT(mh::MenuMatcher(phoneParameters())
        .item(mh::MenuItemMatcher()
            .state_icons({"nm-signal-0"})
            .mode(mh::MenuItemMatcher::Mode::starts_with)
            .item(flightModeSwitch(false))
            .item(mh::MenuItemMatcher()
                .item(modemInfo("", "No SIM", "no-simcard"))
                .item(cellularSettings())
            )
            .item(wifiEnableSwitch(true))
            .item(mh::MenuItemMatcher()
                .item(accessPoint("ADS", Secure::insecure, ApMode::adhoc, ConnectionStatus::disconnected, 20)
                      .action("indicator.accesspoint.6"))
                .item(accessPoint("CFT", Secure::insecure, ApMode::infra, ConnectionStatus::disconnected, 40)
                      .action("indicator.accesspoint.7"))
                .item(accessPoint("DGN", Secure::secure, ApMode::infra, ConnectionStatus::disconnected, 60)
                      .action("indicator.accesspoint.3"))
                .item(accessPoint("GDF", Secure::insecure, ApMode::adhoc, ConnectionStatus::disconnected, 60)
                      .action("indicator.accesspoint.8"))
                .item(accessPoint("JDR", Secure::secure, ApMode::adhoc, ConnectionStatus::disconnected, 40)
                      .action("indicator.accesspoint.2"))
                .item(accessPoint("JDY", Secure::secure, ApMode::adhoc, ConnectionStatus::disconnected, 80)
                      .action("indicator.accesspoint.4"))
                .item(accessPoint("NSD", Secure::secure, ApMode::infra, ConnectionStatus::disconnected, 20)
                      .action("indicator.accesspoint.1"))
                .item(accessPoint("SCE", Secure::insecure, ApMode::infra, ConnectionStatus::disconnected, 0)
                      .action("indicator.accesspoint.5"))
            )
        ).match());

    // connect to 1-bar unsecure AP
    setGlobalConnectedState(NM_STATE_CONNECTING);
    auto connection = createAccessPointConnection("6", "ADS", device);
    auto active_connection = createActiveConnection("6", device, connection, ap6);
    setGlobalConnectedState(NM_STATE_CONNECTED_GLOBAL);

    // check indicator is just a 1-bar wifi icon
    // check that AP list contains the connected AP highlighted at top then other APs underneath in alphabetical order.
    EXPECT_MATCHRESULT(mh::MenuMatcher(phoneParameters())
        .item(mh::MenuItemMatcher()
            .state_icons({"nm-signal-25"})
            .mode(mh::MenuItemMatcher::Mode::starts_with)
            .item(flightModeSwitch(false)).item(mh::MenuItemMatcher()).item(wifiEnableSwitch(true))
            .item(mh::MenuItemMatcher()
                .item(accessPoint("ADS", Secure::insecure, ApMode::adhoc, ConnectionStatus::connected, 20))
                .item(accessPoint("CFT", Secure::insecure, ApMode::infra, ConnectionStatus::disconnected, 40))
                .item(accessPoint("DGN", Secure::secure, ApMode::infra, ConnectionStatus::disconnected, 60))
                .item(accessPoint("GDF", Secure::insecure, ApMode::adhoc, ConnectionStatus::disconnected, 60))
                .item(accessPoint("JDR", Secure::secure, ApMode::adhoc, ConnectionStatus::disconnected, 40))
                .item(accessPoint("JDY", Secure::secure, ApMode::adhoc, ConnectionStatus::disconnected, 80))
                .item(accessPoint("NSD", Secure::secure, ApMode::infra, ConnectionStatus::disconnected, 20))
                .item(accessPoint("SCE", Secure::insecure, ApMode::infra, ConnectionStatus::disconnected, 0))
            )
        ).match());

    // vary AP signal strength 0
    setNmProperty(ap6, NM_DBUS_INTERFACE_ACCESS_POINT, "Strength", QVariant::fromValue(uchar(0)));
    setGlobalConnectedState(NM_STATE_CONNECTING);
    setGlobalConnectedState(NM_STATE_CONNECTED_GLOBAL);

    // check indicator is a 0-bar wifi icon.
    // check that AP signal icon also updates accordingly.
    auto ap_item = mh::MenuItemMatcher::checkbox();
    EXPECT_MATCHRESULT(mh::MenuMatcher(phoneParameters())
        .item(mh::MenuItemMatcher()
            .state_icons({"nm-signal-0"})
            .mode(mh::MenuItemMatcher::Mode::starts_with)
            .item(flightModeSwitch(false)).item(mh::MenuItemMatcher()).item(wifiEnableSwitch(true))
            .item(mh::MenuItemMatcher()
                .item(accessPoint("ADS", Secure::insecure, ApMode::adhoc, ConnectionStatus::connected, 0))
                .item(ap_item).item(ap_item).item(ap_item).item(ap_item).item(ap_item).item(ap_item).item(ap_item)
            )
        ).match());

    // vary AP signal strength 40
    setNmProperty(ap6, NM_DBUS_INTERFACE_ACCESS_POINT, "Strength", QVariant::fromValue(uchar(40)));
    setGlobalConnectedState(NM_STATE_CONNECTING);
    setGlobalConnectedState(NM_STATE_CONNECTED_GLOBAL);

    // check indicator is a 2-bar wifi icon.
    // check that AP signal icon also updates accordingly.
    EXPECT_MATCHRESULT(mh::MenuMatcher(phoneParameters())
        .item(mh::MenuItemMatcher()
            .state_icons({"nm-signal-50"})
            .mode(mh::MenuItemMatcher::Mode::starts_with)
            .item(flightModeSwitch(false)).item(mh::MenuItemMatcher()).item(wifiEnableSwitch(true))
            .item(mh::MenuItemMatcher()
                .item(accessPoint("ADS", Secure::insecure, ApMode::adhoc, ConnectionStatus::connected, 40))
                .item(ap_item).item(ap_item).item(ap_item).item(ap_item).item(ap_item).item(ap_item).item(ap_item)
            )
        ).match());

    // vary AP signal strength 60
    setNmProperty(ap6, NM_DBUS_INTERFACE_ACCESS_POINT, "Strength", QVariant::fromValue(uchar(60)));
    setGlobalConnectedState(NM_STATE_CONNECTING);
    setGlobalConnectedState(NM_STATE_CONNECTED_GLOBAL);

    // check indicator is a 3-bar wifi icon.
    // check that AP signal icon also updates accordingly.
    EXPECT_MATCHRESULT(mh::MenuMatcher(phoneParameters())
        .item(mh::MenuItemMatcher()
            .state_icons({"nm-signal-75"})
            .mode(mh::MenuItemMatcher::Mode::starts_with)
            .item(flightModeSwitch(false)).item(mh::MenuItemMatcher()).item(wifiEnableSwitch(true))
            .item(mh::MenuItemMatcher()
                .item(accessPoint("ADS", Secure::insecure, ApMode::adhoc, ConnectionStatus::connected, 60))
                .item(ap_item).item(ap_item).item(ap_item).item(ap_item).item(ap_item).item(ap_item).item(ap_item)
            )
        ).match());

    // vary AP signal strength 80
    setNmProperty(ap6, NM_DBUS_INTERFACE_ACCESS_POINT, "Strength", QVariant::fromValue(uchar(80)));
    setGlobalConnectedState(NM_STATE_CONNECTING);
    setGlobalConnectedState(NM_STATE_CONNECTED_GLOBAL);

    // check indicator is a 4-bar wifi icon.
    // check that AP signal icon also updates accordingly.
    EXPECT_MATCHRESULT(mh::MenuMatcher(phoneParameters())
        .item(mh::MenuItemMatcher()
            .state_icons({"nm-signal-100"})
            .mode(mh::MenuItemMatcher::Mode::starts_with)
            .item(flightModeSwitch(false)).item(mh::MenuItemMatcher()).item(wifiEnableSwitch(true))
            .item(mh::MenuItemMatcher()
                .item(accessPoint("ADS", Secure::insecure, ApMode::adhoc, ConnectionStatus::connected, 80))
                .item(ap_item).item(ap_item).item(ap_item).item(ap_item).item(ap_item).item(ap_item).item(ap_item)
            )
        ).match());

    // set wifi off
    EXPECT_MATCHRESULT(mh::MenuMatcher(phoneParameters())
        .item(mh::MenuItemMatcher()
            .mode(mh::MenuItemMatcher::Mode::starts_with)
              .item(flightModeSwitch(false))
              .item(mh::MenuItemMatcher())
              .item(wifiEnableSwitch(true)
                  .activate()
              )
        ).match());

    setGlobalConnectedState(NM_STATE_DISCONNECTED);
    removeActiveConnection(device, active_connection);
    removeWifiConnection(device, connection);
    removeAccessPoint(device, ap1);
    removeAccessPoint(device, ap2);
    removeAccessPoint(device, ap3);
    removeAccessPoint(device, ap4);
    removeAccessPoint(device, ap5);
    removeAccessPoint(device, ap6);
    removeAccessPoint(device, ap7);
    removeAccessPoint(device, ap8);

    // check indicator is just a 0-bar wifi icon
    // check that AP list is empty
    EXPECT_MATCHRESULT(mh::MenuMatcher(phoneParameters())
        .item(mh::MenuItemMatcher()
            .state_icons({"nm-no-connection"})
            .mode(mh::MenuItemMatcher::Mode::starts_with)
            .item(flightModeSwitch(false)).item(mh::MenuItemMatcher()).item(wifiEnableSwitch(false))
            .item(mh::MenuItemMatcher()
                .is_empty()
            )
        ).match());
}

TEST_F(TestIndicatorNetworkService, WifiStates_Connect2APs)
{
    // set wifi on, flight mode off
    setGlobalConnectedState(NM_STATE_CONNECTED_GLOBAL);

    // set no sim
    setSimManagerProperty(0, "Present", false);

    // add some APs (secure / unsecure / adhoc / varied strength)
    auto device = createWiFiDevice(NM_DEVICE_STATE_ACTIVATED);
    auto ap1 = createAccessPoint("1", "NSD", device, 20, Secure::secure, ApMode::infra);
    auto ap2 = createAccessPoint("2", "JDR", device, 40, Secure::secure, ApMode::adhoc);
    auto ap3 = createAccessPoint("3", "DGN", device, 60, Secure::secure, ApMode::infra);
    auto ap4 = createAccessPoint("4", "JDY", device, 80, Secure::secure, ApMode::adhoc);
    auto ap5 = createAccessPoint("5", "SCE", device, 0, Secure::insecure, ApMode::infra);
    auto ap6 = createAccessPoint("6", "ADS", device, 20, Secure::insecure, ApMode::adhoc);
    auto ap7 = createAccessPoint("7", "CFT", device, 40, Secure::insecure, ApMode::infra);
    auto ap8 = createAccessPoint("8", "GDF", device, 60, Secure::insecure, ApMode::adhoc);

    // connect to 4-bar secure AP
    auto connection = createAccessPointConnection("4", "JDY", device);
    auto active_connection = createActiveConnection("4", device, connection, ap4);

    // check indicator is just a 4-bar locked wifi icon
    // check that AP list contains the connected AP highlighted at top then other APs underneath in alphabetical order.


    // vary AP signal strength 1..4
    // check indicator is a 1..4-bar locked wifi icon.
    // check that AP signal icon also updates accordingly.
    // connect to 3-bar unsecure AP
    // check indicator is just a 3-bar wifi icon
    // check that AP list contains the connected AP highlighted at top then other APs underneath in alphabetical order.
    // set wifi off
    // check indicator is just a 0-bar wifi icon
    // check that AP list is empty
    // set wifi on
    // check that the 4-bar secure AP is reconnected (as it has the highest signal).
    // check indicator is just a 4-bar locked wifi icon
    // check that AP list contains the connected AP highlighted at top then other APs underneath in alphabetical order.
}

} // namespace
