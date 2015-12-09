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

ColumnLayout {
    property var connection

    spacing: units.gu(1)

    RowLayout {
        Layout.fillWidth: true

        Label {text: i18n.tr("User certificate:")}
        TextField {
            text: connection.cert
            onTextChanged: connection.cert = text
            Layout.fillWidth: true
        }
    }

    RowLayout {
        Layout.fillWidth: true

        Label {text: i18n.tr("CA certificate:")}
        TextField {
            text: connection.ca
            onTextChanged: connection.ca = text
            Layout.fillWidth: true
        }
    }

    RowLayout {
        Layout.fillWidth: true

        Label {text: i18n.tr("Private key:")}
        TextField {
            text: connection.key
            onTextChanged: connection.key = text
            Layout.fillWidth: true
        }
    }

    RowLayout {
        Layout.fillWidth: true

        Label {text: i18n.tr("Private key password:")}
        TextField {
            text: connection.certPass
            onTextChanged: connection.certPass = text
            Layout.fillWidth: true
        }
    }
}
