#ifndef QDBUSMENU_H
#define QDBUSMENU_H

#include <QString>
#include <QObject>
#include <QHash>
#include <QVariantMap>

#include <glib.h>
#include <libdbusmenu-glib/client.h>

typedef QHash<int, QObject* > ItemList;

/*!
    \class QDBusMenuItem
    \brief A proxy class for DbusmenuMenuitem

    This is a helper class to provide access to DbusmenuMenuitem using QObject properties and signals/slot.
*/
class QDBusMenuItem: public QObject
{
    Q_OBJECT
public:
    //! Destructor
    ~QDBusMenuItem();

    //! The position of menu item in the menu list
    int position() const;

    //! The menu item type
    QByteArray type() const;

    //! Retun any property which starts with "x-" in the name
    QVariantMap extraProperties() const;

Q_SIGNALS:
    //! This signal is called when all properties related with menu item type is avaliable, and a type was setted in the menu item
    void typeDiscovered();

    //! This signal is called when any property is changed
    void changed();

    //! This signal is called when the menu item position was changed
    void moved(int newPos, int oldPos);

private:
    Q_DISABLE_COPY(QDBusMenuItem)
    DbusmenuMenuitem *m_gitem;
    static ItemList m_globalItemList;
    QByteArray m_type;
    QVariantMap m_extraProperties;

    //! Constructor
    QDBusMenuItem() {}

    //! Contructor
    QDBusMenuItem(DbusmenuMenuitem * gitem, QObject * parent);

    //! Update the current item type
    void updateType();

    //! Load all menu children
    void loadChildren();

    //! Load all necessary information/property from the menu
    void loadProperties();

    //! Update the property
    /*!
      \param name The name of the property to be updated
      \param value The new value of the property
    */
    bool updateProperty(const QByteArray & name, QVariant value);

    //! The original DbusmenuMenuitem attached to this object
    DbusmenuMenuitem * item() const;

    //! Return the QDBusMenuItem  related with the child DbusmenuMenuitem
    QDBusMenuItem * getChild(DbusmenuMenuitem * gitem) const;

    //gobject slots
    //! slot used to intercept property change from the DbusmenuMenuitem
    static void onItemPropertyChanged(DbusmenuMenuitem * mi, gchar * property, GVariant * value, gpointer * data);

    //! slot used to intercept childAdded from DbusmenuMenuitem
    static void onChildAdded(DbusmenuMenuitem * mi, DbusmenuMenuitem * child, guint position, gpointer * data);

    //! slot used to intercept childRemoved from DbusmenuMenuitem
    static void onChildRemoved(DbusmenuMenuitem * mi, DbusmenuMenuitem * child, gpointer * data);

    //! slot used to intercept childModed from DbusmenuMenuitem
    static void onChildMoved(DbusmenuMenuitem * mi, DbusmenuMenuitem * child, guint newpos, guint oldpos, gpointer * data);

    friend class DBusControl;
};

#endif
