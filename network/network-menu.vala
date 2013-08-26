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
 *      Alberto Ruiz <alberto.ruiz@canonical.com>
 */

using NM;

namespace Network
{
	private const string APPLICATION_ID  = "com.canonical.indicator.network";
	private const string PHONE_MENU_PATH = "/com/canonical/indicator/network/phone";
	private const string PHONE_WIFI_SETTINGS_MENU_PATH = "/com/canonical/indicator/network/phone_wifi_settings";
	private const string DESKTOP_MENU_PATH = "/com/canonical/indicator/network/desktop";
	private const string ACTION_GROUP_PATH = "/com/canonical/indicator/network";

	internal class ProfileMenu {
		public Menu root_menu = new Menu();
		public MenuItem root_item;
		public Menu shown_menu = new Menu();

		private MenuModel? pre_settings = null;
		private MenuModel? post_settings = null;

		private GLib.DBusConnection conn;
		private uint export_id;

		public ProfileMenu (GLib.DBusConnection conn, string path) throws GLib.Error {
			root_item = new MenuItem.submenu (null, shown_menu as MenuModel);
			root_item.set_attribute (GLib.Menu.ATTRIBUTE_ACTION, "s", "indicator.global.network-status");
			root_item.set_attribute ("x-canonical-type", "s", "com.canonical.indicator.root");
			root_menu.append_item (root_item);

			export_id = conn.export_menu_model (path, root_menu);
		}

		~ProfileMenu () {
			conn.unexport_menu_model(export_id);
		}

		public void remove_device (string path) {
			for (int i = 0; i < (shown_menu as MenuModel).get_n_items(); i++) {
				var dev = (shown_menu as MenuModel).get_item_link(i, Menu.LINK_SECTION) as Device.Base;

				if (dev == null) {
					continue;
				}

				if (dev.device.get_path() == path) {
					shown_menu.remove(i);
					break;
				}
			}
		}

		public Device.Base? find_device (string path) {
			Device.Base? founddev = null;

			for (int i = 0; i < (shown_menu as MenuModel).get_n_items(); i++) {
				var dev = (shown_menu as MenuModel).get_item_link(i, Menu.LINK_SECTION) as Device.Base;

				if (dev == null) {
					continue;
				}

				if (dev.device.get_path() == path) {
					founddev = dev;
					break;
				}
			}

			return founddev;
		}

		/* Return a sorting value based on the type of networking that
		   is in the current menu */
		private uint dev2sort (Device.Base dev) {
			if ((dev as Device.Ethernet) != null) {
				return 0;
			}
			if ((dev as Device.Wifi) != null) {
				return 1;
			}
			if ((dev as Device.Mobile) != null) {
				return 2;
			}

			return 3;
		}

		/* Figures out where the device needs to go and inserts it in
		   the proper location */
		public void append_device (Device.Base device) {
			/* Handle the empty case right away, no reason to mess up code later on */
			if (shown_menu.get_n_items() == 0) {
				shown_menu.append_section(null, device);
				return;
			}

			int i;
			uint insort = dev2sort(device);
			
			for (i = (pre_settings == null ? 0 : 1);
					i < shown_menu.get_n_items(); i++) {
				var imenu = shown_menu.get_item_link(i, Menu.LINK_SECTION) as Device.Base;

				if (imenu == null) {
					continue;
				}

				var isort = dev2sort(imenu);

				if (isort > insort) {
					shown_menu.insert_section(i, null, device);
					break;
				}
			}

			if (i == shown_menu.get_n_items()) {
				if (post_settings == null) {
					shown_menu.append_section(null, device);
				} else {
					shown_menu.insert_section(shown_menu.get_n_items() - 1, null, device);
				}
			}
		}

		/* A settings section to put before all the devices */
		public void set_pre_settings (MenuModel settings) {
			if (pre_settings != null) {
				warning("Already have a pre-setting menus, can't have two!");
				return;
			}

			pre_settings = settings;
			shown_menu.insert_section(0, null, settings);
		}

