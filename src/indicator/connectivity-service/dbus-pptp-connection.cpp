/*
 * Copyright (C) 2015 Canonical, Ltd.
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
 *     Pete Woods <pete.woods@canonical.com>
 */

#include <connectivity-service/dbus-pptp-connection.h>
#include <PptpAdaptor.h>
#include <util/dbus-utils.h>

using namespace std;
using namespace nmofono::vpn;

#define DEFINE_PROPERTY_GETTER(varname, type)\
type DBusPptpConnection::varname() const\
{\
    return m_pptpConnection->varname();\
}

#define DEFINE_PROPERTY_GETTER_ENUM(varname)\
int DBusPptpConnection::varname() const\
{\
    return static_cast<int>(m_pptpConnection->varname());\
}

#define DEFINE_PROPERTY_SETTER_ENUM(uppername, type)\
void DBusPptpConnection::set##uppername(int value)\
{\
    m_pptpConnection->set##uppername(static_cast<PptpConnection::type>(value));\
}

#define DEFINE_PROPERTY_UPDATER(varname, strname, type)\
void DBusPptpConnection::varname##Updated(type)\
{\
    notifyProperty(strname);\
}

#define DEFINE_PROPERTY_UPDATER_ENUM(varname, strname, type)\
void DBusPptpConnection::varname##Updated(PptpConnection::type)\
{\
    notifyProperty(strname);\
}

