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
    root = ret[0][2][0]

    #TODO: Check several devices/sections
    self.assertTrue(':section' in root)
    self.assertTrue('label'   in root)
    self.assertTrue('type'    in root)
    for prop in ('busy-action', 'children-display', 'widget-type', 'wifi-device-path'):
        self.assertTrue(('x-canonical-' + prop) in root)

    for ap in aps:
        ap_path = AP_PREFIX + ap
        items = [item[2][0] for item in ret[1:]]

        res = map(lambda item: item['x-canonical-wifi-ap-dbus-path'] == ap_path, items)
        self.assertTrue (functools.reduce(lambda a, b: a and b, res))
            

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
