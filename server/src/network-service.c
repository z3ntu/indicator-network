#include <gio/gio.h>
#include <nm-client.h>
#include <libdbusmenu-glib/dbusmenu-glib.h>

GMainLoop *loop;

static void
on_bus (GDBusConnection * connection, const gchar * name, gpointer user_data)
{
  int                i, id;
  const GPtrArray   *devarray;
  NMClient          *client;
  NMDevice         **devices;
	DbusmenuServer    *server = dbusmenu_server_new("/com/canonical/networksettings");
	DbusmenuMenuitem  *root   = dbusmenu_menuitem_new_with_id (0);
	id++;
	
	dbusmenu_server_set_root (server, root);
	
	/* TODO: Enable/disable Wifi */
	
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
          break;
        }
    }
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
	               "com.canonical.networksettings",
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
