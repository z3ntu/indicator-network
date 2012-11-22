// vim: tabstop=4 noexpandtab shiftwidth=4 softtabstop=4
using NM;

namespace Unity.Settings.Network
{
	internal class WifiActionManager
	{
		private Application   app;
		private NM.Client     client;
		public  NM.DeviceWifi wifidev = null;

		public WifiActionManager (Application app, NM.Client client, NM.DeviceWifi dev)
		{
			this.client  = client;
			this.app     = app;
			this.wifidev = dev;

			/* This object should be disposed by ActionManager on device removal
			 * but we still disconnect signals if that signal is emmited before
			 * this object was disposed
			 */
			client.device_removed.connect (remove_device);

			wifidev.access_point_added.connect   (access_point_added_cb);
			wifidev.access_point_removed.connect (access_point_removed_cb);
			wifidev.notify.connect               (active_access_point_changed);
			wifidev.state_changed.connect        (device_state_changed_cb);

			var busy_action_id = wifidev.get_path () + "::is-busy";
			var is_busy = device_is_busy (wifidev);
			var action = new SimpleAction.stateful (busy_action_id,
			                                        VariantType.BOOLEAN,
			                                        new Variant.boolean (is_busy));
			app.add_action (action);

			var aps = dev.get_access_points ();
			if (aps == null)
				return;

			for (int i = 0; i < aps.length; i++)
				insert_ap (aps.get(i));
		}

		~WifiActionManager ()
		{
			remove_device (client, wifidev);
			client.device_removed.disconnect (remove_device);

			app.remove_action (wifidev.get_path () + "::is-busy");
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
				app.change_action_state (ap.get_path(),
				                         new Variant.boolean (false));
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

				if (app.lookup_action (ap.get_path ()) == null)
				{
					insert_ap (ap);
					continue;
				}

				if (ap.get_path () == active_ap)
				{
					app.change_action_state (ap.get_path(),
				                             new Variant.boolean (true));
				}
				else
				{
					app.change_action_state (ap.get_path(),
				                             new Variant.boolean (false));
				}
			}
		}

		private void insert_ap (NM.AccessPoint ap)
		{
			if (ap == null)
				return;

			bool is_active = false;

			/* If the ap is already included we skip this callback */
			if (app.lookup_action (ap.get_path ()) != null)
				return;

			//TODO: Add actions for each AP NM connection
			var strength_action_id = ap.get_path () + "::strength";
			ap.notify.connect (ap_strength_changed);
			if (wifidev.active_access_point != null)
				is_active = ap.get_path () == wifidev.active_access_point.get_path ();

			var strength = new SimpleAction.stateful (strength_action_id,
			                                          VariantType.BYTE,
			                                          new Variant.byte (ap.get_strength ()));
			var activate = new SimpleAction.stateful (ap.get_path (),
			                                          VariantType.BOOLEAN,
			                                          new Variant.boolean (is_active));
			activate.activate.connect (ap_activated);
			activate.change_state.connect (ap_state_changed);

			app.add_action (strength);
			app.add_action (activate);
		}

		private void remove_ap (NM.AccessPoint ap)
		{
			//TODO: Check if AP has connection action
			app.remove_action (ap.get_path ());
			app.remove_action (ap.get_path () + "::strength");
		}

		private void ap_activated (SimpleAction ac, Variant? val)
		{
			if (val.get_boolean () == false         &&
				wifidev.active_access_point != null &&
				wifidev.active_access_point.get_path () == ac.get_name ())
			{
				wifidev.disconnect (null);
				return;
			}

			var rs = new NM.RemoteSettings (null);
			if (wifidev.active_access_point != null &&
				wifidev.active_access_point.get_path () == ac.get_name ())
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

		private void ap_state_changed (SimpleAction ac, Variant? val)
		{
			string path = ac.get_name ();

			if (wifidev.active_access_point != null &&
				wifidev.active_access_point.get_path () != path &&
				val.get_boolean ())
			{
				ap_activated (ac, val);
				return;
			}

			var aps = wifidev.get_access_points ();
			if (aps == null)
			{
				ac.set_state (new Variant.boolean (false));
				return;
			}

			for (uint i = 0; i < aps.length; i++)
			{
				var ap = aps.get(i);
				if (ap == null)
					continue;

				if (ap.get_path () != path)
					continue;

                ac.set_state (val);

                if (val.get_boolean ())
                    ap_activated (ac, val);
			}
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
		private List<WifiActionManager*> wifimgrs;

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
			client.device_removed.connect ((client, device) => {remove_device (device);});

			var devices = client.get_devices ();
			if (devices == null)
				return;

			for (uint i = 0; i < devices.length ; i++)
			{
				var device = devices.get(i);
				add_device (device);
			}
		}

		private void add_device (NM.Device device)
		{
			//TODO: More device types
			device.state_changed.connect (device_state_changed);
			switch (device.get_device_type ())
			{
				case NM.DeviceType.WIFI:
					var wifidev = (NM.DeviceWifi)device;
					//TODO: Get rid of the action manager on device removal
					//FIXME: Manage WAM instances
					wifimgrs.append (new WifiActionManager (app, client, wifidev));
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

		private void remove_wifi_device (NM.DeviceWifi device)
		{
			for (uint i = 0; i < wifimgrs.length (); i++)
			{
				var mgr = wifimgrs.nth_data(i);
				if (mgr == null || mgr->wifidev == null)
					continue;
				if (mgr->wifidev.get_path () != device.get_path ())
					continue;

				wifimgrs.remove (mgr);
				delete mgr;
				return;
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

	/* Common utils */
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
}
