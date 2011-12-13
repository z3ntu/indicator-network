
#include "dbusmodeltest.h"
#include "dbusmodel.h"
#include "dbuscontrol.h"

#include <QObject>
#include <QtTestGui>
#include <QDebug>

void dumpMenu(DBusModel *model)
{
    if (!model)
        return;

    QTest::qWait(500);
    for(int i=0; i < model->rowCount(); i++) {
        QModelIndex index = model->index(i);
        if (model->data(index, 5).toBool()) {
            DBusModel subModel;
            subModel.setControl(model->control());
            subModel.setMenuId(model->data(index, 0).toInt());
            subModel.load();
            dumpMenu(&subModel);
        }
    }
}

void DBusModelTest::testDumpMenu()
{
    DBusControl control;
    control.setService("org.dbusmenu.test");
    control.setObjectPath("/org/test");
    control.connectToServer();

    QTest::qWait(500);
    QVERIFY(control.isConnected());

    DBusModel model;
    model.setControl(&control);
    model.setMenuId(0);
    model.load();
    dumpMenu(&model);
}

void DBusModelTest::testInvalidConnection()
{
    DBusControl control;
    control.setService("org.dbusmenu.invalid.test");
    control.setObjectPath("/org/test");
    control.connectToServer();

    QTest::qWait(500);
    QVERIFY(control.isConnected() == false);
}

QTEST_MAIN(DBusModelTest)

#include "dbusmodeltest.moc"
