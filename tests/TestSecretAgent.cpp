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

#include <QSignalSpy>
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

		dbusMock.registerCustomMock("org.freedesktop.Notifications",
				"/org/freedesktop/Notifications",
				OrgFreedesktopNotificationsInterface::staticInterfaceName(),
				QDBusConnection::SessionBus);

		dbusMock.registerCustomMock("com.canonical.snapdecisions.feedback",
				"/com/canonical/snapdecisions/feedback",
				ComCanonicalSnapdecisionsFeedbackInterface::staticInterfaceName(),
				QDBusConnection::SessionBus);

		dbusMock.registerNetworkManager();
		dbusTestRunner.startServices();

		secretAgent.start(SECRET_AGENT_BIN, QStringList() << "--print-address");
		secretAgent.waitForStarted();
		secretAgent.waitForReadyRead();
		agentBus = secretAgent.readAll().trimmed();

		agentInterface.reset(
				new OrgFreedesktopNetworkManagerSecretAgentInterface(agentBus,
						NM_DBUS_PATH_SECRET_AGENT,
						dbusTestRunner.systemConnection()));

		notificationsInterface.reset(
				new OrgFreedesktopDBusMockInterface(
						"org.freedesktop.Notifications",
						"/org/freedesktop/Notifications",
						dbusTestRunner.sessionConnection()));

		feedbackInterface.reset(
				new OrgFreedesktopDBusMockInterface(
						"com.canonical.snapdecisions.feedback",
						"/com/canonical/snapdecisions/feedback",
						dbusTestRunner.sessionConnection()));
	}

	virtual ~TestSecretAgentCommon() {
		secretAgent.terminate();
		secretAgent.waitForFinished();
	}

	QVariantDictMap connection(const QString &keyManagement) {
		QVariantMap wirelessSecurity;
		wirelessSecurity[SecretAgent::WIRELESS_SECURITY_KEY_MGMT] =
				keyManagement;

		QVariantMap conn;
		conn[SecretAgent::CONNECTION_ID] = "the ssid";

		QVariantDictMap connection;
		connection[SecretAgent::WIRELESS_SECURITY_SETTING_NAME] =
				wirelessSecurity;
		connection[SecretAgent::CONNECTION_SETTING_NAME] = conn;

		return connection;
	}

	QVariantDictMap expected(const QString &keyManagement,
			const QString &keyName, const QString &password) {

		QVariantMap wirelessSecurity;
		wirelessSecurity[SecretAgent::WIRELESS_SECURITY_KEY_MGMT] =
				keyManagement;
		wirelessSecurity[keyName] = password;

		QVariantMap conn;
		conn[SecretAgent::CONNECTION_ID] = "the ssid";

		QVariantDictMap connection;
		connection[SecretAgent::WIRELESS_SECURITY_SETTING_NAME] =
				wirelessSecurity;
		connection[SecretAgent::CONNECTION_SETTING_NAME] = conn;

		return connection;
	}

	DBusTestRunner dbusTestRunner;

	DBusMock dbusMock;

	QProcess secretAgent;

	QString agentBus;

	QScopedPointer<OrgFreedesktopNetworkManagerSecretAgentInterface> agentInterface;

	QScopedPointer<OrgFreedesktopDBusMockInterface> notificationsInterface;

	QScopedPointer<OrgFreedesktopDBusMockInterface> feedbackInterface;
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
	notificationsInterface->AddMethod(
			OrgFreedesktopNotificationsInterface::staticInterfaceName(),
			"Notify", "susssasa{sv}i", "u", "ret = 1").waitForFinished();

	feedbackInterface->AddMethod(
			ComCanonicalSnapdecisionsFeedbackInterface::staticInterfaceName(),
			"collect", "u", "as", "ret = ['password', 'hard-coded-password']").waitForFinished();

	QDBusPendingReply<QVariantDictMap> reply(
			agentInterface->GetSecrets(connection(GetParam().keyManagement),
					QDBusObjectPath("/connection/foo"),
					SecretAgent::WIRELESS_SECURITY_SETTING_NAME, QStringList(),
					5));

	QSignalSpy notificationSpy(notificationsInterface.data(),
			SIGNAL(MethodCalled(const QString &, const QVariantList &)));
	notificationSpy.wait();

	ASSERT_EQ(1, notificationSpy.size());
	const QVariantList &call(notificationSpy.at(0));
	ASSERT_EQ("Notify", call.at(0));

	const QVariantList &args(call.at(1).toList());
	ASSERT_EQ(8, args.size());
	EXPECT_EQ("indicator-network", args.at(0));
	EXPECT_EQ("Connect to \"the ssid\"", args.at(3).toString().toStdString());

	notificationsInterface->EmitSignal(
			OrgFreedesktopNotificationsInterface::staticInterfaceName(),
			"ActionInvoked", "us", QVariantList() << 1 << "connect_id");

	QVariantDictMap result(reply);

	EXPECT_EQ(
			expected(GetParam().keyManagement, GetParam().passwordKey,
					GetParam().password), result);
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
			agentInterface->GetSecrets(
					connection(SecretAgent::KEY_MGMT_WPA_PSK),
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
	agentInterface->CancelGetSecrets(QDBusObjectPath("/connection/foo"),
			SecretAgent::WIRELESS_SECURITY_SETTING_NAME).waitForFinished();
}

TEST_F(TestSecretAgent, SaveSecrets) {
	agentInterface->SaveSecrets(QVariantDictMap(),
			QDBusObjectPath("/connection/foo")).waitForFinished();
}

TEST_F(TestSecretAgent, DeleteSecrets) {
	agentInterface->DeleteSecrets(QVariantDictMap(),
			QDBusObjectPath("/connection/foo")).waitForFinished();
}

} // namespace
