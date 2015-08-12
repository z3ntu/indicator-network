/*
 * Copyright © 2015 Canonical Ltd.
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
 *     Pete Woods <pete.woods@canonical.com>
 */

#include <notification-manager.h>
#include <NotificationsInterface.h>
#include <QDebug>

using namespace std;

namespace notify
{

class NotificationManager::Priv
{
public:
    QString m_appName;

    shared_ptr<OrgFreedesktopNotificationsInterface> m_notificationInterface;
};

NotificationManager::NotificationManager(const QString &appName,
                                         const QDBusConnection& sessionConnection) :
        d(new Priv)
{
    d->m_appName = appName;
    d->m_notificationInterface = make_shared<
            OrgFreedesktopNotificationsInterface>(DBusTypes::NOTIFY_DBUS_NAME,
                                                  DBusTypes::NOTIFY_DBUS_PATH,
                                                  sessionConnection);

    d->m_notificationInterface->GetServerInformation();

    connect(d->m_notificationInterface.get(),
            &OrgFreedesktopNotificationsInterface::ActionInvoked, this,
            &NotificationManager::actionInvoked);

    connect(d->m_notificationInterface.get(),
            &OrgFreedesktopNotificationsInterface::NotificationClosed, this,
            &NotificationManager::notificationClosed);

    connect(d->m_notificationInterface.get(),
            &OrgFreedesktopNotificationsInterface::dataChanged, this,
            &NotificationManager::dataChanged);
}

NotificationManager::~NotificationManager()
{
}

Notification::UPtr
NotificationManager::notify(const QString &summary, const QString &body,
                            const QString &icon, const QStringList &actions,
                            const QVariantMap &hints, int expireTimeout)
{
    return make_unique<Notification>(d->m_appName, summary, body, icon, actions,
                                     hints, expireTimeout,
                                     d->m_notificationInterface);
}

}
