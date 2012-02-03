#include <gtk/gtk.h>
#include "secret-agent.h"

GtkWidget                *dialog;

void
secret_requested_cb (UnitySettingsSecretAgent      *self,
                     guint64                        id,
                     NMConnection                  *connection,
                     const char                    *setting_name,
                     const char                   **hints,
                     NMSecretAgentGetSecretsFlags   flags,
                     gpointer                       data)
{
  gint response = gtk_dialog_run (GTK_DIALOG (dialog));
  gtk_widget_hide (dialog);

  g_debug ("Secret requested <%d> %s", (int)id, setting_name);

  unity_settings_secret_agent_cancel_request (self, id);
}

void
request_cancelled_cb (UnitySettingsSecretAgent      *self,
                      guint64                        id)
{
  g_debug ("Request cancelled <%d>", (int)id);
}

gint
main (gint argc, gchar** argv)
{

  GMainLoop                *loop;
  UnitySettingsSecretAgent *agent;

  gtk_init (0, NULL);

  agent = unity_settings_secret_agent_new ();
  nm_secret_agent_register (NM_SECRET_AGENT (agent));

  g_signal_connect (G_OBJECT (agent),
                    UNITY_SETTINGS_SECRET_AGENT_SECRET_REQUESTED,
                    G_CALLBACK (secret_requested_cb),
                    NULL);

  g_signal_connect (G_OBJECT (agent),
                    UNITY_SETTINGS_SECRET_AGENT_REQUEST_CANCELLED,
                    G_CALLBACK (secret_requested_cb),
                    NULL);

  dialog = gtk_dialog_new_with_buttons ("My dialog",
                                        NULL,
                                        GTK_DIALOG_MODAL,
                                        GTK_STOCK_OK,
                                        GTK_RESPONSE_ACCEPT,
                                        GTK_STOCK_CANCEL,
                                        GTK_RESPONSE_REJECT,
                                        NULL);

  loop = g_main_loop_new (NULL, FALSE);
  g_main_loop_run (loop);
  g_main_loop_unref (loop);
  return 0;
}
