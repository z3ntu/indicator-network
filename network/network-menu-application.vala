// vim: tabstop=4 noexpandtab shiftwidth=4 softtabstop=4
using NM;

namespace Unity.Settings
{
	public class NetworkMenu : Application
	{
		private Menu gmenu;
		private SimpleActionGroup ag;
		private RemoteSettings rs;
		private NM.Client client;

		public NetworkMenu ()
		{
			GLib.Object (application_id: "com.ubuntu.networksettings");
			//flags = ApplicationFlags.IS_SERVICE;

			gmenu = new Menu ();
			ag    = new SimpleActionGroup ();
			rs    = new NM.RemoteSettings (null);
			client = new NM.Client();
			bootstrap_menu ();


			try
			{
				var conn = Bus.get_sync (BusType.SESSION, null);
				conn.export_menu_model ("/com/ubuntu/networksettings", gmenu);
				conn.export_action_group ("/com/ubuntu/networksettings/actions", ag);
			} catch (GLib.Error e) {
				return;
			} catch (GLib.IOError e) {
				return;
			}
		}

		private void bind_ap_item (AccessPoint ap, MenuItem item)
		{
			var strength_action_id = ap.get_path () + "::strength";

			item.set_label     (Utils.ssid_to_utf8 (ap.get_ssid ()));
			item.set_attribute ("type",                      "s", "x-system-settings");
			item.set_attribute ("x-tablet-widget",           "s", "unity.widgets.systemsettings.tablet.accesspoint");
			item.set_attribute ("x-wifi-ap-is-adhoc",        "b",  ap.get_mode ()  == NM.80211Mode.ADHOC);
			item.set_attribute ("x-wifi-ap-is-secure",       "b",  ap.get_flags () == NM.80211ApFlags.PRIVACY);
			item.set_attribute ("x-wifi-ap-bssid",           "s",  ap.get_bssid ());
			item.set_attribute ("x-wifi-ap-strength-action", "s",  strength_action_id);
			item.set_attribute ("x-wifi-ap-dbus-path",       "s",  ap.get_path ());

			var strength = new SimpleAction.stateful (strength_action_id, new VariantType((string)VariantType.BYTE), ap.get_strength ());
			ag.insert (strength);

			ap.notify.connect (strength_changed);
			//TODO: Active or inactive property
		}

		private void strength_changed (GLib.Object obj, ParamSpec pspec)
		{
			var prop = pspec.get_name ();
			if (prop == "strength")
			{
				AccessPoint _ap = (AccessPoint)obj;
				var action = ag.lookup (_ap.get_path() + "::strength");
				if (action != null)
				{
					((SimpleAction)action).set_state (new Variant.byte(_ap.get_strength ()));
				}
			}
		}

		private void bootstrap_menu ()
		{
			//TODO: Add WiFi toggle if at least one item is found
			var devices = client.get_devices ();
			for (uint i = 0; i < devices.length; i++)
			{
				var device = devices.get(i);
				if (device.get_state () == NM.DeviceState.UNMANAGED)
					continue;
				switch (device.get_device_type ())
				{
					case NM.DeviceType.WIFI:
						add_wifi_device (device);
						break;
				}
			}
		}

		private void add_wifi_device (NM.Device device)
		{
			var wifidevice = (NM.DeviceWifi)device;
			var apsmenu = new Menu ();

			gmenu.append_section ("Select wireless network", apsmenu);

			var aps = wifidevice.get_access_points ();
			for (uint i = 0; i<aps.length; i++)
			{
				insert_ap (wifidevice, aps.get (i), apsmenu);
			}

			wifidevice.access_point_added.connect ((device, ap) => { insert_ap (device, (AccessPoint)ap, apsmenu); });
			wifidevice.access_point_removed.connect ((device, ap) => { remove_ap (device, (AccessPoint)ap, apsmenu); });
			//TODO: Use a delegate to disconnect once wifidevice is removed
			wifidevice.notify.connect ((obj, pspec) =>
									   {
											if (pspec.get_name () == "active-access-point")
											{
												var wifi = (NM.DeviceWifi)obj;
												set_active_ap (wifidevice, wifi.active_access_point, apsmenu);
											}
									   });
			//TODO: subscribe to the active-access-point prop notify wifidevice.notify.connect();
			//TODO: subscribe to the device add/remove
			//TODO: subscribe to the device state change
		}

