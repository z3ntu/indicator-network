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

#include "menu.h"
#include "action-group-exporter.h"

class MenuExporter
{
    Menu::Ptr m_menu;
    gint m_exportId;
    std::shared_ptr<SessionBus> m_sessionBus;
public:

    typedef std::shared_ptr<MenuExporter> Ptr;

    MenuExporter(const std::string &path, Menu::Ptr menu)
        : m_menu {menu},
          m_exportId {0}
    {
        assert(!path.empty());
        assert(menu);

        m_sessionBus.reset(new SessionBus());
        GError *error = NULL;
        m_exportId = g_dbus_connection_export_menu_model(m_sessionBus->bus().get(),
                                                         path.c_str(),
                                                         *menu,
                                                         &error);
        if (error) {
            if (error->domain != G_IO_ERROR || error->code != G_IO_ERROR_CANCELLED) {
                std::cerr << "Error exporting menu model:" << error->message;
            }
            g_error_free(error);
            /// @todo throw something
            return;
        }
    }

    ~MenuExporter()
    {
        if (!m_exportId)
            return;

        g_dbus_connection_unexport_menu_model(m_sessionBus->bus().get(), m_exportId);
    }
};
