#include "plugin.h"
#include "pagestack.h"
#include "gradientrectangle.h"

#include <QtDeclarative>

void SystemUIQmlPlugin::registerTypes(const char *uri)
{
    qmlRegisterType<PageStack>(uri, 1, 0, "PageStack");
    qmlRegisterType<GradientRectangle>(uri, 1, 0, "GradientRectangle");
}

Q_EXPORT_PLUGIN2(systemuiqml, SystemUIQmlPlugin)
