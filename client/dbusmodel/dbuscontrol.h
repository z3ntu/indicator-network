#ifndef DBUSCONTROL_H
#define DBUSCONTROL_H

#include <QAbstractListModel>
#include <libdbusmenu-glib/client.h>

#include "dbusmodel.h"

class QDbusMenuItem;

/*!
    \class DBusControl
    \brief The DBusControl class provides the dbusmenu control element in QML side

    This class must be used with DBusModel to have access to dbusmenu service, see below a example of usage.

    \verbatim
    import QtQuick 1.1
    import DBusMenu 1.0

    DBusMenuClientControl {
        id: menuControl

        service: "org.dbusmenu.test"
        objectPath: "/org/test"
    }

    DBusMenuClientModel {
        id: menuModel

        control: menuControl
        menuId: 0
    }

    ListView {
        model: menuModel
        delegate: Text {
            text: model.label
        }
    }
    \endverbatim
*/
class DBusControl : public QObject
{
    Q_OBJECT

    /*!
      \property DBusControl::service

      This property is used to read/write the service property which contains the dbus service name
    */
    Q_PROPERTY(QString service READ service WRITE setService NOTIFY serviceChanged)

    /*!
        \property DBusControl::objectPath

        This property are used to read/write the dbus objectPath value property
    */
    Q_PROPERTY(QString objectPath READ objectPath WRITE setObjectPath NOTIFY objectPathChanged)

    /*!
        \property DBusControl::connected
        This property is used tor read the connected property, which is used to check if the DBusControl connection is active
    */
    Q_PROPERTY(bool connected READ isConnected NOTIFY rootChanged)
public:
    Q_ENUMS(EventType)

    //! EventType enum used to specify the event type in the function DBusControl::sendEvent
    enum EventType {
        Clicked,    /*!< Mouse click Event */
        Hovered,    /*!< Mouse Hover Event */
        Opened,     /*!< Menu Open Event */
        Closed,     /*!< Menu Close Event */
        TextChanged /*!< Event used for text extry object to notify text changed */
    };

    //! DBusControl constructor

    DBusControl(QObject *parent = 0);

    //! DBusControl destructor
    ~DBusControl();

    //! Connect to dbus menu server
    /*!
        Returns true if the connection was successful otherwise return false
    */
    Q_INVOKABLE bool connectToServer();


    //! Disconnect from dbus menu server and clear all resources used
    Q_INVOKABLE void disconnectFromServer();


    //! Load information about the menu item.
    /*!
        \param id The menu id
    */
    Q_INVOKABLE QDBusMenuItem *load(int id);

    //! This method is used to send custom event to server.
    /*!
        \param id Is the menu unique id which will receive the event
        \param eventType is the type of the event
        \param data Is a extra data to be send in the event
    */
    Q_INVOKABLE void sendEvent(int id, EventType eventType, QVariant data = QVariant());

    /* Properties */
    //! Retrieve the service name
    QString service() const;

    //! Retrieve the object path name
    QString objectPath() const;

    //! Retrieve if the control is connected to server
    bool isConnected() const;

    //! Set the service name to connect to
    void setService(const QString &service);

    //! Set the dbus object path to connect to
    void setObjectPath(const QString &objectPath);

Q_SIGNALS:
    //! Called when the root object has changed
    void rootChanged();

    //! Called when service name has changed
    void serviceChanged();

    //! Called when the object path has changed
    void objectPathChanged();

private:
    Q_DISABLE_COPY(DBusControl)

    DbusmenuClient *m_client;
    QDBusMenuItem *m_root;
    QString m_service;
    QString m_objectPath;

    //! Function called when the rootChanged signal is emmited from dbus
    static void onRootChanged(DbusmenuClient *client, DbusmenuMenuitem *newroot, gpointer data);
};

#endif
