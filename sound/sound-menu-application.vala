// vim: tabstop=4 noexpandtab shiftwidth=4 softtabstop=4
using PulseAudio;


namespace Unity.Settings
{
	[CCode(cname="pa_cvolume_set", cheader_filename = "pulse/volume.h")]
	extern unowned PulseAudio.CVolume? vol_set (PulseAudio.CVolume? cv, uint channels, PulseAudio.Volume v);

	public class VolumeControl : Object
	{
		private PulseAudio.GLibMainLoop loop;
		private PulseAudio.Context context;
		private bool   _mute = true;
		private double _volume = 0.0;

		public signal void ready ();
		public signal void mute_toggled (bool mute);
		public signal void volume_changed (double v);

		public VolumeControl ()
		{
			loop = new PulseAudio.GLibMainLoop ();

			var props = new Proplist ();
			props.sets (Proplist.PROP_APPLICATION_NAME, "Ubuntu Audio Settings");
			props.sets (Proplist.PROP_APPLICATION_ID, "com.canonical.settings.sound");
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

		/* PulseAudio logic*/
		private void context_events_cb (Context c, Context.SubscriptionEventType t, uint32 index)
		{
			if ((t & Context.SubscriptionEventType.FACILITY_MASK) == Context.SubscriptionEventType.SINK)
			{
				get_properties ();
			}
		}

		private void sink_info_cb_for_props (Context c, SinkInfo? i, int eol)
		{
			if (i == null)
				return;

			if (_mute != (bool)i.mute)
			{
				_mute = (bool)i.mute;
				mute_toggled (_mute);
			}

			if (_volume != volume_to_double (i.volume.values[0]))
			{
				_volume = volume_to_double (i.volume.values[0]);
				volume_changed (_volume);
			}
		}

		private void server_info_cb_for_props (Context c, ServerInfo? i)
		{
			if (i == null)
				return;
			context.get_sink_info_by_name (i.default_sink_name, sink_info_cb_for_props);
		}

		private void get_properties ()
		{
			context.get_server_info (server_info_cb_for_props);
		}

		private void notify_cb (Context c)
		{
			if (c.get_state () == Context.State.READY)
			{
				c.subscribe (PulseAudio.Context.SubscriptionMask.SINK);
				c.set_subscribe_callback (context_events_cb);
				get_properties ();
				ready ();
			}
		}

		/* Mute operations */
		private void toggle_mute_success (Context c, int success)
		{
			if ((bool)success)
				mute_toggled (_mute);
		}

		private void toggle_mute_cb (Context c, SinkInfo? i, int eol)
		{
			if (i == null)
				return;

			_mute = ! (bool) i.mute;
			c.set_sink_mute_by_index (i.index, _mute, toggle_mute_success);
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

		public bool is_muted ()
		{
			return _mute;
		}

		/* Volume operations */
		private static PulseAudio.Volume double_to_volume (double vol)
		{
			double tmp = (double)(PulseAudio.Volume.NORM - PulseAudio.Volume.MUTED) * vol;
			return (PulseAudio.Volume)tmp + PulseAudio.Volume.MUTED;
		}

		private static double volume_to_double (PulseAudio.Volume vol)
		{
			double tmp = (double)(vol - PulseAudio.Volume.MUTED);
			return tmp / (double)(PulseAudio.Volume.NORM - PulseAudio.Volume.MUTED);
		}

		private void set_volume_success_cb (Context c, int success)
		{
			if ((bool)success)
				volume_changed (_volume);
		}

		private void sink_info_set_volume_cb (Context c, SinkInfo? i, int eol)
		{
			if (i == null)
				return;

			unowned CVolume cvol = vol_set (i.volume, 1, double_to_volume (_volume));
			c.set_sink_volume_by_index (i.index, cvol, set_volume_success_cb);
		}

		private void server_info_cb_for_set_volume (Context c, ServerInfo? i)
		{
			if (i == null)
			{
				warning ("Could not get PulseAudio server info");
				return;
			}

			context.get_sink_info_by_name (i.default_sink_name, sink_info_set_volume_cb);
		}

		public void set_volume (double volume)
		{
			if (context.get_state () != Context.State.READY)
			{
				warning ("Could not change volume: PulseAudio server connection is not ready.");
				return;
			}
			_volume = volume;

			context.get_server_info (server_info_cb_for_set_volume);
		}

		public double get_volume ()
		{
			return _volume;
		}
	}


	public class SoundMenu : Application
	{
		private DBusConnection conn;
		private GLib.Menu gmenu;
		private VolumeControl ac;
		private bool _ready = false;
		private SimpleAction mute_action;
		private SimpleAction volume_action;

		public SoundMenu ()
		{
			Object (application_id: "com.canonical.settings.sound");
			flags = ApplicationFlags.IS_SERVICE;

			gmenu = new Menu ();
			//TODO: Port to GMenuMap

			ac = new VolumeControl ();
			ac.ready.connect (ready_cb);
			ac.volume_changed.connect (volume_changed_cb);
			ac.mute_toggled.connect (mute_toggled_cb);
		}

		private void state_changed_cb (ActionGroup action_group, string name, Variant val)
		{
			if (name == "volume")
			{
				ac.set_volume (val.get_double ());
			}

			if (name == "mute")
			{
				if (val.get_boolean () == ac.is_muted ())
					return;

				ac.switch_mute ();
			}
		}

		private void bootstrap_actions ()
		{
			mute_action =   new SimpleAction.stateful ("mute", new VariantType("b"),  new Variant.boolean(true));
			volume_action = new SimpleAction.stateful ("volume", new VariantType("d"), new Variant.double(0.0));

			add_action (mute_action);
			add_action (volume_action);

			action_state_changed.connect (state_changed_cb);
		}

		private void bootstrap_menu ()
		{
			var mute_control = new MenuItem (null, null);
			mute_control.set_attribute ("type",                     "s", "x-canonical-system-settings");
			mute_control.set_attribute (GLib.Menu.ATTRIBUTE_LABEL,  "s", "Mute");
			mute_control.set_attribute ("x-canonical-widget-type",  "s", "unity.widgets.systemsettings.tablet.switch");
			mute_control.set_attribute (GLib.Menu.ATTRIBUTE_ACTION, "s", "mute");
			gmenu.append_item (mute_control);

			var volume_control = new MenuItem (null, null);
			volume_control.set_attribute ("type",                     "s", "x-canonical-system-settings");
			volume_control.set_attribute ("x-canonical-widget-type",  "s", "unity.widgets.systemsettings.tablet.volumecontrol");
			volume_control.set_attribute (GLib.Menu.ATTRIBUTE_ACTION, "s", "volume");

			gmenu.append_item (volume_control);
		}

		private void ready_cb ()
		{
			if (_ready)
				return;

			_ready = true;

			bootstrap_actions ();
			bootstrap_menu ();

			try
			{
				conn = Bus.get_sync (BusType.SESSION, null);
			}
			catch (IOError e)
			{
				warning ("Could not connect to the session bus");
				return;
			}

			try
			{
				conn.export_menu_model   ("/com/canonical/settings/sound/phone", gmenu);
			}
			catch (GLib.Error e)
			{
				warning ("Menu model and/or action group could not be exported.");
				return;
			}

		}

		private void volume_changed_cb (double vol)
		{
			volume_action.set_state (new Variant.double(ac.get_volume ()));
		}

		private void mute_toggled_cb (bool mute)
		{
			mute_action.set_state (new Variant.boolean(ac.is_muted()));
		}
	}
}
