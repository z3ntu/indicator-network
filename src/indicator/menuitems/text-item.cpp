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

#include <menuitems/text-item.h>

using namespace std;

TextItem::TextItem(const QString &label, const QString &prefix, const QString &name)
{
    QString action_name = prefix + "." + name;

    m_action = make_shared<Action>(action_name, nullptr);
    m_actionGroup->add(m_action);
    connect(m_action.get(), &Action::activated, this, &TextItem::activated);
    m_item = make_shared<MenuItem>(label, "indicator." + action_name);
}

void
TextItem::setLabel(const QString &label)
{
    m_item->setLabel(label);
}

MenuItem::Ptr
TextItem::menuItem() {
    return m_item;
}

