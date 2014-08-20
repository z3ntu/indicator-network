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

#include "tst_networking.h"

#include <ubuntu/connectivity/NetworkingStatus>

#include <QtTest/QtTest>

void
TestNetworking::initTestCase()
{}

void
TestNetworking::cleanupTestCase()
{}

void
TestNetworking::cleanup()
{}


void
TestNetworking::testApi()
{
    using namespace ubuntu::connectivity;

    auto ns = new NetworkingStatus();

    QVERIFY(ns->metaObject()->indexOfProperty("limitations") != -1);
    QVERIFY(ns->metaObject()->indexOfProperty("status") != -1);

    QVERIFY(ns->limitations() == QVector<NetworkingStatus::Limitations>());
    QVERIFY(ns->status() == NetworkingStatus::Online);
}
