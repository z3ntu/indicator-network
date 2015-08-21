/*
 * Copyright (C) 2014, 2015 Canonical, Ltd.
 *
 * Authors:
 *    Jussi Pakkanen <jussi.pakkanen@canonical.com>
 *    Jonas G. Drange <jonas.drange@canonical.com>
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3, as published
 * by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <nmofono/hotspot-manager.h>
#include <qpowerd/qpowerd.h>
#include <NetworkManagerActiveConnectionInterface.h>
#include <NetworkManagerDeviceInterface.h>
#include <NetworkManagerInterface.h>
#include <NetworkManagerSettingsInterface.h>
#include <NetworkManagerSettingsConnectionInterface.h>
#include <URfkillInterface.h>

#include <QStringList>
#include <QDBusReply>
#include <QtDebug>
#include <QDBusInterface>
#include <QDBusMetaType>
#include <NetworkManager.h>

using namespace std;

namespace
{
const static QString wpa_supplicant_service = "fi.w1.wpa_supplicant1";
const static QString wpa_supplicant_interface = "fi.w1.wpa_supplicant1";
const static QString wpa_supplicant_path = "/fi/w1/wpa_supplicant1";
}

class HotspotManager::Priv: public QObject
{

public:
    Priv(HotspotManager& parent) :
        p(parent)
    {
    }

    /**
     * Disables a hotspot.
     */
    void disable()
    {
        getHotspot();
        if (!m_hotspot)
        {
            qWarning() << "Could not find a hotspot setup to disable.\n";
            return;
        }

        // Get new settings
        QVariantDictMap new_settings = createConnectionSettings(m_ssid,
                                                                m_password,
                                                                m_mode, m_auth, false);
        auto updating = m_hotspot->Update(new_settings);
        updating.waitForFinished();
        if (!updating.isValid())
        {
            qCritical()
                    << "Could not update connection with autoconnect=false: "
                    << updating.error().message();
        }

        auto active_connections = m_manager->activeConnections();
        for (const auto &active_connection : active_connections)
        {
            OrgFreedesktopNetworkManagerConnectionActiveInterface iface(
                    NM_DBUS_SERVICE, active_connection.path(),
                    m_manager->connection());

            // Cast the property to a object path.
            QDBusObjectPath backingConnection = iface.connection();

            // This active connection's Connection property was our hotspot,
            // so we will deactivate it. Note that we do not remove the hotspot,
            // as we are storing the ssid, password and mode on the connection.
            if (backingConnection.path() == m_hotspot->path())
            {
                // Deactivate the connection.
                auto deactivation = m_manager->DeactivateConnection(
                        active_connection);
                deactivation.waitForFinished();
                if (!deactivation.isValid())
                {
                    qCritical() << "Could not get deactivate connection: "
                            << deactivation.error().message();
                }
                return;
            }
        }
    }

    bool destroy()
    {
        if (!m_hotspot)
        {
            qCritical() << __PRETTY_FUNCTION__ << "Failed to destroy hotspot because it didn't exist";
            return false;
        }

        // Subscribe to the connection proxy's Removed signal.
        connect(m_hotspot.get(),
                &OrgFreedesktopNetworkManagerSettingsConnectionInterface::Removed,
                this, &Priv::onRemoved);

        auto del = m_hotspot->Delete();
        del.waitForFinished();
        if (del.isError())
        {
            qWarning() << __PRETTY_FUNCTION__ << "Failed to delete connection:" << del.error().message();
    }
        return !del.isError();
    }

    void setStored(bool value)
    {
        if (m_stored != value)
        {
            m_stored = value;
            Q_EMIT p.storedChanged(value);
        }
    }

    void setEnable(bool value)
    {
        if (m_enabled != value)
        {
            m_enabled = value;
            // Request or clear the wakelock, depending on the hotspot state
            if (value)
            {
                m_wakelock = m_powerd->requestSysState(
                        "connectivity-service", QPowerd::SysPowerState::active);
            }
            else
            {
                m_wakelock.reset();
            }
            Q_EMIT p.enabledChanged(value);
        }
    }

    void updateSettingsFromDbus()
    {
        bool active = isHotspotActive();
        setEnable(active);

        QVariantDictMap settings = getConnectionSettings(*m_hotspot);
        const char wifi_key[] = "802-11-wireless";
        const char security_key[] = "802-11-wireless-security";

        if (settings.find(wifi_key) != settings.end())
        {
            QByteArray ssid = settings[wifi_key]["ssid"].toByteArray();
            if (!ssid.isEmpty())
            {
                p.setSsid(ssid);
            }

            QString mode = settings[wifi_key]["mode"].toString();
            if (!mode.isEmpty())
            {
                p.setMode(mode);
            }
        }

        QVariantDictMap secrets = getConnectionSecrets(*m_hotspot,
                                                       security_key);

        if (secrets.find(security_key) != secrets.end())
        {
            QString pwd = secrets[security_key]["psk"].toString();
            if (!pwd.isEmpty())
            {
                p.setPassword(pwd);
            }
        }
    }

    // wpa_supplicant interaction

    bool isHybrisWlan()
    {
        QString program("getprop");
        QStringList arguments;
        arguments << "urfkill.hybris.wlan";

        QProcess getprop;
        getprop.start(program, arguments);

        if (!getprop.waitForFinished())
        {
            qCritical() << "getprop process failed:" << getprop.errorString();
            return false;
        }

        int index = getprop.readAllStandardOutput().indexOf("1");

        // A non-negative integer means getprop returned 1
        return index >= 0;
    }

    /**
     * True if changed successfully, or there was no need. Otherwise false.
     * Supported modes are 'p2p', 'sta' and 'ap'.
     */
    bool changeInterfaceFirmware(const QString& interface, const QString& mode)
    {
        // Not supported.
        if (mode == "adhoc")
        {
            return true;
        }

        if (m_isHybrisWlan)
        {
            QDBusInterface wpasIface(wpa_supplicant_service,
                                     wpa_supplicant_path,
                                     wpa_supplicant_interface,
                                     m_manager->connection());

            const QDBusObjectPath interface_path(interface);

            // TODO(jgdx): We need to guard against calling this
            // when the interface is not soft blocked.
            auto set_interface = wpasIface.call(
                    "SetInterfaceFirmware", QVariant::fromValue(interface_path),
                    QVariant(mode));

            if (set_interface.type() == QDBusMessage::ErrorMessage)
            {
                qCritical() << "Failed to change interface firmware:"
                        << set_interface.errorMessage();
                return false;
            }
            else
            {
                return true;
            }
        }

        // We had no need to change the firmware.
        return true;
    }

    // wpa_supplicant interaction

    // UrfKill interaction

    /*
     * True if call went through and returned true.
     */
    bool setWifiBlock(bool block)
    {
        OrgFreedesktopURfkillInterface urfkill_dbus_interface(
                DBusTypes::URFKILL_BUS_NAME, DBusTypes::URFKILL_OBJ_PATH,
                m_manager->connection());

        const unsigned int device_type = 1; /* wifi type */
        auto reply = urfkill_dbus_interface.Block(device_type, block);
        reply.waitForFinished();

        if (reply.isError())
        {
            qCritical() << "Failed to block wifi" << reply.error().message();
            return false;
        }

        if (!reply)
        {
            qCritical() << "URfkill Block call did not succeed";
        }

        return reply;
    }

    // UrfKill interaction

    /**
     * Helper that maps QStrings to other QVariantMaps, i.e.
     * QMap<QString, QVariantMap>. QVariantMap is an alias for
     * QMap<QString, QVariant>.
     * See http://doc.qt.io/qt-5/qvariant.html#QVariantMap-typedef and
     * https://developer.gnome.org/NetworkManager/0.9/spec.html
     *     #type-String_String_Variant_Map_Map
     */
    QVariantDictMap createConnectionSettings(
        const QByteArray &ssid, const QString &password,
        QString mode, QString auth, bool autoConnect = true)
    {
        QVariantDictMap connection;

        QString s_ssid = QString::fromLatin1(ssid);
        QString s_uuid = QUuid().createUuid().toString();
        // Remove {} from the generated uuid.
        s_uuid.remove(0, 1);
        s_uuid.remove(s_uuid.size() - 1, 1);

        QVariantMap wireless;

        if (auth != "none")
        {
            wireless[QStringLiteral("security")] = QVariant(QStringLiteral("802-11-wireless-security"));
        }
        wireless[QStringLiteral("ssid")] = QVariant(ssid);
        wireless[QStringLiteral("mode")] = QVariant(mode);

        connection["802-11-wireless"] = wireless;

        QVariantMap connsettings;
        connsettings[QStringLiteral("autoconnect")] = QVariant(autoConnect);
        connsettings[QStringLiteral("id")] = QVariant(s_ssid);
        connsettings[QStringLiteral("uuid")] = QVariant(s_uuid);
        connsettings[QStringLiteral("type")] = QVariant(QStringLiteral("802-11-wireless"));
        connection["connection"] = connsettings;

        QVariantMap ipv4;
        ipv4[QStringLiteral("addressess")] = QVariant(QStringList());
        ipv4[QStringLiteral("dns")] = QVariant(QStringList());
        ipv4[QStringLiteral("method")] = QVariant(QStringLiteral("shared"));
        ipv4[QStringLiteral("routes")] = QVariant(QStringList());
        connection["ipv4"] = ipv4;

        QVariantMap ipv6;
        ipv6[QStringLiteral("method")] = QVariant(QStringLiteral("ignore"));
        connection["ipv6"] = ipv6;

        if (auth != "none")
        {
            QVariantMap security;
            security[QStringLiteral("proto")] = QVariant(QStringList{ "rsn" });
            security[QStringLiteral("pairwise")] = QVariant(QStringList{ "ccmp" });
            security[QStringLiteral("group")] = QVariant(QStringList{ "ccmp" });
            security[QStringLiteral("key-mgmt")] = QVariant(auth);
            security[QStringLiteral("psk")] = QVariant(password);
            connection["802-11-wireless-security"] = security;
        }

        return connection;
    }

    /**
     * Helper that returns a QMap<QString, QVariantMap> given a QDBusObjectPath.
     * See https://developer.gnome.org/NetworkManager/0.9/spec.html
     *     #org.freedesktop.NetworkManager.Settings.Connection.GetSettings
     */
    QVariantDictMap getConnectionSettings (OrgFreedesktopNetworkManagerSettingsConnectionInterface& conn) {
        auto connection_settings = conn.GetSettings();
        connection_settings.waitForFinished();
        return connection_settings.value();
    }


    /**
     * Helper that returns a QMap<QString, QVariantMap> given a QDBusObjectPath.
     * See https://developer.gnome.org/NetworkManager/0.9/spec.html
     *     #org.freedesktop.NetworkManager.Settings.Connection.GetSettings
     */
    QVariantDictMap getConnectionSecrets (OrgFreedesktopNetworkManagerSettingsConnectionInterface& conn,
        const QString key)
    {
        auto connection_secrets = conn.GetSecrets(key);
        connection_secrets.waitForFinished();
        return connection_secrets.value();
    }

    /**
     * Helper that adds a connection and returns the QDBusObjectPath
     * of the newly created connection.
     * See https://developer.gnome.org/NetworkManager/0.9/spec.html
     *     #org.freedesktop.NetworkManager.Settings.AddConnection
     */
    void addConnection()
    {
        QVariantDictMap connection = createConnectionSettings(m_ssid, m_password,
                                                              m_mode, m_auth);

        auto add_connection_reply = m_settings->AddConnection(connection);
        add_connection_reply.waitForFinished();

        if (add_connection_reply.isError())
        {
            qCritical() << "Failed to add connection: "
                    << add_connection_reply.error().message();
            m_hotspot.reset();
            return;
        }
        QDBusObjectPath connectionPath(add_connection_reply);

        m_hotspot = make_shared<
                OrgFreedesktopNetworkManagerSettingsConnectionInterface>(
                NM_DBUS_SERVICE, connectionPath.path(), m_manager->connection());
    }

    /**
     * Returns a QDBusObjectPath of a hotspot given a mode.
     * Valid modes are 'p2p', 'ap' and 'adhoc'.
     */
    void getHotspot()
    {
        const char wifi_key[] = "802-11-wireless";

        auto listed_connections = m_settings->ListConnections();
        listed_connections.waitForFinished();

        for (const auto &connection : listed_connections.value())
        {
            auto conn = make_shared<OrgFreedesktopNetworkManagerSettingsConnectionInterface>(
                    NM_DBUS_SERVICE, connection.path(),
                    m_manager->connection());

            auto connection_settings = getConnectionSettings(*conn);

            if (connection_settings.find(wifi_key) != connection_settings.end())
            {
                auto wifi_setup = connection_settings[wifi_key];
                QString wifi_mode = wifi_setup["mode"].toString();

                if (wifi_mode == m_mode)
                {
                    m_hotspot = conn;
                    return;
                }
            }
        }
        m_hotspot.reset();
    }

    /**
     * Helper to check if the hotspot on a given QDBusObjectPath is active
     * or not. It checks if the Connection.Active [1] for the given
     * path is in NetworkManager's ActiveConnections property [2].
     * [1] https://developer.gnome.org/NetworkManager/0.9/spec.html
     *     #org.freedesktop.NetworkManager.Connection.Active
     * [2] https://developer.gnome.org/NetworkManager/0.9/spec.html
     *     #org.freedesktop.NetworkManager
     */
    bool isHotspotActive ()
    {
        if (!m_hotspot)
        {
            return false;
        }

        QSet<QDBusObjectPath> active_relevant_connections;
        auto active_connections = m_manager->activeConnections();
        for (const auto &active_connection : active_connections)
        {

            // Active connection interface proxy. It might have a connection
            // property we can use to deduce if this active connection represents
            // our active hotspot.
            OrgFreedesktopNetworkManagerConnectionActiveInterface active_connection_dbus_interface(
                    NM_DBUS_SERVICE, active_connection.path(),
                    m_manager->connection());

            // Get the Connection property, if any.
            auto connection_path =
                    active_connection_dbus_interface.connection();

            // The object path is the same as the given hotspot path.
            if (m_hotspot->path() == connection_path.path())
            {
                return true;
            }
        }

        // No active connection had a Connection property equal to the
        // given path, so return false.
        return false;
    }

    void generatePassword()
    {
        static const std::string items("abcdefghijklmnopqrstuvwxyz01234567890");
        const int password_length = 8;
        std::string result;

        for (int i = 0; i < password_length; i++)
        {
            result.push_back(items[std::rand() % items.length()]);
        }

        m_password = QString::fromStdString(result);
    }

    void setDisconnectWifi(bool disconnect)
    {
        if (!m_isHybrisWlan)
        {
            return;
        }

        if (m_disconnectWifi == disconnect)
        {
            return;
        }

        m_disconnectWifi = disconnect;
        Q_EMIT p.disconnectWifiChanged(m_disconnectWifi);
    }

