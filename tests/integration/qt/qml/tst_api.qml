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

import Ubuntu.Connectivity 1.0
import QtQuick 2.0
import QtTest 1.0

/* This test will make sure there are no unintentional changes to the
 * schemantics of the API (default properties work as expected, etc)
 * so that any files written to the prior release of the library
 * will not bail out when loaded resulting in client applications failing
 * to start.
 */
Item {

    NetworkingStatus {
        id: networkingStatus
        onStatusChanged: {}
        onLimitationsChanged: {}
        onOnlineChanged: {}
    }

    TestCase {
        name: "API Test"
        id: test_api

        property int status : networkingStatus.status
        property int online : networkingStatus.online

        function test_api() {
            // just make sure this file can be loaded properly by the QmlEngine.
            verify(1)
        }
    }
}
