/*
 * Copyright (C) 2014 Canonical, Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3, as published
 * by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranties of
 * MERCHANTABILITY, SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors:
 *     Antti Kaijanmäki <antti.kaijanmaki@canonical.com>
 */

#include "modem-manager.h"

#include "sim-unlock-dialog.h"

#include "notify-cpp/notification.h"

#include "dbus-cpp/services/ofono.h"
#include <core/dbus/asio/executor.h>

#include <core/dbus/dbus.h>
#include <core/dbus/service_watcher.h>

class ModemManager::Private
{
public:
    enum class RequiredType
    {
        pin,
        puk
    };

    core::Property<std::set<Modem::Ptr>> modems;

    std::string newPin;
    std::string oldPin;
    std::string pukCode;

    std::set<SimUnlockDialog::Ptr> dialogs;
    std::set<notify::Notification::Ptr> notifications;

    bool pinUnlocking;

    std::thread ofono_worker;
    std::shared_ptr<core::dbus::Bus> bus;
    std::shared_ptr<org::ofono::Service> ofono;

    std::unique_ptr<core::dbus::ServiceWatcher> watcher;
    std::unique_ptr<core::dbus::DBus> dbus;

    Private()
    {
        bus = std::make_shared<core::dbus::Bus>(core::dbus::WellKnownBus::system);

        auto executor = core::dbus::asio::make_executor(bus);
        bus->install_executor(executor);
        ofono_worker = std::move(std::thread([this](){
            bus->run();
        }));

        dbus.reset(new core::dbus::DBus(bus));

        auto names = dbus->list_names();
        if (std::find(names.begin(), names.end(), org::ofono::Service::name()) != names.end()) {
            ofono_appeared();
        } else {
            ofono_disappeared();
        }
        watcher = dbus->make_service_watcher(org::ofono::Service::name());
        watcher->service_registered().connect([this](){
            ofono_appeared();
        });
        watcher->service_unregistered().connect([this](){
            ofono_disappeared();
        });
    }

    ~Private()
    {
        bus->stop();
        if (ofono_worker.joinable())
            ofono_worker.join();
    }

    void ofono_appeared()
    {
        try {
            ofono = std::make_shared<org::ofono::Service>(bus);
        } catch (std::exception &e) {
            std::cerr << e.what() << std::endl;
        }

        ofono->manager->modems.changed().connect([this](std::map<core::dbus::types::ObjectPath, org::ofono::Interface::Modem::Ptr> ofonoModems){
            modems_changed(ofonoModems);
        });
        modems_changed(ofono->manager->modems);
    }

    void modems_changed(std::map<core::dbus::types::ObjectPath, org::ofono::Interface::Modem::Ptr> ofonoModemsMap)
    {
        std::set<org::ofono::Interface::Modem::Ptr> ofonoModems;
        for (auto element : ofonoModemsMap)
            ofonoModems.insert(element.second);

        auto currentModems = modems.get();

        std::set<org::ofono::Interface::Modem::Ptr> current;
        for (auto modem : currentModems)
            current.insert(modem->ofonoModem());

        std::set<org::ofono::Interface::Modem::Ptr> removed;
        std::set_difference(current.begin(), current.end(),
                            ofonoModems.begin(),ofonoModems.end(),
                            std::inserter(removed, removed.begin()));

        std::set<org::ofono::Interface::Modem::Ptr> added;
        std::set_difference(ofonoModems.begin(), ofonoModems.end(),
                            current.begin(), current.end(),
                            std::inserter(added, added.begin()));

        for (auto iter = currentModems.cbegin(); iter != currentModems.cend(); ++iter) {
            if (removed.find((*iter)->ofonoModem()) != removed.end()) {
                iter = currentModems.erase(iter);
                --iter; // call -- as loop does ++ next;
                continue;
            }
        }

        for (auto ofonoModem : added) {
            currentModems.insert(std::make_shared<Modem>(ofonoModem));
        }

        modems.set(currentModems);
    }

    void ofono_disappeared()
    {
        ofono.reset();
        modems.set(std::set<Modem::Ptr>());
    }

    void clearStoredData()
    {
        newPin = "";
        oldPin = "";
        pukCode = "";
    }


