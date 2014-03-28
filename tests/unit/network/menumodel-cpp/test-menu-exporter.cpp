/*
 * Copyright (C) 2013 Canonical, Ltd.
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
 * Author: Pete Woods <pete.woods@canonical.com>
 */

#include <cassert>
#include <menumodel-cpp/action.h>
#include <menumodel-cpp/action-group-exporter.h>
#include <menumodel-cpp/action-group.h>
#include <menumodel-cpp/menu-exporter.h>
#include <menumodel-cpp/menu.h>

#include <libqtdbustest/DBusTestRunner.h>
#include <libqtdbusmock/DBusMock.h>
#include <QSignalSpy>
#include <qmenumodel/unitymenumodel.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace std;
using namespace testing;
using namespace QtDBusTest;
using namespace QtDBusMock;

namespace
{

class TestMenuExporter : public Test
{
protected:
    TestMenuExporter () :
            dbusMock(dbus)
    {
    }

    void
    SetUp ()
    {
        sessionBus = make_shared<SessionBus>();
        actionGroup = make_shared<ActionGroup>();
        menu = make_shared<Menu>();
    }

    DBusTestRunner dbus;

    DBusMock dbusMock;

    std::shared_ptr<SessionBus> sessionBus;

    ActionGroup::Ptr actionGroup;

    Menu::Ptr menu;

    std::unique_ptr<ActionGroupExporter> actionGroupExporter;

    std::unique_ptr<MenuExporter> menuExporter;
}
;

TEST_F(TestMenuExporter, GetSecretsWithNone)
{
    actionGroup->add(make_shared<::Action>("apple"));
    actionGroup->add(make_shared<::Action>("banana"));
    actionGroup->add(make_shared<::Action>("coconut"));
    actionGroupExporter.reset(
            new ActionGroupExporter(actionGroup, "/actions/path", "prefix"));

    menu->append(make_shared<MenuItem>("Apple", "prefix.apple"));
    menu->append(make_shared<MenuItem>("Banana", "prefix.banana"));
    menu->append(make_shared<MenuItem>("Coconut", "prefix.coconut"));
    menuExporter.reset(new MenuExporter("/menus/path", menu));

    UnityMenuModel menuModel;
    QSignalSpy menuSpy(&menuModel,
                       SIGNAL(rowsInserted(const QModelIndex&, int, int)));

    menuModel.setBusName(
            g_dbus_connection_get_unique_name(sessionBus->bus().get()));
    menuModel.setMenuObjectPath("/menus/path");
    QVariantMap actions;
    actions["prefix"] = "/actions/path";
    menuModel.setActions(actions);

    menuSpy.wait();
    ASSERT_FALSE(menuSpy.isEmpty());
    EXPECT_EQ(3, menuModel.rowCount());
}

} // namespace
