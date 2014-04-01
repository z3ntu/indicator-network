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

#include <menuitems/switch-item.h>
#include <utils/action-utils.h>

#include <libqtdbustest/DBusTestRunner.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace std;
using namespace testing;
using namespace QtDBusTest;
using namespace testutils;

namespace
{

class TestSwitchItem : public Test
{
protected:
    DBusTestRunner dbus;
};

TEST_F(TestSwitchItem, ExportBasicActionsAndMenu)
{
    auto switchItem = make_shared<SwitchItem>("label", "prefix", "name");
    EXPECT_FALSE(switchItem->state());

    auto menuItem = switchItem->menuItem();
    EXPECT_EQ("label", menuItem->label());

    string name = menuItem->action();
    EXPECT_EQ("indicator.prefix.name", name);

    auto actionGroup = switchItem->actionGroup();
    auto action = findAction(actionGroup, name);
    ASSERT_FALSE(action == nullptr);

    EXPECT_FALSE(action->state().as<bool>());

    action->setState(TypedVariant<bool>(true));

    // FIXME Why can't we make this assertion
//    EXPECT_TRUE(switchItem->state());
}

} // namespace
