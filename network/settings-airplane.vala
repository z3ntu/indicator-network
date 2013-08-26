
namespace Network.Settings
{
	public class Airplane : Base {
		public Airplane (GLibLocal.ActionMuxer muxer) {
			GLib.Object(
				namespace: "wifi-settings",
				muxer: muxer
			);
		}
	}
}
