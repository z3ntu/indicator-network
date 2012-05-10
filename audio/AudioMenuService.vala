// vim: tabstop=4 noexpandtab shiftwidth=4 softtabstop=4
using PulseAudio;

namespace Unity.Settings
{
	public class AudioMenu : Object
	{
		private PulseAudio.GLibMainLoop loop;
		private PulseAudio.Context context;

		public AudioMenu ()
		{
			loop = new PulseAudio.GLibMainLoop();
			context = new PulseAudio.Context(loop.get_api(), null);

			if (context.connect(null, Context.Flags.NOFAIL, null) < 0)
				print( "pa_context_connect() failed: %s\n", PulseAudio.strerror(context.errno()));

		}
		public static int main (string[] args)
		{
			var loop = new GLib.MainLoop ();
			var menu = new AudioMenu ();
			loop.run();
			return 0;
		}
	}
}
