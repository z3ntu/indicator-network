/*
 * Copyright © 2016 Canonical Ltd.
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
 * Authors:
 *     Antti Kaijanmäki <antti.kaijanmaki@canonical.com>
 */

#pragma once

#include <QDBusConnection>
#include <QDBusObjectPath>
#include <QObject>

#include <lomiri/util/DefinesPtrs.h>

namespace connectivityqt
{

class Q_DECL_EXPORT Sim : public QObject, public std::enable_shared_from_this<Sim>
{
    Q_OBJECT

public:
    LOMIRI_DEFINES_PTRS(Sim);

    Sim(const QDBusObjectPath& path, const QDBusConnection& connection, QObject* parent = 0);

    virtual ~Sim();

    Q_PROPERTY(QDBusObjectPath path READ path)
    QDBusObjectPath path() const;

    Q_PROPERTY(QString Iccid READ iccid CONSTANT)
    QString iccid() const;

    Q_PROPERTY(QString Imsi READ imsi NOTIFY imsiChanged)
    QString imsi() const;

    Q_PROPERTY(QString PrimaryPhoneNumber READ primaryPhoneNumber NOTIFY primaryPhoneNumberChanged)
    QString primaryPhoneNumber() const;

    Q_PROPERTY(bool Locked READ locked NOTIFY lockedChanged)
    bool locked() const;

    Q_PROPERTY(bool Present READ present NOTIFY presentChanged)
    bool present() const;

    Q_PROPERTY(QString Mcc READ mcc NOTIFY mccChanged)
    QString mcc() const;

    Q_PROPERTY(QString Mnc READ mnc NOTIFY mncChanged)
    QString mnc() const;

    Q_PROPERTY(QList<QString> PreferredLanguages READ preferredLanguages NOTIFY preferredLanguagesChanged)
    QList<QString> preferredLanguages() const;

    Q_PROPERTY(bool DataRoamingEnabled READ dataRoamingEnabled WRITE setDataRoamingEnabled NOTIFY dataRoamingEnabledChanged)
    bool dataRoamingEnabled() const;
    void setDataRoamingEnabled(bool value);

public Q_SLOTS:

    void unlock();

Q_SIGNALS:
    void lockedChanged(bool value);
    void presentChanged(bool value);
    void dataRoamingEnabledChanged(bool value);
    void imsiChanged(const QString &value);
    void primaryPhoneNumberChanged(const QString &value);
    void mccChanged(const QString &value);
    void mncChanged(const QString &value);
    void preferredLanguagesChanged();

protected:
    class Priv;
    std::shared_ptr<Priv> d;
};

}

Q_DECLARE_METATYPE(connectivityqt::Sim*)
