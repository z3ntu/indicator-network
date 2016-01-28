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

    title: i18n.tr("Advanced")

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

            OptionalValue {
                text: i18n.tr("Use custom gateway port:")

                checked: connection.portSet
                onCheckedChanged: connection.portSet = checked

                control: TextField {
                    text: connection.port
                    onTextChanged: connection.port = parseInt(text) || 0
                    enabled: connection.portSet
                    validator: IntValidator{bottom: 0}
                    width: units.gu(10)
                    inputMethodHints: Qt.ImhDigitsOnly
                }
            }

            OptionalValue {
                text: i18n.tr("Use renegotiation interval:")

                checked: connection.renegSecondsSet
                onCheckedChanged: connection.renegSecondsSet = checked

                control: TextField {
                    text: connection.renegSeconds
                    onTextChanged: connection.renegSeconds = parseInt(text) || 0
                    enabled: connection.renegSecondsSet
                    validator: IntValidator{bottom: 0}
                    width: units.gu(10)
                    inputMethodHints: Qt.ImhDigitsOnly
                }
            }

            ListItems.Standard {
                control: CheckBox {
                    id: compLzoCheckbox
                    Binding {target: compLzoCheckbox; property: "checked"; value: connection.compLzo}
                    onCheckedChanged: connection.compLzo = checked
                }
                text: i18n.tr("Use LZO data compression")
            }

            ListItems.Standard {
                control: CheckBox {
                    id: protoTcpCheckbox
                    Binding {target: protoTcpCheckbox; property: "checked"; value: connection.protoTcp}
                    onCheckedChanged: connection.protoTcp = checked
                }
                text: i18n.tr("Use a TCP connection")
            }

            OptionalValue {
                text: i18n.tr("Use custom virtual device type:")
                id: devTypeSetCheckbox
                Binding {target: devTypeSetCheckbox; property: "checked"; value: connection.devTypeSet}
                onCheckedChanged: connection.devTypeSet = checked
            }
            ListItems.ItemSelector {
                model: [
                    i18n.tr("TUN"),
                    i18n.tr("TAP"),
                ]
                selectedIndex: connection.devType
                onSelectedIndexChanged: connection.devType = selectedIndex
                enabled: connection.devTypeSet
            }
            ListItems.Standard {
                control: TextField {
                    text: connection.dev
                    onTextChanged: connection.dev = text
                    enabled: connection.devTypeSet
                    placeholderText: i18n.tr("(automatic)")
                    width: units.gu(20)
                    inputMethodHints: Qt.ImhNoPredictiveText
                }
                text: i18n.tr("and name:")
            }

            OptionalValue {
                text: i18n.tr("Use custom tunnel MTU:")

                id: tunnelMtuSetCheckbox
                Binding {target: tunnelMtuSetCheckbox; property: "checked"; value: connection.tunnelMtuSet}
                onCheckedChanged: connection.tunnelMtuSet = checked

                control: TextField {
                    text: connection.tunnelMtu
                    onTextChanged: connection.tunnelMtu = parseInt(text) || 0
                    enabled: connection.tunnelMtuSet
                    validator: IntValidator{bottom: 0}
                    width: units.gu(10)
                    inputMethodHints: Qt.ImhDigitsOnly
                }
            }

            OptionalValue {
                text: i18n.tr("Use UDP fragment size:")

                id: fragmentSizeSetCheckbox
                Binding {target: fragmentSizeSetCheckbox; property: "checked"; value: connection.fragmentSizeSet}
                onCheckedChanged: connection.fragmentSizeSet = checked

                control: TextField {
                    text: connection.fragmentSize
                    onTextChanged: connection.fragmentSize = parseInt(text) || 0
                    enabled: connection.fragmentSizeSet
                    validator: IntValidator{bottom: 0}
                    width: units.gu(10)
                    inputMethodHints: Qt.ImhDigitsOnly
                }
            }

            ListItems.Standard {
                control: CheckBox {
                    id: mssFixCheckbox
                    Binding {target: mssFixCheckbox; property: "checked"; value: connection.mssFix}
                    onCheckedChanged: connection.mssFix = checked
                }
                text: i18n.tr("Restrict tunnel TCP MSS")
            }

            ListItems.Standard {
                control: CheckBox {
                    id: remoteRandomCheckbox
                    Binding {target: remoteRandomCheckbox; property: "checked"; value: connection.remoteRandom}
                    onCheckedChanged: connection.remoteRandom = checked
                }
                text: i18n.tr("Randomize remote hosts")
            }
        }
    }
}
