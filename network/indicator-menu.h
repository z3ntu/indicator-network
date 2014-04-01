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

#ifndef INDICATOR_MENU_H
#define INDICATOR_MENU_H

#include "gio-helpers/util.h"
#include "menuitems/section.h"
#include "menumodel-cpp/menu-merger.h"
#include "menumodel-cpp/action-group-merger.h"

#include <vector>

class IndicatorMenu
{
public:
    typedef std::shared_ptr<IndicatorMenu> Ptr;

    IndicatorMenu() = delete;
    IndicatorMenu(const std::string &prefix)
        : m_prefix { prefix }
    {
        m_actionGroupMerger = std::make_shared<ActionGroupMerger>();
        m_actionGroup = std::make_shared<ActionGroup>();

        m_actionGroupMerger->add(m_actionGroup);

        updateRootState();
        m_rootAction = std::make_shared<Action>(prefix + ".network-status",
                                                nullptr,
                                                m_rootState);
        m_actionGroup->add(m_rootAction);

        m_rootMenu = std::make_shared<Menu>();
        m_subMenuMerger = std::make_shared<MenuMerger>();

        m_rootItem = MenuItem::newSubmenu(m_subMenuMerger);

        m_rootItem->setAction("indicator." + prefix + ".network-status");
        m_rootItem->setAttribute("x-canonical-type", TypedVariant<std::string>("com.canonical.indicator.root"));
        m_rootMenu->append(m_rootItem);
    }

    Variant createIcon(const std::string name)
    {
        GError *error = NULL;
        auto gicon = g_icon_new_for_string(name.c_str(), &error);
        if (error) {
            g_error_free(error);
            throw std::runtime_error("Could not create GIcon: " + std::string(error->message));
        }

        Variant ret = Variant::fromGVariant(g_icon_serialize(gicon));
        /// @todo not sure about this one:
        g_object_unref(gicon);
        return ret;
    }

