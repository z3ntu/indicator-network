#!/usr/bin/env python3
# vim: tabstop=8 expandtab shiftwidth=4 softtabstop=4

import unittest
import dbus
import dbusmock
import subprocess
import os
import sys
import time
import functools

ACTIVATED = 100
AP_PREFIX = "/org/freedesktop/NetworkManager/AccessPoint/"

def check_aps_actions(self, ret, aps):
    for ap in aps:
        ap_action = AP_PREFIX + ap
        strength_action = ap_action + "::strength"
        self.assertTrue(ap_action in ret)
        self.assertTrue(strength_action in ret)

def check_aps_in_menu(self, ret, aps):
    ap_items = []
    for group in ret:
        a, b, items = group
        for item in items:
            if 'x-canonical-wifi-ap-dbus-path' in item:
                ap_items.append(item)

    self.assertTrue(len(ap_items) > 0)

    for ap in aps:
        ap_path = AP_PREFIX + ap
        items_map = map(lambda item: item['x-canonical-wifi-ap-dbus-path'] == ap_path, items)
        has_ap = functools.reduce(lambda a, b: a or b, items_map)
        self.assertTrue (has_ap)


class TestNetworkMenu(dbusmock.DBusTestCase):
    '''Test network menu'''

    @classmethod
    def setUpClass(klass):
        klass.start_system_bus()
        klass.dbus_con = klass.get_dbus(True)

    def setUp(self):
        (self.p_mock, self.obj_networkmanager) = self.spawn_server_template(
                'networkmanager',
                {'NetworkingEnabled': True, 'WwanEnabled': False},
                stdout=subprocess.PIPE)
        self.dbusmock = dbus.Interface(self.obj_networkmanager,
                                       dbusmock.MOCK_IFACE)

    def tearDown(self):
        self.p_mock.terminate()
        self.p_mock.wait()

    def test_phone_one_wlan(self):
        aps   = ['mock_ap',]
        wifi1 = self.dbusmock.AddWiFiDevice('mock_wifi0', 'wlan0', ACTIVATED)

        self.dbusmock.AddAccessPoint (wifi1, 'mock_ap',
                'myap', '00:23:f8:7e:12:ba', 0, 2425, 5400, 80, 0x400)
        
        p = subprocess.Popen(['chewie-network-menu-server'])
        time.sleep (0.3)

        bus = dbus.SessionBus ()

        actions = bus.get_object ('com.canonical.settings.network',
                                  '/com/canonical/settings/network')
        phone   = bus.get_object ('com.canonical.settings.network',
                                  '/com/canonical/settings/network/phone')

        actions_iface    = dbus.Interface(actions, dbus_interface='org.gtk.Actions')
        phone_menu_iface = dbus.Interface(phone,   dbus_interface='org.gtk.Menus')

        check_aps_actions (self, actions_iface.List(), aps)
        check_aps_in_menu (self, phone_menu_iface.Start([0]), aps)

        p.terminate()
        p.wait()

if __name__ == '__main__':
    unittest.main(testRunner=unittest.TextTestRunner(stream=sys.stdout, verbosity=2))
