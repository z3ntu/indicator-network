#ifndef DBUSMODEL_H
#define DBUSMODEL_H

#include <QAction>
#include <QAbstractListModel>
#include <QDBusPendingCallWatcher>

#include "dbusmenutypes_p.h"

typedef QList<QAction* > ItemList;

class DBusMenuInterface;

class DBusModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(bool connected READ isConnected NOTIFY connectionChanged)
public:
    DBusModel(QObject *parent = 0);
    ~DBusModel();

    /* Avaliable fields */
    enum MenuRoles {
        Id,
        Title,
        Action,
        HasSubmenu,
        Submenu
    };

    /* Connectc to dbusmenu server */
    Q_INVOKABLE bool connectToServer(const QString &service, const QString &objectPath);
    Q_INVOKABLE void disconnectFromServer();
    bool isConnected();

    /* QAbstractItemModel */
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QModelIndex parent (const QModelIndex &index) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;

signals:
    void connected();
    void disconnected();
    void connectionChanged();

private Q_SLOTS:
    void onLayoutUpdated(uint, int);
    void onItemActivationRequested(int, uint);
    void onItemsPropertiesUpdated(const DBusMenuItemList &updatedList, const DBusMenuItemKeysList &removedList);

    //Async reply
    void onGetLayoutReply(QDBusPendingCallWatcher*);

private:
    Q_DISABLE_COPY(DBusModel)

    int m_id;
    DBusMenuInterface *m_interface;
    ItemList m_actions;
    static QHash<int, QAction* > m_allActions;

    DBusModel *submenu(QAction *act) const;

    void append(QList<QAction*> items);
    void loadMenu(int id);
    void updateActionProperty(QAction *action, const QString &key, const QVariant &value);
    QAction* parseAction(int id, const QVariantMap &_map, QWidget *parent);
    void updateAction(QAction *action, const QVariantMap &map, const QStringList &requestedProperties);
    void updateActionIcon(QAction *action, const QString &iconName);
};

#endif
