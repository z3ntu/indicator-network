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

#pragma once

#include <memory>
#include <QDBusConnection>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariantMap>

#include <notify-cpp/notification.h>

namespace notify
{

class NotificationManager: public QObject
{
    Q_OBJECT

public:
    typedef std::unique_ptr<NotificationManager> UPtr;
    typedef std::shared_ptr<NotificationManager> SPtr;

    NotificationManager(const QString &appName,
           const QDBusConnection& sessionConnection = QDBusConnection::sessionBus());

    ~NotificationManager();

    Notification::UPtr notify(
           const QString &summary,
           const QString &body,
           const QString &icon,
           const QStringList &actions,
           const QVariantMap &hints,
           int expireTimeout = -1);

Q_SIGNALS:
    void actionInvoked(uint id, const QString &action_key);

    void notificationClosed(uint id, uint reason);

    void dataChanged(uint id);

protected:
    class Priv;
    std::shared_ptr<Priv> d;
};

}
