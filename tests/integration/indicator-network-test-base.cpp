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
#include <dbus-types.h>

using namespace QtDBusTest;
using namespace QtDBusMock;
using namespace std;
using namespace testing;
namespace mh = menuharness;

IndicatorNetworkTestBase::IndicatorNetworkTestBase() :
    dbusMock(dbusTestRunner)
{
}

IndicatorNetworkTestBase::~IndicatorNetworkTestBase()
{
}

void IndicatorNetworkTestBase::SetUp()
{
    if (qEnvironmentVariableIsSet("TEST_WITH_BUSTLE"))
    {
        const TestInfo* const test_info =
                UnitTest::GetInstance()->current_test_info();

        QDir::temp().mkpath("indicator-network-tests");
        QDir testDir(QDir::temp().filePath("indicator-network-tests"));

        dbusTestRunner.registerService(
                DBusServicePtr(
                        new QProcessDBusService(
                                "", QDBusConnection::SessionBus,
                                "/usr/bin/bustle-pcap",
                                QStringList{"-e", testDir.filePath(QString("%1-%2").arg(test_info->name(), "session.log"))})));
        dbusTestRunner.registerService(
                DBusServicePtr(
                        new QProcessDBusService(
                                "", QDBusConnection::SystemBus,
                                "/usr/bin/bustle-pcap",
                                QStringList{"-y", testDir.filePath(QString("%1-%2").arg(test_info->name(), "system.log"))})));
    }

    dbusMock.registerNetworkManager();
    dbusMock.registerNotificationDaemon();
    // By default the ofono mock starts with one modem
    dbusMock.registerOfono();
    dbusMock.registerURfkill();

    dbusMock.registerCustomMock(
                        "com.canonical.URLDispatcher",
                        "/com/canonical/URLDispatcher",
                        "com.canonical.URLDispatcher",
                        QDBusConnection::SessionBus);

    dbusTestRunner.startServices();

    // Set up a basic URL dispatcher mock
    auto& urlDispatcher = dbusMock.mockInterface(
                        "com.canonical.URLDispatcher",
                        "/com/canonical/URLDispatcher",
                        "com.canonical.URLDispatcher",
                        QDBusConnection::SessionBus);
    urlDispatcher.AddMethod(
                        "com.canonical.URLDispatcher",
                        "DispatchURL", "ss", "",
                        ""
                     ).waitForFinished();
    urlDispatcher.AddMethod(
                        "com.canonical.URLDispatcher",
                        "TestURL", "as", "as",
                        "ret = args[0]"
                     ).waitForFinished();

    // mock service creates ril_0 automatically
    // Initial ConnectionManager properties are insane, fix them here
    setConnectionManagerProperty(firstModem(), "Bearer", "none");
    setConnectionManagerProperty(firstModem(), "Powered", false);
    setConnectionManagerProperty(firstModem(), "Attached", false);

    // Identify the test when looking at Bustle logs
    QDBusConnection systemConnection = dbusTestRunner.systemConnection();
    systemConnection.registerService("org.TestIndicatorNetworkService");
    QDBusConnection sessionConnection = dbusTestRunner.sessionConnection();
    sessionConnection.registerService("org.TestIndicatorNetworkService");
}

mh::MenuMatcher::Parameters IndicatorNetworkTestBase::phoneParameters()
{
    return mh::MenuMatcher::Parameters(
            "com.canonical.indicator.network",
            { { "indicator", "/com/canonical/indicator/network" } },
            "/com/canonical/indicator/network/phone");
}

mh::MenuMatcher::Parameters IndicatorNetworkTestBase::unlockSimParameters(std::string const& busName, int exportId)
{
    return mh::MenuMatcher::Parameters(
            busName,
            { { "notifications", "/com/canonical/indicator/network/unlocksim" + to_string(exportId) } },
            "/com/canonical/indicator/network/unlocksim" + to_string(exportId));
}

void IndicatorNetworkTestBase::startIndicator()
{
    try
    {
        indicator.reset(
                new QProcessDBusService(DBusTypes::DBUS_NAME,
                                        QDBusConnection::SessionBus,
                                        NETWORK_SERVICE_BIN,
                                        QStringList()));
        indicator->start(dbusTestRunner.sessionConnection());
    }
    catch (exception const& e)
    {
        cout << "startIndicator(): " << e.what() << endl;
        throw;
    }
}

QString IndicatorNetworkTestBase::createWiFiDevice(int state, const QString& id)
{
    auto& networkManager(dbusMock.networkManagerInterface());
    auto reply = networkManager.AddWiFiDevice(id, "eth1", state);
    reply.waitForFinished();
    return reply;
}

QString IndicatorNetworkTestBase::randomMac()
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
        hardwareAddress.append(QString("%1").arg(r, 2, 16, QChar('0')));
        first = false;
    }

    return hardwareAddress;
}

