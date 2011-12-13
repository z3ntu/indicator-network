#ifndef DBUSMODELTEST_H
#define DBUSMODELTEST_H

#include <QObject>

class DBusModelTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testInvalidConnection();
    void testDumpMenu();
};

#endif
