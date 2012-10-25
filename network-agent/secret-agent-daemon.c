#include <gtk/gtk.h>
#include "secret-agent.h"

struct SecretData
{
  UnitySettingsSecretAgent  *agent;
  NMConnection *connection;
  guint64 id;
};

void secret_dialog_response_cb (GtkDialog *dialog,
                                gint response_id,
                                struct SecretData *data)
{
  GtkWidget *entry;
  GHashTable *settings;

  gtk_widget_hide (GTK_WIDGET (dialog));
  entry = gtk_container_get_children ((GTK_CONTAINER (gtk_dialog_get_content_area (GTK_DIALOG (dialog)))))->data;

  if (response_id != GTK_RESPONSE_ACCEPT)
    {
      unity_settings_secret_agent_cancel_request (data->agent, data->id);
      return;
    }

  NMSettingWirelessSecurity *wisec = nm_connection_get_setting_wireless_security (data->connection);
  const gchar *key_mgmt = nm_setting_wireless_security_get_key_mgmt (wisec);

  if (!g_strcmp0 (key_mgmt, "wpa-none") || !g_strcmp0 (key_mgmt, "wpa-psk"))
    {
      g_object_set (G_OBJECT (wisec),
                NM_SETTING_WIRELESS_SECURITY_PSK, gtk_entry_get_text (GTK_ENTRY (entry)),
                NULL);
    }
  else if (!g_strcmp0 (key_mgmt, "none"))
    {
      g_object_set (G_OBJECT (wisec),
                NM_SETTING_WIRELESS_SECURITY_WEP_KEY0, gtk_entry_get_text (GTK_ENTRY (entry)),
                NULL);
    }
  settings = nm_connection_to_hash (data->connection, NM_SETTING_HASH_FLAG_ALL);

  unity_settings_secret_agent_provide_secret (data->agent, data->id, settings);
  g_object_unref (data->connection);
  g_hash_table_unref (settings);
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
  /* The GetSecrets NM call expects a{sa{sv}}
   * the outer hash table stores secrets per settings keys
   * and the inner one the secrets themselves
   */
  GValue mgmt  = G_VALUE_INIT;
  GValue pw    = G_VALUE_INIT;
  GValue ktype = G_VALUE_INIT;
  GValue auth  = G_VALUE_INIT;

  struct SecretData *secretData = g_new0 (struct SecretData, 1);
  secretData->agent = self;
  secretData->connection = connection;
  secretData->id = id;

  g_signal_connect (G_OBJECT (dialog),
                    "response",
                    G_CALLBACK (secret_dialog_response_cb),
                    secretData);

  g_object_ref (connection);
  gtk_widget_show (GTK_WIDGET (dialog));
}

void
request_cancelled_cb (UnitySettingsSecretAgent      *self,
                      guint64                        id,
                      gpointer                       data)
{
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
