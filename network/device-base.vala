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
		Menu _menu = new Menu();

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

