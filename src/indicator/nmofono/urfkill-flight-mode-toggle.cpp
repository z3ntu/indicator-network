/*
 * Copyright © 2014-2016 Canonical Ltd.
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
 *     Marcus Tomlinson <marcus.tomlinson@canonical.com>
 *     Pete Woods <pete.woods@canonical.com>
 */

#include <backend-utils.h>
#include <dbus-types.h>
#include <nmofono/urfkill-flight-mode-toggle.h>

#include <URfkillInterface.h>

using namespace std;

namespace nmofono
{

class UrfkillFlightModeToggle::Private: public QObject
{
    Q_OBJECT

public:
    UrfkillFlightModeToggle& p;


    shared_ptr<OrgFreedesktopURfkillInterface> m_urfkill;

    bool m_flightMode = false;

    Private(UrfkillFlightModeToggle& parent)
        : p(parent)
    {}

public Q_SLOTS:
    void setFlightMode(bool flightMode)
    {
        if (flightMode == m_flightMode)
        {
            return;
        }

        m_flightMode = flightMode;
        Q_EMIT p.flightModeChanged(flightMode);
    }
};


UrfkillFlightModeToggle::UrfkillFlightModeToggle(const QDBusConnection& systemBus) : d(new Private(*this))
{
    d->m_urfkill = make_shared<OrgFreedesktopURfkillInterface>(DBusTypes::URFKILL_BUS_NAME, DBusTypes::URFKILL_OBJ_PATH, systemBus);

    auto reply = d->m_urfkill->IsFlightMode();
    reply.waitForFinished();
    d->setFlightMode(reply.isValid() ? reply.value() : false);
    connect(d->m_urfkill.get(), &OrgFreedesktopURfkillInterface::FlightModeChanged, d.get(), &Private::setFlightMode);
}

UrfkillFlightModeToggle::~UrfkillFlightModeToggle()
{}

bool UrfkillFlightModeToggle::setFlightMode(bool enable)
{
    if (enable == d->m_flightMode)
    {
        return true;
    }

    try
    {
        return utils::getOrThrow(d->m_urfkill->FlightMode(enable));
    }
    catch (std::runtime_error& e)
    {
        qWarning() << e.what();
        return false;
    }
}

bool UrfkillFlightModeToggle::isFlightMode() const
{
    return d->m_flightMode;
}

bool UrfkillFlightModeToggle::isValid() const
{
    return true;
}

}

#include "urfkill-flight-mode-toggle.moc"
