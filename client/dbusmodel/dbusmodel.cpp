#include "qdbusmenuitem.h"
#include "dbusmodel.h"
#include "dbuscontrol.h"
#include "dbusmenuconst.h"

#include <QDebug>
#include <QAction>
#include <QThread>
#include <QAbstractItemModel>
#include <QChildEvent>

DBusModel::DBusModel(QObject *parent)
    : QAbstractListModel(parent),
      m_id(-1),
      m_control(0),
      m_root(0)
{

    static QHash<int, QByteArray> rolesNames;
    if (rolesNames.empty()) {
        rolesNames[Id] = "menuId";
        rolesNames[Type] = "type";
        rolesNames[ToggleType] = "toggleType";
        rolesNames[Label] = "label";
        rolesNames[State] = "state";
        rolesNames[HasSubmenu] = "hasSubmenu";
        rolesNames[IsInline] = "isInline";
        rolesNames[Visible] = "visible";
        rolesNames[Control] = "control";
        rolesNames[Data] = "data";
        rolesNames[Properties] = "properties";
    }
    setRoleNames(rolesNames);
    QObject::connect(this, SIGNAL(rowsInserted(const QModelIndex&, int, int)),
                     this, SIGNAL(countChanged()));
    QObject::connect(this, SIGNAL(rowsRemoved(const QModelIndex &, int, int)),
                     this, SIGNAL(countChanged()));
}

DBusModel::~DBusModel()
{
    Q_FOREACH(QDBusMenuItem *item, m_pendingItems) {
        removeItem(item);
    }

    Q_FOREACH(QObject *item, m_items) {
        QDBusMenuItem *mItem = qobject_cast<QDBusMenuItem*>(item);
        removeItem(mItem);
    }

    // Do not destroy item this will be destroyed by DBusControl
    m_root = 0;

}

int DBusModel::menuId() const
{
    return m_id;
}

void DBusModel::setMenuId(int id)
{
    if (m_id != id) {
        m_id = id;
        Q_EMIT menuIdChanged();
    }
}

QObject *DBusModel::control() const
{
    return m_control;
}

void DBusModel::setControl(QObject *control)
{
    if (m_control != control) {
        m_control = qobject_cast<DBusControl*>(control);
        Q_EMIT controlChanged();
    }
}

int DBusModel::count() const
{
    return rowCount();
}

void DBusModel::load()
{
    if ((m_id > -1) && m_control) {
        if (m_root) {
            m_root->removeEventFilter(this);
            reset();
        }

        m_root = m_control->load(m_id);
        if (m_root) {
            m_root->installEventFilter(this);
            appendNewItems(m_root->children());
        }
    }
}

void DBusModel::appendNewItem(QDBusMenuItem * item)
{
    if (!item->type().isEmpty())  {
        appendItem(item);
    } else {
        m_pendingItems << item;
        QObject::connect(item, SIGNAL(typeDiscovered()), this, SLOT(onItemTypeDiscovered()));
    }
}

void DBusModel::appendNewItems(QObjectList items)
{
    Q_FOREACH(QObject *item, items) {
        QDBusMenuItem *menuItem = qobject_cast<QDBusMenuItem *>(item);
        appendNewItem(menuItem);
    }
}

void DBusModel::onItemTypeDiscovered()
{
    QObject *sender = QObject::sender();
    QObject::disconnect(sender, SIGNAL(typeDiscovered()), this, SLOT(onItemTypeDiscovered()));
    appendItem(sender);
}

void DBusModel::appendItem(QObject * obj)
{
    QDBusMenuItem *menuItem = qobject_cast<QDBusMenuItem*>(obj);

    //remove from pending
    m_pendingItems.removeOne(menuItem);

    int position = menuItem->position();

    QObjectList cpyItems = m_items;
    cpyItems.insert(position, obj);
    int row = cpyItems.indexOf(obj);
    beginInsertRows(QModelIndex(), row, row);

    m_items.insert(position, obj);
    QObject::connect(obj, SIGNAL(changed()), this, SLOT(onItemChanged()));
    QObject::connect(obj, SIGNAL(moved(int, int)), this, SLOT(onItemMoved(int, int)));
    QObject::connect(obj, SIGNAL(destroyed(QObject*)), this, SLOT(onItemDestroyed(QObject*)));

    endInsertRows();
    //reset();
}

