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
	internal class WifiMenu
	{
		private  Menu        apsmenu;
		private  MenuItem    device_item;
		private  MenuItem    settings_item;
		public   DeviceWifi  device;
		private  SimpleActionGroup actions;
		private  NM.Client   client;
		private  string      action_prefix;
		private  bool        show_settings;

		public WifiMenu (NM.Client client, DeviceWifi device, Menu global_menu, SimpleActionGroup actions, string action_prefix, bool show_settings)
		{
			this.apsmenu = global_menu;
			this.actions = actions;
			this.device = device;
			this.client = client;
			this.action_prefix = action_prefix;
			this.show_settings = show_settings;

			device_item = create_item_for_wifi_device ();
			this.apsmenu.append_item(device_item);

			if (show_settings) {
				settings_item = new MenuItem(_("Wi-Fi settings…"), "indicator.global.settings::wifi");
				this.apsmenu.append_item(settings_item);
			}

			device.access_point_added.connect   (access_point_added_cb);
			device.access_point_removed.connect (access_point_removed_cb);
			device.notify.connect               (active_access_point_changed);

			var aps = device.get_access_points ();

			if (aps == null)
				return;

			for (uint i = 0; i<aps.length; i++)
			{
				insert_ap (aps.get (i));
			}
		}

		~WifiMenu ()
		{
			device.access_point_added.disconnect   (access_point_added_cb);
			device.access_point_removed.disconnect (access_point_removed_cb);
			device.notify.disconnect               (active_access_point_changed);
		}


		private void bind_ap_item (AccessPoint ap, MenuItem item)
		{
			var strength_action_id = action_prefix + ap.get_path () + "::strength";
			var activate_action_id = action_prefix + ap.get_path ();

			item.set_label     (Utils.ssid_to_utf8 (ap.get_ssid ()));
			item.set_attribute ("x-canonical-type",                    "s", "unity.widgets.systemsettings.tablet.accesspoint");
			item.set_attribute ("x-canonical-wifi-ap-is-adhoc",        "b",  ap.get_mode ()  == NM.80211Mode.ADHOC);
			item.set_attribute ("x-canonical-wifi-ap-is-secure",       "b",  ap.get_flags () == NM.80211ApFlags.PRIVACY);
			item.set_attribute ("x-canonical-wifi-ap-bssid",           "s",  ap.get_bssid ());
			item.set_attribute ("x-canonical-wifi-ap-dbus-path",       "s",  ap.get_path ());

			item.set_attribute ("x-canonical-wifi-ap-strength-action", "s",  strength_action_id);
			item.set_attribute ("action", "s",  activate_action_id);
		}

		private void access_point_added_cb (NM.DeviceWifi device, GLib.Object ap)
		{
			insert_ap ((AccessPoint)ap);
		}

		private void access_point_removed_cb (NM.DeviceWifi device, GLib.Object ap)
		{
			remove_ap ((AccessPoint)ap);
		}

		private void active_access_point_changed (GLib.Object obj, ParamSpec pspec)
		{
			if (pspec.get_name () != "active-access-point")
				return;

			if (apsmenu == null)
				return;

			set_active_ap (device.active_access_point);
		}

		private MenuItem create_item_for_wifi_device ()
		{
			var device_item = new MenuItem (_("Wi-Fi"), action_prefix + "device-enabled");
			device_item.set_attribute ("x-canonical-type"  ,           "s", "com.canonical.indicator.switch");
			device_item.set_attribute ("x-canonical-busy-action",      "s", action_prefix + "device-busy");

			return device_item;
		}

		/*
		 * AccessPoints are inserted with the follow priority policy:
		 * - The active access point of the device always goes first
		 * - Previously used APs go first
		 * - Previously used APs are ordered by signal strength
		 * - Unused APs are ordered by signal strength
		 */
		private void insert_ap (AccessPoint ap)
		{
			var rs = new NM.RemoteSettings (null);
			SList <NM.Connection>? dev_conns = null;
			bool has_connection = false;

			if (ap == null)
				return;

			//If it is the active access point it always goes first
			if (ap == device.active_access_point)
			{
				var item = new MenuItem (null, null);
				bind_ap_item (ap, item);
				apsmenu.insert_item (1, item);
				//TODO: Remove duplicates???
				return;
			}

			var conns = rs.list_connections ();
			if (conns.length () > 0)
			{
				dev_conns = device.filter_connections (conns);
				if (dev_conns.length () > 0)
					has_connection = ap_has_connections (ap, dev_conns);
			}


			//Remove duplicate SSID
			for (int i = 0; i < apsmenu.get_n_items(); i++)
			{
				string path;

				if (!apsmenu.get_item_attribute (i, "x-canonical-wifi-ap-dbus-path", "s", out path))
					continue;

				var i_ap = device.get_access_point_by_path (path);
				if (i_ap == null)
					continue;

				//If both have the same SSID and security flags they are a duplicate
				if (Utils.same_ssid (i_ap.get_ssid (), ap.get_ssid (), false) && i_ap.get_flags () == ap.get_flags ())
				{

					//The one AP with the srongest signal wins
					if (i_ap.get_strength () >= ap.get_strength ())
						return;

					remove_item (i, i_ap);
					continue;
				}
			}

			//Find the right spot for the AP
			var item = new MenuItem (null, null);
			bind_ap_item (ap, item);
			for (int i = 0; i < apsmenu.get_n_items(); i++)
			{
				string path;

				if (!apsmenu.get_item_attribute (i, "x-canonical-wifi-ap-dbus-path", "s", out path))
					continue;
				var i_ap = device.get_access_point_by_path (path);

				if (i_ap == null)
					continue;

				//APs that have been used previously have priority
				if (ap_has_connections(i_ap, dev_conns))
				{
					if (!has_connection)
						continue;
				}
				//APs with higher strenght have priority
				if (ap.get_strength () > i_ap.get_strength ())
				{
					apsmenu.insert_item (i, item);
					return;
				}
			}

			//AP is last in the menu (avoid the settings item)
			if (show_settings) {
				apsmenu.insert_item (apsmenu.get_n_items() - 1, item);
			} else {
				apsmenu.append_item (item);
			}
		}

		private void set_active_ap (AccessPoint? ap)
		{
			//TODO: Set the previously active AP in the right order
			if (ap == null)
				return;

			for (int i = 1; i < apsmenu.get_n_items(); i++)
			{
				string path;
				if (!apsmenu.get_item_attribute (i, "action", "s", out path))
					continue;
				if (path != ap.get_path ())
					continue;
				remove_item (i, ap);
				var item = new MenuItem (null, null);
				bind_ap_item (ap, item);
				apsmenu.append_item (item);
			}
		}

		private static bool ap_has_connections (AccessPoint ap, SList<NM.Connection>? dev_conns)
		{
			if (dev_conns.length () < 1)
				return false;

			var ap_conns = ap.filter_connections (dev_conns);
			return ap_conns.length () > 0;
		}

		private void remove_ap (AccessPoint ap)
		{
			for (int i = 1; i < apsmenu.get_n_items(); i++)
			{
				string path;

				if (!apsmenu.get_item_attribute (i, "x-canonical-wifi-ap-dbus-path", "s", out path))
					continue;

				if (path == ap.get_path ())
				{
					remove_item (i, ap);
					return;
				}
			}
		}

		private void remove_item (int index, AccessPoint ap)
		{
			string strength_action_id;
			string activate_action_id;

			if (apsmenu.get_item_attribute (index, "x-canonical-wifi-ap-strength-action", "s", out strength_action_id))
				actions.remove (strength_action_id.substring((long)(action_prefix).size(), -1));
			if (apsmenu.get_item_attribute (index, "action", "s", out activate_action_id))
				actions.remove (activate_action_id.substring((long)(action_prefix).size(), -1));

			apsmenu.remove (index);
			//TODO: Check if removed dups need to be added
		}
	}

	internal class WifiActionManager
	{
		private SimpleActionGroup   actions;
		private NM.Client         client;
		public NM.RemoteSettings rs  = null;

		public  NM.DeviceWifi     wifidev = null;

		public WifiActionManager (SimpleActionGroup actions, NM.Client client, NM.DeviceWifi dev)
		{
			this.client  = client;
			this.actions = actions;
			this.wifidev = dev;

			rs = new NM.RemoteSettings (wifidev.get_connection ());
			rs.connections_read.connect (bootstrap_actions);
		}


		private  void bootstrap_actions ()
		{
			/* This object should be disposed by ActionManager on device removal
			 * but we still disconnect signals if that signal is emmited before
			 * this object was disposed
			 */
			client.device_removed.connect (remove_device);

			wifidev.access_point_added.connect   (access_point_added_cb);
			wifidev.access_point_removed.connect (access_point_removed_cb);
			wifidev.notify.connect               (active_access_point_changed);
			wifidev.state_changed.connect        (device_state_changed_cb);


			var aps = wifidev.get_access_points ();
			if (aps == null)
				return;

			for (int i = 0; i < aps.length; i++)
				insert_ap (aps.get(i));
		}

		~WifiActionManager ()
		{
			remove_device (client, wifidev);

			client.device_removed.disconnect (remove_device);
			rs.connections_read.disconnect          (bootstrap_actions);
		}

		private void remove_device (NM.Client client, NM.Device device)
		{
			wifidev.access_point_added.disconnect (access_point_added_cb);
			wifidev.access_point_removed.disconnect (access_point_removed_cb);
			wifidev.notify.disconnect (active_access_point_changed);
			wifidev.state_changed.disconnect (device_state_changed_cb);
		}

		private void device_state_changed_cb (NM.Device  device,
		                                      uint       new_state,
		                                      uint       old_state,
		                                      uint       reason)
		{
			if (new_state != DeviceState.DISCONNECTED)
				return;
			var wifidev = (NM.DeviceWifi)device;

			var aps = wifidev.get_access_points ();
			if (aps == null)
				return;

			for (uint i = 0; i < aps.length; i++)
			{
				var ap = aps.get(i);
				var action = actions.lookup(ap.get_path());
				if (action != null) {
					action.change_state(new Variant.boolean (false));
				}
			}
		}

		private void access_point_added_cb (NM.DeviceWifi device, GLib.Object ap)
		{
			insert_ap ((AccessPoint)ap);
		}

		private void access_point_removed_cb (NM.DeviceWifi device, GLib.Object ap)
		{
			remove_ap ((AccessPoint)ap);
		}

		private void active_access_point_changed (GLib.Object obj, ParamSpec pspec)
		{
			string? active_ap = null;
			if (pspec.get_name () != "active-access-point")
				return;

			var aps = wifidev.get_access_points ();
			if (aps == null)
				return;

			if (wifidev.active_access_point != null)
				active_ap = wifidev.active_access_point.get_path ();

			for (uint i = 0; i < aps.length; i++)
			{
				var ap = aps.get(i);
				if (ap == null)
					continue;

				if (actions.lookup (ap.get_path ()) == null)
				{
					insert_ap (ap);
					continue;
				}

				if (ap.get_path () == active_ap)
				{
					var action = actions.lookup(ap.get_path());
					if (action != null) {
						action.change_state(new Variant.boolean (true));
					}
				}
				else
				{
					var action = actions.lookup(ap.get_path());
					if (action != null) {
						action.change_state(new Variant.boolean (false));
					}
				}
			}
		}

		private void insert_ap (NM.AccessPoint ap)
		{
			if (ap == null)
				return;

			bool is_active = false;

			/* If the ap is already included we skip this callback */
			if (actions.lookup (ap.get_path ()) != null)
				return;

			GLib.debug("Adding new access point: " + ap.get_bssid() + " at " + ap.get_path());

			//TODO: Add actions for each AP NM connection
			var strength_action_id = ap.get_path () + "::strength";
			ap.notify.connect (ap_strength_changed);
			if (wifidev.active_access_point != null)
				is_active = ap.get_path () == wifidev.active_access_point.get_path ();

			var strength = new SimpleAction.stateful (strength_action_id,
			                                          null,
			                                          new Variant.byte (ap.get_strength ()));
			var activate = new SimpleAction.stateful (ap.get_path (),
			                                          null,
			                                          new Variant.boolean (is_active));
			activate.activate.connect (ap_activated);

			actions.insert (strength);
			actions.insert (activate);
		}

		private void remove_ap (NM.AccessPoint ap)
		{
			GLib.debug("Removing access point: " + ap.get_bssid() + " at " + ap.get_path());

			//TODO: Check if AP has connection action
			actions.remove (ap.get_path ());
			actions.remove (ap.get_path () + "::strength");
		}

		private void ap_activated (SimpleAction ac, Variant? val)
		{
			if (ac.state.get_boolean () == true &&
				wifidev.active_access_point != null &&
				wifidev.active_access_point.get_path () == ac.name)
			{
				wifidev.disconnect (null);
				return;
			}

			if (wifidev.active_access_point != null &&
				wifidev.active_access_point.get_path () == ac.name)
				return;

			var ap = wifidev.get_access_point_by_path (ac.name);
			if (ap == null) {
				warning("Unable to access access point path: " + ac.name);
				return;
			}

			var conns = rs.list_connections ();
			if (conns == null)
			{
				add_and_activate_ap (ac.name);
				return;
			}

			var dev_conns = wifidev.filter_connections (conns);
			if (dev_conns == null)
			{
				add_and_activate_ap (ac.name);
				return;
			}

			var ap_conns = ap.filter_connections (dev_conns);
			if (ap_conns == null)
			{
				add_and_activate_ap (ac.name);
				return;
			}

			client.activate_connection (ap_conns.data, wifidev, ac.name, null);
		}

		private void add_and_activate_ap (string ap_path)
		{
			client.add_and_activate_connection (null, wifidev, ap_path, null);
		}

		private void ap_strength_changed (GLib.Object obj, ParamSpec pspec)
		{
			var prop = pspec.get_name ();
			if (prop == "strength")
			{
				AccessPoint ap = (AccessPoint)obj;
				var action_name = ap.get_path() + "::strength";
				var action = actions.lookup(action_name);
				if (action != null)
					action.change_state (new Variant.byte(ap.get_strength ()));
			}
		}
	}

	public class Wifi : Base {
		private WifiMenu wifimenu;
		private WifiActionManager wifiactionmanager;

		public Wifi (NM.Client client, NM.DeviceWifi device, GLibLocal.ActionMuxer muxer, bool show_settings) {
			GLib.Object(
				client: client,
				device: device,
				namespace: device.get_iface(),
				muxer: muxer
			);

			wifimenu = new WifiMenu(client, device, this._menu, actions, "indicator." + this.namespace + ".", show_settings);
			wifiactionmanager = new WifiActionManager(actions, client, device);
		}

		protected override void disable_device ()
		{
			device.disconnect(null);
			client.wireless_set_enabled(false);
		}

		protected override void enable_device ()
		{
			client.wireless_set_enabled(true);
			device.set_autoconnect(true);
		}
	}
}
