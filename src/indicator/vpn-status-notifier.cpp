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
        static const QString FAILED_MESSAGE{_("The VPN connection '%1' failed.")};
        static const QMap<ActiveVpnConnection::Reason, QString> REASON_MAP{
            {ActiveVpnConnection::Reason::DEVICE_DISCONNECTED, _("The VPN connection '%1' failed because the network connection was interrupted.")},
            {ActiveVpnConnection::Reason::SERVICE_STOPPED, _("The VPN connection '%1' failed because the VPN service stopped unexpectedly.")},
            {ActiveVpnConnection::Reason::IP_CONFIG_INVALID, _("The VPN connection '%1' failed because the VPN service returned invalid configuration.")},
            {ActiveVpnConnection::Reason::CONNECT_TIMEOUT, _("The VPN connection '%1' failed because the connection attempt timed out.")},
            {ActiveVpnConnection::Reason::SERVICE_START_TIMEOUT, _("The VPN connection '%1' failed because the VPN service did not start in time.")},
            {ActiveVpnConnection::Reason::SERVICE_START_FAILED, _("The VPN connection '%1' failed because the VPN service failed to start.")},
            {ActiveVpnConnection::Reason::NO_SECRETS, _("The VPN connection '%1' failed because there were no valid VPN secrets.")},
            {ActiveVpnConnection::Reason::LOGIN_FAILED, _("The VPN connection '%1' failed because of invalid VPN secrets.")},
        };
        if (state == ActiveVpnConnection::State::FAILED
                || (state == ActiveVpnConnection::State::DISCONNECTED
                        && reason == ActiveVpnConnection::Reason::SERVICE_STOPPED))
        {
            auto activeVpnConnection = qobject_cast<ActiveVpnConnection*>(sender());
            QString id = activeVpnConnection->activeConnection().id();
            QString message = REASON_MAP.value(reason, FAILED_MESSAGE).arg(id);

            qDebug() << __PRETTY_FUNCTION__ << "VPN Connection Failed" << message;
            m_notificationManager->notify(_("VPN Connection Failed"), message, "network-vpn", {}, {}, 5)->show();
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
