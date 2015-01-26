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
 * Authored by: Pete Woods <pete.woods@canonical.com>
 */

#pragma once

#include <memory>
#include <string>

class QModelIndex;
class UnityMenuModel;

namespace menuharness
{
class MatchResult;

class MenuItemMatcher
{
public:
    enum class Mode
    {
        all,
        starts_with
    };

    enum class Type
    {
        plain,
        checkbox,
        radio,
        separator
    };

    static MenuItemMatcher checkbox();

    static MenuItemMatcher separator();

    static MenuItemMatcher radio();

    MenuItemMatcher();

    ~MenuItemMatcher();

    MenuItemMatcher(const MenuItemMatcher& other);

    MenuItemMatcher(MenuItemMatcher&& other);

    MenuItemMatcher& operator=(const MenuItemMatcher& other);

    MenuItemMatcher& operator=(MenuItemMatcher&& other);

    MenuItemMatcher& type(Type type);

    MenuItemMatcher& label(const std::string& label);

    MenuItemMatcher& action(const std::string& action);

    MenuItemMatcher& icon(const std::string& icon);

    MenuItemMatcher& widget(const std::string& widget);

    MenuItemMatcher& toggled(bool toggled);

    MenuItemMatcher& mode(Mode mode);

    MenuItemMatcher& item(const MenuItemMatcher& item);

    MenuItemMatcher& item(MenuItemMatcher&& item);

    MenuItemMatcher& activate();

    void match(MatchResult& matchResult, UnityMenuModel& menuModel, const QModelIndex& index) const;

protected:
    struct Priv;

    std::shared_ptr<Priv> p;
};

}
