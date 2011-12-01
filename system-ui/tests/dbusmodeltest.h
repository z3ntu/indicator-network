#ifndef DBUSMODELTEST_H
#define DBUSMODELTEST_H

#include <QObject>

class DBusModelTest : public QObject
{
    Q_OBJECT
private slots:
    void testDumpMenu();
};

#endif
