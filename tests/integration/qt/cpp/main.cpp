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

#include <QtTest/QtTest>
#include <QCoreApplication>

#include "tst_networking.h"

int main(int argc, char *argv[])
{
    // needed for QTest::qWait
    QCoreApplication app(argc, argv);

    TestNetworking tst_networking;

    if (QTest::qExec(&tst_networking, argc, argv) != 0)
        return 1;

    return 0;
}
