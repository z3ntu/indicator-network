#!/usr/bin/python -tt

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

import subprocess

class PhonesimManager:
    def __init__(self):
        self.phonesim_exe = '/usr/bin/ofono-phonesim'
        self.sims = [('sim1', 12345, '/usr/share/phonesim/default.xml'),
                     ('sim2', 12346, '/usr/share/phonesim/default.xml')]
        self.sim_processes = {}

    def start_phonesim_processes(self):
        for simname, simport, conffile in self.sims:
            cmd = [self.phonesim_exe, '-p', str(simport), conffile]
            p = subprocess.Popen(cmd)
            self.sim_processes[simname] = p

    def shutdown(self):
        for p in self.sim_processes.values():
            p.kill()
        self.sim_processes = {}

if __name__ == '__main__':
    m = PhonesimManager()
    m.start_phonesim_processes()
    m.shutdown()

