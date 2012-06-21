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
		private uint32    nwifi;

		public NetworkMenu ()
		{
			GLib.Object (application_id: "com.ubuntu.networksettings");
			//flags = ApplicationFlags.IS_SERVICE;

			gmenu  = new Menu ();
			ag     = new SimpleActionGroup ();
			rs     = new NM.RemoteSettings (null);
			client = new NM.Client();
			nwifi  = 0;

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
			var devices = client.get_devices ();
			for (uint i = 0; i < devices.length; i++)
			{
				nwifi++;
				//TODO: Add WiFi toggle if at least one item is found
				add_device (devices.get (i));
			}
		}

		private void wifi_device_state_changed (NM.Device  dev,
		                                        uint       new_state,
		                                        uint       old_state,
		                                        uint       reason)
		{
			var wifidevice = (NM.DeviceWifi)dev;
			if (new_state == NM.DeviceState.UNMANAGED && reason == NM.DeviceStateReason.NOW_UNMANAGED)
				remove_wifi_device (wifidevice);

			if (new_state != NM.DeviceState.UNMANAGED && reason == NM.DeviceStateReason.NOW_MANAGED)
				add_wifi_device (wifidevice);
		}

		private void remove_wifi_device (NM.DeviceWifi wifidevice)
		{
			for (int i = 0; i < gmenu.get_n_items (); i++)
			{
				string path;
				string busy_action_id;

				if (!gmenu.get_item_attribute (i, "x-wifi-device-path", "s", out path))
					continue;
				if (path != wifidevice.get_path ())
					continue;
				if (gmenu.get_item_attribute (i, "x-busy-action", "s", out  busy_action_id))
					ag.remove (busy_action_id);

				gmenu.remove (i);

				wifidevice.access_point_added.disconnect (access_point_added_cb);
				wifidevice.access_point_removed.disconnect (access_point_removed_cb);
				wifidevice.notify.disconnect (active_access_point_changed);
				return;
			}
		}

		private void add_device (NM.Device device)
		{
			switch (device.get_device_type ())
			{
				case NM.DeviceType.WIFI:
					add_wifi_device (device);
					break;
			}
		}

		private void add_wifi_device (NM.Device device)
		{
			device.state_changed.connect (wifi_device_state_changed);
			if (device.get_state () == NM.DeviceState.UNMANAGED)
				return;

			var wifidevice = (NM.DeviceWifi)device;

			add_wifi_device (device);
			var apsmenu = new Menu ();

			var device_item = create_item_for_wifi_device (wifidevice);
			device_item.set_section (apsmenu);
			gmenu.append_item (device_item);

			var aps = wifidevice.get_access_points ();
			for (uint i = 0; i<aps.length; i++)
			{
				insert_ap (wifidevice, aps.get (i), apsmenu);
			}

			wifidevice.access_point_added.connect (access_point_added_cb);
			wifidevice.access_point_removed.connect (access_point_removed_cb);
			wifidevice.notify.connect (active_access_point_changed);
		}

		private void access_point_added_cb (NM.DeviceWifi device, GLib.Object ap)
		{
			var apsmenu = find_menu_index_for_device(device);
			insert_ap (device, (AccessPoint)ap, apsmenu);
		}

		private void access_point_removed_cb (NM.DeviceWifi device, GLib.Object ap)
		{
			var apsmenu = find_menu_index_for_device(device);
			remove_ap (device, (AccessPoint)ap, apsmenu);
		}

		private void active_access_point_changed (GLib.Object obj, ParamSpec pspec)
		{
			var wifidevice = (NM.DeviceWifi)obj;
			var apsmenu = find_menu_index_for_device(wifidevice);

			if (pspec.get_name () == "active-access-point")
				set_active_ap (wifidevice, wifidevice.active_access_point, apsmenu);
		}

		private Menu? find_menu_index_for_device (NM.Device device)
		{
			for (int i = 0; i < gmenu.get_n_items (); i++)
			{
				string path;
				string busy_action_id;

				if (!gmenu.get_item_attribute (i, "x-wifi-device-path", "s", out path))
					continue;
				if (path != device.get_path ())
					continue;

				return (Menu?)gmenu.get_item_link (i, Menu.LINK_SECTION);
			}
			return null;
		}

		private MenuItem create_item_for_wifi_device (NM.DeviceWifi device)
		{
			var busy_action_id = device.get_path () + "::is-busy";
			var device_item = new MenuItem ("Select wireless network", null);
			device_item.set_attribute ("type",   "x-system-settings");
			device_item.set_attribute ("x-tablet-widget", "unity.widget.systemsettings.tablet.sectiontitle");
			device_item.set_attribute ("x-children-display", "s", "inline");
			device_item.set_attribute ("x-wifi-device-path", "s",  device.get_path ());
			device_item.set_attribute ("x-busy-action", "s", busy_action_id);

			var strength = new SimpleAction.stateful (busy_action_id, new VariantType((string)VariantType.BOOLEAN), device_is_busy (device));
			ag.insert (strength);

			return device_item;
		}

		private bool device_is_busy (NM.Device device)
		{
			switch (device.get_state ())
			{
			    case NM.DeviceState.UNKNOWN:
			    case NM.DeviceState.DISCONNECTED:
				case NM.DeviceState.UNMANAGED:
				case NM.DeviceState.ACTIVATED:
					return false;
				default:
					return true;
			}
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
				//If it is the active access point it always goes first
				if (ap == dev.active_access_point)
				{
					var item = new MenuItem (null, null);
					bind_ap_item (ap, item);
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
				var item = new MenuItem (null, null);
				bind_ap_item (ap, item);
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
					//APs with higher strenght have priority
					if (ap.get_strength () > i_ap.get_strength ())
					{
						m.insert_item (i, item);
						return;
					}
				}
				//AP is last in the menu
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
			string strength_action_id;
			if (m.get_item_attribute (index, "x-wifi-ap-strength-action", "s", out strength_action_id))
				ag.remove (strength_action_id);
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
