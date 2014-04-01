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

#ifndef ACCESS_POINT_ITEM_H
#define ACCESS_POINT_ITEM_H

#include "item.h"
#include "gio-helpers/util.h"
#include "menumodel-cpp/action.h"
#include "menumodel-cpp/menu-item.h"
#include "gio-helpers/variant.h"

#include <core/signal.h>

#include <com/ubuntu/connectivity/networking/wifi/access-point.h>
namespace networking = com::ubuntu::connectivity::networking;

#include <functional>
#include <vector>

class AccessPointItem : public Item
{

public:
    typedef std::shared_ptr<AccessPointItem> Ptr;

    AccessPointItem() = delete;
    AccessPointItem(networking::wifi::AccessPoint::Ptr accessPoint, bool isActive = false)
        : m_accessPoint{accessPoint},
          m_isActive{isActive}
    {
        static int id = 0;
        ++id; /// @todo guard me.

        std::string actionId = "accesspoint." + std::to_string(id);
        std::string strengthActionId = actionId + "::strength";

        m_item = std::make_shared<MenuItem>(m_accessPoint->ssid(),
                                            "indicator." + actionId);



        m_item->setAttribute("x-canonical-type", TypedVariant<std::string>("unity.widgets.systemsettings.tablet.accesspoint"));
        m_item->setAttribute("x-canonical-wifi-ap-is-adhoc", TypedVariant<bool>(m_accessPoint->adhoc()));
        m_item->setAttribute("x-canonical-wifi-ap-is-secure", TypedVariant<bool>(m_accessPoint->secured()));
        m_item->setAttribute("x-canonical-wifi-ap-strength-action", TypedVariant<std::string>("indicator." + strengthActionId));

        m_actionStrength = std::make_shared<Action>(strengthActionId,
                                                    nullptr,
                                                    TypedVariant<std::uint8_t>(m_accessPoint->strength().get()));

        auto con = m_accessPoint->strength().changed().connect(std::bind(&AccessPointItem::setStrength,
                                                                         this,
                                                                         std::placeholders::_1));
        m_connections.push_back(con);

        m_actionActivate = std::make_shared<Action>(actionId,
                                                    nullptr,
                                                    TypedVariant<bool>(m_isActive),
                                                    [this](Variant){
                //bool state = m_actionActivate->getState().as<bool>();

                ///@ todo something weird is happening as the indicator side is not changing the state..
                //value = !value;
    });
        m_actionActivate->activated().connect([this](Variant){
            //bool value = m_actionActivate->getState().as<bool>();
            m_activated();
        });

        m_actionGroup->add(m_actionActivate);
        m_actionGroup->add(m_actionStrength);
    }

    ~AccessPointItem()
    {
        for (auto con : m_connections)
            con.disconnect();
    }

    void setStrength(double value)
    {
        /// @todo narrow_cast<>;
        m_actionStrength->setState(TypedVariant<std::uint8_t>(value));
    }

    void setActive(bool value)
    {
        m_isActive = value;
        m_actionActivate->setState(TypedVariant<bool>(m_isActive));
    }

    virtual MenuItem::Ptr
    menuItem()
    {
        return m_item;
    }

    core::Signal<void> &activated()
    {
        return m_activated;
    }

private:
    networking::wifi::AccessPoint::Ptr m_accessPoint;
    bool m_isActive;

    core::Signal<void> m_activated;

    std::vector<core::Connection> m_connections;

    Action::Ptr m_actionActivate;
    Action::Ptr m_actionStrength;
    MenuItem::Ptr m_item;
};

#endif // ACCESS_POINT_ITEM_H
