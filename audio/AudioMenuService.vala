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

		public void activate_cb (Application app)
		{
			stdout.printf("asdasdasd");

			var loop = new GLib.MainLoop ();
			loop.run ();
		}

		public AudioMenu () throws IOError
		{
			Object (application_id: "com.ubuntu.audiosettings");
			flags = ApplicationFlags.IS_SERVICE;

			loop = new PulseAudio.GLibMainLoop();
			context = new PulseAudio.Context(loop.get_api(), null);

			if (context.connect(null, Context.Flags.NOFAIL, null) < 0)
			{
				print( "pa_context_connect() failed: %s\n", PulseAudio.strerror(context.errno()));
			}

			activate.connect(activate_cb);

			conn = Bus.get_sync (BusType.SESSION, null);
			menu = new Menu ();
			ag   = new SimpleActionGroup ();

			try
			{
				conn.export_menu_model ("/com/ubuntu/audiosettings", menu);
				conn.export_action_group ("/com/ubuntu/audiosettings/actions", ag);
			}
			catch (GLib.Error e)
			{
				return;
			}
		}

		public static int main (string[] args)
		{
			try
			{
				var menu = new AudioMenu ();
				menu.hold ();
				return menu.run ();
			}
			catch (IOError e)
			{
				return -1;
			}
		}
	}
}
