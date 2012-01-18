#include <string.h>
#include <glib.h>
#include <gio/gio.h>
#include <nm-utils.h>
#include <nm-client.h>
#include <nm-remote-settings.h>
#include <nm-device-wifi.h>
#include <libdbusmenu-glib/dbusmenu-glib.h>

#include "accesspointitem.h"

/*
 * unity.widgets.systemsettings.*.sectiontitle
 * Build properties:
 *    "type"             - string = "x-system-settings"
 *    "children-display" - string = "inline"
 *    "x-tablet-widget"  - string = "unity.widgets.systemsettings.tablet.sectiontitle"
 * Other properties:
 *    "label"  - string  - Optional, may have label or not
 *    "x-busy" - boolean - Shows a progress indicator
 *
 * unity.widgets.systemsettings.*.accesspoint
 * Build properties:
 *    "type"            - string = "x-system-settings"
 *    "toggle-type"     - string = "radio"
 *    "x-tablet-widget" - string = "unity.widgets.systemsettings.tablet.accesspoint"
 * Other properties:
 *    "x-wifi-strength"   - int    - Signal strength
 *    "x-wifi-is-adhoc"   - bool   - Whether it is an adhoc network or not
 *    "x-wifi-is-secure"  - bool   - Whether the network is open or requires password
 *    "x-wifi-bssid"      - string - The internal unique id for the AP
 */

typedef struct {
  NMDevice *device;
  NMClient *client;
} ClientDevice;

typedef struct {
  NMAccessPoint *active_ap;
  GSList        *connections;
} ActiveAPConnections;

static void
destroy_client_device (gpointer  data,
                       GClosure *closure)
{
  g_free (data);
}

static void
access_point_selected (DbusmenuMenuitem *item,
                       guint             timestamp,
                       gpointer          data)
{
  gint              i;
  ClientDevice     *cd       = (ClientDevice*)data;
  NMClient         *client   = cd->client;
  NMDeviceWifi     *device   = NM_DEVICE_WIFI (cd->device);
  const GPtrArray  *apsarray = nm_device_wifi_get_access_points (device);
  NMAccessPoint    *ap       = NULL;

  /* Use SSID instead of BSSID */
  for (i=0; i<apsarray->len; i++)
    {
      const gchar *bssid = dbusmenu_menuitem_property_get (item, "x-wifi-bssid");
      NMAccessPoint **aps = (NMAccessPoint**)(apsarray->pdata);
      if (g_strcmp0 (nm_access_point_get_bssid (aps[i]), bssid) == 0)
        {
          ap = aps[i];
          break;
        }
    }

  if (ap && ap == nm_device_wifi_get_active_access_point (device))
    return;

  if (ap != NULL)
    {
      NMRemoteSettings *rs              = nm_remote_settings_new (NULL);
      GSList           *rs_connections  = nm_remote_settings_list_connections (rs);
      GSList           *dev_connections = nm_device_filter_connections (NM_DEVICE (device), rs_connections);
      GSList           *connections     = nm_access_point_filter_connections (ap, dev_connections);

      if (g_slist_length (connections) > 0)
        {
          /* TODO: Select the most recently used one */
          nm_client_activate_connection(client,
                                        (NMConnection*)(connections->data),
                                        NM_DEVICE (device),
                                        nm_object_get_path (NM_OBJECT (ap)),
                                        NULL,
                                        NULL);
        }
      else
        {
          nm_client_add_and_activate_connection (client,
                                                 NULL,
                                                 NM_DEVICE (device),
                                                 nm_object_get_path (NM_OBJECT (ap)),
                                                 NULL,
                                                 NULL);
        }

      g_slist_free (connections);
      g_slist_free (rs_connections);
      g_slist_free (dev_connections);
      g_object_unref (rs);
    }
}

/* This function removes any duplicates by AP SSID and leaves
 * the ones with a strongest signal.
 */
