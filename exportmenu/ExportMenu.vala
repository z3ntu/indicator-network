// vim: tabstop=4 noexpandtab shiftwidth=4 softtabstop=4
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
