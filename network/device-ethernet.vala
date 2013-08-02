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
		private GLib.SimpleActionGroup actions = new GLib.SimpleActionGroup();
		private GLib.SimpleAction enabled_action;
		private GLib.SimpleAction busy_action;
		private GLib.MenuItem enabled_item;
		private NM.ActiveConnection? active_connection = null;
		private ulong active_connection_notify = 0;

		public Ethernet (NM.Client client, NM.DeviceEthernet device, GLibLocal.ActionMuxer muxer) {
			GLib.Object(
				client: client,
				device: device,
				namespace: device.get_iface(),
				muxer: muxer
			);

			muxer.insert(namespace, actions);

			enabled_action = new SimpleAction.stateful("device-enabled", null, new Variant.boolean(false));
			busy_action = new SimpleAction.stateful("device-busy", null, new Variant.boolean(false));

			actions.insert(enabled_action);
			actions.insert(busy_action);

			enabled_action.activate.connect((param) => {
				if (enabled_action.state.get_boolean()) {
					device.disconnect(null);
				} else {
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
			});

			device.notify.connect((pspec) => {
				if (pspec.name == "active-connection") {
					active_connection_changed();
				}
			});
			active_connection_changed();

			enabled_item = new MenuItem("Wired", "indicator." + device.get_iface() + ".device-enabled");
			enabled_item.set_attribute ("x-canonical-type"  ,           "s", "com.canonical.indicator.switch");
			_menu.append_item(enabled_item);
			/* TODO: Need busy action */
		}

		~Ethernet ()
		{
			muxer.remove(namespace);
		}

		private void active_connection_changed () {
			if (active_connection != null) {
				active_connection.disconnect(active_connection_notify);
			}

			active_connection = this.device.get_active_connection();

			if (active_connection != null) {
				active_connection_notify = active_connection.notify.connect((pspec) => {
					if (pspec.name == "state") {
						active_connection_state_changed(active_connection.state);
					}
				});

				active_connection_state_changed(active_connection.state);
			}
		}

		private void active_connection_state_changed (uint new_state) {
			switch (new_state) {
				case NM.ActiveConnectionState.ACTIVATING:
					debug("Marking Ethernet as Activating");
					busy_action.set_state(new Variant.boolean(true));
					enabled_action.set_state(new Variant.boolean(true));
					break;
				case NM.ActiveConnectionState.ACTIVATED:
					debug("Marking Ethernet as Active");
					busy_action.set_state(new Variant.boolean(false));
					enabled_action.set_state(new Variant.boolean(true));
					break;
				case NM.ActiveConnectionState.DEACTIVATING:
					debug("Marking Ethernet as Deactivating");
					busy_action.set_state(new Variant.boolean(true));
					enabled_action.set_state(new Variant.boolean(false));
					break;
				default:
					debug("Marking Ethernet as Disabled");
					enabled_action.set_state(new Variant.boolean(false));
					enabled_action.set_state(new Variant.boolean(false));
					break;
			}
		}

	}
}
