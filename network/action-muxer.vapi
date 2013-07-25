[CCode (cprefix = "G", lower_case_cprefix = "g_")]
namespace GLibLocal {
	[CCode (cheader_filename = "action-muxer.h", type_id = "G_TYPE_ACTION_MUXER")]
	public class ActionMuxer {
		public ActionMuxer ();
		public void insert (string prefix, GLib.ActionGroup group);
		public void remove (string prefix);
		public GLib.ActionGroup get (string prefix);
	}
}
