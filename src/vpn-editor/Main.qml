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
import "DialogFile"

MainView {
    id: root
    objectName: "mainView"
    applicationName: "com.ubuntu.developer.pete-woods.vpn-editor"

    width: units.gu(40)
    height: units.gu(60)

    Component {
        id: fileDialogComponent
        DialogFile {
            id: fileDialog
        }
    }

    PageStack {
        id: pageStack
        Component.onCompleted: push(vpnList)

        VpnList {
            id: vpnList
            visible: false
            model: Connectivity.vpnConnections
        }
    }
}