void DBusModel::removeItem(QDBusMenuItem * item)
{
    int index = m_pendingItems.indexOf(item);
    if (index >= 0) {
        QObject::disconnect(item, SIGNAL(typeDiscovered()), this, SLOT(onItemTypeDiscovered()));
        m_pendingItems.takeAt(index);
    } else {
        int row = m_items.indexOf(item);
        if (row >= 0) {
            QObject::disconnect(item, SIGNAL(changed()), this, SLOT(onItemChanged()));
            QObject::disconnect(item, SIGNAL(moved(int, int)), this, SLOT(onItemMoved(int, int)));
            QObject::disconnect(item, SIGNAL(destroyed(QObject*)), this, SLOT(onItemDestroyed(QObject*)));

            beginRemoveRows(QModelIndex(), row, row);
            m_items.removeAt(row);
            endRemoveRows();
        }
    }
}

void DBusModel::onItemDestroyed(QObject * obj)
{
    QDBusMenuItem *item = qobject_cast<QDBusMenuItem*>(obj);
    removeItem(item);
}

void DBusModel::onItemMoved(int newPos, int oldPos)
{
    beginMoveRows(QModelIndex(), oldPos, oldPos, QModelIndex(), newPos);
    m_items.move(oldPos, newPos);
    endMoveRows();
}

void DBusModel::onItemChanged()
{
    QDBusMenuItem *item = qobject_cast<QDBusMenuItem*>(QObject::sender());
    int row = item->position();
    if (row >=0)
        dataChanged(index(row), index(row));
}

bool DBusModel::eventFilter(QObject * obj, QEvent * event)
{
    switch(event->type())
    {
        case QEvent::ChildAdded:
        {
            QChildEvent *cEvent = reinterpret_cast<QChildEvent*>(event);
            QDBusMenuItem *item = qobject_cast<QDBusMenuItem*>(cEvent->child());

            Q_ASSERT(item);
            appendNewItem(item);
            break;
        }
        case QEvent::ChildRemoved: {
            QChildEvent *cEvent = reinterpret_cast<QChildEvent*>(event);
            onItemDestroyed(cEvent->child());
            break;
        }
    }
    return QObject::eventFilter(obj, event);
}

/* QAbstractItemModel */
int DBusModel::columnCount(const QModelIndex & parent) const
{
    return 1;
}

QVariant DBusModel::getProperty(QDBusMenuItem * item, QByteArray name, QVariant defaultValue) const
{
    QVariant prop = item->property(name);
    if (prop.isValid()) {
        return prop;
    } else {
        return defaultValue;
    }
}

QVariant DBusModel::data(const QModelIndex & index, int role) const
{
    int row = index.row();
    if(!index.isValid() || (row < 0) || (row > m_items.size()))
        return QVariant();

    QDBusMenuItem *item = qobject_cast<QDBusMenuItem*>(m_items[row]);
    Q_ASSERT(item);
    switch (role) {
    case Id:
        return item->id();
    case Type:
        return item->type();
    case ToggleType:
        return getProperty(item, DBUSMENU_MENUITEM_PROP_TOGGLE_TYPE, "");
    case Label:
        return getProperty(item, DBUSMENU_MENUITEM_PROP_LABEL, "");
    case State:
        return getProperty(item, DBUSMENU_MENUITEM_PROP_TOGGLE_STATE, false);
    case Visible:
        return getProperty(item, DBUSMENU_MENUITEM_PROP_VISIBLE, true);
    case Control:
        return QVariant::fromValue<QObject*>(m_control);
    case HasSubmenu:
        return item->hasSubMenu();
    case IsInline:
        return item->isInline();
    case Properties:
        return item->extraProperties();
    default:
        qWarning() << "Invalid role name" << role;
        break;
    }
}

QModelIndex DBusModel::parent(const QModelIndex &) const
{
    return QModelIndex();
}

int DBusModel::rowCount(const QModelIndex &) const
{
    return m_items.size();
}
