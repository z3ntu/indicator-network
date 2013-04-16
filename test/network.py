#!/usr/bin/env python3
# vim: tabstop=8 expandtab shiftwidth=4 softtabstop=4

# Copyright 2013 Canonical Ltd.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation; version 3.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
# Authors:
#      Alberto Ruiz <alberto.ruizo@canonical.com>

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


    def start_menu_server(self):
        self.menuserver = subprocess.Popen(['indicator-network-menu-server'])
        time.sleep (1.0)

    def stop_menu_server(self):
        self.menuserver.terminate()
        self.menuserver.wait()

    def add_aps (self, wifi_dev, aps):
        for ap in aps:
            self.dbusmock.AddAccessPoint(wifi_dev, ap[0], ap[1], ap[2], ap[3], ap[4], ap[5], ap[6], ap[7])

    def get_actions_and_menus(self):
        bus = dbus.SessionBus()
        actions = bus.get_object('com.canonical.settings.network',
                                 '/com/canonical/settings/network')
        phone   = bus.get_object('com.canonical.settings.network',
                                 '/com/canonical/settings/network/phone')

        actions_iface    = dbus.Interface(actions, dbus_interface='org.gtk.Actions')
        phone_menu_iface = dbus.Interface(phone,   dbus_interface='org.gtk.Menus')

        return (actions_iface, phone_menu_iface)

    def test_phone_one_wlan(self):
        aps   = ['mock_ap',]
        wifi1 = self.dbusmock.AddWiFiDevice('mock_wifi0', 'wlan0', ACTIVATED)

        self.dbusmock.AddAccessPoint(wifi1, 'mock_ap',
                'myap', '00:23:f8:7e:12:ba', 0, 2425, 5400, 80, 0x400)
        
        self.start_menu_server()

        actions, menus = self.get_actions_and_menus ()

        check_aps_actions(self, actions.List(), aps)
        check_aps_in_menu(self, menus.Start([0, 1]), aps)

        self.stop_menu_server()

    def test_phone_duplicated_aps(self):
        # ap path name, ssid, hw address, mode, freq, rate, strength, security
        aps = [('1', 'ap1', '00:23:f8:7e:12:00', 0, 2425, 5400, 50, 0x400),
               ('2', 'ap1', '00:23:f8:7e:12:01', 0, 2425, 5400, 80, 0x400),
               ('3', 'ap1', '00:23:f8:7e:12:02', 0, 2425, 5400, 90, 0x400),
               ('4', 'ap1', '00:23:f8:7e:12:03', 0, 2425, 5400, 40, 0x400),
               ('5', 'ap1', '00:23:f8:7e:12:04', 0, 2425, 5400, 50, 0x400),]

        wifi1 = self.dbusmock.AddWiFiDevice('mock_wifi0', 'wlan0', ACTIVATED)

        self.add_aps(wifi1, aps)

        actions, menus = self.get_actions_and_menus ()

        print (actions.List())

        self.start_menu_server()
        self.stop_menu_server()


if __name__ == '__main__':
    unittest.main(testRunner=unittest.TextTestRunner(stream=sys.stdout, verbosity=2))
