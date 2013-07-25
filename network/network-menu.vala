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

namespace Unity.Settings.Network
{
	private const string APPLICATION_ID  = "com.canonical.indicator.network";
	private const string PHONE_MENU_PATH = "/com/canonical/indicator/network/phone";
	private const string DESKTOP_MENU_PATH = "/com/canonical/indicator/network/desktop";

	public class NetworkMenu : Application
	{
		private Menu            root_menu;
		private MenuItem        root_item;
		private Menu            gmenu;
		private NM.Client       client;
		private ActionManager   am;
		private List<DeviceAbstraction*> device_menus;

		public NetworkMenu ()
		{
			GLib.Object (application_id: APPLICATION_ID);
			flags = ApplicationFlags.IS_SERVICE;

			client    = new NM.Client();

			bootstrap_menu ();
			client.device_added.connect   ((client, device) => { add_device (device); });
			client.device_removed.connect ((client, device) => { remove_device (device); });

			try
			{
				var conn = Bus.get_sync (BusType.SESSION, null);
				conn.export_menu_model (PHONE_MENU_PATH, root_menu);
				conn.export_menu_model (DESKTOP_MENU_PATH, root_menu);
			}
			catch (GLib.IOError e)
			{
				return;
			}
			catch (GLib.Error e)
			{
				return;
			}
		}

		private void bootstrap_menu ()
		{
			root_menu = new Menu ();
			gmenu     = new Menu ();
			am        = new ActionManager (this, client);

			root_item = new MenuItem.submenu (null, gmenu as MenuModel);
			root_item.set_attribute (GLib.Menu.ATTRIBUTE_ACTION, "s", "indicator.network-status");
			root_item.set_attribute ("x-canonical-type", "s", "com.canonical.indicator.root");
			root_menu.append_item (root_item);

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
			device.state_changed.connect (device_state_changed);
			switch (device.get_device_type ())
			{
				case NM.DeviceType.WIFI:
					add_wifi_device ((NM.DeviceWifi)device);
					break;
			}
		}

		private void remove_device (NM.Device device)
		{
			device.state_changed.disconnect (device_state_changed);
			switch (device.get_device_type ())
			{
				case NM.DeviceType.WIFI:
					remove_wifi_device ((NM.DeviceWifi)device);
					break;
			}
		}

		private void add_wifi_device (NM.DeviceWifi device)
		{
			for (int i = 0; i < gmenu.get_n_items (); i++)
			{
				string path;

				if (!gmenu.get_item_attribute (i, "x-canonical-wifi-device-path", "s", out path))
					continue;
				if (path == device.get_path ())
					return;
			}

			// device_menus.append (new WifiMenu (client, device, gmenu, this));
		}

		private void remove_wifi_device (NM.DeviceWifi device)
		{
			//TODO: Move this code to WifiMenu
			if (device == null)
				return;

			for (int i = 0; i < gmenu.get_n_items (); i++)
			{
				string path;

				if (!gmenu.get_item_attribute (i, "x-canonical-wifi-device-path", "s", out path))
					continue;
				if (path != device.get_path ())
					continue;

				gmenu.remove (i);
				break;
			}

			for (uint i = 0; i < device_menus.length (); i++)
			{
				var wifimenu = device_menus.nth_data (i);
				if (wifimenu         != null &&
					wifimenu->device != null &&
					wifimenu->device.get_path () == device.get_path ())
				{
					device_menus.remove (wifimenu);
					delete wifimenu;
					break;
				}
			}
		}

		private void device_state_changed (NM.Device  device,
		                                   uint       new_state,
		                                   uint       old_state,
		                                   uint       reason)
		{
			var type = device.get_device_type ();

			switch (new_state)
			{
				case NM.DeviceState.UNAVAILABLE:
				case NM.DeviceState.UNKNOWN:
				case NM.DeviceState.UNMANAGED:
					if (type == NM.DeviceType.WIFI)
						remove_wifi_device ((NM.DeviceWifi)device);
					break;
				default:
					if (type == NM.DeviceType.WIFI)
						add_wifi_device ((NM.DeviceWifi)device);
					break;
			}
		}
	}
}
