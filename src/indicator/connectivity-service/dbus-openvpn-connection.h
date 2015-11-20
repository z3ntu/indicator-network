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

#pragma once

#include <connectivity-service/dbus-vpn-connection.h>
#include <nmofono/vpn/openvpn-connection.h>

class OpenVpnAdaptor;

namespace connectivity_service
{

class DBusOpenvpnConnection : public DBusVpnConnection
{
    friend OpenVpnAdaptor;

    Q_OBJECT

public:
    DBusOpenvpnConnection(nmofono::vpn::VpnConnection::SPtr vpnConnection, const QDBusConnection& connection);

    ~DBusOpenvpnConnection();

    nmofono::vpn::VpnConnection::Type type() const override;

    // Basic properties

    Q_PROPERTY(QString ca READ ca WRITE setCa)
    QString ca() const;

    Q_PROPERTY(QString cert READ cert WRITE setCert)
    QString cert() const;

    Q_PROPERTY(QString certPass READ certPass WRITE setCertPass)
    QString certPass() const;

    Q_PROPERTY(int connectionType READ connectionType WRITE setConnectionType)
    int connectionType() const;

    Q_PROPERTY(QString key READ key WRITE setKey)
    QString key() const;

    Q_PROPERTY(QString localIp READ localIp WRITE setLocalIp)
    QString localIp() const;

    Q_PROPERTY(QString password READ password WRITE setPassword)
    QString password() const;

    Q_PROPERTY(QString remote READ remote WRITE setRemote)
    QString remote() const;

    Q_PROPERTY(QString remoteIp READ remoteIp WRITE setRemoteIp)
    QString remoteIp() const;

    Q_PROPERTY(QString staticKey READ staticKey WRITE setStaticKey)
    QString staticKey() const;

    Q_PROPERTY(QString username READ username WRITE setUsername)
    QString username() const;

    // Advanced general properties

    Q_PROPERTY(int port READ port WRITE setPort)
    int port() const;

    Q_PROPERTY(bool portSet READ portSet WRITE setPortSet)
    bool portSet() const;

    Q_PROPERTY(int renegSeconds READ renegSeconds WRITE setRenegSeconds)
    int renegSeconds() const;

    Q_PROPERTY(bool renegSecondsSet READ renegSecondsSet WRITE setRenegSecondsSet)
    bool renegSecondsSet() const;

    Q_PROPERTY(bool compLzo READ compLzo WRITE setCompLzo)
    bool compLzo() const;

    Q_PROPERTY(bool protoTcp READ protoTcp WRITE setProtoTcp)
    bool protoTcp() const;

    Q_PROPERTY(QString dev READ dev WRITE setDev)
    QString dev() const;

    Q_PROPERTY(int devType READ devType WRITE setDevType)
    int devType() const;

    Q_PROPERTY(bool devTypeSet READ devTypeSet WRITE setDevTypeSet)
    bool devTypeSet() const;

    Q_PROPERTY(int tunnelMtu READ tunnelMtu WRITE setTunnelMtu)
    int tunnelMtu() const;

    Q_PROPERTY(bool tunnelMtuSet READ tunnelMtuSet WRITE setTunnelMtuSet)
    bool tunnelMtuSet() const;

    Q_PROPERTY(int fragmentSize READ fragmentSize WRITE setFragmentSize)
    int fragmentSize() const;

    Q_PROPERTY(bool fragmentSizeSet READ fragmentSizeSet WRITE setFragmentSizeSet)
    bool fragmentSizeSet() const;

    Q_PROPERTY(bool mssFix READ mssFix WRITE setMssFix)
    bool mssFix() const;

    Q_PROPERTY(bool remoteRandom READ remoteRandom WRITE setRemoteRandom)
    bool remoteRandom() const;

    // Advanced security properties

    Q_PROPERTY(int cipher READ cipher WRITE setCipher)
    int cipher() const;

    Q_PROPERTY(int keysize READ keysize WRITE setKeysize)
    int keysize() const;

    Q_PROPERTY(bool keysizeSet READ keysizeSet WRITE setKeysizeSet)
    bool keysizeSet() const;

    Q_PROPERTY(int auth READ auth WRITE setAuth)
    int auth() const;

    // Advanced TLS auth properties

    Q_PROPERTY(QString tlsRemote READ tlsRemote WRITE setTlsRemote)
    QString tlsRemote() const;

    Q_PROPERTY(int remoteCertTls READ remoteCertTls WRITE setRemoteCertTls)
    int remoteCertTls() const;

    Q_PROPERTY(bool remoteCertTlsSet READ remoteCertTlsSet WRITE setRemoteCertTlsSet)
    bool remoteCertTlsSet() const;

    Q_PROPERTY(QString ta READ ta WRITE setTa)
    QString ta() const;

    Q_PROPERTY(int taDir READ taDir WRITE setTaDir)
    int taDir() const;

    Q_PROPERTY(bool taSet READ taSet WRITE setTaSet)
    bool taSet() const;

    // Advanced proxy settings

