/*
 * Copyright (C) 2016 Canonical, Ltd.
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

#pragma once

#include <agent/CredentialStore.h>

#include <QList>
#include <QPair>
#include <QString>

namespace agent {

class KeyringCredentialStore: public CredentialStore {
public:
	KeyringCredentialStore();

	~KeyringCredentialStore();

	void save(const QString& uuid, const QString& settingName,
			const QString& settingKey, const QString& displayName,
			const QString& secret) override;

	QMap<QString, QString> get(const QString& uuid, const QString& settingName) override;

	void clear(const QString& uuid) override;
};

}
