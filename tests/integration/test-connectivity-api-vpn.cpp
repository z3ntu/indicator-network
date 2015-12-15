
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
 * Author: Pete Woods <pete.woods@canonical.com>
 */

#include <connectivityqt/openvpn-connection.h>
#include <connectivityqt/vpn-connections-list-model.h>
#include <indicator-network-test-base.h>
#include <dbus-types.h>
#include <NetworkManagerInterface.h>
#include <NetworkManagerSettingsInterface.h>
#include <NetworkManagerSettingsConnectionInterface.h>

#include <QDebug>
#include <QSortFilterProxyModel>
#include <QTestEventLoop>

using namespace std;
using namespace testing;
using namespace connectivityqt;

typedef QPair<QString, QVector<bool>> CS;
typedef QList<CS> CSL;

inline void PrintTo(const CS& list, std::ostream* os) {
    QString output;
    QDebug dbg(&output);
    dbg << list;

    *os << "CSL(" << output.toStdString() << ")";
}

namespace
{

class TestConnectivityApiVpn: public IndicatorNetworkTestBase
{
protected:
    CSL vpnList(QAbstractItemModel& model)
    {
        CSL connectionStates;
        int rowCount(model.rowCount());
        for (int i = 0; i < rowCount; ++i)
        {
            auto idx = model.index(i, 0);
            CS connectionState;
            connectionState.first = model.data(idx, VpnConnectionsListModel::Roles::RoleId).toString();
            connectionState.second << model.data(idx, VpnConnectionsListModel::Roles::RoleActive).toBool();
            connectionState.second << model.data(idx, VpnConnectionsListModel::Roles::RoleActivatable).toBool();
            connectionStates << connectionState;
        }
        return connectionStates;
    }

    unique_ptr<QSortFilterProxyModel> getSortedVpnConnections(Connectivity& connectivity)
    {
        auto vpnConnections = connectivity.vpnConnections();

        auto sortedVpnConnections = make_unique<QSortFilterProxyModel>();
        sortedVpnConnections->setSortRole(VpnConnectionsListModel::Roles::RoleId);
        sortedVpnConnections->sort(0);

        sortedVpnConnections->setSourceModel(vpnConnections);

        return sortedVpnConnections;
    }

    OpenvpnConnection* getOpenvpnConnection(QAbstractItemModel* vpnConnections, int i)
    {
        return qobject_cast<OpenvpnConnection*>(
                vpnConnections->data(
                        vpnConnections->index(i, 0),
                        VpnConnectionsListModel::Roles::RoleConnection).value<
                        QObject*>());
    }
};

TEST_F(TestConnectivityApiVpn, VpnListStartsEmpty)
{
    // Add a physical device to use for the connection
    setGlobalConnectedState(NM_STATE_CONNECTED_GLOBAL);
    auto device = createWiFiDevice(NM_DEVICE_STATE_ACTIVATED);

    // Start the indicator
    ASSERT_NO_THROW(startIndicator());

    // Connect the the service
    auto connectivity(newConnectivity());

    auto sortedVpnConnections = getSortedVpnConnections(*connectivity);

    QSignalSpy rowsAboutToBeRemovedSpy(sortedVpnConnections.get(), SIGNAL(rowsAboutToBeRemoved(const QModelIndex &, int, int)));
    QSignalSpy rowsRemovedSpy(sortedVpnConnections.get(), SIGNAL(rowsRemoved(const QModelIndex &, int, int)));
    QSignalSpy rowsAboutToBeInsertedSpy(sortedVpnConnections.get(), SIGNAL(rowsAboutToBeInserted(const QModelIndex &, int, int)));
    QSignalSpy rowsInsertedSpy(sortedVpnConnections.get(), SIGNAL(rowsInserted(const QModelIndex &, int, int)));
    QSignalSpy dataChangedSpy(sortedVpnConnections.get(), SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &, const QVector<int> &)));

    EXPECT_EQ(CSL(), vpnList(*sortedVpnConnections));

    rowsAboutToBeRemovedSpy.clear();
    rowsRemovedSpy.clear();
    rowsAboutToBeInsertedSpy.clear();
    rowsInsertedSpy.clear();
    dataChangedSpy.clear();

    // Add VPN configuration
    auto appleConnection = createVpnConnection("apple");

    WAIT_FOR_SIGNALS(rowsAboutToBeInsertedSpy, 1);
    WAIT_FOR_SIGNALS(rowsInsertedSpy, 1);
    EXPECT_TRUE(rowsAboutToBeRemovedSpy.isEmpty());
    EXPECT_TRUE(rowsRemovedSpy.isEmpty());
    EXPECT_TRUE(dataChangedSpy.isEmpty());

    EXPECT_EQ(CSL({{"apple", {false, true}}}), vpnList(*sortedVpnConnections));

    rowsAboutToBeRemovedSpy.clear();
    rowsRemovedSpy.clear();
    rowsAboutToBeInsertedSpy.clear();
    rowsInsertedSpy.clear();
    dataChangedSpy.clear();

    // Delete the only VPN connection
    deleteSettings(appleConnection);

    WAIT_FOR_SIGNALS(rowsAboutToBeRemovedSpy, 1);
    WAIT_FOR_SIGNALS(rowsRemovedSpy, 1);
    EXPECT_TRUE(rowsAboutToBeInsertedSpy.isEmpty());
    EXPECT_TRUE(rowsInsertedSpy.isEmpty());
    EXPECT_TRUE(dataChangedSpy.isEmpty());

    // List should be empty again
    EXPECT_EQ(CSL(), vpnList(*sortedVpnConnections));
}

