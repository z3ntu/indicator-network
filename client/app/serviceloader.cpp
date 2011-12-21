#include "serviceloader.h"

#include <QDir>
#include <QString>
#include <QSettings>
#include <QDebug>

#define SYSTEM_SETTINGS_GROUP_NAME      "System Settings Servirce"
#define SYSTEM_SETTINGS_TITLE_KEY       "title"
#define SYSTEM_SETTINGS_NAME_KEY        "name"
#define SYSTEM_SETTINGS_PATH_KEY        "path"

struct ServiceData
{
    QString m_title;
    QString m_service;
    QString m_objectPath;

    ServiceData(const QString & title, const QString & service, const QString & objectPath)
        : m_title(title),
          m_service(service),
          m_objectPath(objectPath)
    {
    }
};

ServiceLoader::ServiceLoader(const QString &prefix, QObject * parent)
    : QObject(parent)
{
    load(prefix);
}

int ServiceLoader::count() const
{
    return m_services.count();
}

QHash<QString, QVariant> ServiceLoader::service(int index)
{
    QHash<QString, QVariant> serviceInfo;

    if ((index >= 0)  && (index < m_services.count())) {
        ServiceData *data = m_services[index];

        serviceInfo["description"] = data->m_title;
        serviceInfo["name"] = data->m_service;
        serviceInfo["path"] = data->m_objectPath;
    }
    return serviceInfo;
}

ServiceLoader::~ServiceLoader()
{
    Q_FOREACH(ServiceData * service, m_services)
        delete service;
    m_services.clear();
}

void ServiceLoader::load(const QString & prefix)
{
    QDir dir(prefix, "*.service");
    Q_FOREACH(QString fileName, dir.entryList()) {
        QSettings settings(dir.filePath(fileName), QSettings::IniFormat);
        if (settings.childGroups().contains(SYSTEM_SETTINGS_GROUP_NAME)) {
            settings.beginGroup(SYSTEM_SETTINGS_GROUP_NAME);
            m_services << new ServiceData(settings.value(SYSTEM_SETTINGS_TITLE_KEY).toString(),
                                          settings.value(SYSTEM_SETTINGS_NAME_KEY).toString(),
                                          settings.value(SYSTEM_SETTINGS_PATH_KEY).toString());
        } else {
            qWarning() << "File: "  << fileName << " does not contain " << SYSTEM_SETTINGS_GROUP_NAME << "group";
        }
    }
}
