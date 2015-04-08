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
 *     Jussi Pakkanen <jussi.pakkanen@canonical.com>
 */

#include "eventprinter.h"

#include "dbusdata.h"
#include "nmroot.h"
#include "ofonomodemmodem.h"
#include "ofonomodemnetworkregistration.h"
#include "ofonomodemsimmanager.h"
#include "ofonoroot.h"
#include "urfkillroot.h"
#include "urfkillswitch.h"

EventPrinter::EventPrinter(QObject *parent) : QObject(parent) {
    urfkill = new UrfkillRoot(URFKILL_SERVICE, URFKILL_OBJECT, QDBusConnection::systemBus(), this);
    btkill = new UrfkillSwitch(URFKILL_SERVICE, URFKILL_BLUETOOTH_OBJECT, QDBusConnection::systemBus(), this);
    wlankill = new UrfkillSwitch(URFKILL_SERVICE, URFKILL_WLAN_OBJECT, QDBusConnection::systemBus(), this);
    nmroot = new NetworkManagerRoot(NM_SERVICE, NM_OBJECT, QDBusConnection::systemBus(), this);

    connect(urfkill, SIGNAL(FlightModeChanged(bool)), this, SLOT(flightModeChanged(bool)));
    connect(btkill, SIGNAL(StateChanged()), this, SLOT(btKillswitchChanged()));
    connect(wlankill, SIGNAL(StateChanged()), this, SLOT(wlanKillswitchChanged()));
    connect(nmroot, SIGNAL(StateChanged(uint)), this, SLOT(nmStateChanged(uint)));
    connect(nmroot, SIGNAL(PropertiesChanged(const QVariantMap&)), this, SLOT(nmPropertiesChanged(const QVariantMap&)));

    OfonoRoot ofono(OFONO_SERVICE, OFONO_OBJECT, QDBusConnection::systemBus(), nullptr);
    auto modems = ofono.GetModems().value();
    if(modems.length() > 0) {
        modem1 = new OfonoModemModem(OFONO_SERVICE, modems[0].first.path(), QDBusConnection::systemBus(), this);
        connect(modem1, SIGNAL(PropertyChanged(const QString, const QDBusVariant)), this,
                SLOT(modem1PropertyChanged(const QString, const QDBusVariant)));
        simman1 = new OfonoModemSimManager(OFONO_SERVICE, modems[0].first.path(), QDBusConnection::systemBus(), this);
        connect(simman1, SIGNAL(PropertyChanged(const QString, const QDBusVariant)), this,
                SLOT(simman1PropertyChanged(const QString, const QDBusVariant)));
        netreg1 = new OfonoModemNetworkRegistration(OFONO_SERVICE, modems[0].first.path(), QDBusConnection::systemBus(), this);
        connect(netreg1, SIGNAL(PropertyChanged(const QString, const QDBusVariant)), this,
                SLOT(netreg1PropertyChanged(const QString, const QDBusVariant)));
    } else {
        modem1 = nullptr;
        simman1 = nullptr;
        netreg1 = nullptr;
    }
    if(modems.length() > 1) {
        modem2 = new OfonoModemModem(OFONO_SERVICE, modems[1].first.path(), QDBusConnection::systemBus(), this);
        connect(modem2, SIGNAL(PropertyChanged(const QString, const QDBusVariant)), this,
                SLOT(modem2PropertyChanged(const QString, const QDBusVariant)));
        simman2 = new OfonoModemSimManager(OFONO_SERVICE, modems[1].first.path(), QDBusConnection::systemBus(), this);
        connect(simman2, SIGNAL(PropertyChanged(const QString, const QDBusVariant)), this,
                SLOT(simman2PropertyChanged(const QString, const QDBusVariant)));
        netreg2 = new OfonoModemNetworkRegistration(OFONO_SERVICE, modems[1].first.path(), QDBusConnection::systemBus(), this);
        connect(netreg2, SIGNAL(PropertyChanged(const QString, const QDBusVariant)), this,
                SLOT(netreg2PropertyChanged(const QString, const QDBusVariant)));
    } else {
        modem2 = nullptr;
        simman2 = nullptr;
    }
}

