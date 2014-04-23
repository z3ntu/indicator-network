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

#include "root-state.h"

#include <functional>

#include <connectivity/networking/wifi/link.h>
#include <connectivity/networking/wifi/access-point.h>

namespace networking = connectivity::networking;

class RootState::Private
{
public:
    std::shared_ptr<networking::Manager> m_manager;
    ModemManager::Ptr m_modemManager;
    core::Property<Variant> m_state;

    std::string m_preLabel;
    std::string m_label;

    /// @todo multiple adapters etc..
    std::string m_networkingIcon;

    std::string m_modemTechIcon;

    bool m_inFlightMode;
    bool m_roaming;

    std::map<Modem::Ptr, std::string> m_cellularIcons;

    Private() = delete;
    Private(std::shared_ptr<networking::Manager> manager, ModemManager::Ptr modemManager);

    void flightModeChanged(networking::Manager::FlightModeStatus status);
    void modemsChanged(const std::set<Modem::Ptr> &modems);

    void updateModem(Modem::WeakPtr weakModem);

    void updateNetworkingIcon();

    Variant createIcon(const std::string name);
    void updateRootState();
};

RootState::Private::Private(std::shared_ptr<networking::Manager> manager, ModemManager::Ptr modemManager)
    : m_manager{manager},
      m_modemManager{modemManager}
{    
    m_manager->flightMode().changed().connect(std::bind(&Private::flightModeChanged, this, std::placeholders::_1));
    flightModeChanged(m_manager->flightMode().get());

    m_modemManager->modems().changed().connect(std::bind(&Private::modemsChanged, this, std::placeholders::_1));
    modemsChanged(m_modemManager->modems().get());

    m_manager->status().changed().connect(std::bind(&Private::updateNetworkingIcon, this));
    m_manager->links().changed().connect(std::bind(&Private::updateNetworkingIcon, this));
    updateNetworkingIcon();
}

void
RootState::Private::flightModeChanged(networking::Manager::FlightModeStatus status)
{
    switch(status) {
    case networking::Manager::FlightModeStatus::off:
        m_inFlightMode = false;
        break;
    case networking::Manager::FlightModeStatus::on:
        m_inFlightMode = true;
        break;
    }
    updateRootState();
}

void
RootState::Private::modemsChanged(const std::set<Modem::Ptr> &modems)
{
    /// @todo we have to address the correct ordering of the modems

    std::set<Modem::Ptr> current;
    for (auto element : m_cellularIcons)
        current.insert(element.first);

    std::set<Modem::Ptr> removed;
    std::set_difference(current.begin(), current.end(),
                        modems.begin(), modems.end(),
                        std::inserter(removed, removed.begin()));

    std::set<Modem::Ptr> added;
    std::set_difference(modems.begin(), modems.end(),
                        current.begin(), current.end(),
                        std::inserter(added, added.begin()));
    for (auto modem : removed)
        m_cellularIcons.erase(modem);

    for (auto modem : added) {
        modem->simStatus().changed().connect(std::bind(&Private::updateModem, this, Modem::WeakPtr(modem)));
        modem->status().changed().connect(std::bind(&Private::updateModem, this, Modem::WeakPtr(modem)));
        modem->technology().changed().connect(std::bind(&Private::updateModem, this, Modem::WeakPtr(modem)));
        modem->strength().changed().connect(std::bind(&Private::updateModem, this, Modem::WeakPtr(modem)));
        updateModem(modem);
    }
}

void
RootState::Private::updateModem(Modem::WeakPtr weakModem)
{
    auto modem = weakModem.lock();
    if (!modem) {
        std::cout << std::string(__PRETTY_FUNCTION__) << ": modem expired" << std::endl;
        return;
    }

    /// @todo multisim for all of these
    m_preLabel.clear();
    m_roaming = false;
    m_modemTechIcon.clear();

    m_cellularIcons[modem] = "";


    switch(modem->simStatus().get()) {
    case Modem::SimStatus::offline:
        /// @todo show something.
        break;
    case Modem::SimStatus::missing:
        m_preLabel = _("No SIM");
        break;
    case Modem::SimStatus::error:
        m_preLabel = _("SIM Error");
        break;
    case Modem::SimStatus::locked:
    case Modem::SimStatus::permanentlyLocked:
        /// @todo handle perm blocked somehow
        m_preLabel = _("SIM Locked");
        break;
    case Modem::SimStatus::ready:
    {
        switch (modem->status().get()) {
        case org::ofono::Interface::NetworkRegistration::Status::unregistered:
            /// @todo show something?
            break;
        case org::ofono::Interface::NetworkRegistration::Status::denied:
            /// @todo maybe show something like "Denied" ?
            m_preLabel = _("No Signal");
            break;
        case org::ofono::Interface::NetworkRegistration::Status::unknown:
        case org::ofono::Interface::NetworkRegistration::Status::searching:
        case org::ofono::Interface::NetworkRegistration::Status::registered:
            /// @todo show something?
            break;
        case org::ofono::Interface::NetworkRegistration::Status::roaming:
            /// @todo multisim
            m_roaming = true;
            break;
        }

        auto strength = modem->strength().get();
        if (strength >= 80)
            m_cellularIcons[modem] = "gsm-3g-full";
        else if (strength >= 60)
            m_cellularIcons[modem] = "gsm-3g-high";
        else if (strength >= 40)
            m_cellularIcons[modem] = "gsm-3g-medium";
        else if (strength >= 20)
            m_cellularIcons[modem] = "gsm-3g-low";
        else
            m_cellularIcons[modem] = "gsm-3g-none";

        switch (modem->technology().get()){
        case org::ofono::Interface::NetworkRegistration::Technology::notAvailable:
            /// @todo check this..
            // "network-cellular-pre-edge"
            //  a11ydesc = _("Network (cellular, %s)").printf(current_protocol)
            break;
        case org::ofono::Interface::NetworkRegistration::Technology::gsm:
            m_modemTechIcon = "network-cellular-pre-edge";
            break;
        case org::ofono::Interface::NetworkRegistration::Technology::edge:
            m_modemTechIcon = "network-cellular-edge";
            break;
        case org::ofono::Interface::NetworkRegistration::Technology::umts:
            m_modemTechIcon = "network-cellular-umts";
            break;
        case org::ofono::Interface::NetworkRegistration::Technology::hspa:
            m_modemTechIcon = "network-cellular-hspa";
            break;
        /// @todo oFono can't tell us about hspa+ yet
        //case org::ofono::Interface::NetworkRegistration::Technology::hspaplus:
        //    break;
        case org::ofono::Interface::NetworkRegistration::Technology::lte:
            m_modemTechIcon = "network-cellular-lte";
            break;
        }
        // we might have changed the modem tech icon which affects the networkingIcon.
        updateNetworkingIcon();
        break;
    }}

    updateRootState();
}

