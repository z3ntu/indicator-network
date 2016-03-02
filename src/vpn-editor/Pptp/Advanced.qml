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

            ListItems.Standard {
                control: CheckBox {
                    id: neverDefaultCheckbox
                    Binding {target: neverDefaultCheckbox; property: "checked"; value: connection.neverDefault}
                    onCheckedChanged: connection.neverDefault = checked
                }
                text: i18n.tr("Only use connection for VPN resources")
            }

            ListItems.Header {text: i18n.tr("Authentication methods")}

            ListItems.Standard {
                control: CheckBox {
                    id: allowPapCheckbox
                    Binding {target: allowPapCheckbox; property: "checked"; value: connection.allowPap}
                    onCheckedChanged: connection.allowPap = checked
                }
                text: i18n.tr("PAP")
                enabled: !connection.requireMppe
            }

            ListItems.Standard {
                control: CheckBox {
                    id: allowChapCheckbox
                    Binding {target: allowChapCheckbox; property: "checked"; value: connection.allowChap}
                    onCheckedChanged: connection.allowChap = checked
                }
                text: i18n.tr("CHAP")
                enabled: !connection.requireMppe
            }

            ListItems.Standard {
                control: CheckBox {
                    id: allowMschapCheckbox
                    Binding {target: allowMschapCheckbox; property: "checked"; value: connection.allowMschap}
                    onCheckedChanged: connection.allowMschap = checked
                }
                text: i18n.tr("MSCHAP")
            }

            ListItems.Standard {
                control: CheckBox {
                    id: allowMschapv2Checkbox
                    Binding {target: allowMschapv2Checkbox; property: "checked"; value: connection.allowMschapv2}
                    onCheckedChanged: connection.allowMschapv2 = checked
                }
                text: i18n.tr("MSCHAPv2")
            }

            ListItems.Standard {
                control: CheckBox {
                    id: allowEapCheckbox
                    Binding {target: allowEapCheckbox; property: "checked"; value: connection.allowEap}
                    onCheckedChanged: connection.allowEap = checked
                }
                text: i18n.tr("EAP")
                enabled: !connection.requireMppe
            }

            ListItems.Header {text: i18n.tr("Security")}

            ListItems.Standard {
                control: CheckBox {
                    id: requireMppeCheckbox
                    Binding {target: requireMppeCheckbox; property: "checked"; value: connection.requireMppe}
                    onCheckedChanged: connection.requireMppe = checked
                }
                text: i18n.tr("Use Point-to-Point encryption")
            }

            ListItems.ItemSelector {
                model: [
                    i18n.tr("All Availale (Default)"),
                    i18n.tr("128-bit (most secure)"),
                    i18n.tr("40-bit (less secure)")
                ]
                selectedIndex: connection.mppeType
                onSelectedIndexChanged: connection.mppeType = selectedIndex
                enabled: connection.requireMppe
            }

            ListItems.Standard {
                control: CheckBox {
                    id: mppeStatefulCheckbox
                    Binding {target: mppeStatefulCheckbox; property: "checked"; value: connection.mppeStateful}
                    onCheckedChanged: connection.mppeStateful = checked
                }
                text: i18n.tr("Allow stateful encryption")
            }

            ListItems.Header {text: i18n.tr("Compression")}

            ListItems.Standard {
                control: CheckBox {
                    id: bsdCompressionCheckbox
                    Binding {target: bsdCompressionCheckbox; property: "checked"; value: connection.bsdCompression}
                    onCheckedChanged: connection.bsdCompression = checked
                }
                text: i18n.tr("Allow BSD data compression")
            }

            ListItems.Standard {
                control: CheckBox {
                    id: deflateCompressionCheckbox
                    Binding {target: deflateCompressionCheckbox; property: "checked"; value: connection.deflateCompression}
                    onCheckedChanged: connection.deflateCompression = checked
                }
                text: i18n.tr("Allow Deflate data compression")
            }

            ListItems.Standard {
                control: CheckBox {
                    id: tcpHeaderCompressionCheckbox
                    Binding {target: tcpHeaderCompressionCheckbox; property: "checked"; value: connection.tcpHeaderCompression}
                    onCheckedChanged: connection.tcpHeaderCompression = checked
                }
                text: i18n.tr("Use TCP header compression")
            }

            ListItems.Header {text: i18n.tr("Echo")}

            ListItems.Standard {
                control: CheckBox {
                    id: sendPppEchoPacketsCheckbox
                    Binding {target: sendPppEchoPacketsCheckbox; property: "checked"; value: connection.sendPppEchoPackets}
                    onCheckedChanged: connection.sendPppEchoPackets = checked
                }
                text: i18n.tr("Send PPP echo packets")
            }
        }
    }
}
