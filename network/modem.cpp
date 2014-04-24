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

#include "modem.h"

class Modem::Private
{
public:
    org::ofono::Interface::Modem::Ptr m_ofonoModem;
    core::Property<Modem::SimStatus> m_simStatus;
    core::Property<Modem::PinType> m_requiredPin;
    core::Property<std::map<Modem::PinType, std::uint8_t>> m_retries;
    core::Property<std::string> m_subscriberIdentity;

    core::Property<std::string> m_operatorName;
    core::Property<org::ofono::Interface::NetworkRegistration::Status> m_status;
    core::Property<std::int8_t> m_strength;
    core::Property<org::ofono::Interface::NetworkRegistration::Technology> m_technology;

    void networkRegistrationChanged(org::ofono::Interface::NetworkRegistration::Ptr netreg);
    void simManagerChanged(org::ofono::Interface::SimManager::Ptr simmgr);

    void simPresentChanged(bool value);
    void pinRequiredChanged(org::ofono::Interface::SimManager::PinType pin);
    void retriesChanged(std::map<org::ofono::Interface::SimManager::PinType, std::uint8_t> retries) ;
};

void
Modem::Private::networkRegistrationChanged(org::ofono::Interface::NetworkRegistration::Ptr netreg)
{
    if (netreg) {
        m_operatorName.set(netreg->operatorName.get());
        netreg->operatorName.changed().connect([this](std::string value){
            m_operatorName.set(value);
        });

        m_status.set(netreg->status.get());
        netreg->status.changed().connect([this](org::ofono::Interface::NetworkRegistration::Status value){
            m_status.set(value);
        });

        m_strength.set(netreg->strength.get());
        netreg->strength.changed().connect([this](std::int8_t value){
            m_strength.set(value);
        });

        m_technology.set(netreg->technology.get());
        netreg->technology.changed().connect([this](org::ofono::Interface::NetworkRegistration::Technology value){
            m_technology.set(value);
        });

    } else {
        m_operatorName.set("");
        m_status.set(org::ofono::Interface::NetworkRegistration::Status::unknown);
        m_strength.set(-1);
        m_technology.set(org::ofono::Interface::NetworkRegistration::Technology::notAvailable);
    }
}


void
Modem::Private::simManagerChanged(org::ofono::Interface::SimManager::Ptr simmgr)
{
    if (simmgr) {
        simmgr->present.changed().connect(std::bind(&Private::simPresentChanged, this, std::placeholders::_1));
        simPresentChanged(simmgr->present.get());

        simmgr->pinRequired.changed().connect(std::bind(&Private::pinRequiredChanged, this, std::placeholders::_1));
        pinRequiredChanged(simmgr->pinRequired.get());

        simmgr->retries.changed().connect(std::bind(&Private::retriesChanged, this, std::placeholders::_1));

        /// @todo subscriberIdentity

    } else {
        m_simStatus.set(SimStatus::offline);
        m_requiredPin.set(PinType::none);
    }
}

void
Modem::Private::pinRequiredChanged(org::ofono::Interface::SimManager::PinType pin)
{
    switch(pin)
    {
    case org::ofono::Interface::SimManager::PinType::none:
        m_requiredPin.set(PinType::none);
        m_simStatus.set(SimStatus::ready);
        break;
    case org::ofono::Interface::SimManager::PinType::pin:
        m_requiredPin.set(PinType::pin);
        m_simStatus.set(SimStatus::locked);
        break;
    case org::ofono::Interface::SimManager::PinType::puk:
        m_requiredPin.set(PinType::puk);
        m_simStatus.set(SimStatus::locked);
        break;
    default:
        throw std::runtime_error("Ofono requires a PIN we have not been prepared to handle (" +
                                 org::ofono::Interface::SimManager::pin2str(pin) +
                                 "). Bailing out.");
    }
}

