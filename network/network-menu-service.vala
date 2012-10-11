// vim: tabstop=4 noexpandtab shiftwidth=4 softtabstop=4

public static int main (string[] args)
{
	var menu = new Unity.Settings.NetworkMenu ();
	menu.hold ();
	return menu.run (args);
}
