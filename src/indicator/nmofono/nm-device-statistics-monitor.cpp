/*
 * Copyright (C) 2016 Canonical, Ltd.
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
 * Authors:
 *     Antti Kaijanmäki <antti.kaijanmaki@canonical.com>
 */

#include <nmofono/nm-device-statistics-monitor.h>

#include <NetworkManager.h>

#include <QGSettings>
#include <QDBusInterface>

#include <QMap>
#include <QDebug>

#include <NetworkManagerDeviceStatisticsInterface.h>

#include <gio/gio.h>
#define G_SETTINGS_ENABLE_BACKEND
#include <gio/gsettingsbackend.h>

#include "wifi/wifi-link-impl.h"
#include "wwan/modem.h"

using namespace std;

namespace nmofono
{

class NMDeviceStatisticsMonitor::Private : public QObject, public std::enable_shared_from_this<Private>
{
    Q_OBJECT

    NMDeviceStatisticsMonitor& p;

public:

    QMap<QString, std::shared_ptr<OrgFreedesktopNetworkManagerDeviceStatisticsInterface>> m_interfaces;
    QTimer m_txTimer;
    QTimer m_rxTimer;

    bool m_tx{false};
    bool m_rx{false};

    bool m_enabled{false};

    /***************/
    // This QDBusInterface magic is blatantly adapted from unity8: src/plugins/Powerd/Powerd.cpp
    // BTW: the service com.canonical.Unity.Screen is provided by lp:repowerd
    // BUGISH: no xml2qdbus goodness for us
    //         https://bugs.launchpad.net/ubuntu/+source/repowerd/+bug/1637730

    enum Status {
        Off,
        On,
    };

    QDBusInterface m_unityScreen{QStringLiteral("com.canonical.Unity.Screen"),
                                 QStringLiteral("/com/canonical/Unity/Screen"),
                                 QStringLiteral("com.canonical.Unity.Screen"),
                                 QDBusConnection::systemBus()};
    Status m_status {Status::On}; // fingers crossed..
                                  // BUG: https://bugs.launchpad.net/ubuntu/+source/repowerd/+bug/1637722
    /***************/

    Private(NMDeviceStatisticsMonitor& parent)
        : p(parent)
    {
        if (qgetenv("GSETTINGS_BACKEND") == "memory")
        {
            GSettingsBackend *backend = g_settings_backend_get_default();

            GError *error = NULL;
            GSettingsSchemaSource *schema_source;
            schema_source = g_settings_schema_source_new_from_directory (qgetenv("INDICATOR_NETWORK_TEST_GSETTINGS_SCHEMA_DIR").constData(),
                                                                         NULL,
                                                                         TRUE,
                                                                         &error);
            if (error) {
                qWarning("%s", error->message);
                g_error_free(error);
            }

            GSettingsSchema *schema;
            schema = g_settings_schema_source_lookup (schema_source,
                                                      "com.canonical.indicator.network", FALSE);

            GSettings *settings = g_settings_new_full (schema, backend, NULL);

            g_settings_set_value (settings,
                                  "data-usage-indication",
                                  g_variant_new_boolean(true));

            g_object_unref(backend);
            g_settings_schema_source_unref(schema_source);
            g_settings_schema_unref(schema);
            g_object_unref(settings);
        }

        QGSettings settings{"com.canonical.indicator.network"};
        m_enabled = settings.get("dataUsageIndication").toBool();

        m_unityScreen.connection().connect(QStringLiteral("com.canonical.Unity.Screen"),
                                           QStringLiteral("/com/canonical/Unity/Screen"),
                                           QStringLiteral("com.canonical.Unity.Screen"),
                                           QStringLiteral("DisplayPowerStateChange"),
                                           this,
                                           SLOT(handleDisplayPowerStateChange(int, int)));

        m_txTimer.setInterval(1000);
        m_txTimer.setSingleShot(true);
        connect(&m_rxTimer, &QTimer::timeout, this, &Private::txShot);

        m_rxTimer.setInterval(1000);
        m_rxTimer.setSingleShot(true);
        connect(&m_rxTimer, &QTimer::timeout, this, &Private::rxShot);
    }

