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
 *     Pete Woods <pete.woods@canonical.com>
 */

#include <utils/action-utils.h>

using namespace std;

QString
testutils::string_value (MenuItem::Ptr menuItem, const QString &name)
{
    char *attribute = NULL;
    if (g_menu_item_get_attribute(menuItem->gmenuitem(), name.toUtf8().constData(), "s",
                                  &attribute))
    {
        QString result = QString::fromUtf8(attribute);
        g_free(attribute);
        return result;
    }
    throw std::logic_error("could not get string attribute");
}

bool
testutils::bool_value (MenuItem::Ptr menuItem, const QString &name)
{
    gboolean result;
    if (!g_menu_item_get_attribute(menuItem->gmenuitem(), name.toUtf8().constData(), "b",
                                   &result))
    {
        throw std::logic_error("could not get boolean attribute");
    }
    return (result == TRUE);
}

Action::Ptr
testutils::findAction (ActionGroup::Ptr actionGroup, const QString &name)
{
    auto pos = name.indexOf('.');
    QString shortName = name.mid(pos + 1);

    ::Action::Ptr action;
    std::set< ::Action::Ptr> actions = actionGroup->actions();
    for (auto it(actions.begin()); it != actions.end(); ++it)
    {
        if ((*it)->name() == shortName)
        {
            action = *it;
        }
    }
    return action;
}
