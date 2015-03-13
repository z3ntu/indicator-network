/*
 * Copyright (C) 2014 Canonical, Ltd.
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
 * Authors:
 *     Antti Kaijanmäki <antti.kaijanmaki@canonical.com>
 */

#include "service.h"
#include <DBusTypes.h>
#include "connectivity-service/connectivity-service.h"

#include <iostream>
#include <memory>

#include <QCoreApplication>

#include <string.h>
#include <libintl.h>

#include <glib.h>
#include <libnotify/notify.h>

#include <config.h>

struct sigaction act;
static void
signal_handler(int signo) {
    std::cerr << "*** Received " << strsignal(signo) << ". ***"<< std::endl;

    switch(signo) {
    case SIGHUP:
    case SIGINT:
    case SIGQUIT:
    case SIGTERM:

        QCoreApplication::exit();
        notify_uninit();
        break;
    }
}

int
main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    DBusTypes::registerMetaTypes();

    act.sa_handler = signal_handler;
    sigaction(SIGHUP, &act, 0);
    sigaction(SIGINT, &act, 0);
    sigaction(SIGQUIT, &act, 0);
    sigaction(SIGTERM, &act, 0);
    sigaction(SIGCONT, &act, 0);

    bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
    setlocale(LC_ALL, "");
    bindtextdomain(GETTEXT_PACKAGE, LOCALE_DIR);
    textdomain(GETTEXT_PACKAGE);

    notify_init(GETTEXT_PACKAGE);

    std::shared_ptr<networking::Manager> manager = networking::Manager::createInstance();

    std::shared_ptr<Service> menu {new Service(manager)};
    std::unique_ptr<ConnectivityService> connectivityService {new ConnectivityService(manager)};

    // unlockAllModems is dispatched from GMainLoop
    connectivityService->unlockAllModems().connect([menu](){ menu->unlockAllModems(); });

    // unlockModem is dispatched from GMainLoop
    connectivityService->unlockModem().connect([menu](const std::string &name){ menu->unlockModem(name); });

    return app.exec();
}
