/*
 * Copyright (C) 2015 Canonical, Ltd.
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
 * Author: Pete Woods <pete.woods@canonical.com>
 */

#pragma once

#include <menuitems/switch-item.h>
#include <menuitems/section.h>
#include <nmofono/vpn/vpn-manager.h>

#include <lomiri/util/DefinesPtrs.h>

class VpnSection : public Section
{
    class Private;
    std::shared_ptr<Private> d;

public:
    LOMIRI_DEFINES_PTRS(VpnSection);

    explicit VpnSection(nmofono::vpn::VpnManager::SPtr vpnManager);
    virtual ~VpnSection() = default;

    virtual ActionGroup::Ptr actionGroup();
    virtual MenuModel::Ptr menuModel();
};
