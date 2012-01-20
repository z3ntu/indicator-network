import dbus
import dbus.service
from dbus import glib
from dbus.mainloop.glib import DBusGMainLoop
from gi.repository import GObject
from gi.repository import Dbusmenu
from gi.repository import Gio

SERVICE_NAME     = "com.canonical.test"
INTERFACE_NAME   = "com.canonical.test.menuscript"
OBJECT_PATH      = "/com/canonical/test/menuscript"
MENU_SERVICE_NAME= SERVICE_NAME + ".menu"
MENU_OBJECT_PATH = OBJECT_PATH + "/menu"
bus = None

class Script(dbus.service.Object):
    def __init__(self, aList, session, object_path):
        dbus.service.Object.__init__(self, session, object_path)
        self._list = aList
        self._session  = session

    def run(self):
        self._loop = GObject.MainLoop()
        self._list.start()
        self._loop.run()

    @dbus.service.method(dbus_interface=INTERFACE_NAME,
                         in_signature='', out_signature='',
                         sender_keyword='sender')
    def quit(self, sender=None):
        self._loop.quit()


    @dbus.service.method(dbus_interface=INTERFACE_NAME,
                         in_signature='i', out_signature='',
                         sender_keyword='sender')
    def walk(self, steps, sender=None):
        if steps == -1 or steps > self._list.size():
            steps = self._list.size()

        while(steps > 0):
            self._list.walk()
            steps -= 1

    @staticmethod
    def create(objectPath, aList):
        global bus

        GObject.threads_init()
        glib.threads_init()

        dbus_loop = DBusGMainLoop()
        bus = dbus.SessionBus(mainloop=dbus_loop)
        bus_name = dbus.service.BusName(SERVICE_NAME, bus=bus)
        return Script(aList, bus_name, OBJECT_PATH)

class Action(object):
    def __init__(self, aList, action, **kwargs):
        self._list = aList
        self._action = action
        self._kargs = kwargs

    def createMenu(self):
        menu = Dbusmenu.Menuitem.new_with_id(self._kargs['menuId'])
        menu.property_set('label', self._kargs['label'])
        props = self._kargs['properties']
        for key in props:
            propType = type(props[key])
            if propType == int or propType == long:
                menu.property_set_int(key, props[key])
            elif propType == bool:
                menu.property_set_bool(key, props[key])
            elif propType == str:
                menu.property_set(key, props[key])
            else:
                menu.property_set_variant(key, props[key])

        parentId = self._kargs['parentId']
        if parentId > 0:
            parent = self._list.getMenu(parentId)
        else:
            parent = self._list._root
        parent.child_append(menu)

    def destroyMenu(self):
        menu = self._list.getMenu(self._kargs['menuId'])
        menu.get_parent().child_delete(menu)

    def moveMenu(self):
        menu = self._list.getMenu(self._kargs['menuId'])
        menuParent = menu.get_parent()
        menuPos = menu.get_position(menuParent)

        newPos = menuPos + self._kargs['posOffset']
        menuParent.child_reorder(menu, newPos)

    def run(self):
        if self._action == 'create':
            self.createMenu()
        elif self._action == 'destroy':
            self.destroyMenu()
        elif self._action == 'move':
            self.moveMenu()

class ActionList(object):
    def __init__(self, objectPath):
        self._actions = []
        self._objectPath = objectPath
        self._server = None
        self._root = None
        self._menuCount = 10

    def createMenu(self, parentId, label, properties):
        self._actions.append(Action(self, 'create', menuId=self._menuCount, parentId=parentId, label=label, properties=properties))
        menuId = self._menuCount
        self._menuCount += 1
        return menuId

    def destroyMenu(self, menuId):
        self._actions.append(Action(self, 'destroy', menuId=menuId))

    def moveMenu(self, menuId, offset):
        self._actions.append(Action(self, 'move', menuId=menuId, posOffset=offset))

    def getMenu(self, menuId):
        if self._root:
            return self._root.find_id(menuId)
        else:
            return None

    def walk(self):
        item = self._actions.pop(0)
        item.run()

    def size(self):
        return len(self._actions)

    def _exportService(self, connection, name):
        self._server = Dbusmenu.Server.new(self._objectPath)
        self._root = Dbusmenu.Menuitem()
        self._server.set_root(self._root)

    def start(self):
        Gio.bus_own_name(2, MENU_SERVICE_NAME, 0, self._exportService, None, None)


