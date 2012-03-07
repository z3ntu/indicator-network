#include <glib.h>

static void
test_parser ()
{
  return;
}

static void
test_parser_suite ()
{
  g_test_add_func ("/chewie/exportmenu/parser", test_parser);
}

gint
main (gint argc, gchar** argv)
{
  g_type_init ();
  g_test_init (&argc, &argv, NULL);

  test_parser_suite ();

  return g_test_run ();
}
