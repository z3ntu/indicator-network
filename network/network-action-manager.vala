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
		/* Action Stuff */
		private GLibLocal.ActionMuxer    muxer;
		private SimpleActionGroup        actions = new SimpleActionGroup();
		private SimpleAction?            conn_status = null;

		/* Network Manager Stuff */
		private NM.Client                client;

		/* Tracks the current data connection */
		private NM.ActiveConnection?     act_conn = null;
		private NM.Device?               act_dev  = null;
		private NM.AccessPoint?          act_ap   = null;

		/* Tracking a cell modem if there is one */
		private NM.DeviceModem?          modemdev = null;
		private oFono.SIMManager?        simmanager = null;
		private oFono.NetworkRegistration? netreg = null;
		private bool                     airplane_mode = false;
		private bool                     sim_error = false;
		private bool                     sim_installed = false;
		private bool                     sim_locked = false;
		private bool                     roaming = false;
		private string?                  current_protocol = null;
		private int                      cell_strength = 0;
		private int                      last_cell_strength = 0;
		private HashTable<string, oFono.Modem> watched_modems = new HashTable<string, oFono.Modem>(str_hash, str_equal);

		/* State tracking stuff */
		private int                      last_wifi_strength = 0;

		/* Cellular State */

		public ActionManager (GLibLocal.ActionMuxer muxer, NM.Client client)
		{
			this.client = client;
			this.muxer  = muxer;

			muxer.insert("global", actions);

			client.device_added.connect(device_added);
			client.device_removed.connect(device_removed);

			var devices = client.get_devices();
			for (var i = 0; i < devices.length && modemdev == null; i++) {
				device_added(devices[i]);
			}

			/* Make sure this is last as it'll set the state of the
			   icon, so everything needs to be ready */
			add_network_status_action ();

			var settings_action = new SimpleAction("settings", VariantType.STRING);
			settings_action.activate.connect((value) => {
				URLDispatcher.send("settings:///system/" + value.get_string(), (url, success) => {
					if (!success) {
						warning(@"Unable to activate settings URL: $url");
					}
				});
			});
			actions.insert(settings_action);
		}

		~ActionManager ()
		{
			muxer.remove("global");
		}

		private void device_added (NM.Device device) {
			debug(@"Action Manager Device Added: $(device.get_iface())");

			NM.DeviceModem? modemmaybe = device as NM.DeviceModem;

			/* If it's not a modem, we can move on */
			if (modemmaybe == null) {
				return;
			}

			/* We're only going to deal with oFono modems for now */
			if ((modemmaybe.get_current_capabilities() & NM.DeviceModemCapabilities.OFONO) == 0) {
				debug(@"Modem $(device.get_iface()) doesn't have an OFONO capability");
				debug("Not erroring for now");
				/* return; TODO: Galaxy Nexus doesn't do this, so for testing we need to ignore it. */
			}

			/* Check to see if the modem supports voice */
			try {
				oFono.Modem? ofono_modem = watched_modems.lookup(modemmaybe.get_iface());

				if (ofono_modem == null) {
					ofono_modem = Bus.get_proxy_sync (BusType.SYSTEM, "org.ofono", modemmaybe.get_iface());

					ofono_modem.property_changed.connect((prop, value) => {
						if (prop == "Interfaces") {
							device_added(modemmaybe);
						}
					});

					watched_modems.insert(modemmaybe.get_iface(), ofono_modem);
				}

				var modem_properties = ofono_modem.get_properties();
				var interfaces = modem_properties.lookup("Interfaces");

				if (interfaces == null) {
					debug(@"Modem '$(modemmaybe.get_iface())' doesn't have voice support, no interfaces");
					return;
				}

				if (!Utils.variant_contains(interfaces, "org.ofono.VoiceCallManager")) {
					debug(@"Modem '$(modemmaybe.get_iface())' doesn't have voice support only: $(interfaces.print(false))");
					return;
				}
				if (!Utils.variant_contains(interfaces, "org.ofono.SimManager")) {
					debug(@"Modem '$(modemmaybe.get_iface())' doesn't have SIM management support only: $(interfaces.print(false))");
					return;
				}
				if (!Utils.variant_contains(interfaces, "org.ofono.NetworkRegistration")) {
					debug(@"Modem '$(modemmaybe.get_iface())' doesn't have Network Registration support only: $(interfaces.print(false))");
					return;
				}
			} catch (Error e) {
				warning(@"Unable to get oFono modem properties for '$(modemmaybe.get_iface())': $(e.message)");
				return;
			}

			debug("Got a modem");
			modemdev = modemmaybe;

			try {
				/* Initialize the SIM Manager */
				simmanager = Bus.get_proxy_sync (BusType.SYSTEM, "org.ofono", modemmaybe.get_iface());
				simmanager.property_changed.connect(simmanager_property);
				var simprops = simmanager.get_properties();
				simprops.foreach((k, v) => {
					simmanager_property(k, v);
				});

				/* Initialize the Network Registration */
				netreg = Bus.get_proxy_sync (BusType.SYSTEM, "org.ofono", modemmaybe.get_iface());
				netreg.property_changed.connect(netreg_property);
				var netregprops = netreg.get_properties();
				netregprops.foreach((k, v) => {
					netreg_property(k, v);
				});
			} catch (Error e) {
				warning(@"Unable to get oFono information from $(modemdev.get_iface()): $(e.message)");
				simmanager = null;
				netreg = null;
				modemdev = null;
			}

			return;
		}

		private void device_removed (NM.Device device)
		{
			bool changed = false;

			/* Remove the proxy if we have one, ignore the error if not */
			watched_modems.remove(device.get_iface());

			/* The voice modem got killed, bugger */
			if (device.get_iface() == modemdev.get_iface()) {
				changed = true;

				/* Clear the old modemdevice */
				modemdev = null;
				simmanager = null;
				netreg = null;
				current_protocol = null;

				/* Look through the current devices to see if we can find a new modem */
				var devices = client.get_devices();
				for (var i = 0; i < devices.length && modemdev == null; i++) {
					device_added(devices[i]);
				}
			}

			/* Oh, they went for the data!  Jerks!  This is a civil rights violation! */
			if (device.get_iface() == act_dev.get_iface()) {
				active_connections_changed(null, null);
				/* NOTE: Note setting changed because ^ does it already */
			}

			if (changed && conn_status != null)
				conn_status.set_state(build_state());

			return;
		}

		/* Properties from the SIM manager allow us to know the state of the SIM
		   that we've got installed. */
		private void simmanager_property (string prop, Variant value)
		{
			bool changed = false;

			switch (prop) {
			case "Present": {
				var old = sim_installed;
				sim_installed = value.get_boolean();
				changed = (old != sim_installed);
				debug(@"SIM Installed: $(sim_installed)");
				break;
			}
			case "PinRequired": {
				var old = sim_locked;
				sim_locked = (value.get_string() != "none");
				changed = (old != sim_locked);
				debug(@"SIM Lock: $(sim_locked)");
				break;
			}
			}

			if (changed && conn_status != null)
				conn_status.set_state(build_state());

			return;
		}

		/* Properties from the Network Registration which gives us the strength
		   and how we're connecting to it. */
		private void netreg_property (string prop, Variant value)
		{
			bool changed = false;

			switch (prop) {
			case "Technology": {
				var old = current_protocol;
				current_protocol = ofono_tech_to_icon_name(value.get_string());
				changed = (old != current_protocol);
				debug(@"Current Protocol: $(current_protocol)");
				break;
			}
			case "Strength": {
				var old = cell_strength;
				cell_strength = value.get_byte();
				strength_icon(ref last_cell_strength, cell_strength);
				changed = (old != cell_strength);
				/* debug(@"Cell Strength: $(cell_strength)"); */
				break;
			}
			case "Status": {
				var old = roaming;
				roaming = (value.get_string() == "roaming");
				changed = (old != roaming);
				debug(@"Roaming Status: $(roaming)");
				break;
			}
			}

			if (changed && conn_status != null)
				conn_status.set_state(build_state());

			return;
		}

		private string? ofono_tech_to_icon_name (string tech)
		{
			switch (tech) {
			case "gsm":
				return "pre-edge";
			case "edge":
				return "edge";
			case "umts":
				return "3g";
			case "hspa":
				return "hspa";
			/* TODO: oFono can't tell us about hspa+ yet
			case "hspa+":
				return "hspa-plus";
			*/
			case "lte":
				return "lte";
			}

			warning(@"Technology type $tech that we don't understand.  Calling it 'pre-edge'");
			return "pre-edge";
		}

		private Variant? icon_serialize (string icon_name)
		{
			try {
				var icon = GLib.Icon.new_for_string(icon_name);
				return icon.serialize();
			} catch (Error e) {
				warning("Unable to serialize icon '$icon_name' error: $(e.message)");
				return null;
			}
		}

		private Variant build_state ()
		{
			var params = new HashTable<string, Variant>(str_hash, str_equal);
			bool multiicon = false;
			var icons = new Array<Variant>();

			/* If we have cellular data to report or if we need to show the
			   captured connection information, we need more than one icon. */
			if (modemdev != null) {
				if (airplane_mode) {
					var icon = icon_serialize("airplane-mode");
					if (icon != null) {
						icons.append_val(icon);
						multiicon = true;
					}
				} else if (!sim_installed) {
					params.insert("pre-label", new Variant.string(_("No SIM")));
				} else if (sim_error) {
					params.insert("pre-label", new Variant.string(_("SIM Error")));
				} else {
					if (cell_strength == 0) {
						/* Note, looking to the cell strength here, as it's unmodified or
						   parsed for consistency.  We want to know things are really dead. */
						params.insert("pre-label", new Variant.string(_("No Signal")));
					} else {
						string icon_name = "gsm-3g-none";
						switch (last_cell_strength) {
						case 100:
							icon_name = "gsm-3g-full";
							break;
						case 75:
							icon_name = "gsm-3g-high";
							break;
						case 50:
							icon_name = "gsm-3g-medium";
							break;
						case 25:
							icon_name = "gsm-3g-low";
							break;
						}
						var icon = icon_serialize(icon_name);
						if (icon != null) {
							icons.append_val(icon);
							multiicon = true;
						}
					}
				}
			}

			/* Look for the first icon if we need it */
			if (roaming) {
				var icon = icon_serialize("network-cellular-roaming");
				if (icon != null) {
					icons.append_val(icon);
					multiicon = true;
				}
			}

			string data_icon;
			string a11ydesc;

			data_icon_name(out data_icon, out a11ydesc);
			/* We're doing icon always right now so we have a fallback before everyone
			   supports multi-icon.  We shouldn't set both in the future. */
			var icon = icon_serialize(data_icon);
			if (icon != null) {
				params.insert("icon", icon);
				if (multiicon)
					icons.append_val(icon);
			}

			params.insert("title", new Variant.string(_("Network")));
			params.insert("accessibility-desc", new Variant.string(a11ydesc));
			params.insert("visible", new Variant.boolean(true));

			/* Turn the icons array into a variant in the param list */
			if (multiicon && icons.length > 0) {
				VariantBuilder builder = new VariantBuilder(VariantType.ARRAY);

				for (int i = 0; i < icons.length; i++) {
					builder.add_value(icons.index(i));
				}

				params.insert("icons", builder.end());
			}

			/* Convert to a Variant dictionary */
			VariantBuilder final = new VariantBuilder(VariantType.DICTIONARY);

			params.foreach((key, value) => {
				final.add("{sv}", key, value);
			});

			return final.end();
		}

		private void data_icon_name (out string icon_name, out string a11ydesc)
		{
			if (act_dev == null) {
				icon_name = "nm-no-connection";
				a11ydesc = "Network (none)";
				return;
			}

			switch (act_dev.get_device_type ())
			{
				case NM.DeviceType.WIFI: {
					uint8 strength = 0;
					bool secure = false;

					if (act_ap != null) {
						strength = act_ap.strength;
						secure = ((act_ap.get_flags() & NM.80211ApFlags.PRIVACY) != 0);
					}

					if (secure) {
						a11ydesc = _("Network (wireless, %d%%, secure)").printf(strength);
					} else {
						a11ydesc = _("Network (wireless, %d%%)").printf(strength);
					}

					strength_icon(ref last_wifi_strength, strength);

					if (secure) {
						icon_name = "nm-signal-%d-secure".printf(last_wifi_strength);
					} else {
						icon_name = "nm-signal-%d".printf(last_wifi_strength);
					}

					break;
				}
				case NM.DeviceType.ETHERNET:
					icon_name = "network-wired";
					a11ydesc = _("Network (wired)");
					break;
				case NM.DeviceType.MODEM:
					if (current_protocol != null) {
						icon_name = "network-cellular-" + current_protocol;
						a11ydesc = _("Network (cellular, %s)").printf(current_protocol);
					} else {
						icon_name = "network-cellular-pre-edge";
						a11ydesc = _("Network (cellular)");
					}
					break;
				default:
					icon_name = "nm-no-connection";
					a11ydesc = _("Network (none)");
					break;
			}

			return;
		}

		private static void strength_icon (ref int last_strength, int strength)
		{
			if (strength > 70 || (last_strength == 100 && strength > 65)) {
				last_strength = 100;
			} else if (strength > 50 || (last_strength == 75 && strength > 45)) {
				last_strength = 75;
			} else if (strength > 30 || (last_strength == 50 && strength > 25)) {
				last_strength = 50;
			} else if (strength > 10 || (last_strength == 25 && strength > 5)) {
				last_strength = 25;
			} else {
				last_strength = 0;
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
													 null,
													 build_state());
			actions.insert (conn_status);

			client.notify["active-connections"].connect (active_connections_changed);
			active_connections_changed(null, null);
		}

		private void active_access_point_changed (GLib.Object? client, ParamSpec? ps)
		{
			if (act_ap != null)
			{
				act_ap.notify["strength"].disconnect (active_connection_strength_changed);
				act_ap = null;
			}

			act_ap = (act_dev as NM.DeviceWifi).get_active_access_point();
			if (act_ap != null) {
				act_ap.notify["strength"].connect (active_connection_strength_changed);
			}
		}

		private void active_connection_strength_changed (GLib.Object? client, ParamSpec? ps)
		{
			conn_status.set_state(build_state());
		}

		private void active_connections_changed (GLib.Object? client, ParamSpec? ps)
		{
			/* Remove previous active connection */
			if (act_conn != null)
			{
				act_conn.notify["state"].disconnect (active_connections_changed);
				act_conn.notify["default"].disconnect (active_connections_changed);
				act_conn.notify["default6"].disconnect (active_connections_changed);

				if (act_dev != null)
				{
					switch (act_dev.get_device_type ())
					{
						case NM.DeviceType.WIFI:
							var dev = act_dev as NM.DeviceWifi;

							dev.notify["active-access-point"].disconnect (active_access_point_changed);

							if (act_ap != null)
							{
								act_ap.notify["strength"].disconnect (active_connection_strength_changed);
								act_ap = null;
							}

							break;
						default:
							break;
					}

					act_dev = null;
				}

				act_conn = null;
			}

			act_conn = get_active_connection ();
			if (act_conn != null) {
				act_dev = get_device_from_connection (act_conn);
			}

			/* If the connection doesn't have a device yet, let's not switch
			   to looking at it */
			if (act_dev == null) {
				act_conn = null;
			}

			/* If we have a device, we have a connection, let's make this our
			   active connection */
			if (act_dev != null) {
				act_conn.notify["state"].connect (active_connections_changed);
				act_conn.notify["default"].connect (active_connections_changed);
				act_conn.notify["default6"].connect (active_connections_changed);

				debug(@"Active connection changed to: $(act_dev.get_iface())");

				if (act_dev.get_device_type() == NM.DeviceType.WIFI) {
					act_dev.notify["active-access-point"].connect (active_access_point_changed);
					active_access_point_changed(null, null);
				}
			}

			conn_status.set_state(build_state());
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

		private NM.Device? get_device_from_connection (NM.ActiveConnection conn)
		{
			var devices = conn.get_devices ();

			if (devices == null)
				return null;

			/* The list length should always == 1 */
			if (devices.length == 1)
				return devices.get (0);

			warning ("Connection has a list of devices length different than 0");
			return null;
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
