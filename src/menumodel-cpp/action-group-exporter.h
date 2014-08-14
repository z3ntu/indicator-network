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

#ifndef ACTION_GROUP_EXPORTER_H
#define ACTION_GROUP_EXPORTER_H

#include <memory>
#include <gio/gio.h>

#include "gio-helpers/util.h"

#include "action-group.h"

class ActionGroupExporter
{
    typedef std::shared_ptr<GSimpleActionGroup> GSimpleActionGroupPtr;
    GSimpleActionGroupPtr make_gsimpleactiongroup_ptr() { return std::shared_ptr<GSimpleActionGroup>(g_simple_action_group_new(), GObjectDeleter()); }

    std::shared_ptr<SessionBus> m_sessionBus;
    GSimpleActionGroupPtr m_gSimpleActionGroup;
    gint m_exportId;
    ActionGroup::Ptr m_actionGroup;
    std::string m_prefix;

public:
    typedef std::shared_ptr<ActionGroupExporter> Ptr;

    ActionGroupExporter() = delete;
    ActionGroupExporter(SessionBus::Ptr sessionBus, ActionGroup::Ptr actionGroup, const std::string &path, const std::string &prefix = "")
        : m_sessionBus(sessionBus),
          m_exportId {0},
          m_actionGroup {actionGroup},
          m_prefix {prefix}
    {
        m_gSimpleActionGroup = make_gsimpleactiongroup_ptr();

        GError *error = NULL;
        m_exportId = g_dbus_connection_export_action_group(m_sessionBus->bus().get(),
                                                           path.c_str(),
                                                           G_ACTION_GROUP(m_gSimpleActionGroup.get()),
                                                           &error);
        if (error) {
            if (error->domain != G_IO_ERROR || error->code != G_IO_ERROR_CANCELLED) {
                std::cerr << "Error exporting action group: " << error->message;
            }
            g_error_free(error);
            /// @todo throw something
            return;
        }

        GMainLoopDispatch([=](){
            for (auto action : actionGroup->actions()) {
                g_action_map_add_action(G_ACTION_MAP(m_gSimpleActionGroup.get()), action->gaction().get());
            }
        });
        actionGroup->actionAdded().connect([this](Action::Ptr action){
            GMainLoopDispatch([=](){
                g_action_map_add_action(G_ACTION_MAP(m_gSimpleActionGroup.get()), action->gaction().get());
            });
        });
        actionGroup->actionRemoved().connect([this](Action::Ptr action){
            GMainLoopDispatch([=](){
                g_action_map_remove_action(G_ACTION_MAP(m_gSimpleActionGroup.get()),
                                           action->name().c_str());
            });
        });
    }

    ~ActionGroupExporter()
    {
        if (!m_exportId)
            return;
        g_dbus_connection_unexport_action_group(m_sessionBus->bus().get(), m_exportId);
    }
};

#endif // ACTION_GROUP_EXPORTER_H
