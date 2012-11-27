// vim: tabstop=4 noexpandtab shiftwidth=4 softtabstop=4
using NM;

namespace Unity.Settings.Network
{
	private const string APPLICATION_ID  = "com.canonical.settings.network";
	private const string PHONE_MENU_PATH = "/com/canonical/settings/network/phone";

	internal class WifiMenu
	{
		private  Menu        gmenu;
		private  Menu        apsmenu;
		private  MenuItem    device_item;
		public   DeviceWifi  device;
		private  Application app;
		private  NM.Client   client;

		public WifiMenu (NM.Client client, DeviceWifi device, Menu global_menu, Application app)
		{
			gmenu = global_menu;
			this.app = app;
			this.device = device;
			this.client = client;

			apsmenu = new Menu ();
			device_item = create_item_for_wifi_device ();

			device_item.set_section (apsmenu);
			gmenu.append_item (device_item);

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
			var busy_action_id = device.get_path () + "::is-busy";
			var device_item = new MenuItem ("Select wireless network", null);
			device_item.set_attribute ("type",                         "s", "x-canonical-system-settings");
			device_item.set_attribute ("x-canonical-widget-type"  ,    "s", "unity.widget.systemsettings.tablet.sectiontitle");
			device_item.set_attribute ("x-canonical-children-display", "s", "inline");
			device_item.set_attribute ("x-canonical-wifi-device-path", "s",  device.get_path ());
			device_item.set_attribute ("x-canonical-busy-action",      "s",  busy_action_id);


			//TODO: Submenu for active and previously used APs

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
				apsmenu.prepend_item (item);
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
			for (int i = 1; i < apsmenu.get_n_items(); i++)
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
			for (int i = 1; i < apsmenu.get_n_items(); i++)
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
				app.remove_action (strength_action_id);
			if (apsmenu.get_item_attribute (index, "action", "s", out activate_action_id))
				app.remove_action (activate_action_id);

			apsmenu.remove (index);
			//TODO: Check if removed dups need to be added
		}
	}

	public class NetworkMenu : Application
	{
		private Menu            gmenu;
		private NM.Client       client;
		private ActionManager   am;
		private List<WifiMenu*> device_menus;

		public NetworkMenu ()
		{
			GLib.Object (application_id: APPLICATION_ID);
			flags = ApplicationFlags.IS_SERVICE;

			gmenu  = new Menu ();
			client = new NM.Client();
			am     = new ActionManager (this, client);

			bootstrap_menu ();
			client.device_added.connect   ((client, device) => { add_device (device); });
			client.device_removed.connect ((client, device) => { remove_device (device); });

			try
			{
				var conn = Bus.get_sync (BusType.SESSION, null);
				conn.export_menu_model (PHONE_MENU_PATH, gmenu);
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

			device_menus.append (new WifiMenu (client, device, gmenu, this));
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
