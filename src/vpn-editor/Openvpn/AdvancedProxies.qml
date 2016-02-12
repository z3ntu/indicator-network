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
import Ubuntu.Components.ListItems 1.3 as ListItems

Page {
    property var connection

    property bool usesProxy: connection.proxyType !== OpenvpnConnection.NOT_REQUIRED
    property bool usesHttp: connection.proxyType === OpenvpnConnection.HTTP

    title: i18n.tr("Proxies")

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

            ListItems.ValueSelector {
                text: i18n.tr("Proxy type:")
                values: [
                    i18n.tr("Not required"),
                    i18n.tr("HTTP"),
                    i18n.tr("Socks"),
                ]
                selectedIndex: connection.proxyType
                onSelectedIndexChanged: connection.proxyType = selectedIndex
            }

            ListItems.Standard {
                text: i18n.tr("Server address:")
                control: TextField {
                    text: connection.proxyServer
                    onTextChanged: connection.proxyServer = text
                    width: units.gu(20)
                    inputMethodHints: Qt.ImhNoPredictiveText
                }
                enabled: usesProxy
            }

            ListItems.Standard {
                text: i18n.tr("Port:")
                control: TextField {
                    text: connection.proxyPort
                    onTextChanged: connection.proxyPort = parseInt(text) || 0
                    validator: IntValidator{bottom: 0}
                    width: units.gu(10)
                    inputMethodHints: Qt.ImhDigitsOnly
                }
                enabled: usesProxy
            }

            ListItems.Standard {
                text: i18n.tr("Retry indefinitely:")
                control: CheckBox {
                    id: proxyRetryCheckbox
                    Binding {target: proxyRetryCheckbox; property: "checked"; value: connection.proxyRetry}
                    onCheckedChanged: connection.proxyRetry = checked
                }
                enabled: usesProxy
            }

            ListItems.Standard {
                text: i18n.tr("Proxy username:")
                control: TextField {
                    text: connection.proxyUsername
                    onTextChanged: connection.proxyUsername = text
                    width: units.gu(20)
                    inputMethodHints: Qt.ImhNoPredictiveText
                }
                enabled: usesHttp
            }

            ListItems.Standard {
                text: i18n.tr("Proxy password:")
                control: TextField {
                    text: connection.proxyPassword
                    onTextChanged: connection.proxyPassword = text
                    width: units.gu(20)
                    echoMode: TextInput.Password
                }
                enabled: usesHttp
            }
        }
    }
}
