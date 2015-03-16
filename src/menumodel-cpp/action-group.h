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

#include <QObject>

class ActionGroup: public QObject
{
    Q_OBJECT

    std::string m_prefix;
    std::set<Action::Ptr> m_actions;

public:
    typedef std::shared_ptr<ActionGroup> Ptr;

    ActionGroup(const std::string &prefix = "");

    std::set<Action::Ptr> actions();

    void add(Action::Ptr action);

    void remove(Action::Ptr action);

    bool contains(Action::Ptr action);

Q_SIGNALS:
    void actionAdded(Action::Ptr);

    void actionRemoved(Action::Ptr);
};

#endif // ACTION_GROUP_MERGER_H
