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

#pragma once

#include <memory>

#include "menumodel-cpp/gio-helpers/variant.h"

#include <QObject>

class OrgFreedesktopNotificationsInterface;

namespace notify
{

class NotificationManager;

class Notification: public QObject
{
    Q_OBJECT

    class Private;
    std::unique_ptr<Private> d;

public:

    typedef std::unique_ptr<Notification> UPtr;
    typedef std::shared_ptr<Notification> SPtr;

    virtual ~Notification();

    /// @todo remember show() after set().
    Q_PROPERTY(QString summary READ summary WRITE setSummary NOTIFY summaryUpdated)
    QString summary() const;

    /// @todo remember show() after set().
    Q_PROPERTY(QString body READ body WRITE setBody NOTIFY bodyUpdated)
    QString body() const;

    /// @todo remember show() after set().
    Q_PROPERTY(QString icon READ icon WRITE setIcon NOTIFY iconUpdated)
    QString icon() const;

    Q_PROPERTY(QStringList actions READ actions WRITE setActions NOTIFY actionsUpdated)
    QStringList actions() const;

    Q_PROPERTY(QVariantMap hints READ hints WRITE setHints NOTIFY hintsUpdated)
    QVariantMap hints() const;

    void show();
    void close();

public Q_SLOTS:
    void setActions(const QStringList& actions);

    void setHints(const QVariantMap& hints);

    void addHint(const QString &key, const QVariant& value);

    void setSummary(const QString& summary);

    void setBody(const QString& body);

    void setIcon(const QString& icon);

Q_SIGNALS:
    void closed(uint reason);

    void hintsUpdated(const QVariantMap&);

    void actionsUpdated(const QStringList&);

    void summaryUpdated(const QString&);

    void bodyUpdated(const QString&);

    void iconUpdated(const QString&);

    void actionInvoked(const QString& name);

public:
    Notification(const QString& appName,
                 const QString &summary, const QString &body,
                 const QString &icon, const QStringList &actions,
                 const QVariantMap &hints, int expireTimeout,
                 std::shared_ptr<OrgFreedesktopNotificationsInterface> notificationsInterface);
};
}
