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

namespace Network.Device
{
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

			update_sim_lock_menu(mobilesimmanager.pin_required);
			mobilesimmanager.notify["pin-required"].connect((s, value) => {
				update_sim_lock_menu(mobilesimmanager.pin_required);
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

		private void update_sim_lock_menu(bool pin_required)
		{
			string action_name = action_prefix + "unlock";
			debug(@"sim lock updated $(pin_required) - action $action_name");
			for (int i = 0; i < apsmenu.get_n_items(); i++)
			{
				string name;

				if (!apsmenu.get_item_attribute (i, "action", "s", out name))
					continue;
				debug(@"update_sim_lock_menu action $name");

				if (name == action_name) {
					if (!pin_required) {
						apsmenu.remove (i);
					}
					return;
				}
			}

			if (pin_required) {
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
				if (mobilesimmanager.pin_unlocking) {
					debug(@"SIM unlock already in progress");
					return;
				}
				mobilesimmanager.send_unlock_notification();
			});
			actions.insert(unlock_action);

			unlock_action.set_enabled(mobilesimmanager.pin_required && !mobilesimmanager.pin_unlocking);
			mobilesimmanager.notify["pin-required"].connect((s, value) => {
				unlock_action.set_enabled(mobilesimmanager.pin_required && !mobilesimmanager.pin_unlocking);
			});
			mobilesimmanager.notify["pin-unlocking"].connect((s, value) => {
				unlock_action.set_enabled(mobilesimmanager.pin_required && !mobilesimmanager.pin_unlocking);
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

}
