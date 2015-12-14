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
        Action {
            iconName: "add"
            text: i18n.tr("New configuration")
            // TODO Pick from OpenVPN or PPTP when supported
            onTriggered: model.add(VpnConnection.OPENVPN)
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
        model: root.model

        delegate: ListItem {
            ListItems.Standard {
                anchors.fill: parent
                text: id
                progression: true
                onClicked: openConnection(connection)

                control: Switch {
                    checked: active
                    onCheckedChanged: active = checked
                    enabled: activatable

                    // Not sure why I need to do this
                    property bool forceChecked: active
                    onForceCheckedChanged: checked = active
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
}
