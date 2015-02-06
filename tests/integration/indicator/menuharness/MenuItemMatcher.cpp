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
#include <menuharness/MatchUtils.h>
#include <menuharness/MenuItemMatcher.h>

#include <iostream>
#include <vector>

using namespace std;

namespace menuharness
{
namespace
{
enum class LinkType
{
    any,
    section,
    submenu
};

static shared_ptr<GVariant> get_attribute(const shared_ptr<GMenuItem> menuItem, const gchar* attribute)
{
    shared_ptr<GVariant> value(
            g_menu_item_get_attribute_value(menuItem.get(), attribute, nullptr),
            &gvariant_deleter);
    return value;
}

static bool get_boolean_attribute(const shared_ptr<GMenuItem> menuItem, const gchar* attribute)
{
    gboolean result;
    g_menu_item_get_attribute(menuItem.get(), attribute, "b", &result);
    return result;
}

static string get_string_attribute(const shared_ptr<GMenuItem> menuItem, const gchar* attribute)
{
    string result;
    char* temp = nullptr;
    if (g_menu_item_get_attribute(menuItem.get(), attribute, "s", &temp))
    {
        result = temp;
        g_free(temp);
    }
    return result;
}

static pair<string, string> split_action(const string& action)
{
    auto index = action.find('.');

    if (index == string::npos)
    {
        return make_pair(string(), action);
    }

    return make_pair(action.substr(0, index), action.substr(index + 1, action.size()));
}

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
    }

    return string();
}
}

struct MenuItemMatcher::Priv
{
    void all(MatchResult& matchResult, const shared_ptr<GMenuModel>& menu,
             map<string, shared_ptr<GActionGroup>>& actions, int index)
    {
        int count = g_menu_model_get_n_items(menu.get());

        if (m_items.size() != (unsigned int) count)
        {
            matchResult.failure(
                    "Expected " + to_string(m_items.size())
                            + " children at index " + to_string(index)
                            + ", but found "
                            + to_string(count));
            return;
        }

        for (size_t i = 0; i < m_items.size(); ++i)
        {
            const auto& matcher = m_items.at(i);
            matcher.match(matchResult, menu, actions, i);
        }
    }

    void startsWith(MatchResult& matchResult, const shared_ptr<GMenuModel>& menu,
                 map<string, shared_ptr<GActionGroup>>& actions, int index)
    {
        int count = g_menu_model_get_n_items(menu.get());
        if (m_items.size() > (unsigned int) count)
        {
            matchResult.failure(
                    "Expected at least " + to_string(m_items.size())
                            + " children at index " + to_string(index)
                            + ", but found "
                            + to_string(count));
            return;
        }

        for (size_t i = 0; i < m_items.size(); ++i)
        {
            const auto& matcher = m_items.at(i);
            matcher.match(matchResult, menu, actions, i);
        }
    }

    Type m_type = Type::plain;

    Mode m_mode = Mode::all;

    LinkType m_linkType = LinkType::any;

    shared_ptr<string> m_label;

    shared_ptr<string> m_icon;

    shared_ptr<string> m_action;

    vector<pair<string, bool>> m_boolean_attributes;

    vector<pair<string, string>> m_string_attributes;

    vector<string> m_has_string_attributes;

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
    p->m_boolean_attributes = other.p->m_boolean_attributes;
    p->m_string_attributes = other.p->m_string_attributes;
    p->m_has_string_attributes = other.p->m_has_string_attributes;
    p->m_isToggled = other.p->m_isToggled;
    p->m_linkType = other.p->m_linkType;
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
    return string_attribute("x-canonical-type", widget);
}

MenuItemMatcher& MenuItemMatcher::boolean_attribute(const string& name, bool value)
{
    p->m_boolean_attributes.emplace_back(name, value);
    return *this;
}

MenuItemMatcher& MenuItemMatcher::string_attribute(const string& name, const string& value)
{
    p->m_string_attributes.emplace_back(name, value);
    return *this;
}

