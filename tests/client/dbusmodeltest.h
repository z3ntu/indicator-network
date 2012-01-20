#ifndef DBUSMODELTEST_H
#define DBUSMODELTEST_H

#include <QObject>
#include <QDBusInterface>

class DBusModelTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testInvalidConnection();
    void testSingleMenu();
};

#endif
