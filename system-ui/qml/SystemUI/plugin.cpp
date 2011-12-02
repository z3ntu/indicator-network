#include "plugin.h"
#include "pagestack.h"
#include "gradientrectangle.h"

#include <QtDeclarative>

void SystemUIQmlPlugin::registerTypes(const char *uri)
{
    qmlRegisterType<PageStack>(uri, 0, 1, "PageStack");
    qmlRegisterType<GradientRectangle>(uri, 0, 1, "GradientRectangle");
}

Q_EXPORT_PLUGIN2(systemuiqml, SystemUIQmlPlugin)
