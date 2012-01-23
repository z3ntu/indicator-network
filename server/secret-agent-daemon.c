#include "secret-agent.h"

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
