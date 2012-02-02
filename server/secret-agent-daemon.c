#include <gtk/gtk.h>
#include "secret-agent.h"

void
secret_requested_cb (UnitySettingsSecretAgent      *self,
                     guint64                        id,
                     NMConnection                  *connection,
                     const char                    *setting_name,
                     const char                   **hints,
                     NMSecretAgentGetSecretsFlags   flags,
                     gpointer                       data)
{
  g_debug ("Secret requested <%d> %s", (int)id, setting_name);
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

  g_type_init ();

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

  loop = g_main_loop_new (NULL, FALSE);
  g_main_loop_run (loop);
  g_main_loop_unref (loop);
  return 0;
}
