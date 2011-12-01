#include "plugin.h"
#include "menustack.h"
#include "gradientrectangle.h"

#include <QtDeclarative>

void SystemUIQmlPlugin::registerTypes(const char *uri)
{
    qmlRegisterType<MenuStack>(uri, 0, 1, "MenuStack");
    qmlRegisterType<GradientRectangle>(uri, 0, 1, "GradientRectangle");
}

Q_EXPORT_PLUGIN2(systemuiqml, SystemUIQmlPlugin)
