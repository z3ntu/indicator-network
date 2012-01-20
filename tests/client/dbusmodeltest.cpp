#include "dbusmodeltest.h"
#include "dbusmodel.h"
#include "dbuscontrol.h"
#include "dbusmenuscript.h"

#include <QObject>
#include <QtTestGui>
#include <QDebug>

/*
 * Check safe exit when connecting to a invalid connection
 */
void DBusModelTest::testInvalidConnection()
{
    DBusControl control;
    control.setService("org.dbusmenu.invalid.test");
    control.setObjectPath("/org/test");
    control.connectToServer();

    QTest::qWait(500);
    QVERIFY(control.isConnected() == false);
}


/*
 * Check item added and item removed in runtinme for DBusModel
 */
void DBusModelTest::testSingleMenu()
{
    DBusMenuScript script;
    script.start();

    // Create control object
    DBusControl control;
    control.setService(MENU_SERVICE_NAME);
    control.setObjectPath(MENU_OBJECT_PATH);
    control.connectToServer();

    // Check if control is connected
    QTest::qWait(500);
    QVERIFY(control.isConnected());

    // Create Model
    DBusModel model;
    model.setControl(&control);
    model.setMenuId(0);

    // Load Model
    model.load();

    // Model is empty in the begginer
    QCOMPARE(model.count(), 0);

    // Insert 5 intems into model
    script.walk(5);
    QCOMPARE(model.count(), 5);

    // Remove one item
    script.walk(1);
    QCOMPARE(model.count(), 4);

    // Remove one item
    script.walk(1);
    QCOMPARE(model.count(), 3);

    // Add a new item
    script.walk(1);
    QCOMPARE(model.count(), 4);

    // Leave script
    script.stop();
}


QTEST_MAIN(DBusModelTest)

#include "dbusmodeltest.moc"
