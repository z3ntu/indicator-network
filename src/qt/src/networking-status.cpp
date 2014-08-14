/*
 * Copyright © 2014 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License version 3,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors:
 *     Antti Kaijanmäki <antti.kaijanmaki@canonical.com>
 */

#include <ubuntu/connectivity/NetworkingStatus>
#include "dbus-properties-interface.h"

#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusInterface>
#include <QDBusServiceWatcher>
#include <QDBusMetaType>

#include <QDebug>

using namespace ubuntu::connectivity;

#define SERVICE_NAME "com.ubuntu.connectivity1"
#define SERVICE_INTERFACE "com.ubuntu.connectivity1.NetworkingStatus"
#define SERVICE_PATH "/com/ubuntu/connectivity1/NetworkingStatus"


class NetworkingStatus::Private : public QObject
{
    Q_OBJECT

public:
    // Yes, it's raw.
    NetworkingStatus *q;

    QDBusConnection m_bus;

    QVector<NetworkingStatus::Limitations> m_limitations;
    NetworkingStatus::Status m_status;

    QScopedPointer<QDBusServiceWatcher> m_watch;
    QScopedPointer<QDBusInterface> m_interface;
    QScopedPointer<DBusPropertiesInterface> m_properties_interface;

    Private() = delete;
    Private(NetworkingStatus *q)
        : q{q},
          m_bus{QDBusConnection::sessionBus()}
    {
        m_limitations = {};
        m_status = Status::Online;

        m_interface.reset(new QDBusInterface(SERVICE_NAME,
                                             SERVICE_PATH,
                                             SERVICE_INTERFACE,
                                             m_bus));
        m_properties_interface.reset(new DBusPropertiesInterface(SERVICE_NAME,
                                                                 SERVICE_PATH,
                                                                 m_bus));
        connect(m_properties_interface.data(),
                SIGNAL(PropertiesChanged(QString,QVariantMap,QStringList)),
                this,
                SLOT(propertiesChanged(QString,QVariantMap,QStringList)));

        m_watch.reset(new QDBusServiceWatcher(SERVICE_NAME,
                                              m_bus,
                                              QDBusServiceWatcher::WatchForOwnerChange));
        connect(m_watch.data(), &QDBusServiceWatcher::serviceOwnerChanged, this, &Private::serviceOwnerChanged);
        if (m_bus.interface()->isServiceRegistered(SERVICE_NAME)) {
            serviceOwnerChanged(SERVICE_NAME,
                                "",
                                m_bus.interface()->serviceOwner(SERVICE_NAME).value());
        } else {
            serviceOwnerChanged(SERVICE_NAME, "", "");
        }
    }

    void updateLimitations(QStringList values)
    {
        QVector<NetworkingStatus::Limitations> tmp;
        for (auto str : values) {
            if (str == "bandwith")
                tmp << NetworkingStatus::Limitations::Bandwith;
            else
                qWarning() << __PRETTY_FUNCTION__ << ": Invalid limitation: " << str;
        }
        if (m_limitations == tmp)
            return;
        m_limitations = tmp;
        emit q->limitationsChanged();
    }

    void updateStatus(QString value)
    {
        NetworkingStatus::Status tmp;
        if (value == "offline")
            tmp = NetworkingStatus::Status::Offline;
        else if (value == "connecting")
            tmp = NetworkingStatus::Status::Connecting;
        else if (value == "online")
            tmp = NetworkingStatus::Status::Online;
        else {
            qWarning() << __PRETTY_FUNCTION__ << ": Invalid status: " << value;
            return;
        }

        if (m_status == tmp)
            return;
        m_status = tmp;
        emit q->statusChanged(m_status);
    }

    void reset()
    {
        updateLimitations({});
        updateStatus("online");
    }

public Q_SLOTS:
    void serviceOwnerChanged(const QString &serviceName,
                             const QString &oldOwner,
                             const QString &newOwner)
    {
        Q_UNUSED(serviceName);
        Q_UNUSED(oldOwner);
        if (newOwner.isEmpty()) {
            // disappeared
            reset();
        } else {
            // appeared
            updateLimitations(m_interface->property("Limitations").toStringList());
            updateStatus(m_interface->property("Status").toString());
        }
    }

    void propertiesChanged(const QString &interface_name,
                           const QVariantMap &changed_properties,
                           const QStringList &invalidated_properties)
    {
        Q_UNUSED(interface_name);
        Q_UNUSED(invalidated_properties);

        if (changed_properties.contains("Limitations")) {
            updateLimitations(changed_properties["Limitations"].toStringList());
        }

        if (changed_properties.contains("Status")) {
            updateStatus(changed_properties["Status"].toString());
        }
    }
};

NetworkingStatus::NetworkingStatus(QObject *parent)
        : QObject(parent),
          d{new Private(this)}
{

    qRegisterMetaType<ubuntu::connectivity::NetworkingStatus::Limitations>();
    qRegisterMetaType<QVector<ubuntu::connectivity::NetworkingStatus::Limitations>>();
    qRegisterMetaType<ubuntu::connectivity::NetworkingStatus::Status>();
}

NetworkingStatus::~NetworkingStatus()
{}

QVector<NetworkingStatus::Limitations>
NetworkingStatus::limitations() const
{
    return d->m_limitations;
}

NetworkingStatus::Status
NetworkingStatus::status() const
{
    return d->m_status;
}

#include "networking-status.moc"