    Q_PROPERTY(QString proxyPassword READ proxyPassword WRITE setProxyPassword)
    QString proxyPassword() const;

    Q_PROPERTY(int proxyPort READ proxyPort WRITE setProxyPort)
    int proxyPort() const;

    Q_PROPERTY(bool proxyRetry READ proxyRetry WRITE setProxyRetry)
    bool proxyRetry() const;

    Q_PROPERTY(QString proxyServer READ proxyServer WRITE setProxyServer)
    QString proxyServer() const;

    Q_PROPERTY(int proxyType READ proxyType WRITE setProxyType)
    int proxyType() const;

    Q_PROPERTY(QString proxyUsername READ proxyUsername WRITE setProxyUsername)
    QString proxyUsername() const;

protected:
    void notifyProperty(const QString& propertyName);

protected Q_SLOTS:
    // Enum properties
    void setConnectionType(int value);

    void setDevType(int value);

    void setCipher(int value);

    void setAuth(int value);

    void setRemoteCertTls(int value);

    void setProxyType(int value);

    // Basic properties

    void caUpdated(const QString &value);

    void certUpdated(const QString &value);

    void certPassUpdated(const QString &value);

    void connectionTypeUpdated(nmofono::vpn::OpenvpnConnection::ConnectionType value);

    void keyUpdated(const QString &value);

    void localIpUpdated(const QString &value);

    void passwordUpdated(const QString &value);

    void remoteUpdated(const QString &value);

    void remoteIpUpdated(const QString &value);

    void staticKeyUpdated(const QString &value);

    void usernameUpdated(const QString& value);

    // Advanced general properties

    void portUpdated(int value);

    void portSetUpdated(bool value);

    void renegSecondsUpdated(int value);

    void renegSecondsSetUpdated(bool value);

    void compLzoUpdated(bool value);

    void protoTcpUpdated(bool value);

    void devUpdated(const QString &value);

    void devTypeUpdated(nmofono::vpn::OpenvpnConnection::DevType value);

    void devTypeSetUpdated(bool value);

    void tunnelMtuUpdated(int value);

    void tunnelMtuSetUpdated(bool value);

    void fragmentSizeUpdated(int value);

    void fragmentSizeSetUpdated(bool value);

    void mssFixUpdated(bool value);

    void remoteRandomUpdated(bool value);

    // Advanced security properties

    void cipherUpdated(nmofono::vpn::OpenvpnConnection::Cipher value);

    void keysizeUpdated(int value);

    void keysizeSetUpdated(bool value);

    void authUpdated(nmofono::vpn::OpenvpnConnection::Auth value);

    // Advanced TLS auth properties

    void tlsRemoteUpdated(const QString &value);

    void remoteCertTlsUpdated(nmofono::vpn::OpenvpnConnection::TlsType value);

    void remoteCertTlsSetUpdated(bool value);

    void taUpdated(const QString &value);

    void taDirUpdated(nmofono::vpn::OpenvpnConnection::TaDir value);

    void taSetUpdated(bool value);

    // Advanced proxy tings

    void proxyTypeUpdated(nmofono::vpn::OpenvpnConnection::ProxyType value);

    void proxyServerUpdated(const QString &value);

    void proxyPortUpdated(int value);

    void proxyRetryUpdated(bool value);

    void proxyUsernameUpdated(const QString &value);

    void proxyPasswordUpdated(const QString &value);

Q_SIGNALS:
    // Basic properties

    void setCa(const QString &value);

    void setCert(const QString &value);

    void setCertPass(const QString &value);

    void setKey(const QString &value);

    void setLocalIp(const QString &value);

    void setPassword(const QString &value);

    void setRemote(const QString &value);

    void setRemoteIp(const QString &value);

    void setStaticKey(const QString &value);

    void setUsername(const QString &value);

    // Advanced general properties

    void setPort(int value);

    void setPortSet(bool value);

    void setRenegSeconds(int value);

    void setRenegSecondsSet(bool value);

    void setCompLzo(bool value);

    void setProtoTcp(bool value);

    void setDev(const QString &value);

    void setDevTypeSet(bool value);

    void setTunnelMtu(int value);

    void setTunnelMtuSet(bool value);

    void setFragmentSize(int value);

    void setFragmentSizeSet(bool value);

    void setMssFix(bool value);

    void setRemoteRandom(bool value);

    // Advanced security properties

    void setKeysize(int value);

    void setKeysizeSet(bool value);


    // Advanced TLS auth properties

    void setTlsRemote(const QString &value);

    void setRemoteCertTlsSet(bool value);

    void setTa(const QString &value);

    void setTaDir(int value);

    void setTaSet(bool value);

    // Advanced proxy settings

    void setProxyServer(const QString &value);

    void setProxyPort(int value);

    void setProxyRetry(bool value);

    void setProxyUsername(const QString &value);

    void setProxyPassword(const QString &value);

protected:
    nmofono::vpn::OpenvpnConnection::SPtr m_openvpnConnection;
};

}
