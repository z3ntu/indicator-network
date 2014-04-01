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

#include <menuitems/access-point-item.h>
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

class MockAccessPoint : public networking::wifi::AccessPoint
{
public:
    const core::Property<double>&
    strength ()
    {
        return m_strength;
    }

    MOCK_METHOD0(ssid, std::string());

    MOCK_METHOD0(secured, bool());

    MOCK_METHOD0(adhoc, bool());

    core::Property<double> m_strength;
};

class TestAccessPointItem : public Test
{
protected:
    DBusTestRunner dbus;
};

TEST_F(TestAccessPointItem, ExportBasicActionsAndMenu)
{
    shared_ptr<MockAccessPoint> accessPoint = make_shared<
            NiceMock<MockAccessPoint>>();
    ON_CALL(*accessPoint, ssid()).WillByDefault(Return("the ssid"));
    ON_CALL(*accessPoint, secured()).WillByDefault(Return(true));
    ON_CALL(*accessPoint, adhoc()).WillByDefault(Return(false));
    accessPoint->m_strength = 70.0;

    auto accessPointItem = make_shared<AccessPointItem>(accessPoint);

    auto menuItem = accessPointItem->menuItem();

    EXPECT_EQ("the ssid", menuItem->label());
    EXPECT_EQ(false, bool_value(menuItem, "x-canonical-wifi-ap-is-adhoc"));
    EXPECT_EQ(true, bool_value(menuItem, "x-canonical-wifi-ap-is-secure"));

    string strengthActionName = string_value(
            menuItem, "x-canonical-wifi-ap-strength-action");

    auto strengthAction = findAction(accessPointItem->actionGroup(),
                                     strengthActionName);

    ASSERT_FALSE(strengthAction.get() == nullptr);
    EXPECT_EQ(70, strengthAction->state().as<uint8_t>());

    accessPoint->m_strength = 20.0;
    EXPECT_EQ(20, strengthAction->state().as<uint8_t>());
}

} // namespace
