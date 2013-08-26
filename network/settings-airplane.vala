// vim: tabstop=4 noexpandtab shiftwidth=4 softtabstop=4
/*
 * Copyright 2013 Canonical Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors:
 *      Ted Gould <ted.gould@canonical.com>
 */

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
