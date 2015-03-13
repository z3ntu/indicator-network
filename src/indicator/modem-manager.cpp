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

#include "modem-manager.h"
#include <menumodel-cpp/gio-helpers/util.h>
#include <notify-cpp/snapdecision/sim-unlock.h>
#include "sim-unlock-dialog.h"

#include <algorithm>
#undef QT_NO_KEYWORDS
#include <qofono-qt5/qofonomanager.h>
#include <qofono-qt5/qofonomodem.h>
#define QT_NO_KEYWORDS = 1

using namespace std;

class ModemManager::Private : public QObject, public std::enable_shared_from_this<Private>
{
    Q_OBJECT
public:

    QMap<QString, Modem::Ptr> m_modems;

    shared_ptr<QOfonoManager> m_ofono;

    SimUnlockDialog::Ptr m_unlockDialog;

    QList<Modem::Ptr> m_pendingUnlocks;

    Private()
    {
        m_unlockDialog = make_shared<SimUnlockDialog>();
//        m_unlockDialog->ready().connect([this](){
//            if (!m_pendingUnlocks.empty()) {
//                auto modem = m_pendingUnlocks.front();
//                m_pendingUnlocks.pop_front();
//                if (modem->requiredPin().get() != Modem::PinType::none)
//                    m_unlockDialog->unlock(modem);
//            }
//        });

        m_ofono = make_shared<QOfonoManager>();
        connect(m_ofono.get(), &QOfonoManager::modemsChanged, this, &Private::modems_changed);
        modems_changed(m_ofono->modems());
    }

    ~Private()
    {
    }

public Q_SLOTS:
    void modems_changed(const QStringList& value)
    {
        QSet<QString> modemPaths(value.toSet());
        QSet<QString> currentModemPaths(m_modems.keys().toSet());

        auto toRemove = currentModemPaths;
        toRemove.subtract(modemPaths);

        auto toAdd = modemPaths;
        toAdd.subtract(currentModemPaths);

        for (const auto& path : toRemove)
        {
            auto modem = m_modems.take(path);
            if (m_pendingUnlocks.contains(modem))
            {
                m_unlockDialog->cancel();
                m_pendingUnlocks.removeOne(modem);
            }
        }

        for (const auto& path : toAdd)
        {
            auto modemInterface = make_shared<QOfonoModem>();
            modemInterface->setModemPath(path);

            auto modem = make_shared<Modem>(modemInterface);
            m_modems[path] = modem;
        }

        m_unlockDialog->showSimIdentifiers().set(m_modems.size() > 1);
    }
};

ModemManager::ModemManager()
    : d{new Private}
{
}

ModemManager::~ModemManager()
{}

void
ModemManager::unlockModem(Modem::Ptr modem)
{
//    try {
//        auto modems = d->m_modems.get();
//
//        if (std::count(modems.begin(), modems.end(), modem) == 0
//                || d->m_unlockDialog->modem() == modem
//                || std::count(d->m_pendingUnlocks.begin(), d->m_pendingUnlocks.end(), modem) != 0)
//            return;
//
//        if (d->m_unlockDialog->state() == SimUnlockDialog::State::ready
//                && d->m_pendingUnlocks.size() == 0)
//            d->m_unlockDialog->unlock(modem);
//        else
//            d->m_pendingUnlocks.push_back(modem);
//    } catch(const std::exception &e) {
//        // Something unexpected has happened. As an example, unity8 might have
//        // crashed taking the notification server with it. There is no graceful
//        // and reliable way to recover so die and get restarted.
//        // See also https://bugs.launchpad.net/unity-notifications/+bug/1238990
//        std::cerr << __PRETTY_FUNCTION__ << " sim unlocking failed: " << e.what() << "\n";
//        std::quick_exit(0);
//    }
}

void
ModemManager::unlockAllModems()
{
//#ifdef INDICATOR_NETWORK_TRACE_MESSAGES
//    std::cout << __PRETTY_FUNCTION__ << std::endl;
//#endif
//    std::multimap<int, Modem::Ptr, Modem::Compare> sorted;
//    for (auto m : d->m_modems.get()) {
//        sorted.insert(std::make_pair(m->index(), m));
//    }
//    for (auto pair : sorted) {
//#ifdef INDICATOR_NETWORK_TRACE_MESSAGES
//        std::cout << "Unlocking " << pair.second->simIdentifier().get() << std::endl;
//#endif
//        unlockModem(pair.second);
//    }
}

void
ModemManager::unlockModemByName(const std::string &name)
{
//#ifdef INDICATOR_NETWORK_TRACE_MESSAGES
//    std::cout << __PRETTY_FUNCTION__ << std::endl;
//#endif
//    for (auto const &m : d->m_modems.get()) {
//        if (m->name() == name) {
//            unlockModem(m);
//            return;
//        }
//    }
}


const core::Property<std::set<Modem::Ptr>> &
ModemManager::modems()
{
//    return d->m_modems;
}

#include "modem-manager.moc"