TEST_F(TestConnectivityApiVpn, VpnListStartsPopulated)
{
    // Add VPN configurations
    auto appleConnection = createVpnConnection("apple");
    auto bananaConnection = createVpnConnection("banana");
    auto coconutConnection = createVpnConnection("coconut");

    // Add a physical device to use for the connection
    setGlobalConnectedState(NM_STATE_CONNECTED_GLOBAL);
    auto device = createWiFiDevice(NM_DEVICE_STATE_ACTIVATED);

    // Start the indicator
    ASSERT_NO_THROW(startIndicator());

    // Connect the the service
    auto connectivity(newConnectivity());

    auto sortedVpnConnections = getSortedVpnConnections(*connectivity);

    QSignalSpy rowsAboutToBeRemovedSpy(sortedVpnConnections.get(), SIGNAL(rowsAboutToBeRemoved(const QModelIndex &, int, int)));
    QSignalSpy rowsRemovedSpy(sortedVpnConnections.get(), SIGNAL(rowsRemoved(const QModelIndex &, int, int)));
    QSignalSpy rowsAboutToBeInsertedSpy(sortedVpnConnections.get(), SIGNAL(rowsAboutToBeInserted(const QModelIndex &, int, int)));
    QSignalSpy rowsInsertedSpy(sortedVpnConnections.get(), SIGNAL(rowsInserted(const QModelIndex &, int, int)));
    QSignalSpy dataChangedSpy(sortedVpnConnections.get(), SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &, const QVector<int> &)));

    EXPECT_EQ(CSL({{"apple", {false, true}}, {"banana", {false, true}}, {"coconut", {false, true}}}), vpnList(*sortedVpnConnections));

    rowsAboutToBeRemovedSpy.clear();
    rowsRemovedSpy.clear();
    rowsAboutToBeInsertedSpy.clear();
    rowsInsertedSpy.clear();
    dataChangedSpy.clear();

    // Delete a connection and check it is removed
    deleteSettings(appleConnection);

    WAIT_FOR_SIGNALS(rowsAboutToBeRemovedSpy, 1);
    WAIT_FOR_SIGNALS(rowsRemovedSpy, 1);
    EXPECT_TRUE(rowsAboutToBeInsertedSpy.isEmpty());
    EXPECT_TRUE(rowsInsertedSpy.isEmpty());
    EXPECT_TRUE(dataChangedSpy.isEmpty());

    EXPECT_EQ(CSL({{"banana", {false, true}}, {"coconut", {false, true}}}), vpnList(*sortedVpnConnections));

    rowsAboutToBeRemovedSpy.clear();
    rowsRemovedSpy.clear();
    rowsAboutToBeInsertedSpy.clear();
    rowsInsertedSpy.clear();
    dataChangedSpy.clear();

    // Add a connection that should appear at the start
    auto avocadoConnection = createVpnConnection("avocado");

    EXPECT_TRUE(rowsAboutToBeRemovedSpy.isEmpty());
    EXPECT_TRUE(rowsRemovedSpy.isEmpty());
    WAIT_FOR_SIGNALS(rowsAboutToBeInsertedSpy, 1);
    WAIT_FOR_SIGNALS(rowsInsertedSpy, 1);
    EXPECT_TRUE(dataChangedSpy.isEmpty());

    EXPECT_EQ(CSL({{"avocado", {false, true}}, {"banana", {false, true}}, {"coconut", {false, true}}}), vpnList(*sortedVpnConnections));

    rowsAboutToBeRemovedSpy.clear();
    rowsRemovedSpy.clear();
    rowsAboutToBeInsertedSpy.clear();
    rowsInsertedSpy.clear();
    dataChangedSpy.clear();

    // Activate the banana connection
    auto activeConnection = createActiveConnection("0", device, bananaConnection, "/");

    WAIT_FOR_SIGNALS(dataChangedSpy, 3);
    EXPECT_TRUE(rowsAboutToBeRemovedSpy.isEmpty());
    EXPECT_TRUE(rowsRemovedSpy.isEmpty());
    EXPECT_TRUE(rowsAboutToBeInsertedSpy.isEmpty());
    EXPECT_TRUE(rowsInsertedSpy.isEmpty());

    EXPECT_EQ(CSL({{"avocado", {false, false}}, {"banana", {true, true}}, {"coconut", {false, false}}}), vpnList(*sortedVpnConnections));

    rowsAboutToBeRemovedSpy.clear();
    rowsRemovedSpy.clear();
    rowsAboutToBeInsertedSpy.clear();
    rowsInsertedSpy.clear();
    dataChangedSpy.clear();

    // Deactivate the banana connection
    removeActiveConnection(device, activeConnection);

    WAIT_FOR_SIGNALS(dataChangedSpy, 3);
    EXPECT_TRUE(rowsAboutToBeRemovedSpy.isEmpty());
    EXPECT_TRUE(rowsRemovedSpy.isEmpty());
    EXPECT_TRUE(rowsAboutToBeInsertedSpy.isEmpty());
    EXPECT_TRUE(rowsInsertedSpy.isEmpty());

    EXPECT_EQ(CSL({{"avocado", {false, true}}, {"banana", {false, true}}, {"coconut", {false, true}}}), vpnList(*sortedVpnConnections));
}

