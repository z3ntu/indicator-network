
#include "dbusmenuscript.h"
#include <QObject>
#include <QtTestGui>
#include <QDebug>


DBusMenuScript::DBusMenuScript()
    :m_script(0)
{
}

DBusMenuScript::~DBusMenuScript()
{
    stop();
}

void DBusMenuScript::start()
{
    QTest::qWait(500); 
    m_script = new QDBusInterface(SCRIPT_SERVICE_NAME,
                                  SCRIPT_OBJECT_PATH,
                                  SCRIPT_INTERFACE_NAME,
                                  QDBusConnection::sessionBus(), 0);
    QVERIFY(m_script->isValid());
}

void DBusMenuScript::stop()
{
    if (m_script) {
        m_script->call("quit");
        delete m_script;
        m_script = 0;
    }
}

void DBusMenuScript::walk(int steps)
{
    m_script->call("walk", steps);
    QTest::qWait(500);
}

void DBusMenuScript::run()
{
    m_script->call("walk", -1);
    QTest::qWait(5000);
}


