#include <nm-connection.h>
#include <nm-access-point.h>
#include <nm-device.h>
#include <nm-device-wifi.h>
#include <nm-utils.h>
#include <libdbusmenu-glib/menuitem.h>

#include "accesspointitem.h"

#define CONNECTED "Connected"
#define DISCONNECTED "Disconnected"

static DbusmenuMenuitem*
create_infobar (gchar *label,
                gchar *extra_label)
{
  DbusmenuMenuitem *item = dbusmenu_menuitem_new ();
  dbusmenu_menuitem_property_set (item, DBUSMENU_MENUITEM_PROP_LABEL, label);
  dbusmenu_menuitem_property_set(DBUSMENU_MENUITEM(item),
                                 "type", "x-system-settings");
  dbusmenu_menuitem_property_set(DBUSMENU_MENUITEM(item),
                                 "x-tablet-widget", "unity.widgets.systemsettings.tablet.listitem");
  dbusmenu_menuitem_property_set(DBUSMENU_MENUITEM(item),
                                 "x-extra-label", extra_label);
  return item;
}


static DbusmenuMenuitem*
create_status_item (NMAccessPoint *ap, NMDevice *device)
{
  NMAccessPoint *active;
  gchar *status = DISCONNECTED;
  gboolean conected = FALSE;

  /* TODO: Remove existing items in the submenu */

  g_object_get(device,
               "active-access-point", &active,
               NULL);

  conected = nm_device_get_state (device) == NM_DEVICE_STATE_ACTIVATED;
  conected = conected && (active == ap);

  if (conected)
      status = CONNECTED;

  return create_infobar ("Status", status);
}

static DbusmenuMenuitem*
create_network_item (NMAccessPoint *ap)
{
  DbusmenuMenuitem *item;
  gchar            *ssid = nm_utils_ssid_to_utf8 (nm_access_point_get_ssid (ap));

  item = create_infobar ("Network", ssid);

  g_free (ssid);
  return item;
}

static DbusmenuMenuitem*
create_signalstrength_item ()
{
  return NULL;
}

void
create_accespoint_submenu (DbusmenuAccesspointitem *item)
{
  DbusmenuMenuitem *status  = create_status_item   (dbusmenu_accesspointitem_get_ap (item),
                                                    dbusmenu_accesspointitem_get_device (item));
  DbusmenuMenuitem *network = create_network_item  (dbusmenu_accesspointitem_get_ap (item));
  DbusmenuMenuitem *strengh = create_signalstrength_item (dbusmenu_accesspointitem_get_ap (item));

  dbusmenu_menuitem_child_append (DBUSMENU_MENUITEM (item), status);
  dbusmenu_menuitem_child_append (DBUSMENU_MENUITEM (item), network);

  g_object_unref (status);
  g_object_unref (network);
  g_object_unref (strengh);
}
