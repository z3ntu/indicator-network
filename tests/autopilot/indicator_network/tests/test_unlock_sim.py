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

import os
from testtools.matchers import Equals, NotEquals

from autopilot import input
from autopilot.matchers import Eventually
from pkg_resources import resource_filename
from unity8.process_helpers import unlock_unity
from unity8.shell.tests import UnityTestCase, _get_device_emulation_scenarios

from indicator_network.helpers.phonesim_manager import PhonesimManager

# FIXME:
# This is a workaround for https://bugs.launchpad.net/ubuntu-ui-toolkit/+bug/1314390  # noqa
# Remove once fixed in upstream and available through repos
from ubuntuuitoolkit import emulators as toolkit_emulators
class QQuickListView(toolkit_emulators.QQuickListView):

    def _get_top_container(self):
        """Return the top-most container with a globalRect."""
        root = self.get_root_instance()
        return root.get_children()[0].get_children()[1]


class UnlockSimTestCase(UnityTestCase):

    scenarios = _get_device_emulation_scenarios()

    def setUp(self):
        super(UnlockSimTestCase, self).setUp()
        # FIXME: use pkg_resources to ship
        sims = [('sim1',
                 12345,
                 os.path.join(
                     os.getcwd(),
                     'indicator-network/data/pin-unlock.xml')),]
        self.phonesim_manager = PhonesimManager(sims)
        self.phonesim_manager.start_phonesim_processes()
        self.phonesim_manager.remove_all_ofono()
        self.phonesim_manager.add_ofono('sim1')
        self.phonesim_manager.power_on('sim1')
        self.unity_proxy = self.launch_unity()
        unlock_unity(self.unity_proxy)
        self.pointing_device = input.Pointer(device=input.Touch.create())

    def tearDown(self):
        self.phonesim_manager.shutdown()
        super(UnlockSimTestCase, self).tearDown()

    def test_unlock_sim(self):
        """Unlock the SIM via the network indicator, entering PIN."""

        indicator_network_widget = self.main_window._get_indicator_widget(
            'indicator-network'
        )
        self.assertThat(
            indicator_network_widget.leftLabel,
            Eventually(Equals('SIM Locked'))
        )

        indicator_page = self.main_window.open_indicator_page(
            'indicator-network'
        )
        list_view = indicator_page.select_single(
            'QQuickListView',
            objectName='mainMenu'
        )
        unlock_sim_standard = list_view.wait_select_single(
            'Standard',
            objectName='indicator.sim.unlock'
        )
        self.assertTrue(unlock_sim_standard.visible)
        list_view.click_element('indicator.sim.unlock')

        pin_lockscreen = self.main_window.get_lockscreen()

        # FIXME: make helper?  Or generalize what's in unity8/shell/tests?
        pin_pad_button_1 = self.main_window.get_pinPadButton(1)
        pin_pad_button_2 = self.main_window.get_pinPadButton(2)
        pin_pad_button_3 = self.main_window.get_pinPadButton(3)
        pin_pad_button_4 = self.main_window.get_pinPadButton(4)
        pin_pad_button_erase = self.main_window.wait_select_single(
            'PinPadButton',
            objectName='pinPadButtonErase'
        )
        self.pointing_device.click_object(pin_pad_button_1)
        self.pointing_device.click_object(pin_pad_button_2)
        self.pointing_device.click_object(pin_pad_button_3)
        self.pointing_device.click_object(pin_pad_button_4)
        self.pointing_device.click_object(pin_pad_button_erase)

        self.assertThat(
            indicator_network_widget.leftLabel,
            Eventually(NotEquals('SIM Locked'))
        )
