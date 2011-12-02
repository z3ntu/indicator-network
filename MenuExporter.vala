using Dbusmenu;

namespace Unity.SettingsMenu {
	class MenuExporter {
		private Unity.SettingsMenu.Settings settings;
		private Dbusmenu.Server server = null;
		
		private int id;

		public MenuExporter (Settings settings) {
			this.settings = settings;
		}
				
		private void property_changed_cb (Dbusmenu.Menuitem item, string property, GLib.Variant variant) {
						debug ("property-changed: %s", property);
		}

		private void export_menus (Menuitem parent, Group g) {
			foreach (Key k in g.keys) {
				var item = new Menuitem.with_id (id);
				item.property_changed .connect (property_changed_cb);
				
				item.property_set ("label", k.display_name);
				parent.child_append (item);
				id++;
				
				if (k.type == "b") {
					item.property_set ("toggle-type", Dbusmenu.MENUITEM_TOGGLE_CHECK);
					item.property_set_int ("toggle-state", Dbusmenu.MENUITEM_TOGGLE_STATE_CHECKED);
				}
				
				item.property_set ("x-gsettings-schema", k.parent.id);
				item.property_set ("x-gsettings-name",   k.name);

				var gset = new GLib.Settings (k.parent.id);
				
				if (gset.get_boolean (k.name))
					debug ("true");
				else
					debug ("false");
			}
			
			//subgroups
			foreach (Group sg in g.groups) {
				var item = new Menuitem.with_id (id);
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
