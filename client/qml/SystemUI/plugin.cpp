#include "plugin.h"
#include "gradientrectangle.h"
#include "pagestack.h"

#include <QtDeclarative>

void SystemUIQmlPlugin::registerTypes(const char *uri)
{
    qmlRegisterType<GradientRectangle>(uri, 1, 0, "GradientRectangle");
    qmlRegisterType<PageStack>(uri, 1, 0, "PageStack");
}

Q_EXPORT_PLUGIN2(systemuiqml, SystemUIQmlPlugin)
