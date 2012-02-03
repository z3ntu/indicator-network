#include <glib.h>
#include <glib-object.h>
#include <nm-secret-agent.h>
#include "secret-agent.h"
#include "secret-marshal.h"

#define UNITY_SETTINGS_TYPE_SECRET_AGENT (unity_settings_secret_agent_get_type ())
#define UNITY_SETTINGS_SECRET_AGENT(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), UNITY_SETTINGS_TYPE_SECRET_AGENT, UnitySettingsSecretAgent))
#define UNITY_SETTINGS_SECRET_AGENT_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), UNITY_SETTINGS_TYPE_SECRET_AGENT, UnitySettingsSecretAgentClass))
#define UNITY_SETTINGS_IS_SECRET_AGENT(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), UNITY_SETTINGS_TYPE_SECRET_AGENT))
#define UNITY_SETTINGS_IS_SECRET_AGENT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), UNITY_SETTINGS_TYPE_SECRET_AGENT))
#define UNITY_SETTINGS_SECRET_AGENT_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), UNITY_SETTINGS_TYPE_SECRET_AGENT, UnitySettingsSecretAgentClass))
#define UNITY_SETTINGS_SECRET_AGENT_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), UNITY_SETTINGS_TYPE_SECRET_AGENT, UnitySettingsSecretAgentPrivate))

static gpointer unity_settings_secret_agent_parent_class = NULL;

typedef struct _UnitySettingsSecretAgentPrivate UnitySettingsSecretAgentPrivate;

struct _UnitySettingsSecretAgentPrivate {
  guint64  request_counter;
  GQueue  *requests;
};

typedef struct _SecretRequest {
  gint                           id;
  NMSecretAgent                 *agent;
  NMConnection                  *connection;
  const char                    *connection_path;
  const char                    *setting_name;
  const char                   **hints;
  NMSecretAgentGetSecretsFlags   flags;
  NMSecretAgentGetSecretsFunc    callback;
  gpointer                       callback_data;
} SecretRequest;

GType unity_settings_secret_agent_get_type (void) G_GNUC_CONST;
enum  {
	UNITY_SETTINGS_SECRET_AGENT_DUMMY_PROPERTY
};

