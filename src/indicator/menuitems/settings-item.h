/*
 * Copyright (C) 2017 Canonical, Ltd.
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
 *     Pete Woods <pete.woods@canonical.com>
 */

#pragma once

#include <menuitems/item.h>
#include <menumodel-cpp/menu-item.h>
#include <menumodel-cpp/action.h>

#include <unity/util/DefinesPtrs.h>

class SettingsItem : public Item
{
    Q_OBJECT

public:
    UNITY_DEFINES_PTRS(SettingsItem);

    SettingsItem() = delete;

    ~SettingsItem() = default;

    SettingsItem(const QString &label, const QString &name);

    virtual MenuItem::Ptr
    menuItem();

private:
    Action::Ptr m_action;
    MenuItem::Ptr m_item;
};
