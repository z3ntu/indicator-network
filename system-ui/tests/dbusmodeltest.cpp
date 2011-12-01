
#include "dbusmodeltest.h"
#include "dbusmodel.h"

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
        qDebug() << "Id:" <<  model->data(index, 0) << "Title:" << model->data(index, 1);
        if (model->data(index, 3).toBool())
            dumpMenu(qobject_cast<DBusModel*>(model->data(index, 4).value<QObject*>()));
    }
}

void DBusModelTest::testDumpMenu()
{
    DBusModel model;
    QVERIFY(model.connectToServer("org.dbusmenu.test", "/org/test"));
    QTest::qWait(1000);
    QVERIFY(model.isConnected());
    qDebug() << "RowCount:" << model.rowCount();
    dumpMenu(&model);
}

QTEST_MAIN(DBusModelTest)

#include "dbusmodeltest.moc"
