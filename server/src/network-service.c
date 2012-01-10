#include <nm-client.h>
#include "settingsmenu.h"

int
main (int argc, char** argv)
{
  int               i;
  const GPtrArray  *devarray;
  NMClient         *client;
  NMDevice        **devices;
	UnitySettingsSettings *settings;
	
  g_type_init ();  
  client = nm_client_new ();
  devarray = nm_client_get_devices (client);

	settings = unity_settings_settings_new ();

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

  g_object_unref (client);
  return 0;
}
