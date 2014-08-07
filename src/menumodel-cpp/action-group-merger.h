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

#ifndef ACTION_GROUP_MERGER_H
#define ACTION_GROUP_MERGER_H

#include <memory>
#include <gio/gio.h>
#include <core/connection.h>

#include "gio-helpers/util.h"
#include "action-group.h"
#include <set>
#include <map>

class ActionGroupMerger
{
    struct Connections
    {
        core::ScopedConnection actionAdded;
        core::ScopedConnection actionRemoved;

        Connections() = delete;
        Connections(core::ScopedConnection added, core::ScopedConnection removed)
            : actionAdded {std::move(added)},
              actionRemoved {std::move(removed)}
        {}
    };

    std::string m_prefix;
    ActionGroup::Ptr m_actionGroup;

    std::map<ActionGroup::Ptr, std::shared_ptr<Connections>> m_groups;

    std::map<Action::Ptr, int> m_count;
    std::map<std::string, Action::Ptr> m_names;

    void addAction(Action::Ptr action)
    {
        auto name_iter = m_names.find(action->name());
        if (name_iter != m_names.end()) {
            // we have two actions with the same name.
            // If they are from the same shared pointer, everything is OK and
            // count is incremented below, but if they have different pointer
            // then they will override each other in GActionGroup so let's catch that
            // early on.
            if (name_iter->second != action) {
                std::cerr << __PRETTY_FUNCTION__ << ": Conflicting action names. \"" << action->name() << "\"" << std::endl;
                /// @todo thow something.
                return;
            }
        } else {
            m_names[action->name()] = action;
        }

        auto count_iter = m_count.find(action);
        if (count_iter != m_count.end()) {
            count_iter->second += 1;
        } else {
            m_count[action] = 1;
            m_actionGroup->add(action);
        }
    }

    void removeAction(Action::Ptr action)
    {
        auto count_iter = m_count.find(action);
        // it should not be possible for this function to be called for an action that
        // was not added before
        assert(count_iter != m_count.end());
        count_iter->second -= 1;
        if (count_iter->second == 0) {
            m_actionGroup->remove(action);
            m_names.erase(action->name());
            m_count.erase(count_iter);
        }
    }


public:
    typedef std::shared_ptr<ActionGroupMerger> Ptr;

    ActionGroupMerger(const std::string &prefix = "")
        : m_prefix {prefix}
    {
        m_actionGroup = std::make_shared<ActionGroup>(m_prefix);
    }

    void add(ActionGroup::Ptr group)
    {
        auto iter = m_groups.find(group);
        if (iter != m_groups.end()) {
            /// @todo throw something.
            std::cerr << __PRETTY_FUNCTION__ << ": Trying to add action group which was already added before." << std::endl;
            return;
        }

        for (auto action : group->actions()) {
            addAction(action);
        }
        auto added = group->actionAdded().connect([this, group](Action::Ptr action){
            addAction(action);
        });
        auto removed = group->actionRemoved().connect([this, group](Action::Ptr action){
            removeAction(action);
        });
        m_groups[group] = std::make_shared<Connections>(added, removed);;
    }

    void remove(ActionGroup::Ptr group)
    {
        auto iter = m_groups.find(group);
        if (iter == m_groups.end()) {
            /// @todo throw something.
            std::cerr << __PRETTY_FUNCTION__ << ": Trying to remove action group which was not added before." << std::endl;
            return;
        }
        m_groups.erase(group);
        for (auto action : group->actions()) {
            removeAction(action);
        }
    }

    ActionGroup::Ptr actionGroup()
    {
        return m_actionGroup;
    }
    operator ActionGroup::Ptr() { return actionGroup(); }
};

#endif // ACTION_GROUP_MERGER_H
