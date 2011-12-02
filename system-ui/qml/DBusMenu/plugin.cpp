#include "plugin.h"
#include "dbusmodel.h"

#include <QtDeclarative>

void SystemUIQmlPlugin::registerTypes(const char *uri)
{
    qmlRegisterType<DBusModel>(uri, 1, 0, "DBusMenuClient");
}

Q_EXPORT_PLUGIN2(systemuiqml, SystemUIQmlPlugin)