		/*
		 * AccessPoints are inserted with the follow priority policy:
		 * - The active access point of the device always goes first
		 * - Previously used APs go first
		 * - Previously used APs are ordered by signal strength
		 * - Unused APs are ordered by signal strenght
		 */
		private void insert_ap (DeviceWifi dev, AccessPoint ap, Menu m)
		{
				var item = new MenuItem (null, null);
				bind_ap_item (ap, item);

				//If it is the active access point it always goes first
				if (ap == dev.active_access_point)
				{
					m.prepend_item (item);
					//TODO: Remove duplicates anyhow?
					return;
				}

				var conns = rs.list_connections ();
				var dev_conns = dev.filter_connections (conns);
				var has_connection = ap_has_connections (ap, dev_conns);

				//Remove duplicate SSID
				for (int i = 1; i < m.get_n_items(); i++)
				{
					string path;

					if (!m.get_item_attribute (i, "x-wifi-ap-dbus-path", "s", out path))
						continue;
					var i_ap = dev.get_access_point_by_path (path);

					//If both have the same SSID and security flags they are a duplicate
					if (Utils.same_ssid (i_ap.get_ssid (), ap.get_ssid (), false) && i_ap.get_flags () == ap.get_flags ())
					{
						//The one AP with the srongest signal wins
						if (i_ap.get_strength () >= ap.get_strength ())
							return;

						remove_item (m, i, i_ap);
						break;
					}
				}

				//Find the right spot for the AP
				for (int i = 1; i < m.get_n_items(); i++)
				{
					string path;

					if (!m.get_item_attribute (i, "x-wifi-ap-dbus-path", "s", out path))
						continue;
					var i_ap = dev.get_access_point_by_path (path);

					//APs that have been used previously have priority
					if (ap_has_connections(i_ap, dev_conns))
					{
						if (!has_connection)
							continue;
					}

					if (ap.get_strength () > i_ap.get_strength ())
					{
						m.insert_item (i, item);
						break;
					}
				}
				m.append_item (item);
		}

		private void set_active_ap (DeviceWifi dev, AccessPoint ap, Menu m)
		{
			for (int i = 1; i < m.get_n_items(); i++)
			{
				string path;
				if (!m.get_item_attribute (i, "x-wifi-ap-dbus-path", "s", out path))
					continue;
				if (path != ap.get_path ())
					continue;
				remove_item (m, i, ap);
				var item = new MenuItem (null, null);
				bind_ap_item (ap, item);
				m.append_item (item);
			}
		}

		private static bool ap_has_connections (AccessPoint ap, SList<NM.Connection> dev_conns)
		{
				var ap_conns = ap.filter_connections (dev_conns);
				return ap_conns.length() > 0;
		}

		private void remove_ap (DeviceWifi dev, AccessPoint ap, Menu m)
		{
			for (int i = 1; i < m.get_n_items(); i++)
			{
				string path;

				if (!m.get_item_attribute (i, "x-wifi-ap-dbus-path", "s", out path))
					continue;

				if (path == ap.get_path ())
				{
					remove_item (m, i, ap);
					return;
				}
			}
		}

		private void remove_item (Menu m, int index, AccessPoint ap)
		{
			m.remove (index);
			ap.notify.disconnect (strength_changed);
			//TODO: Check if removed dups need to be added
		}

		public static int main (string[] args)
		{
			var menu = new NetworkMenu ();
			menu.hold ();
			return menu.run (args);
		}
	}
}
