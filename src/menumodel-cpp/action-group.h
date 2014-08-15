/*
 * Copyright © 2014 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License version 3,
 * as published by the Free Software Foundation.
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
 *     Antti Kaijanmäki <antti.kaijanmaki@canonical.com>
 */

#ifndef ACTION_GROUP_H
#define ACTION_GROUP_H

#include <memory>
#include <gio/gio.h>
#include <set>

#include "action.h"
#include "gio-helpers/util.h"

#include <core/signal.h>

class ActionGroup
{
    std::string m_prefix;
    std::set<Action::Ptr> m_actions;
    core::Signal<Action::Ptr> m_actionAdded;
    core::Signal<Action::Ptr> m_actionRemoved;

public:
    typedef std::shared_ptr<ActionGroup> Ptr;

    ActionGroup(const std::string &prefix = "")
        : m_prefix {prefix}
    {
    }

    std::set<Action::Ptr> actions()
    {
        return m_actions;
    }

    void add(Action::Ptr action)
    {
        auto iter = m_actions.find(action);
        if (iter != m_actions.end()) {
            /// @todo throw something.
            std::cerr << "Trying to add action which was already added before." << std::endl;
            return;
        }
        m_actions.insert(action);
        m_actionAdded(action);
    }

    void remove(Action::Ptr action)
    {
        auto iter = m_actions.find(action);
        if (iter == m_actions.end()) {
            /// @todo throw something.
            std::cerr << "Trying to remove action which was not added before." << std::endl;
            return;
        }
        m_actionRemoved(action);
        m_actions.erase(action);
    }

    bool contains(Action::Ptr action)
    {
        return m_actions.find(action) != m_actions.end();
    }

    core::Signal<Action::Ptr> &actionAdded()
    {
        return m_actionAdded;
    }

    core::Signal<Action::Ptr> &actionRemoved()
    {
        return m_actionRemoved;
    }
};

#endif // ACTION_GROUP_MERGER_H
