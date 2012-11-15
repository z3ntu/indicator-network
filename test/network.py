#!/usr/bin/env python3
# vim: tabstop=8 expandtab shiftwidth=4 softtabstop=4

import unittest
import dbus
import dbusmock
import subprocess
import os
import sys

ACTIVATED = 100

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

    def test_phone_one_eth(self):
        wifi1 = self.dbusmock.AddWiFiDevice('mock_wifi0', 'wlan0', ACTIVATED)
        self.dbusmock.AddAccessPoint (wifi1, 'mock_ap',
                'myap', '00:23:f8:7e:12:ba', 0, 2425, 5400, 80, 0x400) 
        p = subprocess.Popen(['chewie-network-menu-server'])

        bus = dbus.SessionBus ()
        actions = bus.get_object ('com.canonical.settings.network',
                                  '/com/canonical/settings/network')
        phone   = bus.get_object ('com.canonical.settings.network',
                                  '/com/canonical/settings/network/phone')

        p.terminate()
        p.wait()

if __name__ == '__main__':
    unittest.main(testRunner=unittest.TextTestRunner(stream=sys.stdout, verbosity=2))