#define DEFINE_PROPERTY_CONNECTION_FORWARD(uppername)\
connect(this, &DBusPptpConnection::set##uppername, m_pptpConnection.get(), &PptpConnection::set##uppername);

#define DEFINE_PROPERTY_CONNECTION_REVERSE(varname)\
connect(m_pptpConnection.get(), &PptpConnection::varname##Changed, this, &DBusPptpConnection::varname##Updated);

namespace connectivity_service
{

DBusPptpConnection::DBusPptpConnection(VpnConnection::SPtr vpnConnection,
                                             const QDBusConnection& connection) :
        DBusVpnConnection(vpnConnection, connection),
        m_pptpConnection(vpnConnection->pptpConnection())
{
    new PptpAdaptor(this);

    // Basic properties

    DEFINE_PROPERTY_CONNECTION_FORWARD(Gateway)
    DEFINE_PROPERTY_CONNECTION_FORWARD(User)
    DEFINE_PROPERTY_CONNECTION_FORWARD(Password)
    DEFINE_PROPERTY_CONNECTION_FORWARD(Domain)

    // Advanced properties

    DEFINE_PROPERTY_CONNECTION_FORWARD(AllowPap)
    DEFINE_PROPERTY_CONNECTION_FORWARD(AllowChap)
    DEFINE_PROPERTY_CONNECTION_FORWARD(AllowMschap)
    DEFINE_PROPERTY_CONNECTION_FORWARD(AllowMschapv2)
    DEFINE_PROPERTY_CONNECTION_FORWARD(AllowEap)
    DEFINE_PROPERTY_CONNECTION_FORWARD(RequireMppe)
    // mppeType is enum
    DEFINE_PROPERTY_CONNECTION_FORWARD(MppeStateful)
    DEFINE_PROPERTY_CONNECTION_FORWARD(BsdCompression)
    DEFINE_PROPERTY_CONNECTION_FORWARD(DeflateCompression)
    DEFINE_PROPERTY_CONNECTION_FORWARD(TcpHeaderCompression)
    DEFINE_PROPERTY_CONNECTION_FORWARD(SendPppEchoPackets)




    // Basic properties

    DEFINE_PROPERTY_CONNECTION_REVERSE(gateway)
    DEFINE_PROPERTY_CONNECTION_REVERSE(user)
    DEFINE_PROPERTY_CONNECTION_REVERSE(password)
    DEFINE_PROPERTY_CONNECTION_REVERSE(domain)

    // Advanced properties

    DEFINE_PROPERTY_CONNECTION_REVERSE(allowPap)
    DEFINE_PROPERTY_CONNECTION_REVERSE(allowChap)
    DEFINE_PROPERTY_CONNECTION_REVERSE(allowMschap)
    DEFINE_PROPERTY_CONNECTION_REVERSE(allowMschapv2)
    DEFINE_PROPERTY_CONNECTION_REVERSE(allowEap)
    DEFINE_PROPERTY_CONNECTION_REVERSE(requireMppe)
    DEFINE_PROPERTY_CONNECTION_REVERSE(mppeType)
    DEFINE_PROPERTY_CONNECTION_REVERSE(mppeStateful)
    DEFINE_PROPERTY_CONNECTION_REVERSE(bsdCompression)
    DEFINE_PROPERTY_CONNECTION_REVERSE(deflateCompression)
    DEFINE_PROPERTY_CONNECTION_REVERSE(tcpHeaderCompression)
    DEFINE_PROPERTY_CONNECTION_REVERSE(sendPppEchoPackets)



    registerDBusObject();
}

DBusPptpConnection::~DBusPptpConnection()
{
}

nmofono::vpn::VpnConnection::Type DBusPptpConnection::type() const
{
    return nmofono::vpn::VpnConnection::Type::pptp;
}

// Enum properties

DEFINE_PROPERTY_SETTER_ENUM(MppeType, MppeType)

// Basic properties


// Basic properties

DEFINE_PROPERTY_GETTER(gateway, QString)
DEFINE_PROPERTY_GETTER(user, QString)
DEFINE_PROPERTY_GETTER(password, QString)
DEFINE_PROPERTY_GETTER(domain, QString)

// Advanced properties

DEFINE_PROPERTY_GETTER(allowPap, bool)
DEFINE_PROPERTY_GETTER(allowChap, bool)
DEFINE_PROPERTY_GETTER(allowMschap, bool)
DEFINE_PROPERTY_GETTER(allowMschapv2, bool)
DEFINE_PROPERTY_GETTER(allowEap, bool)
DEFINE_PROPERTY_GETTER(requireMppe, bool)
DEFINE_PROPERTY_GETTER_ENUM(mppeType)
DEFINE_PROPERTY_GETTER(mppeStateful, bool)
DEFINE_PROPERTY_GETTER(bsdCompression, bool)
DEFINE_PROPERTY_GETTER(deflateCompression, bool)
DEFINE_PROPERTY_GETTER(tcpHeaderCompression, bool)
DEFINE_PROPERTY_GETTER(sendPppEchoPackets, bool)


void DBusPptpConnection::notifyProperty(const QString& propertyName)
{
    DBusUtils::notifyPropertyChanged(
        m_connection,
        *this,
        m_path.path(),
        PptpAdaptor::staticMetaObject.classInfo(PptpAdaptor::staticMetaObject.indexOfClassInfo("D-Bus Interface")).value(),
        {propertyName}
    );
}

// Basic properties

DEFINE_PROPERTY_UPDATER(gateway, "gateway", const QString &)
DEFINE_PROPERTY_UPDATER(user, "user", const QString &)
DEFINE_PROPERTY_UPDATER(password, "password", const QString &)
DEFINE_PROPERTY_UPDATER(domain, "domain", const QString &)

// Advanced properties

DEFINE_PROPERTY_UPDATER(allowPap, "allowPap", bool)
DEFINE_PROPERTY_UPDATER(allowChap, "allowChap", bool)
DEFINE_PROPERTY_UPDATER(allowMschap, "allowMschap", bool)
DEFINE_PROPERTY_UPDATER(allowMschapv2, "allowMschapv2", bool)
DEFINE_PROPERTY_UPDATER(allowEap, "allowEap", bool)
DEFINE_PROPERTY_UPDATER(requireMppe, "requireMppe", bool)
DEFINE_PROPERTY_UPDATER_ENUM(mppeType, "mppeType", MppeType)
DEFINE_PROPERTY_UPDATER(mppeStateful, "mppeStateful", bool)
DEFINE_PROPERTY_UPDATER(bsdCompression, "bsdCompression", bool)
DEFINE_PROPERTY_UPDATER(deflateCompression, "deflateCompression", bool)
DEFINE_PROPERTY_UPDATER(tcpHeaderCompression, "tcpHeaderCompression", bool)
DEFINE_PROPERTY_UPDATER(sendPppEchoPackets, "sendPppEchoPackets", bool)


}