    void sendUnlock(const std::string &title,
                    std::function<void(std::string)> callback = std::function<void(std::string)>())
    {
        std::string body;
//        gchar *retries_str = nullptr;
//        int tmp = retries.get();
//        if (tmp > 1) {
//            retries_str = g_strdup_printf(_("%d attempts remaining"), tmp);
//        } else if (tmp == 1) {
//            retries_str = gstrdup_printf(_("1 attempt remaining"));
//        } else {
//            std::cerr << "No retries left.";
//            sendFailNotification(_("No retries remaining"));
//            return false;
//        }
//        if (retries_str) {
//            body = {retries_str};
//            g_free(retries_str);
//        }

#if 0
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
        auto dialog = std::make_shared<SimUnlockDialog>(title, body, 34533);
        dialog->cancelled().connect([this](){
            unlockCancelled();
        });
        if (callback) {
            dialog->pinEntered().connect([callback](std::string pin){
                callback(pin);
            });
        }

        dialog->closed().connect([this, &dialog](){
            dialogs.erase(dialog);
            if (dialogs.size() == 0)
                pinUnlocking = false;
        });
        dialog->show();
        dialogs.insert(dialog);
#endif

    }

    void sendFailNotification(const std::string &title)
    {
        notify::Notification::Ptr n = std::make_shared<notify::Notification>(title, "", "");
        n->closed().connect([this, &n](){
            notifications.erase(n);
        });
        try {
            n->show();
        } catch(std::runtime_error &e) {
            std::cerr << "Failed to send fail notification: " << e.what();
        }
    }

    void sendUnlockNotification(RequiredType type)
    {
#if 0
        if (!pinRequired)
            return;

        std::string message;
        std::uint8_t length;

        switch (type) {
        case RequiredType::pin:
            message = _("Please enter SIM PIN");
            length = 4; /// @todo, check spec what is the maximum allowed length for PIN
            break;
        case RequiredType::puk:
            message = _("Please enter SIM PUK");
            length = 4; /// @todo, check spec what is the maximum allowed length for PUK
        }

        sendUnlock(message, type, length, [this, type](){
              if (type == CodeType::puk) {
                  closeAllNotifications(true);
                  sendUnlock(_("Enter new PIN code"), null, 4, pin_reset_new_entered)) {
                    debug("SIM pin request. Sent new pin request");
                    puk_code = pin;
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
        });
#endif
#if 0
        if (send_unlock (required_pin == "puk" ? : ,
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
#endif
    }

    bool sendChangePinNotification()
    {
#if 0
        if (send_unlock(_("Please enter SIM PIN"), "pin", 4, pin_change_old_entered)) {
          debug(@"SIM change. Sent old pin request");
          return true;
        } else {
          debug(@"SIM change. Old pin notification failed");
          return false;
        }
#endif
    }


    void closeAllNotifications(bool resetData)
    {
        notifications.clear();
        dialogs.clear();

        if (resetData)
            clearStoredData();
    }

    void unlockCancelled()
    {
        std::cerr << "SIM notification cancelled" << std::endl;
        closeAllNotifications(true);
    }

    void pinUnlockEntered(std::string pin)
    {
#if 0
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
#endif
    }

    void pinResetNewEntered(std::string pin)
    {
#if 0
        close_all_notifications(false);

        if (send_unlock(_("Please confirm PIN code"), null, 4, pin_reset_confirm_entered)) {
          debug("SIM reset pin request. Sent confirm pin request");
          new_pin = pin;
        } else {
          warning("SIM reset pin request. Confirm pin notification failed");
          clear_stored_data();
        }
#endif
    }

    void pinResetConfirmEntered(std::string pin)
    {
#if 0
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
#endif
    }

    void pinChangeOldEntered(std::string pin)
    {
#if 0
        close_all_notifications(true);

        if (send_unlock(_("Please enter new SIM PIN"), "pin", 4, pin_change_new_entered)) {
          debug("SIM change pin request. Sent new pin request");
          old_pin = pin;
        } else {
          warning("SIM change pin request. New pin notification failed");
          clear_stored_data();
        }
#endif
    }

    void pinChangeNewEntered(std::string pin)
    {
#if 0
        close_all_notifications(false);

        if (send_unlock(_("Please confirm PIN code"), null, 4, pin_change_confirm_entered)) {
          debug("SIM change pin request. Sent confirm pin request");
          new_pin = pin;
        } else {
          warning("SIM change pin request. Confirm pin notification failed");
          clear_stored_data();
        }
#endif
    }

    void pinChangeConfirmEntered(std::string pin)
    {
#if 0
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
#endif
    }
};

ModemManager::ModemManager()
{
    std::cout << __PRETTY_FUNCTION__ << std::endl;
    d.reset(new Private);
}

ModemManager::~ModemManager()
{
    std::cout << __PRETTY_FUNCTION__ << std::endl;
}

const core::Property<std::set<Modem::Ptr>> &
ModemManager::modems()
{
    return d->modems;
}
