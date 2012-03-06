#include <gtk/gtk.h>
#include "secret-agent.h"

void
destroy_secret (GValue *value)
{
  if (g_value_get_gtype (value) == G_TYPE_STRING)
      g_free ((char*)g_value_get_string (value));

  g_free (value);
}

void
secret_requested_cb (UnitySettingsSecretAgent      *self,
                     guint64                        id,
                     NMConnection                  *connection,
                     const char                    *setting_name,
                     const char                   **hints,
                     NMSecretAgentGetSecretsFlags   flags,
                     GtkWidget                     *dialog)
{
  GtkWidget *entry;
  /* The GetSecrets NM call expects a{sa{sv}}
   * the outer hash table stores secrets per settings keys
   * and the inner one the secrets themselves
   */
  GValue    *value;
  GHashTable *settings;
  GHashTable *secrets;

  gint response = gtk_dialog_run (GTK_DIALOG (dialog));
  gtk_widget_hide (dialog);
  entry = gtk_bin_get_child (GTK_BIN (gtk_dialog_get_content_area (GTK_DIALOG (dialog))));

  if (response != GTK_RESPONSE_ACCEPT)
    {
      unity_settings_secret_agent_cancel_request (self, id);
      return;
    }

  settings = g_hash_table_new_full (g_str_hash,
                                    g_str_equal,
                                    g_free,
                                    (GDestroyNotify) g_hash_table_destroy);
  secrets  = g_hash_table_new_full (g_str_hash,
                                    g_str_equal,
                                    g_free,
                                    (GDestroyNotify) destroy_secret);

  value = (GValue*) g_slice_alloc0 (sizeof (GValue));
  g_value_set_string (value, gtk_entry_get_text (GTK_ENTRY (entry)));

  g_hash_table_insert (secrets, "setting-key", value);
  g_hash_table_insert (settings, (gpointer)setting_name, secrets);

  g_debug ("Secret requested <%d> %s", (int)id, setting_name);

  unity_settings_secret_agent_provide_secret (self, id, settings);

//  g_hash_table_destroy (settings);
}

void
request_cancelled_cb (UnitySettingsSecretAgent      *self,
                      guint64                        id,
                      gpointer                       data)
{
  g_debug ("Request cancelled <%d>", (int)id);
}

gint
main (gint argc, gchar** argv)
{
  GtkWidget                *dialog;
  GtkWidget                *entry;
  GMainLoop                *loop;
  UnitySettingsSecretAgent *agent;

  gtk_init (0, NULL);

  dialog = gtk_dialog_new_with_buttons ("My dialog",
                                        NULL,
                                        GTK_DIALOG_MODAL,
                                        GTK_STOCK_OK,
                                        GTK_RESPONSE_ACCEPT,
                                        GTK_STOCK_CANCEL,
                                        GTK_RESPONSE_REJECT,
                                        NULL);

  entry = gtk_entry_new ();
  gtk_container_add (GTK_CONTAINER (gtk_dialog_get_content_area (GTK_DIALOG (dialog))),
                     entry);
  gtk_widget_show (entry);

  agent = unity_settings_secret_agent_new ();
  nm_secret_agent_register (NM_SECRET_AGENT (agent));

  g_signal_connect (G_OBJECT (agent),
                    UNITY_SETTINGS_SECRET_AGENT_SECRET_REQUESTED,
                    G_CALLBACK (secret_requested_cb),
                    dialog);

  g_signal_connect (G_OBJECT (agent),
                    UNITY_SETTINGS_SECRET_AGENT_REQUEST_CANCELLED,
                    G_CALLBACK (secret_requested_cb),
                    dialog);

  loop = g_main_loop_new (NULL, FALSE);
  g_main_loop_run (loop);
  g_main_loop_unref (loop);

  g_object_unref (agent);
  g_object_unref (dialog);
  return 0;
}
