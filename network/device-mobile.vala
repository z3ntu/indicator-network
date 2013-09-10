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
 *      Ted Gould <ted.gould@canonical.com>
 */

using NM;
using Notify;

namespace Network.Device
{

	internal class MobileSimManager : GLib.Object
	{
		private NM.Client         		client;
		private NM.DeviceModem				device;
		private GLib.DBusConnection		conn;
		private string 								namespace;
		private oFono.SIMManager?  		simmanager = null;
		private oFono.Modem?					ofono_modem = null;

		public bool sim_installed { get; private set; default = false; }
		public bool sim_locked { get; private set; default = false; }
		public bool sim_unlocking { get; private set; default = false; }

		// Sim Unlocking
		private Notification? sim_notification = null;
		private Menu? unlock_menu = null;
		private uint unlock_menu_export_id = 0;
		private SimpleActionGroup? unlock_actions = null;
		private uint unlock_actions_export_id = 0;
		private string curren_pin = "";

		private const string APPLICATION_ID  = "com.canonical.indicator.network";
		private const string SIM_UNLOCK_MENU_PATH = "/com/canonical/indicator/network/unlocksim";
		private const string SIM_UNLOCK_ACTION_PATH = "/com/canonical/indicator/network/unlocksim";

		public MobileSimManager (NM.Client client, DeviceModem device, GLib.DBusConnection conn, string namespace)
		{
			this.client = client;
			this.device = device;
			this.conn = conn;
			this.namespace = namespace;

			create_ofono_sim_manager();
		}

		public void sim_unlock_activated ()
		{
			warning(@"SIM Unlock:");

			if (sim_notification != null) {
				return;
			}
			curren_pin = "";

			// notification
			sim_notification = new Notification("Unlock SIM", "", "totem");
			sim_notification.set_hint_string ("x-canonical-ext-snap-decisions", "true");

			warning(@"conn unique_name $(conn.get_unique_name())");

			VariantBuilder menu_model_actions = new VariantBuilder (new VariantType ("a{sv}") );
			menu_model_actions.add ("{sv}", "notifications", new Variant.string (SIM_UNLOCK_ACTION_PATH));

			VariantBuilder menu_model_paths = new VariantBuilder (new VariantType ("a{sv}") );
			menu_model_paths.add ("{sv}", "busName", new Variant.string (APPLICATION_ID));
			menu_model_paths.add ("{sv}", "menuPath", new Variant.string (SIM_UNLOCK_MENU_PATH));
			menu_model_paths.add ("{sv}", "actions", menu_model_actions.end ());

			sim_notification.set_hint ("x-canonical-private-menu-model", menu_model_paths.end ());

			// menu
			unlock_menu = new Menu ();
			var pin_unlock = new MenuItem ("", "notifications." + namespace + ".simunlock");
			pin_unlock.set_attribute ("x-canonical-type", "s", "com.canonical.snapdecision.pinlock");
			unlock_menu.append_item (pin_unlock);

			// actions = true;
			unlock_actions = new SimpleActionGroup();
			var pin_item = new SimpleAction.stateful(namespace + ".simunlock", VariantType.BOOLEAN, new Variant.string(""));
			pin_item.activate.connect (sim_unlock_pinitem_activated);
			pin_item.change_state.connect (sim_unlock_pinitem_changed);
			unlock_actions.insert (pin_item);

			try {
				unlock_menu_export_id = conn.export_menu_model (SIM_UNLOCK_MENU_PATH, unlock_menu);
				unlock_actions_export_id = conn.export_action_group (SIM_UNLOCK_ACTION_PATH, unlock_actions as ActionGroup);

				sim_unlocking = true;
				sim_notification.closed.connect (notification_closed);
				sim_notification.show ();

			} catch (Error e) {
				warning(@"Unable to unlock sim: $(e.message)");
				return;
			}
		}

		private void create_ofono_sim_manager()
		{
			try {
				if (ofono_modem == null) {
					ofono_modem = Bus.get_proxy_sync (BusType.SYSTEM, "org.ofono", device.get_iface());
				}

				ofono_modem.property_changed.connect((prop, value) => {
					if (prop == "Interfaces") {
						create_ofono_sim_manager();
					}
				});

				var modem_properties = ofono_modem.get_properties();
				var interfaces = modem_properties.lookup("Interfaces");

				if (!Utils.variant_contains(interfaces, "org.ofono.SimManager")) {
					debug(@"Modem '$(device.get_iface())' doesn't have SIM management support only: $(interfaces.print(false))");
					return;
				}
			} catch (Error e) {
				warning(@"Unable to get oFono modem properties for '$(device.get_iface())': $(e.message)");
				return;
			}

			try {
				/* Initialize the SIM Manager */
				simmanager = Bus.get_proxy_sync (BusType.SYSTEM, "org.ofono", device.get_iface());
				simmanager.property_changed.connect(simmanager_property);
				var simprops = simmanager.get_properties();
				simprops.foreach((k, v) => {
					simmanager_property(k, v);
				});

			} catch (Error e) {
				warning(@"Unable to get oFono information from $(device.get_iface()): $(e.message)");
				simmanager = null;
			}

			return;
		}

