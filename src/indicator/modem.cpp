/*
 * Copyright (C) 2014 Canonical, Ltd.
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

#include <modem.h>

#include <ofono/dbus.h>

#undef QT_NO_KEYWORDS
#include <qofono-qt5/qofonomodem.h>
#include <qofono-qt5/qofonosimmanager.h>
#include <qofono-qt5/qofononetworkregistration.h>
#include <qofono-qt5/qofonoconnectionmanager.h>
#define QT_NO_KEYWORDS = 1

using namespace std;

namespace
{

static Modem::Status str2status(const QString& str)
{
    if (str == "unregistered")
        return Modem::Status::unregistered;
    if (str == "registered")
        return Modem::Status::registered;
    if (str == "searching")
        return Modem::Status::searching;
    if (str == "denied")
        return Modem::Status::denied;
    if (str == "unknown")
        return Modem::Status::unknown;
    if (str == "roaming")
        return Modem::Status::roaming;

    throw std::runtime_error(std::string(__PRETTY_FUNCTION__) + ": Unknown status '" + str.toStdString() + "'");
}

static Modem::Technology str2technology(const QString& str)
{
    if (str == "")
        return Modem::Technology::notAvailable;
    if (str == "gsm")
        return Modem::Technology::gsm;
    if (str == "edge")
        return Modem::Technology::edge;
    if (str == "umts")
        return Modem::Technology::umts;
    if (str == "hspa")
        return Modem::Technology::hspa;
    if (str == "lte")
        return Modem::Technology::lte;

    throw std::runtime_error(std::string(__PRETTY_FUNCTION__) + ": Unknown techonology '" + str.toStdString() + "'");
}
}

class Modem::Private : public QObject, public std::enable_shared_from_this<Private>
{
    Q_OBJECT

public:
    core::Property<bool> m_online;

    shared_ptr<QOfonoModem> m_ofonoModem;
    core::Property<Modem::SimStatus> m_simStatus;
    core::Property<Modem::PinType> m_requiredPin;
    core::Property<std::map<Modem::PinType, std::uint8_t>> m_retries;

    core::Property<QString> m_operatorName;
    core::Property<Modem::Status> m_status;
    core::Property<std::int8_t> m_strength;
    core::Property<Modem::Technology> m_technology;

    core::Property<bool> m_dataEnabled;

    core::Property<QString> m_simIdentifier;
    int m_index = -1;

    QSet<QString> m_interfaces;

    shared_ptr<QOfonoConnectionManager> m_connectionManager;
    shared_ptr<QOfonoNetworkRegistration> m_networkRegistration;
    shared_ptr<QOfonoSimManager> m_simManager;

    void connectionManagerChanged(shared_ptr<QOfonoConnectionManager> conmgr)
    {
        if (conmgr == m_connectionManager)
        {
            return;
        }

        m_connectionManager = conmgr;
        if (m_connectionManager)
        {
            connect(m_connectionManager.get(),
                    &QOfonoConnectionManager::poweredChanged, this,
                    &Private::update);
        }

        update();
    }

    void networkRegistrationChanged(shared_ptr<QOfonoNetworkRegistration> netreg)
    {
        if (m_networkRegistration == netreg)
        {
            return;
        }

        m_networkRegistration = netreg;
        if (m_networkRegistration)
        {
            connect(m_networkRegistration.get(),
                    &QOfonoNetworkRegistration::nameChanged, this,
                    &Private::update);

            connect(m_networkRegistration.get(),
                    &QOfonoNetworkRegistration::statusChanged, this,
                    &Private::update);

            connect(m_networkRegistration.get(),
                    &QOfonoNetworkRegistration::strengthChanged, this,
                    &Private::update);

            connect(m_networkRegistration.get(),
                    &QOfonoNetworkRegistration::technologyChanged, this,
                    &Private::update);
        }

        update();
    }

    void simManagerChanged(shared_ptr<QOfonoSimManager> simmgr)
    {
        if (m_simManager == simmgr)
        {
            return;
        }

        m_simManager = simmgr;
        if (m_simManager)
        {
            connect(m_simManager.get(),
                    &QOfonoSimManager::presenceChanged, this,
                    &Private::update);

            connect(m_simManager.get(),
                    &QOfonoSimManager::pinRequiredChanged, this,
                    &Private::update);

            // FIXME Bind this signal
//            connect(m_simManager.get(),
//                    &QOfonoSimManager::retriesChanged, this,
//                    &Private::update);
        }

        update();
    }

    void update();

    Private(shared_ptr<QOfonoModem> ofonoModem);

public Q_SLOTS:
    void onlineChanged(bool online)
    {
        m_online.set(online);
    }

    void interfacesChanged(const QStringList& values)
    {
        QSet<QString> interfaces(values.toSet());

        auto toRemove = m_interfaces;
        toRemove.subtract(interfaces);

        auto toAdd = interfaces;
        toAdd.subtract(m_interfaces);

        m_interfaces = interfaces;

        for(const auto& interface: toRemove)
        {
            if (interface == OFONO_CONNECTION_MANAGER_INTERFACE)
            {
                connectionManagerChanged(
                        shared_ptr<QOfonoConnectionManager>());
            }
            else if (interface == OFONO_NETWORK_REGISTRATION_INTERFACE)
            {
                networkRegistrationChanged(
                        shared_ptr<QOfonoNetworkRegistration>());
            }
            else if (interface == OFONO_SIM_MANAGER_INTERFACE)
            {
                simManagerChanged(shared_ptr<QOfonoSimManager>());
            }
        }

        for (const auto& interface: toAdd)
        {
            if (interface == OFONO_CONNECTION_MANAGER_INTERFACE)
            {
                auto connmgr = make_shared<QOfonoConnectionManager>();
                connmgr->setModemPath(m_ofonoModem->modemPath());
                connectionManagerChanged(connmgr);
            }
            else if (interface == OFONO_NETWORK_REGISTRATION_INTERFACE)
            {
                auto netreg = make_shared<QOfonoNetworkRegistration>();
                netreg->setModemPath(m_ofonoModem->modemPath());
                networkRegistrationChanged(netreg);
            }
            else if (interface == OFONO_SIM_MANAGER_INTERFACE)
            {
                auto simmgr = make_shared<QOfonoSimManager>();
                simmgr->setModemPath(m_ofonoModem->modemPath());
                simManagerChanged(simmgr);
            }
        }
    }
};

Modem::Private::Private(shared_ptr<QOfonoModem> ofonoModem)
    : m_ofonoModem{ofonoModem}
{
    auto that = shared_from_this();

    connect(m_ofonoModem.get(), &QOfonoModem::onlineChanged, this, &Private::onlineChanged);
    m_online.set(m_ofonoModem->online());

    connect(m_ofonoModem.get(), &QOfonoModem::interfacesChanged, this, &Private::interfacesChanged);
    interfacesChanged(m_ofonoModem->interfaces());

    /// @todo hook up with system-settings to allow changing the identifier.
    ///       for now just provide the defaults
    auto path = m_ofonoModem->modemPath();
    if (path == "/ril_0") {
        m_simIdentifier.set("SIM 1");
        m_index = 1;
    } else if (path == "/ril_1") {
        m_simIdentifier.set("SIM 2");
        m_index = 2;
    } else {
        m_simIdentifier.set(path);
    }
}


void
Modem::Private::update()
{
    if (m_simManager) {
        // update requiredPin
        switch(m_simManager->pinRequired())
        {
        case QOfonoSimManager::PinType::NoPin:
            m_requiredPin.set(PinType::none);
            break;
        case QOfonoSimManager::PinType::SimPin:
            m_requiredPin.set(PinType::pin);
            break;
        case QOfonoSimManager::PinType::SimPuk:
            m_requiredPin.set(PinType::puk);
            break;
        default:
            throw std::runtime_error("Ofono requires a PIN we have not been prepared to handle (" +
                                     to_string(m_simManager->pinRequired()) +
                                     "). Bailing out.");
        }

        // update retries
        std::map<Modem::PinType, std::uint8_t> tmp;
        // FIXME Figure out what goes here
//        QVariantMap retries = m_simManager->retries();
//        for (auto element : retries) {
//            switch(element.first) {
//            case OfonoPinType::pin:
//                tmp[Modem::PinType::pin] = element.second;
//                break;
//            case OfonoPinType::puk:
//                tmp[Modem::PinType::puk] = element.second;
//                break;
//            default:
//                // don't care
//                break;
//            }
//        }
        m_retries.set(tmp);

        // update simStatus
        bool present = m_simManager->present();
        if (!present)
        {
            m_simStatus.set(SimStatus::missing);
        }
        else if (m_requiredPin == PinType::none)
        {
            m_simStatus.set(SimStatus::ready);
        }
        else
        {
            if (m_retries->count(PinType::puk) != 0
                    && m_retries->at(PinType::puk) == 0)
            {
                m_simStatus.set(SimStatus::permanentlyLocked);
            }
            else
            {
                m_simStatus.set(SimStatus::locked);
            }
        }

    } else {
        m_requiredPin.set(PinType::none);
        m_retries.set({});
        m_simStatus.set(SimStatus::not_available);
    }

    if (m_networkRegistration)
    {
        m_operatorName.set(m_networkRegistration->name());
        m_status.set(str2status(m_networkRegistration->status()));
        m_strength.set(m_networkRegistration->strength());
        m_technology.set(str2technology(m_networkRegistration->technology()));
    }
    else
    {
        m_operatorName.set("");
        m_status.set(Modem::Status::unknown);
        m_strength.set(-1);
        m_technology.set(Modem::Technology::notAvailable);
    }

    if (m_connectionManager)
    {
        m_dataEnabled.set(m_connectionManager->powered());
    }
    else
    {
        m_dataEnabled.set(false);
    }
}

std::string Modem::strengthIcon(int8_t strength)
{
    /* Using same values as used by Android, not linear (LP: #1329945)*/
    if (strength >= 39)
        return "gsm-3g-full";
    else if (strength >= 26)
        return "gsm-3g-high";
    else if (strength >= 16)
        return "gsm-3g-medium";
    else if (strength >= 6)
        return "gsm-3g-low";
    else
        return "gsm-3g-none";
}

