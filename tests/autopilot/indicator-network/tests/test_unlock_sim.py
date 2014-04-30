# -*- Mode: Python; coding: utf-8; indent-tabs-mode: nil; tab-width: 4 -*-
#
# Unity Indicators Autopilot Test Suite
# Copyright (C) 2014 Canonical
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

from __future__ import absolute_import

import logging

from autopilot import logging as autopilot_logging
from autopilot import input, platform
from autopilot.input._common import get_center_point

from unity8.process_helpers import unlock_unity
from unity8.shell.tests import UnityTestCase, _get_device_emulation_scenarios
import dbusmock
import subprocess

from PhonesimManager import PhonesimManager
from time import sleep

logger = logging.getLogger(__name__)

# FIXME:
# This is a workaround for https://bugs.launchpad.net/ubuntu-ui-toolkit/+bug/1314390
# Remove once fixed in upstream and available through repos
from ubuntuuitoolkit import emulators as toolkit_emulators
class QQuickListView(toolkit_emulators.QQuickListView):

    def _get_top_container(self):
        """Return the top-most container with a globalRect."""
        root = self.get_root_instance()
        return root.get_children()[0].get_children()[1]


class IndicatorTestCase(UnityTestCase, dbusmock.DBusTestCase):
    """NOTE that this is proposed for unity8, remains here temporarily."""

    device_emulation_scenarios = _get_device_emulation_scenarios()

    @classmethod
    def setUpClass(cls):
        super(IndicatorTestCase, cls).setUpClass()

    def setUp(self):            
        super(IndicatorTestCase, self).setUp()    
        self.unity_proxy = self.launch_unity()
        unlock_unity(self.unity_proxy)
        self.pointing_device = input.Pointer(device=input.Touch.create())

    def get_indicator_widget(self, indicator_name):
        return self.main_window.select_single(
            'DefaultIndicatorWidget',
            objectName=indicator_name+'-widget'
        )

    def get_indicator_page(self, indicator_name):
        return self.main_window.select_single(
            'DefaultIndicatorPage',
            objectName=indicator_name+'-page'
        )

    @autopilot_logging.log_action(logger.info)
    def open_indicator_page(self, indicator_name):
        """Return the indicator page.

        Swipe to open the indicator, wait until it's open.
        """
        widget = self.get_indicator_widget(indicator_name)
        start_x, start_y = get_center_point(widget)
        end_x = start_x
        end_y = self.main_window.height
        self.pointing_device.drag(start_x, start_y, end_x, end_y)
        # TODO: assert that the indicator page opened [alesage 2013-12-06]
        return self.get_indicator_page(indicator_name)

    @autopilot_logging.log_action(logger.info)
    def close_indicator_page(self, indicator_name):
        """Swipe to close the indicator, wait until it's closed."""
        widget = self.get_indicator_widget(indicator_name)
        end_x, end_y = get_center_point(widget)
        start_x = end_x
        start_y = self.main_window.height
        self.pointing_device.drag(start_x, start_y, end_x, end_y)
        # TODO: assert that the indicator page closed [alesage 2013-12-06]


class UnlockSimTestCase(IndicatorTestCase):

    scenarios = IndicatorTestCase.device_emulation_scenarios

    def setUp(self):
        super(UnlockSimTestCase, self).setUp()
        sims = [('sim1', 12345, '/usr/share/phonesim/default.xml'),]
        self.phonesim_manager = PhonesimManager(sims);
        self.phonesim_manager.start_phonesim_processes()

    def tearDown(self):
        m.shutdown()
        super(UnlockSimTestCase, self).tearDown()

    def test_click_on_unlock_sim(self):
        """Open the network indicator and click on 'unlock sim'."""
        # TODO: self.main_window.open_indicator_page when above lands in unity8
        indicator_page = self.open_indicator_page(
            'indicator-network')
        
        listview = indicator_page.select_single('QQuickListView', objectName='mainMenu')
        unlock_sim_standard = listview.wait_select_single(
            'Standard',
            objectName='indicator.sim.unlock')
        self.assertTrue(unlock_sim_standard.visible)
        self.pointing_device = input.Pointer(device=input.Touch.create())

        #listview.print_tree(output='listview.log')
        listview.click_element('indicator.sim.unlock')

        #self.unity_proxy.print_tree()
        sleep(5)
        # FIXME: delete :)
        

if __name__ == '__main__':
    print("FOOOOO")
    sims = [('sim1', 12345, '/usr/share/phonesim/default.xml'),
            ('sim2', 12346, '/usr/share/phonesim/default.xml')]
    m = PhonesimManager(sims)
    m.start_phonesim_processes()
    sleep(20)
    m.shutdown()
