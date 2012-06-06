// vim: tabstop=4 noexpandtab shiftwidth=4 softtabstop=4
using Unity.Settings;

public static int main (string[] args)
{
	var menu = new SoundMenu ();
	menu.hold ();

	return menu.run (args);
}
