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

#include <libqtdbustest/DBusTestRunner.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace std;
using namespace testing;
using namespace QtDBusTest;

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
    void
    SetUp ()
    {
    }

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

    AccessPointItem::Ptr accessPointItem = make_shared<AccessPointItem>(accessPoint);
    MenuItem::Ptr menuItem = accessPointItem->menuItem();

    EXPECT_EQ("the ssid", menuItem->label());
}

} // namespace
