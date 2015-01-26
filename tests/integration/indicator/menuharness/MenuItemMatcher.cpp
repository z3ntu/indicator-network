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

#include <menuharness/MatchResult.h>
#include <menuharness/MenuItemMatcher.h>

#include <qmenumodel/unitymenumodel.h>

#include <vector>

#include <QObject>
#include <QTestEventLoop>

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

static string type_to_string(MenuItemMatcher::Type type)
{
    switch(type)
    {
        case MenuItemMatcher::Type::plain:
            return "plain";
        case MenuItemMatcher::Type::checkbox:
            return "checkbox";
        case MenuItemMatcher::Type::radio:
            return "radio";
        case MenuItemMatcher::Type::separator:
            return "separator";
    }

    return string();
}
}

struct MenuItemMatcher::Priv
{
    void all(MatchResult& matchResult, UnityMenuModel& submenuModel,
        const QModelIndex& index)
    {
        if (m_items.size() != (unsigned int) submenuModel.rowCount())
        {
            matchResult.failure(
                    "Expected " + to_string(m_items.size())
                            + " children at index " + to_string(index.row())
                            + ", but found "
                            + to_string(submenuModel.rowCount()));
            return;
        }

        for (size_t i = 0; i < m_items.size(); ++i)
        {
            const auto& matcher = m_items.at(i);
            auto index = submenuModel.index(i);
            matcher.match(matchResult, submenuModel, index);
        }
    }

    void startsWith(MatchResult& matchResult, UnityMenuModel& submenuModel,
                const QModelIndex& index)
    {
        if (m_items.size() > (unsigned int) submenuModel.rowCount())
        {
            matchResult.failure(
                    "Expected at least " + to_string(m_items.size())
                            + " children at index " + to_string(index.row())
                            + ", but found "
                            + to_string(submenuModel.rowCount()));
            return;
        }

        for (size_t i = 0; i < m_items.size(); ++i)
        {
            const auto& matcher = m_items.at(i);
            auto index = submenuModel.index(i);
            matcher.match(matchResult, submenuModel, index);
        }
    }

    Type m_type = Type::plain;

    Mode m_mode = Mode::all;

    shared_ptr<string> m_label;

    shared_ptr<string> m_icon;

    shared_ptr<string> m_action;

    shared_ptr<string> m_widget;

    shared_ptr<bool> m_isToggled;

    vector<MenuItemMatcher> m_items;

    bool m_activate = false;
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

MenuItemMatcher MenuItemMatcher::radio()
{
    MenuItemMatcher matcher;
    matcher.type(Type::radio);
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
    p->m_icon = other.p->m_icon;
    p->m_action = other.p->m_action;
    p->m_widget = other.p->m_widget;
    p->m_isToggled = other.p->m_isToggled;
    p->m_items = other.p->m_items;
    p->m_activate = other.p->m_activate;
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

MenuItemMatcher& MenuItemMatcher::icon(const string& icon)
{
    p->m_icon = make_shared<string>(icon);
    return *this;
}

MenuItemMatcher& MenuItemMatcher::widget(const string& widget)
{
    p->m_widget = make_shared<string>(widget);
    return *this;
}

MenuItemMatcher& MenuItemMatcher::toggled(bool isToggled)
{
    p->m_isToggled = make_shared<bool>(isToggled);
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

MenuItemMatcher& MenuItemMatcher::activate()
{
    p->m_activate = true;
    return *this;
}

void MenuItemMatcher::match(MatchResult& matchResult, UnityMenuModel& menuModel, const QModelIndex& index) const
{
    bool isSeparator = menuModel.data(index, MenuRoles::IsSeparatorRole).toBool();
    bool isCheckbox = menuModel.data(index, MenuRoles::IsCheckRole).toBool();
    bool isRadio = menuModel.data(index, MenuRoles::IsRadioRole).toBool();

    Type actualType = Type::plain;
    if (isSeparator)
    {
        actualType = Type::separator;
    }
    else if (isCheckbox)
    {
        actualType = Type::checkbox;
    }
    else if (isRadio)
    {
        actualType = Type::radio;
    }

    if (actualType != p->m_type)
    {
        matchResult.failure(
                "Expected " + type_to_string(p->m_type) + " at index "
                        + to_string(index.row()) + ", found "
                        + type_to_string(actualType));
    }

    string label = menuModel.data(index, MenuRoles::LabelRole).toString().toStdString();
    if (p->m_label && (*p->m_label) != label)
    {
        matchResult.failure(
                "Expected label " + *p->m_label + " at index "
                        + to_string(index.row()) + ", but found " + label);
    }

    string icon = menuModel.data(index, MenuRoles::IconRole).toString().toStdString();
    if (p->m_icon && (*p->m_icon) != icon)
    {
        matchResult.failure(
                "Expected icon " + *p->m_icon + " at index "
                        + to_string(index.row()) + ", but found " + icon);
    }

    string action = menuModel.data(index, MenuRoles::ActionRole).toString().toStdString();
    if (p->m_action && (*p->m_action) != action)
    {
        matchResult.failure(
                "Expected action " + *p->m_action + " at index "
                        + to_string(index.row()) + ", but found " + action);
    }

    string widget = menuModel.data(index, MenuRoles::TypeRole).toString().toStdString();
    if (p->m_widget && (*p->m_widget) != widget)
    {
        matchResult.failure(
                "Expected widget " + *p->m_widget + " at index "
                        + to_string(index.row()) + ", but found " + widget);
    }

    bool isToggled = menuModel.data(index, MenuRoles::IsToggledRole).toBool();
    if (p->m_isToggled && (*p->m_isToggled) != isToggled)
    {
        matchResult.failure(
                "Expected toggled = " + string(*p->m_isToggled ? "true" : "false")
                        + " at index " + to_string(index.row()) + ", but found "
                        + string(isToggled ? "true" : "false"));
    }

    if (!p->m_items.empty())
    {
        UnityMenuModel *submenuModel = qobject_cast<UnityMenuModel *>(
                menuModel.submenu(index.row()));

        // FIXME No magic sleeps
        QTestEventLoop::instance().enterLoopMSecs(100);

        if (!submenuModel)
        {
            matchResult.failure(
                    "Expected " + to_string(p->m_items.size())
                            + " children at index " + to_string(index.row())
                            + ", but found none");
            return;
        }

        switch (p->m_mode)
        {
            case Mode::all:
                p->all(matchResult, *submenuModel, index);
                break;
            case Mode::starts_with:
                p->startsWith(matchResult, *submenuModel, index);
                break;
        }
    }

    if (p->m_activate)
    {
        menuModel.activate(index.row());
    }
}

}
