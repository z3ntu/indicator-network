#ifndef DBUSCONTROL_H
#define DBUSCONTROL_H

#include <QAbstractListModel>
#include <libdbusmenu-glib/client.h>

#include "dbusmodel.h"

class QDbusMenuItem;

class DBusControl : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString service READ service WRITE setService NOTIFY serviceChanged)
    Q_PROPERTY(QString objectPath READ objectPath WRITE setObjectPath NOTIFY objectPathChanged)
    Q_PROPERTY(bool connected READ isConnected NOTIFY rootChanged)
public:
    Q_ENUMS(EventType)
    enum EventType {
        Clicked,
        Hovered,
        Openend,
        Closed,
        TextChanged
    };

    DBusControl(QObject *parent = 0);
    ~DBusControl();

    /* Connectc to dbusmenu server */
    Q_INVOKABLE bool connectToServer();
    Q_INVOKABLE void disconnectFromServer();

    Q_INVOKABLE QDBusMenuItem *load(int id);
    Q_INVOKABLE void sendEvent(int id, EventType eventType, QVariant data = QVariant());

    /* Properties */
    QString service() const;
    QString objectPath() const;
    bool isConnected();

    void setService(const QString &service);
    void setObjectPath(const QString &objectPath);

Q_SIGNALS:
    void rootChanged();
    void serviceChanged();
    void objectPathChanged();

private:
    Q_DISABLE_COPY(DBusControl)

    DbusmenuClient *m_client;
    QDBusMenuItem *m_root;
    QString m_service;
    QString m_objectPath;

    void registeItem(int id, QObject *item);

    static void onRootChanged(DbusmenuClient *client, DbusmenuMenuitem *newroot, gpointer data);
};

#endif
