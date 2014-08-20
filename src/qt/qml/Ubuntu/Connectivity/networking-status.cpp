/*
 * Copyright © 2013 Canonical Ltd.
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

#include "networking-status.h"

NetworkingStatus::NetworkingStatus(QObject *parent)
        : ubuntu::connectivity::NetworkingStatus(parent)
{
    connect(this, &NetworkingStatus::statusChanged, [this](Status value){
        value == Status::Online ? Q_EMIT onlineChanged(true) : Q_EMIT onlineChanged(false);
    });
}

NetworkingStatus::~NetworkingStatus()
{}

bool
NetworkingStatus::online() const
{
    return status() == Status::Online;
}

bool
NetworkingStatus::limitedBandwith() const
{
    return limitations().contains(Limitations::Bandwith);
}
