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
		private List<DeviceAbstraction> device_menus;
		private GLibLocal.ActionMuxer muxer = new GLibLocal.ActionMuxer();

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

		private DeviceAbstraction? device2abstraction (NM.Device device)
		{
			switch (device.get_device_type ())
			{
				case NM.DeviceType.WIFI:
					return new DeviceAbstractionWifi(device as NM.DeviceWifi, this.muxer);
				default:
					warning("Unsupported device type");
					break;
			}

			return null;
		}

		private void add_device (NM.Device device)
		{
			DeviceAbstraction? founddev = null;

			foreach (var dev in device_menus) {
				if (dev.device.get_path() == device.get_path()) {
					founddev = dev;
					break;
				}
			}

			if (founddev == null) {
				founddev = device2abstraction(device);
				if (founddev != null) {
					device_menus.append(founddev);
				}

			}
		}

		private void remove_device (NM.Device device)
		{
			foreach (var dev in device_menus) {
				if (dev.device.get_path() == device.get_path()) {
					device_menus.remove(dev);
					break;
				}
			}
		}
	}
}