TEST_F(TestConnectivityApiVpn, FollowsVpnState)
{
    // Add a single VPN configuration
    auto appleConnection = createVpnConnection("apple");

    // Add a physical device to use for the connection
    setGlobalConnectedState(NM_STATE_CONNECTED_GLOBAL);
    auto device = createWiFiDevice(NM_DEVICE_STATE_ACTIVATED);

    // Start the indicator
    ASSERT_NO_THROW(startIndicator());

    // Connect the the service
    auto connectivity(newConnectivity());

    auto sortedVpnConnections = getSortedVpnConnections(*connectivity);

    QSignalSpy rowsAboutToBeRemovedSpy(sortedVpnConnections.get(), SIGNAL(rowsAboutToBeRemoved(const QModelIndex &, int, int)));
    QSignalSpy rowsRemovedSpy(sortedVpnConnections.get(), SIGNAL(rowsRemoved(const QModelIndex &, int, int)));
    QSignalSpy rowsAboutToBeInsertedSpy(sortedVpnConnections.get(), SIGNAL(rowsAboutToBeInserted(const QModelIndex &, int, int)));
    QSignalSpy rowsInsertedSpy(sortedVpnConnections.get(), SIGNAL(rowsInserted(const QModelIndex &, int, int)));
    QSignalSpy dataChangedSpy(sortedVpnConnections.get(), SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &, const QVector<int> &)));

    EXPECT_EQ(CSL({{"apple", {false, true}}}), vpnList(*sortedVpnConnections));

    rowsAboutToBeRemovedSpy.clear();
    rowsRemovedSpy.clear();
    rowsAboutToBeInsertedSpy.clear();
    rowsInsertedSpy.clear();
    dataChangedSpy.clear();

    OrgFreedesktopNetworkManagerSettingsConnectionInterface appleInterface(
            NM_DBUS_SERVICE, appleConnection, dbusTestRunner.systemConnection());
    QVariantDictMap settings = appleInterface.GetSettings();
    settings["connection"]["id"] = "banana";

    appleInterface.Update(settings).waitForFinished();

    WAIT_FOR_SIGNALS(dataChangedSpy, 1);
    EXPECT_TRUE(rowsAboutToBeRemovedSpy.isEmpty());
    EXPECT_TRUE(rowsRemovedSpy.isEmpty());
    EXPECT_TRUE(rowsAboutToBeInsertedSpy.isEmpty());
    EXPECT_TRUE(rowsInsertedSpy.isEmpty());

    // Name should have changed
    EXPECT_EQ(CSL({{"banana", {false, true}}}), vpnList(*sortedVpnConnections));
}

