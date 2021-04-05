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

#include <HostnameInterface.h>
#include <NetworkManager.h>
#include <NetworkManagerSettingsConnectionInterface.h>
#include <NetworkManagerSettingsInterface.h>

#include <QDebug>

#define G_SETTINGS_ENABLE_BACKEND
#include <gio/gsettingsbackend.h>

using namespace QtDBusTest;
using namespace QtDBusMock;
using namespace std;
using namespace testing;
using namespace connectivityqt;
namespace mh = lomiri::gmenuharness;

IndicatorNetworkTestBase::IndicatorNetworkTestBase() :
    dbusMock(dbusTestRunner)
{
    DBusTypes::registerMetaTypes();
}

IndicatorNetworkTestBase::~IndicatorNetworkTestBase()
{
}

void IndicatorNetworkTestBase::SetUp()
{
    qputenv("INDICATOR_NETWORK_SETTINGS_PATH", temporaryDir.path().toUtf8().constData());
    qputenv("INDICATOR_NETWORK_UNDER_TESTING", "1");
    qputenv("GSETTINGS_SCHEMA_DIR", INDICATOR_NETWORK_TESTING_GSETTINGS_SCHEMA_DIR);
    qputenv("INDICATOR_NETWORK_TESTING_GSETTINGS_INI", INDICATOR_NETWORK_TESTING_GSETTINGS_INI);

    if (QFile::exists(INDICATOR_NETWORK_TESTING_GSETTINGS_INI))
    {
        QFile::remove(INDICATOR_NETWORK_TESTING_GSETTINGS_INI);
    }
    QFile inifile(INDICATOR_NETWORK_TESTING_GSETTINGS_INI);
    inifile.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream out(&inifile);
    out << "[root]\ndata-usage-indication=false\n";
    inifile.close();

    if (qEnvironmentVariableIsSet("TEST_WITH_BUSTLE"))
    {
        QDir::temp().mkpath("indicator-network-tests");
        QDir testDir(QDir::temp().filePath("indicator-network-tests"));

        const TestInfo* const test_info =
                UnitTest::GetInstance()->current_test_info();

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

    registerDBusMocks();
    dbusMock.registerTemplate(DBusTypes::HOSTNAME_BUS_NAME, DBUSMOCK_TEMPLATE_DIR "hostname1.py", {}, QDBusConnection::SystemBus);
    dbusMock.registerTemplate(NM_DBUS_SERVICE, DBUSMOCK_TEMPLATE_DIR "networkmanager.py", {}, QDBusConnection::SystemBus);
    dbusMock.registerNotificationDaemon();
    // By default the ofono mock starts with one modem
    dbusMock.registerOfono({{"no_modem", true}});


    dbusMock.registerCustomMock(
                        DBusTypes::POWERD_DBUS_NAME,
                        DBusTypes::POWERD_DBUS_PATH,
                        DBusTypes::POWERD_DBUS_INTERFACE,
                        QDBusConnection::SystemBus);
    dbusMock.registerCustomMock(
                "com.canonical.Unity.Screen",
                "/com/canonical/Unity/Screen",
                "com.canonical.Unity.Screen",
                QDBusConnection::SystemBus);

    dbusMock.registerCustomMock(
                        DBusTypes::WPASUPPLICANT_DBUS_NAME,
                        DBusTypes::WPASUPPLICANT_DBUS_PATH,
                        DBusTypes::WPASUPPLICANT_DBUS_INTERFACE,
                        QDBusConnection::SystemBus);

    dbusMock.registerCustomMock(
                        "com.canonical.URLDispatcher",
                        "/com/canonical/URLDispatcher",
                        "com.canonical.URLDispatcher",
                        QDBusConnection::SessionBus);

    dbusTestRunner.startServices();

    setupDBusMocks();

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

    // Set up a basic WPA supplicant mock
    auto& wpaSupplicant = dbusMock.mockInterface(
                        DBusTypes::WPASUPPLICANT_DBUS_NAME,
                        DBusTypes::WPASUPPLICANT_DBUS_PATH,
                        DBusTypes::WPASUPPLICANT_DBUS_INTERFACE,
                        QDBusConnection::SystemBus);
    wpaSupplicant.AddMethod(
                        DBusTypes::WPASUPPLICANT_DBUS_INTERFACE,
                        "SetInterfaceFirmware",
                        "os",
                        "",
                        ""
                     ).waitForFinished();

    // Set up a basic powerd mock - only supports power states
    auto& powerd = dbusMock.mockInterface(
                        DBusTypes::POWERD_DBUS_NAME,
                        DBusTypes::POWERD_DBUS_PATH,
                        DBusTypes::POWERD_DBUS_INTERFACE,
                        QDBusConnection::SystemBus);
    powerd.AddMethod(
                        DBusTypes::POWERD_DBUS_INTERFACE,
                        "requestSysState",
                        "si",
                        "s",
                        "ret = 'dummy_cookie'"
                     ).waitForFinished();
    powerd.AddMethod(
                        DBusTypes::POWERD_DBUS_INTERFACE,
                        "clearSysState",
                        "s",
                        "",
                        ""
                     ).waitForFinished();


    // Identify the test when looking at Bustle logs
    QDBusConnection systemConnection = dbusTestRunner.systemConnection();
    systemConnection.registerService("org.TestIndicatorNetworkService");
    QDBusConnection sessionConnection = dbusTestRunner.sessionConnection();
    sessionConnection.registerService("org.TestIndicatorNetworkService");
}

void IndicatorNetworkTestBase::TearDown()
{
    if (QFile::exists(INDICATOR_NETWORK_TESTING_GSETTINGS_INI))
    {
        QFile::remove(INDICATOR_NETWORK_TESTING_GSETTINGS_INI);
    }
}

void IndicatorNetworkTestBase::setDataUsageIndicationSetting(bool value)
{
    GSettingsBackend *backend = g_keyfile_settings_backend_new(INDICATOR_NETWORK_TESTING_GSETTINGS_INI,
                                                               "/com/canonical/indicator/network/",
                                                               "root");
    GSettings *settings = g_settings_new_with_backend("com.canonical.indicator.network",
                                                      backend);
    g_settings_set_value (settings,
                          "data-usage-indication",
                          g_variant_new_boolean(value));
    g_settings_sync();
    g_object_unref(settings);
    g_object_unref(backend);
}


mh::MenuMatcher::Parameters IndicatorNetworkTestBase::phoneParameters()
{
    return mh::MenuMatcher::Parameters(
            "com.canonical.indicator.network",
            { { "indicator", "/com/canonical/indicator/network" } },
            "/com/canonical/indicator/network/phone");
}

mh::MenuMatcher::Parameters IndicatorNetworkTestBase::phoneEthernetSettingsParameters()
{
    return mh::MenuMatcher::Parameters(
            "com.canonical.indicator.network",
            { { "indicator", "/com/canonical/indicator/network" } },
            "/com/canonical/indicator/network/phone_ethernet_settings");
}

mh::MenuMatcher::Parameters IndicatorNetworkTestBase::phoneWifiSettingsParameters()
{
    return mh::MenuMatcher::Parameters(
            "com.canonical.indicator.network",
            { { "indicator", "/com/canonical/indicator/network" } },
            "/com/canonical/indicator/network/phone_wifi_settings");
}

mh::MenuMatcher::Parameters IndicatorNetworkTestBase::unlockSimParameters(std::string const& busName, int exportId)
{
    return mh::MenuMatcher::Parameters(
            busName,
            { { "notifications", "/com/canonical/indicator/network/unlocksim" + to_string(exportId) } },
            "/com/canonical/indicator/network/unlocksim" + to_string(exportId));
}

void
IndicatorNetworkTestBase::startIndicator()
{
    try
    {
        if (qEnvironmentVariableIsSet("TEST_WITH_GDB"))
        {
            indicator.reset(new QProcessDBusService(DBusTypes::DBUS_NAME, QDBusConnection::SessionBus, "/usr/bin/gdb", QStringList
                { "-batch", "-ex", "run", "-ex", "bt", NETWORK_SERVICE_BIN }));
        }
        else
        {
            indicator.reset(new QProcessDBusService(DBusTypes::DBUS_NAME, QDBusConnection::SessionBus,
                NETWORK_SERVICE_BIN, QStringList()));
        }
        indicator->start(dbusTestRunner.sessionConnection());
    }
    catch (exception const& e)
    {
        cout << "startIndicator(): " << e.what() << endl;
        throw;
    }
}

void IndicatorNetworkTestBase::setChassis(Chassis chassis)
{
    static const QMap<Chassis, QString> CHASSIS_MAP{
        {Chassis::desktop, "desktop"},
        {Chassis::laptop, "laptop"},
        {Chassis::server, "server"},
        {Chassis::tablet, "tablet"},
        {Chassis::handset, "handset"},
        {Chassis::vm, "vm"},
        {Chassis::container, "container"}
    };

    OrgFreedesktopHostname1Interface hostnameInterface(DBusTypes::HOSTNAME_BUS_NAME, DBusTypes::HOSTNAME_OBJ_PATH, dbusTestRunner.systemConnection());
    auto reply = hostnameInterface.SetChassis(CHASSIS_MAP[chassis], true);
    reply.waitForFinished();
    if (reply.isError())
    {
        EXPECT_FALSE(reply.isError()) << reply.error().message().toStdString();
    }
}

QString IndicatorNetworkTestBase::createEthernetDevice(int state, const QString& id)
{
    auto& networkManager(dbusMock.networkManagerInterface());
    auto reply = networkManager.AddEthernetDevice(id, "eth" + id, state);
    reply.waitForFinished();
    if (reply.isError())
    {
        EXPECT_FALSE(reply.isError()) << reply.error().message().toStdString();
    }
    QString path = reply;

    if (state == NM_DEVICE_STATE_ACTIVATED)
    {
        setNmProperty(path, NM_DBUS_INTERFACE_DEVICE, "Autoconnect", true);
    }

    return reply;
}

QString IndicatorNetworkTestBase::createEthernetConnection(const QString& name, const QString& device)
{
    auto& networkManager(dbusMock.networkManagerInterface());

    VariantDictMap settings{
        {"802-3-ethernet", QVariantMap{
            {"duplex", "full"},
            {"mac-address", randomMac().toUtf8()},
            {"mac-address-blacklist", QStringList()}
        }},
        {"connection", QVariantMap{
            {"id", name},
            {"interface-name", "ens33"},
            {"type", "802-3-ethernet"},
            {"permissions", QStringList()},
            {"secondaries", QStringList()}
        }}
    };
    auto reply = networkManager.AddDeviceConnection(device, settings);
    reply.waitForFinished();
    if (reply.isError())
    {
        EXPECT_FALSE(reply.isError()) << reply.error().message().toStdString();
    }
    return reply;
}

QString IndicatorNetworkTestBase::createWiFiDevice(int state, const QString& id)
{
    auto& networkManager(dbusMock.networkManagerInterface());
    auto reply = networkManager.AddWiFiDevice(id, "wlan" + id, state);
    reply.waitForFinished();
    if (reply.isError())
    {
        EXPECT_FALSE(reply.isError()) << reply.error().message().toStdString();
    }
    return reply;
}

QString IndicatorNetworkTestBase::createOfonoModemDevice(const QString &ofonoPath, const QString& id)
{
    auto& networkManager(dbusMock.networkManagerInterface());
    QList<QVariant> argumentList;
    argumentList << QVariant(QString("modem") + id) << QVariant(ofonoPath);
    QDBusPendingReply<QString> reply = networkManager.asyncCallWithArgumentList(QStringLiteral("AddOfonoModemDevice"), argumentList);
    reply.waitForFinished();
    if (reply.isError())
    {
        EXPECT_FALSE(reply.isError()) << reply.error().message().toStdString();
    }
    return reply;
}

void IndicatorNetworkTestBase::setDeviceStatistics(const QString &device, quint64 tx, quint64 rx)
{
    auto& networkManager(dbusMock.networkManagerInterface());
    QList<QVariant> argumentList;
    argumentList << QVariant(device) << QVariant(tx) << QVariant(rx);
    networkManager.asyncCallWithArgumentList(QStringLiteral("SetDeviceStatistics"), argumentList).waitForFinished();
}

quint32 IndicatorNetworkTestBase::getStatisticsRefreshRateMs(const QString &device)
{
    QDBusInterface iface(NM_DBUS_SERVICE,
                         device,
                         "org.freedesktop.DBus.Properties",
                          QDBusConnection::systemBus());

    QDBusPendingReply<QVariant> reply = iface.asyncCall("Get", "org.freedesktop.NetworkManager.Device.Statistics", "RefreshRateMs");
    reply.waitForFinished();
    if (reply.isError())
    {
        EXPECT_FALSE(reply.isError()) << reply.error().message().toStdString();
    }
    QVariant tmp = reply;
    return tmp.toInt();
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
    auto reply = urfkillInterface.Block(1, false);
    reply.waitForFinished();
    if (reply.isError())
    {
        EXPECT_FALSE(reply.isError()) << reply.error().message().toStdString();
    }
}

void IndicatorNetworkTestBase::disableWiFi()
{
    auto& urfkillInterface = dbusMock.urfkillInterface();
    auto reply = urfkillInterface.Block(1, true);
    reply.waitForFinished();
    if (reply.isError())
    {
        EXPECT_FALSE(reply.isError()) << reply.error().message().toStdString();
    }
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
    if (reply.isError())
    {
        EXPECT_FALSE(reply.isError()) << reply.error().message().toStdString();
    }
    return reply;
}

void IndicatorNetworkTestBase::removeAccessPoint(const QString& device, const QString& ap)
{
    auto& nm = dbusMock.networkManagerInterface();
    auto reply = nm.RemoveAccessPoint(device, ap);
    reply.waitForFinished();
    if (reply.isError())
    {
        EXPECT_FALSE(reply.isError()) << reply.error().message().toStdString();
    }
}

QString IndicatorNetworkTestBase::createAccessPointConnection(const QString& id, const QString& ssid, const QString& device)
{
    auto& networkManager(dbusMock.networkManagerInterface());
    auto reply = networkManager.AddWiFiConnection(device, id, ssid,
                                                  "");
    reply.waitForFinished();
    if (reply.isError())
    {
        EXPECT_FALSE(reply.isError()) << reply.error().message().toStdString();
    }
    return reply;
}

void IndicatorNetworkTestBase::removeWifiConnection(const QString& device, const QString& connection)
{
    auto& nm = dbusMock.networkManagerInterface();
    auto reply = nm.RemoveWifiConnection(device, connection);
    reply.waitForFinished();
    if (reply.isError())
    {
        EXPECT_FALSE(reply.isError()) << reply.error().message().toStdString();
    }
}

QString IndicatorNetworkTestBase::createActiveConnection(const QString& id, const QString& device, const QString& connection, const QString& specificObject)
{
    auto& nm = dbusMock.networkManagerInterface();
    auto reply = nm.AddActiveConnection(QStringList() << device,
                           connection,
                           specificObject,
                           id,
                           NM_ACTIVE_CONNECTION_STATE_ACTIVATED);
    reply.waitForFinished();
    if (reply.isError())
    {
        EXPECT_FALSE(reply.isError()) << reply.error().message().toStdString();
    }
    return reply;
}

void IndicatorNetworkTestBase::removeActiveConnection(const QString& device, const QString& active_connection)
{
    auto& nm = dbusMock.networkManagerInterface();
    auto reply = nm.RemoveActiveConnection(device, active_connection);
    reply.waitForFinished();
    if (reply.isError())
    {
        EXPECT_FALSE(reply.isError()) << reply.error().message().toStdString();
    }
}

void IndicatorNetworkTestBase::setGlobalConnectedState(int state)
{
    auto& nm = dbusMock.networkManagerInterface();
    auto reply = nm.SetGlobalConnectionState(state);
    reply.waitForFinished();
    if (reply.isError())
    {
        EXPECT_FALSE(reply.isError()) << reply.error().message().toStdString();
    }
}

void IndicatorNetworkTestBase::setNmProperty(const QString& path, const QString& iface, const QString& name, const QVariant& value)
{
    auto& nm = dbusMock.networkManagerInterface();
    auto reply = nm.SetProperty(path, iface, name, QDBusVariant(value));
    reply.waitForFinished();
    if (reply.isError())
    {
        EXPECT_FALSE(reply.isError()) << reply.error().message().toStdString();
    }
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

QVariantMap IndicatorNetworkTestBase::getConnectionManagerProperties(const QString& path)
{
    auto& ofono(dbusMock.ofonoConnectionManagerInterface(path));
    auto reply = ofono.GetProperties();
    reply.waitForFinished();
    if (reply.isError()) {
        EXPECT_FALSE(reply.isError()) << reply.error().message().toStdString();
    }
    return reply;
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

void IndicatorNetworkTestBase::setDisplayPowerState(DisplayPowerState value)
{
    // com.canonical.Unity.Screen interface only supports DisplayPowerStateChange signal
    // Interface does not implement proper properties - https://bugs.launchpad.net/ubuntu/+source/repowerd/+bug/1637722
    auto& unityscreen = dbusMock.mockInterface(
                            "com.canonical.Unity.Screen",
                            "/com/canonical/Unity/Screen",
                            "com.canonical.Unity.Screen",
                            QDBusConnection::SystemBus);

    QVariantList args;
    args << QVariant((qint32)(value)) << QVariant(qint32(0));
    unityscreen.EmitSignal("com.canonical.Unity.Screen", "DisplayPowerStateChange", "ii", args).waitForFinished();
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

OrgFreedesktopDBusMockInterface& IndicatorNetworkTestBase::networkManagerMockInterface()
{
    return dbusMock.mockInterface(NM_DBUS_SERVICE,
                                  NM_DBUS_PATH,
                                  NM_DBUS_INTERFACE,
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

mh::MenuItemMatcher IndicatorNetworkTestBase::mobileDataSwitch(bool toggled)
{
    return mh::MenuItemMatcher::checkbox()
        .label("Cellular data")
        .action("indicator.mobiledata.enabled")
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

mh::MenuItemMatcher IndicatorNetworkTestBase::ethernetInfo(const string& label,
                                                           const string& status,
                                                           Toggle autoConnect)
{
    return mh::MenuItemMatcher()
        .checkbox()
        .widget("com.canonical.indicator.switch")
        .toggled(autoConnect == Toggle::enabled)
        .label(label)
        .pass_through_string_attribute("x-canonical-subtitle-action", status);
}

mh::MenuItemMatcher IndicatorNetworkTestBase::radio(const string& label,
                                                    Toggle toggled)
{
    return mh::MenuItemMatcher()
        .radio()
        .label(label)
        .toggled(toggled == Toggle::enabled);
}

mh::MenuItemMatcher IndicatorNetworkTestBase::ethernetSettings()
{
    return mh::MenuItemMatcher()
        .label("Ethernet settings…")
        .action("indicator.ethernet.settings");
}

QString IndicatorNetworkTestBase::createVpnConnection(const QString& id,
                                                      const QString& serviceType,
                                                      const QStringMap& data,
                                                      const QStringMap& secrets)
{
    OrgFreedesktopNetworkManagerSettingsInterface settingsInterface(
            NM_DBUS_SERVICE, NM_DBUS_PATH_SETTINGS,
            dbusTestRunner.systemConnection());

    QVariantDictMap connection;
    connection["connection"] = QVariantMap {
        {"timestamp", 1441979296},
        {"type", "vpn"},
        {"id", id},
        {"uuid", QUuid::createUuid().toString().mid(1,36)}
    };
    connection["vpn"] = QVariantMap {
        {"service-type", serviceType},
        {"data", QVariant::fromValue(data)}
    };
    if (!secrets.isEmpty())
    {
        connection["vpn"]["secrets"] = QVariant::fromValue(secrets);
    }
    connection["ipv4"] = QVariantMap {
        {"routes", QStringList()},
        {"never-default", true},
        {"addresses", QStringList()},
        {"dns", QStringList()},
        {"method", "auto"}
    };
    connection["ipv6"] = QVariantMap {
        {"addresses", QStringList()},
        {"ip6-privacy", 0},
        {"dns", QStringList()},
        {"never-default", true},
        {"routes", QStringList()},
        {"method", "auto"}
    };
    auto reply = settingsInterface.AddConnection(connection);
    reply.waitForFinished();
    if (reply.isError())
    {
        EXPECT_FALSE(reply.isError()) << reply.error().message().toStdString();
    }
    QDBusObjectPath path(reply);
    return path.path();
}

void IndicatorNetworkTestBase::deleteSettings(const QString& path)
{
    OrgFreedesktopNetworkManagerSettingsConnectionInterface iface(NM_DBUS_SERVICE, path,
                                                                  dbusTestRunner.systemConnection());
    auto reply = iface.Delete();
    reply.waitForFinished();
    if (reply.isError())
    {
        EXPECT_FALSE(reply.isError()) << reply.error().message().toStdString();
    }
}

Connectivity::UPtr IndicatorNetworkTestBase::newConnectivity()
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

QVariantList IndicatorNetworkTestBase::getMethodCall(const QSignalSpy& spy, const QString& method)
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

mh::MenuItemMatcher IndicatorNetworkTestBase::vpnSettings()
{
    return mh::MenuItemMatcher()
        .label("VPN settings…")
        .action("indicator.vpn.settings");
}

mh::MenuItemMatcher IndicatorNetworkTestBase::vpnConnection(const string& name, ConnectionStatus connected)
{
    return mh::MenuItemMatcher::checkbox()
        .label(name)
        .themed_icon("icon", {"network-vpn"})
        .toggled(connected == ConnectionStatus::connected);
}

unique_ptr<QSortFilterProxyModel> IndicatorNetworkTestBase::getSortedModems(Connectivity& connectivity)
{
    auto modems = connectivity.modems();

    auto sortedModems = make_unique<QSortFilterProxyModel>();
    sortedModems->setSortRole(ModemsListModel::RoleIndex);
    sortedModems->sort(0);

    sortedModems->setSourceModel(modems);

    return sortedModems;
}
