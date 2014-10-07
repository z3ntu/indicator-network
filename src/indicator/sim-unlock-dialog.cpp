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

#include "sim-unlock-dialog.h"

#include <notify-cpp/notification.h>
#include <notify-cpp/snapdecision/sim-unlock.h>

#include <functional>

namespace ubuntu {
namespace i18n{

std::string __argumentSubstitute(int /*depth*/, const std::string &format) // base function
{
    return format;
}

template<typename T, typename... Targs>
std::string __argumentSubstitute(int depth, const std::string &format, T value, Targs... Fargs)
{
    std::string tmp = __argumentSubstitute(depth+1, format, Fargs...);

    std::string subs = "%{" + std::to_string(depth) + "}";

    const std::string val {value};
    for (auto it = std::search(tmp.begin(), tmp.end(), subs.begin(), subs.end());
         it != tmp.end();
         it = std::search(it, tmp.end(), subs.begin(), subs.end()))
    {
        auto pos = it;
        pos = tmp.erase(pos, pos+subs.length());
        tmp.insert(pos, val.begin(), val.end());
    }
    return tmp;
}

/// substitute variables in strings by index
/// only accepts std::string as input for now
/// argumentSubstitute("First: %{1}, Second %{2}", std::string{"foo"}, std::string{"bar});
///     --> "First: foo, Second bar"
/// argumentSubstitute("First: %{2}, Second %{1}", std::string{"foo"}, std::string{"bar});
///     --> "First: bar, Second: foor"
template<typename... Targs>
std::string argumentSubstitute(const std::string &format, Targs... Fargs)
{
    return __argumentSubstitute(1, format, Fargs...);
}
}
}

class SimUnlockDialog::Private
{
public:
    enum class Mode
    {
        enterPin,
        resetPin,
        lockPin,
        unlockPin
    };

    enum class EnterPinStates
    {
        initial,
        enterPin,
        enterPuk,
        enterNewPin,
        confirmNewPin
    };
    EnterPinStates m_enterPinState;

    core::Property<State> m_state;
    Modem::Ptr m_modem;

    notify::snapdecision::SimUnlock::Ptr m_sd;

    std::string m_newPin;
    std::string m_oldPin;
    std::string m_pukCode;

    std::recursive_mutex m_updateMutex;

    std::vector<core::Connection> m_connections;

    core::Property<bool> m_showSimIdentifiers;

    void sendEnterPin(std::string pin)
    {
        int retries = -1;
        auto retriesMap = m_modem->retries().get();
        if (retriesMap.find(Modem::PinType::pin) != retriesMap.end())
        {
            retries = retriesMap[Modem::PinType::pin];
        }

        if (!m_modem->enterPin(Modem::PinType::pin, pin)) {
            --retries;
            if (retries == 1) {
                showLastPinAttemptPopup();
            } else if (retries == 0) {
                showPinBlockedPopup();
            }
        }
    }

    void sendResetPin(std::string puk, std::string newPin)
    {
        int retries = -1;
        auto retriesMap = m_modem->retries().get();
        if (retriesMap.find(Modem::PinType::puk) != retriesMap.end())
        {
            retries = retriesMap[Modem::PinType::puk];
        }

        if (!m_modem->resetPin(Modem::PinType::pin, puk, newPin)) {
            --retries;
            if (retries == 1) {
                showLastPinAttemptPopup();
            } else if (retries == 0) {
                showPinBlockedPopup([this](){ m_sd->close(); });
            }
        }
    }

    void showLastPinAttemptPopup(std::function<void()> closed = std::function<void()>())
    {
        std::stringstream output;
        output << ubuntu::i18n::argumentSubstitute(_("Sorry, incorrect %{1} PIN."),
                                                   m_showSimIdentifiers.get() ?
                                                       m_modem->simIdentifier().get()
                                                     : "SIM");
        output << _("This will be your last attempt.");
        output << ubuntu::i18n::argumentSubstitute(_("If %{1} PIN is entered incorrectly you will require your PUK code to unlock."),
                                                   m_showSimIdentifiers.get() ?
                                                       m_modem->simIdentifier().get()
                                                     : "SIM");
        m_sd->showPopup(output.str(), closed);
    }

