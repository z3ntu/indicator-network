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

namespace Network.Device
{

	public class Base : MenuModel {
		NM.Client _client;
		NM.Device _device;
		string _namespace;
		GLibLocal.ActionMuxer _muxer;
		protected Menu _menu = new Menu();

		protected GLib.SimpleActionGroup actions = new GLib.SimpleActionGroup();
		protected GLib.SimpleAction enabled_action = new SimpleAction.stateful("device-enabled", null, new Variant.boolean(false));
		protected GLib.SimpleAction busy_action = new SimpleAction.stateful("device-busy", null, new Variant.boolean(false));

		private NM.ActiveConnection? active_connection = null;
		private ulong active_connection_notify = 0;

		protected virtual void enable_device ()
		{
			warning("Subclass doesn't have a way to enable the device");
			return;
		}

		/*****************************
		 * Properties
		 *****************************/
		public NM.Device device {
			construct {
				_device = value;
				return;
			}
			get {
				return _device;
			}
		}

		public NM.Client client {
			construct {
				_client = value;
				return;
			}
			get {
				return _client;
			}
		}

		public string namespace {
			construct {
				_namespace = value;
				return;
			}
			get {
				return _namespace;
			}
		}

		public GLibLocal.ActionMuxer muxer {
			construct {
				_muxer = value;
				return;
			}
			get {
				return _muxer;
			}
		}

		/*****************************
		 * Functions
		 *****************************/

		construct {
			_menu.items_changed.connect(menu_items_changed);

			actions.insert(enabled_action);
			actions.insert(busy_action);

			_muxer.insert(_namespace, actions);

			enabled_action.activate.connect((param) => {
				if (enabled_action.state.get_boolean()) {
					device.disconnect(null);
				} else {
					enable_device();
				}
			});

			device.notify.connect((pspec) => {
				if (pspec.name == "active-connection") {
					active_connection_changed();
				}
			});
			active_connection_changed();

		}

		~Base ()
		{
			muxer.remove(namespace);
		}

		private void active_connection_changed () {
			if (active_connection != null) {
				active_connection.disconnect(active_connection_notify);
			}

			active_connection = this.device.get_active_connection();

			if (active_connection != null) {
				active_connection_notify = active_connection.notify.connect((pspec) => {
					if (pspec.name == "state") {
						active_connection_state_changed(active_connection.state);
					}
				});

				active_connection_state_changed(active_connection.state);
			}
		}

		private void active_connection_state_changed (uint new_state) {
			switch (new_state) {
				case NM.ActiveConnectionState.ACTIVATING:
					debug("Marking '" + device.get_iface() + "' as Activating");
					busy_action.set_state(new Variant.boolean(true));
					enabled_action.set_state(new Variant.boolean(true));
					break;
				case NM.ActiveConnectionState.ACTIVATED:
					debug("Marking '" + device.get_iface() + "' as Active");
					busy_action.set_state(new Variant.boolean(false));
					enabled_action.set_state(new Variant.boolean(true));
					break;
				case NM.ActiveConnectionState.DEACTIVATING:
					debug("Marking '" + device.get_iface() + "' as Deactivating");
					busy_action.set_state(new Variant.boolean(true));
					enabled_action.set_state(new Variant.boolean(false));
					break;
				default:
					debug("Marking '" + device.get_iface() + "' as Disabled");
					enabled_action.set_state(new Variant.boolean(false));
					enabled_action.set_state(new Variant.boolean(false));
					break;
			}
		}

		void menu_items_changed (int position, int removed, int added) {
			(this as MenuModel).items_changed(position, removed, added);
		}

		/***********************************
		 * Passing on functions to our menu
		 ***********************************/
		public override GLib.Variant get_item_attribute_value (int item_index, string attribute, GLib.VariantType? expected_type) {
			return (_menu as MenuModel).get_item_attribute_value(item_index, attribute, expected_type);
		}

		public override void get_item_attributes (int item_index, out GLib.HashTable<void*,void*> attributes) {
			(_menu as MenuModel).get_item_attributes(item_index, out attributes);
		}

		public override GLib.MenuModel get_item_link (int item_index, string link) {
			return (_menu as MenuModel).get_item_link(item_index, link);
		}

		public override void get_item_links (int item_index, out GLib.HashTable<void*,void*> links) {
			(_menu as MenuModel).get_item_links(item_index, out links);
		}

		public override int get_n_items () {
			return (_menu as MenuModel).get_n_items();
		}

		public override bool is_mutable () {
			return (_menu as MenuModel).is_mutable();
		}

		public override GLib.MenuAttributeIter iterate_item_attributes (int item_index) {
			return (_menu as MenuModel).iterate_item_attributes(item_index);
		}

		public override GLib.MenuLinkIter iterate_item_links (int item_index) {
			return (_menu as MenuModel).iterate_item_links(item_index);
		}


	}

} // namespace

