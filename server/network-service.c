#include <glib.h>
#include <gio/gio.h>
#include <nm-client.h>
#include <nm-device.h>
#include <libdbusmenu-glib/dbusmenu-glib.h>

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
extern void
wifi_device_handler (DbusmenuMenuitem *parent,
                     NMClient         *client,
                     NMDevice         *device,
                     gint             *id);

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
