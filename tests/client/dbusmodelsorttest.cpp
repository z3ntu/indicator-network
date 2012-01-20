
#include "dbusmodelsorttest.h"
#include "dbusmodel.h"
#include "dbuscontrol.h"
#include "dbusmenuscript.h"

#include <QObject>
#include <QtTestGui>
#include <QDebug>


static void checkModelSort(DBusModel & model, const QStringList & labels)
{
    const QHash<int, QByteArray> &roleNames =  model.roleNames();
    int labelRole = roleNames.key("label");
    for(int i=0; i < model.count(); i++) {
        QCOMPARE(labels[i], model.data(model.index(i, 0), labelRole).toString());
    }
}

/*
 * Check if DBusControl handle menu move correct
 */
void DBusModelSortTest::testMenuOrder()
{
    DBusMenuScript script;
    script.start();

    // Initialize model and control
    DBusControl control;
    control.setService(MENU_SERVICE_NAME);
    control.setObjectPath(MENU_OBJECT_PATH);
    control.connectToServer();

    QTest::qWait(500);
    QVERIFY(control.isConnected());

    DBusModel model;
    model.setControl(&control);
    model.setMenuId(0);
    model.load();

    // Insert 5 items in the correct order
    script.walk(5);
    checkModelSort(model, QStringList() << "Menu0"
                                        << "Menu1"
                                        << "Menu2"
                                        << "Menu3"
                                        << "Menu4");

    // Remove item 2
    script.walk(1);
    checkModelSort(model, QStringList() << "Menu0"
                                        << "Menu1"
                                        << "Menu3"
                                        << "Menu4");


    // Remove item 4
    script.walk(1);
    checkModelSort(model, QStringList() << "Menu0"
                                        << "Menu1"
                                        << "Menu3");

    // Add item 5
    script.walk(1);
    checkModelSort(model, QStringList() << "Menu0"
                                        << "Menu1"
                                        << "Menu3"
                                        << "Menu5");

    // Move item 0
    script.walk(1);
    checkModelSort(model, QStringList() << "Menu1"
                                        << "Menu3"
                                        << "Menu0"
                                        << "Menu5");

    script.stop();
}


QTEST_MAIN(DBusModelSortTest)

#include "dbusmodelsorttest.moc"
