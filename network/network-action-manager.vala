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
	internal class WifiActionManager
	{
		private Application       app;
		private NM.Client         client;
		public NM.RemoteSettings rs  = null;

		public  NM.DeviceWifi     wifidev = null;

		public WifiActionManager (Application app, NM.Client client, NM.DeviceWifi dev)
		{
			this.client  = client;
			this.app     = app;
			this.wifidev = dev;

			var busy_action_id = wifidev.get_path () + "::is-busy";
			var is_busy = device_is_busy (wifidev);
			var action = new SimpleAction.stateful (busy_action_id,
			                                        VariantType.BOOLEAN,
			                                        new Variant.boolean (is_busy));
			app.add_action (action);

			rs = new NM.RemoteSettings (wifidev.get_connection ());
			rs.connections_read.connect (bootstrap_actions);
		}


		private  void bootstrap_actions ()
		{
			/* This object should be disposed by ActionManager on device removal
			 * but we still disconnect signals if that signal is emmited before
			 * this object was disposed
			 */
			client.device_removed.connect (remove_device);

			wifidev.access_point_added.connect   (access_point_added_cb);
			wifidev.access_point_removed.connect (access_point_removed_cb);
			wifidev.notify.connect               (active_access_point_changed);
			wifidev.state_changed.connect        (device_state_changed_cb);


			var aps = wifidev.get_access_points ();
			if (aps == null)
				return;

			for (int i = 0; i < aps.length; i++)
				insert_ap (aps.get(i));
		}

		~WifiActionManager ()
		{
			remove_device (client, wifidev);

			client.device_removed.disconnect (remove_device);
			wifidev.access_point_added.disconnect   (access_point_added_cb);
			wifidev.access_point_removed.disconnect (access_point_removed_cb);
			wifidev.notify.disconnect               (active_access_point_changed);
			wifidev.state_changed.disconnect        (device_state_changed_cb);
			rs.connections_read.disconnect          (bootstrap_actions);

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

			if (wifidev.active_access_point != null &&
				wifidev.active_access_point.get_path () == path &&
				val.get_boolean () == false)
			{
				wifidev.disconnect(null);
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

				return;
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
		private Application              app;
		private NM.Client                client;
		private List<WifiActionManager*> wifimgrs;
		private SimpleAction             conn_status;
		private NM.ActiveConnection?     act_conn = null;
		private NM.AccessPoint?          act_ap   = null;

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

			add_network_status_action ();
			var devices = client.get_devices ();
			if (devices == null)
				return;

			for (uint i = 0; i < devices.length ; i++)
			{
				var device = devices.get(i);
				add_device (device);
			}
		}

		private void add_network_status_action ()
		{
			/* This is the action that represents the global status of the network.
			 *
			 * - The first guint32 is for the device type of the main connection as per
			 *   NetworkManager.h's NMDeviceType.
			 * - The second one represents the connection state as per NMActiveConnectionState.
			 * - The third one is for the extended status. In the case of Wifi it represents
			 *   signal strength.
			 */

			conn_status = new SimpleAction.stateful ("network-status",
													 new VariantType ("(sssb)"),
													 new Variant("(sssb)", "", "network-offline", "Network (none)", true));
			app.add_action (conn_status);

			client.notify["active-connections"].connect (active_connections_changed);
			set_active_connection ();
		}

		private void set_active_connection ()
		{
			act_conn = get_active_connection ();
			if (act_conn != null)
			{
				act_conn.notify["state"].connect (connection_state_changed);
				var type = get_device_type_from_connection (act_conn);
				switch (type)
				{
					case NM.DeviceType.WIFI:
						set_wifi_device ();
						break;
					case NM.DeviceType.ETHERNET:
						conn_status.set_state (new Variant ("(sssb)", "", "network-wired", "Network (wired)", true));
						break;
					default:
						conn_status.set_state (new Variant ("(sssb)", "", "network-offline", "Network (none)", true));
						break;
				}
			}
			else
			{
				conn_status.set_state (new Variant ("(sssb)", "", "network-offline", "Network (none)", true));
			}
		}

		private void set_conn_status_wifi (uint8 strength)
		{
			if (strength < 64) {
				conn_status.set_state (new Variant ("(sssb)", "", "nm-signal-25", "Network (wireless, 25%)", true));
			} else if (strength < 128) {
				conn_status.set_state (new Variant ("(sssb)", "", "nm-signal-50", "Network (wireless, 50%)", true));
			} else if (strength < 192) {
				conn_status.set_state (new Variant ("(sssb)", "", "nm-signal-75", "Network (wireless, 75%)", true));
			} else {
				conn_status.set_state (new Variant ("(sssb)", "", "nm-signal-100", "Network (wireless, 100%)", true));
			}
		}

		private void set_wifi_device ()
		{
			uint8 strength = 0;
			var dev = act_conn.get_devices ().get(0) as NM.DeviceWifi;
			if (dev != null)
			{
				dev.notify["active-access-point"].connect (active_access_point_changed);
				act_ap = dev.active_access_point;
				if (act_ap != null)
				{
					act_ap.notify["strength"].connect (active_connection_strength_changed);
					strength = act_ap.strength;
				}
			}

			set_conn_status_wifi(strength);
		}

		private void connection_state_changed (GLib.Object client, ParamSpec ps)
		{
			switch (get_device_type_from_connection (act_conn))
			{
				case NM.DeviceType.WIFI:
					uint8 strength = 0;
					if (act_ap != null)
						strength = act_ap.strength;

					set_conn_status_wifi(strength);
					break;
				case NM.DeviceType.ETHERNET:
					conn_status.set_state (new Variant ("(sssb)", "", "network-wired", "Network (wired)", true));
					break;
				default:
					conn_status.set_state (new Variant ("(sssb)", "", "network-offline", "Network (none)", true));
					break;
			}
		}

		private void active_access_point_changed (GLib.Object client, ParamSpec ps)
		{
			if (act_ap != null)
			{
				act_ap.notify["strength"].disconnect (active_connection_strength_changed);
				act_ap = null;
			}

			set_wifi_device ();
		}

		private void active_connection_strength_changed (GLib.Object client, ParamSpec ps)
		{
			uint8 strength = 0;
			if (act_ap != null)
				strength = act_ap.strength;

			set_conn_status_wifi(strength);
		}

		private void active_connections_changed (GLib.Object client, ParamSpec ps)
		{
			/* Remove previous active connection */
			if (act_conn != null)
			{
				act_conn.notify["state"].disconnect (connection_state_changed);

				var type = get_device_type_from_connection (act_conn);
				switch (type)
				{
					case NM.DeviceType.WIFI:
						var dev = act_conn.get_devices ().get(0) as NM.DeviceWifi;
						if (dev != null)
							dev.notify["active-access-point"].connect (active_access_point_changed);

						if (act_ap != null)
						{
							act_ap.notify["strength"].disconnect (active_connection_strength_changed);
							act_ap = null;
						}

						break;
					default:
						break;
				}
			}

			set_active_connection ();
		}

		/* This function guesses the default connection in case
		 * multiple ones are connected */
		private NM.ActiveConnection? get_active_connection ()
		{
			ActiveConnection? def6 = null;

			/* The default IPv4 connection has precedence */
			var conns = client.get_active_connections ();

			if (conns == null)
				return null;

			for (uint i = 0; i < conns.length; i++)
			{
				var conn = conns.get(i);

				if (conn.default)
					return conn;

				if (conn.default6)
					def6 = conn;
			}

			/* Then the default IPv6 connection otherwise the first in the list */
			if (def6 != null)
				return def6;
			/*TODO: Do we show an active connetion if no default route is present? */
			else if (conns.length > 0)
				return conns.get(0);

			/* If the list is empty we return null */
			return null;
		}

		private NM.DeviceType get_device_type_from_connection (NM.ActiveConnection conn)
		{
			var devices = conn.get_devices ();

			/* The list length should always == 1 */
			if (devices.length == 1)
				return devices.get (0).get_device_type ();

			warning ("Connection has a list of devices length different than 0");
			return NM.DeviceType.UNKNOWN;
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
