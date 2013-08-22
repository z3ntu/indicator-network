
using GLib;

namespace oFono {

	[DBus (name = "org.ofono.Modem") ]
	public interface Modem : GLib.Object {
		public abstract void set_property (string property, GLib.Variant value) throws IOError;
		public abstract GLib.HashTable<string, GLib.Variant> get_properties () throws IOError;
		public signal void property_changed (string property, GLib.Variant value);
	}

	[DBus (name = "org.ofono.NetworkRegistration") ]
	public interface NetworkRegistration : GLib.Object {
		public abstract GLib.HashTable<string, GLib.Variant> get_properties () throws IOError;

		public signal void property_changed (string property, GLib.Variant value);
	}

	[DBus (name = "org.ofono.SIMManager") ]
	public interface SIMManager : GLib.Object {
		public abstract void set_property (string property, GLib.Variant value) throws IOError;
		public abstract GLib.HashTable<string, GLib.Variant> get_properties () throws IOError;
		public signal void property_changed (string property, GLib.Variant value);
	}



}
