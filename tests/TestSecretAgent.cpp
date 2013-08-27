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

#include <libqtdbustest/DBusTestRunner.h>
#include <libqtdbusmock/DBusMock.h>
#include <SecretAgent.h>
#include <SecretAgentInterface.h>
#include <NetworkManager.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace std;
using namespace testing;
using namespace QtDBusTest;
using namespace QtDBusMock;

namespace {

class TestSecretAgentCommon {
protected:
	TestSecretAgentCommon() :
			dbusMock(dbusTestRunner) {

		dbusMock.registerNetworkManager();
		dbusTestRunner.startServices();

		secretAgent.start(SECRET_AGENT_BIN, QStringList() << "--print-address");
		secretAgent.waitForStarted();
		secretAgent.waitForReadyRead();
		agentBus = secretAgent.readAll().trimmed();

		interface.reset(
				new OrgFreedesktopNetworkManagerSecretAgentInterface(agentBus,
						NM_DBUS_PATH_SECRET_AGENT,
						dbusTestRunner.systemConnection()));
	}

	virtual ~TestSecretAgentCommon() {
		secretAgent.terminate();
		secretAgent.waitForFinished();
	}

	QVariantDictMap connection(const QString &keyManagement) {
		QVariantMap wirelessSecurity;
		wirelessSecurity[SecretAgent::WIRELESS_SECURITY_KEY_MGMT] =
				keyManagement;

		QVariantDictMap connection;
		connection[SecretAgent::WIRELESS_SECURITY_SETTING_NAME] =
				wirelessSecurity;

		return connection;
	}

	QVariantDictMap expected(const QString &keyManagement,
			const QString &keyName, const QString &password) {

		QVariantMap wirelessSecurity;
		wirelessSecurity[SecretAgent::WIRELESS_SECURITY_KEY_MGMT] =
				keyManagement;
		wirelessSecurity[keyName] = password;

		QVariantDictMap connection;
		connection[SecretAgent::WIRELESS_SECURITY_SETTING_NAME] =
				wirelessSecurity;

		return connection;
	}

	DBusTestRunner dbusTestRunner;

	DBusMock dbusMock;

	QProcess secretAgent;

	QString agentBus;

	QScopedPointer<OrgFreedesktopNetworkManagerSecretAgentInterface> interface;
};

struct TestSecretAgentParams {
	QString keyManagement;

	QString passwordKey;

	QString password;
};

class TestSecretAgentGetSecrets: public TestSecretAgentCommon,
		public TestWithParam<TestSecretAgentParams> {
};

TEST_P(TestSecretAgentGetSecrets, ProvidesPasswordForWpaPsk) {
	QVariantDictMap reply(
			interface->GetSecrets(connection(GetParam().keyManagement),
					QDBusObjectPath("/connection/foo"),
					SecretAgent::WIRELESS_SECURITY_SETTING_NAME, QStringList(),
					5));

	EXPECT_EQ(
			expected(GetParam().keyManagement, GetParam().passwordKey,
					GetParam().password), reply);
}

INSTANTIATE_TEST_CASE_P(WpaPsk, TestSecretAgentGetSecrets,
		Values(TestSecretAgentParams( { SecretAgent::KEY_MGMT_WPA_PSK,
				SecretAgent::WIRELESS_SECURITY_PSK, "hard-coded-password" })));

INSTANTIATE_TEST_CASE_P(WpaNone, TestSecretAgentGetSecrets,
		Values(TestSecretAgentParams( { SecretAgent::KEY_MGMT_WPA_NONE,
				SecretAgent::WIRELESS_SECURITY_PSK, "hard-coded-password" })));

INSTANTIATE_TEST_CASE_P(None, TestSecretAgentGetSecrets,
		Values(
				TestSecretAgentParams( { SecretAgent::KEY_MGMT_NONE,
						SecretAgent::WIRELESS_SECURITY_WEP_KEY0,
						"hard-coded-password" })));

class TestSecretAgent: public TestSecretAgentCommon, public Test {
};

TEST_F(TestSecretAgent, GetSecretsWithNone) {
	QDBusPendingReply<QVariantDictMap> reply(
			interface->GetSecrets(connection(SecretAgent::KEY_MGMT_WPA_PSK),
					QDBusObjectPath("/connection/foo"),
					SecretAgent::WIRELESS_SECURITY_SETTING_NAME, QStringList(),
					0));
	reply.waitForFinished();

	ASSERT_TRUE(reply.isError());
	EXPECT_EQ(QDBusError::InternalError, reply.error().type());
	EXPECT_EQ("No password found for this connection.",
			reply.error().message());
}

TEST_F(TestSecretAgent, CancelGetSecrets) {
	interface->CancelGetSecrets(QDBusObjectPath("/connection/foo"),
			SecretAgent::WIRELESS_SECURITY_SETTING_NAME).waitForFinished();
}

TEST_F(TestSecretAgent, SaveSecrets) {
	interface->SaveSecrets(QVariantDictMap(),
			QDBusObjectPath("/connection/foo")).waitForFinished();
}

TEST_F(TestSecretAgent, DeleteSecrets) {
	interface->DeleteSecrets(QVariantDictMap(),
			QDBusObjectPath("/connection/foo")).waitForFinished();
}

} // namespace
