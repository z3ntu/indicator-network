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
import "../DialogFile"

Item {
    property var connection

    id: root

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

        ListItems.Standard {
            control: TextField {
                text: connection.gateway
                onTextChanged: connection.gateway = text
                width: units.gu(20)
                inputMethodHints: Qt.ImhNoPredictiveText
            }
            text: i18n.tr("Gateway:")
        }

        ListItems.Header {text: i18n.tr("Optional")}

        ListItems.Standard {
            control: TextField {
                text: connection.user
                onTextChanged: connection.user = text
                width: units.gu(20)
                inputMethodHints: Qt.ImhNoPredictiveText
            }
            text: i18n.tr("User name:")
        }

        ListItems.Standard {
            control: TextField {
                text: connection.password
                onTextChanged: connection.password = text
                width: units.gu(20)
                echoMode: TextInput.PasswordEchoOnEdit
            }
            text: i18n.tr("Password:")
        }

        ListItems.Standard {
            control: TextField {
                text: connection.domain
                onTextChanged: connection.domain = text
                width: units.gu(20)
                inputMethodHints: Qt.ImhNoPredictiveText
            }
            text: i18n.tr("NT Domain:")
        }

        ListItems.Divider {}

        ListItems.Standard {
            text: i18n.tr("Advanced")
            progression: true
            onClicked: pageStack.push(Qt.resolvedUrl("Advanced.qml"), {connection: root.connection})
        }
    }
}
