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

#include <menuitems/settings-item.h>

#include <url-dispatcher-cpp/url-dispatcher.h>

using namespace std;

SettingsItem::SettingsItem(const QString &label, const QString &name)
{
    QString action_name = name + ".settings";
    string url = "settings:///system/" + name.toStdString();

    m_action = make_shared<Action>(action_name, nullptr);
    connect(m_action.get(), &Action::activated, this, [url]()
    {
        UrlDispatcher::send(url, [](string url, bool success)
        {
            if (!success)
            {
                cerr << "URL Dispatcher failed on " << url << endl;
            }
        });
    });

    m_actionGroup->add(m_action);

    m_item = make_shared<MenuItem>(label, "indicator." + action_name);
    m_item->setAttribute("x-canonical-type", TypedVariant<string>("com.canonical.indicators.link"));
}

MenuItem::Ptr
SettingsItem::menuItem()
{
    return m_item;
}

