// vim: tabstop=4 noexpandtab shiftwidth=4 softtabstop=4
/*
 * Copyright 2013 Canonical Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
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
 *      Alberto Ruiz <alberto.ruiz@canonical.com>
 */

using GLib;

public static int main (string[] args)	{
	if (args.length != 2)
		return 1;

	var f = File.new_for_path (args[1]);

	if (!f.query_exists ()) {
		stderr.printf ("File '%s' doesn't exist.\n", f.get_path ());
		return 2;
	}

	var main_loop = new MainLoop();

	var parser = new Unity.Settings.Parser ();
	parser.parse (f);

	parser.parsed.connect ((settings) => {
		var menu = new Unity.Settings.MenuExporter (settings);
		menu.export ();
	});

	main_loop.run ();
	return 0;
}