		/* Properties from the SIM manager allow us to know the state of the SIM
			 that we've got installed. */
		private void simmanager_property (string prop, Variant value)
		{
			debug(@"simmanager_property: $(prop)");

			switch (prop) {
				case "Present": {
					sim_installed = value.get_boolean();
					debug(@"SIM Installed: $(sim_installed)");
					break;
				}
				case "PinRequired": {
					sim_locked = true;//(value.get_string() != "none"););
					debug(@"SIM Lock: $(sim_locked)");
					break;
				}
			}
		}

		private void cancel_callback (Notification notification, string action)
		{
			warning("Cancel: SIM Unlock");
		}

		private void ok_callback (Notification notification, string action)
		{
			warning("OK: SIM Unlock");
		}

		private void notification_closed ()
		{
			warning("Done with notification");
			sim_unlocking = false;
			sim_notification = null;

			conn.unexport_menu_model(unlock_menu_export_id);
			unlock_menu_export_id = 0;
			unlock_menu = null;

			conn.unexport_menu_model(unlock_actions_export_id);
			unlock_actions_export_id = 0;
			unlock_actions = null;
		}

		private void sim_unlock_pinitem_activated (SimpleAction ac, Variant? val)
		{
			if (sim_notification != null && val.get_boolean() == false) {
				try {
					sim_notification.close();
				} catch (Error e) {
					warning(@"Failed to close sim unlock notification");
				}
			}
		}

		private void sim_unlock_pinitem_changed (SimpleAction ac, Variant? val)
		{
			warning("SIM Pin Changed");
			curren_pin = val.get_string();
		}

	}

	internal class MobileMenu
	{
		private NM.Client   		 client;
		private NM.DeviceModem	 device;
		private Menu        		 apsmenu;
		private string      		 action_prefix;
		private MobileSimManager mobilesimmanager;

		private MenuItem    		 device_item;
		private MenuItem    		 settings_item;
		private MenuItem    		 unlock_sim_item;

		public MobileMenu (NM.Client client, DeviceModem device, Menu global_menu, string action_prefix, bool show_enable, MobileSimManager mobilesimmanager)
		{
			this.client = client;
			this.device = device;
			this.apsmenu = global_menu;
			this.action_prefix = action_prefix;
			this.mobilesimmanager = mobilesimmanager;

			if (show_enable) {
				device_item = create_item_for_mobile_device();
				this.apsmenu.append_item(device_item);
			}

			settings_item = new MenuItem(_("Cellular settings…"), "indicator.global.settings::cellular");
			this.apsmenu.append_item(settings_item);

			update_sim_lock_menu(mobilesimmanager.sim_locked);
			mobilesimmanager.notify["sim-locked"].connect((s, value) => {
				update_sim_lock_menu(mobilesimmanager.sim_locked);
			});
		}

		~MobileMenu ()
		{
		}

		private MenuItem create_item_for_mobile_device ()
		{
			var device_item = new MenuItem(_("Cellular"), action_prefix + ".device-enabled");
			device_item.set_attribute ("x-canonical-type" , "s", "com.canonical.indicator.switch");

			return device_item;
		}

		private void update_sim_lock_menu(bool sim_locked)
		{
			string action_name = action_prefix + "unlock";
			for (int i = 1; i < apsmenu.get_n_items(); i++)
			{
				string name;

				if (!apsmenu.get_item_attribute (i, "action", "s", out name))
					continue;

				if (name == action_name) {
					if (!sim_locked) {
						apsmenu.remove (i);
					}
					return;
				}
			}

			if (sim_locked) {
				unlock_sim_item = new MenuItem(_("Unlock SIM…"), action_name);
				apsmenu.insert_item (0, unlock_sim_item);
			}
		}
	}

	internal class MobileActionManager
	{
		private SimpleActionGroup   actions;
		private NM.Client         	client;
		private NM.DeviceModem			device;
		private MobileSimManager 		mobilesimmanager;

		private SimpleAction        unlock_action;

		public MobileActionManager (SimpleActionGroup actions, NM.Client client, NM.DeviceModem device, MobileSimManager mobilesimmanager)
		{
			this.actions = actions;
			this.client  = client;
			this.device = device;
			this.mobilesimmanager = mobilesimmanager;

			unlock_action = new SimpleAction("unlock", null);
			unlock_action.activate.connect((ac,ps) => {
				mobilesimmanager.sim_unlock_activated();
			});
			actions.insert(unlock_action);

			unlock_action.set_enabled(mobilesimmanager.sim_locked && !mobilesimmanager.sim_unlocking);
			mobilesimmanager.notify["sim-locked"].connect((s, value) => {
				unlock_action.set_enabled(mobilesimmanager.sim_locked && !mobilesimmanager.sim_unlocking);
			});
			mobilesimmanager.notify["sim-unlocking"].connect((s, value) => {
				unlock_action.set_enabled(mobilesimmanager.sim_locked && !mobilesimmanager.sim_unlocking);
			});
		}
	}

