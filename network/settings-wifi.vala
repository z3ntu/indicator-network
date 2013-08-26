
namespace Network.Settings
{
	public class Wifi : Base {
		public Wifi (GLibLocal.ActionMuxer muxer) {
			GLib.Object(
				namespace: "wifi-settings",
				muxer: muxer
			);
		}
	}
}
