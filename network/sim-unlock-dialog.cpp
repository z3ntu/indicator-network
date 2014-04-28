
#include "sim-unlock-dialog.h"

#include <notify-cpp/notification.h>
#include <notify-cpp/snapdecision/sim-unlock.h>

#include <functional>

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
        m_sd->showPopup(_("This will be the last attempt.\n"
                          "\n"
                          "If the SIM PIN is entered incorrectly, your SIM "
                          "will be blocked and would require the PUK Code to unlock."),
                        closed);
    }

    void showPinBlockedPopup(std::function<void()> closed = std::function<void()>())
    {
        m_sd->showPopup(_("Your SIM is now blocked.\n"
                          "\n"
                          "Enter the PUK Code to unlock.\n"
                          "\n"
                          "You may contact your network provider for PUK Code."),
                        closed);
    }

    void showLastPukAttemptPopup(std::function<void()> closed = std::function<void()>())
    {
        m_sd->showPopup(_("This will be the last attempt.\n"
                          "\n"
                          "If the PUK code entered incorrectly, your SIM will need to be replaced.\n"
                          "\n"
                          "\nPlease contact your network provider."),
                        closed);
    }

    void showSimPermanentlyBlockedPopup(std::function<void()> closed = std::function<void()>())
    {
        m_sd->showPopup(_("Your SIM is now permanently blocked and needs to be replaced.\n"
                          "\n"
                          "Please contact your network provider."),
                        closed);
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
    m_sd->cancelled().connect(std::bind(&Private::cancelled, this));
    m_sd->pinEntered().connect(std::bind(&Private::pinEntered, this, std::placeholders::_1));
    m_sd->closed().connect(std::bind(&Private::closed, this));

    reset();
}

void
SimUnlockDialog::Private::update()
{
    std::lock_guard<std::recursive_mutex> lock(m_updateMutex);

    std::cout << __PRETTY_FUNCTION__ << std::endl;
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
            /// @todo add SIM identifier
            title = _("Enter SIM PIN");
            break;
        case Modem::PinType::puk:
            /// @todo add SIM identifier
            title = _("Enter PUK code");
            break;
        }
        m_sd->pinMinMax().set(lengths[type]);

        std::string attempts;
        if (retries.find(type) != retries.end()) {
            auto attempt = maxRetries[type] - retries[type] + 1;
            gchar *tmp = g_strdup_printf(_("Attempt %d of %d"), attempt, maxRetries[type]);
            attempts = {tmp};
            g_free(tmp);
        }

        /// @todo get a proper unlock dialog API..
        if (attempts.empty()) {
            m_sd->title().set(title);
        } else {
            m_sd->title().set("<b>" + title + "</b><br>" + attempts);
        }
        std::cout << m_sd->title().get() << std::endl;
        break;
    }
    case EnterPinStates::enterNewPin:
        m_sd->title().set(_("Enter new PIN code"));
        break;
    case EnterPinStates::confirmNewPin:
        m_sd->title().set(_("Confirm new PIN code"));
        break;
    }

    /// @todo should be able to see cleartext puk and pin when entering puk or changing pin.
    /// @todo add phone number or IMSI or something to the body
    m_sd->update();
    m_sd->show();
}

void
SimUnlockDialog::Private::pinEntered(std::string pin)
{
    std::lock_guard<std::recursive_mutex> lock(m_updateMutex);
    std::cout << __PRETTY_FUNCTION__ << std::endl;
    switch (m_enterPinState) {
    case EnterPinStates::initial:
        // should never happen
        assert(0);
        return;
    case EnterPinStates::enterPin:
        if (!m_modem->enterPin(Modem::PinType::pin, pin)) {
            m_sd->showError(_("Oops!<br>Incorrect PIN entered."));
        } else {
            m_sd->close();
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
                m_sd->showError(_("Oops!<br>Incorrect PUK entered."));
                m_enterPinState = EnterPinStates::enterPuk;
                m_pukCode.clear();
                m_newPin.clear();
            } else {
                m_sd->close();
            }
        }
        break;
    }
    update();
}

void
SimUnlockDialog::Private::cancelled()
{
    std::cout << "SIM notification cancelled" << std::endl;
    m_sd->close();
}

void
SimUnlockDialog::Private::closed()
{
    std::cout << __PRETTY_FUNCTION__ << std::endl;
    reset();
}

void
SimUnlockDialog::Private::reset()
{
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

    /// @todo disconnect!
    modem->requiredPin().changed().connect(std::bind(&Private::update, d.get()));
    modem->retries().changed().connect(std::bind(&Private::update, d.get()));

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
    if (d->m_modem)
        throw std::logic_error("Unlocking already in progress.");

    if (modem->simStatus().get() != Modem::SimStatus::ready)
        throw std::logic_error("SIM is not ready");

    d->m_modem = modem;
    d->m_state.set(State::changingPin);

    /// @todo add SIM identifier
    d->m_sd->title().set(_("Enter SIM PIN"));
    d->m_sd->body().set("");
    d->m_sd->pinMinMax().set({4,8});
    d->m_sd->show();

    std::string currentPin;
    std::string newPin;

    /// @todo disconnect.
    d->m_sd->pinEntered().connect([this, &currentPin, &newPin](std::string pin){
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
    /// @todo SimUnlock::cancelled() and SimUnlock::closed()
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

