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

#include <agent/KeyringCredentialStore.h>

#include <libsecret/secret.h>
#include <QDebug>

#define KEYRING_UUID_TAG "connection-uuid"
#define KEYRING_SN_TAG "setting-name"
#define KEYRING_SK_TAG "setting-key"

static const SecretSchema network_manager_secret_schema = {
	"org.freedesktop.NetworkManager.Connection",
	SECRET_SCHEMA_DONT_MATCH_NAME,
	{
		{KEYRING_UUID_TAG, SECRET_SCHEMA_ATTRIBUTE_STRING},
		{KEYRING_SN_TAG, SECRET_SCHEMA_ATTRIBUTE_STRING},
		{KEYRING_SK_TAG, SECRET_SCHEMA_ATTRIBUTE_STRING},
		{NULL, (SecretSchemaAttributeType) 0},
	},
	// The below junk prevents compilation warnings for
	// the uninitialised reserved values
	0,
	(gpointer) 0,
	(gpointer) 0,
	(gpointer) 0,
	(gpointer) 0,
	(gpointer) 0,
	(gpointer) 0,
	(gpointer) 0
};

using namespace std;

namespace agent {

KeyringCredentialStore::KeyringCredentialStore() {
}

KeyringCredentialStore::~KeyringCredentialStore() {
}

void KeyringCredentialStore::save(const QString& uuid,
		const QString& settingName, const QString& settingKey,
		const QString& displayName, const QString& secret) {
	shared_ptr<GHashTable> attrs(
			secret_attributes_build(&network_manager_secret_schema,
			KEYRING_UUID_TAG, uuid.toUtf8().constData(),
			KEYRING_SN_TAG, settingName.toUtf8().constData(),
			KEYRING_SK_TAG, settingKey.toUtf8().constData(),
			NULL), &g_hash_table_unref);

	GError* error = NULL;
	if (!secret_password_storev_sync(&network_manager_secret_schema,
			attrs.get(),
			NULL,
			displayName.toUtf8().constData(),
			secret.toUtf8().constData(),
			NULL, &error)) {
		QString message;
		if (error != NULL) {
			if (error->message) {
				message = QString::fromUtf8(error->message);
			}
			g_error_free(error);
		}
		qCritical() << message;
	}
}

QMap<QString, QString> KeyringCredentialStore::get(const QString& uuid, const QString& settingName) {
	QMap<QString, QString> result;

	shared_ptr<GHashTable> attrs(secret_attributes_build(
					&network_manager_secret_schema,
					KEYRING_UUID_TAG, uuid.toUtf8().constData(),
					KEYRING_SN_TAG, settingName.toUtf8().constData(),
					NULL), &g_hash_table_unref);

	GError* error = NULL;
	shared_ptr<GList> list(secret_service_search_sync(NULL,
			&network_manager_secret_schema, attrs.get(),
			(SecretSearchFlags) (SECRET_SEARCH_ALL | SECRET_SEARCH_UNLOCK
					| SECRET_SEARCH_LOAD_SECRETS), NULL, &error), [](GList* list) {
		g_list_free_full (list, g_object_unref);
	});

	if (list == NULL) {
		if (error != NULL) {
			string errorMessage;
			if (error->message) {
				errorMessage = error->message;
			}
			g_error_free(error);
			throw domain_error(errorMessage);
		}

		return result;
	}

	for (GList* iter = list.get(); iter != NULL; iter = g_list_next(iter)) {
		SecretItem *item = (SecretItem *) iter->data;
		shared_ptr<SecretValue> secret(secret_item_get_secret(item), &secret_value_unref);
		if (secret) {
			shared_ptr<GHashTable> attributes(secret_item_get_attributes(item), &g_hash_table_unref);
			const char *keyName = (const char *) g_hash_table_lookup(attributes.get(),
					KEYRING_SK_TAG);
			if (!keyName) {
				continue;
			}

			QString keyString = QString::fromUtf8(keyName);
			QString secretString = QString::fromUtf8(secret_value_get(secret.get(), NULL));

			result[keyString] = secretString;
		}
	}

	return result;
}

void KeyringCredentialStore::clear(const QString& uuid) {
	GError *error = NULL;
	if (!secret_password_clear_sync(&network_manager_secret_schema, NULL, &error,
						   KEYRING_UUID_TAG, uuid.toUtf8().constData(),
						   NULL)) {
		if (error != NULL) {
			QString message;
			if (error->message) {
				message = QString::fromUtf8(error->message);
			}
			g_error_free(error);
			qCritical() << message;
		}
	}
}

}
