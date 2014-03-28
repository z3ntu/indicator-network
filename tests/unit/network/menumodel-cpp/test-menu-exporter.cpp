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
#include <menumodel-cpp/menu-exporter.h>
#include <menumodel-cpp/menu.h>

#include <libqtdbustest/DBusTestRunner.h>
#include <libqtdbusmock/DBusMock.h>
#include <qmenumodel/unitymenumodel.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace std;
using namespace testing;
using namespace QtDBusTest;
using namespace QtDBusMock;

namespace {

class TestMenuExporter: public Test {
protected:
	TestMenuExporter() :
			dbusMock(dbusTestRunner) {
	}

	void SetUp() {
	}

	DBusTestRunner dbusTestRunner;

	DBusMock dbusMock;

	Menu::Ptr menu;

	std::unique_ptr<MenuExporter> menuExporter;
}
;

TEST_F(TestMenuExporter, GetSecretsWithNone) {
	menu = make_shared<Menu>();
	menuExporter.reset(
			new MenuExporter("/com/canonical/indicator/network/desktop", menu));
}

} // namespace
