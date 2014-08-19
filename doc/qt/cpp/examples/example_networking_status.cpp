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

#include <QCoreApplication>
#include <QScopedPointer>
#include <QDebug>

//! [include]
#include <ubuntu/connectivity/networking-status.h>
using ubuntu::connectivity::NetworkingStatus;
//! [include]

int
main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    //! [create networkingstatus]
    QScopedPointer<NetworkingStatus> ns(new NetworkingStatus());
    //! [create networkingstatus]

    //! [status]
    // normal getter
    if (ns->status() == NetworkingStatus::Online)
        qDebug() << "We are online.";

    // Subscribe to system networking changes
    QObject::connect(ns.data(),
                     &NetworkingStatus::statusChanged,
                     [](NetworkingStatus::Status value)
    {
        switch(value) {
        case NetworkingStatus::Offline:
            qDebug() << "System networking status changed to: Offline";
            break;
        case NetworkingStatus::Connecting:
            qDebug() << "System networking status changed to: Connecting";
            break;
        case NetworkingStatus::Online:
            qDebug() << "System networking status changed to: Online";
            break;
        }
    });
    //! [status]

    //! [limitations]
    // normal getter
    if (ns->limitations().isEmpty())
        qDebug() << "No limitations";

    // Subscribe to limitation changes
    QObject::connect(ns.data(),
                     &NetworkingStatus::limitationsChanged,
                     [&ns](){
        if (ns->limitations().isEmpty()) {
            qDebug() << "No limitations.";
            return;
        }

        qDebug() << "Limitations:";
        if (ns->limitations().contains(NetworkingStatus::Limitations::Bandwith)) {
            qDebug() << "    - Bandwith";
        }
    });
    //! [limitations]

    app.exec();

    return 0;
}
