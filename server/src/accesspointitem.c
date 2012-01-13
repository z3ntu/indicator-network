#include <glib.h>
#include <nm-utils.h>
#include <nm-access-point.h>
#include <libdbusmenu-glib/menuitem.h>
#include <stdlib.h>
#include <string.h>


#define DBUSMENU_TYPE_ACCESSPOINTITEM (dbusmenu_accesspointitem_get_type ())
#define DBUSMENU_ACCESSPOINTITEM(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), DBUSMENU_TYPE_ACCESSPOINTITEM, DbusmenuAccesspointitem))
#define DBUSMENU_ACCESSPOINTITEM_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), DBUSMENU_TYPE_ACCESSPOINTITEM, DbusmenuAccesspointitemClass))
#define DBUSMENU_IS_ACCESSPOINTITEM(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), DBUSMENU_TYPE_ACCESSPOINTITEM))
#define DBUSMENU_IS_ACCESSPOINTITEM_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), DBUSMENU_TYPE_ACCESSPOINTITEM))
#define DBUSMENU_ACCESSPOINTITEM_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), DBUSMENU_TYPE_ACCESSPOINTITEM, DbusmenuAccesspointitemClass))

typedef struct _DbusmenuAccesspointitem DbusmenuAccesspointitem;
typedef struct _DbusmenuAccesspointitemClass DbusmenuAccesspointitemClass;
typedef struct _DbusmenuAccesspointitemPrivate DbusmenuAccesspointitemPrivate;

struct _DbusmenuAccesspointitem {
	DbusmenuMenuitem                parent_instance;
	DbusmenuAccesspointitemPrivate *priv;
};

struct _DbusmenuAccesspointitemClass {
	DbusmenuMenuitemClass parent_class;
};

struct _DbusmenuAccesspointitemPrivate {
        NMAccessPoint *ap;
};


static gpointer dbusmenu_accesspointitem_parent_class = NULL;

GType dbusmenu_accesspointitem_get_type (void) G_GNUC_CONST;
#define DBUSMENU_ACCESSPOINTITEM_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), DBUSMENU_TYPE_ACCESSPOINTITEM, DbusmenuAccesspointitemPrivate))
enum  {
	DBUSMENU_ACCESSPOINTITEM_DUMMY_PROPERTY,
	DBUSMENU_ACCESSPOINTITEM_BSSID
};
void                     dbusmenu_accesspointitem_bind_accesspoint (DbusmenuAccesspointitem *self,
                                                                    NMAccessPoint           *ap);
DbusmenuAccesspointitem* dbusmenu_accesspointitem_new              (void);
DbusmenuAccesspointitem* dbusmenu_accesspointitem_construct        (GType object_type);

static void  dbusmenu_accesspointitem_finalize (GObject      *obj);

static void
ap_notify_cb (GObject    *ap,
              GParamSpec *pspec,
              gpointer    data)
{

  DbusmenuAccesspointitem *item     = (DbusmenuAccesspointitem*)data;
  const gchar             *property = g_param_spec_get_name (pspec);

  if (g_strcmp0 (property, "ssid") == 0)
    {
      gchar *ssid_utf8 = nm_utils_ssid_to_utf8 (nm_access_point_get_ssid (NM_ACCESS_POINT (ap)));
      dbusmenu_menuitem_property_set (DBUSMENU_MENUITEM (item), "label", ssid_utf8);
      g_free (ssid_utf8);
    }
  if (g_strcmp0 (property, "strength") == 0)
    {
      dbusmenu_menuitem_property_set_int (DBUSMENU_MENUITEM (item),
                                          "x-wifi-strenght",
                                          nm_access_point_get_strength (NM_ACCESS_POINT (ap)));
    }
  if (g_strcmp0 (property, "bssid") == 0)
    {
      dbusmenu_menuitem_property_set (DBUSMENU_MENUITEM (item),
                                      "x-wifi-bssid",
                                      nm_access_point_get_bssid (NM_ACCESS_POINT (ap)));
    }
}

void
dbusmenu_accesspointitem_bind_accesspoint (DbusmenuAccesspointitem *self,
                                           NMAccessPoint           *ap)
{
  g_return_if_fail (self != NULL);
  g_return_if_fail (ap != NULL);
  self->priv->ap = ap;

//  dbusmenu_menuitem_property_set_int ((DbusmenuMenuitem*) self, "x-wifi-strength", 100);
  g_signal_connect (ap, "notify",
                    G_CALLBACK (ap_notify_cb),
                    self);
  g_object_ref (ap);
}

DbusmenuAccesspointitem*
dbusmenu_accesspointitem_new (void)
{
  return  (DbusmenuAccesspointitem*)g_object_new (DBUSMENU_TYPE_ACCESSPOINTITEM, NULL);
}

DbusmenuAccesspointitem*
dbusmenu_accesspointitem_new_with_id (gint id)
{
  return (DbusmenuAccesspointitem*)g_object_new (DBUSMENU_TYPE_ACCESSPOINTITEM, "id", id, NULL);
}

static void
dbusmenu_accesspointitem_class_init (DbusmenuAccesspointitemClass * klass)
{
  dbusmenu_accesspointitem_parent_class = g_type_class_peek_parent (klass);
  g_type_class_add_private (klass, sizeof (DbusmenuAccesspointitemPrivate));
  G_OBJECT_CLASS (klass)->finalize = dbusmenu_accesspointitem_finalize;
}


static void
dbusmenu_accesspointitem_instance_init (DbusmenuAccesspointitem * self)
{
  self->priv = DBUSMENU_ACCESSPOINTITEM_GET_PRIVATE (self);
  self->priv->ap = NULL;
}


static void
dbusmenu_accesspointitem_finalize (GObject* obj)
{
  DbusmenuAccesspointitem * self;
  self = DBUSMENU_ACCESSPOINTITEM (obj);
  g_object_unref (self->priv->ap);
  G_OBJECT_CLASS (dbusmenu_accesspointitem_parent_class)->finalize (obj);
}


GType
dbusmenu_accesspointitem_get_type (void)
{
  static volatile gsize dbusmenu_accesspointitem_type_id__volatile = 0;
  if (g_once_init_enter (&dbusmenu_accesspointitem_type_id__volatile))
    {
      GType dbusmenu_accesspointitem_type_id;
      static const GTypeInfo g_define_type_info = { sizeof (DbusmenuAccesspointitemClass),
        (GBaseInitFunc) NULL,
        (GBaseFinalizeFunc) NULL,
        (GClassInitFunc) dbusmenu_accesspointitem_class_init,
        (GClassFinalizeFunc) NULL,
        NULL,
        sizeof (DbusmenuAccesspointitem),
        0,
        (GInstanceInitFunc) dbusmenu_accesspointitem_instance_init,
        NULL };

      dbusmenu_accesspointitem_type_id = g_type_register_static (dbusmenu_menuitem_get_type (),
                                                                 "DbusmenuAccesspointitem",
                                                                 &g_define_type_info,
                                                                 0);
      g_once_init_leave (&dbusmenu_accesspointitem_type_id__volatile,
                         dbusmenu_accesspointitem_type_id);
    }
  return dbusmenu_accesspointitem_type_id__volatile;
}
