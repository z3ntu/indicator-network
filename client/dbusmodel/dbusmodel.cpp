#include "qdbusmenuitem.h"
#include "dbusmodel.h"
#include "dbuscontrol.h"
#include "dbusmenuconst.h"

#include <QDebug>
#include <QAction>


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
        rolesNames[Label] = "label";
        rolesNames[State] = "state";
        rolesNames[HasSubmenu] = "hasSubmenu";
        rolesNames[Control] = "control";
        rolesNames[Data] = "data";
        rolesNames[Properties] = "properties";
    }
    setRoleNames(rolesNames);
}

DBusModel::~DBusModel()
{
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
            appendItems(m_root->children());
        }
    }
}

void DBusModel::appendItems(QObjectList items)
{
    Q_FOREACH(QObject *item, items) {
        QDBusMenuItem *menuItem = qobject_cast<QDBusMenuItem *>(item);

        if (menuItem->type() != QDBusMenuItem::Unknow) {
            appendItem(menuItem);
        } else {
            QObject::connect(item, SIGNAL(typeDiscovered()), this, SLOT(onItemTypeDiscovered()));
        }
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
    int position = menuItem->position();

    //QObjectList cpyItems = m_items;
    //cpyItems.insert(position, obj);
    //int row = cpyItems.indexOf(obj);
    //qDebug() << "InsertItem" << item->property(DBUSMENU_PROPERTY_LABEL) << "pos:" << position << "Row:" << row;
    m_items.insert(position, obj);
    //int row = m_items.indexOf(obj);
    //beginInsertRows(QModelIndex(), row, row);
    //endInsertRows();
    reset();
    QObject::connect(obj, SIGNAL(changed()), this, SLOT(onItemChanged()));
    QObject::connect(obj, SIGNAL(moved(int, int)), this, SLOT(onItemMoved(int, int)));
    QObject::connect(obj, SIGNAL(destroyed(QObject*)), this, SLOT(onItemDestroyed(QObject*)));
}

void DBusModel::onItemDestroyed(QObject * obj)
{
    QDBusMenuItem *item = qobject_cast<QDBusMenuItem*>(obj);
    int row = m_items.indexOf(obj);
    if (row >= 0) {
        beginRemoveRows(QModelIndex(), row, row);
        m_items.removeAt(row);
        endRemoveRows();
    }
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

bool DBusModel::eventFilter(QObject *obj, QEvent *event)
{
    return QObject::eventFilter(obj, event);
}

/* QAbstractItemModel */
int DBusModel::columnCount(const QModelIndex &parent) const
{
    return 1;
}

QVariant DBusModel::data(const QModelIndex &index, int role) const
{
    int row = index.row();
    if(!index.isValid() || (row < 0) || (row > m_items.size()))
        return QVariant();

    QDBusMenuItem *item = qobject_cast<QDBusMenuItem*>(m_root->children()[row]);
    Q_ASSERT(item);
    switch (role) {
    case Id:
        return item->property(DBUSMENU_PROPERTY_ID);
    case Type:
        return item->typeName();
    case Label:
        return item->property(DBUSMENU_PROPERTY_LABEL);
    case State:
        return item->property(DBUSMENU_PROPERTY_STATE);
    case Control:
        return QVariant::fromValue<QObject*>(m_control);
    case HasSubmenu:
        return item->property(DBUSMENU_PROPERTY_HAS_SUBMENU).isValid() &&
               item->property(DBUSMENU_PROPERTY_HAS_SUBMENU).toBool();
    case Data:
        return item->data();
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
