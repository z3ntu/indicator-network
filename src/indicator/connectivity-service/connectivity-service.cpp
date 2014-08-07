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

#include "connectivity-service.h"
#include <dbus-cpp/services/connectivity.h>

#include <core/dbus/asio/executor.h>
#include <core/dbus/helper/type_mapper.h>

#include <sstream>

template<typename T>
std::string generatePropertyIntrospectionXml()
{
    std::ostringstream output;

    output << "<property ";
    output << "name=\"" << T::name() << "\" ";
    output << "type=\"" << (char)core::dbus::helper::TypeMapper<typename T::ValueType>::type_value() << "\" ";

    output << "access=\"";
    if (T::readable && T::writable)
        output << "readwrite";
    else if (T::readable && !T::writable)
        output << "read";
    else if (!T::readable && T::writable)
        output << "write";
    else
        throw(std::logic_error("Property defined as neither readable nor writable."));
    output << "\"";

    output << "/>";
    return output.str();
}

namespace {
struct Interface {
struct Introspectable
{
    inline static const std::string& name()
    {
        static const std::string s{"org.freedesktop.DBus.Introspectable"};
        return s;
    }

    struct Method {
        struct Introspect
        {
            typedef Introspectable Interface;
            inline static std::string name()
            {
                return "Introspect";
            }
            static const bool call_synchronously = true;
            inline static const std::chrono::milliseconds default_timeout()
            {
                return std::chrono::seconds{1};
            }
        };
    };
};
};
}

class ConnectivityService::Private
{
public:
    std::thread m_connectivityServiceWorker;

    core::dbus::Bus::Ptr m_bus;
    core::dbus::Service::Ptr m_service;
    core::dbus::Object::Ptr m_networkingStatusObject;

    std::shared_ptr<com::ubuntu::connectivity::Interface::NetworkingStatus> m_networkingStatus;

    std::shared_ptr<core::dbus::Property<com::ubuntu::connectivity::Interface::NetworkingStatus::Property::Status>> m_status;

    Private();
    ~Private();

    std::string introspectXml();
};

ConnectivityService::Private::Private()
{
    std::cout << __PRETTY_FUNCTION__ << std::endl;
    m_bus = std::make_shared<core::dbus::Bus>(core::dbus::WellKnownBus::session);

    auto executor = core::dbus::asio::make_executor(m_bus);
    m_bus->install_executor(executor);
    m_connectivityServiceWorker = std::move(std::thread([this](){ m_bus->run(); }));

    m_service = core::dbus::Service::add_service<com::ubuntu::connectivity::Service>(m_bus);

    m_networkingStatusObject = m_service->add_object_for_path(core::dbus::types::ObjectPath(com::ubuntu::connectivity::Interface::NetworkingStatus::path()));
    m_networkingStatus = std::make_shared<com::ubuntu::connectivity::Interface::NetworkingStatus>(m_service, m_networkingStatusObject);

    m_networkingStatusObject->install_method_handler<Interface::Introspectable::Method::Introspect>([this](const core::dbus::Message::Ptr& msg)
    {
        auto reply = core::dbus::Message::make_method_return(msg);
        reply->writer() << introspectXml();
        m_bus->send(reply);
    });

    m_status = m_networkingStatusObject->get_property<com::ubuntu::connectivity::Interface::NetworkingStatus::Property::Status>();
    m_status->set("foo");
}

ConnectivityService::Private::~Private()
{
    m_bus->stop();
    try {
        if (m_connectivityServiceWorker.joinable())
            m_connectivityServiceWorker.join();
    } catch(const std::system_error &e) {
        // This is the only exception type that may be thrown.
        // http://en.cppreference.com/w/cpp/thread/thread/join
        std::cerr << "Error when destroying worker thread, error code " << e.code()
                  << ", error message: " << e.what() << std::endl;
    }
}

std::string
ConnectivityService::Private::introspectXml()
{
    std::ostringstream output;

    static const std::string header(
                "<!DOCTYPE node PUBLIC \"-//freedesktop//DTD D-BUS Object Introspection 1.0//EN\n"
                "\"http://www.freedesktop.org/standards/dbus/1.0/introspect.dtd\">\n"
                "<node>\n"
                "    <interface name=\"org.freedesktop.DBus.Introspectable\">\n"
                "        <method name=\"Introspect\">\n"
                "            <arg name=\"data\" direction=\"out\" type=\"s\"/>\n"
                "        </method>\n"
                "    </interface>\n"
                "    <interface name=\"org.freedesktop.DBus.Properties\">\n"
                "        <method name=\"Get\">\n"
                "            <arg name=\"interface\" direction=\"in\" type=\"s\"/>\n"
                "            <arg name=\"propname\" direction=\"in\" type=\"s\"/>\n"
                "            <arg name=\"value\" direction=\"out\" type=\"v\"/>\n"
                "        </method>\n"
                "        <method name=\"Set\">\n"
                "             <arg name=\"interface\" direction=\"in\" type=\"s\"/>\n"
                "             <arg name=\"propname\" direction=\"in\" type=\"s\"/>\n"
                "             <arg name=\"value\" direction=\"in\" type=\"v\"/>\n"
                "        </method>\n"
                "        <method name=\"GetAll\">\n"
                "            <arg name=\"interface\" direction=\"in\" type=\"s\"/>\n"
                "            <arg name=\"props\" direction=\"out\" type=\"a{sv}\"/>\n"
                "        </method>\n"
                "        <signal name=\"PropertiesChanged\"\n>"
                "            <arg type=\"s\" name=\"interface_name\"/>\n"
                "            <arg type=\"a{sv}\" name=\"changed_properties\"/>\n"
                "            <arg type=\"as\" name=\"invalidated_properties\"/>\n"
                "            </signal>\n"
                "    </interface>\n");

    static const std::string footer("</node>\n");

    output << header;

    output << "<interface name=\"" << com::ubuntu::connectivity::Interface::NetworkingStatus::name() << "\">\n";
    output << generatePropertyIntrospectionXml<com::ubuntu::connectivity::Interface::NetworkingStatus::Property::Status>() << std::endl;
    output << "</interface>\n";

    output << footer;

    return output.str();
}

ConnectivityService::ConnectivityService()
    : d{new Private}
{
        std::cout << __PRETTY_FUNCTION__ << std::endl;
}

ConnectivityService::~ConnectivityService()
{}
