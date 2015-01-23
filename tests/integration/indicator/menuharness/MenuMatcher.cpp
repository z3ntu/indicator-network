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

#include <menuharness/MenuMatcher.h>

#include <qmenumodel/unitymenumodel.h>

#include <QTestEventLoop>

using namespace std;

namespace menuharness
{
struct MenuMatcher::Parameters::Priv
{
    string m_busName;

    vector<pair<string, string>> m_actions;

    string m_menuObjectPath;
};

MenuMatcher::Parameters::Parameters(const string& busName,
                                    const vector<pair<string, string>>& actions,
                                    const string& menuObjectPath) :
        p(new Priv)
{
    p->m_busName = busName;
    p->m_actions = actions;
    p->m_menuObjectPath = menuObjectPath;
}

MenuMatcher::Parameters::~Parameters()
{
}

MenuMatcher::Parameters::Parameters(const Parameters& other) :
        p(new Priv)
{
    *this = other;
}

MenuMatcher::Parameters::Parameters(Parameters&& other)
{
    *this = move(other);
}

MenuMatcher::Parameters& MenuMatcher::Parameters::operator=(const Parameters& other)
{
    p->m_busName = other.p->m_busName;
    p->m_actions = other.p->m_actions;
    p->m_menuObjectPath = other.p->m_menuObjectPath;
    return *this;
}

MenuMatcher::Parameters& MenuMatcher::Parameters::operator=(Parameters&& other)
{
    p = move(other.p);
    return *this;
}

struct MenuMatcher::Priv
{
    Priv(const Parameters& parameters) :
        m_parameters(parameters)
    {
    }

    Parameters m_parameters;

    vector<MenuItemMatcher> m_items;

    UnityMenuModel m_menuModel;
};

MenuMatcher::MenuMatcher(const Parameters& parameters) :
        p(new Priv(parameters))
{
    p->m_menuModel.setBusName(QString::fromStdString(p->m_parameters.p->m_busName).toUtf8());
    QVariantMap actionsMap;
    for (const auto& action : p->m_parameters.p->m_actions)
    {
        actionsMap[QString::fromStdString(action.first)] =
                QString::fromStdString(action.second);
    }
    p->m_menuModel.setActions(actionsMap);
    p->m_menuModel.setMenuObjectPath(QString::fromStdString(p->m_parameters.p->m_menuObjectPath).toUtf8());
}

MenuMatcher::~MenuMatcher()
{
}

MenuMatcher& MenuMatcher::item(const MenuItemMatcher& item)
{
    p->m_items.emplace_back(item);
    return *this;
}

void MenuMatcher::match(MatchResult& matchResult) const
{
    // FIXME No magic sleeps
    QTestEventLoop::instance().enterLoopMSecs(200);

    if (p->m_items.size() > (unsigned int) p->m_menuModel.rowCount())
    {
        matchResult.failure("row count mismatch " + to_string(p->m_items.size()) + " vs " + to_string(p->m_menuModel.rowCount()));
        return;
    }

    for (size_t i = 0; i < p->m_items.size(); ++i)
    {
        const auto& matcher = p->m_items.at(i);
        auto index = p->m_menuModel.index(i);
        matcher.match(matchResult, p->m_menuModel, index);
    }
}

MatchResult MenuMatcher::match() const
{
    MatchResult matchResult;
    match(matchResult);
    return matchResult;
}

}

#include "MenuMatcher.moc"