EventPrinter::~EventPrinter() {
}

void EventPrinter::flightModeChanged(bool newMode) {
    printf("Flight mode changed to: %d\n", newMode);
}

void EventPrinter::btKillswitchChanged() {
    printf("Bluetooth killswitch changed to: %d\n", btkill->state());
}

void EventPrinter::wlanKillswitchChanged() {
    printf("Wlan killswitch changed to: %d\n", wlankill->state());
}

void EventPrinter::nmStateChanged(uint new_state) {
    printf("NetworkManager state changed to: %d\n", new_state);
}

void EventPrinter::nmPropertiesChanged(const QVariantMap &props) {
    const static QString activeConnections("ActiveConnections");
    const static QString primaryConnectionType("PrimaryConnectionType");
    printf("NetworkManager properties changed:\n");
    for(const auto &p : props.keys()) {
        if(p == activeConnections || p == primaryConnectionType)
            continue;
        printf("  %s %s\n", p.toUtf8().data(), props[p].toString().toUtf8().data());
    }

    auto i = props.find(activeConnections);
    if(i != props.end()) {
        activeConnectionsChanged(*i);
    }
    i = props.find(primaryConnectionType);
    if(i != props.end()) {
        primaryConnectionTypeChanged(i->toString());
    }
}

void EventPrinter::primaryConnectionTypeChanged(const QString &type) {
    printf("NetworkManager primary connection type changed to %s.\n", type.toUtf8().data());
}

void EventPrinter::activeConnectionsChanged(const QVariant &/*list*/) {
    // list.value<QList<QDBusObjectPath>>() does not work for some reason
    // so grab again over dbus. Yes it sucks.
    printf("  ActiveConnections (now have %d)\n", nmroot->activeConnections().size());
}

void EventPrinter::modem1PropertyChanged(const QString name, const QDBusVariant value) {
    modemPropertyChanged(1, name, value);
}

void EventPrinter::modem2PropertyChanged(const QString name, const QDBusVariant value) {
    modemPropertyChanged(2, name, value);
}

void EventPrinter::modemPropertyChanged(const int modem, const QString &name, const QDBusVariant &value) {
    auto str = value.variant().toString();
    if(str.length() > 0) {
        str = " to " + str;
    }
    printf("Ofono modem %d property %s changed%s.\n", modem, name.toUtf8().data(), str.toUtf8().data());
}

void EventPrinter::simman1PropertyChanged(const QString name, const QDBusVariant value) {
    simmanPropertyChanged(1, name, value);
}

void EventPrinter::simman2PropertyChanged(const QString name, const QDBusVariant value) {
    simmanPropertyChanged(2, name, value);
}

void EventPrinter::simmanPropertyChanged(const int simman, const QString &name, const QDBusVariant &value) {
    if(name == "PinRequired") {
        printf("Ofono modem %d pin required changed to %s.\n", simman, value.variant().toString().toUtf8().data());
    }
}

void EventPrinter::netreg1PropertyChanged(const QString name, const QDBusVariant value) {
    netregPropertyChanged(1, name, value);
}

void EventPrinter::netreg2PropertyChanged(const QString name, const QDBusVariant value) {
    netregPropertyChanged(2, name, value);
}

void EventPrinter::netregPropertyChanged(const int netreg, const QString &name, const QDBusVariant &value) {
    QString valuetext;
    if(name == "Strength") {
        valuetext = QString::number(value.variant().toInt());
    } else {
        valuetext = value.variant().toString();
    }

    printf("Ofono network registration %d property %s changed to %s.\n", netreg, name.toUtf8().data(), valuetext.toUtf8().data());
}
