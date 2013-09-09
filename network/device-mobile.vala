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
using Notify;

namespace Network.Device
{
	public class Mobile : Base {
		private GLib.MenuItem enabled_item;
		private GLib.MenuItem settings_item;
		private GLib.MenuItem unlock_sim_item;
		private GLib.DBusConnection conn;

		private SimpleAction? unlock_action = null;
		private Notification? sim_notification = null;

		private Menu? unlock_menu = null;
		private uint unlock_menu_export_id = 0;
		private SimpleActionGroup? unlock_actions = null;
		private uint unlock_actions_export_id = 0;
		private string curren_pin = "";

		private const string APPLICATION_ID  = "com.canonical.indicator.network";
		private const string SIM_UNLOCK_MENU_PATH = "/com/canonical/indicator/network/unlocksim";
		private const string SIM_UNLOCK_ACTION_PATH = "/com/canonical/indicator/network/unlocksim";

		public Mobile (NM.Client client, NM.DeviceModem device, GLibLocal.ActionMuxer muxer, bool show_enable, GLib.DBusConnection conn) {
			GLib.Object(
				client: client,
				device: device,
				namespace: device.get_iface(),
				muxer: muxer
			);
			this.conn = conn;

			if (show_enable) {
				enabled_item = new MenuItem(_("Cellular"), "indicator." + device.get_iface() + ".device-enabled");
				enabled_item.set_attribute ("x-canonical-type"  ,           "s", "com.canonical.indicator.switch");
				_menu.append_item(enabled_item);
			}

			settings_item = new MenuItem(_("Cellular settings…"), "indicator.global.settings::cellular");
			_menu.append_item(settings_item);

			unlock_action = new SimpleAction("unlock", null);
			unlock_action.activate.connect(sim_unlock_activated);
			actions.insert(unlock_action);
		}

		~Mobile ()
		{
			muxer.remove(namespace);

			if (unlock_menu_export_id != 0)
				conn.unexport_menu_model(unlock_menu_export_id);
			if (unlock_menu_export_id != 0)
				conn.unexport_menu_model(unlock_actions_export_id);
		}

		public void sim_lock_updated (bool sim_locked)
		{
			string action_name = "indicator." + device.get_iface() + ".unlock";
			for (int i = 1; i < _menu.get_n_items(); i++)
			{
				string name;

				if (!_menu.get_item_attribute (i, "action", "s", out name))
					continue;

				if (name == action_name) {
					if (!sim_locked) {
						_menu.remove (i);
					}
					return;
				}
			}

			if (sim_locked) {
				unlock_sim_item = new MenuItem(_("Unlock SIM…"), action_name);
				_menu.insert_item (0, unlock_sim_item);
			}
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

		private void sim_unlock_activated (SimpleAction ac, Variant? val)
		{
			warning(@"SIM Unlock:");

			if (sim_notification != null) {
				return;
			}
			curren_pin = "";

			// notification
			sim_notification = new Notification("Unlock SIM", "", "totem");
			sim_notification.add_action ("cancel_id", _("Cancel"), cancel_callback);
			sim_notification.add_action ("ok_id", _("OK"), ok_callback);
			sim_notification.set_hint_string ("x-canonical-snap-decisions", "true");

			warning(@"conn unique_name $(conn.get_unique_name())");

			sim_notification.set_hint_string ("x-canonical-dbus-name", APPLICATION_ID);
			sim_notification.set_hint_string ("x-canonical-actions-path", SIM_UNLOCK_ACTION_PATH);
			sim_notification.set_hint_string ("x-canonical-menu-path", SIM_UNLOCK_MENU_PATH);

			// menu
			unlock_menu = new Menu ();
			var pin_unlock = new MenuItem ("", "notifications." + namespace + ".simunlock");
			pin_unlock.set_attribute ("x-canonical-type", "s", "com.canonical.snapdecision.pinlock");
			unlock_menu.append_item (pin_unlock);

			// actions
			unlock_actions = new SimpleActionGroup();
			var action = new SimpleAction.stateful(namespace + ".simunlock", null, new Variant.string(""));
			action.change_state.connect (simpin_changed);
			unlock_actions.insert (action);

			try {
				unlock_menu_export_id = conn.export_menu_model (SIM_UNLOCK_MENU_PATH, unlock_menu);
				unlock_actions_export_id = conn.export_action_group (SIM_UNLOCK_ACTION_PATH, unlock_actions as ActionGroup);

				if (unlock_action != null) {
					unlock_action.set_enabled (false);
				}
				sim_notification.closed.connect (notification_closed);
				sim_notification.show ();

			} catch (Error e) {
				warning(@"Unable to unlock sim: $(e.message)");
				return;
			}
		}

		private void cancel_callback (Notification notification, string action)
		{
			warning("Cancel: SIM Unlock");
		}

		private void ok_callback (Notification notification, string action)
		{
			warning("OK: SIM Unlock");
		}

		private void notification_closed ()
		{
			warning("Done with notification");
			if (unlock_action != null) {
				unlock_action.set_enabled(true);
			}
			sim_notification = null;

			conn.unexport_menu_model(unlock_menu_export_id);
			unlock_menu_export_id = 0;
			unlock_menu = null;

			conn.unexport_menu_model(unlock_actions_export_id);
			unlock_actions_export_id = 0;
			unlock_actions = null;
		}

		private void simpin_changed (SimpleAction ac, Variant? val)
		{
			warning("SIM Pin Changed");
			curren_pin = val.get_string();
		}
	}
}
