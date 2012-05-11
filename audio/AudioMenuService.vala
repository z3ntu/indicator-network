// vim: tabstop=4 noexpandtab shiftwidth=4 softtabstop=4
using PulseAudio;

namespace Unity.Settings
{
	public class AudioMenu : Application
	{
		private PulseAudio.GLibMainLoop loop;
		private PulseAudio.Context context;
		private GLib.Menu menu;

		public AudioMenu ()
		{
			Object (application_id: "com.ubuntu.audiosettings");

			loop = new PulseAudio.GLibMainLoop();
			flags = ApplicationFlags.IS_SERVICE;

			context = new PulseAudio.Context(loop.get_api(), null);

			if (context.connect(null, Context.Flags.NOFAIL, null) < 0)
			{
				print( "pa_context_connect() failed: %s\n", PulseAudio.strerror(context.errno()));
			}

			menu = new Menu ();
		}

		public static int main (string[] args)
		{
			var menu = new AudioMenu ();
			menu.run ();
			return 0;
		}
	}
}
