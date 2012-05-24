// vim: tabstop=4 noexpandtab shiftwidth=4 softtabstop=4
using PulseAudio;

namespace Unity.Settings
{
	[CCode(cname="pa_cvolume_set", cheader_filename = "pulse/volume.h")]
	extern unowned PulseAudio.CVolume? vol_set (PulseAudio.CVolume? cv, uint channels, PulseAudio.Volume v);

	public class AudioMenu : Application
	{
		private PulseAudio.GLibMainLoop loop;
		private PulseAudio.Context context;
		private DBusConnection conn;
		private GLib.Menu gmenu;
		private GLib.SimpleActionGroup ag;

		//FIXME: This is here until user_data can be passed to callbacks
		private double _volume;

		private signal void ready ();
		private signal void mute_toggled (bool mute);
		private signal void volume_changed (PulseAudio.Volume v);


		private void
		notify_cb (Context c)
		{
			if (c.get_state () == Context.State.READY)
			{
				set_volume (0.333);
			}
		}

		private void
		toggle_mute_cb (Context c, SinkInfo? i, int eol)
		{
			if (i == null)
				return;

			bool mute = ! (bool) i.mute;
			c.set_sink_mute_by_index (i.index, mute, null);
		}

		private void
		set_volume_cb (Context c, SinkInfo? i, int eol)
		{
			if (i == null)
				return;

			bool mute = ! (bool) i.mute;
//			c.set_sink_mute_by_index (i.index, mute, null);
			double range = (double)(PulseAudio.Volume.NORM - PulseAudio.Volume.MUTED);
			PulseAudio.Volume vol = (PulseAudio.Volume)(range * _volume) + PulseAudio.Volume.MUTED;

			unowned CVolume cvol = vol_set (i.volume, 1, vol);
			c.set_sink_volume_by_index (i.index, cvol);
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

		public void set_volume (double volume)
		{
			if (context.get_state () != Context.State.READY)
			{
				warning ("Could not change volume: PulseAudio server connection is not ready.");
				return;
			}
			_volume = volume;

			context.get_sink_info_by_index (0, set_volume_cb);
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
				warning ("Could not connect to the session bus");
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
			var menu = new AudioMenu ();
			menu.hold ();

			return menu.run ();
		}
	}
}
