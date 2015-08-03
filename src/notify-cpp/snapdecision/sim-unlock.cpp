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

#include "sim-unlock.h"

#include "menumodel-cpp/menu.h"
#include "menumodel-cpp/menu-exporter.h"

#include "menumodel-cpp/action.h"
#include "menumodel-cpp/action-group.h"
#include "menumodel-cpp/action-group-exporter.h"

#include <string>
#include <QDebug>

using namespace notify::snapdecision;
using namespace std;

class SimUnlock::Private: public QObject
{
    Q_OBJECT

public:
    SimUnlock& p;

    NotificationManager::SPtr m_notificationManager;
    Notification::UPtr m_notification;

    QString m_title;
    QString m_body;
    std::pair<std::uint8_t, std::uint8_t> m_pinMinMax;

    std::shared_ptr<SessionBus> m_sessionBus;

    QVariantMap m_modelPaths;

    Action::Ptr m_notifyAction;
    Action::Ptr m_pinMinMaxAction;
    Action::Ptr m_popupAction;
    Action::Ptr m_errorAction;
    ActionGroup::Ptr m_actionGroup;
    ActionGroupExporter::Ptr m_actionGroupExporter;

    Menu::Ptr m_menu;
    MenuItem::Ptr m_menuItem;
    MenuExporter::Ptr m_menuExporter;

    std::function<void()> m_pendingErrorClosed;
    std::function<void()> m_pendingPopupClosed;

public Q_SLOTS:

    void resetActionStates()
    {
        m_popupAction->setState(TypedVariant<std::string>(""));
        if (m_pendingPopupClosed)
        {
            auto tmp = m_pendingPopupClosed;
            m_pendingPopupClosed = std::function<void()>();
            tmp();
        }

        m_errorAction->setState(TypedVariant<std::string>(""));
        if (m_pendingErrorClosed)
        {
            auto tmp = m_pendingErrorClosed;
            m_pendingErrorClosed = std::function<void()>();
            tmp();
        }
    }

    void resetNotification()
    {
        QVariantMap hints({
            { "x-canonical-snap-decisions", "true" },
            { "x-canonical-snap-decisions-timeout", numeric_limits<int32_t>::max() },
            { "x-canonical-private-menu-model", m_modelPaths }
        });

        if (m_notification)
        {
            m_notification->setSummary(m_title);
            m_notification->setBody(m_body);
            m_notification->setIcon("");
            m_notification->setActions(QStringList());
            m_notification->setHints(hints);
        }
        else
        {
            m_notification = m_notificationManager->notify(
                m_title, m_body, "", QStringList(), hints);

            connect(&p, &SimUnlock::titleUpdated, m_notification.get(), &Notification::setSummary);
            connect(&p, &SimUnlock::bodyUpdated, m_notification.get(), &Notification::setBody);
        }
        resetActionStates();
    }

    void pinEntered(const Variant& state)
    {
        Q_EMIT p.pinEntered(QString::fromStdString(state.as<std::string>()));
    }

    void notifyActivated(const Variant& parameter)
    {
        if (!parameter.as<bool>())
        {
            p.close();
            Q_EMIT p.cancelled();
        }
    }

    void popupActivated(const Variant&)
    {
        m_popupAction->setState(TypedVariant<std::string>(""));
        if (m_pendingPopupClosed)
        {
            // Pop the pending function off, to allow recursion
            auto tmp = m_pendingPopupClosed;
            m_pendingPopupClosed = std::function<void()>();
            tmp();
        }
    }

    void errorActivated(const Variant&)
    {
        m_errorAction->setState(TypedVariant<std::string>(""));
        if (m_pendingErrorClosed)
        {
            // Pop the pending function off, to allow recursion
            auto tmp = m_pendingErrorClosed;
            m_pendingErrorClosed = std::function<void()>();
            tmp();
        }
    }

