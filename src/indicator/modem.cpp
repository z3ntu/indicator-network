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
    core::Property<bool> m_online;

    org::ofono::Interface::Modem::Ptr m_ofonoModem;
    core::Property<Modem::SimStatus> m_simStatus;
    core::Property<Modem::PinType> m_requiredPin;
    core::Property<std::map<Modem::PinType, std::uint8_t>> m_retries;

    core::Property<std::string> m_operatorName;
    core::Property<org::ofono::Interface::NetworkRegistration::Status> m_status;
    core::Property<std::int8_t> m_strength;
    core::Property<org::ofono::Interface::NetworkRegistration::Technology> m_technology;

    core::Property<std::string> m_simIdentifier;
    int m_index = -1;

    void networkRegistrationChanged(org::ofono::Interface::NetworkRegistration::Ptr netreg);
    void simManagerChanged(org::ofono::Interface::SimManager::Ptr simmgr);

    void update();
};

void
Modem::Private::update()
{
    auto simmgr = m_ofonoModem->simManager.get();
    if (simmgr) {
        // update requiredPin
        switch(simmgr->pinRequired.get())
        {
        case org::ofono::Interface::SimManager::PinType::none:
            m_requiredPin.set(PinType::none);
            break;
        case org::ofono::Interface::SimManager::PinType::pin:
            m_requiredPin.set(PinType::pin);
            break;
        case org::ofono::Interface::SimManager::PinType::puk:
            m_requiredPin.set(PinType::puk);
            break;
        default:
            throw std::runtime_error("Ofono requires a PIN we have not been prepared to handle (" +
                                     org::ofono::Interface::SimManager::pin2str(simmgr->pinRequired.get()) +
                                     "). Bailing out.");
        }

        // update retries
        std::map<Modem::PinType, std::uint8_t> tmp;
        for (auto element : simmgr->retries.get()) {
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

        // update simStatus
        if (!simmgr->present.get()) {
            m_simStatus.set(SimStatus::missing);
        } else if (m_requiredPin == PinType::none){
            m_simStatus.set(SimStatus::ready);
        } else {
            if (m_retries->count(PinType::puk) != 0 && m_retries->at(PinType::puk) == 0)
                m_simStatus.set(SimStatus::permanentlyLocked);
            else
                m_simStatus.set(SimStatus::locked);
        }

    } else {
        m_requiredPin.set(PinType::none);
        m_retries.set({});
        m_simStatus.set(SimStatus::missing);
    }

    auto netreg = m_ofonoModem->networkRegistration.get();
    if (netreg) {
        m_operatorName.set(netreg->operatorName.get());
        m_status.set(netreg->status.get());
        m_strength.set(netreg->strength.get());
        m_technology.set(netreg->technology.get());
    } else {
        m_operatorName.set("");
        m_status.set(org::ofono::Interface::NetworkRegistration::Status::unknown);
        m_strength.set(-1);
        m_technology.set(org::ofono::Interface::NetworkRegistration::Technology::notAvailable);
    }
}

void
Modem::Private::networkRegistrationChanged(org::ofono::Interface::NetworkRegistration::Ptr netreg)
{
    if (netreg) {
        netreg->operatorName.changed().connect(std::bind(&Private::update, this));
        netreg->status.changed().connect(std::bind(&Private::update, this));
        netreg->strength.changed().connect(std::bind(&Private::update, this));
        netreg->technology.changed().connect(std::bind(&Private::update, this));
    }
    update();
}


void
Modem::Private::simManagerChanged(org::ofono::Interface::SimManager::Ptr simmgr)
{
    if (simmgr) {
        simmgr->present.changed().connect(std::bind(&Private::update, this));
        simmgr->pinRequired.changed().connect(std::bind(&Private::update, this));
        simmgr->retries.changed().connect(std::bind(&Private::update, this));
    }
    update();
}

Modem::Modem(org::ofono::Interface::Modem::Ptr ofonoModem)
{
    d.reset(new Private);
    d->m_ofonoModem = ofonoModem;

    d->m_online.set(d->m_ofonoModem->online.get());
    d->m_ofonoModem->online.changed().connect([this](bool value){
        d->m_online.set(value);
    });

    d->simManagerChanged(d->m_ofonoModem->simManager.get());
    d->m_ofonoModem->simManager.changed().connect(std::bind(&Private::simManagerChanged, d.get(), std::placeholders::_1));

    d->networkRegistrationChanged(d->m_ofonoModem->networkRegistration.get());
    d->m_ofonoModem->networkRegistration.changed().connect(std::bind(&Private::networkRegistrationChanged, d.get(), std::placeholders::_1));

    /// @todo hook up with system-settings to allow changing the identifier.
    ///       for now just provide the defaults
    const auto path = ofonoModem->object->path().as_string();
    if (path == "/ril_0") {
        d->m_simIdentifier.set("SIM 1");
        d->m_index = 1;
    } else if (path == "/ril_1") {
        d->m_simIdentifier.set("SIM 2");
        d->m_index = 2;
    } else {
        d->m_simIdentifier.set(path);
    }
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
    case PinType::puk:
        return d->m_ofonoModem->simManager.get()->resetPin(org::ofono::Interface::SimManager::PinType::puk,
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
const core::Property<std::string> &
Modem::simIdentifier()
{
    return d->m_simIdentifier;
}

int
Modem::index()
{
    return d->m_index;
}

