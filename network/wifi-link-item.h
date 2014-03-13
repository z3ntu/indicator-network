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

#ifndef WIFI_LINK_ITEM_H
#define WIFI_LINK_ITEM_H

#include "menumodel-cpp/util.h"

#include "menuitems/switch-item.h"
#include "menuitems/access-point-item.h"
#include "menuitems/text-item.h"

#include "url-dispatcher.h"

#include "menumodel-cpp/action-group.h"
#include "menumodel-cpp/action-group-merger.h"
#include "menumodel-cpp/menu.h"
#include "menumodel-cpp/menu-merger.h"

#include <com/ubuntu/connectivity/networking/wifi/link.h>
namespace networking = com::ubuntu::connectivity::networking;

#include <algorithm>
#include <locale>

class WifiLinkItem : public Item
{
    networking::wifi::Link::Ptr m_link;

    /// @todo do something with me...
    Action::Ptr m_actionBusy;

    SwitchItem::Ptr m_switch;

    networking::wifi::AccessPoint::Ptr m_activeAccessPoint;
    std::map<networking::wifi::AccessPoint::Ptr, AccessPointItem::Ptr> m_accessPoints;

    Menu::Ptr m_topMenu;

    Menu::Ptr m_connectedBeforeApsMenu;
    Menu::Ptr m_neverConnectedApsMenu;
    MenuMerger::Ptr m_apsMerger;

    Menu::Ptr m_bottomMenu;

    MenuMerger::Ptr m_rootMerger;
    MenuItem::Ptr m_item;

    std::function<bool(MenuItem::Ptr a, MenuItem::Ptr b)> m_accessPointCompare;

    std::string m_icon;
    std::string m_a11ydesc;

    TextItem::Ptr m_otherNetwork;

public:
    typedef std::shared_ptr<WifiLinkItem> Ptr;

    WifiLinkItem() = delete;
    WifiLinkItem(networking::wifi::Link::Ptr link)
        : m_link {link}
    {
        m_topMenu = std::make_shared<Menu>();

        m_connectedBeforeApsMenu = std::make_shared<Menu>();
        m_neverConnectedApsMenu = std::make_shared<Menu>();
        m_apsMerger = std::make_shared<MenuMerger>();

        m_bottomMenu = std::make_shared<Menu>();

        m_rootMerger = std::make_shared<MenuMerger>();

        m_switch = std::make_shared<SwitchItem>(_("Wi-Fi"), "wifi", "enable");
        m_actionGroupMerger->add(m_switch->actionGroup());
        switch (m_link->status().get()) {
        case networking::Link::Status::disabled:
            m_switch->state().set(false);
            break;
        case networking::Link::Status::offline:
        case networking::Link::Status::connecting:
        case networking::Link::Status::connected:
        case networking::Link::Status::online:
            m_switch->state().set(true);
            break;
        }
        m_link->status().changed().connect([this](networking::Link::Status value){
            switch (value) {
            case networking::Link::Status::disabled:
                m_switch->state().set(false);
                break;
            case networking::Link::Status::offline:
            case networking::Link::Status::connecting:
            case networking::Link::Status::connected:
            case networking::Link::Status::online:
                m_switch->state().set(true);
                break;
            }
        });
        m_switch->state().changed().connect([this](bool value){
            if (value) {
                m_link->enable();
            } else {
                m_link->disable();
            }
        });

        m_accessPointCompare = [](MenuItem::Ptr a, MenuItem::Ptr b){
            // order alphabetically by SSID

            std::locale loc;

            std::string a_label = a->label();
            std::string b_label = b->label();

            std::string a_upper;
            std::string b_upper;

            for (std::string::size_type i=0; i<a_label.length(); ++i)
                a_upper += std::toupper(a_label[i], loc);

            for (std::string::size_type i=0; i<b_label.length(); ++i)
                b_upper += std::toupper(b_label[i], loc);

            return a_upper < b_upper;
        };

        updateAccessPoints(m_link->accessPoints().get());
        m_link->accessPoints().changed().connect([this](std::set<networking::wifi::AccessPoint::Ptr> accessPoints){
            updateAccessPoints(accessPoints);
        });

        updateActiveAccessPoint(m_link->activeAccessPoint().get());
        m_link->activeAccessPoint().changed().connect([this](networking::wifi::AccessPoint::Ptr ap){
            updateActiveAccessPoint(ap);
        });

        m_otherNetwork = std::make_shared<TextItem>(_("Other network…"), "wifi", "othernetwork");
        m_actionGroupMerger->add(m_otherNetwork->actionGroup());

        m_topMenu->append(m_switch->menuItem());
        m_rootMerger->append(m_topMenu);

        m_apsMerger->append(m_connectedBeforeApsMenu);
        m_apsMerger->append(m_neverConnectedApsMenu);
        m_rootMerger->append(m_apsMerger);

        m_bottomMenu->append(m_otherNetwork->menuItem());
        m_rootMerger->append(m_bottomMenu);

        m_item = MenuItem::newSection(m_rootMerger);
    }

