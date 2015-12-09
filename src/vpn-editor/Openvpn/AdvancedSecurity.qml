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
import QtQuick.Controls 1.3 as QQC
import Ubuntu.Components 1.3

Page {
    property var connection

    title: i18n.tr("Security")

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

            Label {text: i18n.tr("Cipher:")}
            QQC.ComboBox {
                model: [
                    i18n.tr("Default"),
                    i18n.tr("DES-CBC"),
                    i18n.tr("RC2-CBC"),
                    i18n.tr("DES-EDE-CBC"),
                    i18n.tr("DES-EDE3-CBC"),
                    i18n.tr("DESX-CBC"),
                    i18n.tr("RC2-40-CBC"),
                    i18n.tr("CAST5-CBC"),
                    i18n.tr("AES-128-CBC"),
                    i18n.tr("AES-192-CBC"),
                    i18n.tr("CAMELLIA-128-CBC"),
                    i18n.tr("CAMELLIA-192-CBC"),
                    i18n.tr("CAMELLIA-256-CBC"),
                    i18n.tr("SEED-CBC"),
                    i18n.tr("AES-128-CBC-HMAC-SHA1"),
                    i18n.tr("AES-256-CBC-HMAC-SHA1"),
                ]
                currentIndex: connection.cipher
                onCurrentIndexChanged: connection.cipher = currentIndex
                Layout.fillWidth: true
            }
        }

        RowLayout {
            Layout.fillWidth: true

            CheckBox {
                checked: connection.keysizeSet
                onCheckedChanged: connection.keysizeSet = checked
            }

            Label {text: i18n.tr("Use custom size of cipher key:")}

            TextField {
                text: connection.keysize
                onTextChanged: connection.keysize = parseInt(text) || 0
                enabled: connection.keysizeSet
                validator: IntValidator{bottom: 0}
                Layout.fillWidth: true
            }
        }

        RowLayout {
            Layout.fillWidth: true

            Label {text: i18n.tr("HMAC authentication:")}
            QQC.ComboBox {
                model: [
                    i18n.tr("Default"),
                    i18n.tr("None"),
                    i18n.tr("RSA MD-4"),
                    i18n.tr("MD-5"),
                    i18n.tr("SHA-1"),
                    i18n.tr("SHA-224"),
                    i18n.tr("SHA-256"),
                    i18n.tr("SHA-384"),
                    i18n.tr("SHA-512"),
                    i18n.tr("RIPEMD-160")
                ]
                currentIndex: connection.auth
                onCurrentIndexChanged: connection.auth = currentIndex
                enabled: connection.devTypeSet
                Layout.fillWidth: true
            }
        }
    }
}
