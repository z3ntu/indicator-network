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

Page {
    property var connection

    title: i18n.tr("TLS authentication:")

    Flickable {
        anchors.fill: parent
        contentHeight: contentItem.childrenRect.height
        boundsBehavior: (contentHeight > root.height) ?
                            Flickable.DragAndOvershootBounds :
                            Flickable.StopAtBounds
        flickableDirection: Flickable.VerticalFlick

        Column {
            anchors.left: parent.left
            anchors.right: parent.right

            ListItems.Standard {
                control: TextField {
                    text: connection.tlsRemote
                    onTextChanged: connection.tlsRemote = text
                    width: units.gu(19)
                    inputMethodHints: Qt.ImhNoPredictiveText
                }
                text: i18n.tr("Subject match:")
            }

            OptionalValue {
                text: i18n.tr("Verify peer certificate:")

                checked: connection.remoteCertTlsSet
                onCheckedChanged: connection.remoteCertTlsSet = checked
            }
            ListItems.ValueSelector {
                text: i18n.tr("Peer certificate TLS type:")
                values: [
                    i18n.tr("Server"),
                    i18n.tr("Client"),
                ]
                selectedIndex: connection.remoteCertTls
                onSelectedIndexChanged: connection.remoteCertTls = selectedIndex
                enabled: connection.remoteCertTlsSet
            }

            OptionalValue {
                text: i18n.tr("Use additional TLS authentication:")

                checked: connection.taSet
                onCheckedChanged: connection.taSet = checked
            }
            ListItems.Standard {
                text: i18n.tr("Key file:")
                control: TextField {
                    text: connection.ta
                    onTextChanged: connection.ta = text
                    inputMethodHints: Qt.ImhNoPredictiveText
                }
                enabled: connection.taSet
            }
            ListItems.ValueSelector {
                text: i18n.tr("Key direction:")
                values: [
                    i18n.tr("None"),
                    i18n.tr("0"),
                    i18n.tr("1"),
                ]
                selectedIndex: connection.taDir
                onSelectedIndexChanged: connection.taDir = selectedIndex
                enabled: connection.taSet
            }
        }
    }
}
