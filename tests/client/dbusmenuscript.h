#ifndef DBUSMENUSCRIPT_H
#define DBUSMENUSCRITP_H

#include <QObject>
#include <QDBusInterface>

#define SCRIPT_SERVICE_NAME     "com.canonical.test"
#define SCRIPT_OBJECT_PATH      "/com/canonical/test/menuscript"
#define SCRIPT_INTERFACE_NAME   "com.canonical.test.menuscript"

#define MENU_SERVICE_NAME       SCRIPT_SERVICE_NAME ".menu"
#define MENU_OBJECT_PATH        SCRIPT_OBJECT_PATH "/menu"


class DBusMenuScript
{
public:
    DBusMenuScript();
    ~DBusMenuScript();

    void start();
    void walk(int steps = 1);
    void run();
    void stop();

private:
    QDBusInterface *m_script;
};

#endif
