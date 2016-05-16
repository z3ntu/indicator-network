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
 *     Antti Kaijanmäki <antti.kaijanmaki@canonical.com>
 */

#include <nmofono/connectivity-service-settings.h>

using namespace nmofono;

class ConnectivityServiceSettings::Private : public QObject
{
    Q_OBJECT
public:

    ConnectivityServiceSettings &p;
    QSettings *m_settings;

    Private(ConnectivityServiceSettings &parent)
        : p(parent)
    {
        m_settings = new QSettings(QSettings::IniFormat,
                                   QSettings::UserScope,
                                   "Ubuntu",
                                   "connectivityservice",
                                   this);
    }

    virtual ~Private()
    {

    }

public Q_SLOTS:

Q_SIGNALS:

};

ConnectivityServiceSettings::ConnectivityServiceSettings(QObject *parent)
    : QObject(parent),
      d{new Private(*this)}
{

}

ConnectivityServiceSettings::~ConnectivityServiceSettings()
{

}

QVariant ConnectivityServiceSettings::mobileDataEnabled()
{
    return d->m_settings->value("MobileDataEnabled");
}
void ConnectivityServiceSettings::setMobileDataEnabled(bool value)
{
    d->m_settings->setValue("MobileDataEnabled", value);
    d->m_settings->sync();
}

QVariant ConnectivityServiceSettings::simForMobileData()
{
    return d->m_settings->value("SimForMobileData");
}
void ConnectivityServiceSettings::setSimForMobileData(const QString &imsi)
{
    d->m_settings->setValue("SimForMobileData", imsi);
    d->m_settings->sync();
}

QStringList ConnectivityServiceSettings::knownSims()
{
    QVariant ret;
    ret = d->m_settings->value("KnownSims");
    if (ret.isNull())
    {
        /* This is the first time we are running on a system.
         */
        setKnownSims(QStringList());
        return QStringList();
    }

    return ret.toStringList();
}

void ConnectivityServiceSettings::setKnownSims(const QStringList &list)
{
    d->m_settings->setValue("KnownSims", QVariant(list));
    d->m_settings->sync();
}

wwan::Sim::Ptr ConnectivityServiceSettings::createSimFromSettings(const QString &imsi)
{
    d->m_settings->beginGroup(QString("Sims/%1/").arg(imsi));
    QVariant primaryPhoneNumber_var = d->m_settings->value("PrimaryPhoneNumber");
    QVariant mcc_var = d->m_settings->value("Mcc");
    QVariant mnc_var = d->m_settings->value("Mnc");
    QVariant preferredLanguages_var = d->m_settings->value("PreferredLanguages");
    QVariant dataRoamingEnabled_var = d->m_settings->value("DataRoamingEnabled");
    d->m_settings->endGroup();

    if (imsi.isNull() ||
            primaryPhoneNumber_var.isNull() ||
            mcc_var.isNull() ||
            mnc_var.isNull() ||
            preferredLanguages_var.isNull() ||
            dataRoamingEnabled_var.isNull())
    {
        qWarning() << "Corrupt settings for SIM: " << imsi;
        d->m_settings->remove(QString("Sims/%1/").arg(imsi));
        return wwan::Sim::Ptr();
    }

    return wwan::Sim::Ptr(new wwan::Sim(imsi,
                                        primaryPhoneNumber_var.toString(),
                                        mcc_var.toString(),
                                        mnc_var.toString(),
                                        preferredLanguages_var.toStringList(),
                                        dataRoamingEnabled_var.toBool()));
}

void ConnectivityServiceSettings::saveSimToSettings(wwan::Sim::Ptr sim)
{
    d->m_settings->beginGroup(QString("Sims/%1/").arg(sim->imsi()));
    d->m_settings->setValue("PrimaryPhoneNumber", QVariant(sim->primaryPhoneNumber()));
    d->m_settings->setValue("Mcc", sim->mcc());
    d->m_settings->setValue("Mnc", sim->mnc());
    d->m_settings->setValue("PreferredLanguages", QVariant(sim->preferredLanguages()));
    d->m_settings->setValue("DataRoamingEnabled", sim->dataRoamingEnabled());
    d->m_settings->endGroup();
    d->m_settings->sync();
}

#include "connectivity-service-settings.moc"
