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
# FIXME: remove when finished testing
from time import sleep

from autopilot import logging as autopilot_logging
from autopilot import input, platform
from autopilot.input._common import get_center_point

from unity8.process_helpers import unlock_unity
from unity8.shell.tests import UnityTestCase, _get_device_emulation_scenarios
import dbusmock
import subprocess

logger = logging.getLogger(__name__)

class IndicatorTestCase(UnityTestCase, dbusmock.DBusTestCase):
    """NOTE that this is proposed for unity8, remains here temporarily."""

    device_emulation_scenarios = _get_device_emulation_scenarios()

    @classmethod
    def setUpClass(cls):
        super(IndicatorTestCase, cls).setUpClass()
        # Set up a private buses.
        cls.start_session_bus();
        cls.start_system_bus()

    def setUp(self):
        #if platform.model() == 'Desktop':
        #    self.skipTest('Test cannot be run on the desktop.')
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
        os.environ['OFONO_PHONESIM_CONFIG'] = '/path/to/phonesim.conf'
        phonesim_cmd = ['ofono-phonesim', '-p', '12345', 'path/to/conf.xml']
        ofono_cmd = ['ofonod', 'arguments here']
        self.phonesim_process = subprocess.Popen(phonesim_cmd)
        self.ofono_process = subproces.Popen(ofono_cmd)

    def tearDown(self):
        self.phonesim_process.terminate()
        self.phonesim_process.wait()
        self.ofono_process.terminate()
        self.ofono_process.wait()

    def test_click_on_unlock_sim(self):
        """Open the network indicator and click on 'unlock sim'."""
        # TODO: self.main_window.open_indicator_page when above lands in unity8
        indicator_page = self.open_indicator_page(
            'indicator-network')
        unlock_sim_standard = indicator_page.wait_select_single(
            'Standard',
            objectName='indicator.sim.unlock')
        self.assertTrue(unlock_sim_standard.visible)
        self.pointing_device = input.Pointer(device=input.Touch.create())
        self.pointing_device.click_object(unlock_sim_standard)
        # FIXME: delete :)
        sleep(20)
