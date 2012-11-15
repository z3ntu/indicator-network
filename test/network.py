#!/usr/bin/env python3
# vim: tabstop=8 expandtab shiftwidth=4 softtabstop=4

import unittest
import dbus
import dbusmock
import subprocess
import os
import sys

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

if __name__ == '__main__':
    unittest.main(testRunner=unittest.TextTestRunner(stream=sys.stdout, verbosity=2))
