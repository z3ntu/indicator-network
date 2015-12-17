/*
 * Copyright (C) 2015 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License version 3,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.4
import QtQuick.Layouts 1.1
import Ubuntu.Components 1.3
import Ubuntu.Components.ListItems 1.3 as ListItems
import Ubuntu.Connectivity 1.0

Item {
    property var connection

    id: root

    Component.onCompleted: {
        connectionTypeChanged(connection.connectionType)
    }

    Connections {
        target: connection
        onConnectionTypeChanged: connectionTypeChanged(connectionType)
    }

    // This function sets the source and properties of the dynamic
    // elements at the same time using the setSource method.
    // This ensures we don't get any "undefined property" type errors.
    function connectionTypeChanged(connectionType) {
        var props = {"connection": connection}

        switch (connectionType) {
        case OpenvpnConnection.TLS:
            basicPropertiesLoader.setSource("Tls.qml", props)
            break
        case OpenvpnConnection.PASSWORD:
            basicPropertiesLoader.setSource("Password.qml", props)
            break
        case OpenvpnConnection.PASSWORD_TLS:
            basicPropertiesLoader.setSource("PasswordTls.qml", props)
            break
        case OpenvpnConnection.STATIC_KEY:
            basicPropertiesLoader.setSource("StaticKey.qml", props)
            break
        }
    }

    Column {
        id: topPart
        anchors.left: parent.left
        anchors.right: parent.right


        ListItems.Header {text: i18n.tr("General")}

        ListItems.Standard {
            control: TextField {
                text: connection.id
                onTextChanged: connection.id = text
                width: units.gu(20)
                inputMethodHints: Qt.ImhNoPredictiveText
            }
            text: i18n.tr("ID:")
        }

        ListItems.Header {text: i18n.tr("Authentication")}

        ListItems.Standard {
            control: TextField {
                text: connection.remote
                onTextChanged: connection.remote = text
                width: units.gu(20)
                inputMethodHints: Qt.ImhNoPredictiveText
            }
            text: i18n.tr("Remote:")
        }

        ListItems.ValueSelector {
            values: [
                i18n.tr("Certificates (TLS)"),
                i18n.tr("Password"),
                i18n.tr("Password with certificates (TLS)"),
                i18n.tr("Static key")
            ]
            selectedIndex: connection.connectionType
            onSelectedIndexChanged: connection.connectionType = selectedIndex
            text: i18n.tr("Type:")
        }

        // Basic properties handled here
        // Load in a different component depending on the type
        Loader {
            anchors.left: parent.left
            anchors.right: parent.right

            id: basicPropertiesLoader
        }

        ListItems.Divider {}

        ListItems.Standard {
            text: i18n.tr("Advanced")
            progression: true
            onClicked: pageStack.push(Qt.resolvedUrl("AdvancedGeneral.qml"), {connection: root.connection})
        }

        ListItems.Standard {
            text: i18n.tr("Security")
            progression: true
            onClicked: pageStack.push(Qt.resolvedUrl("AdvancedSecurity.qml"), {connection: root.connection})
        }

        ListItems.Standard {
            text: i18n.tr("TLS")
            progression: true
            visible: (connection.connectionType !== OpenvpnConnection.STATIC_KEY)
            onClicked: pageStack.push(Qt.resolvedUrl("AdvancedTls.qml"), {connection: root.connection})
        }

        ListItems.Standard {
            text: i18n.tr("Proxies")
            progression: true
            onClicked: pageStack.push(Qt.resolvedUrl("AdvancedProxies.qml"), {connection: root.connection})
        }
    }
}
