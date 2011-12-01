#include "dbusmodel.h"
#include "dbusmenu.h"

#include <QDebug>
#include <QAction>
#include <QDBusAbstractInterface>
#include <QDBusPendingReply>
#include <QDBusInterface>

static const char *DBUSMENU_INTERFACE = "com.canonical.dbusmenu";
static const char *DBUSMENU_PROPERTY_ID = "dbusmenu_id";
static const char *DBUSMENU_PROPERTY_ICON = "dbusmenu_icon";
static const char *DBUSMENU_PROPERTY_HAS_SUBMENU = "dbusmenu_has_submenu";
static const char *DBUSMENU_PROPERTY_SUBMENU = "dbusmenu_submenu";

QHash<int, QAction* > DBusModel::m_allActions;

DBusModel::DBusModel(QObject *parent)
    : QAbstractListModel(parent),
      m_id(-1),
      m_interface(0)
{
    static QHash<int, QByteArray> rolesNames;
    if (rolesNames.empty()) {
        DBusMenuTypes_register();
        rolesNames[Id] = "id";
        rolesNames[Title] = "title";
        rolesNames[Action] = "action";
        rolesNames[HasSubmenu] = "hasSubmenu";
        rolesNames[Submenu] = "submenu";
    }
    setRoleNames(rolesNames);
}

DBusModel::~DBusModel()
{
    /* only root menu can destroy the interface */
    if (m_id == 0)
        disconnectFromServer();
    else
        m_interface = 0;

    foreach(QAction *act, m_actions) {
        int id = act->property(DBUSMENU_PROPERTY_ID).toInt();
        m_allActions.remove(id);
        delete act;
    }
    m_actions.clear();
}

DBusModel *DBusModel::submenu(QAction *act) const
{
    if (act->property(DBUSMENU_PROPERTY_HAS_SUBMENU).isValid()
        && act->property(DBUSMENU_PROPERTY_HAS_SUBMENU).toBool()) {
        if (act->property(DBUSMENU_PROPERTY_SUBMENU).isValid())
            return qobject_cast<DBusModel *>(act->property(DBUSMENU_PROPERTY_SUBMENU).value<QObject*>());

        DBusModel *submenu = new DBusModel(const_cast<DBusModel*>(this));
        submenu->m_interface = m_interface;
        submenu->loadMenu(act->property(DBUSMENU_PROPERTY_ID).toInt());
        act->setProperty(DBUSMENU_PROPERTY_SUBMENU, QVariant::fromValue<QObject*>(submenu));
        return submenu;
    } else {
        return 0;
    }
}

void DBusModel::loadMenu(int id)
{
    Q_ASSERT(m_interface);

    m_id = id;
    QDBusPendingCall call = m_interface->GetLayout(id, 1, QStringList());
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(call, this);
    /* Wait for reply */
    QObject::connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)), this, SLOT(onGetLayoutReply(QDBusPendingCallWatcher*)));
}

void DBusModel::onGetLayoutReply(QDBusPendingCallWatcher *ptrReply)
{
    QDBusPendingReply<uint, DBusMenuLayoutItem> reply = *ptrReply;
    if (!reply.isValid()) {
        qWarning() << "Invalid reply for GetLayou:" << reply.error().message();
        return;
    }

    QList<QAction* > actions;
    DBusMenuLayoutItem rootItem = reply.argumentAt<1>();
    Q_FOREACH(const DBusMenuLayoutItem &dbusMenuItem, rootItem.children) {
        QAction *act = parseAction(dbusMenuItem.id, dbusMenuItem.properties, 0);
        if (act)
            actions << act;
    }

    if (actions.size() > 0)
        append(actions);
}

QAction* DBusModel::parseAction(int id, const QVariantMap &_map, QWidget *parent)
{
    QVariantMap map = _map;
    QAction *action = new QAction(parent);
    action->setProperty(DBUSMENU_PROPERTY_ID, id);

    QString type = map.take("type").toString();
    if (type == "separator") {
        action->setSeparator(true);
    }

    updateAction(action, map, map.keys());
    return action;
}

void DBusModel::updateAction(QAction *action, const QVariantMap &map, const QStringList &requestedProperties)
{
    Q_FOREACH(const QString &key, requestedProperties) {
        updateActionProperty(action, key, map.value(key));
    }
}

