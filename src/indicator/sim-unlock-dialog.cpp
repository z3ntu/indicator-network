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
#include <sstream>

using namespace std;

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
         it = std::search(tmp.begin(), tmp.end(), subs.begin(), subs.end()))
    {
        auto pos = it;
        pos = tmp.erase(pos, pos+subs.length());
        tmp.insert(pos, val.begin(), val.end());
    }
    return tmp;
}

/// substitute variables in strings by index
/// only accepts std::string as input for now
/// argumentSubstitute("First: %{1}, Second %{2}", std::string{"foo"}, std::string{"bar"});
///     --> "First: foo, Second bar"
/// argumentSubstitute("First: %{2}, Second %{1}", std::string{"foo"}, std::string{"bar"});
///     --> "First: bar, Second: foor"
template<typename... Targs>
std::string argumentSubstitute(const std::string &format, Targs... Fargs)
{
    return __argumentSubstitute(1, format, Fargs...);
}
}
}

class SimUnlockDialog::Private: public QObject
{
    Q_OBJECT

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

    SimUnlockDialog& p;

    EnterPinStates m_enterPinState;

    State m_state = State::ready;
    Modem::Ptr m_modem;

    notify::snapdecision::SimUnlock::Ptr m_sd;

    QString m_newPin;
    QString m_oldPin;
    QString m_pukCode;

    std::recursive_mutex m_updateMutex;

    std::vector<QMetaObject::Connection> m_connections;

    bool m_showSimIdentifiers;

    /// @todo see comment in reset()
    bool m_doCleanUp;

    bool sendEnterPin(const QString& pin)
    {
        int retries = -1;
        map<Modem::PinType, uint8_t> retriesMap = m_modem->retries();
        if (retriesMap.find(Modem::PinType::pin) != retriesMap.end())
        {
            retries = retriesMap[Modem::PinType::pin];
        }

        // FIXME: API is async now
//        if (m_modem->enterPin(Modem::PinType::pin, pin))
//        {
//            return true;
//        }

        m_sd->showError(_("Sorry, incorrect PIN"));
        --retries;
        if (retries == 1) {
            showLastPinAttemptPopup();
        } else if (retries == 0) {
            showPinBlockedPopup();
        }

        return false;
    }

    bool sendResetPin(const QString& puk, const QString& newPin)
    {
        int retries = -1;
        map<Modem::PinType, uint8_t> retriesMap = m_modem->retries();
        if (retriesMap.find(Modem::PinType::puk) != retriesMap.end())
        {
            retries = retriesMap[Modem::PinType::puk];
        }

        // FIXME: This has changed to an async API
//        if (!m_modem->resetPin(Modem::PinType::puk, puk, newPin)) {
//            m_sd->showError(_("Sorry, incorrect PUK"));
//            --retries;
//            if (retries == 1) {
//                showLastPukAttemptPopup();
//            } else if (retries == 0) {
//                showSimPermanentlyBlockedPopup([this](){ m_sd->close(); });
//            }
//            return false;
//        }
        return true;
    }

    void showLastPinAttemptPopup(std::function<void()> closed = std::function<void()>())
    {
        std::stringstream output;
        output << ubuntu::i18n::argumentSubstitute(_("Sorry, incorrect %{1} PIN."),
                                                   m_showSimIdentifiers ?
                                                       m_modem->simIdentifier().toStdString()
                                                     : "SIM");
        output << " ";
        output << _("This will be your last attempt.");
        output << " ";
        output << ubuntu::i18n::argumentSubstitute(_("If %{1} PIN is entered incorrectly you will require your PUK code to unlock."),
                                                   m_showSimIdentifiers ?
                                                       m_modem->simIdentifier().toStdString()
                                                     : "SIM");
        m_sd->showPopup(output.str(), closed);
    }

    void showPinBlockedPopup(std::function<void()> closed = std::function<void()>())
    {
        std::stringstream output;
        output << ubuntu::i18n::argumentSubstitute(std::string{_("Sorry, your %{1} is now blocked.")},
                                                   m_showSimIdentifiers ?
                                                       m_modem->simIdentifier().toStdString()
                                                     : "SIM");
        output << " ";
        output << _("Please enter your PUK code to unblock SIM card.");
        output << " ";
        output << _("You may need to contact your network provider for PUK code.");

        m_sd->showPopup(output.str(), closed);
    }

    void showLastPukAttemptPopup(std::function<void()> closed = std::function<void()>())
    {
        std::stringstream output;
        output << _("Sorry, incorrect PUK.");
        output << " ";
        output << _("This will be your last attempt.");
        output << " ";
        output << _("If PUK code is entered incorrectly, your SIM card will be blocked and needs replacement.");
        output << " ";
        output << _("Please contact your network provider.");

        m_sd->showPopup(output.str(), closed);
    }

    void showSimPermanentlyBlockedPopup(std::function<void()> closed = std::function<void()>())
    {
        std::stringstream output;
        output << _("Sorry, incorrect PUK.");
        output << " ";
        output << _("Your SIM card is now permanently blocked and needs replacement.");
        output << " ";
        output << _("Please contact your service provider.");

        m_sd->showPopup(output.str(), closed);
    }