    void showPinBlockedPopup(std::function<void()> closed = std::function<void()>())
    {
        std::stringstream output;
        output << ubuntu::i18n::argumentSubstitute(std::string{_("Sorry, your %{1} is now blocked.")},
                                                   m_showSimIdentifiers.get() ?
                                                       m_modem->simIdentifier().get()
                                                     : "SIM");
        output << _("Please enter your PUK code to unblock SIM card.");
        output << _("You may need to contact your network provider for PUK code.");

        m_sd->showPopup(output.str(), closed);
    }

    void showLastPukAttemptPopup(std::function<void()> closed = std::function<void()>())
    {
        std::stringstream output;
        output << _("Sorry, incorrect PUK.");
        output << _("This will be your last attempt.");
        output << _("If PUK code is entered incorrectly, your SIM card will be blocked and needs replacement.");
        output << _("Please contact your network provider.");

        m_sd->showPopup(output.str(), closed);
    }

    void showSimPermanentlyBlockedPopup(std::function<void()> closed = std::function<void()>())
    {
        std::stringstream output;
        output << _("Sorry, incorrect PUK.");
        output << _("Your SIM card is now permanently blocked and needs replacement.");
        output << _("Please contact your service provider.");

        m_sd->showPopup(output.str(), closed);
    }

    Private();

    void update();

    void cancelled();
    void pinEntered(std::string pin);
    void closed();

    void reset();
    void sendFailNotification(const std::string &title);
};

SimUnlockDialog::Private::Private()
{
    m_sd = std::make_shared<notify::snapdecision::SimUnlock>();
    m_showSimIdentifiers.set(false);
    reset();
}

void
SimUnlockDialog::Private::update()
{
    std::lock_guard<std::recursive_mutex> lock(m_updateMutex);

    if (!m_modem || !m_sd)
        return;

    m_sd->title().set("");
    m_sd->body().set("");
    m_sd->pinMinMax().set({0, 0});

    std::map<Modem::PinType, std::pair<std::uint8_t, std::uint8_t>> lengths;
    lengths = {{Modem::PinType::pin, {4, 8}},
               {Modem::PinType::puk, {8, 8}}};
    std::map<Modem::PinType, std::uint8_t> maxRetries;
    maxRetries = {{Modem::PinType::pin, 3},
                  {Modem::PinType::puk, 10}};

    auto type = m_modem->requiredPin().get();
    auto retries = m_modem->retries().get();

    if (m_enterPinState == EnterPinStates::enterPin &&
        type == Modem::PinType::puk) {
        // we transitioned from pin query to PUK query
        m_enterPinState = EnterPinStates::enterPuk;
    }

    switch (m_enterPinState) {
    case EnterPinStates::initial:
        return;
    case EnterPinStates::enterPin:
    case EnterPinStates::enterPuk:
    {
        std::string title;
        switch(type){
        case Modem::PinType::none:
            // we are done.
            return;
        case Modem::PinType::pin:
            title = ubuntu::i18n::argumentSubstitute(_("Enter ${1} PIN"),
                                                     m_showSimIdentifiers.get() ?
                                                         m_modem->simIdentifier().get()
                                                       : "SIM");
            break;
        case Modem::PinType::puk:
            if (!m_showSimIdentifiers.get())
                title = _("Enter PUK code");
            else
                title = ubuntu::i18n::argumentSubstitute(_("Enter PUK code for %{1}"),
                                                           m_modem->simIdentifier().get());
            break;
        }
        m_sd->pinMinMax().set(lengths[type]);

        std::string attempts;
        if (retries.find(type) != retries.end()) {
            gchar *tmp = g_strdup_printf(ngettext("1 attempt remaining", "%d attempts remaining", retries[type]), retries[type]);
            attempts = {tmp};
            g_free(tmp);
        }

        m_sd->title().set(title);
        m_sd->body().set(attempts);
        break;
    }
    case EnterPinStates::enterNewPin:
        m_sd->body().set("Create new PIN");
        m_sd->title().set(ubuntu::i18n::argumentSubstitute(_("Enter new %{1} PIN"),
                                                           m_showSimIdentifiers.get() ?
                                                               m_modem->simIdentifier().get()
                                                             : "SIM"));
        m_sd->pinMinMax().set(lengths[Modem::PinType::pin]);
        break;
    case EnterPinStates::confirmNewPin:
        m_sd->body().set("Create new PIN");
        m_sd->title().set(ubuntu::i18n::argumentSubstitute(_("Confirm new %{1} PIN"),
                                                           m_showSimIdentifiers.get() ?
                                                               m_modem->simIdentifier().get()
                                                             : "SIM"));
        m_sd->pinMinMax().set(lengths[Modem::PinType::pin]);
        break;
    }

    /// @todo should be able to see cleartext puk and pin when entering puk or changing pin.
    m_sd->update();
    m_sd->show();
}

