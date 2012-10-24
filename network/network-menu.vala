// vim: tabstop=4 noexpandtab shiftwidth=4 softtabstop=4
using NM;

namespace Unity.Settings
{
	public class WifiMenu : GLib.Object
	{
		private Menu              gmenu;
		private Menu              apsmenu;
		private MenuItem          device_item;
		internal DeviceWifi       device;
		private SimpleActionGroup ag;
		private RemoteSettings    rs;
		private NM.Client         client;

		public WifiMenu (NM.Client client, DeviceWifi device, Menu global_menu, SimpleActionGroup ag)
		{
			gmenu = global_menu;
			this.ag = ag;
			this.device = device;
			this.client = client;

			apsmenu = new Menu ();
			device_item = create_item_for_wifi_device ();

			device_item.set_section (apsmenu);
			gmenu.append_item (device_item);

			var aps = device.get_access_points ();
			for (uint i = 0; i<aps.length; i++)
			{
				insert_ap (aps.get (i));
			}

			device.access_point_added.connect (access_point_added_cb);
			device.access_point_removed.connect (access_point_removed_cb);
			device.notify.connect (active_access_point_changed);
		}

		~WifiMenu ()
		{
			device.access_point_added.disconnect (access_point_added_cb);
			device.access_point_removed.disconnect (access_point_removed_cb);
			device.notify.disconnect (active_access_point_changed);
		}

		/*private void wifi_device_state_changed (NM.Device  dev,
		                                        uint       new_state,
		                                        uint       old_state,
		                                        uint       reason)
		{
		}*/

		private void add_and_activate_ap (string ap_path)
		{
			client.add_and_activate_connection (null, device, ap_path, null);
		}

		private void ap_activated (SimpleAction ac, Variant? val)
		{
			var rs = new NM.RemoteSettings (null);
			if (ac.get_name () == device.get_active_access_point ().get_path ())
				return;

			var ap = new NM.AccessPoint (device.get_connection (), ac.name);
			var conns = rs.list_connections ();
			if (conns == null)
			{
				add_and_activate_ap (ac.name);
				return;
			}

			var dev_conns = device.filter_connections (conns);
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

			client.activate_connection (ap_conns.data, device, ac.name, null);
		}

		private void bind_ap_item (AccessPoint ap, MenuItem item)
		{
			var strength_action_id = ap.get_path () + "::strength";
			var activate_action_id = ap.get_path ();

			item.set_label     (Utils.ssid_to_utf8 (ap.get_ssid ()));
			item.set_attribute ("type",                                "s", "x-canonical-system-settings");
			item.set_attribute ("x-canonical-widget-type",             "s", "unity.widgets.systemsettings.tablet.accesspoint");
			item.set_attribute ("x-canonical-wifi-ap-is-adhoc",        "b",  ap.get_mode ()  == NM.80211Mode.ADHOC);
			item.set_attribute ("x-canonical-wifi-ap-is-secure",       "b",  ap.get_flags () == NM.80211ApFlags.PRIVACY);
			item.set_attribute ("x-canonical-wifi-ap-bssid",           "s",  ap.get_bssid ());
			item.set_attribute ("x-canonical-wifi-ap-dbus-path",       "s",  ap.get_path ());

			item.set_attribute ("x-canonical-wifi-ap-strength-action", "s",  strength_action_id);
			item.set_attribute ("action", "s",  activate_action_id);

			var strength = new SimpleAction.stateful (strength_action_id, new VariantType((string)VariantType.BYTE), ap.get_strength ());
			var activate = new SimpleAction.stateful (activate_action_id, new VariantType((string)VariantType.BOOLEAN), device.active_access_point == ap);
			activate.activate.connect (ap_activated);

			ag.insert (strength);
			ag.insert (activate);
			ap.notify.connect (strength_changed);
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
			if (apsmenu == null)
				return;

			if (pspec.get_name () == "active-access-point")
				set_active_ap (device.active_access_point);

			for (int i = 0; i < apsmenu.get_n_items (); i++)
			{
				string path;
				string action;

				if (!apsmenu.get_item_attribute (i, "x-canonical-wifi-ap-dbus-path", "s", out path))
					continue;
				if (!apsmenu.get_item_attribute (i, "action", "s", out action))
					continue;

				if (device.active_access_point.get_path () == path)
					ag.change_action_state (action, new Variant.boolean (true));
				else
					ag.change_action_state (action, new Variant.boolean (false));

				//TODO: Change the appropriate submenu
			}
		}

