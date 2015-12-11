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
 * Authors:
 *     Pete Woods <pete.woods@canonical.com>
 */

#include <vpn-status-notifier.h>
#include <util/localisation.h>

#include <QDebug>
#include <QMap>
#include <QObject>

using namespace nmofono::connection;
using namespace notify;

class VpnStatusNotifier::Priv: public QObject
{
    Q_OBJECT

public Q_SLOTS:
    void connectionsChanged(const QSet<ActiveConnection::SPtr>& connections)
    {
        for (const auto& connection: connections)
        {
            if (connection->type() == "vpn")
            {
                connect(connection->vpnConnection().get(), &ActiveVpnConnection::stateChanged, this, &Priv::vpnStateChanged, Qt::UniqueConnection);
            }
        }
    }

    void vpnStateChanged(ActiveVpnConnection::State state, ActiveVpnConnection::Reason reason)
    {
        static const QMap<ActiveVpnConnection::Reason, QString> REASON_MAP{
            {ActiveVpnConnection::Reason::UNKNOWN, _("Unknown reason")},
            {ActiveVpnConnection::Reason::NONE, _("No reason")},
            {ActiveVpnConnection::Reason::DISCONNECTED, _("Disconnected")},
            {ActiveVpnConnection::Reason::DEVICE_DISCONNECTED, _("Device disconnected")},
            {ActiveVpnConnection::Reason::SERVICE_STOPPED, _("VPN service stopped")},
            {ActiveVpnConnection::Reason::IP_CONFIG_INVALID, _("IP configuration invalid")},
            {ActiveVpnConnection::Reason::CONNECT_TIMEOUT, _("Connection timeout")},
            {ActiveVpnConnection::Reason::SERVICE_START_TIMEOUT, _("VPN service start timeout")},
            {ActiveVpnConnection::Reason::SERVICE_START_FAILED, _("VPN service start failed")},
            {ActiveVpnConnection::Reason::NO_SECRETS, _("Required password not provided")},
            {ActiveVpnConnection::Reason::LOGIN_FAILED, _("Login failed")},
            {ActiveVpnConnection::Reason::CONNECTION_REMOVED, _("Connection removed")}
        };
        if (state == ActiveVpnConnection::State::FAILED)
        {
            qDebug() << __PRETTY_FUNCTION__ << "VPN connection failed" << REASON_MAP[reason];
            m_notificationManager->notify(_("VPN connection failed"), REASON_MAP[reason], "network-vpn", {}, {}, 5)->show();
        }
    }

public:
    ActiveConnectionManager::SPtr m_activeConnectionManager;

    NotificationManager::SPtr m_notificationManager;
};

VpnStatusNotifier::VpnStatusNotifier(ActiveConnectionManager::SPtr activeConnectionManager, NotificationManager::SPtr notificationManager) :
        d(new Priv)
{
    d->m_activeConnectionManager = activeConnectionManager;
    d->m_notificationManager = notificationManager;

    QObject::connect(activeConnectionManager.get(), &ActiveConnectionManager::connectionsChanged, d.get(), &Priv::connectionsChanged);

    d->connectionsChanged(d->m_activeConnectionManager->connections());
}

VpnStatusNotifier::~VpnStatusNotifier()
{
}

#include "vpn-status-notifier.moc"