public Q_SLOTS:
    void onNewConnection(const QDBusObjectPath& path)
    {
        if (!m_hotspot)
        {
            return;
        }

        // The new connection is the same as the stored hotspot path.
        if (path.path() == m_hotspot->path())
        {
            // If a new hotspot was added, it is also given that
            // Wi-Fi has been soft blocked. We can now unblock Wi-Fi
            // and let NetworkManager pick up the newly created
            // connection.
            bool unblocked = setWifiBlock(false);
            if (!unblocked)
            {
                // "The device could not be readied for configuration"
                Q_EMIT p.reportError(5);
            }
            else
            {
                // We successfully unblocked the Wi-Fi, so set m_enable to true.
                setEnable(true);
            }

            // This also mean we have successfully created a hotspot connection
            // object in NetworkManager, so m_stored should now be true.
            setStored(true);
        }
    }


    void onRemoved()
    {
        m_hotspot.reset();

        // The UI does not support direct deletion of a hotspot, and given how
        // hotspots currently work, every time we want to re-use a hotspot, we
        // delete it an add a new one.
        // Thus, if a hotspot was deleted, we now create a new one.
        addConnection();

        // We could not add a connection, so report, disable and unblock.
        if (!m_hotspot)
        {
            qWarning() << __PRETTY_FUNCTION__ << "  Adding connection failed";
            // Emit "Unknown Error".
            Q_EMIT p.reportError(0);
            setEnable(false);
            setWifiBlock(false);
        }
    }

    void onPropertiesChanged(const QVariantMap& properties)
    {
        // If we have no hotspot path, ignore changes in NetworkManager.
        if (!m_hotspot)
        {
            return;
        }

        // Set flag so we know that ActiveConnections changed.
        bool active_connection_changed = false;

        for (QVariantMap::const_iterator iter = properties.begin();
                iter != properties.end(); ++iter)
        {
            if (iter.key() == "ActiveConnections")
            {
                active_connection_changed = true;

                const QDBusArgument args = qvariant_cast<QDBusArgument>(
                        iter.value());
                if (args.currentType() == QDBusArgument::ArrayType)
                {
                    args.beginArray();

                    while (!args.atEnd())
                    {
                        QDBusObjectPath path = qdbus_cast<QDBusObjectPath>(
                                args);

                        OrgFreedesktopNetworkManagerConnectionActiveInterface active_connection_dbus_interface(
                                NM_DBUS_SERVICE, path.path(),
                                m_manager->connection());

                        QDBusObjectPath connection_path =
                                active_connection_dbus_interface.connection();

                        // We see our connection as being active, so we emit that is
                        // enabled and return.
                        if (connection_path.path() == m_hotspot->path())
                        {
                            setEnable(true);
                            return;
                        }
                    }
                    args.endArray();
                }
            }
        }

        // At this point ActiveConnections changed, but
        // our hotspot was not in that list.
        if (active_connection_changed)
        {
            setEnable(false);
        }
    }

    // NetworkManager interaction

