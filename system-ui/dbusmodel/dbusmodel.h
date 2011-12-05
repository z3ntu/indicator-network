#ifndef DBUSMODEL_H
#define DBUSMODEL_H

#include <QAction>
#include <QAbstractListModel>

typedef QList<QAction* > ItemList;
class DBusControl;

class DBusModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int menuId READ menuId WRITE setMenuId NOTIFY menuIdChanged)
    Q_PROPERTY(QObject* control READ control WRITE setControl NOTIFY controlChanged)

public:
    DBusModel(QObject *parent = 0);
    ~DBusModel();

    /* Properties */
    int menuId() const;
    QObject *control() const;

    void setMenuId(int id);
    void setControl(QObject *control);

    Q_INVOKABLE void load();

    /* Avaliable fields */
    enum MenuRoles {
        Id,
        Type,
        Title,
        Data,
        IsCheckable,
        HasSubmenu,
        IsChecked,
        Action,
        Control
    };

    /* QAbstractItemModel */
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QModelIndex parent (const QModelIndex &index) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;

Q_SIGNALS:
    void menuIdChanged();
    void controlChanged();

private Q_SLOTS:
    void onEntryLoaded(int id, QList<QAction*> items);
    void onActionChanged();

private:
    Q_DISABLE_COPY(DBusModel)

    int m_id;
    ItemList m_actions;
    DBusControl *m_control;
};

#endif