    ~Private()
    {
        for (auto path : m_interfaces.keys())
        {
            resetInterface(path);
        }
    }

    void setTx(bool value)
    {
        if (m_tx == value)
        {
            return;
        }

        m_tx = value;
        Q_EMIT p.txChanged();
    }

    void setRx(bool value)
    {
        if (m_rx == value)
        {
            return;
        }

        m_rx = value;
        Q_EMIT p.rxChanged();
    }


    void setUpInterface(const QString &path)
    {
        if (!m_interfaces.contains(path))
        {
            return;
        }

        if (!m_enabled)
        {
            return;
        }

        auto iface = m_interfaces[path];

        connect(iface.get(), &OrgFreedesktopNetworkManagerDeviceStatisticsInterface::PropertiesChanged, this, &Private::propertiesChanged);
        iface->setRefreshRateMs(500);
    }

    void resetInterface(const QString &path)
    {
        if (!m_interfaces.contains(path))
        {
            return;
        }
        auto iface = m_interfaces[path];
        disconnect(iface.get(), &OrgFreedesktopNetworkManagerDeviceStatisticsInterface::PropertiesChanged, this, &Private::propertiesChanged);
        iface->setRefreshRateMs(0);
    }

    void connectAllInterfaces()
    {
        for (auto path : m_interfaces.keys())
        {
            setUpInterface(path);
        }
    }

    void disconnectAllInterfaces()
    {
        for (auto path : m_interfaces.keys())
        {
            resetInterface(path);
        }
    }


public Q_SLOTS:

    void handleDisplayPowerStateChange(int status, int reason)
    {
        Q_UNUSED(reason)

        if (m_status != (Status)status) {
            m_status = (Status)status;
            switch (m_status)
            {
            case Status::On:
            {
                connectAllInterfaces();
            }
            case Status::Off:
                disconnectAllInterfaces();
            }
        }
    }

    void propertiesChanged(const QVariantMap &properties)
    {
        if (properties.contains("TxBytes"))
        {
               setTx(true);
               m_txTimer.start();
        }

        if (properties.contains("RxBytes"))\
        {
            setRx(true);
            m_rxTimer.start();
        }
    }

    void txShot()
    {
        setTx(false);
    }

    void rxShot()
    {
        setRx(false);
    }


};


NMDeviceStatisticsMonitor::NMDeviceStatisticsMonitor()
    : d{new Private(*this)}
{
}

NMDeviceStatisticsMonitor::~NMDeviceStatisticsMonitor()
{

}

void
NMDeviceStatisticsMonitor::addLink(Link::Ptr link)
{
    QString path;

    if (std::dynamic_pointer_cast<wifi::WifiLinkImpl>(link))
    {
        path = std::dynamic_pointer_cast<wifi::WifiLinkImpl>(link)->device_path().path();
    }
    else if (std::dynamic_pointer_cast<wwan::Modem>(link))
    {
        path = std::dynamic_pointer_cast<wwan::Modem>(link)->nmPath();
    }
    else
    {
        qWarning() << "Unhandled link type with name:" << link->name();
    }

    if (path.isEmpty())
    {
        qWarning() <<  "Could not determine NMDevice path for link with name:" << link->name();
        return;
    }

    auto dev = make_shared<OrgFreedesktopNetworkManagerDeviceStatisticsInterface>(
                NM_DBUS_SERVICE,
                path,
                QDBusConnection::systemBus());

    d->m_interfaces[path] = dev;

    d->setUpInterface(path);
}

void
NMDeviceStatisticsMonitor::remove(const QString &nmPath)
{

    if (d->m_interfaces.contains(nmPath))
    {
        d->resetInterface(nmPath);
        d->m_interfaces.remove(nmPath);
    }
}

bool
NMDeviceStatisticsMonitor::tx() const
{
    return d->m_tx;
}

bool
NMDeviceStatisticsMonitor::rx() const
{
    return d->m_rx;
}

}

#include "nm-device-statistics-monitor.moc"
