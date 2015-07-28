/*
 * Copyright © 2015 Canonical Ltd.
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
 *     Pete Woods <pete.woods@canonical.com>
 */

#include <qpowerd/qpowerd.h>
#include <dbus-types.h>
#include <PowerdInterface.h>

using namespace std;

class QPowerd::Priv
{
public:
    shared_ptr<ComCanonicalPowerdInterface> m_powerd;
};

class QPowerd::QSysStateRequest
{
public:
    QSysStateRequest(QPowerd& parent, const QString& cookie)
            : m_parent(parent), m_cookie(cookie)
    {

    }

    ~QSysStateRequest()
    {
        m_parent.d->m_powerd->clearSysState(m_cookie);
    }

protected:
    QPowerd m_parent;

    QString m_cookie;
};

QPowerd::QPowerd(const QDBusConnection& connection) :
        d(new Priv)
{
    d->m_powerd = make_shared<ComCanonicalPowerdInterface>(
                DBusTypes::POWERD_DBUS_NAME, DBusTypes::POWERD_DBUS_PATH,
                connection);
}

QPowerd::~QPowerd()
{
}

QPowerd::RequestSPtr QPowerd::requestSysState(const QString& name, SysPowerState state)
{
    auto reply = d->m_powerd->requestSysState(name, static_cast<int>(state));
    QString cookie;
    reply.waitForFinished();
    if (reply.isError())
    {
        qWarning() << __PRETTY_FUNCTION__ << reply.error().message();
    }
    else
    {
        cookie = reply;
    }
    return make_shared<QSysStateRequest>(*this, cookie);
}

