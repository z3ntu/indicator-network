/*
 * Copyright © 2014 Canonical Ltd.
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

#ifndef MOCKS_URFKILL_H
#define MOCKS_URFKILL_H

#include <services/urfkill.h>

class DefaultURfkillMock : public org::freedesktop::URfkill::Service::Mock
{
    typedef org::freedesktop::URfkill::Interface::Killswitch Killswitch;

    bool inFlightMode;

public:

    DefaultURfkillMock() = delete;
    DefaultURfkillMock(const core::dbus::Bus::Ptr& bus)
        : org::freedesktop::URfkill::Service::Mock(bus),
          inFlightMode{false},
          bt{Killswitch(urfkill->service->add_object_for_path(Killswitch::path(Killswitch::Type::bluetooth)),
                        Killswitch::Type::bluetooth)},
          fm{Killswitch(urfkill->service->add_object_for_path(Killswitch::path(Killswitch::Type::fm)),
                        Killswitch::Type::fm)},
          gps{Killswitch(urfkill->service->add_object_for_path(Killswitch::path(Killswitch::Type::gps)),
                         Killswitch::Type::gps)},
          nfc{Killswitch(urfkill->service->add_object_for_path(Killswitch::path(Killswitch::Type::nfc)),
                         Killswitch::Type::nfc)},
          uwb{Killswitch(urfkill->service->add_object_for_path(Killswitch::path(Killswitch::Type::uwb)),
                         Killswitch::Type::uwb)},
          wimax{Killswitch(urfkill->service->add_object_for_path(Killswitch::path(Killswitch::Type::wimax)),
                           Killswitch::Type::wimax)},
          wlan{Killswitch(urfkill->service->add_object_for_path(Killswitch::path(Killswitch::Type::wlan)),
                          Killswitch::Type::wlan)},
          wwan{Killswitch(urfkill->service->add_object_for_path(Killswitch::path(Killswitch::Type::wwan)),
                          Killswitch::Type::wwan)}
    {
        urfkill->object->install_method_handler
            <org::freedesktop::URfkill::Interface::URfkill::Method::Block>
                ([this, bus](const core::dbus::Message::Ptr& msg)
        {
            auto reader = msg->reader();

            std::uint32_t type = reader.pop_uint32();
            bool value = reader.pop_boolean();

            auto reply = core::dbus::Message::make_method_return(msg);
            reply->writer() << block(type, value);
            bus->send(reply);
        });

        urfkill->object->install_method_handler
            <org::freedesktop::URfkill::Interface::URfkill::Method::FlightMode>
                ([this, bus](const core::dbus::Message::Ptr& msg)
        {
            auto reply = core::dbus::Message::make_method_return(msg);

            auto reader = msg->reader();
            bool value = reader.pop_boolean();

            reply->writer() << true;
            bus->send(reply);

            if (inFlightMode != value) {
                inFlightMode = value;
                auto signal_flightmode_changed
                        = urfkill->object->get_signal<org::freedesktop::URfkill::Interface::URfkill::Signal::FlightModeChanged>();
                signal_flightmode_changed->emit(inFlightMode);

                block(1, inFlightMode);
            }
        });

        urfkill->object->install_method_handler
            <org::freedesktop::URfkill::Interface::URfkill::Method::IsFlightMode>
                ([this, bus](const core::dbus::Message::Ptr& msg)
        {
            auto reply = core::dbus::Message::make_method_return(msg);
            reply->writer() << inFlightMode;
            bus->send(reply);
        });

        bt._state->set(0);
        fm._state->set(0);
        gps._state->set(0);
        nfc._state->set(0);
        uwb._state->set(0);
        wimax._state->set(0);
        wlan._state->set(0);
        wwan._state->set(0);
    }

    bool block(std::uint32_t type, bool block)
    {
        switch (type) {
        case 1:
        {
            if (block)
                wlan._state->set(1); // soft blocked
            else
                wlan._state->set(0); // unblocked

            auto signal_properties_changed
                    = wlan.object->get_signal<core::dbus::interfaces::Properties::Signals::PropertiesChanged>();
            std::map<std::string, core::dbus::types::Variant> changed;
            changed["state"] = core::dbus::types::TypedVariant<std::int32_t>(wlan._state->get());
            core::dbus::interfaces::Properties::Signals::PropertiesChanged::ArgumentType args
                    (org::freedesktop::URfkill::Interface::Killswitch::name(),
                     changed,
                     {}
                   );
            signal_properties_changed->emit(args);
            break;
        }
        case 2:
            //Killswitch::Type::bluetooth
        case 3:
            //Killswitch::Type::uwb
        case 4:
            //Killswitch::Type::wimax
        case 5:
            //Killswitch::Type::wwan
        case 6:
            //Killswitch::Type::gps
        case 7:
            //Killswitch::Type::fm
        case 8:
            //Killswitch::Type::nfc
            break;
        default:
            std::logic_error("Unknown killswitch id: " + std::to_string(type));
        }

        return true;
    }

    org::freedesktop::URfkill::Interface::Killswitch bt;
    org::freedesktop::URfkill::Interface::Killswitch fm;
    org::freedesktop::URfkill::Interface::Killswitch gps;
    org::freedesktop::URfkill::Interface::Killswitch nfc;
    org::freedesktop::URfkill::Interface::Killswitch uwb;
    org::freedesktop::URfkill::Interface::Killswitch wimax;
    org::freedesktop::URfkill::Interface::Killswitch wlan;
    org::freedesktop::URfkill::Interface::Killswitch wwan;
};

#endif
