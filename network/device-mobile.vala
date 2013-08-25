// vim: tabstop=4 noexpandtab shiftwidth=4 softtabstop=4
/*
 * Copyright 2013 Canonical Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors:
 *      Ted Gould <ted.gould@canonical.com>
 */

using NM;

namespace Network.Device
{
	public class Mobile : Base {
		private GLib.MenuItem enabled_item;
		private GLib.MenuItem settings_item;

		public Mobile (NM.Client client, NM.DeviceModem device, GLibLocal.ActionMuxer muxer, bool show_enable) {
			GLib.Object(
				client: client,
				device: device,
				namespace: device.get_iface(),
				muxer: muxer
			);

			if (show_enable) {
				enabled_item = new MenuItem("Cellular", "indicator." + device.get_iface() + ".device-enabled");
				enabled_item.set_attribute ("x-canonical-type"  ,           "s", "com.canonical.indicator.switch");
				_menu.append_item(enabled_item);
			}

			settings_item = new MenuItem("Cellular settings…", "indicator.global.settings::cellular");
			_menu.append_item(settings_item);
		}

		~Mobile ()
		{
			muxer.remove(namespace);
		}

		protected override void disable_device ()
		{
			device.disconnect(null);
			client.wwan_set_enabled(false);
		}

		protected override void enable_device ()
		{
			client.wwan_set_enabled(true);
			device.set_autoconnect(true);
		}
	}
}