public:
    HotspotManager& p;

    QString m_mode = "ap";
    QString m_auth = "wpa-psk";
    bool m_enabled = false;
    bool m_stored = false;
    QString m_password;
    QByteArray m_ssid = "Ubuntu";

    QPowerd::UPtr m_powerd;
    QPowerd::RequestSPtr m_wakelock;

    bool m_isHybrisWlan = false;
    bool m_disconnectWifi = false;

    /**
     * NetworkManager dbus interface proxy we will use to query
     * against NetworkManager. See
     * https://developer.gnome.org/NetworkManager/0.9/spec.html
     *     #org.freedesktop.NetworkManager
     */
    unique_ptr<OrgFreedesktopNetworkManagerInterface> m_manager;

    /**
     * NetworkManager Settings interface proxy we use to get
     * the list of connections, as well as adding connections.
     * See https://developer.gnome.org/NetworkManager/0.9/spec.html
     *     #org.freedesktop.NetworkManager.Settings
     */
    unique_ptr<OrgFreedesktopNetworkManagerSettingsInterface> m_settings;

    shared_ptr<OrgFreedesktopNetworkManagerSettingsConnectionInterface> m_hotspot;
};

HotspotManager::HotspotManager(const QDBusConnection& connection, QObject *parent) :
        QObject(parent), d(new Priv(*this))
{
    d->m_isHybrisWlan = d->isHybrisWlan();

    d->m_manager = make_unique<OrgFreedesktopNetworkManagerInterface>(
            NM_DBUS_SERVICE, NM_DBUS_PATH, connection);
    d->m_settings = make_unique<OrgFreedesktopNetworkManagerSettingsInterface>(
            NM_DBUS_SERVICE, NM_DBUS_PATH_SETTINGS, connection);

    d->m_powerd = make_unique<QPowerd>(connection);

    d->generatePassword();

    // Stored is false if hotspot path is empty.
    d->getHotspot();
    d->setStored(bool(d->m_hotspot));

    if (d->m_stored)
    {
        d->updateSettingsFromDbus();
    }

    // Watches for new connections added to NetworkManager's Settings
    // interface.
    connect(d->m_settings.get(),
            &OrgFreedesktopNetworkManagerSettingsInterface::NewConnection,
            d.get(), &Priv::onNewConnection);

    // Watches changes in NetworkManager
    connect(d->m_manager.get(),
            &OrgFreedesktopNetworkManagerInterface::PropertiesChanged, d.get(),
            &Priv::onPropertiesChanged);
}

