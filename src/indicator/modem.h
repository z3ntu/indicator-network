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

#include <map>
#include <memory>
#include <string>

#include <QString>
#include <core/property.h>

class QOfonoModem;

/**
 * all signals and property changes emitted from GMainLoop
 */
class Modem
{
    class Private;
    std::shared_ptr<Private> d;

public:
    enum class PinType
    {
        none,
        pin,
        puk
    };

    enum class SimStatus
    {
        missing,
        error,
        locked,
        permanentlyLocked,
        ready,
        not_available
    };

    enum class Status
    {
        unregistered,
        registered,
        searching,
        denied,
        unknown,
        roaming
    };

    enum class Technology
    {
        notAvailable,
        gsm,
        edge,
        umts,
        hspa,
        lte
    };

    typedef std::shared_ptr<Modem> Ptr;
    typedef std::weak_ptr<Modem> WeakPtr;

    struct Compare
    {
        bool operator()(int lhs, int rhs)
        {
            if (lhs == -1 && rhs == -1)
                return false;
            if (lhs == -1)
                return false;
            if (rhs == -1)
                return true;
            return lhs < rhs;
        }
    };

    Modem() = delete;
    explicit Modem(std::shared_ptr<QOfonoModem> ofonoModem);
    virtual ~Modem();

//    org::ofono::Interface::Modem::Ptr ofonoModem() const;

    void enterPin(PinType type,
                  const QString &pin);

    void resetPin(PinType type,
                  const QString &puk,
                  const QString &pin);

    void changePin(PinType type,
                   const QString &oldPin,
                   const QString &newPin);

    const core::Property<bool> &online();

    const core::Property<SimStatus> &simStatus();
    const core::Property<PinType> &requiredPin();
    const core::Property<std::map<PinType, std::uint8_t>> &retries();

    const core::Property<QString> &operatorName();
    const core::Property<Status> &status();
    const core::Property<std::int8_t> &strength();
    const core::Property<Technology> &technology();

    const core::Property<bool> &dataEnabled();

    const core::Property<QString> &simIdentifier();
    int index();

    QString name() const;

    static std::string strengthIcon(int8_t strength);

    static std::string technologyIcon(Technology tech);
};

#endif
