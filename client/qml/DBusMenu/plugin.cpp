#include "plugin.h"
#include "dbusmodel.h"
#include "dbuscontrol.h"

#include <QtDeclarative>

void SystemUIQmlPlugin::registerTypes(const char *uri)
{
    qmlRegisterType<DBusControl>(uri, 1, 0, "DBusMenuClientControl");
    qmlRegisterType<DBusModel>(uri, 1, 0, "DBusMenuClientModel");
}

Q_EXPORT_PLUGIN2(systemuiqml, SystemUIQmlPlugin)
