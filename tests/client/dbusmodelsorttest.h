#ifndef DBUSMODELSORTTEST_H
#define DBUSMODELSORTTEST_H

#include <QObject>
#include <QDBusInterface>

class DBusModelSortTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testMenuOrder();
};

#endif
