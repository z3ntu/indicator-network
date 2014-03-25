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

#ifndef GIO_HELPERS_UTIL_H
#define GIO_HELPERS_UTIL_H

#include <libintl.h>
#define _(String) gettext (String)

#include <gio/gio.h>
#include <memory>
#include <condition_variable>

struct GMainLoopSync
{
    std::condition_variable m_cv;
    std::mutex m_mutex;
    std::function<void()> m_func;

    static gboolean dispatch_cb(gpointer user_data)
    {
        GMainLoopSync *that = static_cast<GMainLoopSync *>(user_data);
        std::lock_guard<std::mutex> lk(that->m_mutex);
        that->m_func();
        that->m_cv.notify_all();
        return G_SOURCE_REMOVE;
    }

public:
    GMainLoopSync(std::function<void()> func)
        : m_func(func)
    {
        if (g_main_context_acquire(g_main_context_default())) {
            func();
            g_main_context_release(g_main_context_default());
        } else {
            std::unique_lock<std::mutex> lk(m_mutex);
            g_idle_add_full(G_PRIORITY_HIGH,
                            GSourceFunc(GMainLoopSync::dispatch_cb),
                            this,
                            NULL);
            if (m_cv.wait_for(lk, std::chrono::seconds(3)) == std::cv_status::timeout) {
                std::cerr << "Timeout when waiting for GMainLoop sync." << std::endl;
            }
            lk.unlock();
        }
    }
};

struct GMainLoopDispatch
{
    static gboolean dispatch_cb(gpointer user_data)
    {
        std::function<void()> *funcPtr = static_cast<std::function<void()>*>(user_data);
        (*funcPtr)();
        return G_SOURCE_REMOVE;
    }

public:
    GMainLoopDispatch(std::function<void()> func)
    {
        if (g_main_context_acquire(g_main_context_default())) {
            func();
            g_main_context_release(g_main_context_default());
        } else {
            std::function<void()> *funcPtr = new std::function<void()>(func);
            g_idle_add_full(G_PRIORITY_HIGH,
                            GSourceFunc(GMainLoopDispatch::dispatch_cb),
                            funcPtr,
                            NULL);
        }
    }
};

struct GObjectDeleter {
    GObjectDeleter() noexcept {}
   ~GObjectDeleter() noexcept {}

    void operator() (void *ptr)
    {
        if (!ptr)
            return;
        GObject *obj = G_OBJECT(ptr);
        if (obj) {
            g_clear_object(&obj);
        }
    }
};

struct GVariantDeleter {
    GVariantDeleter() noexcept {}
    ~GVariantDeleter() noexcept {}

    void operator() (GVariant *ptr)
    {
        if (!ptr)
            return;
        if (g_variant_is_floating(ptr))
            g_variant_ref_sink(ptr);
        g_variant_unref(ptr);
    }
};

namespace {

typedef std::shared_ptr<GVariant> GVariantPtr;
GVariantPtr make_gvariant_ptr(GVariant *ptr)
{
    if (ptr && g_variant_is_floating(ptr))
        g_variant_ref_sink(ptr);
    return GVariantPtr(ptr, GVariantDeleter());
}

typedef std::shared_ptr<GMenu> GMenuPtr;
GMenuPtr make_gmenu_ptr() { return std::shared_ptr<GMenu>(g_menu_new(), GObjectDeleter()); }

typedef std::shared_ptr<GMenuItem> GMenuItemPtr;
GMenuItemPtr make_gmenuitem_ptr(GMenuItem *gmenuitem) { return std::shared_ptr<GMenuItem>(gmenuitem, GObjectDeleter()); }

// returns the index of the appended item.
int append_item_to_gmenu(GMenu *menu, GMenuItem *item)
{
    int n_items = g_menu_model_get_n_items(G_MENU_MODEL(menu));
    g_menu_insert_item(menu, n_items, item);
    return n_items;
}

}

class SessionBus
{
    std::shared_ptr<GDBusConnection> m_bus;

public:
    SessionBus()
    {
        GError *error = NULL;
        m_bus.reset(g_bus_get_sync(G_BUS_TYPE_SESSION,
                                   NULL,
                                   &error),
                    GObjectDeleter());
        if (!m_bus) {
            g_assert(error != NULL);
            if (error->domain != G_IO_ERROR || error->code != G_IO_ERROR_CANCELLED) {
                std::cerr << "Error getting the bus: " << error->message;
            }
            g_error_free(error);
            /// @todo throw something
        }
    }

    std::shared_ptr<GDBusConnection> bus()
    {
        return m_bus;
    }
};

class BusName
{
    guint m_busOwnId;
    std::function<void(std::string)> m_acquired;
    std::function<void(std::string)> m_lost;
    std::shared_ptr<SessionBus> m_bus;

    static void nameAcquired(GDBusConnection *,
                             const gchar *name,
                             gpointer user_data)
    {
        BusName *that = static_cast<BusName *>(user_data);
        that->m_acquired(name);
    }

    static void nameLost(GDBusConnection *,
                         const gchar *name,
                         gpointer user_data)
    {
        BusName *that = static_cast<BusName *>(user_data);
        that->m_lost(name);
    }

public:
    BusName(std::string name,
            std::function<void(std::string)> acquired,
            std::function<void(std::string)> lost,
            std::shared_ptr<SessionBus> bus)
        : m_acquired {acquired},
          m_lost {lost},
          m_bus {bus}
    {
        m_busOwnId =
            g_bus_own_name_on_connection(m_bus->bus().get(),
                                         name.c_str(),
                                         G_BUS_NAME_OWNER_FLAGS_NONE,
                                         (GBusNameAcquiredCallback)nameAcquired,
                                         (GBusNameLostCallback)nameLost,
                                         this,
                                         NULL);

    }
    ~BusName()
    {
        if (m_busOwnId != 0)
            g_bus_unown_name(m_busOwnId);
    }
};


#endif
