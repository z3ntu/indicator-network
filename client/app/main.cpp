#include "serviceloader.h"

#include <QString>
#include <QApplication>
#include <QDeclarativeView>
#include <QGraphicsObject>
#include <QDeclarativeEngine>
#include <QDebug>

#define SERVICE_DIR_ARGUMENT    "--service-dir"
#define INCLUDE_DIR_ARGUMENT    "--include-dir"
#define DEFAULT_SERVICE_DIR     "/usr/share/system-settings/services"

int main(int argc, char ** argv)
{
    QString serviceDir = DEFAULT_SERVICE_DIR;
    QStringList importPaths;

    for(int i=1; i < argc; i++) {
        QString strArg(argv[i]);
        if (strArg.startsWith(SERVICE_DIR_ARGUMENT)) {
            serviceDir = strArg.split("=")[1];
        } else if (strArg.startsWith(INCLUDE_DIR_ARGUMENT)) {
            importPaths << strArg.split("=")[1];
        }
    }

    ServiceLoader *serviceLoader = new ServiceLoader(serviceDir);
    if (serviceLoader->count() <= 0) {
        qWarning() << "No services found in:" << serviceDir;
        delete serviceLoader;
        return -1;
    }

    QApplication *app = new QApplication(argc, argv);
    QDeclarativeView *view = new QDeclarativeView;
    Q_FOREACH(QString importPath, importPaths) {
        view->engine()->addImportPath(importPath);
    }

    view->setSource(QUrl("qrc:/qml/main.qml"));

    if (view->errors().size() > 0) {
        qWarning() << "Fail to load qml file:" << view->errors();
    } else {
        //Populate services
        QGraphicsObject *rootObject = view->rootObject();
        Q_ASSERT(rootObject);
        for(int i=0; i < serviceLoader->count(); i++) {
            QHash<QString, QVariant> service = serviceLoader->service(i);
            qDebug() << "Add Service:" << service["description"];

            QMetaObject::invokeMethod(rootObject, "addService",
                                      Q_ARG(QVariant, service["description"]),
                                      Q_ARG(QVariant, service["name"]),
                                      Q_ARG(QVariant, service["path"]));
        }

        view->resize(300, 600);
        view->show();
        app->exec();
    }

    delete view;
    delete app;
    return 0;
}
