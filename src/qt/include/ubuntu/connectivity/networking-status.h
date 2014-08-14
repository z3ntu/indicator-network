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

#ifndef CONNECTIVITY_NETWORKING_STATUS_H
#define CONNECTIVITY_NETWORKING_STATUS_H

#include <QObject>
#include <QScopedPointer>
#include <QVector>

namespace ubuntu {
namespace connectivity {


class NetworkingStatus : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(NetworkingStatus)

    Q_ENUMS(Limitations)
    Q_ENUMS(Status)

    Q_PROPERTY(QVector<Limitations> limitations
               READ limitations
               NOTIFY limitationsChanged)
    Q_PROPERTY(Status status
               READ status
               NOTIFY statusChanged)

public:
    explicit NetworkingStatus(QObject *parent = 0);
    virtual ~NetworkingStatus();

    enum Limitations {
        Bandwith
    };

    enum Status {
        Offline,
        Connecting,
        Online
    };

    QVector<Limitations> limitations() const;
    Status status() const;

Q_SIGNALS:
    void limitationsChanged();
    void statusChanged(Status value);

private:
    class Private;
    QScopedPointer<Private> d;
};

}
}


Q_DECLARE_METATYPE(ubuntu::connectivity::NetworkingStatus::Limitations)
Q_DECLARE_METATYPE(QVector<ubuntu::connectivity::NetworkingStatus::Limitations>)
Q_DECLARE_METATYPE(ubuntu::connectivity::NetworkingStatus::Status)

#endif // CONNECTIVITY_NETWORKING_STATUS_H
