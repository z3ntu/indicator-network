import os.path
from apport.hookutils import *
from xdg.BaseDirectory import xdg_cache_home

def add_info(report):
	if not apport.packaging.is_distro_package(report['Package'].split()[0]):
		report['ThirdParty'] = 'True'
		report['CrashDB'] = 'indicator_network'

	attach_file_if_exists(report, os.path.join(xdg_cache_home, 'upstart', 'indicator-network.log'), 'indicator-network.log')
