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
 * Author: Pete Woods <pete.woods@canonical.com>
 */

#pragma once

#include <QObject>
#include <QSet>

#include <nmofono/connection/active-connection.h>

namespace nmofono
{
namespace connection
{

class ActiveConnectionManager: public QObject
{
    Q_OBJECT

public:
    UNITY_DEFINES_PTRS(ActiveConnectionManager);

    ActiveConnectionManager(const QDBusConnection& systemConnection);

    ~ActiveConnectionManager() = default;

    QSet<ActiveConnection::SPtr> connections() const;

    bool deactivate(ActiveConnection::SPtr activeConnection);

Q_SIGNALS:
    void connectionsChanged(const QSet<ActiveConnection::SPtr>& connections);

    void connectionsUpdated();

protected:
    class Priv;
    std::shared_ptr<Priv> d;
};

}
}
