
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

#include <indicator-network-test-base.h>
#include <connectivityqt/internal/connections-list-model.h>
#include <dbus-types.h>
#include <NetworkManagerSettingsInterface.h>

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
            connectionState.first = model.data(idx, connectivityqt::internal::ConnectionsListModel::Roles::RoleId).toString();
            connectionState.second = model.data(idx, connectivityqt::internal::ConnectionsListModel::Roles::RoleActive).toBool();
            connectionStates << connectionState;
        }
        return connectionStates;
    }
};

TEST_F(TestConnectivityApiVpn, ReadsVpn)
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

    auto vpnConnections = connectivity->vpnConnections();

    auto sortedVpnConnections = make_unique<QSortFilterProxyModel>();
    sortedVpnConnections->setSortRole(Qt::DisplayRole);
    sortedVpnConnections->sort(0);

    sortedVpnConnections->setSourceModel(vpnConnections);

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

}
