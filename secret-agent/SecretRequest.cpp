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

#include <SecretRequest.h>
#include <SecretAgent.h>
#include <Localisation.h>

SecretRequest::SecretRequest(unsigned int requestId, SecretAgent &secretAgent,
		const QVariantDictMap &connection,
		const QDBusObjectPath &connectionPath, const QString &settingName,
		const QStringList &hints, uint flags, const QDBusMessage &message,
		QObject *parent) :
		QObject(parent), m_requestId(requestId), m_secretAgent(secretAgent), m_connection(
				connection), m_connectionPath(connectionPath), m_settingName(
				settingName), m_hints(hints), m_flags(flags), m_message(message) {

	connect(&m_secretAgent.notifications(),
			SIGNAL(ActionInvoked(uint, const QString &)), this,
			SLOT(actionInvoked(uint, const QString &)));

	Hints notificationHints;
	notificationHints["x-canonical-snap-decisions"] = QDBusVariant(true);
	notificationHints["x-canonical-private-button-tint"] = QDBusVariant(true);
	notificationHints["x-canonical-private-system-dialog"] = QDBusVariant(
			"wifi-auth");

	const QVariantMap &conn = connection[SecretAgent::CONNECTION_SETTING_NAME];

	QString ssid(_("Connect to \"%1\""));
	m_notificationId = m_secretAgent.notifications().Notify("indicator-network",
			0, "totem", ssid.arg(conn[SecretAgent::CONNECTION_ID].toString()),
			"dummy",
			QStringList() << "connect_id" << _("Connect") << "cancel_id"
					<< _("Cancel"), notificationHints, 0);
}

SecretRequest::~SecretRequest() {
}

void SecretRequest::actionInvoked(uint id, const QString &actionKey) {
	// Ignore other requests' notifications
	if (id != m_notificationId) {
		return;
	}

	QString key("");

	if (actionKey == "connect_id") {
		QStringList result(m_secretAgent.feedback().collect(m_notificationId));
		if (result.size() == 2) {
			key = result.at(1);
		}
	}

	auto wirelessSecurity = m_connection.find(m_settingName);
	QString keyMgmt(
			wirelessSecurity->value(SecretAgent::WIRELESS_SECURITY_KEY_MGMT).toString());

	if (keyMgmt == SecretAgent::KEY_MGMT_WPA_NONE
			|| keyMgmt == SecretAgent::KEY_MGMT_WPA_PSK) {
		wirelessSecurity->insert(SecretAgent::WIRELESS_SECURITY_PSK, key);
	} else if (keyMgmt == SecretAgent::KEY_MGMT_NONE) {
		wirelessSecurity->insert(SecretAgent::WIRELESS_SECURITY_WEP_KEY0, key);
	}

	m_secretAgent.FinishGetSecrets(*this);
}

unsigned int SecretRequest::requestId() const {
	return m_requestId;
}

const QVariantDictMap & SecretRequest::connection() const {
	return m_connection;
}

const QDBusMessage & SecretRequest::message() const {
	return m_message;
}
