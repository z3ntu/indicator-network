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
 * Authored by: Antti Kaijanmäki <antti.kaijanmaki@canonical.com>
 */

#include <core/dbus/dbus.h>
#include <core/dbus/fixture.h>
#include <core/dbus/object.h>
#include <core/dbus/property.h>
#include <core/dbus/service.h>
#include <core/dbus/interfaces/properties.h>
#include <core/dbus/types/stl/tuple.h>
#include <core/dbus/types/stl/vector.h>

#include <core/dbus/asio/executor.h>

#include "sig_term_catcher.h"
#include "test_data.h"
#include "test_service.h"

#include <core/testing/cross_process_sync.h>
#include <core/testing/fork_and_run.h>

#include <gtest/gtest.h>

#include <system_error>
#include <thread>

#include <services/nm.h>
#include "mocks/urfkill.h"
namespace fdo = org::freedesktop;
namespace NM = fdo::NetworkManager;

#include <connectivity/networking/manager.h>

namespace dbus = core::dbus;

namespace
{
struct Service : public core::dbus::testing::Fixture {};

auto session_bus_config_file =
        core::dbus::testing::Fixture::default_session_bus_config_file() =
        core::testing::session_bus_configuration_file();

auto system_bus_config_file =
        core::dbus::testing::Fixture::default_system_bus_config_file() =
        core::testing::system_bus_configuration_file();

}

TEST_F(Service, flightMode)
{
        core::testing::CrossProcessSync services_ready;
        core::testing::CrossProcessSync client_ready;

        auto service = [&, this]()
        {
            core::testing::SigTermCatcher sc;

            auto bus = system_bus();
            bus->install_executor(core::dbus::asio::make_executor(bus));

            auto nm_service = NM::Service::Mock(bus);
            auto nm_root = nm_service.nm;

            nm_root->object->install_method_handler<NM::Interface::NetworkManager::Method::GetDevices>([bus, nm_root](const dbus::Message::Ptr& msg)
            {
                auto reply = dbus::Message::make_method_return(msg);
                reply->writer() << std::vector<dbus::types::ObjectPath>();
                bus->send(reply);
            });

            nm_root->state->set(0);

            DefaultURfkillMock urfkill(bus);

            std::thread t{[bus](){ bus->run(); }};

            services_ready.try_signal_ready_for(std::chrono::milliseconds{2000});

            EXPECT_EQ(1, client_ready.wait_for_signal_ready_for(std::chrono::milliseconds{2000}));
            sc.wait_for_signal_for(std::chrono::seconds{10});

            bus->stop();

            if (t.joinable())
                t.join();

            return ::testing::Test::HasFailure() ? core::posix::exit::Status::failure : core::posix::exit::Status::success;
        };

        auto client = [&, this]()
        {
            EXPECT_EQ(1, services_ready.wait_for_signal_ready_for(std::chrono::milliseconds{2000}));
            std::unique_ptr<connectivity::networking::Manager> mgr;
            mgr = connectivity::networking::Manager::createInstance();

            std::vector<connectivity::networking::Manager::FlightModeStatus> statuses;
            try {
                EXPECT_EQ(mgr->flightMode().get(), connectivity::networking::Manager::FlightModeStatus::off);

                mgr->flightMode().changed().connect([&statuses](connectivity::networking::Manager::FlightModeStatus status) {
                    statuses.push_back(status);
                });

                mgr->enableFlightMode();
                mgr->disableFlightMode();

            } catch (const std::runtime_error &e) {
                std::cout << "exception: " << e.what() << std::endl;
            }

            sleep(1);
            client_ready.try_signal_ready_for(std::chrono::milliseconds{2000});

            EXPECT_EQ(statuses.size(), 2);
            EXPECT_EQ(statuses.at(0), connectivity::networking::Manager::FlightModeStatus::on);
            EXPECT_EQ(statuses.at(1), connectivity::networking::Manager::FlightModeStatus::off);

            return ::testing::Test::HasFailure() ? core::posix::exit::Status::failure : core::posix::exit::Status::success;
        };

        EXPECT_EQ(core::testing::ForkAndRunResult::empty, core::testing::fork_and_run(service, client));
}
