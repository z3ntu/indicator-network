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

#ifndef SECRETREQUEST_H_
#define SECRETREQUEST_H_

#include <DBusTypes.h>

#include <QtDBus>
#include <QSharedPointer>

class SecretRequest;
class SecretAgent;

typedef QSharedPointer<SecretRequest> SecretRequestPtr;

class SecretRequest: public QObject {
Q_OBJECT
public:
	explicit SecretRequest(unsigned int requestId, SecretAgent &secretAgent,
			const QVariantDictMap &connection,
			const QDBusObjectPath &connectionPath, const QString &settingName,
			const QStringList &hints, uint flags, const QDBusMessage &reply,
			QObject *parent = 0);

	virtual ~SecretRequest();

public Q_SLOTS:
	void actionInvoked(uint id, const QString &actionKey);

public:
	unsigned int requestId() const;

	const QVariantDictMap & connection() const;

	const QDBusMessage & message() const;

protected:
	const unsigned int m_requestId;

	unsigned int m_notificationId;

	SecretAgent &m_secretAgent;

	QVariantDictMap m_connection;

	QDBusObjectPath m_connectionPath;

	QString m_settingName;

	QStringList m_hints;

	uint m_flags;

	QDBusMessage m_message;

	QTimer m_timer;
};

#endif /* SECRETREQUEST_H_ */
