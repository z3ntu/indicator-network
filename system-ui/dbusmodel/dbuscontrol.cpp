#include "dbuscontrol.h"
#include "dbusmenu.h"
#include "dbusmenuconst.h"

DBusControl::DBusControl(QObject *parent)
    :QObject(parent),
      m_interface(0)
{
    DBusMenuTypes_register();
}

DBusControl::~DBusControl()
{
    disconnectFromServer();
}

bool DBusControl::connectToServer()
{
    if (m_interface)
        return m_interface->isValid();

    m_interface = new DBusMenuInterface(m_service, m_objectPath, QDBusConnection::sessionBus(), this);
    if (!m_interface || !m_interface->isValid()) {
        qWarning() << "Fail to connect to:" << m_service << m_objectPath;
        disconnectFromServer();
        return false;
    }

    /* Only the main menu keep track of these signals */
    QObject::connect(m_interface, SIGNAL(LayoutUpdated(uint, int)),
                     this, SLOT(onLayoutUpdated(uint, int)));
    QObject::connect(m_interface, SIGNAL(ItemActivationRequested(int, uint)),
                     this, SLOT(onItemActivationRequested(int, uint)));
    QObject::connect(m_interface, SIGNAL(ItemsPropertiesUpdated(DBusMenuItemList, DBusMenuItemKeysList)),
                     this, SLOT(onItemsPropertiesUpdated(DBusMenuItemList, DBusMenuItemKeysList)));
    emit connectionChanged();
    return true;
}

void DBusControl::disconnectFromServer()
{
    if (!m_interface)
        return;

    delete m_interface;
    m_interface = 0;
}

QString DBusControl::service() const
{
    return m_service;
}

QString DBusControl::objectPath() const
{
    return m_objectPath;
}

bool DBusControl::isConnected()
{
    return m_interface != 0;
}

void DBusControl::setService(const QString &service)
{
    if (m_service != service) {
        m_service = service;
        emit serviceChanged();
    }
}

void DBusControl::setObjectPath(const QString &objectPath)
{
    if (m_objectPath != objectPath) {
        m_objectPath = objectPath;
        emit objectPathChanged();
    }
}

void DBusControl::sendEvent(int id, EventType eventType)
{
    static QHash<int, QString> eventNames;
    static QDBusVariant empty;

    if (!m_actions.contains(id)) {
        qWarning() << "Invalid target menu id:" << id  << "on menu control event function";
        return;
    }

    if (!empty.variant().isValid())
        empty.setVariant(QVariant(QString()));

    if (eventNames.empty()) {
        eventNames[Clicked] = "clicked";
        eventNames[Hovered] = "hovered";
        eventNames[Openend] = "openend";
        eventNames[Closed] = "closed";
    }

    uint timestamp = QDateTime::currentDateTime().toTime_t();
    m_interface->Event(id, eventNames[eventType], empty, timestamp);
}

void DBusControl::load(int id)
{
    Q_ASSERT(m_interface);

    QDBusPendingCall call = m_interface->GetLayout(id, 1, QStringList());
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(call, this);
    watcher->setProperty(DBUSMENU_PROPERTY_ID, id);

    /* Wait for reply */
    QObject::connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)), this, SLOT(onGetLayoutReply(QDBusPendingCallWatcher*)));
}

void DBusControl::onGetLayoutReply(QDBusPendingCallWatcher *ptrReply)
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
        if (act) {
            actions << act;
            m_actions.insert(dbusMenuItem.id, act);
        }
    }

    if (actions.size() > 0)
        emit entryLoaded(ptrReply->property(DBUSMENU_PROPERTY_ID).toInt(), actions);
}

QAction* DBusControl::parseAction(int id, const QVariantMap &_map, QWidget *parent)
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

void DBusControl::updateAction(QAction *action, const QVariantMap &map, const QStringList &requestedProperties)
{
    Q_FOREACH(const QString &key, requestedProperties) {
        updateActionProperty(action, key, map.value(key));
    }
}

void DBusControl::updateActionProperty(QAction *action, const QString &key, const QVariant &value)
{
    if (key == "label")
        action->setText(value.toString());
    else if (key == "enabled")
        action->setEnabled(value.isValid() ? value.toBool(): true);
    else if (key == "icon-name")
        updateActionIcon(action, value.toString());
    else if (key == "visible")
        action->setVisible(value.isValid() ? value.toBool() : true);
    else if (key == "toggle-type")
        action->setCheckable(value.toString() != "");
    else if (key == "children-display")
        action->setProperty(DBUSMENU_PROPERTY_HAS_SUBMENU, value.toString() == "submenu");
    else
        qWarning() << "Unhandled property update" << key;
}

void DBusControl::updateActionIcon(QAction *action, const QString &iconName)
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

void DBusControl::onItemsPropertiesUpdated(const DBusMenuItemList &updatedList, const DBusMenuItemKeysList &removedList)
{
    Q_FOREACH(const DBusMenuItem &item, updatedList) {
        QAction *action = m_actions.value(item.id);
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
        QAction *action = m_actions.value(item.id);
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

void DBusControl::onItemActivationRequested(int id, uint timestamp)
{
    qDebug() << "TODO: Item activated";
}

void DBusControl::onLayoutUpdated(uint, int)
{
    qDebug() << "TODO: Layout Update";
}
