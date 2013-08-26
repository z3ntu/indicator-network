
namespace Network.Settings
{
	public class Wifi : Base {
		public Wifi (GLibLocal.ActionMuxer muxer) {
			GLib.Object(
				namespace: "wifi-settings",
				muxer: muxer
			);

			var joinact = new SimpleAction.stateful("auto-join", null, new Variant.boolean(true));
			actions.add_action(joinact);

			var promptact = new SimpleAction.stateful("prompt", null, new Variant.boolean(true));
			actions.add_action(promptact);

			var joinitem = new MenuItem(_("Auto-join previous networks"), "indicator.wifi-settings.auto-join");
			_menu.append_item(joinitem);

			var promptitem = new MenuItem(_("Prompt when not connected"), "indicator.wifi-settings.prompt");
			_menu.append_item(promptitem);

			var captionitem = new MenuItem(_("Lists available wi-fi networks, if any, when you're using cellular data."), null);
			_menu.append_item(captionitem);
		}
	}
}
