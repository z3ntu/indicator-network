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

#include <connectivity-service/dbus-openvpn-connection.h>
#include <OpenVpnAdaptor.h>
#include <util/dbus-utils.h>

using namespace std;
using namespace nmofono::vpn;

#define DEFINE_PROPERTY_GETTER(varname, type)\
type DBusOpenvpnConnection::varname() const\
{\
    return m_openvpnConnection->varname();\
}

#define DEFINE_PROPERTY_GETTER_ENUM(varname)\
int DBusOpenvpnConnection::varname() const\
{\
    return static_cast<int>(m_openvpnConnection->varname());\
}

#define DEFINE_PROPERTY_SETTER_ENUM(uppername, type)\
void DBusOpenvpnConnection::set##uppername(int value)\
{\
    m_openvpnConnection->set##uppername(static_cast<OpenvpnConnection::type>(value));\
}

#define DEFINE_PROPERTY_UPDATER(varname, strname, type)\
void DBusOpenvpnConnection::varname##Updated(type)\
{\
    notifyProperty(strname);\
}

#define DEFINE_PROPERTY_UPDATER_ENUM(varname, strname, type)\
void DBusOpenvpnConnection::varname##Updated(OpenvpnConnection::type)\
{\
    notifyProperty(strname);\
}

#define DEFINE_PROPERTY_CONNECTION_FORWARD(uppername)\
connect(this, &DBusOpenvpnConnection::set##uppername, m_openvpnConnection.get(), &OpenvpnConnection::set##uppername);

#define DEFINE_PROPERTY_CONNECTION_REVERSE(varname)\
connect(m_openvpnConnection.get(), &OpenvpnConnection::varname##Changed, this, &DBusOpenvpnConnection::varname##Updated);

namespace connectivity_service
{

DBusOpenvpnConnection::DBusOpenvpnConnection(VpnConnection::SPtr vpnConnection,
                                             const QDBusConnection& connection) :
        DBusVpnConnection(vpnConnection, connection),
        m_openvpnConnection(vpnConnection->openvpnConnection())
{
    new OpenVpnAdaptor(this);

    // Basic properties
    DEFINE_PROPERTY_CONNECTION_FORWARD(Ca)
    DEFINE_PROPERTY_CONNECTION_FORWARD(Cert)
    DEFINE_PROPERTY_CONNECTION_FORWARD(CertPass)
    // connectionType is enum
    DEFINE_PROPERTY_CONNECTION_FORWARD(Key)
    DEFINE_PROPERTY_CONNECTION_FORWARD(LocalIp)
    DEFINE_PROPERTY_CONNECTION_FORWARD(Password)
    DEFINE_PROPERTY_CONNECTION_FORWARD(Remote)
    DEFINE_PROPERTY_CONNECTION_FORWARD(RemoteIp)
    DEFINE_PROPERTY_CONNECTION_FORWARD(StaticKey)
    // staticKeyDirection is enum
    DEFINE_PROPERTY_CONNECTION_FORWARD(Username)

    // Advanced general properties

    DEFINE_PROPERTY_CONNECTION_FORWARD(Port)
    DEFINE_PROPERTY_CONNECTION_FORWARD(PortSet)
    DEFINE_PROPERTY_CONNECTION_FORWARD(RenegSeconds)
    DEFINE_PROPERTY_CONNECTION_FORWARD(RenegSecondsSet)
    DEFINE_PROPERTY_CONNECTION_FORWARD(CompLzo)
    DEFINE_PROPERTY_CONNECTION_FORWARD(ProtoTcp)
    DEFINE_PROPERTY_CONNECTION_FORWARD(Dev)
    // devType is enum
    DEFINE_PROPERTY_CONNECTION_FORWARD(DevTypeSet)
    DEFINE_PROPERTY_CONNECTION_FORWARD(TunnelMtu)
    DEFINE_PROPERTY_CONNECTION_FORWARD(TunnelMtuSet)
    DEFINE_PROPERTY_CONNECTION_FORWARD(FragmentSize)
    DEFINE_PROPERTY_CONNECTION_FORWARD(FragmentSizeSet)
    DEFINE_PROPERTY_CONNECTION_FORWARD(MssFix)
    DEFINE_PROPERTY_CONNECTION_FORWARD(RemoteRandom)

    // Advanced security properties

    // cipher is enum
    DEFINE_PROPERTY_CONNECTION_FORWARD(Keysize)
    DEFINE_PROPERTY_CONNECTION_FORWARD(KeysizeSet)
    // auth is enum

    // Advanced TLS auth properties

    DEFINE_PROPERTY_CONNECTION_FORWARD(TlsRemote)
    // remoteCertTls is enum
    DEFINE_PROPERTY_CONNECTION_FORWARD(RemoteCertTlsSet)
    DEFINE_PROPERTY_CONNECTION_FORWARD(Ta)
    // taDir is enum
    DEFINE_PROPERTY_CONNECTION_FORWARD(TaSet)

    // Advanced proxy settings

    // proxyType is enum
    DEFINE_PROPERTY_CONNECTION_FORWARD(ProxyServer)
    DEFINE_PROPERTY_CONNECTION_FORWARD(ProxyPort)
    DEFINE_PROPERTY_CONNECTION_FORWARD(ProxyRetry)
    DEFINE_PROPERTY_CONNECTION_FORWARD(ProxyUsername)
    DEFINE_PROPERTY_CONNECTION_FORWARD(ProxyPassword)



    // Basic properties

    DEFINE_PROPERTY_CONNECTION_REVERSE(ca)
    DEFINE_PROPERTY_CONNECTION_REVERSE(cert)
    DEFINE_PROPERTY_CONNECTION_REVERSE(certPass)
    DEFINE_PROPERTY_CONNECTION_REVERSE(connectionType)
    DEFINE_PROPERTY_CONNECTION_REVERSE(key)
    DEFINE_PROPERTY_CONNECTION_REVERSE(localIp)
    DEFINE_PROPERTY_CONNECTION_REVERSE(password)
    DEFINE_PROPERTY_CONNECTION_REVERSE(remote)
    DEFINE_PROPERTY_CONNECTION_REVERSE(remoteIp)
    DEFINE_PROPERTY_CONNECTION_REVERSE(staticKey)
    DEFINE_PROPERTY_CONNECTION_REVERSE(staticKeyDirection)
    DEFINE_PROPERTY_CONNECTION_REVERSE(username)

    // Advanced general properties

    DEFINE_PROPERTY_CONNECTION_REVERSE(port)
    DEFINE_PROPERTY_CONNECTION_REVERSE(portSet)
    DEFINE_PROPERTY_CONNECTION_REVERSE(renegSeconds)
    DEFINE_PROPERTY_CONNECTION_REVERSE(renegSecondsSet)
    DEFINE_PROPERTY_CONNECTION_REVERSE(compLzo)
    DEFINE_PROPERTY_CONNECTION_REVERSE(protoTcp)
    DEFINE_PROPERTY_CONNECTION_REVERSE(dev)
    DEFINE_PROPERTY_CONNECTION_REVERSE(devType)
    DEFINE_PROPERTY_CONNECTION_REVERSE(devTypeSet)
    DEFINE_PROPERTY_CONNECTION_REVERSE(tunnelMtu)
    DEFINE_PROPERTY_CONNECTION_REVERSE(tunnelMtuSet)
    DEFINE_PROPERTY_CONNECTION_REVERSE(fragmentSize)
    DEFINE_PROPERTY_CONNECTION_REVERSE(fragmentSizeSet)
    DEFINE_PROPERTY_CONNECTION_REVERSE(mssFix)
    DEFINE_PROPERTY_CONNECTION_REVERSE(remoteRandom)

    // Advanced security properties

    DEFINE_PROPERTY_CONNECTION_REVERSE(cipher)
    DEFINE_PROPERTY_CONNECTION_REVERSE(keysize)
    DEFINE_PROPERTY_CONNECTION_REVERSE(keysizeSet)
    DEFINE_PROPERTY_CONNECTION_REVERSE(auth)

    // Advanced TLS auth properties

    DEFINE_PROPERTY_CONNECTION_REVERSE(tlsRemote)
    DEFINE_PROPERTY_CONNECTION_REVERSE(remoteCertTls)
    DEFINE_PROPERTY_CONNECTION_REVERSE(remoteCertTlsSet)
    DEFINE_PROPERTY_CONNECTION_REVERSE(ta)
    DEFINE_PROPERTY_CONNECTION_REVERSE(taDir)
    DEFINE_PROPERTY_CONNECTION_REVERSE(taSet)

    // Advanced proxy settings

    DEFINE_PROPERTY_CONNECTION_REVERSE(proxyPassword)
    DEFINE_PROPERTY_CONNECTION_REVERSE(proxyPort)
    DEFINE_PROPERTY_CONNECTION_REVERSE(proxyRetry)
    DEFINE_PROPERTY_CONNECTION_REVERSE(proxyServer)
    DEFINE_PROPERTY_CONNECTION_REVERSE(proxyType)
    DEFINE_PROPERTY_CONNECTION_REVERSE(proxyUsername)

    registerDBusObject();
}

DBusOpenvpnConnection::~DBusOpenvpnConnection()
{
}

nmofono::vpn::VpnConnection::Type DBusOpenvpnConnection::type() const
{
    return nmofono::vpn::VpnConnection::Type::openvpn;
}

// Enum properties

DEFINE_PROPERTY_SETTER_ENUM(ConnectionType, ConnectionType)
DEFINE_PROPERTY_SETTER_ENUM(StaticKeyDirection, KeyDir)
DEFINE_PROPERTY_SETTER_ENUM(DevType, DevType)
DEFINE_PROPERTY_SETTER_ENUM(Cipher, Cipher)
DEFINE_PROPERTY_SETTER_ENUM(Auth, Auth)
DEFINE_PROPERTY_SETTER_ENUM(RemoteCertTls, TlsType)
DEFINE_PROPERTY_SETTER_ENUM(TaDir, KeyDir)
DEFINE_PROPERTY_SETTER_ENUM(ProxyType, ProxyType)

// Basic properties

DEFINE_PROPERTY_GETTER(ca, QString)
DEFINE_PROPERTY_GETTER(cert, QString)
DEFINE_PROPERTY_GETTER(certPass, QString)
DEFINE_PROPERTY_GETTER_ENUM(connectionType)
DEFINE_PROPERTY_GETTER(key, QString)
DEFINE_PROPERTY_GETTER(localIp, QString)
DEFINE_PROPERTY_GETTER(password, QString)
DEFINE_PROPERTY_GETTER(remote, QString)
DEFINE_PROPERTY_GETTER(remoteIp, QString)
DEFINE_PROPERTY_GETTER(staticKey, QString)
DEFINE_PROPERTY_GETTER_ENUM(staticKeyDirection)
DEFINE_PROPERTY_GETTER(username, QString)

// Advanced general properties

DEFINE_PROPERTY_GETTER(port, int)
DEFINE_PROPERTY_GETTER(portSet, bool)
DEFINE_PROPERTY_GETTER(renegSeconds, int)
DEFINE_PROPERTY_GETTER(renegSecondsSet, bool)
DEFINE_PROPERTY_GETTER(compLzo, bool)
DEFINE_PROPERTY_GETTER(protoTcp, bool)
DEFINE_PROPERTY_GETTER(dev, QString)
DEFINE_PROPERTY_GETTER_ENUM(devType)
DEFINE_PROPERTY_GETTER(devTypeSet, bool)
DEFINE_PROPERTY_GETTER(tunnelMtu, int)
DEFINE_PROPERTY_GETTER(tunnelMtuSet, bool)
DEFINE_PROPERTY_GETTER(fragmentSize, int)
DEFINE_PROPERTY_GETTER(fragmentSizeSet, bool)
DEFINE_PROPERTY_GETTER(mssFix, bool)
DEFINE_PROPERTY_GETTER(remoteRandom, bool)

// Advanced security properties

DEFINE_PROPERTY_GETTER_ENUM(cipher)
DEFINE_PROPERTY_GETTER(keysize, int)
DEFINE_PROPERTY_GETTER(keysizeSet, bool)
DEFINE_PROPERTY_GETTER_ENUM(auth)

// Advanced TLS auth properties

DEFINE_PROPERTY_GETTER(tlsRemote, QString)
DEFINE_PROPERTY_GETTER_ENUM(remoteCertTls)
DEFINE_PROPERTY_GETTER(remoteCertTlsSet, bool)
DEFINE_PROPERTY_GETTER(ta, QString)
DEFINE_PROPERTY_GETTER_ENUM(taDir)
DEFINE_PROPERTY_GETTER(taSet, bool)

// Advanced proxy settings

DEFINE_PROPERTY_GETTER_ENUM(proxyType)
DEFINE_PROPERTY_GETTER(proxyServer, QString)
DEFINE_PROPERTY_GETTER(proxyPort, int)
DEFINE_PROPERTY_GETTER(proxyRetry, bool)
DEFINE_PROPERTY_GETTER(proxyUsername, QString)
DEFINE_PROPERTY_GETTER(proxyPassword, QString)

void DBusOpenvpnConnection::notifyProperty(const QString& propertyName)
{
    DBusUtils::notifyPropertyChanged(
        m_connection,
        *this,
        m_path.path(),
        OpenVpnAdaptor::staticMetaObject.classInfo(OpenVpnAdaptor::staticMetaObject.indexOfClassInfo("D-Bus Interface")).value(),
        {propertyName}
    );
}

// Basic properties

DEFINE_PROPERTY_UPDATER(ca, "ca", const QString &)
DEFINE_PROPERTY_UPDATER(cert, "cert", const QString &)
DEFINE_PROPERTY_UPDATER(certPass, "certPass", const QString &)
DEFINE_PROPERTY_UPDATER_ENUM(connectionType, "connectionType", ConnectionType)
DEFINE_PROPERTY_UPDATER(key, "key", const QString &)
DEFINE_PROPERTY_UPDATER(localIp, "localIp", const QString &)
DEFINE_PROPERTY_UPDATER(password, "password", const QString &)
DEFINE_PROPERTY_UPDATER(remote, "remote", const QString &)
DEFINE_PROPERTY_UPDATER(remoteIp, "remoteIp", const QString &)
DEFINE_PROPERTY_UPDATER(staticKey, "staticKey", const QString &)
DEFINE_PROPERTY_UPDATER_ENUM(staticKeyDirection, "staticKeyDirection", KeyDir)
DEFINE_PROPERTY_UPDATER(username, "username", const QString &)

// Advanced general properties

DEFINE_PROPERTY_UPDATER(port, "port", int)
DEFINE_PROPERTY_UPDATER(portSet, "portSet", bool)
DEFINE_PROPERTY_UPDATER(renegSeconds, "renegSeconds", int)
DEFINE_PROPERTY_UPDATER(renegSecondsSet, "renegSecondsSet", bool)
DEFINE_PROPERTY_UPDATER(compLzo, "compLzo", bool)
DEFINE_PROPERTY_UPDATER(protoTcp, "protoTcp", bool)
DEFINE_PROPERTY_UPDATER(dev, "dev", const QString &)
DEFINE_PROPERTY_UPDATER_ENUM(devType, "devType", DevType)
DEFINE_PROPERTY_UPDATER(devTypeSet, "devTypeSet", bool)
DEFINE_PROPERTY_UPDATER(tunnelMtu, "tunnelMtu", int)
DEFINE_PROPERTY_UPDATER(tunnelMtuSet, "tunnelMtuSet", bool)
DEFINE_PROPERTY_UPDATER(fragmentSize, "fragmentSize", int)
DEFINE_PROPERTY_UPDATER(fragmentSizeSet, "fragmentSizeSet", bool)
DEFINE_PROPERTY_UPDATER(mssFix, "mssFix", bool)
DEFINE_PROPERTY_UPDATER(remoteRandom, "remoteRandom", bool)

// Advanced security properties

DEFINE_PROPERTY_UPDATER_ENUM(cipher, "cipher", Cipher)
DEFINE_PROPERTY_UPDATER(keysize, "keysize", int)
DEFINE_PROPERTY_UPDATER(keysizeSet, "keysizeSet", bool)
DEFINE_PROPERTY_UPDATER_ENUM(auth, "auth", Auth)

// Advanced TLS auth properties

DEFINE_PROPERTY_UPDATER(tlsRemote, "tlsRemote", const QString &)
DEFINE_PROPERTY_UPDATER_ENUM(remoteCertTls, "remoteCertTls", TlsType)
DEFINE_PROPERTY_UPDATER(remoteCertTlsSet, "remoteCertTlsSet", bool)
DEFINE_PROPERTY_UPDATER(ta, "ta", const QString &)
DEFINE_PROPERTY_UPDATER_ENUM(taDir, "taDir", KeyDir)
DEFINE_PROPERTY_UPDATER(taSet, "taSet", bool)

// Advanced proxy settings

DEFINE_PROPERTY_UPDATER_ENUM(proxyType, "proxyType", ProxyType)
DEFINE_PROPERTY_UPDATER(proxyServer, "proxyServer", const QString &)
DEFINE_PROPERTY_UPDATER(proxyPort, "proxyPort", int)
DEFINE_PROPERTY_UPDATER(proxyRetry, "proxyRetry", bool)
DEFINE_PROPERTY_UPDATER(proxyUsername, "proxyUsername", const QString &)
DEFINE_PROPERTY_UPDATER(proxyPassword, "proxyPassword", const QString &)

}
