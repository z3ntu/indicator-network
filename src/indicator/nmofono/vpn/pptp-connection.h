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

#include <QObject>
#include <QVariantMap>

#include <unity/util/DefinesPtrs.h>

class OrgFreedesktopNetworkManagerSettingsConnectionInterface;

namespace nmofono
{
namespace vpn
{

class PptpConnection : public QObject
{
    Q_OBJECT

public:
    UNITY_DEFINES_PTRS(PptpConnection);

    enum class MppeType
    {
        MPPE_ALL,
        MPPE_128,
        MPPE_40
    };

    PptpConnection();

    ~PptpConnection();

    // Basic properties

    Q_PROPERTY(QString gateway READ gateway WRITE setGateway NOTIFY gatewayChanged)
    QString gateway() const;

    Q_PROPERTY(QString user READ user WRITE setUser NOTIFY userChanged)
    QString user() const;

    Q_PROPERTY(QString password READ password WRITE setPassword NOTIFY passwordChanged)
    QString password() const;

    Q_PROPERTY(QString domain READ domain WRITE setDomain NOTIFY domainChanged)
    QString domain() const;

    // Advanced properties

    Q_PROPERTY(bool allowPap READ allowPap WRITE setAllowPap NOTIFY allowPapChanged)
    bool allowPap() const;

    Q_PROPERTY(bool allowChap READ allowChap WRITE setAllowChap NOTIFY allowChapChanged)
    bool allowChap() const;

    Q_PROPERTY(bool allowMschap READ allowMschap WRITE setAllowMschap NOTIFY allowMschapChanged)
    bool allowMschap() const;

    Q_PROPERTY(bool allowMschapv2 READ allowMschapv2 WRITE setAllowMschapv2 NOTIFY allowMschapv2Changed)
    bool allowMschapv2() const;

    Q_PROPERTY(bool allowEap READ allowEap WRITE setAllowEap NOTIFY allowEapChanged)
    bool allowEap() const;

    Q_PROPERTY(bool requireMppe READ requireMppe WRITE setRequireMppe NOTIFY requireMppeChanged)
    bool requireMppe() const;

    Q_PROPERTY(MppeType mppeType READ mppeType WRITE setMppeType NOTIFY mppeTypeChanged)
    MppeType mppeType() const;

    Q_PROPERTY(bool mppeStateful READ mppeStateful WRITE setMppeStateful NOTIFY mppeStatefulChanged)
    bool mppeStateful() const;

    Q_PROPERTY(bool bsdCompression READ bsdCompression WRITE setBsdCompression NOTIFY bsdCompressionChanged)
    bool bsdCompression() const;

    Q_PROPERTY(bool deflateCompression READ deflateCompression WRITE setDeflateCompression NOTIFY deflateCompressionChanged)
    bool deflateCompression() const;

    Q_PROPERTY(bool tcpHeaderCompression READ tcpHeaderCompression WRITE setTcpHeaderCompression NOTIFY tcpHeaderCompressionChanged)
    bool tcpHeaderCompression() const;

    Q_PROPERTY(bool sendPppEchoPackets READ sendPppEchoPackets WRITE setSendPppEchoPackets NOTIFY sendPppEchoPacketsChanged)
    bool sendPppEchoPackets() const;

public Q_SLOTS:
    void updateData(const QMap<QString, QString>& data);

    void updateSecrets(const QMap<QString, QString>& data);

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

    void setMppeType(MppeType value);

    void setMppeStateful(bool value);

    void setBsdCompression(bool value);

    void setDeflateCompression(bool value);

    void setTcpHeaderCompression(bool value);

    void setSendPppEchoPackets(bool value);

Q_SIGNALS:
    void updateVpnData(const QMap<QString, QString>& vpnData);

    void updateVpnSecrets(const QMap<QString, QString>& vpnSecrets);

    // Basic properties

    void gatewayChanged(const QString &value);

    void userChanged(const QString &value);

    void passwordChanged(const QString &value);

    void domainChanged(const QString &value);

    // Advanced properties

    void allowPapChanged(bool value);

    void allowChapChanged(bool value);

    void allowMschapChanged(bool value);

    void allowMschapv2Changed(bool value);

    void allowEapChanged(bool value);

    void requireMppeChanged(bool value);

    void mppeTypeChanged(MppeType value);

    void mppeStatefulChanged(bool value);

    void bsdCompressionChanged(bool value);

    void deflateCompressionChanged(bool value);

    void tcpHeaderCompressionChanged(bool value);

    void sendPppEchoPacketsChanged(bool value);

protected:
    class Priv;
    std::shared_ptr<Priv> d;
};

}
}
