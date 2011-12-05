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
        rolesNames[Type] = "type";
        rolesNames[Title] = "title";
        rolesNames[Action] = "action";
        rolesNames[HasSubmenu] = "hasSubmenu";
        rolesNames[IsCheckable] = "checkable";
        rolesNames[IsChecked] = "checked";
        rolesNames[Control] = "control";
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
        QObject::connect(m_control, SIGNAL(entryLoaded(int,QList<QAction*>)), this, SLOT(onEntryLoaded(int, QList<QAction*>)));
        m_control->load(m_id);
    }
}

void DBusModel::onEntryLoaded(int id, QList<QAction*> items)
{
    if (id == m_id) {
        beginInsertRows(QModelIndex(), m_actions.size(), m_actions.size() + items.size() - 1);
        Q_FOREACH(QAction* act, items) {
            if (!m_actions.contains(act)) {
                m_actions << act;
                QObject::connect(act, SIGNAL(changed()), this, SLOT(onActionChanged()));
            }
        }
        endInsertRows();
    }
}

void DBusModel::onActionChanged()
{
    QAction *act = qobject_cast<QAction*>(QObject::sender());
    int row = m_actions.indexOf(act);
    if (row >=0)
        dataChanged(index(row), index(row));
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
    case Type:
        return act->property(DBUSMENU_PROPERTY_GSETTINGS_TYPE);
    case Title:
        return act->text();
    case IsCheckable:
        return act->isCheckable();
    case IsChecked:
        return act->isChecked();
    case Action:
        return QVariant::fromValue<QObject*>(act);
    case Control:
        return QVariant::fromValue<QObject*>(m_control);
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