std::string Modem::technologyIcon(Modem::Technology tech)
{
    switch (tech){
    case Modem::Technology::notAvailable:
    case Modem::Technology::gsm:
        return "network-cellular-pre-edge";
    case Modem::Technology::edge:
        return "network-cellular-edge";
    case Modem::Technology::umts:
        return "network-cellular-3g";
    case Modem::Technology::hspa:
        return "network-cellular-hspa";
    /// @todo oFono can't tell us about hspa+ yet
    //case Modem::Technology::hspaplus:
    //    break;
    case Modem::Technology::lte:
        return "network-cellular-lte";
    }
    // shouldn't be reached
    return "";
}

Modem::Modem(shared_ptr<QOfonoModem> ofonoModem)
    : d{new Private(ofonoModem)}
{
}

Modem::~Modem()
{}

//org::ofono::Interface::Modem::Ptr
//Modem::ofonoModem() const
//{
//    return d->m_ofonoModem;
//}

void
Modem::enterPin(PinType type, const QString &pin)
{
    if (!d->m_simManager)
    {
        throw std::runtime_error(std::string(__PRETTY_FUNCTION__) + ": no simManager.");
    }

    switch(type) {
    case PinType::none:
        break;
    case PinType::pin:
        d->m_simManager->enterPin(QOfonoSimManager::PinType::SimPin,
                                pin);
        break;
    case PinType::puk:
        d->m_simManager->enterPin(QOfonoSimManager::PinType::SimPuk,
                                pin);
        break;
    }

    throw std::logic_error("code should not be reached.");
}


