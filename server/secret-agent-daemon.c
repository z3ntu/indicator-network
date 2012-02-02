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
  g_debug ("Secret requested %s", setting_name);
}

/*
  void  (*request_cancelled) (UnitySettingsSecretAgent      *self,
                              guint64                        id,
                              NMConnection                  *connection,
                              const char                    *setting_name,
                              const char                   **hints,
                              NMSecretAgentGetSecretsFlags   flags);
                              */
gint
main (gint argc, gchar** argv)
{
  GMainLoop     *loop;
  NMSecretAgent *agent;

  g_type_init ();

  agent = NM_SECRET_AGENT (unity_settings_secret_agent_new ());
  nm_secret_agent_register (agent);

  loop = g_main_loop_new (NULL, FALSE);
  g_main_loop_run (loop);
  g_main_loop_unref (loop);
  return 0;
}
