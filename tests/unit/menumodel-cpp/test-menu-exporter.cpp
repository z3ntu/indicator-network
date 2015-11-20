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

#include <cassert>
#include <dbus-types.h>
#include <menumodel-cpp/action.h>
#include <menumodel-cpp/action-group-exporter.h>
#include <menumodel-cpp/action-group.h>
#include <menumodel-cpp/menu-exporter.h>
#include <menumodel-cpp/menu.h>

#include <libqtdbustest/DBusTestRunner.h>
#include <QSignalSpy>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <unity/gmenuharness/MenuMatcher.h>

using namespace std;
using namespace testing;
using namespace QtDBusTest;
namespace mh = unity::gmenuharness;

namespace
{

class TestMenuExporter : public Test
{
protected:
    void
    SetUp () override
    {
        Variant::registerMetaTypes();
        sessionBus = make_shared<SessionBus>();
        g_dbus_connection_set_exit_on_close(sessionBus->bus().get(), FALSE);
        actionGroup = make_shared<ActionGroup>();
        menu = make_shared<Menu>();
    }

    mh::MenuMatcher::Parameters parameters(const string& prefix)
    {
        return mh::MenuMatcher::Parameters(sessionBus->address(), {{ prefix, "/actions/path" }},
                                           "/menus/path");
    }

    DBusTestRunner dbus;

    std::shared_ptr<SessionBus> sessionBus;

    ActionGroup::Ptr actionGroup;

    Menu::Ptr menu;

    std::unique_ptr<ActionGroupExporter> actionGroupExporter;

    std::unique_ptr<MenuExporter> menuExporter;
}
;

TEST_F(TestMenuExporter, ExportBasicActionsAndMenu)
{
    actionGroup->add(make_shared< ::Action>("apple"));
    actionGroup->add(make_shared< ::Action>("banana"));
    actionGroup->add(make_shared< ::Action>("coconut"));
    actionGroupExporter.reset(
            new ActionGroupExporter(sessionBus, actionGroup, "/actions/path"));

    menu->append(make_shared<MenuItem>("Apple", "prefix.apple"));
    menu->append(make_shared<MenuItem>("Banana", "prefix.banana"));
    menu->append(make_shared<MenuItem>("Coconut", "prefix.coconut"));
    menuExporter.reset(new MenuExporter(sessionBus, "/menus/path", menu));

    EXPECT_MATCHRESULT(mh::MenuMatcher(parameters("prefix"))
        .item(mh::MenuItemMatcher()
            .label("Apple")
            .action("prefix.apple")
        )
        .item(mh::MenuItemMatcher()
            .label("Banana")
            .action("prefix.banana")
        )
        .item(mh::MenuItemMatcher()
            .label("Coconut")
            .action("prefix.coconut")
        ).match());
}

TEST_F(TestMenuExporter, ActionActivation)
{
    std::shared_ptr< ::Action> apple = make_shared< ::Action>("apple");
    actionGroup->add(apple);
    actionGroupExporter.reset(
            new ActionGroupExporter(sessionBus, actionGroup, "/actions/path"));

    menu->append(make_shared<MenuItem>("Apple", "app.apple"));
    menuExporter.reset(new MenuExporter(sessionBus, "/menus/path", menu));

    QSignalSpy signalSpy(apple.get(),
                         SIGNAL(activated(const Variant&)));

    EXPECT_MATCHRESULT(mh::MenuMatcher(parameters("app"))
        .item(mh::MenuItemMatcher()
            .label("Apple")
            .action("app.apple")
            .activate()
        ).match());

    if (signalSpy.isEmpty())
    {
        ASSERT_TRUE(signalSpy.wait());
    }
    auto v = qvariant_cast<Variant>(signalSpy.first().first());
    EXPECT_EQ("null", v.to_string());
}

TEST_F(TestMenuExporter, ParameterizedActionActivation)
{
    std::shared_ptr< ::Action> parameterized = make_shared< ::Action>("param", G_VARIANT_TYPE_STRING);
    actionGroup->add(parameterized);
    actionGroupExporter.reset(
            new ActionGroupExporter(sessionBus, actionGroup, "/actions/path"));

    menu->append(make_shared<MenuItem>("Param", "app.param"));
    menuExporter.reset(new MenuExporter(sessionBus, "/menus/path", menu));

    QSignalSpy signalSpy(parameterized.get(),
                             SIGNAL(activated(const Variant&)));

    EXPECT_MATCHRESULT(mh::MenuMatcher(parameters("app"))
        .item(mh::MenuItemMatcher()
            .label("Param")
            .action("app.param")
            .activate(shared_ptr<GVariant>(g_variant_new_string("hello"), &g_variant_unref))
        ).match());

    if (signalSpy.isEmpty())
    {
        ASSERT_TRUE(signalSpy.wait());
    }
    auto v = qvariant_cast<Variant>(signalSpy.first().first());
    EXPECT_EQ("hello", v.as<string>());
}

} // namespace
