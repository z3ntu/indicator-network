#include <glib.h>
#include <gio/gio.h>
#include "unity-settings.h"


#define TEST_DATA  "<settings> \
  <group id=\"com.ubuntu.test.chewie\" name=\"Chewie tests\"  tablet:icon=\"time.png\"> \
    <display_name>Display name</display_name> \
    <key type=\"s\" name=\"somestring\"></key> \
  </group> \
</settings"


static GMainLoop* mainloop;

static void
test_parsing_parsed_cb (UnitySettingsParser   *parser,
                        UnitySettingsSettings *settings,
                        gpointer               data)
{
  UnitySettingsGroup *group;
  UnitySettingsKey   *key;
  /* number of groups */
  g_assert (g_list_length (settings->groups) == 1);

  /* id property */
  group = (UnitySettingsGroup*) settings->groups->data;
  g_assert (g_strcmp0 (group->id, "com.ubuntu.test.chewie") == 0);

  /* no subgroups */
  g_assert (g_list_length (group->groups) == 0);

  /* just one key */
  g_assert (g_list_length (group->keys) == 1);

  /* key properties */
  key = (UnitySettingsKey*) group->keys->data;
  g_assert (g_strcmp0 (key->type, "s")          == 0);
  g_assert (g_strcmp0 (key->name, "somestring") == 0);
  g_main_loop_quit(mainloop);
}

gboolean
test_parsing_timeout (gpointer data)
{
  g_warning ("Parsing timed out");
  g_assert_not_reached ();
}

static void
test_parsing ()
{
  UnitySettingsParser *parser = unity_settings_parser_new ();

  GInputStream *stream = g_memory_input_stream_new_from_data (TEST_DATA,
                                                              strlen (TEST_DATA) + 1,
                                                              NULL);
  mainloop = g_main_loop_new (NULL, FALSE);
  unity_settings_parser_parse_input_stream (parser,
                                            stream,
                                            NULL,
                                            NULL);

  g_signal_connect (parser, "parsed",
                    G_CALLBACK (test_parsing_parsed_cb), NULL);
  g_timeout_add (1500, test_parsing_timeout, NULL);
  g_main_loop_run (mainloop);

  g_main_loop_unref (mainloop);
  g_object_unref (stream);
  g_object_unref (parser);
}

static void
test_gsettings ()
{
  GSettings *settings = g_settings_new ("com.ubuntu.test.chewie");
  g_object_unref (settings);
  return;
}

static void
test_parser_suite ()
{
  g_debug (g_get_current_dir ());
  g_test_add_func ("/chewie/exportmenu/gsettings", test_gsettings);
  g_test_add_func ("/chewie/exportmenu/parsing",   test_parsing);
}

gint
main (gint argc, gchar** argv)
{
  g_type_init ();
  g_test_init (&argc, &argv, NULL);

  test_parser_suite ();

  return g_test_run ();
}
