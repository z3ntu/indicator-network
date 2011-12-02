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
        DBusMenuTypes_register();
        rolesNames[Id] = "id";
        rolesNames[Title] = "title";
        rolesNames[Action] = "action";
        rolesNames[HasSubmenu] = "hasSubmenu";
        rolesNames[Checkable] = "checkable";
    }
    setRoleNames(rolesNames);
}

DBusModel::~DBusModel()
{
    m_actions.clear();
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
        emit menuIdChanged();
    }
}

void DBusModel::setControl(QObject *control)
{
    if (m_control != control) {
        m_control = qobject_cast<DBusControl*>(control);
        emit controlChanged();
    }
}

void DBusModel::load()
{
    if ((m_id > -1) && m_control) {
        QObject::connect(m_control, SIGNAL(entryLoaded(int,QList<QAction*>)), this, SLOT(onEntryLoaded(int, QList<QAction*>)));
        m_control->load(m_id);
    }
}

void DBusModel::onEntryLoaded(int id, QList<QAction*> items)
{
    if (id == m_id) {
        beginInsertRows(QModelIndex(), m_actions.size(), m_actions.size() + items.size() - 1);
        foreach(QAction* act, items) {
            if (!m_actions.contains(act))
                m_actions << act;
        }
        endInsertRows();
    }
}

/* QAbstractItemModel */
int DBusModel::columnCount(const QModelIndex &parent) const
{
    return 1;
}

QVariant DBusModel::data(const QModelIndex &index, int role) const
{
    int row = index.row();
    if(!index.isValid() || (row < 0) || (row > m_actions.count()))
        return QVariant();

    QAction *act = m_actions[row];
    Q_ASSERT(act);
    switch (role) {
    case Id:
        return act->property(DBUSMENU_PROPERTY_ID);
    case Title:
        return act->text();
    case Checkable:
        return act->isCheckable();
    case Action:
        return QVariant::fromValue<QObject*>(act);
    case HasSubmenu:
        return act->property(DBUSMENU_PROPERTY_HAS_SUBMENU).isValid() &&
               act->property(DBUSMENU_PROPERTY_HAS_SUBMENU).toBool();
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
    return m_actions.count();
}
