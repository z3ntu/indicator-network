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
    org::ofono::Interface::Modem::Ptr ofonoModem;
    core::Property<Modem::SimStatus> simStatus;
    core::Property<Modem::PinType> requiredPin;
    core::Property<std::map<Modem::PinType, std::uint8_t>> retries;
    core::Property<std::string> subscriberIdentity;

    core::Property<std::string> operatorName;
    core::Property<org::ofono::Interface::NetworkRegistration::Status> status;
    core::Property<std::int8_t> strength;
    core::Property<org::ofono::Interface::NetworkRegistration::Technology> technology;

    void networkRegistrationChanged(org::ofono::Interface::NetworkRegistration::Ptr netreg);
    void simManagerChanged(org::ofono::Interface::SimManager::Ptr simmgr);

    void simPresentChanged(bool value);
    void pinRequiredChanged(org::ofono::Interface::SimManager::PinType pin);
};

void
Modem::Private::networkRegistrationChanged(org::ofono::Interface::NetworkRegistration::Ptr netreg)
{
    std::cout << __PRETTY_FUNCTION__ << std::endl;
    std::cout << "NETWORK REGISTRATION CHANGED: " << netreg << std::endl;
    if (netreg) {
        operatorName.set(netreg->operatorName.get());
        netreg->operatorName.changed().connect([this](std::string value){
            operatorName.set(value);
        });

        status.set(netreg->status.get());
        netreg->status.changed().connect([this](org::ofono::Interface::NetworkRegistration::Status value){
            status.set(value);
        });

        strength.set(netreg->strength.get());
        netreg->strength.changed().connect([this](std::int8_t value){
            strength.set(value);
        });

        technology.set(netreg->technology.get());
        netreg->technology.changed().connect([this](org::ofono::Interface::NetworkRegistration::Technology value){
            technology.set(value);
        });

    } else {
        operatorName.set("");
        status.set(org::ofono::Interface::NetworkRegistration::Status::unknown);
        strength.set(-1);
        technology.set(org::ofono::Interface::NetworkRegistration::Technology::notAvailable);
    }
}


void
Modem::Private::simManagerChanged(org::ofono::Interface::SimManager::Ptr simmgr)
{
    std::cout << __PRETTY_FUNCTION__ << std::endl;
    std::cout << "SIM MANAGER CHANGED: " << simmgr << std::endl;
    if (simmgr) {
        simmgr->present.changed().connect(std::bind(&Private::simPresentChanged, this, std::placeholders::_1));
        simPresentChanged(simmgr->present.get());

        simmgr->pinRequired.changed().connect(std::bind(&Private::pinRequiredChanged, this, std::placeholders::_1));
        pinRequiredChanged(simmgr->pinRequired.get());

        /// @todo retries
        /// @todo subscriberIdentity

    } else {
        simStatus.set(SimStatus::offline);
        requiredPin.set(PinType::none);
    }
}

void
Modem::Private::pinRequiredChanged(org::ofono::Interface::SimManager::PinType pin)
{
    std::cout << __PRETTY_FUNCTION__ << std::endl;
    std::cout << "PIN REQUIRED CHANGED: " << (int)pin << std::endl;
    switch(pin)
    {
    case org::ofono::Interface::SimManager::PinType::none:
        requiredPin.set(PinType::none);
        simStatus.set(SimStatus::ready);
        break;
    case org::ofono::Interface::SimManager::PinType::pin:
        requiredPin.set(PinType::pin);
        simStatus.set(SimStatus::locked);
        break;
    case org::ofono::Interface::SimManager::PinType::puk:
        requiredPin.set(PinType::puk);
        simStatus.set(SimStatus::locked);
        break;
    default:
        throw std::runtime_error("Ofono requires a PIN we have not been prepared to handle (" +
                                 org::ofono::Interface::SimManager::pin2str(pin) +
                                 "). Bailing out.");
    }
}

void
Modem::Private::simPresentChanged(bool value)
{
    std::cout << "PRESENT CHANGED: " << value << std::endl;
    if (!value) {
        simStatus.set(SimStatus::missing);
    }
}


