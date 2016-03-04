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

#include <nmofono/vpn/pptp-connection.h>

#include <NetworkManagerSettingsConnectionInterface.h>

using namespace std;

#define DEFINE_UPDATE_SETTER(name, uppername, type)\
void set##uppername(type value)\
{\
    if (m_data.m_##name == value)\
    {\
        return;\
    }\
    m_data.m_##name = value;\
    Q_EMIT p.name##Changed(m_data.m_##name);\
}

#define DEFINE_UPDATE_PROPERTY_STRING(name, uppername, strname) \
void update##uppername(const QStringMap& data)\
{\
    set##uppername(data.value(strname));\
}\
\
DEFINE_UPDATE_SETTER(name, uppername, const QString &)

#define DEFINE_UPDATE_PROPERTY_ANTIBOOL(name, uppername, strname) \
void update##uppername(const QStringMap& data)\
{\
    set##uppername(data.value(strname) != "yes");\
}\
\
DEFINE_UPDATE_SETTER(name, uppername, bool)

#define DEFINE_UPDATE_PROPERTY_PAIR(name, uppername, strname, type, conversion)\
void update##uppername(const QStringMap& data)\
{\
    auto it = data.constFind(strname);\
    bool found = (it != data.constEnd());\
    set##uppername##Set(found);\
    if (found)\
    {\
        set##uppername(it->conversion);\
    }\
}\
\
DEFINE_UPDATE_SETTER(name, uppername, type)\
\
DEFINE_UPDATE_SETTER(name##Set, uppername##Set, bool)\

#define DEFINE_PROPERTY_GETTER(name,type) \
type PptpConnection::name() const\
{\
    return d->m_data.m_##name;\
}\

#define DEFINE_PROPERTY_SETTER(varname, uppername, type) \
void PptpConnection::set##uppername(type value)\
{\
    if (d->m_data.m_##varname == value)\
    {\
        return;\
    }\
    if (!d->m_dirty)\
    {\
        d->m_pendingData = d->m_data;\
    }\
    d->m_dirty = true;\
    d->m_pendingData.m_##varname = value;\
    Q_EMIT updateVpnData(d->m_pendingData.buildData());\
}

#define DEFINE_SECRET_PROPERTY_SETTER(varname, uppername, type) \
void PptpConnection::set##uppername(type value)\
{\
    if (d->m_data.m_##varname == value)\
    {\
        return;\
    }\
    if (!d->m_dirty)\
    {\
        d->m_pendingData = d->m_data;\
    }\
    d->m_dirty = true;\
    d->m_pendingData.m_##varname = value;\
    Q_EMIT updateVpnSecrets(d->m_pendingData.buildSecrets());\
}

namespace nmofono
{
namespace vpn
{

class PptpConnection::Priv
{
public:
    struct Data
    {
        Data()
        {
        }

        Data(const Data& other) = delete;

        Data& operator=(const Data& other)
        {
            // Basic properties

            m_gateway = other.m_gateway;
            m_user = other.m_user;
            m_password = other.m_password;
            m_domain = other.m_domain;

            // Advanced properties

            m_allowPap = other.m_allowPap;
            m_allowChap = other.m_allowChap;
            m_allowMschap = other.m_allowMschap;
            m_allowMschapv2 = other.m_allowMschapv2;
            m_allowEap = other.m_allowEap;
            m_requireMppe = other.m_requireMppe;
            m_mppeType = other.m_mppeType;
            m_mppeStateful = other.m_mppeStateful;
            m_bsdCompression = other.m_bsdCompression;
            m_deflateCompression = other.m_deflateCompression;
            m_tcpHeaderCompression = other.m_tcpHeaderCompression;
            m_sendPppEchoPackets = other.m_sendPppEchoPackets;

            return *this;
        }

        QStringMap buildSecrets()
        {
            QStringMap secrets;

            if (!m_password.isEmpty())
            {
                secrets["password"] = m_password;
            }

            return secrets;
        }

        QStringMap buildData()
        {
            QStringMap data;

            // Basic properties

            data["gateway"] = m_gateway;
            data["user"] = m_user;
            if (!m_domain.isEmpty())
            {
                data["domain"] = m_domain;
            }

            data["password-flags"] = "1";

            // Advanced properties

            static const QMap<MppeType, QString> mppeTypeMap
            {
                {MppeType::MPPE_ALL, "require-mppe"},
                {MppeType::MPPE_128, "require-mppe-128"},
                {MppeType::MPPE_40, "require-mppe-40"},
            };

            if (!m_requireMppe)
            {
                if (!m_allowPap)
                {
                    data["refuse-pap"] = "yes";
                }
                if (!m_allowChap)
                {
                    data["refuse-chap"] = "yes";
                }
            }
            if (!m_allowMschap)
            {
                data["refuse-mschap"] = "yes";
            }
            if (!m_allowMschapv2)
            {
                data["refuse-mschapv2"] = "yes";
            }
            if (!m_requireMppe)
            {
                if (!m_allowEap)
                {
                    data["refuse-eap"] = "yes";
                }
            }

            if ((m_allowMschap || m_allowMschapv2) && m_requireMppe)
            {
                data[mppeTypeMap[m_mppeType]] = "yes";
                if (m_mppeStateful)
                {
                    data["mppe-stateful"] = "yes";
                }
            }

            if (!m_bsdCompression)
            {
                data["nobsdcomp"] = "yes";
            }
            if (!m_deflateCompression)
            {
                data["nodeflate"] = "yes";
            }
            if (!m_tcpHeaderCompression)
            {
                data["no-vj-comp"] = "yes";
            }

            if (m_sendPppEchoPackets)
            {
                data["lcp-echo-interval"] = "30";
                data["lcp-echo-failure"] = "5";
            }

            return data;
        }

        // Basic properties

        QString m_gateway;
        QString m_user;
        QString m_password;
        QString m_domain;

        // Advanced properties

        bool m_allowPap = true;
        bool m_allowChap = true;
        bool m_allowMschap = true;
        bool m_allowMschapv2 = true;
        bool m_allowEap = true;
        bool m_requireMppe = false;
        MppeType m_mppeType = MppeType::MPPE_ALL;
        bool m_mppeStateful = false;
        bool m_bsdCompression = true;
        bool m_deflateCompression = true;
        bool m_tcpHeaderCompression = true;
        bool m_sendPppEchoPackets = false;
    };

    Priv(PptpConnection& parent) :
        p(parent)
    {
    }

    // Basic properties

    DEFINE_UPDATE_PROPERTY_STRING(gateway, Gateway, "gateway")
    DEFINE_UPDATE_PROPERTY_STRING(user, User, "user")
    DEFINE_UPDATE_PROPERTY_STRING(password, Password, "password")
    DEFINE_UPDATE_PROPERTY_STRING(domain, Domain, "domain")

    // Advanced properties

    DEFINE_UPDATE_PROPERTY_ANTIBOOL(allowPap, AllowPap, "refuse-pap")
    DEFINE_UPDATE_PROPERTY_ANTIBOOL(allowChap, AllowChap, "refuse-chap")
    DEFINE_UPDATE_PROPERTY_ANTIBOOL(allowMschap, AllowMschap, "refuse-mschap")
    DEFINE_UPDATE_PROPERTY_ANTIBOOL(allowMschapv2, AllowMschapv2, "refuse-mschapv2")
    DEFINE_UPDATE_PROPERTY_ANTIBOOL(allowEap, AllowEap, "refuse-eap")

    void updateMppe(const QStringMap& data)
    {
        bool requireMppe = false;
        MppeType mppeType = MppeType::MPPE_ALL;

        QString tmp = data.value("require-mppe");
        if (tmp == "yes")
        {
            requireMppe = true;
            mppeType = MppeType::MPPE_ALL;
        }
        else
        {
            tmp = data.value("require-mppe-128");
            if (tmp == "yes")
            {
                requireMppe = true;
                mppeType = MppeType::MPPE_128;
            }
            else
            {
                tmp = data.value("require-mppe-40");
                if (tmp == "yes")
                {
                    requireMppe = true;
                    mppeType = MppeType::MPPE_40;
                }
            }
        }

        setRequireMppe(requireMppe);
        if (requireMppe)
        {
            setMppeType(mppeType);
        }
        setMppeStateful(data.value("mppe-stateful") == "yes");
    }
    DEFINE_UPDATE_SETTER(requireMppe, RequireMppe, bool)
    DEFINE_UPDATE_SETTER(mppeType, MppeType, PptpConnection::MppeType)
    DEFINE_UPDATE_SETTER(mppeStateful, MppeStateful, bool)

    DEFINE_UPDATE_PROPERTY_ANTIBOOL(bsdCompression, BsdCompression, "nobsdcomp")
    DEFINE_UPDATE_PROPERTY_ANTIBOOL(deflateCompression, DeflateCompression, "nodeflate")
    DEFINE_UPDATE_PROPERTY_ANTIBOOL(tcpHeaderCompression, TcpHeaderCompression, "no-vj-comp")

    void updateSendPppEchoPackets(const QStringMap& data)
    {
        setSendPppEchoPackets(data.contains("lcp-echo-interval") || data.contains("lcp-echo-failure"));
    }
    DEFINE_UPDATE_SETTER(sendPppEchoPackets, SendPppEchoPackets, bool)

    PptpConnection& p;

    Data m_data;

    Data m_pendingData;

    bool m_dirty = false;
};

PptpConnection::PptpConnection() :
        d(new Priv(*this))
{
}

PptpConnection::~PptpConnection()
{
}

void PptpConnection::updateData(const QStringMap& data)
{
    // Basic properties

    d->updateGateway(data);
    d->updateUser(data);
    d->updateDomain(data);

    // Advanced properties

    d->updateAllowPap(data);
    d->updateAllowChap(data);
    d->updateAllowMschap(data);
    d->updateAllowMschapv2(data);
    d->updateAllowEap(data);
    d->updateMppe(data);
    d->updateBsdCompression(data);
    d->updateDeflateCompression(data);
    d->updateTcpHeaderCompression(data);
    d->updateSendPppEchoPackets(data);

}

void PptpConnection::updateSecrets(const QStringMap& secrets)
{
    d->updatePassword(secrets);
}

void PptpConnection::markClean()
{
    d->m_dirty = false;
}

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
DEFINE_PROPERTY_GETTER(mppeType, PptpConnection::MppeType)
DEFINE_PROPERTY_GETTER(mppeStateful, bool)
DEFINE_PROPERTY_GETTER(bsdCompression, bool)
DEFINE_PROPERTY_GETTER(deflateCompression, bool)
DEFINE_PROPERTY_GETTER(tcpHeaderCompression, bool)
DEFINE_PROPERTY_GETTER(sendPppEchoPackets, bool)



// Basic properties

DEFINE_PROPERTY_SETTER(gateway, Gateway, const QString &)
DEFINE_PROPERTY_SETTER(user, User, const QString &)
DEFINE_SECRET_PROPERTY_SETTER(password, Password, const QString &)
DEFINE_PROPERTY_SETTER(domain, Domain, const QString &)

// Advanced properties

DEFINE_PROPERTY_SETTER(allowPap, AllowPap, bool)
DEFINE_PROPERTY_SETTER(allowChap, AllowChap, bool)
DEFINE_PROPERTY_SETTER(allowMschap, AllowMschap, bool)
DEFINE_PROPERTY_SETTER(allowMschapv2, AllowMschapv2, bool)
DEFINE_PROPERTY_SETTER(allowEap, AllowEap, bool)
DEFINE_PROPERTY_SETTER(requireMppe, RequireMppe, bool)
DEFINE_PROPERTY_SETTER(mppeType, MppeType, MppeType)
DEFINE_PROPERTY_SETTER(mppeStateful, MppeStateful, bool)
DEFINE_PROPERTY_SETTER(bsdCompression, BsdCompression, bool)
DEFINE_PROPERTY_SETTER(deflateCompression, DeflateCompression, bool)
DEFINE_PROPERTY_SETTER(tcpHeaderCompression, TcpHeaderCompression, bool)
DEFINE_PROPERTY_SETTER(sendPppEchoPackets, SendPppEchoPackets, bool)

}
}
