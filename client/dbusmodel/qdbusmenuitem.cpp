#include "qdbusmenuitem.h"
#include "dbusmenuconst.h"

#include <libdbusmenu-glib/client.h>

#include <QVariant>
#include <QDebug>

#define DBUSMENU_MENUITEM_WIDGET_TYPE "x-tablet-widget"

ItemList QDBusMenuItem::m_globalItemList;
static QList<QByteArray> mapedProperties;
static QList<QByteArray> typeRelatedProperties;
static QHash<QByteArray, QDBusMenuItem::ItemType> widgetTypeMap;
static QHash<QDBusMenuItem::ItemType, QByteArray> widgetDataMap;


static QVariant GVariantToQVariant(GVariant *value)
{
    if (g_variant_is_of_type(value, G_VARIANT_TYPE_BOOLEAN))
        return QVariant(g_variant_get_boolean(value) == TRUE);
    else if (g_variant_is_of_type(value, G_VARIANT_TYPE_BYTE))
        return QVariant(g_variant_get_byte(value));
    else if (g_variant_is_of_type(value, G_VARIANT_TYPE_INT16))
        return QVariant(g_variant_get_int16(value));
    else if (g_variant_is_of_type(value, G_VARIANT_TYPE_UINT16))
        return QVariant(g_variant_get_uint16(value));
    else if (g_variant_is_of_type(value, G_VARIANT_TYPE_INT32))
        return QVariant(g_variant_get_int32(value));
    else if (g_variant_is_of_type(value, G_VARIANT_TYPE_UINT32))
        return QVariant(g_variant_get_uint32(value));
    else if (g_variant_is_of_type(value, G_VARIANT_TYPE_INT64))
        return QVariant((qlonglong) g_variant_get_int64(value));
    else if (g_variant_is_of_type(value, G_VARIANT_TYPE_UINT64))
        return QVariant((qulonglong) g_variant_get_uint64(value));
    else if (g_variant_is_of_type(value, G_VARIANT_TYPE_DOUBLE))
        return QVariant(g_variant_get_double(value));
    else if (g_variant_is_of_type(value, G_VARIANT_TYPE_STRING)) {
        gsize size = 0;
        const gchar *v = g_variant_get_string(value, &size);
        return QVariant(QByteArray(v, size));
    }

    qWarning() << "Unhandle variant type" << g_variant_get_type_string(value);
    return QVariant();
}

void QDBusMenuItem::onItemPropertyChanged(DbusmenuMenuitem *mi, gchar *property, GVariant *value, gpointer *data)
{
    QDBusMenuItem *self = reinterpret_cast<QDBusMenuItem*>(data);
    self->updateProperty(property, GVariantToQVariant(value));

    if (typeRelatedProperties.contains(property))
        self->updateType();
    Q_EMIT self->changed();
}

void QDBusMenuItem::onChildAdded(DbusmenuMenuitem *mi, DbusmenuMenuitem *child, guint position, gpointer *data)
{
    QDBusMenuItem *self = reinterpret_cast<QDBusMenuItem*>(data);
    new QDBusMenuItem(child, self);
}

void QDBusMenuItem::onChildRemoved(DbusmenuMenuitem *mi, DbusmenuMenuitem *child, gpointer *data)
{
    QDBusMenuItem *self = reinterpret_cast<QDBusMenuItem*>(data);
    delete self->getChild(child);
}

void QDBusMenuItem::onChildMoved(DbusmenuMenuitem *mi, DbusmenuMenuitem *child, guint newpos, guint oldpos, gpointer *data)
{
    int id = dbusmenu_menuitem_get_id(child);
    Q_ASSERT(QDBusMenuItem::m_globalItemList.contains(id));
    QDBusMenuItem *item = qobject_cast<QDBusMenuItem*>(QDBusMenuItem::m_globalItemList[id]);
    Q_ASSERT(item);
    Q_EMIT item->moved(newpos, oldpos);
}

QDBusMenuItem::QDBusMenuItem(DbusmenuMenuitem *gitem, QObject *parent)
    : QObject(parent),
      m_gitem(gitem)
{
    /*
      Only those properties are the current supported
    */
    if (mapedProperties.size() == 0) {
        mapedProperties << DBUSMENU_MENUITEM_PROP_LABEL
                        << DBUSMENU_MENUITEM_PROP_ICON_NAME
                        << DBUSMENU_MENUITEM_PROP_TOGGLE_TYPE
                        << DBUSMENU_MENUITEM_PROP_TOGGLE_STATE
                        << DBUSMENU_MENUITEM_PROP_CHILD_DISPLAY;
     }

    /*
      Those properties are related with Object type because of that,
      is necessary update the object type if one of those properties change;
    */
    if (typeRelatedProperties.size() == 0) {
        typeRelatedProperties << DBUSMENU_MENUITEM_WIDGET_TYPE
                              << DBUSMENU_MENUITEM_PROP_TOGGLE_TYPE
                              << DBUSMENU_MENUITEM_TOGGLE_CHECK
                              << DBUSMENU_MENUITEM_TOGGLE_RADIO;
    }

    if (widgetTypeMap.size() == 0) {
        widgetTypeMap["x-textentry"] = TextEntry;
        widgetTypeMap["x-toggle"] = ToggleButton;
    }

    if (widgetDataMap.size() == 0) {
        widgetDataMap[TextEntry] = "x-text";
    }

    loadChildren();
    loadProperties();

    g_signal_connect(G_OBJECT(m_gitem), DBUSMENU_MENUITEM_SIGNAL_PROPERTY_CHANGED, G_CALLBACK(onItemPropertyChanged), this);
    g_signal_connect(G_OBJECT(m_gitem), DBUSMENU_MENUITEM_SIGNAL_CHILD_ADDED, G_CALLBACK(onChildAdded), this);
    g_signal_connect(G_OBJECT(m_gitem), DBUSMENU_MENUITEM_SIGNAL_CHILD_REMOVED, G_CALLBACK(onChildRemoved), this);
    g_signal_connect(G_OBJECT(m_gitem), DBUSMENU_MENUITEM_SIGNAL_CHILD_MOVED, G_CALLBACK(onChildMoved), this);

    // Regiter itself on global list
    m_globalItemList[dbusmenu_menuitem_get_id(m_gitem)] = this;
}