		/* A settings section to put after all the devices */
		public void set_post_settings (MenuModel settings) {
			if (post_settings != null) {
				warning("Already have a post-setting menus, can't have two!");
				return;
			}

			post_settings = settings;
			shown_menu.append_section(null, settings);
		}
	}

	public class NetworkMenu : GLib.Object
	{
		private ProfileMenu     desktop;
		private ProfileMenu     phone;
		private ProfileMenu     phone_wifi_settings;

		private NM.Client       client;
		private ActionManager   am;
		private GLibLocal.ActionMuxer muxer = new GLibLocal.ActionMuxer();

		public NetworkMenu ()
		{
			client = new NM.Client();
			am = new ActionManager (muxer, client);

			client.device_added.connect   ((client, device) => { add_device (device); });
			client.device_removed.connect ((client, device) => { remove_device (device); });

			try
			{
				var conn = Bus.get_sync (BusType.SESSION, null);

				conn.export_action_group (ACTION_GROUP_PATH, muxer as ActionGroup);

				desktop = new ProfileMenu(conn, DESKTOP_MENU_PATH);
				phone = new ProfileMenu(conn, PHONE_MENU_PATH);
				phone_wifi_settings = new ProfileMenu(conn, PHONE_WIFI_SETTINGS_MENU_PATH);

				Bus.own_name_on_connection(conn, APPLICATION_ID, BusNameOwnerFlags.NONE, null, ((conn, name) => { error("Unable to get D-Bus bus name"); }));
			}
			catch (GLib.IOError e)
			{
				return;
			}
			catch (GLib.Error e)
			{
				return;
			}

			/* Put an airplane mode setting at the top of the desktop and phone
			   menus */
			var airplane = new Network.Settings.Airplane(muxer);
			desktop.set_pre_settings(airplane);
			phone.set_pre_settings(airplane);

			/* Add some items at the end of the settings menu */
			var wifisettings = new Network.Settings.Wifi(muxer);
			phone_wifi_settings.set_post_settings(wifisettings);

			var devices = client.get_devices ();

			if (devices == null)
				return;
			for (uint i = 0; i < devices.length; i++)
			{
				add_device (devices.get (i));
			}
		}

		private void add_device (NM.Device device)
		{
			Device.Base? founddev = null;
			
			founddev = desktop.find_device(device.get_path());
			if (founddev != null) return;

			founddev = phone.find_device(device.get_path());
			if (founddev != null) return;

			debug("Adding device: " + device.get_iface());
			switch (device.get_device_type ())
			{
				case NM.DeviceType.WIFI:
					var wifidev = new Device.Wifi(this.client, device as NM.DeviceWifi, this.muxer);
					desktop.append_device(wifidev);
					phone.append_device(wifidev);

					var wifisettingsdev = new Device.Wifi(this.client, device as NM.DeviceWifi, this.muxer);
					phone_wifi_settings.append_device(wifisettingsdev);

					break;
				case NM.DeviceType.MODEM:
					var mobiledesktopdev = new Device.Mobile(this.client, device as NM.DeviceModem, this.muxer, true);
					desktop.append_device(mobiledesktopdev);
					var mobilephonedev = new Device.Mobile(this.client, device as NM.DeviceModem, this.muxer, false);
					phone.append_device(mobilephonedev);
					break;
				case NM.DeviceType.ETHERNET:
					var ethdev = new Device.Ethernet(this.client, device as NM.DeviceEthernet, this.muxer);
					desktop.append_device(ethdev);
					phone.append_device(ethdev);
					break;
				default:
					warning("Unsupported device type: " + device.get_iface());
					break;
			}
		}

		private void remove_device (NM.Device device)
		{
			debug("Removing device: " + device.get_iface());
			desktop.remove_device(device.get_path());
			phone.remove_device(device.get_path());
		}
	}
}
