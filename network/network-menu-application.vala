// vim: tabstop=4 noexpandtab shiftwidth=4 softtabstop=4
using NM;

namespace Unity.Settings
{
	public class NetworkMenu : Application
	{
		private Menu gmenu;
		private SimpleActionGroup ag;
		private RemoteSettings rs;

		public NetworkMenu ()
		{
			GLib.Object (application_id: "com.ubuntu.networksettings");
			//flags = ApplicationFlags.IS_SERVICE;

			gmenu = new Menu ();
			ag    = new SimpleActionGroup ();
			rs    = new NM.RemoteSettings (null);

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

		private void bind_ap_item (MenuItem item, AccessPoint ap)
		{
			var strength_action_id = ap.get_path () + "::strength";

			item.set_label     (Utils.ssid_to_utf8 (ap.get_ssid()));
			item.set_attribute ("type",                      "s", "x-system-settings");
			item.set_attribute ("x-tablet-widget",           "s", "unity.widgets.systemsettings.tablet.accesspoint");
			item.set_attribute ("x-wifi-ap-is-adhoc",        "b",  ap.get_mode()  == NM.80211Mode.ADHOC);
			item.set_attribute ("x-wifi-ap-is-secure",       "b",  ap.get_flags() == NM.80211ApFlags.PRIVACY);
			item.set_attribute ("x-wifi-ap-bssid",           "s",  ap.get_bssid());
			item.set_attribute ("x-wifi-ap-strength-action", "s",  strength_action_id);
			item.set_attribute ("x-wifi-ap-dbus-path",       "s",  ap.get_path ());

			var strength = new SimpleAction.stateful (strength_action_id, new VariantType((string)VariantType.UINT16), ap.get_strength ());
			ag.insert (strength);

			ap.notify.connect ((obj, pspec) => {
									var prop = pspec.get_name();
									if (prop == "strength")
									{
										AccessPoint _ap = (AccessPoint)obj;
										var action = ag.lookup(_ap.get_path() + "::strength");
										if (action != null)
										{
											((SimpleAction)action).set_state (new Variant.uint16(_ap.get_strength ()));
										}
									}
							   });
		}

		private void bootstrap_menu ()
		{
			//TODO: Add WiFi toggle if at least one item is found
			var client = new NM.Client();
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
				var item = new MenuItem (null, null);
				bind_ap_item (item, aps.get(i));
				//TODO: Insert method with ordering algorithm
				apsmenu.append_item (item);
			}

			wifidevice.access_point_added.connect ((device, ap) => { return; });
			wifidevice.access_point_removed.connect ((device, ap) => { return; });
		}
		public static int main (string[] args)
		{
			var menu = new NetworkMenu ();
			menu.hold ();
			return menu.run (args);
		}
	}
}