    void updateAccessPoints(std::set<networking::wifi::AccessPoint::Ptr> accessPoints)
    {

        /// @todo previously connected
        /// @todo apply visibility policy.

        std::set<networking::wifi::AccessPoint::Ptr> old;
        for (auto pair: m_accessPoints) {
            old.insert(pair.first);
        }

        std::set<networking::wifi::AccessPoint::Ptr> added;
        std::set_difference(accessPoints.begin(), accessPoints.end(),
                            old.begin(), old.end(),
                            std::inserter(added, added.begin()));

        std::set<networking::wifi::AccessPoint::Ptr> removed;
        std::set_difference(old.begin(), old.end(),
                            accessPoints.begin(), accessPoints.end(),
                            std::inserter(removed, removed.begin()));

        for (auto ap: removed) {
            bool isActive = (ap == m_activeAccessPoint);
            if (isActive)
                m_connectedBeforeApsMenu->removeAll(m_accessPoints[ap]->menuItem());
            else
                m_neverConnectedApsMenu->removeAll(m_accessPoints[ap]->menuItem());
            /// @todo disconnect activated...
            m_actionGroupMerger->remove(m_accessPoints[ap]->actionGroup());
            m_accessPoints.erase(ap);
        }

        for (auto ap : added) {

            /// @todo handle hidden APs all the way
            if (ap->ssid().empty())
                continue;

            bool isActive = (ap == m_activeAccessPoint);
            auto item = std::make_shared<AccessPointItem>(ap, isActive);
            item->activated().connect([this, ap](){
                m_link->connect_to(ap);
            });
            m_accessPoints[ap] = item;
            m_actionGroupMerger->add(item->actionGroup());
            if (isActive) {
                updateActiveAccessPoint(m_activeAccessPoint);
            } else {
                m_neverConnectedApsMenu->insert(item->menuItem(), m_accessPointCompare);
            }
        }
    }

    void updateActiveAccessPoint(networking::wifi::AccessPoint::Ptr ap)
    {
        m_activeAccessPoint = ap;

        auto current = m_connectedBeforeApsMenu->begin();
        if (current != m_connectedBeforeApsMenu->end()) {
            // move to other menu
            m_neverConnectedApsMenu->insert(*current, m_accessPointCompare);
            m_connectedBeforeApsMenu->clear();
        }

        for (auto pair : m_accessPoints) {
            if (ap && ap == pair.first) {
                m_connectedBeforeApsMenu->insert(pair.second->menuItem(), m_connectedBeforeApsMenu->begin());
                pair.second->setActive(true);
                m_neverConnectedApsMenu->removeAll(pair.second->menuItem());
                continue;
            }
            pair.second->setActive(false);
        }
    }


    /// @todo make a variadict function out of this.
    ///       and make it use locations %1, %2, etc.
    std::string stringPrintf(const std::string &format, ...)
    {
        int bufsize = 256;
        char buf[bufsize];
        va_list arglist;
        va_start(arglist, format);
        vsnprintf(buf, bufsize, format.c_str(), arglist);
        va_end(arglist);
        return std::string(buf);
    }

    void updateIcon()
    {
        if (!m_activeAccessPoint) {
            m_icon = "nm-no-connection";
            m_a11ydesc = _("Network (none)");
            return;
        }

        /// @todo connecting animation..

        int strength = m_activeAccessPoint->strength().get();
        bool secured = m_activeAccessPoint->secured();

        if (secured) {
            m_a11ydesc = stringPrintf(_("Network (wireless, %d%%, secure)"), strength);
        } else {
            m_a11ydesc = stringPrintf(_("Network (wireless, %d%%)"), strength);
        }

        int iconStrength;
        if (strength >= 75)
            iconStrength = 75;
         else if (strength >= 50)
            iconStrength = 50;
        else if (strength >= 25)
            iconStrength = 25;
        else
            iconStrength = 0;

        if (secured)
            m_icon = stringPrintf("nm-signal-%d-secure", iconStrength);
        else
            m_icon = stringPrintf("nm-signal-%d", iconStrength);
    }

    virtual MenuItem::Ptr
    menuItem() {
        return m_item;
    }


};

#endif // WIFI_LINK_ITEM_H
