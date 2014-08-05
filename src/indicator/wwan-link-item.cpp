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

#include "wwan-link-item.h"

#include "menuitems/text-item.h"

#include "menumodel-cpp/menu.h"

class WwanLinkItem::Private
{
public:

    ActionGroupMerger::Ptr m_actionGroupMerger;
    Menu::Ptr m_menu;

    Modem::Ptr m_modem;
    ModemManager::Ptr m_modemManager;

    TextItem::Ptr m_unlockSim;
    bool m_wasLocked;

    Private() = delete;
    Private(Modem::Ptr modem, ModemManager::Ptr modemManager);

    void simStatusChanged(Modem::SimStatus status);
    void unlockSim();
};

WwanLinkItem::Private::Private(Modem::Ptr modem, ModemManager::Ptr modemManager)
    : m_modem{modem},
      m_modemManager{modemManager}
{
    m_actionGroupMerger = std::make_shared<ActionGroupMerger>();
    m_menu = std::make_shared<Menu>();

    /// @todo add stuff to control the link

    m_unlockSim = std::make_shared<TextItem>(_("Unlock SIM…"), "sim", "unlock");
    m_unlockSim->activated().connect(std::bind(&ModemManager::unlockModem, m_modemManager.get(), m_modem));

    m_actionGroupMerger->add(*m_unlockSim);

    m_wasLocked = false;
    m_modem->simStatus().changed().connect(std::bind(&Private::simStatusChanged, this, std::placeholders::_1));
    simStatusChanged(m_modem->simStatus().get());
}

void
WwanLinkItem::Private::simStatusChanged(Modem::SimStatus status)
{
    if (status == Modem::SimStatus::locked) {
        if (!m_wasLocked) {
            //m_actionGroupMerger->add(*m_unlockSim);
            m_menu->append(*m_unlockSim);
            m_wasLocked = true;
        }
    } else {
        if (m_wasLocked) {
            //m_actionGroupMerger->remove(*m_unlockSim);
            m_menu->remove(m_menu->find(*m_unlockSim));
            m_wasLocked = false;
        }
    }
}

WwanLinkItem::WwanLinkItem(Modem::Ptr modem, ModemManager::Ptr modemManager)
{
    d.reset(new Private(modem, modemManager));
}

WwanLinkItem::~WwanLinkItem()
{}

ActionGroup::Ptr
WwanLinkItem::actionGroup()
{
    return *d->m_actionGroupMerger;
}

MenuModel::Ptr
WwanLinkItem::menuModel()
{
    return d->m_menu;
}