    void updateRootState()
    {
        /** @todo pre-label std::string
         *  @todo accessible-desc std::string
         */

        std::map<std::string, Variant> state;
        if (!m_label.empty()) {
            state["label"] = TypedVariant<std::string>(m_label);
        }

        /// @todo translation hint
        state["title"] = TypedVariant<std::string>(_("Network"));
        state["visible"] = TypedVariant<bool>(true); /// @todo is this really necessary/useful?

        if (!m_icon.empty()) {
            try {
                state["icon"] = createIcon(m_icon);
            } catch (std::exception &e) {
                std::cerr << e.what();
            }
        }

        if (!m_icons.empty()) {
            std::vector<Variant> icons;
            for (auto name : m_icons) {
                try {
                    icons.push_back(createIcon(name));
                } catch (std::exception &e) {
                    std::cerr << e.what();
                }
            }
            state["icons"] = TypedVariant<std::vector<Variant>>(icons);
        }

#if 0

        if (m_airplaneMode) {
            try {
                icons += createIcon("airplane-mode");
            } catch (std::exception &e) {
                std::cerr << e.what();
            }
        }

        if (!m_simInstalled) {
            state["pre-label"] = TypedVariant<std::string>(_("No SIM"));
        } else if (m_simLocked) {
            state["pre-label"] = TypedVariant<std::string>(_("SIM Locked"));
        } else if (m_simError) {
            state["pre-label"] = TypedVariant<std::string>(_("SIM Error"));
        } else {
            if (!m_cellStrength) {
                state["pre-label"] = TypedVariant<std::string>(_("No Signal"));
            } else {
                std::string iconName = "gsm-3g-none";
                switch (m_lastCellStrength) {
                case 100:
                    iconName = "gsm-3g-full";
                    break;
                case 75:
                    iconName = "gsm-3g-high";
                    break;
                case 50:
                    iconName = "gsm-3g-medium";
                    break;
                case 25:
                    iconName = "gsm-3g-low";
                    break;
                }
                try {
                    icons += createIcon(iconName);
                } catch (std::exception &e) {
                    std::cerr << e.what();
                }
            }
        }

        if (m_roaming) {
            try {
                icons += createIcon("network-cellular-roaming");
            } catch (std::exception &e) {
                std::cerr << e.what();
            }
        }



#endif
#if 0


        string data_icon;
        string a11ydesc;

        data_icon_name(out data_icon, out a11ydesc);
        /* We're doing icon always right now so we have a fallback before everyone
           supports multi-icon.  We shouldn't set both in the future. */
        var icon = icon_serialize(data_icon);
        if (icon != null) {
            params.insert("icon", icon);
            if (multiicon)
                icons.append_val(icon);
        }

        params.insert("title", new Variant.string(_("Network")));
        params.insert("accessibility-desc", new Variant.string(a11ydesc));



        private void data_icon_name (out string icon_name, out string a11ydesc)
        {

            switch (act_dev.get_device_type ())
            {


                default:
                    icon_name = "nm-no-connection";
                    a11ydesc = _("Network (none)");
                    break;
            }

            return;
        }

        private static void strength_icon (ref int last_strength, int strength)
        {
            if (strength > 70 || (last_strength == 100 && strength > 65)) {
                last_strength = 100;
            } else if (strength > 50 || (last_strength == 75 && strength > 45)) {
                last_strength = 75;
            } else if (strength > 30 || (last_strength == 50 && strength > 25)) {
                last_strength = 50;
            } else if (strength > 10 || (last_strength == 25 && strength > 5)) {
                last_strength = 25;
            } else {
                last_strength = 0;
            }
        }

        if (act_dev != null) {
            act_conn.notify["state"].connect (active_connections_changed);
            act_conn.notify["default"].connect (active_connections_changed);
            act_conn.notify["default6"].connect (active_connections_changed);

            debug(@"Active connection changed to: $(act_dev.get_iface())");

            if (act_dev.get_device_type() == NM.DeviceType.WIFI) {
                act_dev.notify["active-access-point"].connect (active_access_point_changed);
                active_access_point_changed(null, null);
            }
        }

        /* This function guesses the default connection in case
         * multiple ones are connected */
        private NM.ActiveConnection? get_active_connection ()
        {
            ActiveConnection? def6 = null;

            /* The default IPv4 connection has precedence */
            var conns = client.get_active_connections ();

            if (conns == null)
                return null;

            for (uint i = 0; i < conns.length; i++)
            {
                var conn = conns.get(i);

                if (conn.default)
                    return conn;

                if (conn.default6)
                    def6 = conn;
            }

            /* Then the default IPv6 connection otherwise the first in the list */
            if (def6 != null)
                return def6;
            /*TODO: Do we show an active connetion if no default route is present? */
            else if (conns.length > 0)
                return conns.get(0);

            /* If the list is empty we return null */
            return null;
        }

#endif

        m_rootState = TypedVariant<std::map<std::string, Variant>>(state);
        if (m_rootAction) {
            m_rootAction->setState(m_rootState);
        }
    }

    void setLabel(const std::string &value)
    {
        m_label = value;
        updateRootState();
    }

    void setIcon(const std::string &name)
    {
        m_icon = name;
        updateRootState();
    }

    void setIcons(std::vector<std::string> icons)
    {
        m_icons = icons;
        updateRootState();
    }

    virtual void
    addSection(Section::Ptr section)
    {
        m_sections.push_back(section);
        m_actionGroupMerger->add(*section);
        m_subMenuMerger->append(*section);
    }

    Menu::Ptr
    menu() const
    {
        return m_rootMenu;
    }

    ActionGroup::Ptr
    actionGroup() const
    {
        return m_actionGroupMerger->actionGroup();
    }

private:
    std::string m_label;
    std::string m_icon;
    std::vector<std::string> m_icons;
    std::string m_prefix;
    Variant m_rootState;

    Action::Ptr m_rootAction;
    MenuItem::Ptr m_rootItem;

    Menu::Ptr m_rootMenu;
    MenuMerger::Ptr m_subMenuMerger;

    ActionGroupMerger::Ptr m_actionGroupMerger;
    ActionGroup::Ptr m_actionGroup;

    std::vector<Section::Ptr> m_sections;
};

#endif // INDICATOR_MENU_H
