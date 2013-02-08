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

#include <glib.h>
#include <gio/gio.h>
#include <libexportmenu.h>
#include <gtest/gtest.h>

#define TEST_DATA  "<settings> \
  <group id=\"com.ubuntu.test.chewie\" name=\"Chewie tests\"  tablet:icon=\"time.png\"> \
    <display_name>Display name</display_name> \
    <key type=\"s\" name=\"somestring\"></key> \
  </group> \
</settings"


static GMainLoop* mainloop;
static gboolean   name_owned;

static void
test_export_parsed_cb (UnitySettingsParser   *parser,
                       UnitySettingsSettings *settings,
                       gpointer               data)
{
  gboolean *result = (gboolean*)data;
  *result = TRUE;
  g_main_loop_quit(mainloop);
}

static gboolean
timeout_cb (gpointer data)
{
  gboolean *result = (gboolean*)data;
  *result = FALSE;
  g_main_loop_quit(mainloop);
  return FALSE;
}

static gboolean
test_export ()
{
  gboolean result = FALSE;
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
                    G_CALLBACK (test_export_parsed_cb), &result);
  g_timeout_add    (2000, timeout_cb, &result);

  g_main_loop_run (mainloop);

  g_main_loop_unref (mainloop);
  g_object_unref (stream);
  g_object_unref (parser);
  return result;
}

static gboolean
test_gsettings ()
{
  GSettings *settings = g_settings_new ("com.ubuntu.test.chewie");
  g_object_unref (settings);
  return TRUE;
}

TEST(Exportmenu, GSettings)
{
  EXPECT_TRUE(test_gsettings ());
}

TEST(Exportmenu, Parse)
{
  EXPECT_TRUE(test_export ());
}

gint
main (gint argc, gchar** argv)
{
  g_type_init ();
  ::testing::InitGoogleTest (&argc, argv);
  return RUN_ALL_TESTS();
}
