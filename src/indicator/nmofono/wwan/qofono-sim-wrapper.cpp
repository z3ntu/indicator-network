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

#include <nmofono/wwan/qofono-sim-wrapper.h>

#include <ofono/dbus.h>

using namespace std;

namespace nmofono
{
namespace wwan
{
namespace
{

}

class QOfonoSimWrapper::Private : public QObject, public std::enable_shared_from_this<Private>
{
    Q_OBJECT

public:

    QOfonoSimWrapper& p;
    shared_ptr<QOfonoSimManager> m_simManager;

    QString m_iccid;

    bool m_present = false;

    bool m_iccidSet = false;


    Private(QOfonoSimWrapper& parent, shared_ptr<QOfonoSimManager> simmgr)
        : p(parent), m_simManager{simmgr}
    {
        connect(simmgr.get(), &QOfonoSimManager::presenceChanged, this, &Private::presentChanged);
        connect(simmgr.get(), &QOfonoSimManager::cardIdentifierChanged, this, &Private::iccidChanged);
    }

public Q_SLOTS:

    void presentChanged(bool value)
    {
        if (m_present == value)
        {
            return;
        }
        m_present = value;
        if (!m_present)
        {
            m_iccidSet = false;
            Q_EMIT p.readyChanged(false);
        }
        Q_EMIT p.presentChanged(value);
    }

    void iccidChanged(const QString &value)
    {
        if (value.isEmpty())
        {
            return;
        }

        if (m_iccidSet)
        {
            qWarning() << "Unexpected update on ICCID: " << m_iccid << ", " << value;
        }

        m_iccid = value;
        m_iccidSet = true;
        if (p.ready())
        {
            Q_EMIT p.readyChanged(true);
        }
    }
};



QOfonoSimWrapper::QOfonoSimWrapper(std::shared_ptr<QOfonoSimManager> simmgr)
    : d{new Private(*this, simmgr)}
{
}

QOfonoSimWrapper::~QOfonoSimWrapper()
{}

QString QOfonoSimWrapper::iccid() const
{
    return d->m_iccid;
}

bool QOfonoSimWrapper::present() const
{
    return d->m_present;
}


bool QOfonoSimWrapper::ready() const {
    return d->m_iccidSet;
}

std::shared_ptr<QOfonoSimManager> QOfonoSimWrapper::ofonoSimManager() const
{
    return d->m_simManager;
}


}
}

#include "qofono-sim-wrapper.moc"