static GPtrArray*
filter_access_points (const GPtrArray *apsarray,
                      NMDeviceWifi    *device)
{
  gint i;
  GPtrArray *collapsedaps = g_ptr_array_new ();
  NMAccessPoint *active;

  g_object_get(device,
               "active-access-point", &active,
               NULL);

  for (i = 0; i < apsarray->len; i++)
    {
      gint j;
      NMAccessPoint *ap = (NMAccessPoint*) g_ptr_array_index (apsarray, i);
      NMAccessPoint *candidate = NULL;

      for (j = 0; j < collapsedaps->len; j++)
        {
          NMAccessPoint *tmp = (NMAccessPoint*) g_ptr_array_index (collapsedaps, j);
          if (nm_utils_same_ssid (nm_access_point_get_ssid (ap),
                                  nm_access_point_get_ssid (tmp),
                                  TRUE))
            {
              candidate = tmp;
              break;
            }
        }
      if (candidate != NULL)
        {
          guint8 strength1, strength2;
          strength1 = nm_access_point_get_strength (ap);
          strength2 = nm_access_point_get_strength (candidate);

          /* If active access point or stronger then it replaces the previous AP
           * with the same SSID */
          if ((candidate == active) || (strength2 > strength1))
            collapsedaps->pdata[i] = candidate;

          continue;
        }
      g_ptr_array_add (collapsedaps, ap);
    }
  return collapsedaps;
}

/* This is a sort function to pass on to g_ptr_array_sort_with_data */
static gint
wifi_aps_sort (NMAccessPoint       **a,
               NMAccessPoint       **b,
               ActiveAPConnections  *sort_data)
{
  /* Note that we are sorting from top to bottom so we return -1 when the
   * accesspoint has a higher relevance instead of 1 */
  NMAccessPoint *ap1 = *a;
  NMAccessPoint *ap2 = *b;
  gint           result;
  GSList        *ap1_connections, *ap2_connections;

  guint8 strength1 = nm_access_point_get_strength (ap1);
  guint8 strength2 = nm_access_point_get_strength (ap2);

  /* If one of them is the active connection, that one goes up */
  if (sort_data->active_ap == ap1)
    return -1;
  if (sort_data->active_ap == ap2)
    return  1;

  ap1_connections = nm_access_point_filter_connections (ap1, sort_data->connections);
  ap2_connections = nm_access_point_filter_connections (ap2, sort_data->connections);

  /* If only one has an existing connection, that one goes up */
  if (ap1_connections != NULL && ap2_connections == NULL)
    {
      g_slist_free (ap1_connections);
      return -1;
    }
  else if (ap1_connections == NULL && ap2_connections != NULL)
    {
      g_slist_free (ap2_connections);
      return 1;
    }

  /* If both or none of them has connections, we sort by signal strength */
  if (strength1 == strength2)
    result = 0;
  if (strength1 > strength2)
    result = -1;
  else
    result = 1;

  /* If they have the same strenght, we order alphabetically */
  if (result == 0)
  {
    gchar *ap1id = nm_utils_ssid_to_utf8 (nm_access_point_get_ssid (ap1));
    gchar *ap2id = nm_utils_ssid_to_utf8 (nm_access_point_get_ssid (ap2));

    result = g_strcmp0 (ap2id, ap1id);

    g_free (ap1id);
    g_free (ap2id);
  }

  g_slist_free (ap1_connections);
  g_slist_free (ap2_connections);
  return result;
}

