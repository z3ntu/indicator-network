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


ListItems.Base {
    id: listItem

    property alias text: label.text
    property alias control: controlContainer.control
    property alias checked: checkbox.checked

    CheckBox {
        id: checkbox

        anchors {
            verticalCenter: parent.verticalCenter
            left: parent.left
        }
    }

    Label {
        id: label
        anchors {
            verticalCenter: parent.verticalCenter
            left: checkbox.right
            margins: units.gu(1)
        }
        width: Math.min(implicitWidth, parent.width * 0.8)
    }

    Item {
        id: controlContainer
        property Item control
        // use the width of the control if there is (possibly elided) text,
        // or full width available if there is no text.
        width: control ? control.width : undefined
        height: control ? control.height : undefined
        anchors {
            verticalCenter: parent.verticalCenter
            right: parent.right
//            left: label.right
//            leftMargin: listItem.__contentsMargins
//            rightMargin: listItem.__contentsMargins
        }
        onControlChanged: {
            if (control) control.parent = controlContainer;
        }

        Connections {
            target: listItem.__mouseArea
            onClicked: listItem.clicked()
            onPressAndHold: listItem.pressAndHold()
        }
    }

}
