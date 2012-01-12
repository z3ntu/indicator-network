#include <gio/gio.h>
#include <nm-client.h>
#include <nm-device-wifi.h>
#include <libdbusmenu-glib/dbusmenu-glib.h>

GMainLoop *loop;

/*static void
wireless_state_changed (GObject *client, GParamSpec *pspec, gpointer user_data)
{
}*/

static void
wifi_populate_accesspoints (DbusmenuMenuitem *parent,
                            NMClient         *client,
                            NMDeviceWifi     *device,
                            gint             *id)
{
  gint              i;
  const GPtrArray  *apsarray = nm_device_wifi_get_access_points (device);
  NMAccessPoint   **aps;

  aps = (NMAccessPoint**) apsarray->pdata;
  for (i=0; i < apsarray->len; i++)
    {
      gboolean          is_private = FALSE;
      gboolean          is_adhoc   = FALSE;
      NMAccessPoint    *ap = aps[i];
      DbusmenuMenuitem *ap_item = dbusmenu_menuitem_new_with_id ((*id)++);
      const GByteArray *ssid;

      ssid = nm_access_point_get_ssid (ap);

      if (nm_access_point_get_flags (ap) == NM_802_11_AP_FLAGS_PRIVACY)
        is_private = TRUE;
      if (nm_access_point_get_mode (ap) == NM_802_11_MODE_ADHOC)
        is_adhoc   = TRUE;

      dbusmenu_menuitem_property_set  (ap_item, DBUSMENU_MENUITEM_PROP_LABEL, (gchar*)ssid->data);
      dbusmenu_menuitem_property_set  (ap_item, "type", "x-system-settings");
      dbusmenu_menuitem_property_set  (ap_item, "x-tablet-widget", "unity.widgets.systemsettings.tablet.accesspoint");

      dbusmenu_menuitem_property_set_int  (ap_item, "x-wifi-strength",   nm_access_point_get_strength (ap));
      dbusmenu_menuitem_property_set_bool (ap_item, "x-wifi-is-private", is_private);
      dbusmenu_menuitem_property_set_bool (ap_item, "x-wifi-is-adhoc",   is_adhoc);

      dbusmenu_menuitem_child_append  (parent, ap_item);
    }
}

static void
wireless_toggle_activated (DbusmenuMenuitem *toggle,
                           guint             timestamp,
                           gpointer          data)
{
  g_debug ("State toggled");
}

static void
wifi_device_handler (DbusmenuMenuitem *parent, NMClient *client, NMDevice *device, gint *id)
{
  /* Wifi enable/disable toggle */
  gboolean          wifienabled   = nm_client_wireless_get_enabled (client);
  DbusmenuMenuitem *togglesep     = dbusmenu_menuitem_new_with_id ((*id)++);
  DbusmenuMenuitem *toggle        = dbusmenu_menuitem_new_with_id ((*id)++);

  DbusmenuMenuitem *networksgroup = dbusmenu_menuitem_new_with_id ((*id)++);

  dbusmenu_menuitem_property_set (togglesep, DBUSMENU_MENUITEM_PROP_LABEL, "Turn Wifi On/Off");
  dbusmenu_menuitem_property_set (togglesep, DBUSMENU_MENUITEM_PROP_TYPE,  "separator");

  dbusmenu_menuitem_property_set (toggle, DBUSMENU_MENUITEM_PROP_TOGGLE_TYPE, DBUSMENU_MENUITEM_TOGGLE_CHECK);
  dbusmenu_menuitem_property_set (toggle, DBUSMENU_MENUITEM_PROP_LABEL, "Wifi");
  dbusmenu_menuitem_property_set_int (toggle, DBUSMENU_MENUITEM_PROP_TOGGLE_STATE, wifienabled);

  g_signal_connect (toggle, "item-activated",
                    G_CALLBACK (wireless_toggle_activated),
                    NULL);

  dbusmenu_menuitem_child_append (parent, togglesep);
  dbusmenu_menuitem_child_append (parent, toggle);

  /* Access points */
  if (wifienabled)
    {
      dbusmenu_menuitem_property_set (networksgroup, DBUSMENU_MENUITEM_PROP_LABEL, "Select wireless network");
      dbusmenu_menuitem_property_set (networksgroup, "x-group-type", "inline");
      dbusmenu_menuitem_property_set_bool (networksgroup, "x-busy", TRUE);
      dbusmenu_menuitem_property_set (networksgroup, "x-group-class", "accesspoints");
      dbusmenu_menuitem_property_set (networksgroup, "type", "x-system-settings");

      dbusmenu_menuitem_child_append (parent, networksgroup);
      wifi_populate_accesspoints (networksgroup, client, NM_DEVICE_WIFI (device), id);
    }

  /* TODO: Remove this when toggle is removed */
  /*g_signal_connect (client, "notify::WirelessEnabled",
                    G_CALLBACK (wireless_state_changed)
                    toggle);*/
  /*g_signal_connect (client, "notify::WirelessHardwareEnabled",
                    G_CALLBACK (wireless_state_changed)
                    toggle);*/
}

static void
on_bus (GDBusConnection * connection, const gchar * name, gpointer user_data)
{
  gint               i, id = 0;
  const GPtrArray   *devarray;
  NMClient          *client;
  NMDevice         **devices;
  DbusmenuServer    *server = dbusmenu_server_new("/com/ubuntu/networksettings");
  DbusmenuMenuitem  *root   = dbusmenu_menuitem_new_with_id (id++);

  dbusmenu_server_set_root (server, root);

  client = nm_client_new ();
  devarray = nm_client_get_devices (client);

  devices = (NMDevice**) devarray->pdata;
  for (i=0; i < devarray->len; i++)
    {
      NMDevice *device = devices[i];
      gint type = nm_device_get_device_type (device);

      switch (type)
        {
        case NM_DEVICE_TYPE_WIFI:
          wifi_device_handler (root, client, device, &id);
          break;
        }
    }
  /* TODO: Advance tab (per device?) */
  /* TODO: Airplane mode */

  g_object_unref (client);
  return;
}

static void
name_lost (GDBusConnection * connection, const gchar * name, gpointer user_data)
{
  g_main_loop_quit (loop);
	return;
}

int
main (int argc, char** argv)
{
  g_type_init ();

  g_bus_own_name(G_BUS_TYPE_SESSION,
                 "com.ubuntu.networksettings",
                 G_BUS_NAME_OWNER_FLAGS_NONE,
                 on_bus,
                 NULL,
                 name_lost,
                 NULL,
                 NULL);

  loop = g_main_loop_new (NULL, FALSE);
  g_main_loop_run (loop);
  g_main_loop_unref (loop);

  return 0;
}