TEST_F(TestConnectivityApiVpn, UpdatesVpnState)
{
    // Add a single VPN configuration
    auto appleConnection = createVpnConnection("apple");

    // Add a physical device to use for the connection
    setGlobalConnectedState(NM_STATE_CONNECTED_GLOBAL);
    auto device = createWiFiDevice(NM_DEVICE_STATE_ACTIVATED);

    // Start the indicator
    ASSERT_NO_THROW(startIndicator());

    // Connect the the service
    auto connectivity(newConnectivity());

    auto sortedVpnConnections = getSortedVpnConnections(*connectivity);

    QSignalSpy rowsAboutToBeRemovedSpy(sortedVpnConnections.get(), SIGNAL(rowsAboutToBeRemoved(const QModelIndex &, int, int)));
    QSignalSpy rowsRemovedSpy(sortedVpnConnections.get(), SIGNAL(rowsRemoved(const QModelIndex &, int, int)));
    QSignalSpy rowsAboutToBeInsertedSpy(sortedVpnConnections.get(), SIGNAL(rowsAboutToBeInserted(const QModelIndex &, int, int)));
    QSignalSpy rowsInsertedSpy(sortedVpnConnections.get(), SIGNAL(rowsInserted(const QModelIndex &, int, int)));
    QSignalSpy dataChangedSpy(sortedVpnConnections.get(), SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &, const QVector<int> &)));

    EXPECT_EQ(CSL({{"apple", {false, true}}}), vpnList(*sortedVpnConnections));

    rowsAboutToBeRemovedSpy.clear();
    rowsRemovedSpy.clear();
    rowsAboutToBeInsertedSpy.clear();
    rowsInsertedSpy.clear();
    dataChangedSpy.clear();

    // Change the VPN connection's id
    sortedVpnConnections->setData(
            sortedVpnConnections->index(0, 0), "banana",
            VpnConnectionsListModel::Roles::RoleId);

    WAIT_FOR_SIGNALS(dataChangedSpy, 1);
    EXPECT_TRUE(rowsAboutToBeRemovedSpy.isEmpty());
    EXPECT_TRUE(rowsRemovedSpy.isEmpty());
    EXPECT_TRUE(rowsAboutToBeInsertedSpy.isEmpty());
    EXPECT_TRUE(rowsInsertedSpy.isEmpty());

    // Name should have changed
    EXPECT_EQ(CSL({{"banana", {false, true}}}), vpnList(*sortedVpnConnections));

    OrgFreedesktopNetworkManagerSettingsConnectionInterface appleInterface(
                NM_DBUS_SERVICE, appleConnection, dbusTestRunner.systemConnection());
    QVariantDictMap settings = appleInterface.GetSettings();
    EXPECT_EQ("banana", settings["connection"]["id"].toString());

    rowsAboutToBeRemovedSpy.clear();
    rowsRemovedSpy.clear();
    rowsAboutToBeInsertedSpy.clear();
    rowsInsertedSpy.clear();
    dataChangedSpy.clear();

    // Activate the VPN connection
    sortedVpnConnections->setData(
            sortedVpnConnections->index(0, 0), true,
            VpnConnectionsListModel::Roles::RoleActive);

    WAIT_FOR_SIGNALS(dataChangedSpy, 1);
    EXPECT_TRUE(rowsAboutToBeRemovedSpy.isEmpty());
    EXPECT_TRUE(rowsRemovedSpy.isEmpty());
    EXPECT_TRUE(rowsAboutToBeInsertedSpy.isEmpty());
    EXPECT_TRUE(rowsInsertedSpy.isEmpty());

    EXPECT_EQ(CSL({{"banana", {true, true}}}), vpnList(*sortedVpnConnections));

    OrgFreedesktopNetworkManagerInterface managerInterface(
            NM_DBUS_SERVICE, NM_DBUS_PATH,
            dbusTestRunner.systemConnection());
    auto activeConnections = managerInterface.activeConnections();
    EXPECT_EQ(QList<QDBusObjectPath>({QDBusObjectPath("/org/freedesktop/NetworkManager/ActiveConnection/0")}),
              activeConnections);
}

