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

#ifndef MODEM_H
#define MODEM_H

#include <string>
#include <memory>

#include <core/property.h>

#include "dbus-cpp/services/ofono.h"

class Modem
{
    class Private;
    std::unique_ptr<Private> d;

public:
    enum class PinType
    {
        none,
        pin,
        puk
    };

    enum class SimStatus
    {
        offline,
        missing,
        error,
        locked,
        permanentlyLocked,
        ready
    };

    typedef std::shared_ptr<Modem> Ptr;
    typedef std::weak_ptr<Modem> WeakPtr;
    Modem() = delete;
    Modem(org::ofono::Interface::Modem::Ptr ofonoModem);
    virtual ~Modem();

    org::ofono::Interface::Modem::Ptr ofonoModem() const;

    void enterPin(PinType type,
                  const std::string &pin);

    void resetPin(PinType type,
                  const std::string &puk,
                  const std::string &pin);

    void changePin(PinType type,
                   const std::string &oldPin,
                   const std::string &newPin);


    const core::Property<SimStatus> &simStatus();
    const core::Property<PinType> &requiredPin();
    const core::Property<std::map<PinType, std::uint8_t>> &retries();
    const core::Property<std::string> &subscriberIdentity();

    const core::Property<std::string> &operatorName();
    const core::Property<org::ofono::Interface::NetworkRegistration::Status> &status();
    const core::Property<std::int8_t> &strength();
    const core::Property<org::ofono::Interface::NetworkRegistration::Technology> &technology();

};

#endif