    void pinMinMaxChanged(std::pair<std::uint8_t, uint8_t> value) {
        m_pinMinMaxAction->setState(TypedVariant<std::vector<std::int32_t>>({value.first, value.second}));
    }

public:
    Private(SimUnlock& parent,
            NotificationManager::SPtr notificationManager,
            const QString &title,
            const QString &body,
            std::pair<std::uint8_t, std::uint8_t> pinMinMax)
        : p(parent)
    {
        m_notificationManager = notificationManager;
        m_title = title;
        m_body = body;
        m_pinMinMax = pinMinMax;

        m_sessionBus.reset(new SessionBus());

        /// @todo atomic
        static int exportId = 0;
        std::string actionPath = "/com/canonical/indicator/network/unlocksim" + std::to_string(exportId);
        std::string menuPath = "/com/canonical/indicator/network/unlocksim" + std::to_string(exportId);
        ++exportId;

        QVariantMap modelActions;
        modelActions["notifications"] = QString::fromStdString(actionPath);

        m_modelPaths["busName"] = QString::fromStdString(m_sessionBus->address());
        m_modelPaths["menuPath"] = QString::fromStdString(menuPath);
        m_modelPaths["actions"] = modelActions;

        m_menu = std::make_shared<Menu>();
        m_menuItem = std::make_shared<MenuItem>("", "notifications.simunlock");
        m_menuItem->setAttribute("x-canonical-type", TypedVariant<std::string>("com.canonical.snapdecision.pinlock"));
        m_menuItem->setAttribute("x-canonical-pin-min-max", TypedVariant<std::string>("notifications.pinMinMax"));
        m_menuItem->setAttribute("x-canonical-pin-popup", TypedVariant<std::string>("notifications.popup"));
        m_menuItem->setAttribute("x-canonical-pin-error", TypedVariant<std::string>("notifications.error"));
        m_menu->append(m_menuItem);

        m_actionGroup = std::make_shared<ActionGroup>();
        m_notifyAction = std::make_shared<Action>("simunlock",
                                                  G_VARIANT_TYPE_BOOLEAN,
                                                  TypedVariant<std::string>(""));
        connect(m_notifyAction.get(), &Action::stateUpdated, this, &Private::pinEntered);
        connect(m_notifyAction.get(), &Action::activated, this, &Private::notifyActivated);
        m_actionGroup->add(m_notifyAction);

        m_pinMinMaxAction = std::make_shared<Action>("pinMinMax",
                                                     nullptr,
                                                     TypedVariant<std::vector<std::int32_t>>({m_pinMinMax.first, m_pinMinMax.second}));
        m_actionGroup->add(m_pinMinMaxAction);

        m_popupAction = std::make_shared<Action>("popup",
                                                  nullptr,
                                                  TypedVariant<std::string>(""));
        connect(m_popupAction.get(), &Action::activated, this, &Private::popupActivated);
        m_actionGroup->add(m_popupAction);

        m_errorAction = std::make_shared<Action>("error",
                                                  nullptr,
                                                  TypedVariant<std::string>(""));
        connect(m_errorAction.get(), &Action::activated, this, &Private::errorActivated);
        m_actionGroup->add(m_errorAction);

        m_menuExporter = std::make_shared<MenuExporter>(m_sessionBus, menuPath, m_menu);
        m_actionGroupExporter = std::make_shared<ActionGroupExporter>(m_sessionBus, m_actionGroup, actionPath);

        resetNotification();

        connect(&p, &SimUnlock::pinMinMaxUpdated, this, &Private::pinMinMaxChanged);
    }
};

SimUnlock::SimUnlock(NotificationManager::SPtr notificationManager,
                     const QString &title,
                     const QString &body,
                     std::pair<std::uint8_t, std::uint8_t> pinMinMax)
{
    d.reset(new Private(*this, notificationManager, title, body, pinMinMax));
}

SimUnlock::~SimUnlock()
{
}

QString
SimUnlock::title()
{
    return d->m_title;
}

QString
SimUnlock::body()
{
    return d->m_body;
}

std::pair<std::uint8_t, std::uint8_t>
SimUnlock::pinMinMax()
{
    return d->m_pinMinMax;
}

void
SimUnlock::update()
{
    d->m_notification->show();
}

void
SimUnlock::show()
{
    d->m_notification->show();
}

void
SimUnlock::close()
{
    d->resetActionStates();
    d->m_notification->close();
}

void
SimUnlock::showError(const std::string& message, std::function<void()> closed)
{
    d->m_errorAction->setState(TypedVariant<std::string>(message));
    if (d->m_pendingErrorClosed)
    {
        auto tmp = d->m_pendingErrorClosed;
        d->m_pendingErrorClosed = std::function<void()>();
        tmp();
    }
    d->m_pendingErrorClosed = closed;
}

void
SimUnlock::showPopup(const std::string& message, std::function<void()> closed)
{
    d->m_popupAction->setState(TypedVariant<std::string>(message));
    if (d->m_pendingPopupClosed)
    {
        auto tmp = d->m_pendingPopupClosed;
        d->m_pendingPopupClosed = std::function<void()>();
        tmp();
    }
    d->m_pendingPopupClosed = closed;
}

void
SimUnlock::setTitle(const QString& title)
{
    if (d->m_title == title)
    {
        return;
    }

    d->m_title = title;
    Q_EMIT titleUpdated(d->m_title);
}

void
SimUnlock::setBody(const QString& body)
{
    if (d->m_body == body)
    {
        return;
    }

    d->m_body = body;
    Q_EMIT bodyUpdated(d->m_body);
}

void
SimUnlock::setPinMinMax(const std::pair<std::uint8_t, std::uint8_t>& pinMinMax)
{
    if (d->m_pinMinMax == pinMinMax)
    {
        return;
    }

    d->m_pinMinMax = pinMinMax;
    Q_EMIT pinMinMaxUpdated(d->m_pinMinMax);
}

#include "sim-unlock.moc"
