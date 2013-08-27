/*
 * Copyright (C) 2013 Canonical, Ltd.
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
 * Author: Pete Woods <pete.woods@canonical.com>
 */

#include <Localisation.h>
#include <SecretAgent.h>
#include <SecretAgentAdaptor.h>

#include <NetworkManager.h>

#include <stdexcept>

using namespace std;
using namespace org::freedesktop::NetworkManager;

const QString SecretAgent::WIRELESS_SECURITY_SETTING_NAME(
		"802-11-wireless-security");

const QString SecretAgent::WIRELESS_SECURITY_PSK("psk");
const QString SecretAgent::WIRELESS_SECURITY_WEP_KEY0("wep-key0");

const QString SecretAgent::WIRELESS_SECURITY_KEY_MGMT("key-mgmt");

const QString SecretAgent::KEY_MGMT_WPA_NONE("wpa-none");
const QString SecretAgent::KEY_MGMT_WPA_PSK("wpa-psk");
const QString SecretAgent::KEY_MGMT_NONE("none");

SecretAgent::SecretAgent(const QDBusConnection &connection, QObject *parent) :
		QObject(parent), m_adaptor(new SecretAgentAdaptor(this)), m_connection(
				connection), m_agentManager(NM_DBUS_SERVICE,
				NM_DBUS_PATH_AGENT_MANAGER, m_connection), m_requestCounter(0) {
	if (!m_connection.registerObject(NM_DBUS_PATH_SECRET_AGENT, this)) {
		throw logic_error(
				_("Unable to register user secret agent object on DBus"));
	}

	m_agentManager.Register("com.canonical.indicator.SecretAgent").waitForFinished();
}

SecretAgent::~SecretAgent() {
	m_agentManager.Unregister().waitForFinished();
	m_connection.unregisterObject(NM_DBUS_PATH_SECRET_AGENT);
}

/**
 * Example call:
 * [Argument: a{sa{sv}}
 *   {
 *     "802-11-wireless" = [Argument: a{sv} {
 *       "security" = [Variant(QString): "802-11-wireless-security"],
 *       "ssid" = [Variant(QByteArray): {83, 119, 97, 108, 108, 111, 119, 115, 32, 66, 97, 114, 110}],
 *       "mode" = [Variant(QString): "infrastructure"],
 *       "mac-address" = [Variant(QByteArray): {8, 212, 43, 19, 139, 130}]
 *     }],
 *     "connection" = [Argument: a{sv} {
 *       "id" = [Variant(QString): "Swallows Barn"],
 *       "uuid" = [Variant(QString): "40fdd8b6-e119-41ae-87a3-7bfc8044f753"],
 *       "type" = [Variant(QString): "802-11-wireless"]
 *     }],
 *     "ipv4" = [Argument: a{sv} {
 *       "addresses" = [Variant: [Argument: aau {}]],
 *       "dns" = [Variant: [Argument: au {}]],
 *       "method" = [Variant(QString): "auto"],
 *       "routes" = [Variant: [Argument: aau {}]]
 *     }],
 *     "802-11-wireless-security" = [Argument: a{sv} {
 *       "auth-alg" = [Variant(QString): "open"],
 *       "key-mgmt" = [Variant(QString): "wpa-psk"]
 *     }],
 *     "ipv6" = [Argument: a{sv} {
 *       "addresses" = [Variant: [Argument: a(ayuay) {}]],
 *       "dns" = [Variant: [Argument: aay {}]],
 *       "method" = [Variant(QString): "auto"],
 *       "routes" = [Variant: [Argument: a(ayuayu) {}]]
 *     }]
 *   }
 * ],
 * [ObjectPath: /org/freedesktop/NetworkManager/Settings/0],
 * "802-11-wireless-security",
 * {},
 * 5
 */
QVariantDictMap SecretAgent::GetSecrets(const QVariantDictMap &connection,
		const QDBusObjectPath &connectionPath, const QString &settingName,
		const QStringList &hints, uint flags) {

	setDelayedReply(true);

	if (flags == 0) {
		m_connection.send(
				message().createErrorReply(QDBusError::InternalError,
						"No password found for this connection."));
	} else {
		SecretRequestPtr request(
				new SecretRequest(m_requestCounter, *this, connection,
						connectionPath, settingName, hints, flags, message()));
		m_requests.insert(m_requestCounter, request);

		++m_requestCounter;
	}

	return QVariantDictMap();
}

void SecretAgent::FinishGetSecrets(SecretRequest &request) {
	m_connection.send(
			request.message().createReply(
					QVariant::fromValue(request.connection())));
	m_requests.remove(request.requestId());
}

void SecretAgent::CancelGetSecrets(const QDBusObjectPath &connectionPath,
		const QString &settingName) {
}

void SecretAgent::DeleteSecrets(const QVariantDictMap &connection,
		const QDBusObjectPath &connectionPath) {
}

void SecretAgent::SaveSecrets(const QVariantDictMap &connection,
		const QDBusObjectPath &connectionPath) {
}
