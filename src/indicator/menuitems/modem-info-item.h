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

#ifndef MODEM_INFO_ITEM_H
#define MODEM_INFO_ITEM_H

#include "item.h"
#include "menumodel-cpp/action.h"
#include "menumodel-cpp/menu-item.h"
#include "menumodel-cpp/gio-helpers/variant.h"

#include <core/signal.h>

#include <functional>
#include <vector>

class ModemInfoItem : public Item
{
    class Private;
    std::unique_ptr<Private> d;

public:
    typedef std::shared_ptr<ModemInfoItem> Ptr;

    ModemInfoItem();
    virtual ~ModemInfoItem();

    void setStatusIcon(const std::string &name);
    void setStatusText(const std::string &value);
    void setConnectivityIcon(const std::string &name);
    void setSimIdentifierText(const std::string &value);
    void setLocked(bool value);
    void setRoaming(bool value);

    virtual MenuItem::Ptr menuItem();

    core::Signal<void> &unlock();
};

#endif // MODEM_INFO_ITEM_H
