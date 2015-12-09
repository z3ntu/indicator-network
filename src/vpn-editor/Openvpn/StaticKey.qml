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

ColumnLayout {
    property var connection

    spacing: units.gu(1)

    RowLayout {
        Layout.fillWidth: true

        Label {text: i18n.tr("Static key:")}
        TextField {
            text: connection.staticKey
            onTextChanged: connection.staticKey = text
            Layout.fillWidth: true
        }
    }

    RowLayout {
        Layout.fillWidth: true

        Label {text: i18n.tr("Key direction:")}
        QQC.ComboBox {
            model: [
                i18n.tr("None"),
                i18n.tr("0"),
                i18n.tr("1"),
            ]
            currentIndex: connection.staticKeyDirection
            onCurrentIndexChanged: connection.staticKeyDirection = currentIndex
            Layout.fillWidth: true
        }
    }

    RowLayout {
        Layout.fillWidth: true

        Label {text: i18n.tr("Remote IP address")}
        TextField {
            text: connection.remoteIp
            onTextChanged: connection.remoteIp = text
            Layout.fillWidth: true
        }
    }

    RowLayout {
        Layout.fillWidth: true

        Label {text: i18n.tr("Local IP address")}
        TextField {
            text: connection.localIp
            onTextChanged: connection.localIp = text
            Layout.fillWidth: true
        }
    }
}
