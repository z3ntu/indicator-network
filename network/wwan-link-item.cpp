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

#include "menuitems/modem-info-item.h"

#include "menumodel-cpp/menu.h"

class WwanLinkItem::Private
{
public:

    ActionGroupMerger::Ptr m_actionGroupMerger;
    Menu::Ptr m_menu;

    Modem::Ptr m_modem;
    ModemManager::Ptr m_modemManager;

    ModemInfoItem::Ptr m_infoItem;

    Private() = delete;
    Private(Modem::Ptr modem, ModemManager::Ptr modemManager);

    void update();
    void unlockSim();
};

WwanLinkItem::Private::Private(Modem::Ptr modem, ModemManager::Ptr modemManager)
    : m_modem{modem},
      m_modemManager{modemManager}
{
    m_actionGroupMerger = std::make_shared<ActionGroupMerger>();
    m_menu = std::make_shared<Menu>();

    m_infoItem = std::make_shared<ModemInfoItem>();
    m_infoItem->unlock().connect(std::bind(&ModemManager::unlockModem, m_modemManager.get(), m_modem));

    m_actionGroupMerger->add(*m_infoItem);
    m_menu->append(*m_infoItem);

    m_modem->simStatus().changed().connect(std::bind(&Private::update, this));
    m_modem->operatorName().changed().connect(std::bind(&Private::update, this));
    m_modem->status().changed().connect(std::bind(&Private::update, this));
    m_modem->strength().changed().connect(std::bind(&Private::update, this));
    m_modem->technology().changed().connect(std::bind(&Private::update, this));
    update();


//    void setStatusIcon(const std::string &name);
//    void setStatusText(const std::string &value);
//    void setConnectivityIcon(const std::string &name);
//    void setSimIdentifierText(const std::string &value);
//    void setLocked(bool value);
//    void setRoaming(bool value);
}

void
WwanLinkItem::Private::update()
{
    std::string simIdentifier = "";

    switch(m_modem->simStatus().get()) {
    case Modem::SimStatus::missing:
        m_infoItem->setStatusIcon("");
        m_infoItem->setStatusText(_("No SIM"));
        m_infoItem->setConnectivityIcon("");
        m_infoItem->setSimIdentifierText(simIdentifier);
        m_infoItem->setLocked(false);
        m_infoItem->setRoaming(false);
        break;
    case Modem::SimStatus::error:
        m_infoItem->setStatusIcon("");
        m_infoItem->setStatusText(_("SIM Error"));
        m_infoItem->setConnectivityIcon("");
        m_infoItem->setSimIdentifierText(simIdentifier);
        m_infoItem->setLocked(false);
        m_infoItem->setRoaming(false);
        break;
    case Modem::SimStatus::locked:
    case Modem::SimStatus::permanentlyLocked:
        m_infoItem->setStatusIcon("");
        m_infoItem->setStatusText(_("SIM Locked"));
        m_infoItem->setConnectivityIcon("");
        m_infoItem->setSimIdentifierText(simIdentifier);
        m_infoItem->setLocked(true);
        m_infoItem->setRoaming(false);
        break;
    case Modem::SimStatus::ready:
        m_infoItem->setStatusIcon("");
        m_infoItem->setStatusText(m_modem->operatorName().get());
        m_infoItem->setConnectivityIcon("");
        m_infoItem->setSimIdentifierText(simIdentifier);
        m_infoItem->setLocked(false);
        m_infoItem->setRoaming(false);

        std::cout << (int) m_modem->status().get() << std::endl;
        if (m_modem->online().get()) {
            switch (m_modem->status().get()) {
            case org::ofono::Interface::NetworkRegistration::Status::unregistered:
                m_infoItem->setStatusText(_("Unregistered"));
                break;
            case org::ofono::Interface::NetworkRegistration::Status::denied:
                m_infoItem->setStatusText(_("Denied"));
                break;
            case org::ofono::Interface::NetworkRegistration::Status::searching:
                m_infoItem->setStatusText(_("Searching"));
                break;
            case org::ofono::Interface::NetworkRegistration::Status::roaming:
                m_infoItem->setRoaming(true);
                /* fallthrough */
            case org::ofono::Interface::NetworkRegistration::Status::registered:
                std::cout << "OPERATOR: " << m_modem->operatorName().get() << std::endl;
                m_infoItem->setStatusText(m_modem->operatorName());
                m_infoItem->setStatusIcon(Modem::strengthIcon(m_modem->strength().get()));
                if (m_modem->strength().get() == 0) {
                    m_infoItem->setStatusText(_("No Signal"));
                }
                break;
            case org::ofono::Interface::NetworkRegistration::Status::unknown:
                break;
            }
        } else {
            m_infoItem->setStatusIcon("airplane-mode");
            m_infoItem->setStatusText(_("Offline"));
        }

        break;
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
