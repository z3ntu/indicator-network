#!/usr/bin/python2.7

from menuscript import Script, ActionList, MENU_OBJECT_PATH

al = ActionList(MENU_OBJECT_PATH)
menu0Id = al.createMenu(0, "Menu0", {"type" : "standard"})
menu1Id = al.createMenu(0, "Menu1", {"type" : "standard"})
menu2Id = al.createMenu(0, "Menu2", {"type" : "standard"})
menu3Id = al.createMenu(0, "Menu3", {"type" : "standard"})
menu4Id = al.createMenu(0, "Menu4", {"type" : "standard"})
al.destroyMenu(menu2Id)
al.destroyMenu(menu4Id)
menu4Id = al.createMenu(0, "Menu5", {"type": "standard"})
al.moveMenu(menu0Id, 2)

t = Script.create("/com/canonical/taxi", al)
t.run()
