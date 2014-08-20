/*
 * Copyright © 2013 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License version 3,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors:
 *     Antti Kaijanmäki <antti.kaijanmaki@canonical.com>
 */

#include "manager.h"
#include "wifi/link.h"
#include "set_name_for_thread.h"
#include <services/nm.h>
#include <services/urfkill.h>

#include <core/dbus/asio/executor.h>
#include <thread>

using namespace platform::nmofono;
namespace networking = connectivity::networking;
namespace dbus = core::dbus;
namespace fdo = org::freedesktop;
namespace NM = fdo::NetworkManager;


namespace platform {
namespace nmofono {

struct Manager::State
{
    State();
    ~State();

    std::shared_ptr<core::dbus::Bus> m_bus;
    std::thread worker;
};

struct Manager::Private {
    std::shared_ptr<org::freedesktop::NetworkManager::Interface::NetworkManager> nm;
    std::shared_ptr<org::freedesktop::URfkill::Interface::URfkill> urfkill;

    core::Property<connectivity::networking::Manager::FlightModeStatus> m_flightMode;
    core::Property<std::set<std::shared_ptr<connectivity::networking::Link>>> m_links;
    core::Property<std::set<std::shared_ptr<connectivity::networking::Service>>> m_services;
    core::Property<connectivity::networking::Manager::NetworkingStatus> m_status;
    core::Property<std::uint32_t> m_characteristics;
    std::unique_ptr<State> m_state;
};
}
}

Manager::State::State()
{
    try {
        m_bus = std::make_shared<dbus::Bus>(dbus::WellKnownBus::system);
    } catch(const std::runtime_error &e) {
        std::cerr << "Failed to connect to the bus: " << e.what() << std::endl;
        throw;
    }

    auto executor = dbus::asio::make_executor(m_bus);
    m_bus->install_executor(executor);
    worker = std::move(std::thread([this]()
    {
        m_bus->run();
    }));
    location::set_name_for_thread(
                worker,
                "OfonoNmConnectivityManagerWorkerThread");
}

Manager::State::~State()
{
    if (worker.joinable())
    {
        m_bus->stop();
        worker.join();
    }
}

void
Manager::updateNetworkingStatus(std::uint32_t status)
{
    switch(status) {
    case NM_STATE_UNKNOWN:
    case NM_STATE_ASLEEP:
    case NM_STATE_DISCONNECTED:
    case NM_STATE_DISCONNECTING:
    {
        p->m_status.set(networking::Manager::NetworkingStatus::offline);
        break;
    }
    case NM_STATE_CONNECTING:
    {
        p->m_status.set(networking::Manager::NetworkingStatus::connecting);
        break;
    }
    case NM_STATE_CONNECTED_LOCAL:
    case NM_STATE_CONNECTED_SITE:
    case NM_STATE_CONNECTED_GLOBAL:
    {
        p->m_status.set(networking::Manager::NetworkingStatus::online);
        break;
    }
    }
}

Manager::Manager() : p(new Manager::Private())
{
    try {
        p->m_state.reset(new State);
    } catch (...) {

        throw;
    }

    auto nm_service = std::make_shared<NM::Service>(p->m_state->m_bus);
    p->nm = nm_service->nm;

    auto urfkill_service = std::make_shared<fdo::URfkill::Service>(p->m_state->m_bus);
    p->urfkill = urfkill_service->urfkill;

    /// @todo add a watcher for the service
    /// @todo exceptions
    /// @todo offload the initialization to a thread or something
    /// @todo those Id() thingies

    p->nm->device_added->connect([this](const dbus::types::ObjectPath &path){
        std::cout << "Device Added:" << path.as_string() << std::endl;
        auto links = p->m_links.get();
        for (const auto &dev : links) {
            if (std::dynamic_pointer_cast<wifi::Link>(dev)->device_path() == path) {
                // already in the list
                return;
            }
        }

        NM::Interface::Device dev(p->nm->service,
                                  p->nm->service->object_for_path(path));
        if (dev.type() == NM::Interface::Device::Type::wifi) {
            links.insert(std::make_shared<wifi::Link>(dev,
                                                      *p->nm.get(),
                                                      std::make_shared<KillSwitch>(p->urfkill,
                                                                                   p->urfkill->switches[fdo::URfkill::Interface::Killswitch::Type::wlan])));
            p->m_links.set(links);
        }
    });
    p->nm->device_removed->connect([this](const dbus::types::ObjectPath &path){
        std::cout << "Device Removed:" << path.as_string() << std::endl;
        auto links = p->m_links.get();
        for (const auto &dev : links) {
            if (std::dynamic_pointer_cast<wifi::Link>(dev)->device_path() == path) {
                links.erase(dev);
                break;
            }
        }
        p->m_links.set(links);
    });
    std::set<std::shared_ptr<networking::Link> > links;
    for(auto dev : p->nm->get_devices()) {
        switch (dev.type()) {
        case NM::Interface::Device::Type::wifi:
        {
            std::shared_ptr<networking::Link> link;
            link.reset(new wifi::Link(dev,
                                      *p->nm.get(),
                                      std::make_shared<KillSwitch>(p->urfkill,
                                                                   p->urfkill->switches[fdo::URfkill::Interface::Killswitch::Type::wlan])));
            links.insert(link);
            break;
        }
        default:
            ;
        }
    }
    p->m_links.set(links);

    updateNetworkingStatus(p->nm->state->get());
    p->nm->properties_changed->connect([this](NM::Interface::NetworkManager::Signal::PropertiesChanged::ArgumentType map) {
        for (auto entry : map) {
            const std::string &key = entry.first;
            if (key == "ActiveConnections") {

            } else if (key == "PrimaryConnection") {
            } else if (key == "State") {
                updateNetworkingStatus(entry.second.as<std::uint32_t>());
            }
        }
    });

    p->urfkill->flightModeChanged->connect([this](bool value){
        if (value)
            p->m_flightMode.set(networking::Manager::FlightModeStatus::on);
        else
            p->m_flightMode.set(networking::Manager::FlightModeStatus::off);
    });
    if (p->urfkill->isFlightMode())
        p->m_flightMode.set(networking::Manager::FlightModeStatus::on);
    else
        p->m_flightMode.set(networking::Manager::FlightModeStatus::off);

    /// @todo set by the default connections.
    p->m_characteristics.set(networking::Link::Characteristics::empty);
}

void
Manager::enableFlightMode()
{
    std::cout << __PRETTY_FUNCTION__ << std::endl;
    if (!p->urfkill->flightMode(true))
        throw std::runtime_error("Failed to enable flightmode.");
}

void
Manager::disableFlightMode()
{
    std::cout << __PRETTY_FUNCTION__ << std::endl;
    if (!p->urfkill->flightMode(false))
        throw std::runtime_error("Failed to disable flightmode");
}

const core::Property<networking::Manager::FlightModeStatus>&
Manager::flightMode() const
{
    // - connect to each individual URfkill.Killswitch interface
    // - make this property to reflect their combined state
    /// @todo implement flightmode status properly when URfkill gets the flightmode API
    return p->m_flightMode;
}

const core::Property<std::set<std::shared_ptr<networking::Link> > >&
Manager::links() const
{
    return p->m_links;
}

const core::Property<std::set<std::shared_ptr<networking::Service>>>&
Manager::services() const
{
    return p->m_services;
}

const core::Property<networking::Manager::NetworkingStatus> &
Manager::status() const
{
    return p->m_status;
}

const core::Property<std::uint32_t>&
Manager::characteristics() const
{
    return p->m_characteristics;
}
