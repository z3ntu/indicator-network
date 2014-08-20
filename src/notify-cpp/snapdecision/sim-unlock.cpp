/*
 * Copyright (C) 2014 Canonical, Ltd.
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

#include "sim-unlock.h"
#include <notify-cpp/notification.h>

#include "menumodel-cpp/menu.h"
#include "menumodel-cpp/menu-exporter.h"

#include "menumodel-cpp/action.h"
#include "menumodel-cpp/action-group.h"
#include "menumodel-cpp/action-group-exporter.h"

using namespace notify::snapdecision;

class SimUnlock::Private
{
public:

    Notification::Ptr m_notification;

    core::Property<std::string> m_title;
    core::Property<std::string> m_body;
    core::Property<std::pair<std::uint8_t, std::uint8_t>> m_pinMinMax;

    core::Signal<std::string> m_pinEntered;
    core::Signal<void> m_cancelled;
    core::Signal<void> m_closed;

    std::shared_ptr<SessionBus> m_sessionBus;

    Action::Ptr m_notifyAction;
    Action::Ptr m_pinMinMaxAction;
    Action::Ptr m_errorAction;
    ActionGroup::Ptr m_actionGroup;
    ActionGroupExporter::Ptr m_actionGroupExporter;

    Menu::Ptr m_menu;
    MenuItem::Ptr m_menuItem;
    MenuExporter::Ptr m_menuExporter;

    Private(const std::string &title,
            const std::string &body,
            std::pair<std::uint8_t, std::uint8_t> pinMinMax)
    {
        m_title.set(title);
        m_body.set(body);
        m_pinMinMax.set(pinMinMax);

        m_sessionBus.reset(new SessionBus());

        /// @todo atomic
        static int exportId = 0;
        std::string actionPath = "/com/canonical/indicator/network/unlocksim" + std::to_string(exportId);
        std::string menuPath = "/com/canonical/indicator/network/unlocksim" + std::to_string(exportId);
        ++exportId;

        std::map<std::string, Variant> modelActions;
        modelActions["notifications"] = TypedVariant<std::string>(actionPath);

        std::map<std::string, Variant> modelPaths;
        /// @todo hack!
        // modelPaths["busName"] = TypedVariant<std::string>("com.canonical.indicator.network");
        modelPaths["busName"] = TypedVariant<std::string>(m_sessionBus->address());
        modelPaths["menuPath"] = TypedVariant<std::string>(menuPath);
        modelPaths["actions"] = TypedVariant<std::map<std::string, Variant>>(modelActions);

        m_menu = std::make_shared<Menu>();
        m_menuItem = std::make_shared<MenuItem>("", "notifications.simunlock");
        m_menuItem->setAttribute("x-canonical-type", TypedVariant<std::string>("com.canonical.snapdecision.pinlock"));
        /// @todo we need both min and max.
        m_menuItem->setAttribute("x-canonical-pin-length", TypedVariant<std::int32_t>(pinMinMax.first));
        m_menuItem->setAttribute("x-canonical-pin-min-max", TypedVariant<std::string>("notifications.pinMinMax"));
        m_menuItem->setAttribute("x-canonical-pin-error", TypedVariant<std::string>("notifications.error"));
        m_menu->append(m_menuItem);

        m_actionGroup = std::make_shared<ActionGroup>();
        m_notifyAction = std::make_shared<Action>("simunlock",
                                                  G_VARIANT_TYPE_BOOLEAN,
                                                  TypedVariant<std::string>(""),
                                                  [this](Variant state)
        {
                m_pinEntered(state.as<std::string>());
        });
        m_notifyAction->activated().connect([this](Variant parameter){
            if (!parameter.as<bool>())
                m_cancelled();
        });
        m_actionGroup->add(m_notifyAction);

#if 0
        m_pinMinMaxAction = std::make_shared<Action>("pinMinMax",
                                                     nullptr,
                                                     TypedVariant<std::vector<std::int32_t>>(m_pinMinMax),
                                                     [this](Variant state)
        {
        });
        m_actionGroup->add(m_pinMinMaxrAction);
#endif
        m_errorAction = std::make_shared<Action>("error",
                                                  nullptr,
                                                  TypedVariant<std::string>(""),
                                                  [this](Variant state)
        {
            auto tmp = state.as<std::string>();
            if (tmp.empty()) {
                // ack from the dialog side.
                // find the error and call the cb
            } else {
            }
        });
        m_actionGroup->add(m_errorAction);

        m_menuExporter = std::make_shared<MenuExporter>(m_sessionBus, menuPath, m_menu);
        m_actionGroupExporter = std::make_shared<ActionGroupExporter>(m_sessionBus, m_actionGroup, actionPath);

        m_notification = std::make_shared<notify::Notification>(title, body, "");
        m_notification->setHintString("x-canonical-snap-decisions", "true");
        m_notification->setHint("x-canonical-snap-decisions-timeout", TypedVariant<std::int32_t>(std::numeric_limits<std::int32_t>::max()));
        m_notification->setHint("x-canonical-private-menu-model", TypedVariant<std::map<std::string, Variant>>(modelPaths));
        m_notification->closed().connect([this](){ m_closed(); });

        m_title.changed().connect([this](const std::string &value){
            /// @todo we need both min and max.
            m_notification->summary().set(value);
        });
        m_body.changed().connect([this](const std::string &value){
            m_notification->body().set(value);
        });
        m_pinMinMax.changed().connect([this](std::pair<std::uint8_t, uint8_t> value) {
            /// @todo assert min <= max, also upon construction
            m_menuItem->setAttribute("x-canonical-pin-length",
                                     TypedVariant<std::int32_t>(value.first));
        });
    }
};

SimUnlock::SimUnlock(const std::string &title,
                                 const std::string &body,
                                 std::pair<std::uint8_t, std::uint8_t> pinMinMax)
{
    d.reset(new Private(title, body, pinMinMax));
}

SimUnlock::~SimUnlock()
{

}

core::Signal<std::string> &
SimUnlock::pinEntered()
{
    return d->m_pinEntered;
}

core::Signal<void> &
SimUnlock::cancelled()
{
    return d->m_cancelled;
}

core::Signal<void> &
SimUnlock::closed()
{
    return d->m_closed;
}

core::Property<std::string> &
SimUnlock::title()
{
    return d->m_title;
}

core::Property<std::string> &
SimUnlock::body()
{
    return d->m_body;
}

core::Property<std::pair<std::uint8_t, std::uint8_t>> &
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
    d->m_notification->close();
}

void
SimUnlock::showError(std::string message, std::function<void()> closed)
{
    std::cerr << __PRETTY_FUNCTION__ << ": " << message << std::endl;
    if (closed)
        closed();
}

void
SimUnlock::showPopup(std::string message, std::function<void()> closed)
{
    std::cerr << __PRETTY_FUNCTION__ << ": " << message << std::endl;
    if (closed)
        closed();
}
