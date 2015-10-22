
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

typedef QPair<QString, bool> CS;
typedef QList<CS> CSL;

inline void PrintTo(const CS& list, std::ostream* os) {
    QString output;
    QDebug dbg(&output);
    dbg << list;

    *os << "CSL(" << output.toStdString() << ")";
}

inline void PrintTo(const QList<QDBusObjectPath>& list, std::ostream* os) {
    QString output;
    for (const auto& path: list)
    {
        output.append("\"" + path.path() + "\",");
    }

    *os << "QList<QDBusObjectPath>(" << output.toStdString() << ")";
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
            connectionState.second = model.data(idx, VpnConnectionsListModel::Roles::RoleActive).toBool();
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

    EXPECT_EQ(CSL({{"apple", false}}), vpnList(*sortedVpnConnections));

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

    EXPECT_EQ(CSL({{"apple", false}, {"banana", false}, {"coconut", false}}), vpnList(*sortedVpnConnections));

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

    EXPECT_EQ(CSL({{"banana", false}, {"coconut", false}}), vpnList(*sortedVpnConnections));

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

    EXPECT_EQ(CSL({{"avocado", false}, {"banana", false}, {"coconut", false}}), vpnList(*sortedVpnConnections));

    rowsAboutToBeRemovedSpy.clear();
    rowsRemovedSpy.clear();
    rowsAboutToBeInsertedSpy.clear();
    rowsInsertedSpy.clear();
    dataChangedSpy.clear();

    // Activate the banana connection
    auto activeConnection = createActiveConnection("0", device, bananaConnection, "/");

    EXPECT_TRUE(rowsAboutToBeRemovedSpy.isEmpty());
    EXPECT_TRUE(rowsRemovedSpy.isEmpty());
    EXPECT_TRUE(rowsAboutToBeInsertedSpy.isEmpty());
    EXPECT_TRUE(rowsInsertedSpy.isEmpty());
    WAIT_FOR_SIGNALS(dataChangedSpy, 1);

    EXPECT_EQ(CSL({{"avocado", false}, {"banana", true}, {"coconut", false}}), vpnList(*sortedVpnConnections));

    rowsAboutToBeRemovedSpy.clear();
    rowsRemovedSpy.clear();
    rowsAboutToBeInsertedSpy.clear();
    rowsInsertedSpy.clear();
    dataChangedSpy.clear();

    // Deactivate the banana connection
    removeActiveConnection(device, activeConnection);

    EXPECT_TRUE(rowsAboutToBeRemovedSpy.isEmpty());
    EXPECT_TRUE(rowsRemovedSpy.isEmpty());
    EXPECT_TRUE(rowsAboutToBeInsertedSpy.isEmpty());
    EXPECT_TRUE(rowsInsertedSpy.isEmpty());
    WAIT_FOR_SIGNALS(dataChangedSpy, 1);

    EXPECT_EQ(CSL({{"avocado", false}, {"banana", false}, {"coconut", false}}), vpnList(*sortedVpnConnections));
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

    EXPECT_EQ(CSL({{"apple", false}}), vpnList(*sortedVpnConnections));

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
    EXPECT_EQ(CSL({{"banana", false}}), vpnList(*sortedVpnConnections));
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

    EXPECT_EQ(CSL({{"apple", false}}), vpnList(*sortedVpnConnections));

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
    EXPECT_EQ(CSL({{"banana", false}}), vpnList(*sortedVpnConnections));

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

    EXPECT_EQ(CSL({{"banana", true}}), vpnList(*sortedVpnConnections));

    OrgFreedesktopNetworkManagerInterface managerInterface(
            NM_DBUS_SERVICE, NM_DBUS_PATH,
            dbusTestRunner.systemConnection());
    auto activeConnections = managerInterface.activeConnections();
    EXPECT_EQ(QList<QDBusObjectPath>({QDBusObjectPath("/org/freedesktop/NetworkManager/ActiveConnection/0")}),
              activeConnections);
}


}