	public class Mobile : Base {
		private MobileMenu mobilemenu;
		private MobileActionManager mobileactionmanager;
		private MobileSimManager mobilesimmanager;

		public Mobile (NM.Client client, NM.DeviceModem device, GLibLocal.ActionMuxer muxer, bool show_enable, GLib.DBusConnection conn) {
			GLib.Object(
				client: client,
				device: device,
				namespace: device.get_iface(),
				muxer: muxer
			);

			mobilesimmanager = new MobileSimManager(client, device, conn, this.namespace);
			mobilemenu = new MobileMenu(client, device, this._menu, "indicator." + this.namespace + ".", show_enable, mobilesimmanager);
			mobileactionmanager = new MobileActionManager(actions, client, device, mobilesimmanager);
		}

		protected override void disable_device ()
		{
			device.disconnect(null);
			client.wwan_set_enabled(false);
		}

		protected override void enable_device ()
		{
			client.wwan_set_enabled(true);
			device.set_autoconnect(true);
		}
	}





	// public class Mobile : Base {
	// 	private GLib.MenuItem enabled_item;
	// 	private GLib.MenuItem settings_item;
	// 	private GLib.MenuItem unlock_sim_item;
	// 	private GLib.DBusConnection conn;

	// 	private oFono.SIMManager? simmanager = null;
	// 	private SimpleAction? unlock_action = null;

	// 	private Notification? sim_notification = null;
	// 	private Menu? unlock_menu = null;
	// 	private uint unlock_menu_export_id = 0;
	// 	private SimpleActionGroup? unlock_actions = null;
	// 	private uint unlock_actions_export_id = 0;
	// 	private string curren_pin = "";

	// 	private const string APPLICATION_ID  = "com.canonical.indicator.network";
	// 	private const string SIM_UNLOCK_MENU_PATH = "/com/canonical/indicator/network/unlocksim";
	// 	private const string SIM_UNLOCK_ACTION_PATH = "/com/canonical/indicator/network/unlocksim";

	// 	public Mobile (NM.Client client, NM.DeviceModem device, GLibLocal.ActionMuxer muxer, bool show_enable, GLib.DBusConnection conn) {
	// 		GLib.Object(
	// 			client: client,
	// 			device: device,
	// 			namespace: device.get_iface(),
	// 			muxer: muxer
	// 		);
	// 		this.conn = conn;

	// 		if (show_enable) {
	// 			enabled_item = new MenuItem(_("Cellular"), "indicator." + device.get_iface() + ".device-enabled");
	// 			enabled_item.set_attribute ("x-canonical-type"  ,           "s", "com.canonical.indicator.switch");
	// 			_menu.append_item(enabled_item);
	// 		}

	// 		settings_item = new MenuItem(_("Cellular settings…"), "indicator.global.settings::cellular");
	// 		_menu.append_item(settings_item);

	// 		unlock_action = new SimpleAction("unlock", null);
	// 		unlock_action.activate.connect(sim_unlock_activated);
	// 		actions.insert(unlock_action);
	// 	}

	// 	~Mobile ()
	// 	{
	// 		muxer.remove(namespace);

	// 		if (unlock_menu_export_id != 0)
	// 			conn.unexport_menu_model(unlock_menu_export_id);
	// 		if (unlock_menu_export_id != 0)
	// 			conn.unexport_menu_model(unlock_actions_export_id);
	// 	}

	// 	public void sim_lock_updated (bool sim_locked)
	// 	{
	// 		string action_name = "indicator." + device.get_iface() + ".unlock";
	// 		for (int i = 1; i < _menu.get_n_items(); i++)
	// 		{
	// 			string name;

	// 			if (!_menu.get_item_attribute (i, "action", "s", out name))
	// 				continue;

	// 			if (name == action_name) {
	// 				if (!sim_locked) {
	// 					_menu.remove (i);
	// 				}
	// 				return;
	// 			}
	// 		}

	// 		if (sim_locked) {
	// 			unlock_sim_item = new MenuItem(_("Unlock SIM…"), action_name);
	// 			_menu.insert_item (0, unlock_sim_item);
	// 		}
	// 	}

	// 	protected override void disable_device ()
	// 	{
	// 		device.disconnect(null);
	// 		client.wwan_set_enabled(false);
	// 	}

	// 	protected override void enable_device ()
	// 	{
	// 		client.wwan_set_enabled(true);
	// 		device.set_autoconnect(true);
	// 	}

	// }
}
