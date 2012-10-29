// vim: tabstop=4 noexpandtab shiftwidth=4 softtabstop=4
using NM;

namespace Unity.Settings.Network
{
	internal class WifiActionManager
	{
		private Application   app;
		private NM.Client     client;
		private NM.DeviceWifi wifidev;

		public WifiActionManager (Application app, NM.Client client, NM.DeviceWifi wifidev)
		{
			this.client  = client;
			this.app     = app;
			this.wifidev = wifidev;

			var aps = wifidev.get_access_points ();
			for (int i = 0; i < aps.length; i++)
				insert_ap (aps.get(i));
			wifidev.access_point_added.connect (access_point_added_cb);
			//wifidev.access_point_removed.connect (access_point_removed_cb);
			wifidev.notify.connect (active_access_point_changed);

			client.device_removed.connect ((client, device) => {remove_device (device);});
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
			string active_ap = wifidev.active_access_point.get_path ();

			var aps = wifidev.get_access_points ();
			for (uint i = 0; i < aps.length; i++)
			{
				var ap = aps.get(i);
				if (ap.get_path () == active_ap)
					app.change_action_state (ap.get_path(),
											 new Variant.boolean (true));
				else
					app.change_action_state (ap.get_path(),
											 new Variant.boolean (false));
			}
		}

		private void remove_device (NM.Device device)
		{
			wifidev.access_point_added.disconnect (access_point_added_cb);
			//wifidev.access_point_removed.disconnect (access_point_removed_cb);
			wifidev.notify.disconnect (active_access_point_changed);
		}

		private void insert_ap (NM.AccessPoint ap)
		{
			//TODO: Add actions for each AP NM connection
			var strength_action_id = ap.get_path () + "::strength";
			ap.notify.connect (ap_strength_changed);

			ActionEntry[] entries = {
				ActionEntry() {
					name           = strength_action_id,
					activate       = null,
					parameter_type = (string)VariantType.BYTE,
					state          = "%f".printf(ap.get_strength ()),
					//TODO: Ensure the client does not change the state
					change_state   = null
				},
				ActionEntry() {
					name           = ap.get_path (),
					activate       = ap_activated,
					parameter_type = (string)VariantType.BOOLEAN,
					state          = wifidev.active_access_point == ap ? "true" : "false",
					change_state   = ap_activated
				}
			};
			app.add_action_entries (entries, (void*)this);
		}

		private void remove_ap (NM.AccessPoint ap)
		{
			//TODO: Check if AP has connection action
			app.remove_action (ap.get_path ());
			app.remove_action (ap.get_path () + "::strength");
		}

		private void ap_activated (SimpleAction ac, Variant? val)
		{
			var rs = new NM.RemoteSettings (null);
			if (ac.get_name () == wifidev.get_active_access_point ().get_path ())
				return;

			var ap = new NM.AccessPoint (wifidev.get_connection (), ac.name);
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
				var action = ap.get_path() + "::strength";
				if (app.has_action (action))
					app.change_action_state (action, new Variant.byte(ap.get_strength ()));
			}
		}
	}

	public class ActionManager
	{
		private Application app;
		private NM.Client   client;

		public ActionManager (Application app, NM.Client client)
		{
			this.client = client;
			this.app    = app;

			bootstrap_actions ();
		}

		private void bootstrap_actions ()
		{
			//TODO: Airplane mode action
			//TODO: Wifi enabled/disabled action

			client.device_added.connect ((client, device) => {add_device (device);});
			//TODO: Counter for device classes
			//client.device_removed.connect (...)

			var devices = client.get_devices ();
			for (uint i = 0; i < devices.length ; i++)
			{
				var device = devices.get(i);
				var busy_action_id = device.get_path () + "::is-busy";
				var is_busy = device_is_busy (device);
				var action = new SimpleAction.stateful (busy_action_id,
				                                        VariantType.BOOLEAN,
				                                        new Variant.boolean (is_busy));

				app.add_action (action);
				add_device (device);
			}
		}

		private void add_device (NM.Device device)
		{
			device.state_changed.connect (device_state_changed);
			switch (device.get_device_type ())
			{
				case NM.DeviceType.WIFI:
					var wifidev = (NM.DeviceWifi)device;
					var wifimgr = new WifiActionManager (app, client, wifidev);
					break;
			}
		}

		private static bool device_is_busy (NM.Device device)
		{
			switch (device.get_state ())
			{
				case NM.DeviceState.ACTIVATED:
				case NM.DeviceState.UNKNOWN:
				case NM.DeviceState.UNMANAGED:
				case NM.DeviceState.UNAVAILABLE:
				case NM.DeviceState.DISCONNECTED:
					return false;
				default:
					return true;
			}
		}

		private void device_state_changed (NM.Device  device,
		                                   uint       new_state,
		                                   uint       old_state,
		                                   uint       reason)
		{
			app.change_action_state (device.get_path() + "::is-busy",
			                         new Variant.boolean (device_is_busy (device)));
		}
	}
}
