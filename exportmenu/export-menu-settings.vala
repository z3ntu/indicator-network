// vim: tabstop=4 noexpandtab shiftwidth=4 softtabstop=4

namespace Unity.Settings {
	public class Enum : Object {
		//HashTable
	}

	public class Key : Object {
		public string type = null;
//		public string val;
		public string name = null;
		public string display_name;
		public Group? parent = null;

		public Key (Group parent) {
			this.parent = parent;
		}

		public void populate_key (string[] attrs_names, string[] attrs_values) {
			if (attrs_names.length != attrs_values.length) {
				error("The amount of attribute names does not match with the amount of values.");
			}

			for (int i = 0; i < attrs_names.length; i++) {
				if (attrs_names[i] == "type")
					type = attrs_values[i];
				else if (attrs_names[i] == "name")
					name = attrs_values[i];
			}
		}
	}

	public enum GroupType {
		SUBMENU,
		INLINE
	}

	public class Group : Object {
		public List<Group> groups = null;
		public List<Key>   keys = null;
		public string    id;
		public string    path;
		public string    display_name;
		public GroupType type = GroupType.SUBMENU;

		public Group? parent = null;
		//TODO: HashTable for the attributes

		public Group () {
		}

                public void populate_group (string[] attrs_names, string[] attrs_values) {
			if (attrs_names.length != attrs_values.length) {
				error("The amount of attribute names does not match with the amount of values.");
			}

			for (int i = 0; i < attrs_names.length; i++) {
				if (attrs_names[i] == "id")
					id = attrs_values[i];
				else if (attrs_names[i] == "path")
					path = attrs_values[i];
			}
		}
	}

	public class Settings : Object {
		public List<Group> groups = null;
//		public List<Enum>  enums = null;

		public Settings () {
		}
	}
}
