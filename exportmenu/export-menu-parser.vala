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

using GLib;

namespace Unity.Settings {
	public class Parser : Object {
		private MarkupParser parser;

		private Settings? settings = null;
		private Group?    current_group = null;
		private Key?      current_key = null;

		public signal void parsed (Settings settings);

		public Parser () {
			parser.start_element = start_element_fn;
			parser.end_element   = end_element_fn;
			parser.text          = text_element_fn;
		}

		public async void parse (File document) {
			try {
				var stream = document.read();
				parse_input_stream (stream);
			} catch (Error e) {
				error (e.message);
			}
		}

		public async void parse_input_stream (InputStream stream) throws GLib.MarkupError
		{
			var ctx = new MarkupParseContext (parser, 0, this, null);

			uint8[] buffer = new uint8[1024];
			ssize_t size = 0;

			while ((size = yield stream.read_async (buffer)) != 0) {
					ctx.parse ((string)buffer, size);
			}

			current_group = null;
			current_key = null;

			parsed (settings);
		}

		private static void start_element_fn (MarkupParseContext   ctx,
			                                   string              element_name,
			                                   string[]            attrs_names,
			                                   string[]            attrs_values) throws MarkupError {
			Parser self     = (Parser)ctx.get_user_data ();
			var elements = ctx.get_element_stack().copy();

			/* Root element */
			if (element_name == "settings" && self.settings == null) {
				self.settings = new Settings ();
				debug ("Added settings object");
			}	else if (element_name == "group") {
				var group = new Group ();

				/* If a settings element is the parent then we append the new group to the parents */
				if (elements.nth_data (1) == "settings") {
					self.settings.groups.append (group);
					self.current_group = group;
				}	else if (elements.nth_data (1) == "group") {
					group.parent = self.current_group;
					group.parent.groups.append (group);
					self.current_group = group;
				}

				group.populate_group (attrs_names, attrs_values);
				debug("Added group object");

			} else if (element_name == "key") {
				self.current_key = new Key(self.current_group);
				self.current_group.keys.append(self.current_key);

				self.current_key.populate_key (attrs_names, attrs_values);

				debug("Added key object");
			}
		}

		private static void text_element_fn (MarkupParseContext ctx,
		                                     string             text,
		                                     size_t             size) throws MarkupError {
			Parser self = (Parser)ctx.get_user_data ();
			var element = ctx.get_element_stack().nth_data(0);
			var parent  = ctx.get_element_stack().nth_data(1);

			if (element == "display_name")
			{
				if (parent == "group" && self.current_group != null)
					self.current_group.display_name = text;
				else if (parent == "key" && self.current_key != null)
					self.current_key.display_name = text;
			}
		}

		private static void end_element_fn (MarkupParseContext ctx,
	                                  string             element_name) throws MarkupError {
			Parser self = (Parser)ctx.get_user_data ();
			if (element_name == "group")
				self.current_group = self.current_group.parent;
			if (element_name == "key")
				self.current_key = null;
		}
	}
}
