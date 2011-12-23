#include "plugin.h"
#include "gradientrectangle.h"

#include <QtDeclarative>

void SystemUIQmlPlugin::registerTypes(const char *uri)
{
    qmlRegisterType<GradientRectangle>(uri, 1, 0, "GradientRectangle");
}

Q_EXPORT_PLUGIN2(systemuiqml, SystemUIQmlPlugin)
