#include <nm-connection.h>
#include <nm-access-point.h>
#include <nm-device.h>
#include <nm-device-wifi.h>
#include <libdbusmenu-glib/menuitem.h>

#include "accesspointitem.h"

#define CONNECTED "Connected"
#define DISCONNECTED "Disconnected"

DbusmenuMenuitem*
create_status_item (NMAccessPoint *ap, NMDevice *device)
{
  NMAccessPoint *active;
  DbusmenuMenuitem* item = dbusmenu_menuitem_new ();
  gchar *status = DISCONNECTED;
  gboolean conected = FALSE;

  dbusmenu_menuitem_property_set (item, DBUSMENU_MENUITEM_PROP_LABEL, "Status");

  g_object_get(device,
               "active-access-point", &active,
               NULL);

  conected = nm_device_get_state (device) == NM_DEVICE_STATE_ACTIVATED;
  conected = conected && (active == ap);

  if (conected)
      status = CONNECTED;

  dbusmenu_menuitem_property_set(DBUSMENU_MENUITEM(item),
                                 "type", "x-system-settings");
  dbusmenu_menuitem_property_set(DBUSMENU_MENUITEM(item),
                                 "x-tablet-widget", "unity.widgets.systemsettings.tablet.infobar");

  dbusmenu_menuitem_property_set(DBUSMENU_MENUITEM(item),
                                 "x-extra-label", status);

  return item;
}

void
create_accespoint_submenu (DbusmenuAccesspointitem *item)
{
  DbusmenuMenuitem *status = create_status_item (dbusmenu_accesspointitem_get_ap (item),
                                                 dbusmenu_accesspointitem_get_device (item));

  dbusmenu_menuitem_child_append (item, status);
}
