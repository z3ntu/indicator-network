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
import Ubuntu.Connectivity 1.0

Page {
    id: vpnEditor
    property var connection

    title: i18n.tr("Editing: %1").arg(connection.id)

    Component.onCompleted: {
        connection.updateSecrets()

        var props = {"connection": connection}
        switch (connection.type) {
        case VpnConnection.OPENVPN:
            editor.setSource("Openvpn/Editor.qml", props)
            break
        case VpnConnection.PPTP:
            editor.setSource("Pptp/Editor.qml", props)
            break
        }
    }

    Item {
        anchors.fill: parent
        Flickable {
            anchors.fill: parent
            contentWidth: parent.width
            contentHeight: units.gu(100)
            clip: true

            Loader {
                id: editor
                width: vpnEditor.width
            }
        }
    }
}
