// vim: tabstop=4 noexpandtab shiftwidth=4 softtabstop=4
/*
 * Copyright 2013 Canonical Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors:
 *      Nick Dedekind <nick-dedekind@canonical.com
 */

using NM;
using Notify;

namespace Network
{
  public class MobileSimManager : GLib.Object
  {
    public bool sim_installed { get; private set; default = false; }
    public bool pin_required  { get { return required_pin != "none" && required_pin != ""; } }
    public bool pin_unlocking { get; private set; default = false; }

    private NM.Client                  client;
    private NM.DeviceModem             device;
    private GLib.DBusConnection        conn;
    private string                     namespace;
    private oFono.SIMManager?          simmanager = null;
    private oFono.Modem?               ofono_modem = null;
    private HashTable<string, uchar>   retries = new HashTable<string, uchar>(str_hash, str_equal);
    private List<CurrentNotification?> notifications = new List<CurrentNotification?> ();

    // Sim Unlocking
    private string _required_pin = "none";
    private string  last_required_pin = "";
    private string  puk_code = "";
    private string  old_pin = "";
    private string  new_pin = "";
    private int     export_id = 0;

    private const string APPLICATION_ID  = "com.canonical.indicator.network";
    private const string SIM_UNLOCK_MENU_PATH = "/com/canonical/indicator/network/unlocksim";
    private const string SIM_UNLOCK_ACTION_PATH = "/com/canonical/indicator/network/unlocksim";
    private const string OFONO_ERROR_FAILED = "org.ofono.Error.Failed";

    struct CurrentNotification {
      public int                id;
      public Notification?      notification;
      public uint               unlock_menu_export_id;
      public uint               unlock_actions_export_id;
      public Menu?              unlock_menu;
      public SimpleActionGroup? unlock_actions;

      public CurrentNotification() {
        this.id = 0;
        this.notification = null;
        this.unlock_menu_export_id = 0;
        this.unlock_actions_export_id = 0;
        this.unlock_menu = null;
        this.unlock_actions = null;
      }
    }

    private string required_pin {
      get { return _required_pin; }
      set { if (_required_pin != value) { _required_pin = value; notify_property("pin-required"); } }
    }

    delegate void PinCancelCallback ();
    delegate void PinEnteredCallback (string pin);

    public MobileSimManager (NM.Client client, DeviceModem device, GLib.DBusConnection conn, string namespace)
    {
      this.client = client;
      this.device = device;
      this.conn = conn;
      this.namespace = namespace;

      create_ofono_sim_manager();
    }

    public bool send_unlock_notification ()
    {
      if (pin_required == false) {
        clear_stored_data();
        return false;
      }

      if (send_unlock (required_pin == "puk" ? _("Please enter SIM PUK") : _("Please enter SIM PIN"),
                        required_pin,
                        required_pin == "puk" ? 8 : 4,
                        pin_unlock_entered)) {
        debug(@"SIM Unlock: $required_pin");
        last_required_pin = required_pin;
        pin_unlocking = true;
        debug(@"PIN LOCKING: $pin_unlocking");
        return true;
      } else {
        debug(@"SIM unlock notification request failed");
        clear_stored_data();
        return false;
      }
    }

    public bool send_change_pin_notification()
    {
      if (send_unlock(_("Please enter SIM PIN"), "pin", 4, pin_change_old_entered)) {
        debug(@"SIM change. Sent old pin request");
        return true;
      } else {
        debug(@"SIM change. Old pin notification failed");
        return false;
      }
    }