TEST_F(TestConnectivityApiVpn, ReadsOpenvpnProperties)
{
    // Add a single VPN configuration
    auto appleConnection = createVpnConnection("apple", "org.freedesktop.NetworkManager.openvpn",
    {
        {"connection-type", "password-tls"},
        {"remote", "remotey"},
        {"ca", "/my/ca.crt"},
        {"cert", "/my/cert.crt"},
        {"cert-pass-flags", "1"},
        {"key", "/my/key.key"},
        {"password-flags", "1"}
    },
    {
        {"cert-pass", "certificate password"},
        {"password", "the password"}
    });

    // Add a physical device to use for the connection
    setGlobalConnectedState(NM_STATE_CONNECTED_GLOBAL);
    auto device = createWiFiDevice(NM_DEVICE_STATE_ACTIVATED);

    // Start the indicator
    ASSERT_NO_THROW(startIndicator());

    // Connect the the service
    auto connectivity(newConnectivity());

    auto vpnConnections = connectivity->vpnConnections();

    ASSERT_EQ(CSL({{"apple", {false, true}}}), vpnList(*vpnConnections));
    auto connection = getOpenvpnConnection(vpnConnections, 0);
    ASSERT_TRUE(connection);

    EXPECT_EQ("apple", connection->id());
    EXPECT_FALSE(connection->active());
    EXPECT_EQ(VpnConnection::Type::OPENVPN, connection->type());

    EXPECT_EQ(OpenvpnConnection::ConnectionType::PASSWORD_TLS, connection->connectionType());
    EXPECT_EQ("remotey", connection->remote());
    EXPECT_EQ("/my/ca.crt", connection->ca());
    EXPECT_EQ("/my/cert.crt", connection->cert());
    EXPECT_EQ("/my/key.key", connection->key());

    QSignalSpy caChangedSpy(connection, SIGNAL(caChanged(const QString &)));
    QSignalSpy certChangedSpy(connection, SIGNAL(certChanged(const QString &)));
    QSignalSpy keyChangedSpy(connection, SIGNAL(keyChanged(const QString &)));
    QSignalSpy remoteChangedSpy(connection, SIGNAL(remoteChanged(const QString &)));

    OrgFreedesktopNetworkManagerSettingsConnectionInterface appleInterface(
                    NM_DBUS_SERVICE, appleConnection, dbusTestRunner.systemConnection());
    QVariantDictMap settings = appleInterface.GetSettings();
    settings["vpn"]["data"] = QVariant::fromValue(QStringMap(
    {
        {"connection-type", "password-tls"},
        {"remote", "remote2"},
        {"ca", "/my/ca2.crt"},
        {"cert", "/my/cert2.crt"},
        {"cert-pass-flags", "1"},
        {"key", "/my/key2.key"},
        {"password-flags", "1"},
    }));
    auto reply = appleInterface.Update(settings);
    reply.waitForFinished();
    if (reply.isError())
    {
        EXPECT_FALSE(reply.isError()) << reply.error().message().toStdString();
    }

    WAIT_FOR_SIGNALS(caChangedSpy, 1);
    WAIT_FOR_SIGNALS(certChangedSpy, 1);
    WAIT_FOR_SIGNALS(keyChangedSpy, 1);
    WAIT_FOR_SIGNALS(remoteChangedSpy, 1);

    EXPECT_EQ("remote2", connection->remote());
    EXPECT_EQ("/my/ca2.crt", connection->ca());
    EXPECT_EQ("/my/cert2.crt", connection->cert());
    EXPECT_EQ("/my/key2.key", connection->key());

    QSignalSpy certPassChangedSpy(connection, SIGNAL(certPassChanged(const QString&)));
    QSignalSpy passwordChangedSpy(connection, SIGNAL(passwordChanged(const QString&)));

    connection->updateSecrets();
    WAIT_FOR_SIGNALS(certPassChangedSpy, 1);
    WAIT_FOR_SIGNALS(passwordChangedSpy, 1);
    EXPECT_EQ("certificate password", connection->certPass());
    EXPECT_EQ("the password", connection->password());
}

