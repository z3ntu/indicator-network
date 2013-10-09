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
	public class Wifi : Base {
		private GLib.Settings settings;

		public Wifi (GLibLocal.ActionMuxer muxer) {
			GLib.Object(
				namespace: "wifi-settings",
				muxer: muxer
			);

			settings = new GLib.Settings ("com.canonical.indicator.network");

			var joinact = settings.create_action("auto-join-previous");
			actions.add_action(joinact);

			var promptact = settings.create_action("prompt-on-new-wifi-ap");
			actions.add_action(promptact);

			var joinitem = new MenuItem(_("Auto-join previous networks"), "indicator.wifi-settings.auto-join-previous");
			_menu.append_item(joinitem);

			var promptitem = new MenuItem(_("Prompt when not connected"), "indicator.wifi-settings.prompt-on-new-wifi-ap");
			/* Commented out for Phone V1 that doesn't have this feature */
			/* _menu.append_item(promptitem); */

			var captionitem = new MenuItem(_("Lists available wi-fi networks, if any, when you're using cellular data."), null);
			_menu.append_item(captionitem);
		}
	}
}