		private MenuItem create_item_for_wifi_device ()
		{
			var busy_action_id = device.get_path () + "::is-busy";
			var device_item = new MenuItem ("Select wireless network", null);
			device_item.set_attribute ("type",                         "s", "x-canonical-system-settings");
			device_item.set_attribute ("x-canonical-widget-type"  ,    "s", "unity.widget.systemsettings.tablet.sectiontitle");
			device_item.set_attribute ("x-canonical-children-display", "s", "inline");
			device_item.set_attribute ("x-canonical-wifi-device-path", "s",  device.get_path ());
			device_item.set_attribute ("x-canonical-busy-action",      "s",  busy_action_id);

			var strength = new SimpleAction.stateful (busy_action_id, new VariantType((string)VariantType.BOOLEAN), device_is_busy (device));
			ag.insert (strength);

			//TODO: Submenu for active and previously used APs

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
		private void insert_ap (AccessPoint ap)
		{
			var rs = new NM.RemoteSettings (null);
			SList <NM.Connection>? dev_conns = null;
			bool has_connection = false;

			//If it is the active access point it always goes first
			if (ap == device.active_access_point)
			{
				var item = new MenuItem (null, null);
				bind_ap_item (ap, item);
				apsmenu.prepend_item (item);
				//TODO: Remove duplicates???
				return;
			}

			var conns = rs.list_connections ();
			if (conns.length() > 0)
			{
				dev_conns = device.filter_connections (conns);
				if (dev_conns.length() > 0)
					has_connection = ap_has_connections (ap, dev_conns);
			}


			//Remove duplicate SSID
			for (int i = 1; i < apsmenu.get_n_items(); i++)
			{
				string path;

				if (!apsmenu.get_item_attribute (i, "x-canonical-wifi-ap-dbus-path", "s", out path))
					continue;
				var i_ap = device.get_access_point_by_path (path);

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
			for (int i = 1; i < apsmenu.get_n_items(); i++)
			{
				string path;

				if (!apsmenu.get_item_attribute (i, "x-canonical-wifi-ap-dbus-path", "s", out path))
					continue;
				var i_ap = device.get_access_point_by_path (path);

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
			//AP is last in the menu
			apsmenu.append_item (item);
		}

		private void set_active_ap (AccessPoint? ap)
		{
			//TODO: Set the previously active AP in the right order
			if (ap == null)
				return;

			for (int i = 1; i < apsmenu.get_n_items(); i++)
			{
				string path;
				if (!apsmenu.get_item_attribute (i, "x-canonical-wifi-ap-dbus-path", "s", out path))
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
			if (dev_conns.length() < 1)
				return false;

			var ap_conns = ap.filter_connections (dev_conns);
			return ap_conns.length() > 0;
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
				ag.remove (strength_action_id);
			if (apsmenu.get_item_attribute (index, "action", "s", out activate_action_id))
				ag.remove (activate_action_id);

			apsmenu.remove (index);
			ap.notify.disconnect (strength_changed);
			//TODO: Check if removed dups need to be added
		}
	}

	public class NetworkMenu : Application
	{
		private Menu gmenu;
		private SimpleActionGroup ag;
		private RemoteSettings rs;
		private NM.Client client;
		private List<WifiMenu> wifidevices;

		public NetworkMenu ()
		{
			GLib.Object (application_id: "com.ubuntu.networksettings");
			//flags = ApplicationFlags.IS_SERVICE;

			gmenu  = new Menu ();
			ag     = new SimpleActionGroup ();
			client = new NM.Client();
			bootstrap_menu ();
			client.device_added.connect   ((client, device) => { add_device (device); });
			client.device_removed.connect ((client, device) => { remove_device (device); });

			try
			{
				var conn = Bus.get_sync (BusType.SESSION, null);
				conn.export_menu_model ("/com/ubuntu/networksettings", gmenu);
				conn.export_action_group ("/com/ubuntu/networksettings/actions", ag);
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
			var devices = client.get_devices ();
			for (uint i = 0; i < devices.length; i++)
			{
				add_device (devices.get (i));
			}
		}

		private void add_device (NM.Device device)
		{
			switch (device.get_device_type ())
			{
				case NM.DeviceType.WIFI:
					add_wifi_device ((NM.DeviceWifi)device);
					break;
			}
		}

		private void remove_device (NM.Device device)
		{
			switch (device.get_device_type ())
			{
				case NM.DeviceType.WIFI:
					remove_wifi_device ((NM.DeviceWifi)device);
					break;
			}
		}

		private void add_wifi_device (NM.DeviceWifi device)
		{
			device.state_changed.connect (wifi_device_state_changed);
			var wifimenu = new WifiMenu (client, device, gmenu, ag);
			wifidevices.append (wifimenu);
		}

		private void remove_wifi_device (NM.DeviceWifi device)
		{
			for (int i = 0; i < gmenu.get_n_items (); i++)
			{
				string path;
				string busy_action_id;

				if (!gmenu.get_item_attribute (i, "x-canonical-wifi-device-path", "s", out path))
					continue;
				if (path != device.get_path ())
					continue;
				if (gmenu.get_item_attribute (i, "x-canonical-busy-action", "s", out  busy_action_id))
					ag.remove (busy_action_id);

				gmenu.remove (i);

				for (int j = 0; j < wifidevices.length (); j++)
				{
					if (wifidevices.nth_data(j).device == device)
					{
						var wifimenu = wifidevices.nth_data(j);
						wifidevices.remove (wifidevices.nth_data (j));
					}
				}

				return;
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
	}
}
