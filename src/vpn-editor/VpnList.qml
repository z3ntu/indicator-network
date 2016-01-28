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
import Ubuntu.Components 1.3
import Ubuntu.Components.ListItems 1.3 as ListItems
import Ubuntu.Connectivity 1.0

Page {
    property var model

    id: root
    title: i18n.tr("VPN configurations")

    head.actions: [
        // There has to be a better way to force all the actions
        // into the menu
        Action {
            enabled: false
        },
        Action {
            enabled: false
        },
        Action {
            iconName: "add"
            onTriggered: model.add(VpnConnection.OPENVPN)
            text: i18n.tr("OpenVPN")
        },
        Action {
            iconName: "add"
            onTriggered: model.add(VpnConnection.PPTP)
            text: i18n.tr("PPTP")
        }
    ]

    function openConnection(connection) {
        pageStack.push(Qt.resolvedUrl("VpnEditor.qml"), {"connection": connection})
    }

    Connections {
        target: model
        onAddFinished: openConnection(connection)
    }

    ListView {
        id: listView
        anchors.fill: parent
        visible: (listView.count !== 0)
        model: root.model

        delegate: ListItem {
            ListItems.Standard {
                anchors.fill: parent
                text: id
                progression: true
                onClicked: openConnection(connection)

                control: Switch {
                    id: vpnSwitch
                    enabled: activatable
                    // If you create a binding normally, it gets lost, so use a Binding element
                    Binding {target: vpnSwitch; property: "checked"; value: active}
                    onTriggered: active = !active
                }
            }

            divider.visible: false

            leadingActions: ListItemActions {
               actions: [
                   Action {
                       iconName: "delete"
                       text: i18n.tr("Delete configuration")
                       onTriggered: connection.remove()
                   }
               ]
           }
        }
    }

    Rectangle {
        visible: (listView.count === 0)
        color: "lightgrey"
        anchors.fill: parent

        Label {
            text: i18n.tr("No VPN connections")
            fontSize: "x-large"
            anchors.centerIn: parent
        }
    }
}