void IndicatorNetworkTestBase::enableWiFi()
{
    auto& urfkillInterface = dbusMock.urfkillInterface();
    urfkillInterface.Block(1, false).waitForFinished();
}

void IndicatorNetworkTestBase::disableWiFi()
{
    auto& urfkillInterface = dbusMock.urfkillInterface();
    urfkillInterface.Block(1, true).waitForFinished();
}

QString IndicatorNetworkTestBase::createAccessPoint(const QString& id, const QString& ssid, const QString& device, uchar strength,
                          Secure secure, ApMode apMode, const QString& mac)
{
    int secflags(NM_802_11_AP_SEC_NONE);
    if (secure == Secure::insecure)
    {
        secflags = NM_802_11_AP_SEC_NONE;
    }
    else if (secure == Secure::wpa)
    {
        secflags = NM_802_11_AP_SEC_KEY_MGMT_PSK;
    }
    else if (secure == Secure::wpa_enterprise)
    {
        secflags = NM_802_11_AP_SEC_KEY_MGMT_802_1X;
    }

    auto& networkManager(dbusMock.networkManagerInterface());
    auto reply = networkManager.AddAccessPoint(
                        device, id, ssid,
                        mac,
                        apMode == ApMode::adhoc ? NM_802_11_MODE_ADHOC : NM_802_11_MODE_INFRA,
                        0, 0, strength,
                        secflags);
    reply.waitForFinished();
    return reply;
}

void IndicatorNetworkTestBase::removeAccessPoint(const QString& device, const QString& ap)
{
    auto& nm = dbusMock.networkManagerInterface();
    nm.RemoveAccessPoint(device, ap).waitForFinished();
}

QString IndicatorNetworkTestBase::createAccessPointConnection(const QString& id, const QString& ssid, const QString& device)
{
    auto& networkManager(dbusMock.networkManagerInterface());
    auto reply = networkManager.AddWiFiConnection(device, id, ssid,
                                                  "");
    reply.waitForFinished();
    return reply;
}

void IndicatorNetworkTestBase::removeWifiConnection(const QString& device, const QString& connection)
{
    auto& nm = dbusMock.networkManagerInterface();
    nm.RemoveWifiConnection(device, connection).waitForFinished();
}

QString IndicatorNetworkTestBase::createActiveConnection(const QString& id, const QString& device, const QString& connection, const QString& ap)
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

void IndicatorNetworkTestBase::removeActiveConnection(const QString& device, const QString& active_connection)
{
    auto& nm = dbusMock.networkManagerInterface();
    nm.RemoveActiveConnection(device, active_connection).waitForFinished();
}

void IndicatorNetworkTestBase::setGlobalConnectedState(int state)
{
    auto& nm = dbusMock.networkManagerInterface();
    nm.SetGlobalConnectionState(state).waitForFinished();
}

void IndicatorNetworkTestBase::setNmProperty(const QString& path, const QString& iface, const QString& name, const QVariant& value)
{
    auto& nm = dbusMock.networkManagerInterface();
    nm.SetProperty(path, iface, name, QDBusVariant(value)).waitForFinished();
}

QString IndicatorNetworkTestBase::createModem(const QString& id)
{
    auto& ofono(dbusMock.ofonoInterface());
    QVariantMap modemProperties {{ "Powered", false } };
    auto reply = ofono.AddModem(id, modemProperties);
    reply.waitForFinished();
    if (reply.isError()) {
        EXPECT_FALSE(reply.isError()) << reply.error().message().toStdString();
        return "";
    }
    QString path = reply.value();

    // Initial ConnectionManager properties are insane, fix them here
    setConnectionManagerProperty(path, "Bearer", "none");
    setConnectionManagerProperty(path, "Powered", false);
    setConnectionManagerProperty(path, "Attached", false);

    return path;
}

void IndicatorNetworkTestBase::setModemProperty(const QString& path, const QString& propertyName, const QVariant& value)
{
    auto& ofono(dbusMock.ofonoModemInterface(path));
    auto reply = ofono.SetProperty(propertyName, QDBusVariant(value));
    reply.waitForFinished();
    if (reply.isError()) {
        EXPECT_FALSE(reply.isError()) << reply.error().message().toStdString();
    }
}

void IndicatorNetworkTestBase::setSimManagerProperty(const QString& path, const QString& propertyName, const QVariant& value)
{
    auto& ofono(dbusMock.ofonoSimManagerInterface(path));
    auto reply = ofono.SetProperty(propertyName, QDBusVariant(value));
    reply.waitForFinished();
    if (reply.isError()) {
        EXPECT_FALSE(reply.isError()) << reply.error().message().toStdString();
    }
}

