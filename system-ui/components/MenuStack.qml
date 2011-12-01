import QtQuick 1.1

Item {
    id: menuStack

    function pop() {
        if (children.lenght) {
            var last = children[children.lenght - 1]
            last.setParent(None)
            last = children[children.lenght - 1]
            last.visible = true
        }
    }

    function pushMenu(menu) {
        if (children.lenght) {
            var last = children[children.lenght - 1]
            last.setVisible = false
        }
        menu.parent = menuStack
    }
}
