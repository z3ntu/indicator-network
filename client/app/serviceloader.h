#ifndef SERVICELOADER_H
#define SERVICELOADER_H

#include <QObject>
#include <QList>
#include <QHash>

struct ServiceData;

class ServiceLoader : public QObject
{
    Q_OBJECT
public:
    ServiceLoader(const QString & prefix, QObject * parent = 0);
    ~ServiceLoader();
    int count() const;
    QHash<QString, QVariant> service(int index);

private:
    QList<ServiceData *> m_services;

    void load(const QString & prefix);
};

#endif