void IndicatorNetworkTestBase::setConnectionManagerProperty(const QString& path, const QString& propertyName, const QVariant& value)
{
    auto& ofono(dbusMock.ofonoConnectionManagerInterface(path));
    auto reply = ofono.SetProperty(propertyName, QDBusVariant(value));
    reply.waitForFinished();
    if (reply.isError()) {
        EXPECT_FALSE(reply.isError()) << reply.error().message().toStdString();
    }
}

void IndicatorNetworkTestBase::setNetworkRegistrationProperty(const QString& path, const QString& propertyName, const QVariant& value)
{
    auto& ofono(dbusMock.ofonoNetworkRegistrationInterface(path));
    auto reply = ofono.SetProperty(propertyName, QDBusVariant(value));
    reply.waitForFinished();
    if (reply.isError()) {
        EXPECT_FALSE(reply.isError()) << reply.error().message().toStdString();
    }
}

OrgFreedesktopDBusMockInterface& IndicatorNetworkTestBase::notificationsMockInterface()
{
    return dbusMock.mockInterface("org.freedesktop.Notifications",
                                   "/org/freedesktop/Notifications",
                                   "org.freedesktop.Notifications",
                                   QDBusConnection::SessionBus);
}

OrgFreedesktopDBusMockInterface& IndicatorNetworkTestBase::modemMockInterface(const QString& path)
{
    return dbusMock.mockInterface("org.ofono",
                                   path,
                                   "",
                                   QDBusConnection::SystemBus);
}

bool IndicatorNetworkTestBase::qDBusArgumentToMap(QVariant const& variant, QVariantMap& map)
{
    if (variant.canConvert<QDBusArgument>())
    {
        QDBusArgument value(variant.value<QDBusArgument>());
        if (value.currentType() == QDBusArgument::MapType)
        {
            value >> map;
            return true;
        }
    }
    return false;
}

QString IndicatorNetworkTestBase::firstModem()
{
    return "/ril_0";
}

mh::MenuItemMatcher IndicatorNetworkTestBase::flightModeSwitch(bool toggled)
{
    return mh::MenuItemMatcher::checkbox()
        .label("Flight Mode")
        .action("indicator.airplane.enabled")
        .toggled(toggled);
}

mh::MenuItemMatcher IndicatorNetworkTestBase::accessPoint(const string& ssid, Secure secure,
            ApMode apMode, ConnectionStatus connectionStatus, uchar strength)
{
    return mh::MenuItemMatcher::checkbox()
        .label(ssid)
        .widget("unity.widgets.systemsettings.tablet.accesspoint")
        .toggled(connectionStatus == ConnectionStatus::connected)
        .pass_through_attribute(
            "x-canonical-wifi-ap-strength-action",
            shared_ptr<GVariant>(g_variant_new_byte(strength), &mh::gvariant_deleter))
        .boolean_attribute("x-canonical-wifi-ap-is-secure", secure != Secure::insecure)
        .boolean_attribute("x-canonical-wifi-ap-is-enterprise", secure == Secure::wpa_enterprise)
        .boolean_attribute("x-canonical-wifi-ap-is-adhoc", apMode == ApMode::adhoc);
}

mh::MenuItemMatcher IndicatorNetworkTestBase::wifiEnableSwitch(bool toggled)
{
    return mh::MenuItemMatcher::checkbox()
        .label("Wi-Fi")
        .action("indicator.wifi.enable") // This action is accessed by system-settings-ui, do not change it
        .toggled(toggled);
}

mh::MenuItemMatcher IndicatorNetworkTestBase::wifiSettings()
{
    return mh::MenuItemMatcher()
        .label("Wi-Fi settings…")
        .action("indicator.wifi.settings");
}

mh::MenuItemMatcher IndicatorNetworkTestBase::modemInfo(const string& simIdentifier,
          const string& label,
          const string& statusIcon,
          bool locked,
          const string& connectivityIcon)
{
    return mh::MenuItemMatcher()
        .widget("com.canonical.indicator.network.modeminfoitem")
        .pass_through_string_attribute("x-canonical-modem-sim-identifier-label-action", simIdentifier)
        .pass_through_string_attribute("x-canonical-modem-connectivity-icon-action", connectivityIcon)
        .pass_through_string_attribute("x-canonical-modem-status-label-action", label)
        .pass_through_string_attribute("x-canonical-modem-status-icon-action", statusIcon)
        .pass_through_boolean_attribute("x-canonical-modem-roaming-action", false)
        .pass_through_boolean_attribute("x-canonical-modem-locked-action", locked);
}

mh::MenuItemMatcher IndicatorNetworkTestBase::cellularSettings()
{
    return mh::MenuItemMatcher()
        .label("Cellular settings…")
        .action("indicator.cellular.settings");
}

