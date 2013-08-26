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

namespace Network.Settings
{
	public class Base : MenuModel {
		protected Menu _menu = new Menu();
		protected string _namespace = "no-namespace-here";
		protected GLib.SimpleActionGroup actions = new GLib.SimpleActionGroup();
		private GLibLocal.ActionMuxer _muxer;

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

		construct {
			_menu.items_changed.connect(menu_items_changed);
			_muxer.insert(_namespace, actions);
		}

		~Base ()
		{
			_muxer.remove(_namespace);
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
}
