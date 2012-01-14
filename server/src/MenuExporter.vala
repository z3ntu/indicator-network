using Dbusmenu;

namespace Unity.Settings {
	public class MenuExporter {
		private Unity.Settings.Settings settings;
		private Dbusmenu.Server server = null;
		private int id;

		public MenuExporter (Settings settings) {
			this.settings = settings;
		}

		private bool menu_item_event_cb (Dbusmenu.Menuitem item, string name, GLib.Variant variant, uint timestamp) {
			if (name == "x-text-changed") {
				var gset = new GLib.Settings (item.property_get("x-gsettings-schema"));
				string val = variant.get_string ();
				gset.set_string(item.property_get("x-gsettings-name"), val);

				item.property_set_variant("x-text", variant);
			}
			return true;
		}

		private void checkbox_item_activated_cb (Dbusmenu.Menuitem item, uint timestamp) {
					int state;

					string schema = item.property_get("x-gsettings-schema");
					string key_name = item.property_get("x-gsettings-name");

					if (schema == null || key_name == null)
						return;

					var gset = new GLib.Settings (schema);
					bool val = gset.get_boolean(key_name);

					if (val)
						state = Dbusmenu.MENUITEM_TOGGLE_STATE_UNCHECKED;
					else
						state = Dbusmenu.MENUITEM_TOGGLE_STATE_CHECKED;

					item.property_set_int ("toggle-state", state);

					gset.set_boolean(key_name, !val);
		}

		private void export_menus (Menuitem parent, Group g) {
			foreach (Key k in g.keys) {
				var item = new Menuitem.with_id (id);

				item.property_set ("type", "x-system-settings");
				item.property_set ("label", k.display_name);
				parent.child_append (item);
				id++;

				/* TODO: Subscribe to setting changes and notify them */
				item.property_set ("x-gsettings-type", k.type);
				item.property_set ("x-gsettings-schema", k.parent.id);
				item.property_set ("x-gsettings-name",   k.name);

				if (k.type == "b") {
					var gset = new GLib.Settings (k.parent.id);

					item.property_set ("toggle-type", Dbusmenu.MENUITEM_TOGGLE_CHECK);

					if (gset.get_boolean (k.name))
						item.property_set_int ("toggle-state", Dbusmenu.MENUITEM_TOGGLE_STATE_CHECKED);
					else
						item.property_set_int ("toggle-state", Dbusmenu.MENUITEM_TOGGLE_STATE_UNCHECKED);

					item.item_activated.connect(checkbox_item_activated_cb);

					item.property_set("x-tablet-widget", "unity.widgets.systemsettings.tablet.togglebutton");
				}
				else if (k.type == "s") {
					var gset = new GLib.Settings (k.parent.id);
					item.property_set("x-text", gset.get_string(k.name));

					item.property_set("x-tablet-widget", "unity.widgets.systemsettings.tablet.textentry");
					item.event.connect(menu_item_event_cb);
				}
      	/* List of strings/ints??? hopefully not! */
			}

			foreach (Group sg in g.groups) {
				var item = new Menuitem.with_id (id);
				item.property_set ("type", "standard");
				item.property_set ("label", sg.display_name);
				parent.child_append (item);

				id++;
				export_menus (item, sg);
			}
		}

		private void on_bus (DBusConnection conn, string name) {
			id = 0;

			server = new Dbusmenu.Server("/org/test");
			var root_item = new Menuitem.with_id (id);
			id++;

			foreach (Group g in settings.groups) {
				var item = new Menuitem.with_id (id);
				item.property_set ("label", g.display_name);
				root_item.child_append (item);

				id++;
				export_menus (item, g);
			}

			server.set_root (root_item);
		}

		public void export () {
			    Bus.own_name (BusType.SESSION, "org.dbusmenu.test", BusNameOwnerFlags.NONE,
                  on_bus,
                  () => {},
                  () => stderr.printf ("Could not aquire name\n"));
		}
	}
}
