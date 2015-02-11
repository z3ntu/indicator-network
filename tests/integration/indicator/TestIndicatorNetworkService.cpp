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
        dbusMock.registerNetworkManager();
        // By default the ofono mock starts with one modem
        dbusMock.registerOfono();
        dbusMock.registerURfkill();

        dbusMock.networkManagerInterface();

        dbusTestRunner.startServices();
    }

    void TearDown() override
    {
        QTestEventLoop::instance().enterLoopMSecs(500);
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
        return networkManager.AddWiFiDevice(id, "eth1", state);
    }

    QString createAccessPoint(const QString& id, const QString& ssid, const QString& device)
    {
        auto& networkManager(dbusMock.networkManagerInterface());
        return networkManager.AddAccessPoint(
                    device, id, ssid,
                    "11:22:33:44:55:66", NM_802_11_MODE_INFRA, 0, 0, 's',
                    NM_802_11_AP_SEC_KEY_MGMT_PSK);
    }

    QString createAccessPointConnection(const QString& id, const QString& ssid, const QString& device)
    {
        auto& networkManager(dbusMock.networkManagerInterface());
        return networkManager.AddWiFiConnection(
                device, id,
                ssid, "wpa-psk");
    }

    QString createActiveConnection(const QString& id, const QString& device, const QString& connection, const QString& ap)
    {
        auto& nm = dbusMock.networkManagerInterface();
        return nm.AddActiveConnection(QStringList() << device,
                               connection,
                               ap,
                               id,
                               NM_ACTIVE_CONNECTION_STATE_ACTIVATED);
    }

    void setGlobalConnectedState(int state)
    {
        auto& nm = dbusMock.networkManagerInterface();
        return nm.SetGlobalConnectionState(state).waitForFinished();
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

    static mh::MenuItemMatcher modemInfo(const string& simIdentifier, const string& label, const string& statusIcon)
    {
        return mh::MenuItemMatcher()
            .widget("com.canonical.indicator.network.modeminfoitem")
            .pass_through_string_attribute("x-canonical-modem-sim-identifier-label-action", simIdentifier)
            .pass_through_string_attribute("x-canonical-modem-connectivity-icon-action", "")
            .pass_through_string_attribute("x-canonical-modem-status-label-action", label)
            .pass_through_string_attribute("x-canonical-modem-status-icon-action", statusIcon)
            .pass_through_boolean_attribute("x-canonical-modem-roaming-action", false)
            .pass_through_boolean_attribute("x-canonical-modem-locked-action", false);
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
    startIndicator();

    EXPECT_MATCHRESULT(mh::MenuMatcher(phoneParameters())
        .item(mh::MenuItemMatcher()
            .action("indicator.phone.network-status")
            .mode(mh::MenuItemMatcher::Mode::all)
            .submenu()
            .item(flightModeSwitch())
            .item(mh::MenuItemMatcher()
                .section()
                .item(modemInfo("",
                                "fake.tel",
                                "gsm-3g-full")
                )
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

    startIndicator();

    EXPECT_MATCHRESULT(mh::MenuMatcher(phoneParameters())
        .item(mh::MenuItemMatcher()
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

    startIndicator();

    EXPECT_MATCHRESULT(mh::MenuMatcher(phoneParameters())
        .item(mh::MenuItemMatcher()
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

    startIndicator();
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

    startIndicator();

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
    auto& ofono(dbusMock.ofonoInterface());
    {
        QVariantMap modemProperties {{ "Powered", false } };
        ofono.AddModem("ril_1", modemProperties).waitForFinished();
    }

    startIndicator();

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
    startIndicator();

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
    startIndicator();

    EXPECT_MATCHRESULT(mh::MenuMatcher(phoneParameters())
        .item(mh::MenuItemMatcher()
            .mode(mh::MenuItemMatcher::Mode::starts_with)
            .submenu()
            .item(flightModeSwitch(false))
        ).match());

    ASSERT_TRUE(dbusMock.urfkillInterface().FlightMode(true));

    EXPECT_MATCHRESULT(mh::MenuMatcher(phoneParameters())
        .item(mh::MenuItemMatcher()
            .mode(mh::MenuItemMatcher::Mode::starts_with)
            .item(flightModeSwitch(true))
        ).match());
}

} // namespace
