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

    core::Property<bool> isLocked;
    core::Property<bool> simPresent;
    core::Property<Modem::PinType> requiredPin;
    core::Property<std::map<Modem::PinType, std::uint8_t>> retries;
    core::Property<std::string> subscriberIdentity;

    core::Property<std::string> operatorName;
    core::Property<org::ofono::Interface::NetworkRegistration::Status> status;
    core::Property<std::int8_t> strength;
    core::Property<org::ofono::Interface::NetworkRegistration::Technology> technology;
};


Modem::Modem(org::ofono::Interface::Modem::Ptr ofonoModem)
{
    d.reset(new Private);
    d->ofonoModem = ofonoModem;

    auto simManagerChanged = [this](org::ofono::Interface::SimManager::Ptr simmgr){
        if (simmgr) {
            d->simPresent.set(simmgr->present.get());
            simmgr->present.changed().connect([this](bool value){
                d->simPresent.set(value);
            });

            auto pinRequiredChanged = [this](org::ofono::Interface::SimManager::PinType pin){
                switch(pin)
                {
                case org::ofono::Interface::SimManager::PinType::none:
                    d->requiredPin.set(PinType::none);
                    d->isLocked.set(false);
                    break;
                case org::ofono::Interface::SimManager::PinType::pin:
                    d->requiredPin.set(PinType::pin);
                    d->isLocked.set(true);
                    break;
                case org::ofono::Interface::SimManager::PinType::puk:
                    d->requiredPin.set(PinType::puk);
                    d->isLocked.set(true);
                    break;
                default:
                    throw std::runtime_error("Ofono requires a PIN we have not been prepared to handle (" +
                                             org::ofono::Interface::SimManager::pin2str(pin) +
                                             "). Bailing out.");
                }
            };
            pinRequiredChanged(simmgr->pinRequired.get());
            simmgr->pinRequired.changed().connect([pinRequiredChanged](org::ofono::Interface::SimManager::PinType pin){
                pinRequiredChanged(pin);
            });

            /// @todo retries
            /// @todo subscriberIdentity

        } else {
            d->requiredPin.set(PinType::none);
            d->simPresent.set(false);
            d->isLocked.set(false);
        }
    };
    simManagerChanged(d->ofonoModem->simManager.get());
    d->ofonoModem->simManager.changed().connect([simManagerChanged](org::ofono::Interface::SimManager::Ptr value){
        simManagerChanged(value);
    });

    auto networkRegistrationChanged = [this](org::ofono::Interface::NetworkRegistration::Ptr netreg){
        if (netreg) {
            d->operatorName.set(netreg->operatorName.get());
            netreg->operatorName.changed().connect([this](std::string value){
                d->operatorName.set(value);
            });

            d->status.set(netreg->status.get());
            netreg->status.changed().connect([this](org::ofono::Interface::NetworkRegistration::Status value){
                d->status.set(value);
            });

            d->strength.set(netreg->strength.get());
            netreg->strength.changed().connect([this](std::int8_t value){
                d->strength.set(value);
            });

            d->technology.set(netreg->technology.get());
            netreg->technology.changed().connect([this](org::ofono::Interface::NetworkRegistration::Technology value){
                d->technology.set(value);
            });

        } else {
            d->operatorName.set("");
            d->status.set(org::ofono::Interface::NetworkRegistration::Status::unknown);
            d->strength.set(-1);
            d->technology.set(org::ofono::Interface::NetworkRegistration::Technology::notAvailable);
        }

    };
    networkRegistrationChanged(d->ofonoModem->networkRegistration.get());
    d->ofonoModem->networkRegistration.changed().connect([networkRegistrationChanged](org::ofono::Interface::NetworkRegistration::Ptr value){
        networkRegistrationChanged(value);
    });
}

org::ofono::Interface::Modem::Ptr
Modem::ofonoModem() const
{
    return d->ofonoModem;
}

Modem::~Modem()
{

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

const core::Property<bool> &
Modem::isLocked()
{
    return d->isLocked;
}

const core::Property<bool> &
Modem::simPresent()
{
    return d->simPresent;
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
