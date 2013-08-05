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
		private GLib.SimpleActionGroup actions = new GLib.SimpleActionGroup();
		private GLib.SimpleAction enabled_action;
		private GLib.MenuItem enabled_item;

		public Mobile (NM.Client client, NM.DeviceModem device, GLibLocal.ActionMuxer muxer) {
			GLib.Object(
				client: client,
				device: device,
				namespace: device.get_iface(),
				muxer: muxer
			);

			muxer.insert(namespace, actions);

			var is_enabled = client.wwan_get_enabled();
			enabled_action = new SimpleAction.stateful("device-enabled", null, new Variant.boolean(is_enabled));
			actions.insert(enabled_action);

			enabled_action.activate.connect((param) => {
				var nmstate = client.wwan_get_enabled();
				nmstate = !nmstate;

				debug("Requesting Mobile to: " + (nmstate ? "True" : "False"));
				client.wwan_set_enabled(nmstate);
			});

			client.notify.connect((pspec) => {
				if (pspec.name == "wwan-enabled") {
					var nmstate = client.wwan_get_enabled();

					debug("Setting Mobile to: " + (nmstate ? "True" : "False"));
					enabled_action.set_state(new Variant.boolean(nmstate));
				}
			});

			enabled_item = new MenuItem("Mobile", "indicator." + device.get_iface() + ".device-enabled");
			enabled_item.set_attribute ("x-canonical-type"  ,           "s", "com.canonical.indicator.switch");
			_menu.append_item(enabled_item);
			/* TODO: Need busy action */
		}

		~Mobile ()
		{
			muxer.remove(namespace);
		}


	}
}
