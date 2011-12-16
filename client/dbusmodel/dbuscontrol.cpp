#include "dbuscontrol.h"
#include "qdbusmenuitem.h"
#include "dbusmenuconst.h"

#include <QByteArray>
#include <QDateTime>
#include <QDebug>

#include <gio/gio.h>
#include <libdbusmenu-glib/client.h>

DBusControl::DBusControl(QObject *parent)
    : QObject(parent),
      m_client(0),
      m_root(0)
{
}

DBusControl::~DBusControl()
{
    disconnectFromServer();
}

void DBusControl::onRootChanged(DbusmenuClient * client, DbusmenuMenuitem * newroot, gpointer data)
{
    DBusControl *self = reinterpret_cast<DBusControl*>(data);

    if (self->m_root)
        delete self->m_root;

    if (newroot)
        self->m_root = new QDBusMenuItem(newroot, self);
    else
        self->m_root = 0;

    Q_EMIT self->rootChanged();
}

bool DBusControl::connectToServer()
{
    if (m_client)
        return true;

    m_client = dbusmenu_client_new(qPrintable(m_service), qPrintable(m_objectPath));
    g_signal_connect(G_OBJECT(m_client), DBUSMENU_CLIENT_SIGNAL_ROOT_CHANGED, G_CALLBACK(onRootChanged), this);

    Q_ASSERT(m_client);
    return true;
}

void DBusControl::disconnectFromServer()
{
    if (!m_client)
        return;

    delete m_root;
    m_root = 0;
    Q_ASSERT(QDBusMenuItem::m_globalItemList.size() == 0);
    g_object_unref(m_client);
    m_client = 0;
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
    return m_root != 0;
}

void DBusControl::setService(const QString &service)
{
    if (m_service != service) {
        m_service = service;
        Q_EMIT serviceChanged();
    }
}

void DBusControl::setObjectPath(const QString &objectPath)
{
    if (m_objectPath != objectPath) {
        m_objectPath = objectPath;
        Q_EMIT objectPathChanged();
    }
}

void DBusControl::sendEvent(int id, EventType eventType, QVariant data)
{
    Q_ASSERT(m_client);
    static QHash<int, QByteArray> eventNames;
    static GVariant *empty = 0;

    if (!empty)
        empty = g_variant_new_byte(0);

    if (eventNames.empty()) {
        eventNames[Clicked] = "clicked";
        eventNames[Hovered] = "hovered";
        eventNames[Openend] = "openend";
        eventNames[Closed] = "closed";
        eventNames[TextChanged] = "x-text-changed";
    }

    QDBusMenuItem *item = qobject_cast<QDBusMenuItem* >(QDBusMenuItem::m_globalItemList[id]);
    if (!item) {
        qWarning() << "Invalid item id on sendEvent function";
        return;
    }
    uint timestamp = QDateTime::currentDateTime().toTime_t();

    if (eventType == TextChanged) {
        GVariant *text = g_variant_new_string(data.toByteArray());
        dbusmenu_menuitem_handle_event(item->item(), eventNames[eventType], text, timestamp);
        qDebug() << "SEND EVENT" << id << eventNames[eventType] << data;
    } else {
        g_variant_ref(empty);
        dbusmenu_menuitem_handle_event(item->item(), eventNames[eventType], empty, timestamp);
    }
}

QDBusMenuItem *DBusControl::load(int id)
{
    Q_ASSERT(m_client);

    if (!m_root)
        return 0;

    return qobject_cast<QDBusMenuItem* >(QDBusMenuItem::m_globalItemList[id]);
}