void DBusModel::updateActionProperty(QAction *action, const QString &key, const QVariant &value)
{
    if (key == "label")
        action->setText(value.toString());
    else if (key == "enabled")
        action->setEnabled(value.isValid() ? value.toBool(): true);
    else if (key == "icon-name")
        updateActionIcon(action, value.toString());
    else if (key == "visible")
        action->setVisible(value.isValid() ? value.toBool() : true);
    else if (key == "children-display")
        action->setProperty(DBUSMENU_PROPERTY_HAS_SUBMENU, value.toString() == "submenu");
    else
        qWarning() << "Unhandled property update" << key;
}

void DBusModel::updateActionIcon(QAction *action, const QString &iconName)
{
    QString previous = action->property(DBUSMENU_PROPERTY_ICON).toString();
    if (previous == iconName)
        return;

    action->setProperty(DBUSMENU_PROPERTY_ICON, iconName);
    if (iconName.isEmpty())
        action->setIcon(QIcon());
    else
        action->setIcon(QIcon()); //TODO
}

bool DBusModel::connectToServer(const QString &service, const QString &objectPath)
{
    if (m_interface)
        disconnectFromServer();

    m_interface = new DBusMenuInterface(service, objectPath, QDBusConnection::sessionBus(), this);
    if (!m_interface->isValid())
        return false;

    /* Load root menu */
    loadMenu(0);

    /* Only the main menu keep track of these signals */
    QObject::connect(m_interface, SIGNAL(LayoutUpdated(uint, int)),
                     this, SLOT(onLayoutUpdated(uint, int)));
    QObject::connect(m_interface, SIGNAL(ItemActivationRequested(int, uint)),
                     this, SLOT(onItemActivationRequested(int, uint)));
    QObject::connect(m_interface, SIGNAL(ItemsPropertiesUpdated(DBusMenuItemList, DBusMenuItemKeysList)),
                     this, SLOT(onItemsPropertiesUpdated(DBusMenuItemList, DBusMenuItemKeysList)));
    emit connected();
    return true;
}

void DBusModel::disconnectFromServer()
{
    if (!m_interface)
        return;

    delete m_interface;
    m_interface = 0;
}

bool DBusModel::isConnected()
{
    return (m_interface && m_interface->isValid());
}

void DBusModel::onItemsPropertiesUpdated(const DBusMenuItemList &updatedList, const DBusMenuItemKeysList &removedList)
{
    Q_FOREACH(const DBusMenuItem &item, updatedList) {
        QAction *action = m_allActions.value(item.id);
        if (!action) {
            qWarning() << "No action for id" << item.id;
            continue;
        }

        QVariantMap::ConstIterator it = item.properties.constBegin();
        QVariantMap::ConstIterator end = item.properties.constEnd();
        for(; it != end; ++it) {
            updateActionProperty(action, it.key(), it.value());
        }
    }

    Q_FOREACH(const DBusMenuItemKeys &item, removedList) {
        QAction *action = m_allActions.value(item.id);
        if (!action) {
            qWarning() << "No action for id" << item.id;
            continue;
        }

        /* This should be removed? */
        Q_FOREACH(const QString &key, item.properties) {
            updateActionProperty(action, key, QVariant());
        }
    }
}

void DBusModel::onItemActivationRequested(int id, uint timestamp)
{
    qDebug() << "TODO: Item activated";
}

void DBusModel::onLayoutUpdated(uint, int)
{
    qDebug() << "TODO: Layout Update";
}

void DBusModel::append(QList<QAction*> items)
{
    beginInsertRows(QModelIndex(), m_actions.size(), m_actions.size() + items.size() - 1);
    foreach(QAction* act, items) {
        int actId = act->property(DBUSMENU_PROPERTY_ID).toInt();
        if (!m_actions.contains(act))
            m_actions << act;

        if (m_allActions.contains(actId) && m_allActions[actId] != act)
            delete m_allActions[actId];
        m_allActions[act->property(DBUSMENU_PROPERTY_ID).toInt()] = act;
    }
    endInsertRows();
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
    case Action:
        return QVariant::fromValue<QObject*>(act);
    case HasSubmenu:
        return act->property(DBUSMENU_PROPERTY_HAS_SUBMENU).isValid() &&
               act->property(DBUSMENU_PROPERTY_HAS_SUBMENU).toBool();
    case Submenu:
        return QVariant::fromValue<QObject*>(submenu(act));
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
