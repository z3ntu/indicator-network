#include <glib.h>
#include <gio/gio.h>

static void
test_settings ()
{
  GSettings *settings = g_settings_new ("com.ubuntu.test.chewie");
  g_object_unref (settings);
  return;
}

static void
test_parser_suite ()
{
  g_test_add_func ("/chewie/exportmenu/settings", test_settings);
}

gint
main (gint argc, gchar** argv)
{
  g_type_init ();
  g_test_init (&argc, &argv, NULL);

  test_parser_suite ();

  return g_test_run ();
}
