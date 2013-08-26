
namespace Network.Settings
{
	public class Airplane : Base {
		public Airplane (GLibLocal.ActionMuxer muxer) {
			GLib.Object(
				namespace: "airplane",
				muxer: muxer
			);

			var enabled = new SimpleAction.stateful("enabled", null, new Variant.boolean(false));
			enabled.activate.connect((value) => {
				try {
					GLib.Process.spawn_command_line_async("notify-send \"Airplane Mode is waiting on Foundations\"");
				} catch (Error e) {
					warning(@"Unable to send notification: $(e.message)");
				}
			});
			actions.add_action(enabled);

			var item = new MenuItem(_("Flight Mode"), "indicator.airplane.enabled");
			item.set_attribute("x-canonical-type", "s", "com.canonical.indicator.switch");
			_menu.append_item(item);
		}
	}
}
