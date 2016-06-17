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

#include <nmofono/wwan/sim.h>

#include <ofono/dbus.h>

#define slots
#include <qofono-qt5/qofonoconnectionmanager.h>
#undef slots

using namespace std;

namespace nmofono
{
namespace wwan
{
namespace
{

}

Sim::Ptr Sim::fromQOfonoSimWrapper(const QOfonoSimWrapper *wrapper)
{
    auto sim = Sim::Ptr(new Sim(wrapper->iccid(),
                                "",
                                "",
                                wrapper->mcc(),
                                wrapper->mnc(),
                                wrapper->preferredLanguages(),
                                false));
    sim->setOfonoSimManager(wrapper->ofonoSimManager());
    return sim;
}

class Sim::Private : public QObject, public std::enable_shared_from_this<Private>
{
    Q_OBJECT

public:

    Sim& p;

    int m_index = -1;

    QString m_simIdentifier;

    shared_ptr<QOfonoSimManager> m_simManager;
    shared_ptr<QOfonoConnectionManager> m_connManager;

    QSet<QString> m_interfaces;

    QString m_iccid;
    QString m_imsi;
    QStringList m_phoneNumbers;
    QString m_primaryPhoneNumber;
    QString m_mcc;
    QString m_mnc;
    QStringList m_preferredLanguages;
    bool m_dataRoamingEnabled = false;
    bool m_mobileDataEnabled = false;

    bool m_locked = false;

    bool m_initialData = false;
    bool m_initialDataSet = false;

    Private(Sim &parent)
        : p(parent)
    {
    }

    Private(Sim& parent, shared_ptr<QOfonoSimManager> simmgr)
        : p(parent)
    {
        simManagerChanged(simmgr);


        /// @todo hook up with system-settings to allow changing the identifier.
        if (m_simIdentifier.isEmpty())
        {
            setSimIdentifier(m_primaryPhoneNumber);
        }
    }

public Q_SLOTS:


    void simManagerChanged(shared_ptr<QOfonoSimManager> simmgr)
    {
        if (m_simManager == simmgr)
        {
            return;
        }

        m_simManager = simmgr;
        connect(simmgr.get(), &QOfonoSimManager::subscriberIdentityChanged, this, &Private::imsiChanged);
        connect(simmgr.get(), &QOfonoSimManager::subscriberNumbersChanged, this, &Private::phoneNumbersChanged);
        update();

        Q_EMIT p.presentChanged(m_simManager.get() != nullptr);
    }

    void phoneNumbersChanged(const QStringList &value)
    {
        if (value.isEmpty())
        {
            return;
        }

        m_phoneNumbers = value;
        m_primaryPhoneNumber = value[0];
        Q_EMIT p.primaryPhoneNumberChanged(m_primaryPhoneNumber);
    }

    void imsiChanged(const QString &value)
    {
        if (value.isEmpty())
        {
            return;
        }

        m_imsi = value;
        Q_EMIT p.imsiChanged(m_imsi);
    }

    void poweredChanged()
    {
        if (!m_initialDataSet)
        {
            m_initialDataSet = true;
            m_initialData = m_connManager->powered();
            Q_EMIT p.initialDataOnSet();
            m_connManager->setPowered(m_mobileDataEnabled);
        }
        update();
    }

    void setConnManager(shared_ptr<QOfonoConnectionManager> connmgr)
    {
        if (m_connManager == connmgr)
        {
            return;
        }

        m_connManager = connmgr;
        if (m_connManager)
        {
            connect(m_connManager.get(),
                    &QOfonoConnectionManager::poweredChanged, this,
                    &Private::poweredChanged);
            connect(m_connManager.get(),
                    &QOfonoConnectionManager::roamingAllowedChanged, this,
                    &Private::update);

            m_connManager->setPowered(m_mobileDataEnabled);
            m_connManager->setRoamingAllowed(m_dataRoamingEnabled);
        }

        update();
    }

