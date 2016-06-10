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
 * Authors:
 *   Antti Kaijanmäki <antti.kaijanmaki@canonical.com>
 *   Pete Woods <pete.woods@canonical.com>
 */

#include <connectivityqt/modem.h>
#include <connectivityqt/modems-list-model.h>
#include <indicator-network-test-base.h>
#include <dbus-types.h>
#include <NetworkManagerSettingsInterface.h>

#include <QDebug>
#include <QTestEventLoop>

#define DEFINE_MODEL_LISTENERS \
    QSignalSpy rowsAboutToBeRemovedSpy(modems.get(), SIGNAL(rowsAboutToBeRemoved(const QModelIndex &, int, int)));\
    QSignalSpy rowsRemovedSpy(modems.get(), SIGNAL(rowsRemoved(const QModelIndex &, int, int)));\
    QSignalSpy rowsAboutToBeInsertedSpy(modems.get(), SIGNAL(rowsAboutToBeInserted(const QModelIndex &, int, int)));\
    QSignalSpy rowsInsertedSpy(modems.get(), SIGNAL(rowsInserted(const QModelIndex &, int, int)));\
    QSignalSpy dataChangedSpy(modems.get(), SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &, const QVector<int> &)));\

#define CLEAR_MODEL_LISTENERS \
    rowsAboutToBeRemovedSpy.clear();\
    rowsRemovedSpy.clear();\
    rowsAboutToBeInsertedSpy.clear();\
    rowsInsertedSpy.clear();\
    dataChangedSpy.clear();

struct SS {
    QString imsi;
    QString primaryPhoneNumber;
    bool locked;
    bool present;
    QString mcc;
    QString mnc;
    QList<QString> preferredLanguages;

    bool operator==(const SS& other) const
    {
        return imsi == other.imsi
                && primaryPhoneNumber == other.primaryPhoneNumber
                && locked == other.locked && present == other.present
                && mcc == other.mcc && mnc == other.mnc
                && preferredLanguages == other.preferredLanguages;
    }
};
typedef QPair<int, SS> MS;
typedef QList<MS> MSL;

inline void PrintTo (const SS& simState, std::ostream* os)
{
    *os << "SS("
            << "IMSI: " << simState.imsi.toStdString () << ", "
            << "Phone #: " << simState.primaryPhoneNumber.toStdString () << ", "
            << "Locked: " << (simState.locked ? "y" : "n") << ", "
            << "Present: " << (simState.present ? "y" : "n") << ", "
            << "MCC: " << simState.mcc.toStdString () << ", "
            << "MNC: " << simState.mnc.toStdString () << ", "
            << "Langs: " << QStringList(simState.preferredLanguages).join (",").toStdString () << ")";
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

    MSL modemList(QAbstractItemModel& model)
    {
        MSL modemStates;
        int rowCount(model.rowCount());
        for (int i = 0; i < rowCount; ++i)
        {
            auto idx = model.index(i, 0);
            MS modemState;
            modemState.first = model.data(idx, ModemsListModel::Roles::RoleIndex).toInt();
            EXPECT_FALSE(model.data(idx, ModemsListModel::Roles::RoleSerial).toString().isEmpty());

            SS simState;
            auto sim = qvariant_cast<Sim*>(model.data(idx, ModemsListModel::Roles::RoleSim));
            if (sim)
            {
                simState.imsi = sim->imsi ();
                simState.primaryPhoneNumber = sim->primaryPhoneNumber ();
                simState.locked = sim->locked ();
                simState.present = sim->present ();
                simState.mcc = sim->mcc ();
                simState.mnc = sim->mnc ();
                simState.preferredLanguages = sim->preferredLanguages ();

                modemState.second = simState;
            }
            else
            {
                EXPECT_TRUE(sim) << "Could not get a SIM at index " << i
                        << " from ModemModel";
            }

            modemStates << modemState;
        }
        return modemStates;
    }
};