Modem::Modem(org::ofono::Interface::Modem::Ptr ofonoModem)
{
    std::cout << __PRETTY_FUNCTION__ << std::endl;

    d.reset(new Private);
    d->ofonoModem = ofonoModem;

    d->simManagerChanged(d->ofonoModem->simManager.get());
    d->ofonoModem->simManager.changed().connect([this](org::ofono::Interface::SimManager::Ptr value){
        d->simManagerChanged(value);
    });

    d->networkRegistrationChanged(d->ofonoModem->networkRegistration.get());
    d->ofonoModem->networkRegistration.changed().connect([this](org::ofono::Interface::NetworkRegistration::Ptr value){
        d->networkRegistrationChanged(value);
    });
}

Modem::~Modem()
{
    std::cout << __PRETTY_FUNCTION__ << std::endl;
}

org::ofono::Interface::Modem::Ptr
Modem::ofonoModem() const
{
    return d->ofonoModem;
}

void
Modem::enterPin(PinType type, const std::string &pin)
{
    if (!d->ofonoModem->simManager.get()) {
        throw std::runtime_error(std::string(__PRETTY_FUNCTION__) + ": no simManager.");
    }

    std::cout << __PRETTY_FUNCTION__ << std::endl;
    switch(type) {
    case PinType::none:
        return;
    case PinType::pin:
        d->ofonoModem->simManager.get()->enterPin(org::ofono::Interface::SimManager::PinType::pin,
                                                  pin);
    case PinType::puk:
        d->ofonoModem->simManager.get()->enterPin(org::ofono::Interface::SimManager::PinType::puk,
                                                  pin);
        break;
    }
}


void
Modem::resetPin(PinType type, const std::string &puk, const std::string &pin)
{
    if (!d->ofonoModem->simManager.get()) {
        throw std::runtime_error(std::string(__PRETTY_FUNCTION__) + ": no simManager.");
    }

    std::cout << __PRETTY_FUNCTION__ << std::endl;
    switch(type) {
    case PinType::none:
        return;
    case PinType::pin:
        d->ofonoModem->simManager.get()->resetPin(org::ofono::Interface::SimManager::PinType::pin,
                                                  puk,
                                                  pin);
    default:
        throw std::runtime_error(std::string(__PRETTY_FUNCTION__) + ": Not Supported.");
    }
}

void
Modem::changePin(PinType type, const std::string &oldPin, const std::string &newPin)
{
    if (!d->ofonoModem->simManager.get()) {
        throw std::runtime_error(std::string(__PRETTY_FUNCTION__) + ": no simManager.");
    }

    std::cout << __PRETTY_FUNCTION__ << std::endl;
    switch(type) {
    case PinType::none:
        return;
    case PinType::pin:
        d->ofonoModem->simManager.get()->changePin(org::ofono::Interface::SimManager::PinType::pin,
                                                   oldPin,
                                                   newPin);
    case PinType::puk:
        d->ofonoModem->simManager.get()->changePin(org::ofono::Interface::SimManager::PinType::puk,
                                                   oldPin,
                                                   newPin);
        break;
    }
}

const core::Property<Modem::SimStatus> &
Modem::simStatus()
{
    return d->simStatus;
}

const core::Property<Modem::PinType> &
Modem::requiredPin()
{
    return d->requiredPin;
}

const core::Property<std::map<Modem::PinType, std::uint8_t> > &
Modem::retries()
{
    return d->retries;
}

const core::Property<std::string> &
Modem::subscriberIdentity()
{
    return d->subscriberIdentity;
}

const core::Property<std::string> &
Modem::operatorName()
{
    return d->operatorName;
}

const core::Property<org::ofono::Interface::NetworkRegistration::Status> &
Modem::status()
{
    return d->status;
}

const core::Property<std::int8_t> &
Modem::strength()
{
    return d->strength;
}

const core::Property<org::ofono::Interface::NetworkRegistration::Technology> &
Modem::technology()
{
    return d->technology;
}