void
SimUnlockDialog::Private::pinEntered(std::string pin)
{
    std::lock_guard<std::recursive_mutex> lock(m_updateMutex);
    switch (m_enterPinState) {
    case EnterPinStates::initial:
        // should never happen
        assert(0);
        return;
    case EnterPinStates::enterPin:
        if (!m_modem->enterPin(Modem::PinType::pin, pin)) {
            m_sd->showError(_("Sorry, incorrect PIN"));
        } else {
            m_sd->close();
            reset();
            return;
        }
        break;
    case EnterPinStates::enterPuk:
        m_pukCode = pin;
        m_enterPinState = EnterPinStates::enterNewPin;
        break;
    case EnterPinStates::enterNewPin:
        m_newPin = pin;
        m_enterPinState = EnterPinStates::confirmNewPin;
        break;
    case EnterPinStates::confirmNewPin:
        if (m_newPin != pin) {
            m_sd->showError(_("PIN codes did not match."));
            m_enterPinState = EnterPinStates::enterNewPin;
            m_newPin.clear();
        } else {
            if (!m_modem->resetPin(Modem::PinType::pin, m_pukCode, pin)) {
                m_sd->showError(_("Sorry, incorrect PUK"));
                m_enterPinState = EnterPinStates::enterPuk;
                m_pukCode.clear();
                m_newPin.clear();
            } else {
                m_sd->close();
                reset();
                return;
            }
        }
        break;
    }
    update();
}

void
SimUnlockDialog::Private::cancelled()
{
    m_sd->close();
}

void
SimUnlockDialog::Private::closed()
{
    reset();
}

void
SimUnlockDialog::Private::reset()
{
    /** @todo
     * bug in dbus-cpp :/
     * can't disconnect here as the reset() is called from pinEnterer() and
     * we can't disconnect a signal inside one of it's handlers right now.
     * uncomment this code once dbus-cpp is fixed.
     * for (auto &c : m_connections)
     *   c.disconnect();
     * m_connections.clear();
     * m_sd.reset();
     */
    m_modem.reset();

    m_newPin.clear();
    m_oldPin.clear();
    m_pukCode.clear();

    m_enterPinState = EnterPinStates::initial;

    m_state.set(State::ready);
}

SimUnlockDialog::SimUnlockDialog()
{
    d.reset(new Private);
}

SimUnlockDialog::~SimUnlockDialog()
{}

