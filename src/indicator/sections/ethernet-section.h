/*
 * Copyright (C) 2016 Canonical, Ltd.
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

#include <nmofono/manager.h>

#include <menuitems/section.h>

class EthernetSection : public Section
{
public:
    LOMIRI_DEFINES_PTRS(EthernetSection);

    explicit EthernetSection(nmofono::Manager::Ptr manager, bool isSettingsMenu);

    ~EthernetSection();

    ActionGroup::Ptr
    actionGroup() override;

    MenuModel::Ptr
    menuModel() override;

protected:
    class Private;
    std::shared_ptr<Private> d;
};
