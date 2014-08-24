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
            auto ap1 = NM::Interface::AccessPoint(nm_root->service->add_object_for_path(dbus::types::ObjectPath("/org/freedesktop/NetworkManager/AccessPoint/1")));
            auto ap2 = NM::Interface::AccessPoint(nm_root->service->add_object_for_path(dbus::types::ObjectPath("/org/freedesktop/NetworkManager/AccessPoint/2")));

            auto wrong_settings = NM::Interface::Connection(nm_root->service->add_object_for_path(dbus::types::ObjectPath("/org/freedesktop/NetworkManager/Settings/1")));
            auto correct_settings = NM::Interface::Connection(nm_root->service->add_object_for_path(dbus::types::ObjectPath("/org/freedesktop/NetworkManager/Settings/2")));

            wrong_settings.object->install_method_handler<NM::Interface::Connection::Method::GetSettings>([&](const dbus::Message::Ptr& msg)
            {
                auto reply = dbus::Message::make_method_return(msg);
                std::map<std::string, std::map<std::string, core::dbus::types::Variant>> conf;
                std::map<std::string, dbus::types::Variant> wireless_conf;
                wireless_conf["ssid"] = dbus::types::Variant::encode<std::vector<std::int8_t>>({0x64, 0x65, 0x66}); // def
                conf["802-11-wireless"] = wireless_conf;
                reply->writer() << conf;
                bus->send(reply);
            });

            correct_settings.object->install_method_handler<NM::Interface::Connection::Method::GetSettings>([&](const dbus::Message::Ptr& msg)
            {
                auto reply = dbus::Message::make_method_return(msg);
                std::map<std::string, std::map<std::string, core::dbus::types::Variant>> conf;
                std::map<std::string, dbus::types::Variant> wireless_conf;
                wireless_conf["ssid"] = dbus::types::Variant::encode<std::vector<std::int8_t>>({static_cast<std::int8_t>(0xC3),
                                                                                                static_cast<std::int8_t>(0xA4),
                                                                                                static_cast<std::int8_t>(0x62),
                                                                                                static_cast<std::int8_t>(0x63)}); // äbc
                conf["802-11-wireless"] = wireless_conf;
                reply->writer() << conf;
                bus->send(reply);
            });

            nm_root->object->install_method_handler<NM::Interface::NetworkManager::Method::GetDevices>([bus, &wifi_dev](const dbus::Message::Ptr& msg)
            {
                auto reply = dbus::Message::make_method_return(msg);
                std::vector<dbus::types::ObjectPath> devices;
                devices.push_back(wifi_dev.object->path());
                reply->writer() << devices;
                bus->send(reply);
            });

            NM::Interface::Device::Property::AvailableConnections::ValueType available_connections;
            available_connections.push_back(wrong_settings.object->path());
            available_connections.push_back(correct_settings.object->path());
            wifi_dev.available_connections->set(available_connections);

            nm_root->object->install_method_handler<NM::Interface::NetworkManager::Method::ActivateConnection>([&](const dbus::Message::Ptr& msg)
            {
                std::cout << "Server: Active Connection" << std::endl;
                auto reader = msg->reader();

                dbus::types::ObjectPath connection = reader.pop_object_path();
                dbus::types::ObjectPath device = reader.pop_object_path();
                dbus::types::ObjectPath specific_object = reader.pop_object_path();

                EXPECT_EQ(connection, correct_settings.object->path());
                EXPECT_EQ(device, wifi_dev.object->path());
                EXPECT_EQ(specific_object, ap1.object->path());

                auto reply = dbus::Message::make_method_return(msg);
                reply->writer() << dbus::types::ObjectPath("/"); /// @todo return an actual ActiveConnection at some point.
                bus->send(reply);
            });
            nm_root->object->install_method_handler<NM::Interface::NetworkManager::Method::AddAndActivateConnection>([&](const dbus::Message::Ptr& msg)
            {
                std::cout << "Server: Add and Active Connection" << std::endl;
                auto reader = msg->reader();

                std::map<std::string, std::map<std::string, core::dbus::types::Variant>> connection;
                reader >> connection;
                dbus::types::ObjectPath device = reader.pop_object_path();
                dbus::types::ObjectPath specific_object = reader.pop_object_path();

                EXPECT_TRUE(connection.find("802-11-wireless") != connection.end());
                auto wificonf = connection["802-11-wireless"];
                EXPECT_TRUE(wificonf.find("ssid") != wificonf.end());
                EXPECT_EQ(wificonf["ssid"].as<std::vector<std::int8_t>>(), ap2.ssid->get());
                EXPECT_EQ(device, wifi_dev.object->path());
                EXPECT_EQ(specific_object, ap2.object->path());

                auto reply = dbus::Message::make_method_return(msg);
                auto ret = std::make_tuple(dbus::types::ObjectPath("/"), dbus::types::ObjectPath("/"));
                reply->writer() << ret; /// @todo return actual Settings and ActiveConnection at some point.
                bus->send(reply);
            });


            nm_root->state->set(NM_STATE_DISCONNECTED);
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
            // SSID name is äbc to test that utf-8 in names works.
            // The casts are because the underlying datatype is
            // signed char (should be unsigned) and 0x63 > 127.
            ap1.ssid->set({static_cast<std::int8_t>(0xC3),
                           static_cast<std::int8_t>(0xA4),
                           static_cast<std::int8_t>(0x62),
                           static_cast<std::int8_t>(0x63)});
            ap1.strength->set(90);

            auto ap1_pc = ap1.properties_changed;

            ap2.flags->set(NM_802_11_AP_FLAGS_NONE);
            ap2.mode->set(NM_802_11_MODE_ADHOC);
            ap2.ssid->set({0x31, 0x32, 0x33}); // 123
            ap2.strength->set(60);
            auto ap2_pc = ap2.properties_changed;

            DefaultURfkillMock urfkill(bus);

            std::thread t{[bus](){ bus->run(); }};

            aps.push_back(ap1.object->path());
            aps.push_back(ap2.object->path());

            services_ready.try_signal_ready_for(std::chrono::milliseconds{5000});
            EXPECT_EQ(1, client_ready.wait_for_signal_ready_for(std::chrono::milliseconds{5000}));


            stage1.try_signal_ready_for(std::chrono::milliseconds{5000});

            sleep(1);

            stage2.try_signal_ready_for(std::chrono::milliseconds{5000});

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

            auto aps = wifilink->accessPoints().get();
            EXPECT_EQ(aps.size(), 2);

            std::shared_ptr<connectivity::networking::wifi::AccessPoint> ap_abc;
            std::shared_ptr<connectivity::networking::wifi::AccessPoint> ap_123;
            for (auto ap : aps) {
                if (ap->ssid() == "äbc")
                    ap_abc = ap;
                if (ap->ssid() == "123")
                    ap_123 = ap;
            }

            EXPECT_TRUE(ap_abc.get() != nullptr);
            EXPECT_TRUE(ap_123.get() != nullptr);

            client_ready.try_signal_ready_for(std::chrono::milliseconds{2000});

            wifilink->connect_to(ap_abc);

            sleep(1);

            EXPECT_EQ(1, stage1.wait_for_signal_ready_for(std::chrono::milliseconds{5000}));

            wifilink->connect_to(ap_123);

            sleep(1);

            return ::testing::Test::HasFailure() ? core::posix::exit::Status::failure : core::posix::exit::Status::success;
        };

        EXPECT_EQ(core::testing::ForkAndRunResult::empty, core::testing::fork_and_run(service, client));
}

