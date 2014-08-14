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

#ifndef QML_CONNECTIVITY_NETWORKING_STATUS_H
#define QML_CONNECTIVITY_NETWORKING_STATUS_H

#include <QObject>
#include <QThread>

#include <ubuntu/connectivity/NetworkingStatus>

class NetworkingStatus : public ubuntu::connectivity::NetworkingStatus
{
    Q_OBJECT

    Q_PROPERTY(bool limitedBandwith
               READ limitedBandwith
               NOTIFY limitedBandwithChanged)

public:
    explicit NetworkingStatus(QObject *parent = 0);
    virtual ~NetworkingStatus();

    bool limitedBandwith() const;

Q_SIGNALS:
    void limitedBandwithChanged(bool value);
};

#endif // QML_CONNECTIVITY_NETWORKING_STATUS_H
