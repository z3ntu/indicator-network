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
import Ubuntu.Connectivity 1.0

Page {
    property var connection

    property bool usesProxy: connection.proxyType !== OpenvpnConnection.NOT_REQUIRED
    property bool usesHttp: connection.proxyType === OpenvpnConnection.HTTP

    title: i18n.tr("Proxies")

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

            Label {text: i18n.tr("Proxy type:")}
            QQC.ComboBox {
                model: [
                    i18n.tr("Not required"),
                    i18n.tr("HTTP"),
                    i18n.tr("Socks"),
                ]
                currentIndex: connection.proxyType
                onCurrentIndexChanged: connection.proxyType = currentIndex
                Layout.fillWidth: true
            }
        }

        RowLayout {
            Layout.fillWidth: true
            enabled: usesProxy

            Label {text: i18n.tr("Server address:")}

            TextField {
                text: connection.proxyServer
                onTextChanged: connection.proxyServer = text
                Layout.fillWidth: true
            }
        }

        RowLayout {
            Layout.fillWidth: true
            enabled: usesProxy

            Label {text: i18n.tr("Port:")}

            TextField {
                text: connection.proxyPort
                onTextChanged: connection.proxyPort = parseInt(text) || 0
                validator: IntValidator{bottom: 0}
                Layout.fillWidth: true
            }
        }

        RowLayout {
            Layout.fillWidth: true
            enabled: usesProxy

            CheckBox {
                checked: connection.proxyRetry
                onCheckedChanged: connection.proxyRetry = checked
            }

            Label {text: i18n.tr("Retry indefinitely when errors occur:")}
        }

        RowLayout {
            Layout.fillWidth: true
            enabled: usesHttp

            Label {text: i18n.tr("Proxy username:")}
            TextField {
                text: connection.proxyUsername
                onTextChanged: connection.proxyUsername = text
                Layout.fillWidth: true
            }
        }

        RowLayout {
            Layout.fillWidth: true
            enabled: usesHttp

            Label {text: i18n.tr("Proxy password:")}
            TextField {
                text: connection.proxyPassword
                onTextChanged: connection.proxyPassword = text
                Layout.fillWidth: true
            }
        }
    }
}
