#include <glib.h>
#include <glib-object.h>
#include <libdbusmenu-glib/client.h>
#include <libdbusmenu-glib/dbusmenu-glib.h>
#include <libdbusmenu-glib/enum-types.h>
#include <libdbusmenu-glib/menuitem-proxy.h>
#include <libdbusmenu-glib/menuitem.h>
#include <libdbusmenu-glib/server.h>
#include <libdbusmenu-glib/types.h>
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
	DbusmenuMenuitem parent_instance;
	DbusmenuAccesspointitemPrivate * priv;
	GByteArray* ssid;
};

struct _DbusmenuAccesspointitemClass {
	DbusmenuMenuitemClass parent_class;
};

struct _DbusmenuAccesspointitemPrivate {
	gchar* _bssid;
};


static gpointer dbusmenu_accesspointitem_parent_class = NULL;

GType dbusmenu_accesspointitem_get_type (void) G_GNUC_CONST;
#define DBUSMENU_ACCESSPOINTITEM_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), DBUSMENU_TYPE_ACCESSPOINTITEM, DbusmenuAccesspointitemPrivate))
enum  {
	DBUSMENU_ACCESSPOINTITEM_DUMMY_PROPERTY,
	DBUSMENU_ACCESSPOINTITEM_BSSID
};
void                     dbusmenu_accesspointitem_bind_accesspoint (DbusmenuAccesspointitem* self);
DbusmenuAccesspointitem* dbusmenu_accesspointitem_new              (void);
DbusmenuAccesspointitem* dbusmenu_accesspointitem_construct        (GType object_type);
const gchar*             dbusmenu_accesspointitem_get_bssid        (DbusmenuAccesspointitem* self);
void                     dbusmenu_accesspointitem_set_bssid        (DbusmenuAccesspointitem* self, const gchar* value);

static void  dbusmenu_accesspointitem_finalize (GObject      *obj);
static void  get_property                      (GObject      *object,
                                                guint         property_id,
                                                GValue       *value,
                                                GParamSpec   *pspec);
static void  set_property                      (GObject      *object,
                                                guint         property_id,
                                                const GValue *value,
                                                GParamSpec   *pspec);


void
dbusmenu_accesspointitem_bind_accesspoint (DbusmenuAccesspointitem* self)
{
	g_return_if_fail (self != NULL);
	dbusmenu_menuitem_property_set_int ((DbusmenuMenuitem*) self, "x-wifi-strength", 100);
}


DbusmenuAccesspointitem*
dbusmenu_accesspointitem_construct (GType object_type)
{
	DbusmenuAccesspointitem * self = NULL;
	self = (DbusmenuAccesspointitem*) g_object_new (object_type, NULL);
	return self;
}


DbusmenuAccesspointitem*
dbusmenu_accesspointitem_new (void)
{
	return dbusmenu_accesspointitem_construct (DBUSMENU_TYPE_ACCESSPOINTITEM);
}


const gchar*
dbusmenu_accesspointitem_get_bssid (DbusmenuAccesspointitem* self)
{
	g_return_val_if_fail (self != NULL, NULL);
	return result;
}


void
dbusmenu_accesspointitem_set_bssid (DbusmenuAccesspointitem* self, const gchar* value)
{
	g_return_if_fail (self != NULL);
        g_free (self->priv->_bssid);
	self->priv->_bssid = g_strdup (value);
	g_object_notify ((GObject *) self, "bssid");
}


static void
dbusmenu_accesspointitem_class_init (DbusmenuAccesspointitemClass * klass)
{
  dbusmenu_accesspointitem_parent_class = g_type_class_peek_parent (klass);
  g_type_class_add_private (klass, sizeof (DbusmenuAccesspointitemPrivate));
  G_OBJECT_CLASS (klass)->get_property = get_property;
  G_OBJECT_CLASS (klass)->set_property = set_property;
  G_OBJECT_CLASS (klass)->finalize = dbusmenu_accesspointitem_finalize;
  g_object_class_install_property (G_OBJECT_CLASS (klass), DBUSMENU_ACCESSPOINTITEM_BSSID, g_param_spec_string ("bssid", "bssid", "bssid", NULL, G_PARAM_STATIC_NAME | G_PARAM_STATIC_NICK | G_PARAM_STATIC_BLURB | G_PARAM_READABLE | G_PARAM_WRITABLE));
}


static void
dbusmenu_accesspointitem_instance_init (DbusmenuAccesspointitem * self)
{
  self->priv = DBUSMENU_ACCESSPOINTITEM_GET_PRIVATE (self);
}


static void
dbusmenu_accesspointitem_finalize (GObject* obj)
{
	DbusmenuAccesspointitem * self;
	self = DBUSMENU_ACCESSPOINTITEM (obj);
	g_free (self->priv->_bssid);
	g_byte_array_free (self->ssid);
	G_OBJECT_CLASS (dbusmenu_accesspointitem_parent_class)->finalize (obj);
}


GType
dbusmenu_accesspointitem_get_type (void)
{
	static volatile gsize dbusmenu_accesspointitem_type_id__volatile = 0;
	if (g_once_init_enter (&dbusmenu_accesspointitem_type_id__volatile))
        {
          static const GTypeInfo g_define_type_info = { sizeof (DbusmenuAccesspointitemClass), (GBaseInitFunc) NULL, (GBaseFinalizeFunc) NULL, (GClassInitFunc) dbusmenu_accesspointitem_class_init, (GClassFinalizeFunc) NULL, NULL, sizeof (DbusmenuAccesspointitem), 0, (GInstanceInitFunc) dbusmenu_accesspointitem_instance_init, NULL };
		GType dbusmenu_accesspointitem_type_id;
		dbusmenu_accesspointitem_type_id = g_type_register_static (dbusmenu_menuitem_get_type (), "DbusmenuAccesspointitem", &g_define_type_info, 0);
		g_once_init_leave (&dbusmenu_accesspointitem_type_id__volatile, dbusmenu_accesspointitem_type_id);
	}
	return dbusmenu_accesspointitem_type_id__volatile;
}


static void
get_property (GObject    *object,
              guint       property_id,
              GValue     *value,
              GParamSpec *pspec)
{
  DbusmenuAccesspointitem *self;
  self = DBUSMENU_ACCESSPOINTITEM (object);

  switch (property_id)
    {
    case DBUSMENU_ACCESSPOINTITEM_BSSID:
      g_value_set_string (value, dbusmenu_accesspointitem_get_bssid (self));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}


static void
set_property (GObject      *object,
              guint         property_id,
              const GValue *value,
              GParamSpec   *pspec)
{
  DbusmenuAccesspointitem *self;

  self = DBUSMENU_ACCESSPOINTITEM (object);

  switch (property_id)
    {
    case DBUSMENU_ACCESSPOINTITEM_BSSID:
      dbusmenu_accesspointitem_set_bssid (self, g_value_get_string (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}