MenuItemMatcher& MenuItemMatcher::has_string_attribute(const string& name)
{
    p->m_has_string_attributes.emplace_back(name);
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

MenuItemMatcher& MenuItemMatcher::submenu()
{
    p->m_linkType = LinkType::submenu;
    return *this;
}

MenuItemMatcher& MenuItemMatcher::section()
{
    p->m_linkType = LinkType::section;
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

void MenuItemMatcher::match(
        MatchResult& matchResult, const shared_ptr<GMenuModel>& menu,
        map<string, shared_ptr<GActionGroup>>& actions,
        int index) const
{
    shared_ptr<GMenuItem> menuItem(g_menu_item_new_from_model(menu.get(), index), &g_object_deleter);

    string action = get_string_attribute(menuItem, G_MENU_ATTRIBUTE_ACTION);

    bool isCheckbox = false;
    bool isRadio = false;
    bool isToggled = false;

    pair<string, string> idPair;
    shared_ptr<GActionGroup> actionGroup;

    if (!action.empty())
    {
        idPair = split_action(action);
        actionGroup = actions[idPair.first];
        shared_ptr<GVariant> state(
                g_action_group_get_action_state(actionGroup.get(),
                                                idPair.second.c_str()),
                &gvariant_deleter);
        auto attributeTarget = get_attribute(menuItem, G_MENU_ATTRIBUTE_TARGET);

        if (attributeTarget && state)
        {
            isToggled = g_variant_equal(state.get(), attributeTarget.get());
            isRadio = true;
        }
        else if (state
                && g_variant_is_of_type(state.get(), G_VARIANT_TYPE_BOOLEAN))
        {
            isToggled = g_variant_get_boolean(state.get());
            isCheckbox = true;
        }
    }

    Type actualType = Type::plain;
    if (isCheckbox)
    {
        actualType = Type::checkbox;
    }
    else if (isRadio)
    {
        actualType = Type::radio;
    }

    cerr << "action = '" << action << "' of type '" << type_to_string(actualType) << "' at index " << index << endl;

    if (actualType != p->m_type)
    {
        matchResult.failure(
                "Expected " + type_to_string(p->m_type) + " at index "
                        + to_string(index) + ", found "
                        + type_to_string(actualType));
    }

    string label = get_string_attribute(menuItem, G_MENU_ATTRIBUTE_LABEL);
    if (p->m_label && (*p->m_label) != label)
    {
        matchResult.failure(
                "Expected label " + *p->m_label + " at index "
                        + to_string(index) + ", but found " + label);
    }

    string icon = get_string_attribute(menuItem, G_MENU_ATTRIBUTE_ICON);
    if (p->m_icon && (*p->m_icon) != icon)
    {
        matchResult.failure(
                "Expected icon " + *p->m_icon + " at index "
                        + to_string(index) + ", but found " + icon);
    }

    if (p->m_action && (*p->m_action) != action)
    {
        matchResult.failure(
                "Expected action " + *p->m_action + " at index "
                        + to_string(index) + ", but found " + action);
    }

    for (const auto& e: p->m_boolean_attributes)
    {
        bool value = get_boolean_attribute(menuItem, e.first.c_str());
        if (e.second != value)
        {
            matchResult.failure(
                    "Expected boolean attribute '" + e.first + "' == "
                            + (e.second ? "true" : "false") + " at index "
                            + to_string(index) + ", but found "
                            + (value ? "true" : "false"));
        }
    }

    for (const auto& e: p->m_string_attributes)
    {
        string value = get_string_attribute(menuItem, e.first.c_str());
        if (e.second != value)
        {
            matchResult.failure(
                    "Expected string attribute '" + e.first + "' == '" + e.second
                            + "' at index " + to_string(index) + ", but found '"
                                    + value + "'");
        }
    }

    for (const string& name: p->m_has_string_attributes)
    {
        auto value = get_attribute(menuItem, name.c_str());
        if (!value)
        {
            matchResult.failure(
                    "Expected string attribute '" + name + "' at index "
                            + to_string(index) + ", but not found");
        }
    }

    if (p->m_isToggled && (*p->m_isToggled) != isToggled)
    {
        matchResult.failure(
                "Expected toggled = " + string(*p->m_isToggled ? "true" : "false")
                        + " at index " + to_string(index) + ", but found "
                        + string(isToggled ? "true" : "false"));
    }

    if (!p->m_items.empty())
    {
        shared_ptr<GMenuModel> link;

        switch (p->m_linkType)
        {
            case LinkType::any:
            {
                link.reset(g_menu_model_get_item_link(menu.get(), (int) index, G_MENU_LINK_SUBMENU), &g_object_deleter);
                if (!link)
                {
                    link.reset(g_menu_model_get_item_link(menu.get(), (int) index, G_MENU_LINK_SECTION), &g_object_deleter);
                }
                break;
            }
            case LinkType::submenu:
            {
                link.reset(g_menu_model_get_item_link(menu.get(), (int) index, G_MENU_LINK_SUBMENU), &g_object_deleter);
                break;
            }
            case LinkType::section:
            {
                link.reset(g_menu_model_get_item_link(menu.get(), (int) index, G_MENU_LINK_SECTION), &g_object_deleter);
                break;
            }
        }

        if (!link)
        {
            matchResult.failure(
                    "Expected " + to_string(p->m_items.size())
                            + " children at index " + to_string(index)
                            + ", but found none");
            return;
        }
        else
        {
            unsigned int waitCounter = 0;
            while (true)
            {
                MatchResult childMatchResult;

                switch (p->m_mode)
                {
                    case Mode::all:
                        p->all(childMatchResult, link, actions, index);
                        break;
                    case Mode::starts_with:
                        p->startsWith(childMatchResult, link, actions, index);
                        break;
                }

                if (childMatchResult.success())
                {
                    matchResult.merge(childMatchResult);
                    break;
                }
                else
                {
                    ++waitCounter;
                    if (waitCounter >= 10)
                    {
                        matchResult.merge(childMatchResult);
                        break;
                    }
                    menuWaitForItems(link);
                }
            }
        }
    }

    if (p->m_activate)
    {
        if (action.empty())
        {
            matchResult.failure(
                    "Tried to activate action at index " + to_string(index)
                            + ", but no action was found");
        }
        else if(!actionGroup)
        {
            matchResult.failure(
                    "Tried to activate action group '" + idPair.first
                            + "' at index " + to_string(index)
                                    + ", but action group wasn't found");
        }
        else
        {
            // TODO Add parameterized activation
            g_action_group_activate_action(actionGroup.get(),
                                           idPair.second.c_str(), nullptr);
        }
    }
}

}