void
Modem::resetPin(PinType type, const QString &puk, const QString &pin)
{
    if (!d->m_simManager) {
        throw std::runtime_error(std::string(__PRETTY_FUNCTION__) + ": no simManager.");
    }

    switch(type) {
    case PinType::none:
        break;
    case PinType::puk:
        d->m_simManager->resetPin(QOfonoSimManager::PinType::SimPuk,
                                puk,
                                pin);
        break;
    default:
        throw std::runtime_error(std::string(__PRETTY_FUNCTION__) + ": Not Supported.");
    }
}

void
Modem::changePin(PinType type, const QString &oldPin, const QString &newPin)
{
    if (!d->m_simManager) {
        throw std::runtime_error(std::string(__PRETTY_FUNCTION__) + ": no simManager.");
    }

    switch(type) {
    case PinType::none:
        break;
    case PinType::pin:
        d->m_simManager->changePin(QOfonoSimManager::PinType::SimPin,
                                 oldPin,
                                 newPin);
        break;
    case PinType::puk:
        d->m_simManager->changePin(QOfonoSimManager::PinType::SimPuk,
                                 oldPin,
                                 newPin);
        break;
    }

    throw std::logic_error("code should not be reached.");
}

const core::Property<bool> &
Modem::online()
{
    return d->m_online;
}

const core::Property<Modem::SimStatus> &
Modem::simStatus()
{
    return d->m_simStatus;
}

const core::Property<Modem::PinType> &
Modem::requiredPin()
{
    return d->m_requiredPin;
}

const core::Property<std::map<Modem::PinType, std::uint8_t> > &
Modem::retries()
{
    return d->m_retries;
}

const core::Property<QString> &
Modem::operatorName()
{
    return d->m_operatorName;
}

const core::Property<Modem::Status> &
Modem::status()
{
    return d->m_status;
}

const core::Property<std::int8_t> &
Modem::strength()
{
    return d->m_strength;
}

const core::Property<Modem::Technology> &
Modem::technology()
{
    return d->m_technology;
}
const core::Property<QString> &
Modem::simIdentifier()
{
    return d->m_simIdentifier;
}

const core::Property<bool> &
Modem::dataEnabled()
{
    return d->m_dataEnabled;
}

int
Modem::index()
{
    return d->m_index;
}

QString
Modem::name() const
{
    return d->m_ofonoModem->modemPath();
}

#include "modem.moc"
