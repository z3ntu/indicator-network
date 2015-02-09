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

#include <sstream>

using namespace std;

namespace menuharness
{

struct MatchResult::Priv
{
    bool m_hardFailure = false;

    bool m_success = true;

    vector<string> m_failures;
};

MatchResult::MatchResult() :
        p(new Priv)
{
}

MatchResult::MatchResult(MatchResult&& other)
{
    *this = move(other);
}

MatchResult::MatchResult(const MatchResult& other) :
        p(new Priv)
{
    *this = other;
}

MatchResult& MatchResult::operator=(const MatchResult& other)
{
    p->m_hardFailure = other.p->m_hardFailure;
    p->m_success = other.p->m_success;
    p->m_failures= other.p->m_failures;
    return *this;
}

MatchResult& MatchResult::operator=(MatchResult&& other)
{
    p = move(other.p);
    return *this;
}

void MatchResult::hardFailure()
{
    p->m_hardFailure = true;
    p->m_success = false;
}

void MatchResult::failure(const string& message)
{
    p->m_success = false;
    p->m_failures.emplace_back(message);
}

void MatchResult::merge(const MatchResult& other)
{
    p->m_hardFailure |= other.p->m_hardFailure;
    p->m_success &= other.p->m_success;
    p->m_failures.insert(p->m_failures.end(), other.p->m_failures.begin(),
                         other.p->m_failures.end());
}

bool MatchResult::hardFailed() const
{
    return p->m_hardFailure;
}

bool MatchResult::success() const
{
    return p->m_success;
}

vector<string>& MatchResult::failures() const
{
    return p->m_failures;
}

string MatchResult::concat_failures() const
{
    stringstream ss;
    ss << "Failed expectations:" << endl;
    for (const auto& failure : p->m_failures)
    {
        ss << failure << endl;
    }
    return ss.str();
}

}