enum {
  SECRET_REQUESTED,
  REQUEST_CANCELLED,
  LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

UnitySettingsSecretAgent* unity_settings_secret_agent_new       (void);
UnitySettingsSecretAgent* unity_settings_secret_agent_construct (GType object_type);

int
secret_request_find (SecretRequest  *req,
                     guint64        *id)
{
  if (req->id > *id)
      return -1;

  if (req->id < *id)
      return 1;

  return 0;
}

void
unity_settings_secret_agent_provide_secret (UnitySettingsSecretAgent *agent,
                                            guint64                   request,
                                            GHashTable               *secrets)
{
  GList                           *iter;
  SecretRequest                   *req;
  UnitySettingsSecretAgentPrivate *priv = agent->priv;

  iter = g_queue_find_custom (priv->requests,
                              &request,
                              (GCompareFunc)secret_request_find);

  if (iter == NULL || iter->data == NULL)
    {
      g_warning ("Secret request with id <%d> was not found", (int)request);
      return;
    }

  req = iter->data;

  req->callback (NM_SECRET_AGENT (agent),
                 req->connection,
                 secrets,
                 NULL,
                 req->callback_data);

  g_queue_remove (priv->requests, req);
  return;
}

void
unity_settings_secret_agent_cancel_request (UnitySettingsSecretAgent *agent,
                                            guint64                   request)
{
  GList                           *iter;
  SecretRequest                   *req;
  UnitySettingsSecretAgentPrivate *priv = agent->priv;
  GError *error;

  iter = g_queue_find_custom (priv->requests,
                              &request,
                              (GCompareFunc)secret_request_find);

  if (iter == NULL || iter->data == NULL)
    {
      g_warning ("Secret request with id <%d> was not found", (int)request);
      return;
    }

  req = iter->data;
  error = g_error_new (NM_SECRET_AGENT_ERROR,
                       NM_SECRET_AGENT_ERROR_INTERNAL_ERROR,
                       "This secret request was canceled by the user.");

  req->callback (NM_SECRET_AGENT (agent),
                 req->connection,
                 NULL,
                 error,
                 req->callback_data);

  g_queue_remove (priv->requests, req);

  g_error_free (error);
  return;
}

static void
delete_secrets (NMSecretAgent *agent,
                NMConnection *connection,
                const char *connection_path,
                NMSecretAgentDeleteSecretsFunc callback,
                gpointer callback_data)
{
  g_debug ("delete secrets");
}

static guint64
find_available_id (UnitySettingsSecretAgentPrivate *priv)
{
  priv->request_counter++;
  return priv->request_counter;
}

static void
get_secrets (NMSecretAgent                 *agent,
             NMConnection                  *connection,
             const char                    *connection_path,
             const char                    *setting_name,
             const char                   **hints,
             NMSecretAgentGetSecretsFlags   flags,
             NMSecretAgentGetSecretsFunc    callback,
             gpointer                       callback_data)
{
  guint64 id;
  UnitySettingsSecretAgentPrivate *priv = UNITY_SETTINGS_SECRET_AGENT_GET_PRIVATE (agent);
  SecretRequest *req = NULL;

  if (flags == NM_SECRET_AGENT_GET_SECRETS_FLAG_NONE)
    {
      GError *error = g_error_new (NM_SECRET_AGENT_ERROR,
                                   NM_SECRET_AGENT_ERROR_INTERNAL_ERROR,
                                   "No password found for this connection.");
      callback (agent, connection, NULL, error, callback_data);
      g_error_free (error);
      return;
    }

  if (priv->request_counter >= G_MAXUINT64)
    {
      GError *error = g_error_new (NM_SECRET_AGENT_ERROR,
                                   NM_SECRET_AGENT_ERROR_INTERNAL_ERROR,
                                   "Reached maximum number of requests.");
      callback (agent, connection, NULL, error, callback_data);
      g_error_free (error);
      return;
    }

  /* Adding a request */
  id = find_available_id (priv);
  req = (SecretRequest*) g_malloc0 (sizeof (SecretRequest));
  *req = ((SecretRequest)
          { id,
            agent,
            connection,
            connection_path,
            setting_name,
            hints,
            flags,
            callback,
            callback_data });

  g_queue_push_tail (priv->requests, req);

  g_signal_emit_by_name (agent,
                         UNITY_SETTINGS_SECRET_AGENT_SECRET_REQUESTED,
                         priv->request_counter,
                         connection,
                         setting_name,
                         hints,
                         flags);
}

static void
save_secrets (NMSecretAgent                *agent,
              NMConnection                 *connection,
              const char                   *connection_path,
              NMSecretAgentSaveSecretsFunc  callback,
              gpointer                      callback_data)
{
  g_debug ("save secrets");
}

static void
cancel_get_secrets (NMSecretAgent *agent,
                    const char *connection_path,
                    const char *setting_name)
{
  g_debug ("cancel get secrets");
}

UnitySettingsSecretAgent*
unity_settings_secret_agent_construct (GType object_type)
{
  UnitySettingsSecretAgent * self = NULL;
  self = (UnitySettingsSecretAgent*) g_object_new (object_type,
                                                   NM_SECRET_AGENT_IDENTIFIER, "com.unity.nm-agent",
                                                   NULL);
  return self;
}


UnitySettingsSecretAgent*
unity_settings_secret_agent_new (void)
{
  return unity_settings_secret_agent_construct (UNITY_SETTINGS_TYPE_SECRET_AGENT);
}


static void
unity_settings_secret_agent_finalize (GObject *agent)
{
  UnitySettingsSecretAgentPrivate *priv = UNITY_SETTINGS_SECRET_AGENT_GET_PRIVATE (agent);
  /*FIXME: Get rid of all requests */
  g_queue_free (priv->requests);
}

static void
unity_settings_secret_agent_class_init (UnitySettingsSecretAgentClass *klass)
{
  unity_settings_secret_agent_parent_class = g_type_class_peek_parent (klass);
  NMSecretAgentClass         *parent_class = NM_SECRET_AGENT_CLASS (klass);
  parent_class->get_secrets = get_secrets;
  parent_class->save_secrets = save_secrets;
  parent_class->delete_secrets = delete_secrets;
  parent_class->cancel_get_secrets = cancel_get_secrets;

  g_type_class_add_private (klass, sizeof(UnitySettingsSecretAgentPrivate));
  G_OBJECT_CLASS (klass)->finalize = unity_settings_secret_agent_finalize;


  signals[SECRET_REQUESTED] = g_signal_new (UNITY_SETTINGS_SECRET_AGENT_SECRET_REQUESTED,
                                            G_OBJECT_CLASS_TYPE (G_OBJECT_CLASS (klass)),
                                            G_SIGNAL_RUN_FIRST,
                                            G_STRUCT_OFFSET (UnitySettingsSecretAgentClass, secret_requested),
                                            NULL, NULL,
                                            _secret_agent_marshal_VOID__UINT64_POINTER_STRING_POINTER_UINT,
                                            G_TYPE_NONE, 5,
                                            G_TYPE_UINT64, G_TYPE_POINTER, G_TYPE_STRING, G_TYPE_POINTER, G_TYPE_UINT);

  signals[REQUEST_CANCELLED] = g_signal_new (UNITY_SETTINGS_SECRET_AGENT_REQUEST_CANCELLED,
                                             G_OBJECT_CLASS_TYPE (G_OBJECT_CLASS (klass)),
                                             G_SIGNAL_RUN_FIRST,
                                             G_STRUCT_OFFSET (UnitySettingsSecretAgentClass, request_cancelled),
                                             NULL, NULL,
                                             _secret_agent_marshal_VOID__UINT64,
                                             G_TYPE_NONE, 1,
                                             G_TYPE_UINT64);
}


static void
unity_settings_secret_agent_instance_init (UnitySettingsSecretAgent *self)
{
  self->priv = UNITY_SETTINGS_SECRET_AGENT_GET_PRIVATE (self);
  self->priv->requests = g_queue_new ();
  self->priv->request_counter = 0;
}

GType
unity_settings_secret_agent_get_type (void)
{
  static volatile gsize unity_settings_secret_agent_type_id__volatile = 0;
  if (g_once_init_enter (&unity_settings_secret_agent_type_id__volatile))
    {
      static const GTypeInfo g_define_type_info =
        {
          sizeof (UnitySettingsSecretAgentClass),
          (GBaseInitFunc) NULL,
          (GBaseFinalizeFunc) NULL,
          (GClassInitFunc) unity_settings_secret_agent_class_init,
          (GClassFinalizeFunc) NULL,
          NULL,
          sizeof (UnitySettingsSecretAgent),
          0,
          (GInstanceInitFunc) unity_settings_secret_agent_instance_init,
          NULL
        };
      GType unity_settings_secret_agent_type_id;
      unity_settings_secret_agent_type_id = g_type_register_static (NM_TYPE_SECRET_AGENT,
                                                                    "UnitySettingsSecretAgent",
                                                                    &g_define_type_info,
                                                                    0);
      g_once_init_leave (&unity_settings_secret_agent_type_id__volatile,
                         unity_settings_secret_agent_type_id);
    }

  return unity_settings_secret_agent_type_id__volatile;
}
