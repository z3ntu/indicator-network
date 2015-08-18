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

    Q_DISABLE_COPY(Connectivity)

    Q_ENUMS(Limitations)
    Q_ENUMS(Status)

    static void registerMetaTypes();

    /**
      * @brief enum for networking limitations
      *
      * Networking limitations may be accessed through the NetworkingStatus::limitations property.
      */
    enum class Limitations
    {
        /**
         * indicates that the bandwith of the Internet connection has limitations.
         * Applications should minimize their bandwith usage if possible.
         */
        Bandwith
    };

    /**
      * @brief enum for networking status
      *
      * Networking status may be accessed through the NetworkingStatus::status property.
      */
    enum class Status
    {
        Offline,     /**< No Internet connection available.            */
        Connecting,  /**< System is actively establising a connection. */
        Online       /**< System is connected to the Internet.         */
    };

    Connectivity(const QDBusConnection& sessionConnection = QDBusConnection::sessionBus(), QObject* parent = 0);

    ~Connectivity();

    Q_PROPERTY(bool flightMode READ flightMode WRITE setFlightMode NOTIFY flightModeUpdated)
    Q_PROPERTY(bool FlightMode READ flightMode WRITE setFlightMode NOTIFY flightModeUpdated)
    bool flightMode() const;

    Q_PROPERTY(bool online READ online NOTIFY onlineUpdated)
    bool online() const;

    Q_PROPERTY(bool limitedBandwith READ limitedBandwith NOTIFY limitedBandwithUpdated)
    bool limitedBandwith() const;

    Q_PROPERTY(QVector<Limitations> Limitations READ limitations NOTIFY limitationsUpdated)
    QVector<Limitations> limitations() const;

    Q_PROPERTY(connectivityqt::Connectivity::Status Status READ status NOTIFY statusUpdated)
    Status status() const;

    Q_PROPERTY(bool wifiEnabled READ wifiEnabled WRITE setwifiEnabled NOTIFY wifiEnabledUpdated)
    Q_PROPERTY(bool WifiEnabled READ wifiEnabled WRITE setwifiEnabled NOTIFY wifiEnabledUpdated)
    bool wifiEnabled() const;

    Q_PROPERTY(bool UnstoppableOperationHappening READ unstoppableOperationHappening NOTIFY unstoppableOperationHappeningUpdated)
    bool unstoppableOperationHappening() const;

    Q_PROPERTY(bool FlightModeSwitchEnabled READ flightModeSwitchEnabled NOTIFY flightModeSwitchEnabledUpdated)
    Q_PROPERTY(bool flightModeSwitchEnabled READ flightModeSwitchEnabled NOTIFY flightModeSwitchEnabledUpdated)
    bool flightModeSwitchEnabled() const;

    Q_PROPERTY(bool WifiSwitchEnabled READ wifiSwitchEnabled NOTIFY wifiSwitchEnabledUpdated)
    Q_PROPERTY(bool wifiSwitchEnabled READ wifiSwitchEnabled NOTIFY wifiSwitchEnabledUpdated)
    bool wifiSwitchEnabled() const;

    Q_PROPERTY(bool HotspotSwitchEnabled READ hotspotSwitchEnabled NOTIFY hotspotSwitchEnabledUpdated)
    Q_PROPERTY(bool hotspotSwitchEnabled READ hotspotSwitchEnabled NOTIFY hotspotSwitchEnabledUpdated)
    bool hotspotSwitchEnabled() const;

    Q_PROPERTY(QByteArray hotspotSsid READ hotspotSsid WRITE setHotspotSsid NOTIFY hotspotSsidUpdated)
    QByteArray hotspotSsid() const;

    Q_PROPERTY(QString hotspotPassword READ hotspotPassword WRITE setHotspotPassword NOTIFY hotspotPasswordUpdated)
    QString hotspotPassword() const;

    Q_PROPERTY(bool hotspotEnabled READ hotspotEnabled WRITE setHotspotEnabled NOTIFY hotspotEnabledUpdated)
    bool hotspotEnabled() const;

    Q_PROPERTY(QString hotspotMode READ hotspotMode WRITE setHotspotMode NOTIFY hotspotModeUpdated)
    QString hotspotMode() const;

    Q_PROPERTY(bool hotspotStored READ hotspotStored NOTIFY hotspotStoredUpdated)
    bool hotspotStored() const;

    Q_PROPERTY(bool Initialized READ isInitialized NOTIFY initialized)
    bool isInitialized() const;

public Q_SLOTS:
    void setFlightMode(bool enabled);

    void setwifiEnabled(bool enabled);

    void setHotspotEnabled(bool active);

    void setHotspotSsid(const QByteArray& ssid);

    void setHotspotPassword(const QString& password);

    void setHotspotMode(const QString& mode);

Q_SIGNALS:
    void flightModeUpdated(bool);

    void onlineUpdated(bool value);

    void limitedBandwithUpdated(bool value);

    void limitationsUpdated(const QVector<Limitations>&);

    void statusUpdated(connectivityqt::Connectivity::Status value);

    void wifiEnabledUpdated(bool);

    void unstoppableOperationHappeningUpdated(bool);

    void flightModeSwitchEnabledUpdated(bool);

    void wifiSwitchEnabledUpdated(bool);

    void hotspotSwitchEnabledUpdated(bool);

    void hotspotSsidUpdated(const QByteArray& name);

    void hotspotPasswordUpdated(const QString& password);

    void hotspotEnabledUpdated(bool);

    void hotspotModeUpdated(const QString& mode);

    void hotspotStoredUpdated(bool);

    void reportError(int reason);

    void initialized();

protected:
    class Priv;
    std::shared_ptr<Priv> d;
};

}

Q_DECLARE_METATYPE(connectivityqt::Connectivity::Limitations)
Q_DECLARE_METATYPE(QVector<connectivityqt::Connectivity::Limitations>)
Q_DECLARE_METATYPE(connectivityqt::Connectivity::Status)

