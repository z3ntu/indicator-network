#ifndef DBUSMODEL_H
#define DBUSMODEL_H

#include <QAction>
#include <QAbstractListModel>

class QDBusMenuItem;
class DBusControl;

/*!
    \class DBusModel
    \brief The DBusModel class provides a menu model for dbusmenu

    This classes provide information about the menu properties
*/
class DBusModel : public QAbstractListModel
{
    Q_OBJECT

    /*!
        \property menuId
        this property is used to specify the menu id used to retrive information for the model
    */
    Q_PROPERTY(int menuId READ menuId WRITE setMenuId NOTIFY menuIdChanged)

    /*!
        \property control
        this property is used to attach a DBusControl to this model
    */
    Q_PROPERTY(QObject* control READ control WRITE setControl NOTIFY controlChanged)

    /*!
      \proeprty count
      The number of data entries in the model
    */
    Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
    //! Constructor
    DBusModel(QObject *parent = 0);

    //! Destructor
    ~DBusModel();

    //! Retrieve the menuId value
    int menuId() const;

    //! Retrive the menu control object utilized on the model
    QObject *control() const;

    //! Set the menuId property
    void setMenuId(int id);

    //! Set the control object utilized on the model
    void setControl(QObject *control);

    //! The number of data entries in the model
    int count() const;

    //! Load all necessary information from the dbusmenu
    Q_INVOKABLE void load();

    //! Proxy function for event from menu object
    bool eventFilter(QObject *obj, QEvent *event);

    /* QAbstractItemModel */
    //! Virtual implementation for QAbstractItemModel::columnCount
    int columnCount(const QModelIndex &parent = QModelIndex()) const;

    //! Virtual implementation for QAbstractItemModel::data
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    //! Virtual implementation for QAbstractItemModel::parent
    QModelIndex parent (const QModelIndex &index) const;

    //! Virtual implementation for QAbstractItemModel::rowCount
    int rowCount(const QModelIndex &parent = QModelIndex()) const;

Q_SIGNALS:
    //! Called when the menuId has changed
    void menuIdChanged();

    //! Called when the control object has changed
    void controlChanged();

    //! Called when the number of intens inside of the model changes
    void countChanged();

private Q_SLOTS:
    void onItemChanged();
    void onItemDestroyed(QObject *obj);
    void onItemMoved(int newPos, int oldPos);
    void onItemTypeDiscovered();

private:
    Q_DISABLE_COPY(DBusModel)

    QObjectList m_items;
    QDBusMenuItem *m_root;
    DBusControl *m_control;
    int m_id;

    //! Avaliable fields
    enum MenuRoles {
        Id,
        Type,
        ToggleType,
        Label,
        State,
        HasSubmenu,
        IsInline,
        Visible,
        Data,
        Properties,
        Control
    };

    //! Append a list of object in the model
    void appendItems(QObjectList items);

    //! Append one object in the model
    void appendItem(QObject * obj);

    //! Help function for get item property
    QVariant getProperty(QDBusMenuItem * item, QByteArray name, QVariant defaultValue) const;
};
#endif