void
Modem::Private::retriesChanged(std::map<org::ofono::Interface::SimManager::PinType, std::uint8_t> retries)
{
    std::map<Modem::PinType, std::uint8_t> tmp;
    for (auto element : retries) {
        switch(element.first) {
        case org::ofono::Interface::SimManager::PinType::pin:
            tmp[Modem::PinType::pin] = element.second;
            break;
        case org::ofono::Interface::SimManager::PinType::puk:
            tmp[Modem::PinType::puk] = element.second;
            break;
        default:
            // don't care
            break;
        }
    }
    m_retries.set(tmp);
}

void
Modem::Private::simPresentChanged(bool value)
{
    if (!value) {
        m_simStatus.set(SimStatus::missing);
    }
}


Modem::Modem(org::ofono::Interface::Modem::Ptr ofonoModem)
{
    d.reset(new Private);
    d->m_ofonoModem = ofonoModem;

    d->simManagerChanged(d->m_ofonoModem->simManager.get());
    d->m_ofonoModem->simManager.changed().connect([this](org::ofono::Interface::SimManager::Ptr value){
        d->simManagerChanged(value);
    });

    d->networkRegistrationChanged(d->m_ofonoModem->networkRegistration.get());
    d->m_ofonoModem->networkRegistration.changed().connect([this](org::ofono::Interface::NetworkRegistration::Ptr value){
        d->networkRegistrationChanged(value);
    });
}

Modem::~Modem()
{}

org::ofono::Interface::Modem::Ptr
Modem::ofonoModem() const
{
    return d->m_ofonoModem;
}

bool
Modem::enterPin(PinType type, const std::string &pin)
{
    if (!d->m_ofonoModem->simManager.get()) {
        throw std::runtime_error(std::string(__PRETTY_FUNCTION__) + ": no simManager.");
    }

    switch(type) {
    case PinType::none:
        return true;
    case PinType::pin:
        return d->m_ofonoModem->simManager.get()->enterPin(org::ofono::Interface::SimManager::PinType::pin,
                                                           pin);
    case PinType::puk:
        return d->m_ofonoModem->simManager.get()->enterPin(org::ofono::Interface::SimManager::PinType::puk,
                                                           pin);
        break;
    }

    throw std::logic_error("code should not be reached.");
}


bool
Modem::resetPin(PinType type, const std::string &puk, const std::string &pin)
{
    if (!d->m_ofonoModem->simManager.get()) {
        throw std::runtime_error(std::string(__PRETTY_FUNCTION__) + ": no simManager.");
    }

    switch(type) {
    case PinType::none:
        return true;
    case PinType::pin:
        return d->m_ofonoModem->simManager.get()->resetPin(org::ofono::Interface::SimManager::PinType::pin,
                                                           puk,
                                                           pin);
    default:
        throw std::runtime_error(std::string(__PRETTY_FUNCTION__) + ": Not Supported.");
    }
}

bool
Modem::changePin(PinType type, const std::string &oldPin, const std::string &newPin)
{
    if (!d->m_ofonoModem->simManager.get()) {
        throw std::runtime_error(std::string(__PRETTY_FUNCTION__) + ": no simManager.");
    }

    switch(type) {
    case PinType::none:
        return true;
    case PinType::pin:
        return d->m_ofonoModem->simManager.get()->changePin(org::ofono::Interface::SimManager::PinType::pin,
                                                            oldPin,
                                                            newPin);
    case PinType::puk:
        return d->m_ofonoModem->simManager.get()->changePin(org::ofono::Interface::SimManager::PinType::puk,
                                                            oldPin,
                                                            newPin);
        break;
    }

    throw std::logic_error("code should not be reached.");
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

const core::Property<std::string> &
Modem::subscriberIdentity()
{
    return d->m_subscriberIdentity;
}

const core::Property<std::string> &
Modem::operatorName()
{
    return d->m_operatorName;
}

const core::Property<org::ofono::Interface::NetworkRegistration::Status> &
Modem::status()
{
    return d->m_status;
}

const core::Property<std::int8_t> &
Modem::strength()
{
    return d->m_strength;
}

const core::Property<org::ofono::Interface::NetworkRegistration::Technology> &
Modem::technology()
{
    return d->m_technology;
}
