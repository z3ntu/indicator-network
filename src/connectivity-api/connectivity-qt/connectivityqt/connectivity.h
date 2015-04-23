/*
 * Copyright (C) 2015 Canonical, Ltd.
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
 *     Pete Woods <pete.woods@canonical.com>
 */

#pragma once

#include <QDBusConnection>
#include <QObject>
#include <QStringList>
#include <memory>

#include <unity/util/DefinesPtrs.h>

namespace connectivityqt
{

class Q_DECL_EXPORT Connectivity: public QObject
{
    Q_OBJECT

public:
    UNITY_DEFINES_PTRS(Connectivity);

    Connectivity(const QDBusConnection& sessionConnection = QDBusConnection::sessionBus());

    ~Connectivity();

    Q_PROPERTY(bool FlightMode READ flightMode WRITE setFlightMode NOTIFY flightModeUpdated)
    bool flightMode() const;

//    Q_PROPERTY(QStringList Limitations READ limitations NOTIFY limitationsUpdated)
//    QStringList limitations() const;

//    Q_PROPERTY(QString Status READ status NOTIFY statusUpdated)
//    QString status() const;

    Q_PROPERTY(bool WifiEnabled READ wifiEnabled WRITE setwifiEnabled NOTIFY wifiEnabledUpdated)
    bool wifiEnabled() const;

    Q_PROPERTY(bool UnstoppableOperationHappening READ unstoppableOperationHappening NOTIFY unstoppableOperationHappeningUpdated)
    bool unstoppableOperationHappening() const;

    Q_PROPERTY(bool Initialized READ isInitialized NOTIFY initialized)
    bool isInitialized() const;

public Q_SLOTS:
    void setFlightMode(bool enabled);

    void setwifiEnabled(bool enabled);

Q_SIGNALS:
    void flightModeUpdated(bool);

//    void limitationsUpdated(const QStringList&);

//    void statusUpdated(const QString&);

    void wifiEnabledUpdated(bool);

    void unstoppableOperationHappeningUpdated(bool);

    void initialized();

protected:
    class Priv;
    std::shared_ptr<Priv> d;
};

}
