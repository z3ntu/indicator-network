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

namespace Network
{

	public class ActionManager
	{
		private Application              app;
		private NM.Client                client;
		private SimpleAction             conn_status;
		private NM.ActiveConnection?     act_conn = null;
		private NM.AccessPoint?          act_ap   = null;
		private int                      last_wifi_strength = 0;

		public ActionManager (Application app, NM.Client client)
		{
			this.client = client;
			this.app    = app;

			add_network_status_action ();
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
			bool secure = act_ap.get_wpa_flags() != 0;

			string a11y_name;
			if (secure) {
				a11y_name = "Network (wireless, %d%, secure)".printf(strength);
			} else {
				a11y_name = "Network (wireless, %d%)".printf(strength);
			}

			if (strength > 70 || (last_wifi_strength == 100 && strength > 65)) {
				last_wifi_strength = 100;
			} else if (strength > 50 || (last_wifi_strength == 75 && strength > 45)) {
				last_wifi_strength = 75;
			} else if (strength > 30 || (last_wifi_strength == 50 && strength > 25)) {
				last_wifi_strength = 50;
			} else if (strength > 10 || (last_wifi_strength == 25 && strength > 5)) {
				last_wifi_strength = 25;
			} else {
				last_wifi_strength = 0;
			}

			string icon_name;
			if (secure) {
				icon_name = "nm-signal-%d-secure".printf(last_wifi_strength);
			} else {
				icon_name = "nm-signal-%d".printf(last_wifi_strength);
			}

			conn_status.set_state (new Variant ("(sssb)", "", icon_name, a11y_name, true));
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
