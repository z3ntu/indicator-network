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
            ap1.strength->set(90);

            auto ap1_pc = ap1.properties_changed;

            ap2.flags->set(NM_802_11_AP_FLAGS_NONE);
            ap2.mode->set(NM_802_11_MODE_ADHOC);
            ap2.ssid->set({0x31, 0x32, 0x33}); // 123
            ap2.strength->set(60);
            auto ap2_pc = ap2.properties_changed;

            DefaultURfkillMock urfkill(bus);

            std::thread t{[bus](){ bus->run(); }};

            // add just one AP initially
            aps.push_back(ap1.object->path());

            services_ready.try_signal_ready_for(std::chrono::milliseconds{5000});
            EXPECT_EQ(1, client_ready.wait_for_signal_ready_for(std::chrono::milliseconds{5000}));

            NM::Interface::AccessPoint::Signal::PropertiesChanged::ArgumentType properties;

            properties["Strength"] = dbus::types::Variant::encode<std::int8_t>(0);
            ap1_pc->emit(properties);
            properties["Strength"] = dbus::types::Variant::encode<std::int8_t>(1);
            ap1_pc->emit(properties);
            properties["Strength"] = dbus::types::Variant::encode<std::int8_t>(2);
            ap1_pc->emit(properties);
            properties["Strength"] = dbus::types::Variant::encode<std::int8_t>(3);
            ap1_pc->emit(properties);
            properties["Strength"] = dbus::types::Variant::encode<std::int8_t>(4);
            ap1_pc->emit(properties);

            stage1.try_signal_ready_for(std::chrono::milliseconds{5000});

            // add second ap
            aps.push_back(ap2.object->path());
            ap_added->emit(ap2.object->path());

            sleep(1);

            properties["Strength"] = dbus::types::Variant::encode<std::int8_t>(5);
            ap1_pc->emit(properties);
            properties["Strength"] = dbus::types::Variant::encode<std::int8_t>(6);
            ap1_pc->emit(properties);
            properties["Strength"] = dbus::types::Variant::encode<std::int8_t>(7);
            ap1_pc->emit(properties);
            properties["Strength"] = dbus::types::Variant::encode<std::int8_t>(8);
            ap1_pc->emit(properties);
            properties["Strength"] = dbus::types::Variant::encode<std::int8_t>(9);
            ap1_pc->emit(properties);

            properties["Strength"] = dbus::types::Variant::encode<std::int8_t>(100);
            ap2_pc->emit(properties);
            properties["Strength"] = dbus::types::Variant::encode<std::int8_t>(99);
            ap2_pc->emit(properties);
            properties["Strength"] = dbus::types::Variant::encode<std::int8_t>(98);
            ap2_pc->emit(properties);
            properties["Strength"] = dbus::types::Variant::encode<std::int8_t>(97);
            ap2_pc->emit(properties);
            properties["Strength"] = dbus::types::Variant::encode<std::int8_t>(96);
            ap2_pc->emit(properties);
            ap2_pc->emit(properties);

            stage2.try_signal_ready_for(std::chrono::milliseconds{5000});

            // remove the first ap
            aps.clear();
            aps.push_back(ap2.object->path());
            ap_removed->emit(ap1.object->path());

            sleep(1);

            properties["Strength"] = dbus::types::Variant::encode<std::int8_t>(95);
            ap2_pc->emit(properties);
            properties["Strength"] = dbus::types::Variant::encode<std::int8_t>(94);
            ap2_pc->emit(properties);
            properties["Strength"] = dbus::types::Variant::encode<std::int8_t>(93);
            ap2_pc->emit(properties);
            properties["Strength"] = dbus::types::Variant::encode<std::int8_t>(92);
            ap2_pc->emit(properties);
            properties["Strength"] = dbus::types::Variant::encode<std::int8_t>(91);
            ap2_pc->emit(properties);

            stage3.try_signal_ready_for(std::chrono::milliseconds{5000});

            // add the first ap back
            aps.push_back(ap1.object->path());
            ap_added->emit(ap1.object->path());

            sleep(1);

            properties["Strength"] = dbus::types::Variant::encode<std::int8_t>(10);
            ap1_pc->emit(properties);
            properties["Strength"] = dbus::types::Variant::encode<std::int8_t>(11);
            ap1_pc->emit(properties);
            properties["Strength"] = dbus::types::Variant::encode<std::int8_t>(12);
            ap1_pc->emit(properties);
            properties["Strength"] = dbus::types::Variant::encode<std::int8_t>(13);
            ap1_pc->emit(properties);
            properties["Strength"] = dbus::types::Variant::encode<std::int8_t>(14);
            ap1_pc->emit(properties);

            properties["Strength"] = dbus::types::Variant::encode<std::int8_t>(90);
            ap2_pc->emit(properties);
            properties["Strength"] = dbus::types::Variant::encode<std::int8_t>(89);
            ap2_pc->emit(properties);
            properties["Strength"] = dbus::types::Variant::encode<std::int8_t>(88);
            ap2_pc->emit(properties);
            properties["Strength"] = dbus::types::Variant::encode<std::int8_t>(87);
            ap2_pc->emit(properties);
            properties["Strength"] = dbus::types::Variant::encode<std::int8_t>(86);
            ap2_pc->emit(properties);

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

            std::vector<double> values_abc;
            std::vector<double> values_123;
            std::map<std::shared_ptr<connectivity::networking::wifi::AccessPoint>, core::ScopedConnection> connections;
            auto ap_connect = [&](std::shared_ptr<connectivity::networking::wifi::AccessPoint> ap) {
                if (ap->ssid() == "abc") {
                    EXPECT_TRUE(ap->secured());
                    EXPECT_FALSE(ap->adhoc());
                    auto con = ap->strength().changed().connect([&values_abc](double value){
                        values_abc.push_back(value);
                    });
                    connections.insert(std::make_pair(ap, core::ScopedConnection(con)));
                } else if (ap->ssid() == "123") {
                    EXPECT_FALSE(ap->secured());
                    EXPECT_TRUE(ap->adhoc());
                    auto con = ap->strength().changed().connect([&values_123](double value){
                        values_123.push_back(value);
                    });
                    connections.insert(std::make_pair(ap, core::ScopedConnection(con)));
                }
            };
            auto aps = wifilink->accessPoints().get();
            EXPECT_EQ(aps.size(), 1);
            for (auto ap : aps) {
                ap_connect(ap);
            }
            aps.clear();
            wifilink->accessPoints().changed().connect([&](std::set<std::shared_ptr<connectivity::networking::wifi::AccessPoint>> aps) {
                connections.clear();
                for (auto ap: aps)
                    ap_connect(ap);
            });

            client_ready.try_signal_ready_for(std::chrono::milliseconds{2000});

            sleep(1);

            EXPECT_EQ(values_abc.size(), 5);
            EXPECT_EQ(values_123.size(), 0);

            EXPECT_FLOAT_EQ(values_abc[0], 0);
            EXPECT_FLOAT_EQ(values_abc[1], 1);
            EXPECT_FLOAT_EQ(values_abc[2], 2);
            EXPECT_FLOAT_EQ(values_abc[3], 3);
            EXPECT_FLOAT_EQ(values_abc[4], 4);

            values_abc.clear();
            values_123.clear();
            EXPECT_EQ(1, stage1.wait_for_signal_ready_for(std::chrono::milliseconds{5000}));

            sleep(1);

            EXPECT_EQ(values_abc.size(), 5);
            EXPECT_EQ(values_123.size(), 5);

            EXPECT_FLOAT_EQ(values_abc[0], 5);
            EXPECT_FLOAT_EQ(values_abc[1], 6);
            EXPECT_FLOAT_EQ(values_abc[2], 7);
            EXPECT_FLOAT_EQ(values_abc[3], 8);
            EXPECT_FLOAT_EQ(values_abc[4], 9);

            EXPECT_FLOAT_EQ(values_123[0], 100);
            EXPECT_FLOAT_EQ(values_123[1], 99);
            EXPECT_FLOAT_EQ(values_123[2], 98);
            EXPECT_FLOAT_EQ(values_123[3], 97);
            EXPECT_FLOAT_EQ(values_123[4], 96);

            values_abc.clear();
            values_123.clear();
            EXPECT_EQ(1, stage2.wait_for_signal_ready_for(std::chrono::milliseconds{5000}));

            sleep(1);

            EXPECT_EQ(values_abc.size(), 0);
            EXPECT_EQ(values_123.size(), 5);

            EXPECT_FLOAT_EQ(values_123[0], 95);
            EXPECT_FLOAT_EQ(values_123[1], 94);
            EXPECT_FLOAT_EQ(values_123[2], 93);
            EXPECT_FLOAT_EQ(values_123[3], 92);
            EXPECT_FLOAT_EQ(values_123[4], 91);

            values_abc.clear();
            values_123.clear();
            EXPECT_EQ(1, stage3.wait_for_signal_ready_for(std::chrono::milliseconds{5000}));

            sleep(1);

            EXPECT_EQ(values_abc.size(), 5);
            EXPECT_EQ(values_123.size(), 5);

            EXPECT_FLOAT_EQ(values_abc[0], 10);
            EXPECT_FLOAT_EQ(values_abc[1], 11);
            EXPECT_FLOAT_EQ(values_abc[2], 12);
            EXPECT_FLOAT_EQ(values_abc[3], 13);
            EXPECT_FLOAT_EQ(values_abc[4], 14);

            EXPECT_FLOAT_EQ(values_123[0], 90);
            EXPECT_FLOAT_EQ(values_123[1], 89);
            EXPECT_FLOAT_EQ(values_123[2], 88);
            EXPECT_FLOAT_EQ(values_123[3], 87);
            EXPECT_FLOAT_EQ(values_123[4], 86);

            return ::testing::Test::HasFailure() ? core::posix::exit::Status::failure : core::posix::exit::Status::success;
        };

        EXPECT_EQ(core::testing::ForkAndRunResult::empty, core::testing::fork_and_run(service, client));
}

