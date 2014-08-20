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
namespace fdo = org::freedesktop;
namespace NM = fdo::NetworkManager;

#include "mocks/urfkill.h"

#include <connectivity/networking/manager.h>
#include <connectivity/networking/wifi/link.h>
#include <connectivity/networking/wifi/access-point.h>

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

TEST_F(Service, wifiLink)
{
        core::testing::CrossProcessSync services_ready;
        core::testing::CrossProcessSync client_ready;
        core::testing::CrossProcessSync stage1;
        core::testing::CrossProcessSync stage2;

        auto service = [&, this]()
        {
            core::testing::SigTermCatcher sc;

            auto bus = system_bus();
            bus->install_executor(core::dbus::asio::make_executor(bus));

            auto service = NM::Service::Mock(bus);
            auto nm_root = service.nm;

            auto wifi_dev = NM::Interface::Device(nm_root->service,
                                                  nm_root->service->add_object_for_path(dbus::types::ObjectPath("/org/freedesktop/NetworkManager/Devices/0")));

            nm_root->object->install_method_handler<NM::Interface::NetworkManager::Method::GetDevices>([bus, &wifi_dev](const dbus::Message::Ptr& msg)
            {
                auto reply = dbus::Message::make_method_return(msg);
                std::vector<dbus::types::ObjectPath> devices;
                devices.push_back(wifi_dev.object->path());
                reply->writer() << devices;
                bus->send(reply);
            });

            nm_root->state->set(NM_STATE_CONNECTED_GLOBAL);
            wifi_dev.device_type->set(NM_DEVICE_TYPE_WIFI);
            wifi_dev.state->set(NM_DEVICE_STATE_ACTIVATED);

            wifi_dev.object->install_method_handler<NM::Interface::Device::Wireless::Method::GetAccessPoints>([&](const dbus::Message::Ptr& msg)
            {
                auto reply = dbus::Message::make_method_return(msg);
                reply->writer() << std::vector<dbus::types::ObjectPath>();
                bus->send(reply);
            });

            DefaultURfkillMock urfkill(bus);
            urfkill.wlan.state.changed().connect([&](fdo::URfkill::Interface::Killswitch::State value) {
                typedef fdo::URfkill::Interface::Killswitch Killswitch;
                switch (value) {
                case Killswitch::State::not_available:
                case Killswitch::State::unblocked:
                    wifi_dev.state_changed->emit(std::make_tuple(NM_DEVICE_STATE_DISCONNECTED, NM_DEVICE_STATE_UNAVAILABLE, NM_DEVICE_STATE_REASON_SUPPLICANT_AVAILABLE));
                    wifi_dev.state_changed->emit(std::make_tuple(NM_DEVICE_STATE_PREPARE, NM_DEVICE_STATE_DISCONNECTED, NM_DEVICE_STATE_REASON_NONE));
                    wifi_dev.state_changed->emit(std::make_tuple(NM_DEVICE_STATE_CONFIG, NM_DEVICE_STATE_PREPARE, NM_DEVICE_STATE_REASON_NONE));
                    wifi_dev.state_changed->emit(std::make_tuple(NM_DEVICE_STATE_NEED_AUTH, NM_DEVICE_STATE_CONFIG, NM_DEVICE_STATE_REASON_NONE));
                    wifi_dev.state_changed->emit(std::make_tuple(NM_DEVICE_STATE_PREPARE, NM_DEVICE_STATE_NEED_AUTH, NM_DEVICE_STATE_REASON_NONE));
                    wifi_dev.state_changed->emit(std::make_tuple(NM_DEVICE_STATE_CONFIG, NM_DEVICE_STATE_PREPARE, NM_DEVICE_STATE_REASON_NONE));
                    wifi_dev.state_changed->emit(std::make_tuple(NM_DEVICE_STATE_IP_CONFIG, NM_DEVICE_STATE_CONFIG, NM_DEVICE_STATE_REASON_NONE));
                    wifi_dev.state_changed->emit(std::make_tuple(NM_DEVICE_STATE_SECONDARIES, NM_DEVICE_STATE_IP_CONFIG, NM_DEVICE_STATE_REASON_NONE));
                    wifi_dev.state_changed->emit(std::make_tuple(NM_DEVICE_STATE_ACTIVATED, NM_DEVICE_STATE_SECONDARIES, NM_DEVICE_STATE_REASON_NONE));
                    break;
                case Killswitch::State::soft_blocked:
                case Killswitch::State::hard_blocked:
                    wifi_dev.state_changed->emit(std::make_tuple(NM_DEVICE_STATE_UNAVAILABLE, NM_DEVICE_STATE_ACTIVATED, NM_DEVICE_STATE_REASON_NONE));
                }
            });

            std::thread t{[bus](){ bus->run(); }};

            services_ready.try_signal_ready_for(std::chrono::milliseconds{5000});
            EXPECT_EQ(1, client_ready.wait_for_signal_ready_for(std::chrono::milliseconds{5000}));


            stage1.try_signal_ready_for(std::chrono::milliseconds{5000});
            sleep(1);

            stage2.try_signal_ready_for(std::chrono::milliseconds{5000});
            sleep(1);

            sc.wait_for_signal_for(std::chrono::seconds{60});

            bus->stop();

            if (t.joinable())
                t.join();

            return ::testing::Test::HasFailure() ? core::posix::exit::Status::failure : core::posix::exit::Status::success;
        };

        auto client = [&, this]()
        {
            EXPECT_EQ(1, services_ready.wait_for_signal_ready_for(std::chrono::milliseconds{5000}));
            std::unique_ptr<connectivity::networking::Manager> mgr;
            mgr = connectivity::networking::Manager::createInstance();

            auto links = mgr->links();
            EXPECT_EQ(links->size(), 1);

            EXPECT_EQ((*links->begin())->type(), connectivity::networking::Link::Type::wifi);
            auto wifilink = std::dynamic_pointer_cast<connectivity::networking::wifi::Link>(*links->begin());
            EXPECT_TRUE(wifilink.get() != nullptr);

            client_ready.try_signal_ready_for(std::chrono::milliseconds{2000});

            wifilink->disable();
            sleep(1);

            EXPECT_EQ(1, stage1.wait_for_signal_ready_for(std::chrono::milliseconds{5000}));

            EXPECT_EQ(wifilink->status().get(), connectivity::networking::Link::Status::disabled);

            wifilink->enable();

            sleep(1);
            EXPECT_EQ(1, stage2.wait_for_signal_ready_for(std::chrono::milliseconds{5000}));

            EXPECT_EQ(wifilink->status().get(), connectivity::networking::Link::Status::online);

            return ::testing::Test::HasFailure() ? core::posix::exit::Status::failure : core::posix::exit::Status::success;
        };

        EXPECT_EQ(core::testing::ForkAndRunResult::empty, core::testing::fork_and_run(service, client));
}

