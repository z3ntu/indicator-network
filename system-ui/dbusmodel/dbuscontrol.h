#ifndef DBUSCONTROL_H
#define DBUSCONTROL_H

#include <QAction>
#include <QAbstractListModel>
#include <QDBusPendingCallWatcher>

#include "dbusmodel.h"
#include "dbusmenutypes_p.h"

typedef QList<QAction* > ItemList;

class DBusMenuInterface;

class DBusControl : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString service READ service WRITE setService NOTIFY serviceChanged)
    Q_PROPERTY(QString objectPath READ objectPath WRITE setObjectPath NOTIFY objectPathChanged)
    Q_PROPERTY(bool connected READ isConnected NOTIFY connectionChanged)
public:
    Q_ENUMS(EventType)
    enum EventType {
        Clicked,
        Hovered,
        Openend,
        Closed
    };

    DBusControl(QObject *parent = 0);
    ~DBusControl();



    /* Connectc to dbusmenu server */
    Q_INVOKABLE bool connectToServer();
    Q_INVOKABLE void disconnectFromServer();


    Q_INVOKABLE void load(int id);
    Q_INVOKABLE void sendEvent(int id, EventType eventType);

    /* Properties */
    QString service() const;
    QString objectPath() const;
    bool isConnected();

    void setService(const QString &service);
    void setObjectPath(const QString &objectPath);

signals:
    void connectionChanged();
    void serviceChanged();
    void objectPathChanged();

    // Load reply
    void entryLoaded(int id, QList<QAction*> entries);

private slots:
    void onItemsPropertiesUpdated(const DBusMenuItemList &updatedList, const DBusMenuItemKeysList &removedList);
    void onItemActivationRequested(int id, uint timestamp);
    void onLayoutUpdated(uint, int);

    //Async reply
    void onGetLayoutReply(QDBusPendingCallWatcher*);

private:
    Q_DISABLE_COPY(DBusControl)

    DBusMenuInterface *m_interface;
    QHash<int, QAction* > m_actions;
    QString m_service;
    QString m_objectPath;

    QAction* parseAction(int id, const QVariantMap &_map, QWidget *parent);
    void updateActionProperty(QAction *action, const QString &key, const QVariant &value);
    void updateAction(QAction *action, const QVariantMap &map, const QStringList &requestedProperties);
    void updateActionIcon(QAction *action, const QString &iconName);
};

#endif
