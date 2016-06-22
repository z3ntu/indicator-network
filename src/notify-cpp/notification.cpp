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

#include "notification.h"
#include <NotificationsInterface.h>

#include <QDebug>

using namespace notify;
using namespace std;

class notify::Notification::Private: public QObject
{
    Q_OBJECT

public:
    Private(Notification& parent) :
        p(parent)
    {
    }

    Notification& p;

    QString m_appName;
    QString m_summary;
    QString m_body;
    QString m_icon;
    QStringList m_actions;
    QVariantMap m_hints;
    int m_expireTimeout = 0;
    uint m_id = 0;

    shared_ptr<OrgFreedesktopNotificationsInterface> m_notificationsInterface;

    bool m_open = false;
    bool m_dirty = false;

public Q_SLOTS:
    void notificationClosed(uint id, uint reason)
    {
        if (id == m_id)
        {
            m_open = false;
            Q_EMIT p.closed(reason);
        }
    }

    void actionInvoked(uint id, const QString& name)
    {
        if (id == m_id)
        {
            Q_EMIT p.actionInvoked(name);
        }
    }
};

Notification::Notification(
        const QString& appName, const QString &summary, const QString &body,
        const QString &icon, const QStringList &actions,
        const QVariantMap &hints, int expireTimeout,
        shared_ptr<OrgFreedesktopNotificationsInterface> notificationsInterface)
{
    d.reset(new Private(*this));
    d->m_appName = appName;
    d->m_summary = summary;
    d->m_body = body;
    d->m_icon = icon;
    d->m_actions = actions;
    d->m_hints = hints;
    d->m_expireTimeout = expireTimeout;
    d->m_notificationsInterface = notificationsInterface;

    connect(d->m_notificationsInterface.get(), &OrgFreedesktopNotificationsInterface::NotificationClosed, d.get(), &Private::notificationClosed);

    connect(d->m_notificationsInterface.get(), &OrgFreedesktopNotificationsInterface::ActionInvoked, d.get(), &Private::actionInvoked);
}

Notification::~Notification()
{
    if (d->m_id > 0 && d->m_open && d->m_expireTimeout <= 0)
    {
        qDebug() << "Closing notification:" << d->m_id;
        auto reply = d->m_notificationsInterface->CloseNotification(d->m_id);
        reply.waitForFinished();
        if (reply.isError())
        {
            qCritical() << reply.error().message();
        }
    }
}

QString
Notification::summary() const
{
    return d->m_summary;
}

QString
Notification::body() const
{
    return d->m_body;
}

QString
Notification::icon() const
{
    return d->m_icon;
}

QStringList
Notification::actions() const
{
    return d->m_actions;
}

QVariantMap
Notification::hints() const
{
    return d->m_hints;
}

void
Notification::addHint(const QString &key, const QVariant& value)
{
    d->m_hints[key] = value;
    d->m_dirty = true;
    Q_EMIT hintsUpdated(d->m_hints);
}

void
Notification::setHints(const QVariantMap& hints)
{
    if (d->m_hints == hints)
    {
        return;
    }

    d->m_hints = hints;
    d->m_dirty = true;
    Q_EMIT hintsUpdated(d->m_hints);
}

void
Notification::setActions(const QStringList& actions)
{
    if (d->m_actions == actions)
    {
        return;
    }

    d->m_actions = actions;
    d->m_dirty = true;
    Q_EMIT actionsUpdated(d->m_actions);
}

void
Notification::setSummary(const QString& summary)
{
    if (d->m_summary == summary)
    {
        return;
    }

    d->m_summary = summary;
    d->m_dirty = true;
    Q_EMIT summaryUpdated(d->m_summary);
}

void
Notification::setBody(const QString& body)
{
    if (d->m_body == body)
    {
        return;
    }

    d->m_body = body;
    d->m_dirty = true;
    Q_EMIT bodyUpdated(d->m_body);
}

void
Notification::setIcon(const QString& icon)
{
    if (d->m_icon == icon)
    {
        return;
    }

    d->m_icon = icon;
    d->m_dirty = true;
    Q_EMIT iconUpdated(d->m_icon);
}

void
Notification::show()
{
    if (d->m_dirty || !d->m_open)
    {
        auto reply = d->m_notificationsInterface->Notify(d->m_appName, d->m_id,
                                                     d->m_icon, d->m_summary,
                                                     d->m_body, d->m_actions,
                                                     d->m_hints,
                                                     d->m_expireTimeout);
        reply.waitForFinished();
        if (reply.isError())
        {
            qCritical() << reply.error().message();
        }
        else
        {
            d->m_id = reply;
            d->m_dirty = false;
            d->m_open = true;
        }
    }
}

void
Notification::close()
{
    if (d->m_id > 0)
    {
        auto reply = d->m_notificationsInterface->CloseNotification(d->m_id);
        reply.waitForFinished();
        if (reply.isError())
        {
            qCritical() << reply.error().message();
        }
        else
        {
            d->m_open = false;
        }
    }
}

#include "notification.moc"