    private bool send_unlock(string title, string? retry_type, int pin_length, PinEnteredCallback? pin_entered_callback)
    {
      string body = "";
      if (retry_type != null) {
        uchar pin_retries = 0;
        if (retries.lookup_extended(retry_type, null, out pin_retries)) {
          if (pin_retries > 1) {
            body = _(@"$(pin_retries) attempts remaining");
          } else if (pin_retries == 1) {
            body = _(@"$(pin_retries) attempt remaining");
          } else {
            debug(@"No pin retries remaining");
            send_fail_notification(_("No retries remaining"));
            return false;
          }
        }
      }

      // set the export hints
      string exported_action_path = @"$(SIM_UNLOCK_ACTION_PATH)$(export_id)";
      string exported_menu_path = @"$(SIM_UNLOCK_MENU_PATH)$(export_id)";
      export_id++;

      VariantBuilder menu_model_actions = new VariantBuilder (new VariantType ("a{sv}") );
      menu_model_actions.add ("{sv}", "notifications", new Variant.string (exported_action_path));

      VariantBuilder menu_model_paths = new VariantBuilder (new VariantType ("a{sv}") );
      menu_model_paths.add ("{sv}", "busName", new Variant.string (APPLICATION_ID));
      menu_model_paths.add ("{sv}", "menuPath", new Variant.string (exported_menu_path));
      menu_model_paths.add ("{sv}", "actions", menu_model_actions.end ());

      // create the menu
      var menu = new Menu ();
      var pin_unlock = new MenuItem ("", "notifications." + namespace + ".simunlock");
      pin_unlock.set_attribute ("x-canonical-type", "s", "com.canonical.snapdecision.pinlock");
      pin_unlock.set_attribute ("x-canonical-pin-length", "i", pin_length);
      menu.append_item (pin_unlock);

      // create the actions
      var actions = new SimpleActionGroup();
      var unlockpin_item = new SimpleAction.stateful(namespace + ".simunlock", VariantType.BOOLEAN, new Variant.string(""));
      unlockpin_item.activate.connect ((ac, value) => {
        if (value.get_boolean() == false) {
          unlock_cancelled();
        }
      });
      if (pin_entered_callback != null) {
        unlockpin_item.change_state.connect ((ac, value) => {
          pin_entered_callback(value.get_string());
        });
      }
      actions.insert (unlockpin_item);

      // export the menu
      uint menu_export_id = 0;
      try {
        menu_export_id = conn.export_menu_model(exported_menu_path, menu);
      } catch (Error e) {
        warning(@"Unable to export sim unlock menu model for '$(device.get_iface())': $(e.message)");
        clear_stored_data();
        return false;
      }

      // export the actions
      uint actions_export_id = 0;
      try {
        actions_export_id = conn.export_action_group(exported_action_path, actions as ActionGroup);
      } catch (Error e) {
        warning(@"Unable to export sim unlock actions group for '$(device.get_iface())': $(e.message)");
        if (menu_export_id != 0) {
          conn.unexport_menu_model(menu_export_id);
        }
        clear_stored_data();
        return false;
      }

      // create and show the notification
      var notification = new Notification(title, body, "");
      notification.set_hint_string ("x-canonical-snap-decisions", "true");
      notification.set_hint ("x-canonical-private-menu-model", menu_model_paths.end ());

      CurrentNotification new_notification = CurrentNotification() {
        notification = notification,
        unlock_menu_export_id = menu_export_id,
        unlock_actions_export_id = actions_export_id,
        unlock_menu = menu,
        unlock_actions = actions
      };

      try {
        new_notification.notification.closed.connect (notification_closed);
        new_notification.notification.show ();

        notifications.append(new_notification);
      } catch (Error e) {
        warning(@"Unable to create sim unlock unlock notification for '$(device.get_iface())': $(e.message)");
        clear_notification(new_notification);
        clear_stored_data();
        return false;
      }
      return true;
    }

    private void send_fail_notification (string title)
    {
      try {
        var notification = new Notification(title, "", "");
        notification.closed.connect (notification_closed);
        notification.show ();
      } catch (Error e) {
        warning(@"Unable to create sim unlock unlock notification for '$(device.get_iface())': $(e.message)");
        return;
      }
    }