    void update()
    {
        if (m_connManager)
        {
            bool powered = m_connManager->powered();
            bool roamingAllowed = m_connManager->roamingAllowed();

            /*
             * Connectivity Service is the policy manager for the system.
             * If ofono has different settings force them back to stored values.
             */
            if (m_mobileDataEnabled != powered)
            {
                m_connManager->setPowered(m_mobileDataEnabled);
            }
            if (m_dataRoamingEnabled != roamingAllowed)
            {
                m_connManager->setRoamingAllowed(m_dataRoamingEnabled);
            }
        }
        else
        {
            /* empty */
        }
    }

    void setSimIdentifier(const QString& simIdentifier)
    {
        if (m_simIdentifier == simIdentifier)
        {
            return;
        }

        m_simIdentifier = simIdentifier;
        Q_EMIT p.simIdentifierUpdated(m_simIdentifier);
    }

    void setOfono(shared_ptr<QOfonoSimManager> simmgr)
    {
        simManagerChanged(simmgr);
        if (simmgr)
        {
            auto connManager = make_shared<QOfonoConnectionManager>(this);
            connManager->setModemPath(simmgr->modemPath());
            setConnManager(connManager);
        }
        else
        {
            setConnManager(shared_ptr<QOfonoConnectionManager>());
        }
    }


};

Sim::Sim(const QString &iccid,
         const QString &imsi,
         const QString &primaryPhoneNumber,
         const QString &mcc,
         const QString &mnc,
         const QStringList &preferredLanguages,
         bool dataRoamingEnabled)
    : d{new Private(*this)}
{
    d->m_iccid = iccid;
    d->m_imsi = imsi;
    d->m_primaryPhoneNumber = primaryPhoneNumber;
    d->m_mcc = mcc;
    d->m_mnc = mnc;
    d->m_preferredLanguages = preferredLanguages;
    d->m_dataRoamingEnabled = dataRoamingEnabled;
}

Sim::~Sim()
{}

const QString&
Sim::simIdentifier() const
{
    return d->m_simIdentifier;
}

QString Sim::iccid() const
{
    return d->m_iccid;
}

QString Sim::imsi() const
{
    return d->m_imsi;
}

QString Sim::primaryPhoneNumber() const
{
    return d->m_primaryPhoneNumber;
}

bool Sim::locked() const
{
    return d->m_locked;
}

bool Sim::present() const
{
    return d->m_simManager.get() != nullptr;
}

QString Sim::mcc() const
{
    return d->m_mcc;
}

QString Sim::mnc() const
{
    return d->m_mnc;
}

QList<QString> Sim::preferredLanguages() const
{
    return d->m_preferredLanguages;
}

bool Sim::dataRoamingEnabled() const
{
    return d->m_dataRoamingEnabled;
}

void Sim::setDataRoamingEnabled(bool value)
{
    if (d->m_dataRoamingEnabled == value)
    {
        return;
    }
    d->m_dataRoamingEnabled = value;
    if (d->m_connManager)
    {
        d->m_connManager->setRoamingAllowed(d->m_dataRoamingEnabled);
    }
    Q_EMIT dataRoamingEnabledChanged(value);
}

bool Sim::mobileDataEnabled() const
{
    return d->m_mobileDataEnabled;
}

void Sim::setMobileDataEnabled(bool value)
{
    if (d->m_mobileDataEnabled == value)
    {
        return;
    }
    d->m_mobileDataEnabled = value;
    if (d->m_connManager)
    {
        d->m_connManager->setPowered(d->m_mobileDataEnabled);
    }
    Q_EMIT mobileDataEnabledChanged(value);
}

void Sim::unlock()
{

}

QString Sim::ofonoPath() const
{
    if (d->m_simManager)
    {
        return d->m_simManager->objectPath();
    }
    return QString();
}

void Sim::setOfonoSimManager(std::shared_ptr<QOfonoSimManager> simmgr)
{
    d->setOfono(simmgr);
}

bool Sim::initialDataOn() const
{
    return d->m_initialData;
}


}
}

#include "sim.moc"
