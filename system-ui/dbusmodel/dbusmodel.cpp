#include "qdbusmenuitem.h"
#include "dbusmodel.h"
#include "dbuscontrol.h"
#include "dbusmenuconst.h"

#include <QDebug>
#include <QAction>

DBusModel::DBusModel(QObject *parent)
    : QAbstractListModel(parent),
      m_id(-1),
      m_control(0)
{

    static QHash<int, QByteArray> rolesNames;
    if (rolesNames.empty()) {
        rolesNames[Id] = "menuId";
        rolesNames[Type] = "type";
        rolesNames[Label] = "label";
        rolesNames[Icon] = "icon";
        rolesNames[State] = "state";
        rolesNames[HasSubmenu] = "hasSubmenu";
        rolesNames[Control] = "control";
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

QObject *DBusModel::control() const
{
    return m_control;
}

void DBusModel::setMenuId(int id)
{
    if (m_id != id) {
        m_id = id;
        Q_EMIT menuIdChanged();
    }
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
        if (m_root)
            delete m_root;
        m_root = m_control->load(m_id);
        if (m_root) {
            m_root->installEventFilter(this);
            appendItems(m_root->children());
        }
    }
}

void DBusModel::appendItems(QObjectList items)
{

    beginInsertRows(QModelIndex(), m_count, m_count + items.size() - 1);
    Q_FOREACH(QObject *item, items) {
        m_count++;
        QObject::connect(item, SIGNAL(changed()), this, SLOT(onItemChanged()));
        QObject::connect(item, SIGNAL(destroyed(QObject*)), this, SLOT(onItemDestroyed(QObject*)));
        QObject::connect(item, SIGNAL(moved(int, int)), this, SLOT(onItemMoved(int, int)));
    }
    endInsertRows();
}

void DBusModel::onItemDestroyed(QObject *obj)
{
    QDBusMenuItem *item = qobject_cast<QDBusMenuItem*>(obj);
    int row = item->position();
    if (row >= 0) {
        beginRemoveRows(QModelIndex(), row, row);
        endRemoveRows();
    }
}

void DBusModel::onItemMoved(int newPos, int oldPos)
{
    beginMoveRows(QModelIndex(), oldPos, oldPos, QModelIndex(), newPos);
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
    if(!index.isValid() || (row < 0) || (row > m_count))
        return QVariant();

    QObject *item = m_root->children()[row];
    Q_ASSERT(item);
    switch (role) {
    case Id:
        return item->property(DBUSMENU_PROPERTY_ID);
    case Type:
        switch(item->property(DBUSMENU_PROPERTY_TYPE).toInt()) {
        case QDBusMenuItem::TextEntry:
            return QVariant("TextEntry");
        case QDBusMenuItem::ToggleButton:
            return QVariant("ToggleButton");
        case QDBusMenuItem::RadioButton:
            return QVariant("RadioButton");
        case QDBusMenuItem::Label:
        default:
            return QVariant("Label");
        }
    case Label:
        return item->property(DBUSMENU_PROPERTY_LABEL);
    case Icon:
        return item->property(DBUSMENU_PROPERTY_ICON_NAME);
    case State:
        return item->property(DBUSMENU_PROPERTY_STATE);
    case Control:
        return QVariant::fromValue<QObject*>(m_control);
    case HasSubmenu:
        return item->property(DBUSMENU_PROPERTY_HAS_SUBMENU).isValid() &&
               item->property(DBUSMENU_PROPERTY_HAS_SUBMENU).toBool();
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
    return m_count;
}
