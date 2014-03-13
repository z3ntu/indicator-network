#if 0
public Wifi (GLibLocal.ActionMuxer muxer) {
    GLib.Object(
        namespace: "wifi-settings",
        muxer: muxer
    );

    settings = new GLib.Settings ("com.canonical.indicator.network");

    var joinact = settings.create_action("auto-join-previous");
    actions.add_action(joinact);

    var promptact = settings.create_action("prompt-on-new-wifi-ap");
    actions.add_action(promptact);

    var joinitem = new MenuItem(_("Auto-join previous networks"), "indicator.wifi-settings.auto-join-previous");
    _menu.append_item(joinitem);

    /* Commented out for Phone V1 that doesn't have this feature */
    /* var promptitem = new MenuItem(_("Prompt when not connected"), "indicator.wifi-settings.prompt-on-new-wifi-ap"); */
    /* _menu.append_item(promptitem); */

    /* Commented out for Phone V1 that doesn't have this feature */
    /* var captionitem = new MenuItem(_("Lists available wi-fi networks, if any, when you're using cellular data."), null); */
    /* _menu.append_item(captionitem); */

    //        settings = new GLib.Settings ("com.canonical.indicator.network");
    //        settings.changed["auto-join-previous"].connect((k) => {
    //            if (client.wireless_get_enabled()) {
    //                device.set_autoconnect(settings.get_boolean("auto-join-previous"));
    //            }
    //        });
    //        device.set_autoconnect(settings.get_boolean("auto-join-previous"));

    // bool show_settings
}
#endif