    private void create_ofono_sim_manager()
    {
      try {
        if (ofono_modem == null) {
          ofono_modem = Bus.get_proxy_sync (BusType.SYSTEM, "org.ofono", device.get_iface(), DBusProxyFlags.DO_NOT_AUTO_START);

          ofono_modem.property_changed.connect((prop, value) => {
            if (prop == "Interfaces") {
              create_ofono_sim_manager();
            }
          });
        }

        var modem_properties = ofono_modem.get_properties();
        var interfaces = modem_properties.lookup("Interfaces");

        if (interfaces == null) {
          debug(@"Modem '$(device.get_iface())' doesn't have voice support, no interfaces");
          return;
        }

        if (!Utils.variant_contains(interfaces, "org.ofono.SimManager")) {
          debug(@"Modem '$(device.get_iface())' doesn't have SIM management support only: $(interfaces.print(false))");
          return;
        }
      } catch (Error e) {
        warning(@"Unable to get oFono modem properties for '$(device.get_iface())': $(e.message)");
        return;
      }

      try {
        /* Initialize the SIM Manager */
        simmanager = Bus.get_proxy_sync (BusType.SYSTEM, "org.ofono", device.get_iface(), DBusProxyFlags.DO_NOT_AUTO_START);
        simmanager.property_changed.connect(simmanager_property);
        var simprops = simmanager.get_properties();
        simprops.foreach((k, v) => {
          simmanager_property(k, v);
        });

      } catch (Error e) {
        warning(@"Unable to get oFono information from $(device.get_iface()): $(e.message)");
        simmanager = null;
      }

      return;
    }

    /* Properties from the SIM manager allow us to know the state of the SIM
       that we've got installed. */
    private void simmanager_property (string prop, Variant value)
    {
      switch (prop) {
        case "Present": {
          sim_installed = value.get_boolean();
          break;
        }
        case "PinRequired": {
          required_pin = value.get_string();
          break;
        }
        case "Retries": {
          if (value.get_type_string() == "a{sy}") {
            for (int i = 0; i < value.n_children(); i++) {
              string? key = null;
              uchar tries = 0;
              value.get_child(i, "{sy}", &key, &tries);

              retries[key] = tries;
            }
          }
          break;
        }
      }
    }

    private void pin_unlock_entered (string pin)
    {
      if (required_pin == last_required_pin) {
        bool retry = false;
        if (required_pin == "puk") {
            // if it's a puk, we need to reset the pin.
            close_all_notifications(true);
            if (send_unlock(_("Enter new PIN code"), null, 4, pin_reset_new_entered)) {
              debug("SIM pin request. Sent new pin request");
              puk_code = pin;
            } else {
              warning("SIM pin request. New pin notification failed");
            }
        } else  if (!enter_pin(required_pin, pin, out retry)) {
          warning("SIM pin request. Failed. retry=$(retry)");
          close_all_notifications(true);
          if (retry) {
            send_unlock_notification();
          } else {
            send_fail_notification("An unexpected error occurred.");
          }
        } else {
          debug("SIM pin request. Done");
          close_all_notifications(true);
        }
      } else {
        warning(@"Required pin type changed. old=$(last_required_pin), new=$(required_pin)");
        close_all_notifications(true);
      }
    }

    private void pin_reset_new_entered (string pin)
    {
      close_all_notifications(false);

      if (send_unlock(_("Please confirm PIN code"), null, 4, pin_reset_confirm_entered)) {
        debug("SIM reset pin request. Sent confirm pin request");
        new_pin = pin;
      } else {
        warning("SIM reset pin request. Confirm pin notification failed");
        clear_stored_data();
      }
    }

    private void pin_reset_confirm_entered (string pin)
    {
      if (new_pin == pin) {
        if (reset_pin("puk", puk_code, pin)) {
          debug("SIM reset pin request. Done.");
          close_all_notifications(true);
        } else {
          warning("SIM reset request. Failed.");
          close_all_notifications(true);
          send_fail_notification("Failed to reset pin.");
        }
      } else {
        warning("SIM reset request. Pin codes did not match");
        close_all_notifications(true);
        send_fail_notification(_("Pin codes did not match"));
      }
    }

    private void pin_change_old_entered (string pin)
    {
      close_all_notifications(true);

      if (send_unlock(_("Please enter new SIM PIN"), "pin", 4, pin_change_new_entered)) {
        debug("SIM change pin request. Sent new pin request");
        old_pin = pin;
      } else {
        warning("SIM change pin request. New pin notification failed");
        clear_stored_data();
      }
    }

    private void pin_change_new_entered (string pin)
    {
      close_all_notifications(false);

      if (send_unlock(_("Please confirm PIN code"), null, 4, pin_change_confirm_entered)) {
        debug("SIM change pin request. Sent confirm pin request");
        new_pin = pin;
      } else {
        warning("SIM change pin request. Confirm pin notification failed");
        clear_stored_data();
      }
    }

