// vim: tabstop=4 noexpandtab shiftwidth=4 softtabstop=4
using PulseAudio;

namespace Unity.Settings
{
	public class AudioMenu : Application
	{
		private PulseAudio.GLibMainLoop loop;
		private PulseAudio.Context context;
		private DBusConnection conn;
		private GLib.Menu menu;
		private GLib.SimpleActionGroup ag;

		private signal void ready ();

		private void
		notify_cb (Context c)
		{
			if (c.get_state () == Context.State.READY)
			{
				c.set_sink_mute_by_index (0, true, null);
			}
		}

		public void init_pa ()
		{
			loop = new PulseAudio.GLibMainLoop ();

			var props = new Proplist ();
			props.sets (Proplist.PROP_APPLICATION_NAME, "Ubuntu Audio Settings");
			props.sets (Proplist.PROP_APPLICATION_ID, "com.ubuntu.audiosettings");
			props.sets (Proplist.PROP_APPLICATION_ICON_NAME, "multimedia-volume-control");
			props.sets (Proplist.PROP_APPLICATION_VERSION, "0.1");

			context = new PulseAudio.Context (loop.get_api(), null, props);

			context.set_state_callback (notify_cb);

			if (context.connect(null, Context.Flags.NOFAIL, null) < 0)
			{
				warning( "pa_context_connect() failed: %s\n", PulseAudio.strerror(context.errno()));
				return;
			}
		}

		public AudioMenu ()
		{
			Object (application_id: "com.ubuntu.audiosettings");
			flags = ApplicationFlags.IS_SERVICE;

			try
			{
				conn = Bus.get_sync (BusType.SESSION, null);
			}
			catch (IOError e)
			{
				return;
			}

			menu = new Menu ();
			ag   = new SimpleActionGroup ();

			try
			{
				conn.export_menu_model ("/com/ubuntu/audiosettings", menu);
				conn.export_action_group ("/com/ubuntu/audiosettings/actions", ag);
			}
			catch (GLib.Error e)
			{
				warning ("Menu model and/or action group could not be exported.");
				return;
			}

			init_pa();
		}

		public static int main (string[] args)
		{
			var menu = new AudioMenu ();
			menu.hold ();

			return menu.run ();
		}
	}
}
