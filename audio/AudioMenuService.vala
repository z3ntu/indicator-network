// vim: tabstop=4 noexpandtab shiftwidth=4 softtabstop=4
using PulseAudio;

private Unity.Settings.AudioMenu menu;

namespace Unity.Settings
{
	public class AudioMenu : Application
	{
		private PulseAudio.GLibMainLoop loop;
		private PulseAudio.Context context;
		private DBusConnection conn;
		private GLib.Menu gmenu;
		private GLib.SimpleActionGroup ag;

		private signal void ready ();

		private static void
		notify_cb (Context c)
		{
			if (c.get_state () == Context.State.READY)
			{
				menu.switch_mute ();
			}
		}

		private static void
		toggle_mute_cb (Context c, SinkInfo? i, int eol)
		{
			if (i == null)
				return;

			bool mute = ! (bool) i.mute;
			c.set_sink_mute_by_index (i.index, mute, null);
		}

		public void switch_mute ()
		{
			if (context.get_state () != Context.State.READY)
			{
				warning ("Could not mute: PulseAudio server connection is not ready.");
				return;
			}

			context.get_sink_info_by_index (0, toggle_mute_cb);
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

			gmenu = new Menu ();
			ag   = new SimpleActionGroup ();

			try
			{
				conn.export_menu_model ("/com/ubuntu/audiosettings", gmenu);
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
			menu = new AudioMenu ();
			menu.hold ();

			return menu.run ();
		}
	}
}