TEST_F(TestConnectivityApiVpn, WritesOpenvpnProperties)
{
    // Add a single VPN configuration
    auto appleConnection = createVpnConnection("apple", "org.freedesktop.NetworkManager.openvpn",
    {
        {"connection-type", "tls"},
        {"remote", "remotey"},
        {"ca", "/my/ca.crt"},
        {"cert", "/my/cert.crt"},
        {"cert-pass-flags", "1"},
        {"key", "/my/key.key"}
    });

    // Add a physical device to use for the connection
    setGlobalConnectedState(NM_STATE_CONNECTED_GLOBAL);
    auto device = createWiFiDevice(NM_DEVICE_STATE_ACTIVATED);

    // Start the indicator
    ASSERT_NO_THROW(startIndicator());

    // Connect the the service
    auto connectivity(newConnectivity());

    auto vpnConnections = connectivity->vpnConnections();

    ASSERT_EQ(CSL({{"apple", {false, true}}}), vpnList(*vpnConnections));
    auto connection = getOpenvpnConnection(vpnConnections, 0);
    ASSERT_TRUE(connection);

    QSignalSpy remoteChangedSpy(connection, SIGNAL(remoteChanged(const QString &)));

    auto& appleMockInterface = dbusMock.mockInterface(
            NM_DBUS_SERVICE, appleConnection,
            NM_DBUS_IFACE_SETTINGS_CONNECTION,
            QDBusConnection::SystemBus);
    QSignalSpy appleMockCallSpy(
            &appleMockInterface,
            SIGNAL(MethodCalled(const QString &, const QVariantList &)));

    OrgFreedesktopNetworkManagerSettingsConnectionInterface appleInterface(
            NM_DBUS_SERVICE, appleConnection,
            dbusTestRunner.systemConnection());

    EXPECT_EQ("apple", connection->id());
    EXPECT_FALSE(connection->active());
    EXPECT_EQ(VpnConnection::Type::OPENVPN, connection->type());

    EXPECT_EQ(OpenvpnConnection::ConnectionType::TLS, connection->connectionType());
    connection->setRemote("remote2");

    WAIT_FOR_SIGNALS(appleMockCallSpy, 1);
    WAIT_FOR_SIGNALS(remoteChangedSpy, 1);

    QStringMap vpnData;
    QVariantDictMap settings = appleInterface.GetSettings();
    settings["vpn"]["data"].value<QDBusArgument>() >> vpnData;

    EXPECT_EQ("remote2", vpnData["remote"]);
}

TEST_F(TestConnectivityApiVpn, CreatesOpenvpnConnection)
{
    setGlobalConnectedState(NM_STATE_CONNECTED_GLOBAL);
    auto device = createWiFiDevice(NM_DEVICE_STATE_ACTIVATED);

    // Start the indicator
    ASSERT_NO_THROW(startIndicator());

    // Connect the the service
    auto connectivity(newConnectivity());

    auto vpnConnections = qobject_cast<VpnConnectionsListModel*>(connectivity->vpnConnections());
    ASSERT_TRUE(vpnConnections);

    QSignalSpy addConnectionSpy(vpnConnections, SIGNAL(addFinished(VpnConnection *)));

    vpnConnections->add(VpnConnection::Type::OPENVPN);
    WAIT_FOR_SIGNALS(addConnectionSpy, 1);

    ASSERT_EQ(CSL({{"VPN connection 1", {false, true}}}), vpnList(*vpnConnections));
    auto connection = getOpenvpnConnection(vpnConnections, 0);
    ASSERT_TRUE(connection);

    // These should be the same object instance
    EXPECT_EQ(addConnectionSpy.first().first().value<VpnConnection*>(), connection);
}

TEST_F(TestConnectivityApiVpn, DeletesVpnConnection)
{
    // Add a single VPN configuration
    auto appleConnection = createVpnConnection("apple", "org.freedesktop.NetworkManager.openvpn",
    {
        {"connection-type", "tls"},
        {"remote", "remotey"},
        {"ca", "/my/ca.crt"},
        {"cert", "/my/cert.crt"},
        {"cert-pass-flags", "1"},
        {"key", "/my/key.key"}
    });

    setGlobalConnectedState(NM_STATE_CONNECTED_GLOBAL);
    auto device = createWiFiDevice(NM_DEVICE_STATE_ACTIVATED);

    // Start the indicator
    ASSERT_NO_THROW(startIndicator());

    // Connect the the service
    auto connectivity(newConnectivity());

    auto vpnConnections = qobject_cast<VpnConnectionsListModel*>(connectivity->vpnConnections());
    ASSERT_TRUE(vpnConnections);

    ASSERT_EQ(CSL({{"apple", {false, true}}}), vpnList(*vpnConnections));
    auto connection = getOpenvpnConnection(vpnConnections, 0);
    ASSERT_TRUE(connection);

    QSignalSpy rowsRemovedSpy(vpnConnections, SIGNAL(rowsRemoved(const QModelIndex &, int, int)));

    vpnConnections->remove(connection);

    WAIT_FOR_SIGNALS(rowsRemovedSpy, 1);
    EXPECT_EQ(0, vpnConnections->rowCount(QModelIndex()));
}


}
