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

#include "quick-access-section.h"

#include "menuitems/switch-item.h"

#include "menumodel-cpp/action-group-merger.h"
#include "menumodel-cpp/menu-merger.h"

#include <unity/util/ResourcePtr.h>
#include <glib.h>
#include <QDebug>

using namespace std;
namespace networking = connectivity::networking;
namespace util = unity::util;

class QuickAccessSection::Private : public QObject
{
    Q_OBJECT

public:
    ActionGroupMerger::Ptr m_actionGroupMerger;
    Menu::Ptr m_menu;

    std::shared_ptr<networking::Manager> m_manager;

    SwitchItem::Ptr m_flightModeSwitch;

    Private() = delete;
    Private(std::shared_ptr<networking::Manager> manager);

public Q_SLOTS:
    void flightModeUpdated(networking::Manager::FlightModeStatus value)
    {
        switch (value) {
        case networking::Manager::FlightModeStatus::off:
            m_flightModeSwitch->setState(false);
            break;
        case networking::Manager::FlightModeStatus::on:
            m_flightModeSwitch->setState(true);
            break;
        }
    }

    void flightModeSwitchActivated(bool state)
    {
        // Give the GActionGroup a change to emit its Changed signal
        shared_ptr<GMainLoop> loop(g_main_loop_new(nullptr, false), &g_main_loop_unref);
        util::ResourcePtr<guint, function<void(guint)>> timer(g_timeout_add(30,
                [](gpointer user_data) -> gboolean
                {
                    g_main_loop_quit((GMainLoop *)user_data);
                    return G_SOURCE_CONTINUE;
                },
                loop.get()),
                &g_source_remove);
        g_main_loop_run(loop.get());

        if (state) {
            try {
                m_manager->enableFlightMode();
            } catch (const std::exception &e) {
                std::cerr << e.what() << std::endl;
            }
        } else {
            try {
                m_manager->disableFlightMode();
            } catch (const std::exception &e) {
                std::cerr << e.what() << std::endl;
            }
        }
    }
};

QuickAccessSection::Private::Private(std::shared_ptr<networking::Manager> manager)
    : m_manager{manager}
{
    m_actionGroupMerger = std::make_shared<ActionGroupMerger>();
    m_menu = std::make_shared<Menu>();

    m_flightModeSwitch = std::make_shared<SwitchItem>(_("Flight Mode"), "airplane", "enabled");
    qDebug() << "initial flight mode status" << (int) m_manager->flightMode();
    flightModeUpdated(m_manager->flightMode());
    connect(m_manager.get(), &networking::Manager::flightModeUpdated, this, &Private::flightModeUpdated);
    connect(m_flightModeSwitch.get(), &SwitchItem::stateUpdated, this, &Private::flightModeSwitchActivated);

    m_actionGroupMerger->add(*m_flightModeSwitch);
    m_menu->append(*m_flightModeSwitch);
}

QuickAccessSection::QuickAccessSection(std::shared_ptr<networking::Manager> manager)
    : d{new Private(manager)}
{
}

QuickAccessSection::~QuickAccessSection()
{
}

ActionGroup::Ptr
QuickAccessSection::actionGroup()
{
    return *d->m_actionGroupMerger;
}

MenuModel::Ptr
QuickAccessSection::menuModel()
{
    return d->m_menu;
}

#include "quick-access-section.moc"
