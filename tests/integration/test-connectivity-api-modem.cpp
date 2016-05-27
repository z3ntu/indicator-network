
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
 * Author: Antti Kaijanmäki <antti.kaijanmaki@canonical.com>
 */

#include <connectivityqt/modems-list-model.h>
#include <indicator-network-test-base.h>
#include <dbus-types.h>
#include <NetworkManagerSettingsInterface.h>

#include <QDebug>
#include <QTestEventLoop>

struct SS {
    QString imsi;
    QString primaryPhoneNumber;
    QList<QString> phoneNumbers;
    bool locked;
    bool present;
    QString mcc;
    QString mnc;
    QList<QString> preferredLanguages;

    bool operator==(const SS& other) const
    {
        return imsi == other.imsi
                && primaryPhoneNumber == other.primaryPhoneNumber
                && phoneNumbers == other.phoneNumbers && locked == other.locked
                && present == other.present && mcc == other.mcc
                && mnc == other.mnc
                && preferredLanguages == other.preferredLanguages;
    }
};
typedef QPair<int, SS> MS;
typedef QList<MS> MSL;

inline void PrintTo (const SS& simState, std::ostream* os)
{
    *os << "SS(" << simState.imsi.toStdString () << ", "
            << simState.primaryPhoneNumber.toStdString () << ", "
            << simState.phoneNumbers.join (",").toStdString () << ", "
            << (simState.locked ? "y" : "n") << ", "
            << (simState.present ? "y" : "n") << ", "
            << simState.mcc.toStdString () << ", "
            << simState.mnc.toStdString () << ", "
            << simState.preferredLanguages.join (",").toStdString () << ")";
}

inline void PrintTo(const MS& modemState, std::ostream* os) {
    *os << "MS(" << modemState.first << ", " ;
    PrintTo(modemState.second, os);
    *os << ")";
}

using namespace std;
using namespace testing;
using namespace connectivityqt;

namespace
{

class TestConnectivityApiModem: public IndicatorNetworkTestBase
{
protected:
    static void SetUpTestCase()
    {
        Connectivity::registerMetaTypes();
    }

    unique_ptr<QSortFilterProxyModel> getSortedModems(Connectivity& connectivity)
    {
        auto modems = connectivity.modems();

        auto sortedModems = make_unique<QSortFilterProxyModel>();
        sortedModems->setSortRole(VpnConnectionsListModel::Roles::RoleId);
        sortedModems->sort(0);

        sortedModems->setSourceModel(modems);

        return sortedModems;
    }

    MSL modemList(QAbstractItemModel& model)
    {
        MSL modemStates;
        int rowCount(model.rowCount());
        for (int i = 0; i < rowCount; ++i)
        {
            auto idx = model.index(i, 0);
            MS modemState;
            modemState.first = model.data(idx, ModemsListModel::Roles::RoleIndex).toInt();
            // Serial is randomly generated, so this is the best we can do
            EXPECT_FALSE(model.data(idx, ModemsListModel::Roles::RoleSerial).toString().isEmpty());

            SS simState;
            auto sim = qvariant_cast<Sim*>(model.data(idx, ModemsListModel::Roles::RoleSim));
            simState.imsi = sim->imsi().left(6);
            simState.primaryPhoneNumber = sim->primaryPhoneNumber();
            simState.phoneNumbers = sim->phoneNumbers();
            simState.locked = sim->locked();
            simState.present = sim->present();
            simState.mcc = sim->mcc();
            simState.mnc = sim->mnc();
            simState.preferredLanguages = sim->preferredLanguages();

            modemState.second = simState;

            modemStates << modemState;
        }
        return modemStates;
    }
};

TEST_F(TestConnectivityApiModem, Index)
{
    // Add a physical device to use for the connection
    setGlobalConnectedState(NM_STATE_CONNECTED_GLOBAL);
    auto device = createWiFiDevice(NM_DEVICE_STATE_ACTIVATED);

    // Start the indicator
    ASSERT_NO_THROW(startIndicator());

    // Connect the the service
    auto connectivity(newConnectivity());

    // Get the modems model
    auto modems = getSortedModems(*connectivity);

    QSignalSpy rowsAboutToBeRemovedSpy(modems.get(), SIGNAL(rowsAboutToBeRemoved(const QModelIndex &, int, int)));
    QSignalSpy rowsRemovedSpy(modems.get(), SIGNAL(rowsRemoved(const QModelIndex &, int, int)));
    QSignalSpy rowsAboutToBeInsertedSpy(modems.get(), SIGNAL(rowsAboutToBeInserted(const QModelIndex &, int, int)));
    QSignalSpy rowsInsertedSpy(modems.get(), SIGNAL(rowsInserted(const QModelIndex &, int, int)));
    QSignalSpy dataChangedSpy(modems.get(), SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &, const QVector<int> &)));

    EXPECT_EQ(MSL({MS{1, SS
        {
            "310150",
            "123456789",
            {"123456789", "234567890"},
            false,
            true,
            "310",
            "150",
            {"en"}
        }
    }}), modemList(*modems));

//    WAIT_FOR_SIGNALS(rowsAboutToBeInsertedSpy, 1);
//    WAIT_FOR_SIGNALS(rowsInsertedSpy, 1);
//    EXPECT_TRUE(rowsAboutToBeRemovedSpy.isEmpty());
//    EXPECT_TRUE(rowsRemovedSpy.isEmpty());
//    EXPECT_TRUE(dataChangedSpy.isEmpty());
//
//    EXPECT_EQ(MSL({1}), modemList(*modems));

//    rowsAboutToBeRemovedSpy.clear();
//        rowsRemovedSpy.clear();
//        rowsAboutToBeInsertedSpy.clear();
//        rowsInsertedSpy.clear();
//        dataChangedSpy.clear();

}

TEST_F(TestConnectivityApiModem, Sim)
{

}

}