static void
wifi_populate_accesspoints (DbusmenuMenuitem *parent,
                            NMClient         *client,
                            NMDeviceWifi     *device,
                            gint             *id)
{
  gint                  i;
  GPtrArray            *sortedarray;
  const GPtrArray      *apsarray = nm_device_wifi_get_access_points (device);
  NMRemoteSettings     *rs;
  GSList               *rs_connections;
  GSList               *dev_connections;
  ActiveAPConnections   sort_data;

  /* Access point list is empty */
  if (apsarray == NULL)
    return;

  rs              = nm_remote_settings_new (NULL);
  rs_connections  = nm_remote_settings_list_connections (rs);
  dev_connections = nm_device_filter_connections (NM_DEVICE (device), rs_connections);

  /* Remove duplicated SSIDs and select the one which is acive or with higher strenght*/
  sortedarray = filter_access_points (apsarray, device);

  /* Sort access points */
  sort_data.connections = dev_connections;
  g_object_get (device,
                "active-access-point", &(sort_data.active_ap),
                NULL);

  g_ptr_array_sort_with_data (sortedarray,
                              (GCompareDataFunc)wifi_aps_sort,
                              (gpointer)&sort_data);

  for (i=0; i < sortedarray->len; i++)
    {
      gboolean          is_adhoc   = FALSE;
      gboolean          is_secure  = FALSE;
      NMAccessPoint    *ap = g_ptr_array_index (sortedarray, i);
      DbusmenuMenuitem *ap_item = DBUSMENU_MENUITEM (dbusmenu_accesspointitem_new_with_id ((*id)++));
      char             *utf_ssid;
      ClientDevice     *cd = g_malloc (sizeof (ClientDevice));
      cd->device = NM_DEVICE (device);
      cd->client = client;

      utf_ssid = nm_utils_ssid_to_utf8 (nm_access_point_get_ssid (ap));

      if (nm_access_point_get_mode (ap) == NM_802_11_MODE_ADHOC)
        is_adhoc  = TRUE;
      if (nm_access_point_get_flags (ap) == NM_802_11_AP_FLAGS_PRIVACY)
        is_secure = TRUE;

      if (sort_data.active_ap && g_strcmp0 (nm_access_point_get_bssid (ap),
                                            nm_access_point_get_bssid (sort_data.active_ap)) == 0)
      {
          dbusmenu_menuitem_property_set_int (DBUSMENU_MENUITEM (ap_item),
                                              DBUSMENU_MENUITEM_PROP_TOGGLE_STATE,
                                              DBUSMENU_MENUITEM_TOGGLE_STATE_CHECKED);
      }


      dbusmenu_accesspointitem_bind_accesspoint (DBUSMENU_ACCESSPOINTITEM (ap_item), ap);
      dbusmenu_accesspointitem_bind_device      (DBUSMENU_ACCESSPOINTITEM (ap_item), NM_DEVICE (device));

      dbusmenu_menuitem_property_set (ap_item, DBUSMENU_MENUITEM_PROP_LABEL, utf_ssid);
      dbusmenu_menuitem_property_set (ap_item, DBUSMENU_MENUITEM_PROP_TOGGLE_TYPE, DBUSMENU_MENUITEM_TOGGLE_RADIO);

      dbusmenu_menuitem_property_set (ap_item, "x-wifi-bssid", nm_access_point_get_bssid (ap));
      dbusmenu_menuitem_property_set (ap_item, "type", "x-system-settings");
      dbusmenu_menuitem_property_set (ap_item, "x-tablet-widget", "unity.widgets.systemsettings.tablet.accesspoint");

      dbusmenu_menuitem_property_set_int  (ap_item, "x-wifi-strength",   nm_access_point_get_strength (ap));
      dbusmenu_menuitem_property_set_bool (ap_item, "x-wifi-is-adhoc",   is_adhoc);
      dbusmenu_menuitem_property_set_bool (ap_item, "x-wifi-is-secure",  is_secure);

      dbusmenu_menuitem_child_append  (parent, ap_item);

      g_signal_connect_data (ap_item, "item-activated",
                             G_CALLBACK (access_point_selected),
                             cd,
                             destroy_client_device,
                             0);

      g_free (utf_ssid);
    }
  g_ptr_array_free (sortedarray, TRUE);
}

static void
wireless_toggle_activated (DbusmenuMenuitem *toggle,
                           guint             timestamp,
                           gpointer          data)
{
  NMClient *client = (NMClient*)data;
  gboolean enabled = nm_client_wireless_get_enabled (client);

  nm_client_wireless_set_enabled (client, !enabled);
  dbusmenu_menuitem_property_set_int (toggle, DBUSMENU_MENUITEM_PROP_TOGGLE_STATE, !enabled);
}

static void
device_state_changed (NMDevice            *device,
                      NMDeviceState        new_state,
                      NMDeviceState        old_state,
                      NMDeviceStateReason  reason,
                      DbusmenuMenuitem    *item)
{
  switch (new_state)
    {
    case NM_DEVICE_STATE_UNKNOWN:
    case NM_DEVICE_STATE_DISCONNECTED:
    case NM_DEVICE_STATE_UNMANAGED:
    case NM_DEVICE_STATE_ACTIVATED:
      dbusmenu_menuitem_property_set_bool (item, "x-busy", FALSE);
      break;
    default:
      dbusmenu_menuitem_property_set_bool (item, "x-busy", TRUE);
    }
}

static void
wireless_state_changed (NMClient         *client,
                        GParamSpec       *pspec,
                        DbusmenuMenuitem *item)
{
  if (g_strcmp0 (g_param_spec_get_name (pspec),
                 NM_CLIENT_WIRELESS_ENABLED) == 0)
  {
    gboolean enabled;
    g_object_get (client,
                  NM_CLIENT_WIRELESS_ENABLED, &enabled,
                  NULL);
    dbusmenu_menuitem_property_set_bool (item, DBUSMENU_MENUITEM_PROP_VISIBLE, enabled);
  }
/*  if (g_strcmp0 (g_param_spec_get_name (pspec),
                 NM_CLIENT_WIRELESS_HARDWARE_ENABLED) == 0)
  {
  }*/
}

