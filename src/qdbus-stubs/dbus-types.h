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

#pragma once

#include <QDBusConnection>
#include <QDBusMetaType>
#include <QDBusMessage>
#include <QtCore>
#include <QString>
#include <QVariantMap>

typedef QMap<QString, QVariantMap> QVariantDictMap;
Q_DECLARE_METATYPE(QVariantDictMap)

namespace DBusTypes
{
    inline void registerMetaTypes()
    {
        qRegisterMetaType<QVariantDictMap>("QVariantDictMap");

        qDBusRegisterMetaType<QVariantDictMap>();
    }

    inline QString vpnConnectionPath()
    {
        static int counter {0};
        static QString path{"/com/ubuntu/connectivity1/vpn/VpnConnection%1"};
        return path.arg(counter++);
    }

    inline void notifyPropertyChanged(
        const QDBusConnection& connection,
        const QObject& o,
        const QString& path,
        const QString& interface,
        const QStringList& propertyNames)
    {
        QDBusMessage signal = QDBusMessage::createSignal(
          path,
          "org.freedesktop.DBus.Properties",
          "PropertiesChanged");
        signal << interface;
        QVariantMap changedProps;
        for(const auto& propertyName: propertyNames)
        {
            changedProps.insert(propertyName, o.property(qPrintable(propertyName)));
        }
        signal << changedProps;
        signal << QStringList();
        connection.send(signal);
    }

    static constexpr char const* WPASUPPLICANT_DBUS_NAME = "fi.w1.wpa_supplicant1";

    static constexpr char const* WPASUPPLICANT_DBUS_INTERFACE = "fi.w1.wpa_supplicant1";

    static constexpr char const* WPASUPPLICANT_DBUS_PATH = "/fi/w1/wpa_supplicant1";

    static constexpr char const* POWERD_DBUS_NAME = "com.canonical.powerd";

    static constexpr char const* POWERD_DBUS_INTERFACE = "com.canonical.powerd";

    static constexpr char const* POWERD_DBUS_PATH = "/com/canonical/powerd";

    static constexpr char const* DBUS_NAME = "com.ubuntu.connectivity1";

    static constexpr char const* SERVICE_INTERFACE = "com.ubuntu.connectivity1.NetworkingStatus";

    static constexpr char const* PRIVATE_INTERFACE = "com.ubuntu.connectivity1.Private";

    static constexpr char const* SERVICE_PATH = "/com/ubuntu/connectivity1/NetworkingStatus";

    static constexpr char const* PRIVATE_PATH = "/com/ubuntu/connectivity1/Private";

    static constexpr char const* URFKILL_BUS_NAME = "org.freedesktop.URfkill";

    static constexpr char const* URFKILL_OBJ_PATH = "/org/freedesktop/URfkill";

    static constexpr char const* URFKILL_WIFI_OBJ_PATH = "/org/freedesktop/URfkill/WLAN";
    static constexpr char const* NOTIFY_DBUS_NAME = "org.freedesktop.Notifications";

    static constexpr char const* NOTIFY_DBUS_INTERFACE = "org.freedesktop.Notifications";

    static constexpr char const* NOTIFY_DBUS_PATH = "/org/freedesktop/Notifications";
}
