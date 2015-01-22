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

#include <menuharness/MenuItemMatcher.h>

#include <vector>

#include <QObject>

using namespace std;

namespace menuharness
{
namespace
{
enum MenuRoles
{
    LabelRole  = Qt::DisplayRole + 1,
    SensitiveRole,
    IsSeparatorRole,
    IconRole,
    TypeRole,
    ExtendedAttributesRole,
    ActionRole,
    ActionStateRole,
    IsCheckRole,
    IsRadioRole,
    IsToggledRole
};
}

struct MenuItemMatcher::Priv
{
    Type m_type = Type::plain;

    Mode m_mode = Mode::all;

    shared_ptr<string> m_label;

    shared_ptr<string> m_action;

    vector<MenuItemMatcher> m_items;
};

MenuItemMatcher MenuItemMatcher::checkbox()
{
    MenuItemMatcher matcher;
    matcher.type(Type::checkbox);
    return matcher;
}

MenuItemMatcher MenuItemMatcher::separator()
{
    MenuItemMatcher matcher;
    matcher.type(Type::separator);
    return matcher;
}

MenuItemMatcher::MenuItemMatcher() :
        p(new Priv)
{
}

MenuItemMatcher::~MenuItemMatcher()
{
}

MenuItemMatcher::MenuItemMatcher(const MenuItemMatcher& other) :
        p(new Priv)
{
    *this = other;
}

MenuItemMatcher::MenuItemMatcher(MenuItemMatcher&& other)
{
    *this = move(other);
}

MenuItemMatcher& MenuItemMatcher::operator=(const MenuItemMatcher& other)
{
    p->m_type = other.p->m_type;
    p->m_mode = other.p->m_mode;
    p->m_label = other.p->m_label;
    p->m_action = other.p->m_action;
    p->m_items = other.p->m_items;
    return *this;
}

MenuItemMatcher& MenuItemMatcher::operator=(MenuItemMatcher&& other)
{
    p = move(other.p);
    return *this;
}

MenuItemMatcher& MenuItemMatcher::type(Type type)
{
    p->m_type = type;
    return *this;
}

MenuItemMatcher& MenuItemMatcher::label(const string& label)
{
    p->m_label = make_shared<string>(label);
    return *this;
}

MenuItemMatcher& MenuItemMatcher::action(const string& action)
{
    p->m_action = make_shared<string>(action);
    return *this;
}

MenuItemMatcher& MenuItemMatcher::mode(Mode mode)
{
    p->m_mode = mode;
    return *this;
}

MenuItemMatcher& MenuItemMatcher::item(const MenuItemMatcher& item)
{
    p->m_items.emplace_back(item);
    return *this;
}

MenuItemMatcher& MenuItemMatcher::item(MenuItemMatcher&& item)
{
    p->m_items.emplace_back(item);
    return *this;
}

void MenuItemMatcher::match(MatchResult& matchResult, UnityMenuModel& menuModel, const QModelIndex& index) const
{
//            QString label = model->data(index, MenuRoles::LabelRole).toString();
//            QString icon = model->data(index, MenuRoles::IconRole).toString();
//            QString action = model->data(index, MenuRoles::ActionRole).toString();
//            QString type = model->data(index, MenuRoles::TypeRole).toString();
//            QString isSeparator = model->data(index, MenuRoles::IsSeparatorRole).toBool() ? "is separator" : "is not separator";
//            QString isCheckbox = model->data(index, MenuRoles::IsCheckRole).toBool() ? "is checkbox" : "is not checkbox";
//            QString isRadio = model->data(index, MenuRoles::IsRadioRole).toBool() ? "is radio" : "is not radio";
//            QString isToggled = model->data(index, MenuRoles::IsToggledRole).toBool() ? "is toggled" : "is not toggled";
//            {
//                for (int j = 0; j < indent * 2; ++j)
//                {
//                    m_buffer += " ";
//                }
//                m_buffer += " > " + label + ", " + icon + ", " + action + ", "
//                        + type + ", " + isSeparator + ", " + isCheckbox + ", "
//                        + isRadio + ", " + isToggled + "\n";
//            }
//            UnityMenuModel *submenu = qobject_cast<UnityMenuModel *>(model->submenu(i));
//            if (submenu)
//            {
//                match....
//            }
}

}
