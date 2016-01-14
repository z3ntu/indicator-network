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
#include <nmofono/vpn/pptp-connection.h>

class PptpAdaptor;

namespace connectivity_service
{

class DBusPptpConnection : public DBusVpnConnection
{
    friend PptpAdaptor;

    Q_OBJECT

public:
    DBusPptpConnection(nmofono::vpn::VpnConnection::SPtr vpnConnection, const QDBusConnection& connection);

    ~DBusPptpConnection();

    nmofono::vpn::VpnConnection::Type type() const override;

    // Basic properties

    Q_PROPERTY(QString gateway READ gateway WRITE setGateway)
    QString gateway() const;

    Q_PROPERTY(QString user READ user WRITE setUser)
    QString user() const;

    Q_PROPERTY(QString password READ password WRITE setPassword)
    QString password() const;

    Q_PROPERTY(QString domain READ domain WRITE setDomain)
    QString domain() const;

    // Advanced properties

    Q_PROPERTY(bool allowPap READ allowPap WRITE setAllowPap)
    bool allowPap() const;

    Q_PROPERTY(bool allowChap READ allowChap WRITE setAllowChap)
    bool allowChap() const;

    Q_PROPERTY(bool allowMschap READ allowMschap WRITE setAllowMschap)
    bool allowMschap() const;

    Q_PROPERTY(bool allowMschapv2 READ allowMschapv2 WRITE setAllowMschapv2)
    bool allowMschapv2() const;

    Q_PROPERTY(bool allowEap READ allowEap WRITE setAllowEap)
    bool allowEap() const;

    Q_PROPERTY(bool requireMppe READ requireMppe WRITE setRequireMppe)
    bool requireMppe() const;

    Q_PROPERTY(int mppeType READ mppeType WRITE setMppeType)
    int mppeType() const;

    Q_PROPERTY(bool mppeStateful READ mppeStateful WRITE setMppeStateful)
    bool mppeStateful() const;

    Q_PROPERTY(bool bsdCompression READ bsdCompression WRITE setBsdCompression)
    bool bsdCompression() const;

    Q_PROPERTY(bool deflateCompression READ deflateCompression WRITE setDeflateCompression)
    bool deflateCompression() const;

    Q_PROPERTY(bool tcpHeaderCompression READ tcpHeaderCompression WRITE setTcpHeaderCompression)
    bool tcpHeaderCompression() const;

    Q_PROPERTY(bool sendPppEchoPackets READ sendPppEchoPackets WRITE setSendPppEchoPackets)
    bool sendPppEchoPackets() const;

protected:
    void notifyProperty(const QString& propertyName);

protected Q_SLOTS:
    // Enum properties
    void setMppeType(int value);

    // Basic properties

    void gatewayUpdated(const QString &value);

    void userUpdated(const QString &value);

    void passwordUpdated(const QString &value);

    void domainUpdated(const QString &value);

    // Advanced properties

    void allowPapUpdated(bool value);

    void allowChapUpdated(bool value);

    void allowMschapUpdated(bool value);

    void allowMschapv2Updated(bool value);

    void allowEapUpdated(bool value);

    void requireMppeUpdated(bool value);

    void mppeTypeUpdated(nmofono::vpn::PptpConnection::MppeType value);

    void mppeStatefulUpdated(bool value);

    void bsdCompressionUpdated(bool value);

    void deflateCompressionUpdated(bool value);

    void tcpHeaderCompressionUpdated(bool value);

    void sendPppEchoPacketsUpdated(bool value);

Q_SIGNALS:
    // Basic properties

    void setGateway(const QString &value);

    void setUser(const QString &value);

    void setPassword(const QString &value);

    void setDomain(const QString &value);

    // Advanced properties

    void setAllowPap(bool value);

    void setAllowChap(bool value);

    void setAllowMschap(bool value);

    void setAllowMschapv2(bool value);

    void setAllowEap(bool value);

    void setRequireMppe(bool value);

    void setMppeStateful(bool value);

    void setBsdCompression(bool value);

    void setDeflateCompression(bool value);

    void setTcpHeaderCompression(bool value);

    void setSendPppEchoPackets(bool value);

protected:
    nmofono::vpn::PptpConnection::SPtr m_pptpConnection;
};

}
