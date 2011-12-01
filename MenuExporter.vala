using Dbusmenu;

namespace Unity.SettingsMenu {
	class MenuExporter {
		private Unity.SettingsMenu.Settings settings;
		private Dbusmenu.Server server = null;
		
		private int id;

		public MenuExporter (Settings settings) {
			this.settings = settings;
		}
		
		private void export_menus (Menuitem parent, Group g) {
			foreach (Key k in g.keys) {
				var item = new Menuitem.with_id (id);
				item.property_set ("label", k.display_name);
				parent.child_append (item);
				id++;
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
