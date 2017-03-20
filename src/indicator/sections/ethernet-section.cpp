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

#include <sections/ethernet-link-section.h>
#include <sections/ethernet-section.h>
#include <menuitems/settings-item.h>
#include <menuitems/switch-item.h>

#include "menumodel-cpp/action-group.h"
#include "menumodel-cpp/action-group-merger.h"
#include "menumodel-cpp/menu.h"
#include "menumodel-cpp/menu-merger.h"

#include <util/localisation.h>
#include <util/qhash-sharedptr.h>

using namespace std;
using namespace nmofono;
using namespace nmofono::ethernet;

class EthernetSection::Private : public QObject
{
    Q_OBJECT
public:
    Manager::Ptr m_manager;

    bool m_isSettingsMenu = false;

    Menu::Ptr m_settingsMenu;

    ActionGroupMerger::Ptr m_actionGroupMerger;

    MenuMerger::Ptr m_menuMerger;

    MenuMerger::Ptr m_links;

    QMap<EthernetLink::SPtr, EthernetLinkSection::SPtr> m_items;

    SettingsItem::Ptr m_openEthernetSettings;

    Private()
    {
    }

public Q_SLOTS:
    void linksChanged()
    {
        auto links = m_manager->ethernetLinks();
        auto current(m_items.keys().toSet());

        auto removed(current);
        removed.subtract(links);

        auto added(links);
        added.subtract(current);

        if (added.isEmpty() && removed.isEmpty())
        {
            return;
        }

        m_links->clear();

        for (auto connection : removed)
        {
            m_actionGroupMerger->remove(m_items[connection]->actionGroup());
            m_items.remove(connection);
        }

        for (auto link : added)
        {
            auto section = make_shared<EthernetLinkSection>(m_manager, link, m_isSettingsMenu);
            m_items[link] = section;
            m_actionGroupMerger->add(section->actionGroup());
        }

        multimap<Link::Id, EthernetLinkSection::SPtr> sorted;
        QMapIterator<EthernetLink::SPtr, EthernetLinkSection::SPtr> it(m_items);
        while (it.hasNext())
        {
            it.next();
            sorted.insert(make_pair(it.key()->id(), it.value()));
        }
        for (auto pair : sorted)
        {
            m_links->append(pair.second->menuModel());
        }

        if (m_isSettingsMenu)
        {
            return;
        }

        if (links.isEmpty())
        {
            m_settingsMenu->removeAll(m_openEthernetSettings->menuItem());
        }
        else
        {
            if (m_settingsMenu->find(m_openEthernetSettings->menuItem()) == m_settingsMenu->end())
            {
                m_settingsMenu->append(m_openEthernetSettings->menuItem());
            }
        }
    }
};

EthernetSection::EthernetSection(Manager::Ptr manager, bool isSettingsMenu)
    : d{new Private()}
{
    d->m_manager = manager;
    d->m_isSettingsMenu = isSettingsMenu;

    d->m_actionGroupMerger = make_shared<ActionGroupMerger>();
    d->m_menuMerger = make_shared<MenuMerger>();
    d->m_links = make_shared<MenuMerger>();

    d->m_menuMerger->append(d->m_links);

    // Don't include a link to the settings in the settings themselves
    if (!isSettingsMenu)
    {
        d->m_settingsMenu = make_shared<Menu>();
        d->m_menuMerger->append(d->m_settingsMenu);

        d->m_openEthernetSettings = make_shared<SettingsItem>(_("Ethernet settings…"), "ethernet");
        d->m_actionGroupMerger->add(d->m_openEthernetSettings->actionGroup());
    }

    QObject::connect(d->m_manager.get(), &Manager::linksUpdated, d.get(), &Private::linksChanged);
    d->linksChanged();
}

EthernetSection::~EthernetSection()
{
}

ActionGroup::Ptr
EthernetSection::actionGroup()
{
    return d->m_actionGroupMerger->actionGroup();
}

MenuModel::Ptr
EthernetSection::menuModel()
{
    return d->m_menuMerger;
}

#include "ethernet-section.moc"
