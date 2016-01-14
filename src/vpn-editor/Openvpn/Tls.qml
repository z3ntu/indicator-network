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
import "../DialogFile"

Column {
    property var connection

    ListItems.Standard {
        control: FileSelector {
            path: connection.cert
            onPathChanged: connection.cert = path
            width: units.gu(20)
        }
        text: i18n.tr("User certificate:")
    }

    ListItems.Standard {
        control: FileSelector {
            path: connection.ca
            onPathChanged: connection.ca = path
            width: units.gu(20)
        }
        text: i18n.tr("CA certificate:")
    }

    ListItems.Standard {
        control: FileSelector {
            path: connection.key
            onPathChanged: connection.key = path
            width: units.gu(20)
        }
        text: i18n.tr("Private key:")
    }

    ListItems.Standard {
        control: TextField {
            text: connection.certPass
            onTextChanged: connection.certPass = text
            width: units.gu(20)
        }
        text: i18n.tr("Key password:")
    }
}
