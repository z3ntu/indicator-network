using GLib;

Unity.Settings.Parser _global_parser;

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
			
			_global_parser.current_group = null;
			_global_parser.current_key = null;
			
			
			parsed (settings);
		}

		private static void start_element_fn (MarkupParseContext ctx,
			                                   string              element_name,
			                                   string[]            attrs_names,
			                                   string[]            attrs_values) throws MarkupError {
			var elements = ctx.get_element_stack().copy();
		
			/* Root element */
			if (element_name == "settings" && _global_parser.settings == null) {
				_global_parser.settings = new Settings ();
				debug ("Added settings object");
			}	else if (element_name == "group") {
				var group = new Group ();

				/* If a settings element is the parent then we append the new group to the parents */
				if (elements.nth_data (1) == "settings") {
					_global_parser.settings.groups.append (group);
					_global_parser.current_group = group;
				}	else if (elements.nth_data (1) == "group") {
					group.parent = _global_parser.current_group;
					group.parent.groups.append (group);
					_global_parser.current_group = group;
				}
			
				group.populate_group (attrs_names, attrs_values);
				debug("Added group object");
			
			} else if (element_name == "key") {
				_global_parser.current_key = new Key(_global_parser.current_group);
				_global_parser.current_group.keys.append(_global_parser.current_key);
				
				_global_parser.current_key.populate_key (attrs_names, attrs_values);

				debug("Added key object");
			}
		}

		private static void text_element_fn (MarkupParseContext ctx,
		                                     string             text,
		                                     size_t             size) throws MarkupError {
			var element = ctx.get_element_stack().nth_data(0);
			var parent = ctx.get_element_stack().nth_data(1);
			
			if (element == "display_name")
			{
				if (parent == "group" && _global_parser.current_group != null)
					_global_parser.current_group.display_name = text;
				else if (parent == "key" && _global_parser.current_key != null)
					_global_parser.current_key.display_name = text;
					
			}
		}

		private static void end_element_fn (MarkupParseContext ctx,
			                                  string             element_name) throws MarkupError {
			if (element_name == "group")
				_global_parser.current_group = _global_parser.current_group.parent;
			if (element_name == "key")
				_global_parser.current_key = null;
		}

		public static int main (string[] args)	{
			if (args.length != 2)
			  return 1;

			var f = File.new_for_path (args[1]);

			if (!f.query_exists ()) {
				stderr.printf ("File '%s' doesn't exist.\n", f.get_path ());
				return 2;
			}

			var main_loop = new MainLoop();
		
			_global_parser = new Parser ();
			_global_parser.parse (f);
			
			_global_parser.parsed.connect ((settings) => {
					var menu = new MenuExporter (settings);
					menu.export ();					
				});
			
			main_loop.run ();
			return 0;
		}
	}
}