TEST_F(TestConnectivityApiModem, SingleModemAtStartup)
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

    DEFINE_MODEL_LISTENERS

    WAIT_FOR_ROW_COUNT(rowsInsertedSpy, modems, 1)
    EXPECT_TRUE(rowsAboutToBeRemovedSpy.isEmpty ());
    EXPECT_TRUE(rowsRemovedSpy.isEmpty ());
    EXPECT_TRUE(dataChangedSpy.isEmpty ());

    EXPECT_EQ(MSL({
        MS{1, SS{
            "310150000000000",
            "123456789",
            false,
            true,
            "310",
            "150",
            {"en"}
        }}
    }), modemList(*modems));
}

TEST_F(TestConnectivityApiModem, TwoModemsAtStartup)
{
    createModem("ril_1");

    // Add a physical device to use for the connection
    setGlobalConnectedState(NM_STATE_CONNECTED_GLOBAL);
    auto device = createWiFiDevice(NM_DEVICE_STATE_ACTIVATED);

    // Start the indicator
    ASSERT_NO_THROW(startIndicator());

    // Connect the the service
    auto connectivity(newConnectivity());

    // Get the modems model
    auto modems = getSortedModems(*connectivity);

    DEFINE_MODEL_LISTENERS

    WAIT_FOR_ROW_COUNT(rowsInsertedSpy, modems, 2)
    EXPECT_TRUE(rowsAboutToBeRemovedSpy.isEmpty ());
    EXPECT_TRUE(rowsRemovedSpy.isEmpty ());

    EXPECT_EQ(MSL({
        MS{1, SS{
            "310150000000000",
            "123456789",
            false,
            true,
            "310",
            "150",
            {"en"}
        }},
        MS{2, SS{
            "310150000000001",
            "123456789",
            false,
            true,
            "310",
            "150",
            {"en"}
        }}
    }), modemList(*modems));
}

TEST_F(TestConnectivityApiModem, AddAModem)
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

    DEFINE_MODEL_LISTENERS

    WAIT_FOR_ROW_COUNT(rowsInsertedSpy, modems, 1)
    EXPECT_TRUE(rowsAboutToBeRemovedSpy.isEmpty ());
    EXPECT_TRUE(rowsRemovedSpy.isEmpty ());
    EXPECT_TRUE(dataChangedSpy.isEmpty ());

    EXPECT_EQ(MSL({
        MS{1, SS{
            "310150000000000",
            "123456789",
            false,
            true,
            "310",
            "150",
            {"en"}
        }}
    }), modemList(*modems));

    CLEAR_MODEL_LISTENERS

    createModem("ril_1");

    WAIT_FOR_ROW_COUNT(rowsInsertedSpy, modems, 2)
    EXPECT_TRUE(rowsAboutToBeRemovedSpy.isEmpty ());
    EXPECT_TRUE(rowsRemovedSpy.isEmpty ());
    EXPECT_TRUE(dataChangedSpy.isEmpty ());

    EXPECT_EQ(MSL({
        MS{1, SS{
            "310150000000000",
            "123456789",
            false,
            true,
            "310",
            "150",
            {"en"}
        }},
        MS{2, SS{
            "310150000000001",
            "123456789",
            false,
            true,
            "310",
            "150",
            {"en"}
        }}
    }), modemList(*modems));

    CLEAR_MODEL_LISTENERS
}

TEST_F(TestConnectivityApiModem, ModemProperties)
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

    DEFINE_MODEL_LISTENERS

    WAIT_FOR_ROW_COUNT(rowsInsertedSpy, modems, 1)
    EXPECT_TRUE(rowsAboutToBeRemovedSpy.isEmpty ());
    EXPECT_TRUE(rowsRemovedSpy.isEmpty ());

    auto modem = qvariant_cast<Modem*>(modems->data(modems->index(0, 0), ModemsListModel::Roles::RoleModem));

    EXPECT_EQ(1, modem->index());
    EXPECT_EQ("ed752c5f-f723-437e-bc6c-000000000000", modem->serial().toStdString());
    EXPECT_TRUE(modem->sim());
}

}
