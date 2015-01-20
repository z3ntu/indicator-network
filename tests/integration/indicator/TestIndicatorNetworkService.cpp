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
#include <libqtdbustest/QProcessDBusService.h>
#include <libqtdbusmock/DBusMock.h>
#include <NetworkManager.h>

#include <qmenumodel/unitymenumodel.h>

#include <QSignalSpy>


#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace std;
using namespace testing;
using namespace QtDBusTest;
using namespace QtDBusMock;

class MenuPrinter : public UnityMenuModel
{
Q_OBJECT

enum MenuRoles {
    LabelRole  = Qt::DisplayRole + 1,
    SensitiveRole,
    IsSeparatorRole,
    IconRole,
    TypeRole,
    ExtendedAttributesRole,
    ActionRole,
    ActionStateRole,
    IsCheckRole,
    IsRadioRole,
    IsToggledRole
};

public:
    MenuPrinter(const QString& busName, const QVariantMap& actions,
                const QString& menuObjectPath, QObject* parent = 0) :
                    UnityMenuModel(parent)
    {

        setBusName(busName.toUtf8());
        setActions(actions);
        setMenuObjectPath(menuObjectPath.toUtf8());

        QObject::connect(this,
                         SIGNAL(rowsInserted(const QModelIndex&, int, int)),
                         SLOT(onModelChanged()));

        m_buffer += "---------------------\n";
        QSignalSpy spy(this, SIGNAL(rowsInserted(const QModelIndex&, int, int)));
        m_buffer += "---------------------\n";
        spy.wait(1000);
        m_buffer += "---------------------\n";
        spy.wait(1000);
        m_buffer += "---------------------\n";
    }

private Q_SLOTS:
    void onModelChanged()
    {
        printModel(this);
        m_buffer += "================\n";
    }

public:
    QString m_buffer;

    void printModel(UnityMenuModel* model, int indent = 0)
    {
        int count = model->rowCount();
        for (int i = 0; i < count; ++i)
        {
            QModelIndex index = model->index(i, 0);
            QString label = model->data(index, MenuRoles::LabelRole).toString();
            QString icon = model->data(index, MenuRoles::IconRole).toString();
            QString action = model->data(index, MenuRoles::ActionRole).toString();
            {
                for (int j = 0; j < indent * 2; ++j)
                    m_buffer += " ";
                m_buffer += " > " + label + ", " + icon + ", " + action + "\n";
            }
            printChilden(model, index, indent + 1);
        }
    }

    void
    printChilden(UnityMenuModel* model, QModelIndex &index, int indent = 0)
    {
        int children = model->rowCount(index);
        for (int i = 0; i < children; ++i)
        {
            QModelIndex childIndex(model->index(i, 0, index));
            QString label = model->data(childIndex, MenuRoles::LabelRole).toString();
            QString icon = model->data(childIndex, MenuRoles::IconRole).toString();
            QString action = model->data(childIndex, MenuRoles::ActionRole).toString();
            {
                for (int j = 0; j < indent * 2; ++j)
                    m_buffer += " ";
                m_buffer += " > " + label + ", " + icon + ", " + action + "\n";
            }
            printChilden(model, childIndex, indent + 1);
        }
    }
};

namespace
{

class TestIndicatorNetworkService : public Test
{
protected:
    TestIndicatorNetworkService() :
            dbusMock(dbusTestRunner)
    {
    }

    void SetUp() override
    {
        dbusMock.registerNetworkManager();
        dbusMock.registerOfono();
        dbusMock.registerURfkill();

        dbusTestRunner.registerService(
                DBusServicePtr(
                        new QProcessDBusService(
                                "com.canonical.indicator.network",
                                QDBusConnection::SessionBus,
                                NETWORK_SERVICE_BIN,
                                QStringList())));

        dbusTestRunner.startServices();
    }

    void TearDown() override
    {
        sleep(1); // FIXME delete this line when the indicator shuts down stably
    }

    DBusTestRunner dbusTestRunner;

    DBusMock dbusMock;
};

TEST_F(TestIndicatorNetworkService, Foo)
{
    MenuPrinter printer("com.canonical.indicator.network",
                        QVariantMap { { "indicator", "/com/canonical/indicator/network" } },
                        "/com/canonical/indicator/network/phone");
    qDebug() << printer.m_buffer;
}

} // namespace

#include "TestIndicatorNetworkService.moc"
