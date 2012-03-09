/* vim: set noet */

using GLib;

/* This global variable is due to a bug in the vala bindings for
 * the markup parser, get_user_data is missing.
 * see: https://bugzilla.gnome.org/show_bug.cgi?id=671749   */
Unity.Settings.Parser self = null;

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
                        self = this;

			var ctx = new MarkupParseContext (parser, 0, this, null);

			try {
				var stream = document.read();
				uint8[] buffer = new uint8[1024];
                                ssize_t size = 0;

				while ((size = yield stream.read_async (buffer)) != 0) {
					ctx.parse ((string)buffer, size);
				}
			} catch (Error e) {
				error (e.message);
			}

			self.current_group = null;
			self.current_key = null;


			parsed (settings);
		}

		private static void start_element_fn (MarkupParseContext ctx,
			                                   string              element_name,
			                                   string[]            attrs_names,
			                                   string[]            attrs_values) throws MarkupError {
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
			if (element_name == "group")
				self.current_group = self.current_group.parent;
			if (element_name == "key")
				self.current_key = null;
		}
	}
}
