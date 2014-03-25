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

#include "sim-unlock-dialog.h"

#include "notify-cpp/notification.h"

#include "menumodel-cpp/menu.h"
#include "menumodel-cpp/menu-exporter.h"

#include "menumodel-cpp/action.h"
#include "menumodel-cpp/action-group.h"
#include "menumodel-cpp/action-group-exporter.h"

#include <glib.h>

class SimUnlockDialog::Private
{
public:

    notify::Notification::Ptr m_notification;

    core::Property<std::string> m_title;
    core::Property<std::string> m_body;
    core::Property<std::pair<std::uint8_t, std::uint8_t>> m_pinMinMax;

    core::Signal<std::string> m_pinEntered;
    core::Signal<void> m_cancelled;
    core::Signal<void> m_closed;

    Action::Ptr m_notifyAction;
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

        /// @todo atomic
        static int exportId = 0;
        std::string actionPath = "/com/canonical/indicator/network/unlocksim" + std::to_string(exportId);
        std::string menuPath = "/com/canonical/indicator/network/unlocksim" + std::to_string(exportId);
        ++exportId;

        std::map<std::string, Variant> modelActions;
        modelActions["notifications"] = TypedVariant<std::string>(actionPath);

        std::map<std::string, Variant> modelPaths;
        modelPaths["busName"] = TypedVariant<std::string>("com.canonical.indicator.network");
        modelPaths["menuPath"] = TypedVariant<std::string>(menuPath);
        modelPaths["actions"] = TypedVariant<std::map<std::string, Variant>>(modelActions);

        m_menu = std::make_shared<Menu>();
        m_menuItem = std::make_shared<MenuItem>("", "notifications.simunlock");
        m_menuItem->setAttribute("x-canonical-type", TypedVariant<std::string>("com.canonical.snapdecision.pinlock"));
        /// @todo we need both min and max.
        m_menuItem->setAttribute("x-canonical-pin-length", TypedVariant<std::int32_t>(pinMinMax.first));
        m_menu->append(m_menuItem);

        m_actionGroup = std::make_shared<ActionGroup>();
        m_notifyAction = std::make_shared<Action>("notifications.simunlock",
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

        m_menuExporter = std::make_shared<MenuExporter>(menuPath, m_menu);
        m_actionGroupExporter = std::make_shared<ActionGroupExporter>(m_actionGroup, actionPath);

        m_notification = std::make_shared<notify::Notification>(title, body, "");
        m_notification->setHintString("x-canonical-snap-decisions", "true");
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

SimUnlockDialog::SimUnlockDialog(const std::string &title,
                                 const std::string &body,
                                 std::pair<std::uint8_t, std::uint8_t> pinMinMax)
{
    d.reset(new Private(title, body, pinMinMax));
}

SimUnlockDialog::~SimUnlockDialog()
{

}

core::Signal<std::string> &
SimUnlockDialog::pinEntered()
{
    return d->m_pinEntered;
}

core::Signal<void> &
SimUnlockDialog::cancelled()
{
    return d->m_cancelled;
}

core::Signal<void> &
SimUnlockDialog::closed()
{
    return d->m_closed;
}

core::Property<std::string> &
SimUnlockDialog::title()
{
    return d->m_title;
}

core::Property<std::string> &
SimUnlockDialog::body()
{
    return d->m_body;
}

core::Property<std::pair<std::uint8_t, std::uint8_t>> &
SimUnlockDialog::pinMinMax()
{
    return d->m_pinMinMax;
}

void
SimUnlockDialog::update()
{
    d->m_notification->show();
}

void
SimUnlockDialog::show()
{
    d->m_notification->show();
}

void
SimUnlockDialog::close()
{
    d->m_notification->close();
}
