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
        core::testing::CrossProcessSync stage3;
        core::testing::CrossProcessSync stage4;

        auto service = [&, this]()
        {
            core::testing::SigTermCatcher sc;

            auto bus = system_bus();
            bus->install_executor(core::dbus::asio::make_executor(bus));

            auto service = NM::Service::Mock(bus);
            auto nm_root = service.nm;

            auto wifi_dev = NM::Interface::Device(nm_root->service,
                                                  nm_root->service->add_object_for_path(dbus::types::ObjectPath("/org/freedesktop/NetworkManager/Devices/0")));
            auto ap1 = NM::Interface::AccessPoint(nm_root->service->add_object_for_path(dbus::types::ObjectPath("/org/freedesktop/NetworkManager/AccessPoint/1")));
            auto ap2 = NM::Interface::AccessPoint(nm_root->service->add_object_for_path(dbus::types::ObjectPath("/org/freedesktop/NetworkManager/AccessPoint/2")));

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

            std::vector<dbus::types::ObjectPath> aps;
            wifi_dev.object->install_method_handler<NM::Interface::Device::Wireless::Method::GetAccessPoints>([bus, &aps](const dbus::Message::Ptr& msg)
            {
                auto reply = dbus::Message::make_method_return(msg);                
                reply->writer() << aps;
                bus->send(reply);
            });

            auto ap_added = wifi_dev.object->get_signal<NM::Interface::Device::Wireless::Signal::AccessPointAdded>();
            auto ap_removed = wifi_dev.object->get_signal<NM::Interface::Device::Wireless::Signal::AccessPointRemoved>();

            ap1.flags->set(NM_802_11_AP_FLAGS_PRIVACY);
            ap1.mode->set(NM_802_11_MODE_INFRA);
            ap1.ssid->set({0x61, 0x62, 0x63}); // abc
            ap1.strength->set(60);

            auto ap1_pc = ap1.properties_changed;

            ap2.flags->set(NM_802_11_AP_FLAGS_PRIVACY);
            ap2.mode->set(NM_802_11_MODE_INFRA);
            ap2.ssid->set({0x61, 0x62, 0x63}); // abc
            ap2.strength->set(90);
            auto ap2_pc = ap2.properties_changed;

            DefaultURfkillMock urfkill(bus);

            std::thread t{[bus](){ bus->run(); }};

            // add just one AP initially
            aps.push_back(ap1.object->path());

            services_ready.try_signal_ready_for(std::chrono::milliseconds{5000});
            EXPECT_EQ(1, client_ready.wait_for_signal_ready_for(std::chrono::milliseconds{5000}));

            NM::Interface::AccessPoint::Signal::PropertiesChanged::ArgumentType properties;

            stage1.try_signal_ready_for(std::chrono::milliseconds{5000});

            // add second ap
            aps.push_back(ap2.object->path());
            ap_added->emit(ap2.object->path());

            sleep(1);

            stage2.try_signal_ready_for(std::chrono::milliseconds{5000});

            // remove second ap
            aps.clear();
            aps.push_back(ap1.object->path());
            ap_removed->emit(ap2.object->path());

            sleep(1);

            stage3.try_signal_ready_for(std::chrono::milliseconds{5000});

            aps.clear();
            ap_removed->emit(ap1.object->path());

            sleep(1);

            stage4.try_signal_ready_for(std::chrono::milliseconds{5000});

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

            std::vector<double> values_abc;
            std::vector<double> values_123;
            std::map<std::shared_ptr<connectivity::networking::wifi::AccessPoint>, core::ScopedConnection> connections;
            auto ap_connect = [&](std::shared_ptr<connectivity::networking::wifi::AccessPoint> ap) {
                EXPECT_EQ(ap->ssid(), "abc");
                EXPECT_TRUE(ap->secured());
                EXPECT_FALSE(ap->adhoc());
                //connections.insert(std::make_pair(ap, core::ScopedConnection(con)));
            };

            wifilink->accessPoints().changed().connect([&](std::set<std::shared_ptr<connectivity::networking::wifi::AccessPoint>> aps) {
                connections.clear();
                for (auto ap: aps)
                    ap_connect(ap);
            });

            client_ready.try_signal_ready_for(std::chrono::milliseconds{2000});

            auto aps = wifilink->accessPoints().get();
            EXPECT_EQ(aps.size(), 1);
            EXPECT_EQ((*aps.begin())->strength().get(), 60);
            sleep(1);

            EXPECT_EQ(1, stage1.wait_for_signal_ready_for(std::chrono::milliseconds{5000}));

            aps = wifilink->accessPoints().get();
            EXPECT_EQ(aps.size(), 1);
            EXPECT_EQ((*aps.begin())->strength().get(), 90);


            EXPECT_EQ(1, stage2.wait_for_signal_ready_for(std::chrono::milliseconds{5000}));

            sleep(1);

            aps = wifilink->accessPoints().get();
            EXPECT_EQ(aps.size(), 1);
            EXPECT_EQ((*aps.begin())->strength().get(), 60);

            EXPECT_EQ(1, stage3.wait_for_signal_ready_for(std::chrono::milliseconds{5000}));

            sleep(1);

            aps = wifilink->accessPoints().get();
            EXPECT_EQ(aps.size(), 0);

            EXPECT_EQ(1, stage4.wait_for_signal_ready_for(std::chrono::milliseconds{5000}));

            return ::testing::Test::HasFailure() ? core::posix::exit::Status::failure : core::posix::exit::Status::success;
        };

        EXPECT_EQ(core::testing::ForkAndRunResult::empty, core::testing::fork_and_run(service, client));
}

