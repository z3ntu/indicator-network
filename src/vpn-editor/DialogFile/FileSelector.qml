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
import Ubuntu.Components.Popups 1.3

Item {
    property string path

    property var __dialog

    function createDialog() {
        __dialog = PopupUtils.open(fileDialogComponent)
        __dialog.accept.connect(pathAccepted)
        __dialog.reject.connect(pathRejected)
    }

    function destroyDialog() {
        __dialog.accept.disconnect(pathAccepted)
        __dialog.reject.disconnect(pathRejected)
        PopupUtils.close(__dialog)
    }

    function pathAccepted(newPath) {
        path = newPath
        destroyDialog()
    }

    function pathRejected() {
        destroyDialog()
    }

    Label {
        anchors {
            left: parent.left
            right: button.right
            verticalCenter: parent.verticalCenter
        }

        text: {
            var list = path.split("/")
            return list[list.length - 1]
        }
    }

    UbuntuShape {
        id: button
        aspect: foo.pressed? UbuntuShape.Inset : UbuntuShape.Flat

        width: units.gu(4)
        height: units.gu(4)

        anchors {
            right: parent.right
            verticalCenter: parent.verticalCenter
        }
        Icon {
            id: icon
            anchors {
                centerIn: button
                margins: units.gu(0.5)
            }

            name: "document-open"
            anchors.fill: parent

            MouseArea {
                id: foo
                anchors.fill: parent
                onClicked: createDialog()
            }
        }
    }
}