/// must not be called is unlocking already in progress
void
SimUnlockDialog::unlock(Modem::Ptr modem)
{
    if (d->m_modem)
        throw std::logic_error("Unlocking already in progress.");

    d->m_modem = modem;
    d->m_state.set(State::unlocking);

    /// @todo delayed disconnections, see comment in reset()
    for (auto &c : d->m_connections)
        c.disconnect();
    d->m_connections.clear();

    auto retries = modem->retries().get();
    auto type = modem->requiredPin().get();
    switch(type){
    case Modem::PinType::none:
        d->reset();
        return;
    case Modem::PinType::pin:
        d->m_enterPinState = Private::EnterPinStates::enterPin;
        break;
    case Modem::PinType::puk:
        d->m_enterPinState = Private::EnterPinStates::enterPuk;
        break;
    }

    auto c = modem->requiredPin().changed().connect(std::bind(&Private::update, d.get()));
    d->m_connections.push_back(c);

    c = modem->retries().changed().connect(std::bind(&Private::update, d.get()));
    d->m_connections.push_back(c);

    c = d->m_sd->cancelled().connect(std::bind(&Private::cancelled, d.get()));
    d->m_connections.push_back(c);

    c = d->m_sd->pinEntered().connect(std::bind(&Private::pinEntered, d.get(), std::placeholders::_1));
    d->m_connections.push_back(c);

    c = d->m_sd->closed().connect(std::bind(&Private::closed, d.get()));
    d->m_connections.push_back(c);

    int pinRetries = -1;
    int pukRetries = -1;

    if (retries.find(Modem::PinType::pin) != retries.end())
        pinRetries = retries[Modem::PinType::pin];
    if (retries.find(Modem::PinType::puk) != retries.end())
        pukRetries = retries[Modem::PinType::puk];

    // remind the user
    if (type == Modem::PinType::pin && pinRetries == 1)
        d->showLastPinAttemptPopup();
    else if (type == Modem::PinType::puk) {
        // we we know the sim is permanently blocked, just show the notification straight away
        if (pukRetries == 0)
            d->showSimPermanentlyBlockedPopup([this](){ cancel(); });
        else
            d->showPinBlockedPopup([this, pukRetries](){
                if (pukRetries == 1) {
                    d->showLastPukAttemptPopup();
                }
            });
    }

    d->update();
}

void
SimUnlockDialog::changePin(Modem::Ptr modem)
{
    /// @todo do we need this?
    if (d->m_modem)
        throw std::logic_error("Unlocking already in progress.");

    if (modem->simStatus().get() != Modem::SimStatus::ready)
        throw std::logic_error("SIM is not ready");

    d->m_modem = modem;
    d->m_state.set(State::changingPin);

    if (d->m_modem)
        throw std::logic_error("Unlocking already in progress.");

    auto c = d->m_sd->cancelled().connect(std::bind(&Private::cancelled, d.get()));
    d->m_connections.push_back(c);

    c = d->m_sd->closed().connect(std::bind(&Private::closed, d.get()));
    d->m_connections.push_back(c);

    /// @todo add SIM identifier
    d->m_sd->title().set(_("Enter SIM PIN"));
    d->m_sd->body().set("");
    d->m_sd->pinMinMax().set({4,8});
    d->m_sd->show();

    std::string currentPin;
    std::string newPin;

    c = d->m_sd->pinEntered().connect([this, &currentPin, &newPin](std::string pin){
        if (currentPin.empty()) {
            currentPin = pin;
            d->m_sd->title().set(_("Enter new PIN code"));
            /// @todo make sure the dialog can't be cancelled from this point onward
        } else if (newPin.empty()) {
            newPin = pin;
            d->m_sd->title().set(_("Confirm new PIN code"));
        } else {
            // we have the current PIN and the newPin and confirmed pin is provided as argument pin
            if (newPin != pin) {
                newPin.clear();
                d->m_sd->showError(_("PIN codes did not match."));
                d->m_sd->title().set(_("Enter new PIN code"));
            } else {
                if (d->m_modem->changePin(Modem::PinType::pin, currentPin, newPin)) {
                    currentPin.clear();
                    newPin.clear();
                    /// @todo make sure the dialog can be cancelled again
                    d->m_sd->showError(_("Failed to change PIN.")); /// @todo instruct that the current pin was most probably wrong.
                    d->m_sd->title().set(_("Enter SIM PIN"));
                } else {
                    d->m_sd->close();
                }
            }
        }
    });
    d->m_connections.push_back(c);
}

void
SimUnlockDialog::cancel()
{
    d->m_sd->close();
}

Modem::Ptr
SimUnlockDialog::modem()
{
    return d->m_modem;
}

core::Property<SimUnlockDialog::State> &
SimUnlockDialog::state()
{
    return d->m_state;
}

core::Property<bool> &
SimUnlockDialog::showSimIdentifiers()
{
    return d->m_showSimIdentifiers;
}