static void
wifi_device_handler (DbusmenuMenuitem *parent,
                     NMClient         *client,
                     NMDevice         *device,
                     gint             *id)
{
  /* Wifi enable/disable toggle */
  gboolean          wifienabled   = nm_client_wireless_get_enabled (client);
  DbusmenuMenuitem *togglesep     = dbusmenu_menuitem_new_with_id ((*id)++);
  DbusmenuMenuitem *toggle        = dbusmenu_menuitem_new_with_id ((*id)++);

  DbusmenuMenuitem *networksgroup = dbusmenu_menuitem_new_with_id ((*id)++);

  dbusmenu_menuitem_property_set (togglesep, DBUSMENU_MENUITEM_PROP_LABEL, "Turn Wifi On/Off");
  dbusmenu_menuitem_property_set (togglesep, DBUSMENU_MENUITEM_PROP_TYPE,  "x-system-settings");
  dbusmenu_menuitem_property_set (togglesep, "x-tablet-widget",            "unity.widgets.systemsettings.tablet.sectiontitle");

  dbusmenu_menuitem_property_set (toggle, DBUSMENU_MENUITEM_PROP_TOGGLE_TYPE, DBUSMENU_MENUITEM_TOGGLE_CHECK);
  dbusmenu_menuitem_property_set (toggle, DBUSMENU_MENUITEM_PROP_LABEL, "Wifi");
  dbusmenu_menuitem_property_set_int (toggle, DBUSMENU_MENUITEM_PROP_TOGGLE_STATE, wifienabled);

  g_signal_connect (toggle, "item-activated",
                    G_CALLBACK (wireless_toggle_activated),
                    client);

  dbusmenu_menuitem_child_append (parent, togglesep);
  dbusmenu_menuitem_child_append (parent, toggle);


  /* Access points */
  if (wifienabled)
    {
      dbusmenu_menuitem_property_set (networksgroup, DBUSMENU_MENUITEM_PROP_LABEL, "Select wireless network");
      dbusmenu_menuitem_property_set (networksgroup, "type",             "x-system-settings");
      device_state_changed (device,
                            nm_device_get_state (device),
                            NM_DEVICE_STATE_UNKNOWN,
                            NM_DEVICE_STATE_REASON_NONE,
                            networksgroup);
      dbusmenu_menuitem_property_set (networksgroup, "x-group-class",    "accesspoints");
      dbusmenu_menuitem_property_set (networksgroup, "children-display", "inline");
      dbusmenu_menuitem_property_set (networksgroup, "x-tablet-widget",  "unity.widgets.systemsettings.tablet.sectiontitle");
      dbusmenu_menuitem_child_append (parent, networksgroup);
      wifi_populate_accesspoints (networksgroup, client, NM_DEVICE_WIFI (device), id);
    }

  /* Network status handling */
  /* FIXME: We may leak memory if we end up having to delete the networksgroup menuitem */
  g_object_ref (networksgroup);
  g_signal_connect (device, "state-changed",
                    G_CALLBACK (device_state_changed),
                    networksgroup);


  /* TODO: Remove this when toggle is removed */
  g_signal_connect (client, "notify",
                    G_CALLBACK (wireless_state_changed),
                    networksgroup);
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
      NMDeviceState state = nm_device_get_state (device);

      if (state == NM_DEVICE_STATE_UNMANAGED ||
          state == NM_DEVICE_STATE_UNAVAILABLE) /* TODO: Inform the user about the situation */
          continue;

      switch (type)
        {
        case NM_DEVICE_TYPE_WIFI:
          wifi_device_handler (root, client, device, &id);
          break;
        }
    }
  /* TODO: Advance tab (per device?) */
  /* TODO: Airplane mode */
  return;
}

static void
name_lost (GDBusConnection * connection, const gchar * name, gpointer user_data)
{
  g_main_loop_quit ((GMainLoop*)user_data);
  return;
}

int
main (int argc, char** argv)
{
  GMainLoop *loop;

  g_type_init ();
  loop = g_main_loop_new (NULL, FALSE);

  g_bus_own_name(G_BUS_TYPE_SESSION,
                 "com.ubuntu.networksettings",
                 G_BUS_NAME_OWNER_FLAGS_NONE,
                 on_bus,
                 NULL,
                 name_lost,
                 loop,
                 NULL);

  g_main_loop_run (loop);
  g_main_loop_unref (loop);
  return 0;
}
