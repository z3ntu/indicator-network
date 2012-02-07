#include <glib.h>
#include <nm-utils.h>
#include <nm-access-point.h>
#include <nm-remote-connection.h>
#include <nm-device.h>
#include <nm-device-wifi.h>
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
        NMDevice      *device;
        gulong         notify_handler_id;
        gulong         ap_rem_handler_id;
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
                                          "x-wifi-strength",
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
  gchar         *utf_ssid = NULL;
  gboolean       is_adhoc = FALSE;
  gboolean       is_secure = FALSE;

  g_return_if_fail (self != NULL);
  g_return_if_fail (ap != NULL);

  if (self->priv->ap)
    {
      g_warning ("Bind accesspoint shouldn't be called more than once");
      return;
    }

  self->priv->ap = ap;
  g_signal_connect (ap, "notify",
                    G_CALLBACK (ap_notify_cb),
                    self);

  g_object_ref (G_OBJECT (ap));

  utf_ssid = nm_utils_ssid_to_utf8 (nm_access_point_get_ssid (ap));

  if (nm_access_point_get_mode (ap) == NM_802_11_MODE_ADHOC)
    is_adhoc  = TRUE;
  if (nm_access_point_get_flags (ap) == NM_802_11_AP_FLAGS_PRIVACY)
    is_secure = TRUE;

  dbusmenu_menuitem_property_set (DBUSMENU_MENUITEM (self),
                                  DBUSMENU_MENUITEM_PROP_TOGGLE_TYPE,
                                  "radio");

  dbusmenu_menuitem_property_set_int  (DBUSMENU_MENUITEM (self),
                                       "x-wifi-strength",   nm_access_point_get_strength (ap));
  dbusmenu_menuitem_property_set_bool (DBUSMENU_MENUITEM (self),
                                       "x-wifi-is-adhoc",   is_adhoc);
  dbusmenu_menuitem_property_set_bool (DBUSMENU_MENUITEM (self),
                                       "x-wifi-is-secure",  is_secure);

  dbusmenu_menuitem_property_set (DBUSMENU_MENUITEM (self),
                                  "x-wifi-bssid", nm_access_point_get_bssid (ap));
  dbusmenu_menuitem_property_set (DBUSMENU_MENUITEM (self),
                                  "type", "x-system-settings");
  dbusmenu_menuitem_property_set (DBUSMENU_MENUITEM (self),
                                  "x-tablet-widget", "unity.widgets.systemsettings.tablet.accesspoint");
  dbusmenu_menuitem_property_set (DBUSMENU_MENUITEM (self),
                                  DBUSMENU_MENUITEM_PROP_LABEL, utf_ssid);
}

static void
connection_changed (NMDevice                *device,
                    GParamSpec              *pspec,
                    DbusmenuAccesspointitem *item)
{
  if (g_strcmp0 (g_param_spec_get_name (pspec), "active-access-point") == 0)
    {
      NMAccessPoint *active;

      g_object_get (device,
              "active-access-point", &active,
              NULL);

       /* TODO: Remove/add active AP submenu */
      dbusmenu_menuitem_property_set_int (DBUSMENU_MENUITEM (item),
                                          DBUSMENU_MENUITEM_PROP_TOGGLE_STATE,
                                          DBUSMENU_MENUITEM_TOGGLE_STATE_UNCHECKED);

      if (active == NULL)
          return;

      if (item->priv->ap == active)
        {
          dbusmenu_menuitem_property_set_int (DBUSMENU_MENUITEM (item),
                                              DBUSMENU_MENUITEM_PROP_TOGGLE_STATE,
                                              DBUSMENU_MENUITEM_TOGGLE_STATE_CHECKED);
          /* TODO: Add active submenu */
        }

      return;
    }
}

static void
ap_removed (NMDeviceWifi            *device,
            NMAccessPoint           *removed,
            DbusmenuAccesspointitem *self)
{
  DbusmenuMenuitem *parent = dbusmenu_menuitem_get_parent (DBUSMENU_MENUITEM (self));

  if (parent)
    dbusmenu_menuitem_child_delete (parent, DBUSMENU_MENUITEM (self));
}

void
dbusmenu_accesspointitem_bind_device (DbusmenuAccesspointitem *self,
                                      NMDevice                *device)
{
  NMAccessPoint *active;
  g_return_if_fail (self != NULL);
  g_return_if_fail (device != NULL);

  if (self->priv->device)
    {
      g_warning ("Bind accesspoint shouldn't be called more than once");
      return;
    }

  g_object_get (device,
                "active-access-point", &active,
                NULL);

  /* TODO: Add active AP submenu */
  if (active != NULL && self->priv->ap == active)
    dbusmenu_menuitem_property_set_int (DBUSMENU_MENUITEM (self),
                                  DBUSMENU_MENUITEM_PROP_TOGGLE_STATE,
                                  DBUSMENU_MENUITEM_TOGGLE_STATE_CHECKED);



  self->priv->device = device;
  self->priv->notify_handler_id = g_signal_connect (NM_DEVICE_WIFI (device), "notify",
                                                    G_CALLBACK (connection_changed), self);
  self->priv->ap_rem_handler_id = g_signal_connect (NM_DEVICE_WIFI (device), "access-point-removed",
                                                    G_CALLBACK (ap_removed), self);
  g_object_ref (device);
}

const NMAccessPoint*
dbusmenu_accesspointitem_get_ap (DbusmenuAccesspointitem *item)
{
  return item->priv->ap;
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
  self->priv->device = NULL;
}


static void
dbusmenu_accesspointitem_finalize (GObject* obj)
{
  DbusmenuAccesspointitem *self = DBUSMENU_ACCESSPOINTITEM (obj);

  if (self->priv->device)
    {
      g_signal_handler_disconnect (self->priv->device, self->priv->notify_handler_id);
      g_signal_handler_disconnect (self->priv->device, self->priv->ap_rem_handler_id);
    }

  g_object_unref (self->priv->ap);
  g_object_unref (self->priv->device);
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
