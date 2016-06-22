/*
 * Copyright (C) 2016 Canonical, Ltd.
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

#pragma once

#include <QObject>
#include <QSettings>
#include <QString>

#include <nmofono/wwan/sim.h>

#include <memory>


namespace nmofono
{

class ConnectivityServiceSettings : public QObject
{
    Q_OBJECT

    class Private;
    std::shared_ptr<Private> d;

public:

    typedef std::shared_ptr<ConnectivityServiceSettings> Ptr;
    typedef std::weak_ptr<ConnectivityServiceSettings> WeakPtr;

    ConnectivityServiceSettings(QObject *parent = 0);
    virtual ~ConnectivityServiceSettings();

    QVariant mobileDataEnabled();
    void setMobileDataEnabled(bool value);

    QVariant simForMobileData();
    void setSimForMobileData(const QString &iccid);

    QStringList knownSims();
    void setKnownSims(const QStringList &list);

    wwan::Sim::Ptr createSimFromSettings(const QString &iccid);
    void saveSimToSettings(wwan::Sim::Ptr sim);

public Q_SLOTS:

Q_SIGNALS:

};

}
