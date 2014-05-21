

//enabled_item = new MenuItem(_("Wired"), "indicator." + device.get_iface() + ".device-enabled");
//protected override void enable_device ()
//{
//    var conn = new NM.Connection();

//    var swired = new NM.SettingWired();
//    conn.add_setting(swired);

//    var sconn = new NM.SettingConnection();
//    sconn.id = "Auto Ethernet";
//    sconn.type = NM.SettingWired.SETTING_NAME;
//    sconn.autoconnect = true;
//    sconn.uuid = NM.Utils.uuid_generate();
//    conn.add_setting(sconn);

//    client.add_and_activate_connection(conn, this.device, "/", null);
//}


//void updateIcon()
//{
//   if (act_dev == null) {
//        icon_name = "nm-no-connection";
//        a11ydesc = "Network (none)";
//        return;
//    }

//   case NM.DeviceType.ETHERNET:
//       icon_name = "network-wired";
//       a11ydesc = _("Network (wired)");
//       break;
//}

