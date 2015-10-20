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

#include <menuitems/vpn-item.h>
#include <util/localisation.h>

#include <menuitems/switch-item.h>

#include <QDebug>

using namespace std;
using namespace nmofono::vpn;

class VpnItem::Private
{
public:
    VpnConnection::SPtr m_vpnConnection;

    SwitchItem::Ptr m_item;
};

VpnItem::VpnItem(VpnConnection::SPtr vpnConnection)
    : d(new Private)
{
    d->m_vpnConnection = vpnConnection;

    static int id = 0;
    ++id;

    d->m_item = make_shared<SwitchItem>(d->m_vpnConnection->id(), "vpn", QString::number(id));
    d->m_item->menuItem()->setIcon("network-vpn");
    d->m_item->setState(d->m_vpnConnection->isActive());
    d->m_item->setEnabled(d->m_vpnConnection->isActivatable());

    m_actionGroupMerger->add(d->m_item->actionGroup());

    connect(d->m_vpnConnection.get(), &VpnConnection::idChanged, d->m_item->menuItem().get(), &MenuItem::setLabel);
    connect(d->m_vpnConnection.get(), &VpnConnection::activatableChanged, d->m_item.get(), &SwitchItem::setEnabled);
    connect(d->m_vpnConnection.get(), &VpnConnection::activeChanged, d->m_item.get(), &SwitchItem::setState);
    connect(d->m_item.get(), &SwitchItem::stateUpdated, d->m_vpnConnection.get(), &VpnConnection::setActive);
}

MenuItem::Ptr
VpnItem::menuItem()
{
    return d->m_item->menuItem();
}