void HotspotManager::setEnabled(bool value)
{
    if (enabled() == value)
    {
        return;
    }

    bool blocked = d->setWifiBlock(true);

    // Failed to soft block, here we revert the enabled setting.
    if (!blocked)
    {
        // "The device could not be readied for configuration"
        Q_EMIT reportError(5);
        d->setEnable(false);
        return;
    }

    d->setDisconnectWifi(value);

    // We are enabling a hotspot
    if (value)
    {
        // If the SSID is empty, we report an error.
        if (d->m_ssid.isEmpty())
        {
            qWarning() << __PRETTY_FUNCTION__ << "  SSID was empty";
            Q_EMIT reportError(1);
            d->setEnable(false);
            return;
        }

        bool changed = d->changeInterfaceFirmware("/", d->m_mode);
        if (!changed)
        {
            // Necessary firmware for the device may be missing
            qWarning() << __PRETTY_FUNCTION__ << "  Could not change interface firmware";
            Q_EMIT reportError(35);
            d->setEnable(false);
            d->setWifiBlock(false);
            return;
        }

        if (d->m_stored)
        {
            // we defer enabling until old hotspot is deleted
            // if we can delete the old one
            // If not, unset stored flag and call this method.
            if (!d->destroy())
            {
                d->setStored(false);
                // recurse?
                setEnabled(true);
                d->setEnable(true);
            }
        }
        else
        {
            // we defer enabling until new hotspot is created
            d->addConnection();
            if (!d->m_hotspot)
            {
                // Emit "Unknown Error".
                qWarning() << __PRETTY_FUNCTION__ << "  Could not create hotspot connection";
                Q_EMIT reportError(0);
                d->setEnable(false);
                d->setWifiBlock(false);
            }
        }

    }
    else
    {
        // Disabling the hotspot.
        d->disable();
        d->setEnable(false);

        bool unblocked = d->setWifiBlock(false);
        if (!unblocked)
        {
            qWarning() << __PRETTY_FUNCTION__ << "  Could not unblock WiFi";
            // "The device could not be readied for configuration"
            Q_EMIT reportError(5);
        }
    }

}

bool HotspotManager::enabled() const {
    return d->m_enabled;
}

bool HotspotManager::stored() const {
    return d->m_stored;
}

QByteArray HotspotManager::ssid() const {
    return d->m_ssid;
}

void HotspotManager::setSsid(const QByteArray& value) {
    if (d->m_ssid != value)
    {
        d->m_ssid = value;
        Q_EMIT ssidChanged(value);
    }
}

QString HotspotManager::password() const {
    return d->m_password;
}

void HotspotManager::setPassword(const QString& value) {
    if (d->m_password != value)
    {
        d->m_password = value;
        Q_EMIT passwordChanged(value);
    }
}

QString HotspotManager::mode() const {
    return d->m_mode;
}

QString HotspotManager::auth() const {
    return d->m_auth;
}

void HotspotManager::setMode(const QString& value) {
    if (d->m_mode != value)
    {
        d->m_mode = value;
        Q_EMIT modeChanged(value);
    }
}

void HotspotManager::setAuth(const QString& value) {
    if (d->m_auth != value)
    {
        d->m_auth = value;
        Q_EMIT authChanged(value);
    }
}

bool HotspotManager::disconnectWifi() const
{
    return d->m_disconnectWifi;
}
