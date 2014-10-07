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

#ifndef SIM_UNLOCK_DIALOG
#define SIM_UNLOCK_DIALOG

#include <memory>
#include "modem.h"

class SimUnlockDialog
{
    class Private;
    std::unique_ptr<Private> d;

public:
    enum class State {
        ready,
        unlocking,
        changingPin
    };

    typedef std::shared_ptr<SimUnlockDialog> Ptr;
    SimUnlockDialog();
    ~SimUnlockDialog();

    void unlock(Modem::Ptr modem);

    /**
     * there must be no other operation active on the dialog
     * modem has to be on ready state, i.e. it has to be unlocked before
     * changing the pin.
     *
     * @param modem
     */
    void changePin(Modem::Ptr modem);

    void cancel();

    Modem::Ptr modem();

    core::Property<State> &state();
    core::Property<bool> &showSimIdentifiers();
};

#endif
