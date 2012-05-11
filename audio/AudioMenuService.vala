// vim: tabstop=4 noexpandtab shiftwidth=4 softtabstop=4
using PulseAudio;

namespace Unity.Settings
{
	public class AudioMenu : Application
	{
		private PulseAudio.GLibMainLoop loop;
		private PulseAudio.Context context;
		private GLib.Menu menu;
		private GLib.SimpleActionGroup ag;

		public void activate_cb (Application app)
		{
			stdout.printf("asdasdasd");

			var loop = new GLib.MainLoop ();
			loop.run ();
		}

		public AudioMenu () throws IOError //FIXME: Throws Error as well
		{
			Object (application_id: "com.ubuntu.audiosettings");
			DBusConnection conn;

			loop = new PulseAudio.GLibMainLoop();
			flags = ApplicationFlags.IS_SERVICE;

			context = new PulseAudio.Context(loop.get_api(), null);

			if (context.connect(null, Context.Flags.NOFAIL, null) < 0)
			{
				print( "pa_context_connect() failed: %s\n", PulseAudio.strerror(context.errno()));
			}

			conn = Bus.get_sync (BusType.SESSION, null);
			menu = new Menu ();
			ag   = new SimpleActionGroup ();

			conn.export_menu_model ("/com/ubuntu/audiosettings", menu);
			conn.export_action_group ("/com/ubuntu/audiosettings/actions", ag);

			activate.connect(activate_cb);
		}

		public static int main (string[] args)
		{
			var menu = new AudioMenu ();
			return menu.run ();
		}
	}
}
