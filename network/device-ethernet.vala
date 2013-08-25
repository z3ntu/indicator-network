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
	public class Ethernet : Base {
		private GLib.MenuItem enabled_item;

		public Ethernet (NM.Client client, NM.DeviceEthernet device, GLibLocal.ActionMuxer muxer) {
			GLib.Object(
				client: client,
				device: device,
				namespace: device.get_iface(),
				muxer: muxer
			);

			enabled_item = new MenuItem(_("Wired"), "indicator." + device.get_iface() + ".device-enabled");
			enabled_item.set_attribute ("x-canonical-type"  ,           "s", "com.canonical.indicator.switch");
			_menu.append_item(enabled_item);
			/* TODO: Need busy action */
		}

		~Ethernet ()
		{
			muxer.remove(namespace);
		}

		protected override void enable_device ()
		{
			var conn = new NM.Connection();

			var swired = new NM.SettingWired();
			conn.add_setting(swired);

			var sconn = new NM.SettingConnection();
			sconn.id = "Auto Ethernet";
			sconn.type = NM.SettingWired.SETTING_NAME;
			sconn.autoconnect = true;
			sconn.uuid = NM.Utils.uuid_generate();
			conn.add_setting(sconn);

			client.add_and_activate_connection(conn, this.device, "/", null);
		}
	}
}
