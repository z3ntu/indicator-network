#include "plugin.h"
#include "dbusmodel.h"

#include <QtDeclarative>

void SystemUIQmlPlugin::registerTypes(const char *uri)
{
    qmlRegisterType<DBusModel>(uri, 0, 1, "DBusClient");
}

Q_EXPORT_PLUGIN2(systemuiqml, SystemUIQmlPlugin)
