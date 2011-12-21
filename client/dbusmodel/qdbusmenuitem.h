#ifndef QDBUSMENU_H
#define QDBUSMENU_H

#include <QString>
#include <QObject>
#include <QHash>

#include <glib.h>
#include <libdbusmenu-glib/client.h>

typedef QHash<int, QObject* > ItemList;

class QDBusMenuItem: public QObject
{
    Q_OBJECT
public:
    Q_ENUMS(ItemType)
    enum ItemType {
        Unknow,
        Label,
        TextEntry,
        ToggleButton,
        RadioButton,
        Separator
    };

    ~QDBusMenuItem();
    int position() const;
    ItemType type() const;
    QByteArray typeName() const;
    QVariant data() const;

Q_SIGNALS:
    void typeDiscovered();
    void changed();
    void moved(int newPos, int oldPos);

private:
    Q_DISABLE_COPY(QDBusMenuItem)
    DbusmenuMenuitem *m_gitem;
    static ItemList m_globalItemList;
    ItemType m_type;

    QDBusMenuItem() {}
    QDBusMenuItem(DbusmenuMenuitem * gitem, QObject * parent);

    void updateType();
    void loadChildren();
    void loadProperties();
    void updateProperty(const QByteArray & name, QVariant value);
    DbusmenuMenuitem * item() const;
    QDBusMenuItem * getChild(DbusmenuMenuitem * gitem) const;

    //gobject slots
    static void onItemPropertyChanged(DbusmenuMenuitem * mi, gchar * property, GVariant * value, gpointer * data);
    static void onChildAdded(DbusmenuMenuitem * mi, DbusmenuMenuitem * child, guint position, gpointer * data);
    static void onChildRemoved(DbusmenuMenuitem * mi, DbusmenuMenuitem * child, gpointer * data);
    static void onChildMoved(DbusmenuMenuitem * mi, DbusmenuMenuitem * child, guint newpos, guint oldpos, gpointer * data);

    friend class DBusControl;
};

#endif