QDBusMenuItem::~QDBusMenuItem()
{
    if (m_gitem) {
        m_globalItemList.remove(dbusmenu_menuitem_get_id(m_gitem));
        m_gitem = 0;
    }
}

DbusmenuMenuitem *QDBusMenuItem::item() const
{
    return m_gitem;
}

int QDBusMenuItem::position() const
{
    return dbusmenu_menuitem_get_position(m_gitem, dbusmenu_menuitem_get_parent(m_gitem));
}

QDBusMenuItem::ItemType QDBusMenuItem::type() const
{
    return m_type;
}

QByteArray QDBusMenuItem::typeName() const
{
    switch(m_type) {
    case TextEntry:
        return "TextEntry";
    case ToggleButton:
        return "ToggleButton";
    case RadioButton:
        return "RadioButton";
    case QDBusMenuItem::Label:
    default:
        return "Label";
    }
}

QVariant QDBusMenuItem::data() const
{
    if (widgetDataMap.contains(m_type)) {
        if (dbusmenu_menuitem_property_exist(m_gitem, widgetDataMap[m_type])) {
            GVariant *var = dbusmenu_menuitem_property_get_variant(m_gitem, widgetDataMap[m_type]);
            return GVariantToQVariant(var);
        }
    }
    return QVariant();
}
void QDBusMenuItem::updateType()
{
    m_type = Label;

    if (dbusmenu_menuitem_property_exist(m_gitem, DBUSMENU_MENUITEM_WIDGET_TYPE)) {
        GVariant *var = dbusmenu_menuitem_property_get_variant(m_gitem, DBUSMENU_MENUITEM_WIDGET_TYPE);
        QByteArray typeName = GVariantToQVariant(var).toByteArray();
        if (widgetTypeMap.contains(typeName))
            m_type = widgetTypeMap[typeName];
    }

    // Check for radio buttom
    if ((m_type == ToggleButton) && dbusmenu_menuitem_property_exist(m_gitem, DBUSMENU_MENUITEM_PROP_TOGGLE_TYPE)) {
        GVariant *var = dbusmenu_menuitem_property_get_variant(m_gitem, DBUSMENU_MENUITEM_PROP_TOGGLE_TYPE);
        gsize size;
        if (qstrcmp(g_variant_get_string(var, &size), DBUSMENU_MENUITEM_TOGGLE_RADIO) == 0)
            m_type = RadioButton;
    }
}

void QDBusMenuItem::loadChildren()
{
    GList *children = dbusmenu_menuitem_get_children(m_gitem);
    while(children != 0) {
        DbusmenuMenuitem *i = reinterpret_cast<DbusmenuMenuitem *>(children->data);
        if (i)
            new QDBusMenuItem(i, this);
        children = g_list_next(children);
    }
}

void QDBusMenuItem::loadProperties()
{
    /* parse the relevant properties */
    setProperty(DBUSMENU_PROPERTY_ID, dbusmenu_menuitem_get_id(m_gitem));
    Q_FOREACH(QByteArray propName, mapedProperties) {
        if (dbusmenu_menuitem_property_exist(m_gitem, propName)) {
            GVariant *var = dbusmenu_menuitem_property_get_variant(m_gitem, propName);
            updateProperty(propName, GVariantToQVariant(var));
        }
    }
}

void QDBusMenuItem::updateProperty(const QByteArray &name, QVariant value)
{
    if (name == DBUSMENU_MENUITEM_PROP_LABEL) {
        setProperty(DBUSMENU_PROPERTY_LABEL, value);
    } else if (name == DBUSMENU_MENUITEM_PROP_ICON_NAME) {
        setProperty(DBUSMENU_PROPERTY_ICON_NAME, value);
    } else if (name == DBUSMENU_MENUITEM_PROP_CHILD_DISPLAY) {
        setProperty(DBUSMENU_PROPERTY_HAS_SUBMENU, value.toString() == "submenu");
    } else if (name == DBUSMENU_MENUITEM_PROP_TOGGLE_STATE) {
        int newValue = -1;
        if (value.toInt() == DBUSMENU_MENUITEM_TOGGLE_STATE_CHECKED)
            newValue = 1;
        else if(value.toInt() == DBUSMENU_MENUITEM_TOGGLE_STATE_UNCHECKED)
            newValue = 0;
        setProperty(DBUSMENU_PROPERTY_STATE, newValue);
    }
}

QDBusMenuItem *QDBusMenuItem::getChild(DbusmenuMenuitem *gitem) const
{
    Q_FOREACH(QObject *obj, children()) {
        QDBusMenuItem *i = qobject_cast<QDBusMenuItem*>(obj);
        if (i && i->item() == gitem)
            return i;
    }
    return 0;
}
