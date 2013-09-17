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

#ifndef SECRETAGENT_H_
#define SECRETAGENT_H_

#include <QScopedPointer>
#include <QDBusConnection>
#include <QDBusContext>
#include <QMap>

#include <DBusTypes.h>
#include <SecretRequest.h>
#include <AgentManagerInterface.h>
#include <NotificationsInterface.h>

class SecretAgentAdaptor;

class SecretAgent: public QObject, protected QDBusContext {
Q_OBJECT

public:
	static const QString CONNECTION_SETTING_NAME;
	static const QString WIRELESS_SECURITY_SETTING_NAME;

	static const QString CONNECTION_ID;

	static const QString WIRELESS_SECURITY_PSK;
	static const QString WIRELESS_SECURITY_WEP_KEY0;

	static const QString WIRELESS_SECURITY_KEY_MGMT;

	static const QString KEY_MGMT_WPA_NONE;
	static const QString KEY_MGMT_WPA_PSK;
	static const QString KEY_MGMT_NONE;

	explicit SecretAgent(const QDBusConnection &systemConnection,
			const QDBusConnection &sessionConnection, QObject *parent = 0);

	virtual ~SecretAgent();

public Q_SLOTS:
	QVariantDictMap GetSecrets(const QVariantDictMap &connection,
			const QDBusObjectPath &connectionPath, const QString &settingName,
			const QStringList &hints, uint flags);

	void FinishGetSecrets(SecretRequest &request);

	void CancelGetSecrets(const QDBusObjectPath &connectionPath,
			const QString &settingName);

	void DeleteSecrets(const QVariantDictMap &connection,
			const QDBusObjectPath &connectionPath);

	void SaveSecrets(const QVariantDictMap &connection,
			const QDBusObjectPath &connectionPath);

	org::freedesktop::Notifications & notifications();

protected:
	QScopedPointer<SecretAgentAdaptor> m_adaptor;

	QDBusConnection m_systemConnection;

	QDBusConnection m_sessionConnection;

	org::freedesktop::NetworkManager::AgentManager m_agentManager;

	org::freedesktop::Notifications m_notifications;

	QMap<unsigned long long, SecretRequestPtr> m_requests;

	unsigned long long m_requestCounter;

};

#endif /* SECRETAGENT_H_ */
