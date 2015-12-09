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

    title: i18n.tr("TLS authentication:")

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

            Label {text: i18n.tr("Subject match:")}

            TextField {
                text: connection.tlsRemote
                onTextChanged: connection.tlsRemote = text
                Layout.fillWidth: true
            }
        }

        RowLayout {
            Layout.fillWidth: true

            CheckBox {
                checked: connection.remoteCertTlsSet
                onCheckedChanged: connection.remoteCertTlsSet = checked
            }

            Label {
                text: i18n.tr("Verify peer certificate:")
            }
        }

        RowLayout {
            Layout.fillWidth: true
            enabled: connection.remoteCertTlsSet

            Label {
                text: i18n.tr("Peer certificate TLS type:")
//                enabled: connection.remoteCertTlsSet
            }
            QQC.ComboBox {
                model: [
                    i18n.tr("Server"),
                    i18n.tr("Client"),
                ]
                currentIndex: connection.remoteCertTls
                onCurrentIndexChanged: connection.remoteCertTls = currentIndex
                enabled: connection.remoteCertTlsSet
                Layout.fillWidth: true
            }
        }

        RowLayout {
            Layout.fillWidth: true

            CheckBox {
                checked: connection.taSet
                onCheckedChanged: connection.taSet = checked
            }

            Label {text: i18n.tr("Use additional TLS authentication:")}
        }

        RowLayout {
            Layout.fillWidth: true

            enabled: connection.taSet

            Label {text: i18n.tr("Key file:")}
            TextField {
                text: connection.ta
                onTextChanged: connection.ta = text
                Layout.fillWidth: true
            }
        }
        RowLayout {
            Layout.fillWidth: true

            enabled: connection.taSet

            Label {text: i18n.tr("Key direction:")}
            QQC.ComboBox {
                model: [
                    i18n.tr("None"),
                    i18n.tr("0"),
                    i18n.tr("1"),
                ]
                currentIndex: connection.taDir
                onCurrentIndexChanged: connection.taDir = currentIndex
                enabled: connection.taSet
                Layout.fillWidth: true
            }
        }
    }
}