    Private(SimUnlockDialog& parent);

public Q_SLOTS:
    void update();

    void cancelled();
    void pinEntered(const QString& pin);
    void closed();

    void reset();
};

SimUnlockDialog::Private::Private(SimUnlockDialog& parent)
    : p(parent), m_doCleanUp{true}
{
    m_sd = std::make_shared<notify::snapdecision::SimUnlock>();
    m_sd->cancelled().connect(std::bind(&Private::cancelled, this));
    // FIXME Connect to signal
//    m_sd->pinEntered().connect(std::bind(&Private::pinEntered, this, std::placeholders::_1));
    m_sd->closed().connect(std::bind(&Private::closed, this));

    p.setShowSimIdentifiers(false);
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

    auto type = m_modem->requiredPin();
    map<Modem::PinType, uint8_t> retries = m_modem->retries();

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
            title = ubuntu::i18n::argumentSubstitute(_("Enter %{1} PIN"),
                                                     m_showSimIdentifiers ?
                                                         m_modem->simIdentifier().toStdString()
                                                       : "SIM");
            break;
        case Modem::PinType::puk:
            if (!m_showSimIdentifiers)
                title = _("Enter PUK code");
            else
                title = ubuntu::i18n::argumentSubstitute(_("Enter PUK code for %{1}"),
                                                           m_modem->simIdentifier().toStdString());
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
                                                           m_showSimIdentifiers ?
                                                               m_modem->simIdentifier().toStdString()
                                                             : "SIM"));
        m_sd->pinMinMax().set(lengths[Modem::PinType::pin]);
        break;
    case EnterPinStates::confirmNewPin:
        m_sd->body().set("Create new PIN");
        m_sd->title().set(ubuntu::i18n::argumentSubstitute(_("Confirm new %{1} PIN"),
                                                           m_showSimIdentifiers ?
                                                               m_modem->simIdentifier().toStdString()
                                                             : "SIM"));
        m_sd->pinMinMax().set(lengths[Modem::PinType::pin]);
        break;
    }

    /// @todo should be able to see cleartext puk and pin when entering puk or changing pin.
    m_sd->update();
    m_sd->show();
}

void
SimUnlockDialog::Private::pinEntered(const QString& pin)
{
    std::lock_guard<std::recursive_mutex> lock(m_updateMutex);
    switch (m_enterPinState) {
    case EnterPinStates::initial:
        // should never happen
        assert(0);
        return;
    case EnterPinStates::enterPin:
        if (sendEnterPin(pin)) {
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
            if (sendResetPin(m_pukCode, pin)) {
                m_sd->close();
                reset();
                return;
            }
            m_enterPinState = EnterPinStates::enterPuk;
            m_pukCode.clear();
            m_newPin.clear();
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
    /** @bug in properties-cpp :/
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

    m_state = State::ready;
    m_doCleanUp = false;
    Q_EMIT p.ready();
    m_doCleanUp = true;
}

SimUnlockDialog::SimUnlockDialog()
{
    d.reset(new Private(*this));
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
    d->m_state = State::unlocking;

    /// @todo delayed disconnections, see comment in reset()
    if (d->m_doCleanUp)
    {
        for (auto &c : d->m_connections)
        {
            disconnect(c);
        }
        d->m_connections.clear();
    }

    map<Modem::PinType, uint8_t> retries = modem->retries();
    auto type = modem->requiredPin();
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

    d->m_connections.emplace_back(connect(modem.get(), &Modem::requiredPinUpdated, d.get(), &Private::update));
    d->m_connections.emplace_back(connect(modem.get(), &Modem::retriesUpdated, d.get(), &Private::update));

    int pinRetries = -1;
    int pukRetries = -1;

    if (retries.find(Modem::PinType::pin) != retries.end())
    {
        pinRetries = retries[Modem::PinType::pin];
    }
    if (retries.find(Modem::PinType::puk) != retries.end())
    {
        pukRetries = retries[Modem::PinType::puk];
    }

    // remind the user
    if (type == Modem::PinType::pin && pinRetries == 1)
    {
        d->showLastPinAttemptPopup();
    }
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
SimUnlockDialog::cancel()
{
    d->m_sd->close();
}

Modem::Ptr
SimUnlockDialog::modem()
{
    return d->m_modem;
}

SimUnlockDialog::State
SimUnlockDialog::state() const
{
    return d->m_state;
}

bool
SimUnlockDialog::showSimIdentifiers() const
{
    return d->m_showSimIdentifiers;
}

void SimUnlockDialog::setShowSimIdentifiers(bool showSimIdentifiers)
{
    if (d->m_showSimIdentifiers == showSimIdentifiers)
    {
        return;
    }

    d->m_showSimIdentifiers = showSimIdentifiers;
    Q_EMIT showSimIdentifiersUpdated(d->m_showSimIdentifiers);
}

#include "sim-unlock-dialog.moc"
