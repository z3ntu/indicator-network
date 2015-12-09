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
import QtQuick.Controls 1.3 as QQC
import QtQuick.Layouts 1.1
import Ubuntu.Components 1.3

Page {
    property var connection

    title: i18n.tr("Advanced")

    ColumnLayout {
        spacing: units.gu(1)
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.topMargin: units.gu(1)
        anchors.rightMargin: units.gu(1)
        anchors.leftMargin: units.gu(1)

        RowLayout {
            Layout.fillWidth: true

            CheckBox {
                checked: connection.portSet
                onCheckedChanged: connection.portSet = checked
            }

            Label {text: i18n.tr("Use custom gateway port:")}

            TextField {
                text: connection.port
                onTextChanged: connection.port = parseInt(text) || 0
                enabled: connection.portSet
                validator: IntValidator{bottom: 0}
                Layout.fillWidth: true
            }
        }

        RowLayout {
            Layout.fillWidth: true

            CheckBox {
                checked: connection.renegSecondsSet
                onCheckedChanged: connection.renegSecondsSet = checked
            }

            Label {text: i18n.tr("Use renegotiation interval:")}

            TextField {
                text: connection.renegSeconds
                onTextChanged: connection.renegSeconds = parseInt(text) || 0
                enabled: connection.renegSecondsSet
                validator: IntValidator{bottom: 0}
                Layout.fillWidth: true
            }
        }

        RowLayout {
            Layout.fillWidth: true

            CheckBox {
                checked: connection.compLzo
                onCheckedChanged: connection.compLzo = checked
            }

            Label {text: i18n.tr("Use LZO data compression")}
        }

        RowLayout {
            Layout.fillWidth: true

            CheckBox {
                checked: connection.protoTcp
                onCheckedChanged: connection.protoTcp = checked
            }

            Label {text: i18n.tr("Use a TCP connection")}
        }

        RowLayout {
            Layout.fillWidth: true

            CheckBox {
                checked: connection.devTypeSet
                onCheckedChanged: connection.devTypeSet = checked
            }

            Label {text: i18n.tr("Virtual device type:")}

            QQC.ComboBox {
                model: [
                    i18n.tr("TUN"),
                    i18n.tr("TAP"),
                ]
                currentIndex: connection.devType
                onCurrentIndexChanged: connection.devType = currentIndex
                enabled: connection.devTypeSet
                Layout.fillWidth: true
            }
        }

        RowLayout {
            Layout.fillWidth: true

            Label {text: i18n.tr("and name:")}

            TextField {
                text: connection.dev
                placeholderText: i18n.tr("(automatic)")
                onTextChanged: connection.dev = text
                enabled: connection.devTypeSet
                Layout.fillWidth: true
            }
        }

        RowLayout {
            Layout.fillWidth: true

            CheckBox {
                checked: connection.tunnelMtuSet
                onCheckedChanged: connection.tunnelMtuSet = checked
            }

            Label {text: i18n.tr("Use custom tunnel MTU:")}

            TextField {
                text: connection.tunnelMtu
                onTextChanged: connection.tunnelMtu = parseInt(text) || 0
                enabled: connection.tunnelMtuSet
                validator: IntValidator{bottom: 0}
                Layout.fillWidth: true
            }
        }

        RowLayout {
            Layout.fillWidth: true

            CheckBox {
                checked: connection.fragmentSizeSet
                onCheckedChanged: connection.fragmentSizeSet = checked
            }

            Label {text: i18n.tr("Use UDP fragment size:")}

            TextField {
                text: connection.fragmentSize
                onTextChanged: connection.fragmentSize = parseInt(text) || 0
                enabled: connection.fragmentSizeSet
                validator: IntValidator{bottom: 0}
                Layout.fillWidth: true
            }
        }

        RowLayout {
            Layout.fillWidth: true

            CheckBox {
                checked: connection.mssFix
                onCheckedChanged: connection.mssFix = checked
            }

            Label {text: i18n.tr("Restrict tunnel TCP MSS")}
        }

        RowLayout {
            Layout.fillWidth: true

            CheckBox {
                checked: connection.remoteRandom
                onCheckedChanged: connection.remoteRandom = checked
            }

            Label {text: i18n.tr("Randomize remote hosts")}
        }
    }
}