    private void pin_change_confirm_entered (string pin)
    {
      if (new_pin == pin) {
        if (change_pin("pin", old_pin, new_pin)) {
          debug("SIM change pin request. Done.");
          close_all_notifications(true);
        } else {
          warning("SIM change pin request. Failed.");
          close_all_notifications(true);
          send_fail_notification("Failed to change pin.");
        }
      } else {
        close_all_notifications(true);
        send_fail_notification(_("Pin codes did not match"));
      }
    }

    private bool enter_pin(string type, string pin, out bool retry)
    {
      retry = false;
      try {
        if (simmanager != null) {
          debug(@"SimManager: Entering $(type) pin");
          simmanager.enter_pin(type, pin);
          return true;
        }
      } catch (DBusError e) {
        warning(@"Failed to enter $(type) pin for '$(device.get_iface())': $(e.message)");
      } catch (IOError e) {
        warning(@"Failed to enter $(type) pin for '$(device.get_iface())': $(e.message)");
        if (check_ofono_error(e, OFONO_ERROR_FAILED)) {
          retry = true;
        }
      }
      return false;
    }

    private bool reset_pin(string type, string puk, string pin)
    {
      try {
        if (simmanager != null) {
          debug(@"SimManager: Resetting");
          simmanager.reset_pin(type, puk, pin);
          return true;
        }
      } catch (DBusError e) {
        warning(@"Failed to reset pin for '$(device.get_iface())': $(e.message)");
      } catch (IOError e) {
        warning(@"Failed to reset pin for '$(device.get_iface())': $(e.message)");
      }
      return false;
    }

    private bool change_pin(string type, string old_pin, string new_pin)
    {
      try {
        if (simmanager != null) {
          debug(@"SimManager: Changing $type pin");
          simmanager.change_pin(type, old_pin, new_pin);
          return true;
        }
      } catch (DBusError e) {
        warning(@"Failed to change pin for '$(device.get_iface())': $(e.message)");
      } catch (IOError e) {
        warning(@"Failed to change pin for '$(device.get_iface())': $(e.message)");
      }
      return false;
    }

    private void unlock_cancelled() {
      debug(@"SIM notification cancelled");
      close_all_notifications(true);
    }

    private void close_all_notifications(bool reset_data) {
      unowned List<CurrentNotification?>? element = notifications.first ();
      while (element != null) {
        CurrentNotification? entry = element.data;
        if (entry != null) {
          try {
            if (entry.unlock_menu_export_id != 0) {
              conn.unexport_menu_model(entry.unlock_menu_export_id);
            }
            if (entry.unlock_actions_export_id != 0) {
              conn.unexport_action_group(entry.unlock_actions_export_id);
            }
            debug(@"closing notification $(entry.notification.id)");
            entry.notification.close();
          } catch (Error e) {
            warning("Failed to close notification for '$(device.get_iface())': $(e.message)");
          }
        }
        element = notifications.next;
      }
      if (reset_data) {
        clear_stored_data();
      }
    }

    private void clear_stored_data()
    {
      new_pin = "";
      old_pin = "";
      puk_code = "";
    }

    private void clear_notification (CurrentNotification? notification)
    {
      if (notification.unlock_menu_export_id != 0) {
        conn.unexport_menu_model(notification.unlock_menu_export_id);
      }

      if (notification.unlock_actions_export_id != 0) {
        conn.unexport_action_group(notification.unlock_actions_export_id);
      }
    }

    private void notification_closed (Notification? notification)
    {
      unowned List<CurrentNotification?>? element = notifications.first ();
      while (element != null) {
        unowned CurrentNotification? entry = element.data;

        if (entry != null && notification.id == entry.notification.id) {
          debug(@"notification_closed $(notification.id)");

          clear_notification(entry);
          notifications.delete_link(element);
          break;
        }
        element = notifications.next;
      }
      if (notifications.length () == 0) {
        pin_unlocking = false;
      }
    }

    private bool check_ofono_error(Error e, string value)
    {
      return DBusError.get_remote_error(e) == value;
    }

  }
}