void
RootState::Private::updateNetworkingIcon()
{
    m_networkingIcon.clear();

    switch (m_manager->status().get()) {
    case networking::Manager::NetworkingStatus::offline:
        m_networkingIcon = "nm-no-connection";
        //a11ydesc = _("Network (none)");
        break;
    case networking::Manager::NetworkingStatus::connecting:
        m_networkingIcon = "nm-no-connection";
        // some sort of connection animation
        break;
    case networking::Manager::NetworkingStatus::online:
        for (auto link : m_manager->links().get()) {
            if (link->status().get() != networking::Link::Status::online)
                continue;
            if (link->type() == networking::Link::Type::wifi) {

                auto wifiLink = std::dynamic_pointer_cast<networking::wifi::Link>(link);

                int strength = wifiLink->activeAccessPoint().get()->strength().get();
                bool secured = wifiLink->activeAccessPoint().get()->secured();

#if 0
                gchar *a11ydesc = nullptr;
                if (secured) {
                    a11ydesc = g_strdup_printf(_("Network (wireless, %d%%, secure)"), strength);
                } else {
                    a11ydesc = g_strdup_printf(_("Network (wireless, %d%%)"), strength);
                }
                m_a11ydesc = {a11ydesc};
                g_free(a11ydesc);
#endif

                if (strength >= 80) {
                    m_networkingIcon = secured ? "nm-signal-100-secure" : "nm-signal-100";
                } else if (strength >= 60) {
                    m_networkingIcon = secured ? "nm-signal-75-secure" : "nm-signal-75";
                } else if (strength >= 40) {
                    m_networkingIcon = secured ? "nm-signal-50-secure" : "nm-signal-50";
                } else if (strength >= 20) {
                    m_networkingIcon = secured ? "nm-signal-25-secure" : "nm-signal-25";
                } else {
                    m_networkingIcon = secured ? "nm-signal-0-secure" : "nm-signal-0";
                }

            } else if (link->type() == networking::Link::Type::wwan) {
                /// @todo show the tech icon
            }
        }

        if (m_networkingIcon.empty()) {
            /// @todo fix this once modems are part of connectivity-api
            // seems we don't have an active wifi connection..
            // it must be a cellular one then.
            m_networkingIcon = m_modemTechIcon;
        }
        break;
    }

    updateRootState();
}

Variant
RootState::Private::createIcon(const std::string name)
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

void
RootState::Private::updateRootState()
{
    std::vector<std::string> icons;
    std::map<std::string, Variant> state;

    if (m_inFlightMode)
        icons.push_back("airplane-mode");

    for (auto icon : m_cellularIcons)
        if (!icon.second.empty())
            icons.push_back(icon.second);

    if (m_roaming)
        icons.push_back("network-cellular-roaming");

    if (!m_networkingIcon.empty()) {

        /* We're doing icon always right now so we have a fallback before everyone
           supports multi-icon.  We shouldn't set both in the future. */
        try {
            state["icon"] = createIcon(m_networkingIcon);
        } catch (std::exception &e) {
            std::cerr << e.what();
        }

        icons.push_back(m_networkingIcon);
    }


    std::cout << "PRELABEL: " << m_preLabel << std::endl;
    if (!m_preLabel.empty())
        state["pre-label"] = TypedVariant<std::string>(m_preLabel);

    if (!m_label.empty())
        state["label"] = TypedVariant<std::string>(m_label);



    /// @todo translation hint
    state["title"] = TypedVariant<std::string>(_("Network"));

    /// @todo state["accessibility-desc"] = TypedVariant<std::string>(a11ydesc);
    state["visible"] = TypedVariant<bool>(true); /// @todo is this really necessary/useful?


    if (!icons.empty()) {
        std::vector<Variant> iconVariants;
        for (auto name : icons) {
            try {
                iconVariants.push_back(createIcon(name));
            } catch (std::exception &e) {
                std::cerr << e.what();
            }
        }
        state["icons"] = TypedVariant<std::vector<Variant>>(iconVariants);
    }

    m_state.set(TypedVariant<std::map<std::string, Variant>>(state));
}

RootState::RootState(std::shared_ptr<connectivity::networking::Manager> manager, ModemManager::Ptr modemManager)
{
    d.reset(new Private(manager, modemManager));
}

RootState::~RootState()
{}

const core::Property<Variant> &
RootState::state()
{
    return d->m_state;
}
