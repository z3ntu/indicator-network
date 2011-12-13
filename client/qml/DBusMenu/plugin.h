#ifndef SYSTEMUIQMLPLUGIN_H
#define SYSTEMUIQMLPLUGIN_H

#include <QDeclarativeExtensionPlugin>

class SystemUIQmlPlugin : public QDeclarativeExtensionPlugin
 {
     Q_OBJECT
 public:
     void registerTypes(const char *uri);
 };

#endif
