
using GLib;

namespace oFono {

	[DBus (name = "org.ofono.Modem") ]
	public interface Modem : GLib.Object {

		[DBus (name = "SetProperty")]
		public abstract void set_property (string property, GLib.Variant value) throws IOError;

		[DBus (name = "GetProperties")]
		public abstract GLib.HashTable<string, GLib.Variant> get_properties () throws IOError;

		[DBus (name = "PropertyChanged")]
		public signal void property_changed (string property, GLib.Variant value);
	}

	[DBus (name = "org.ofono.NetworkRegistration") ]
	public interface NetworkRegistration : GLib.Object {

		[DBus (name = "GetProperties")]
		public abstract GLib.HashTable<string, GLib.Variant> get_properties () throws IOError;

		[DBus (name = "PropertyChanged")]
		public signal void property_changed (string property, GLib.Variant value);
	}

	[DBus (name = "org.ofono.SimManager") ]
	public interface SIMManager : GLib.Object {

		[DBus (name = "SetProperty")]
		public abstract void set_property (string property, GLib.Variant value) throws IOError;

		[DBus (name = "GetProperties")]
		public abstract GLib.HashTable<string, GLib.Variant> get_properties () throws IOError;

		[DBus (name = "PropertyChanged")]
		public signal void property_changed (string property, GLib.Variant value);

		[DBus (name = "ChangePin")]
		public abstract void change_pin(string type, string old_pin, string new_pin) throws DBusError, IOError;

		[DBus (name = "EnterPin")]
		public abstract void enter_pin(string type, string pin) throws DBusError, IOError;

		[DBus (name = "ResetPin")]
		public abstract void reset_pin(string type, string puk, string new_pin) throws DBusError, IOError;

		[DBus (name = "LockPin")]
		public abstract void lock_pin(string type, string pin) throws DBusError, IOError;

		[DBus (name = "UnlockPin")]
		public abstract void unlock_pin(string type, string pin) throws DBusError, IOError;
	}



}
