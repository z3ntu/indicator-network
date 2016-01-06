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

Column {
    property var connection

    ListItems.Standard {
        control: FileSelector {
            path: connection.staticKey
            onPathChanged: connection.staticKey = text
            width: units.gu(20)
        }
        text: i18n.tr("Static key:")
    }

    ListItems.ValueSelector {
        text: i18n.tr("Key direction:")
        values: [
            i18n.tr("None"),
            i18n.tr("0"),
            i18n.tr("1"),
        ]
        selectedIndex: connection.staticKeyDirection
        onSelectedIndexChanged: connection.staticKeyDirection = selectedIndex
    }

    ListItems.Standard {
        control: TextField {
            text: connection.remoteIp
            onTextChanged: connection.remoteIp = text
            width: units.gu(20)
            inputMethodHints: Qt.ImhNoPredictiveText
        }
        text: i18n.tr("Remote IP:")
    }

    ListItems.Standard {
        control: TextField {
            text: connection.localIp
            onTextChanged: connection.localIp = text
            width: units.gu(20)
            inputMethodHints: Qt.ImhNoPredictiveText
        }
        text: i18n.tr("Local IP:")
    }
}
