/********************************************************************************************
**    iLand - an individual based forest landscape and disturbance model
**    https://iland-model.org
**    Copyright (C) 2009-  Werner Rammer, Rupert Seidl
**
**    This program is free software: you can redistribute it and/or modify
**    it under the terms of the GNU General Public License as published by
**    the Free Software Foundation, either version 3 of the License, or
**    (at your option) any later version.
**
**    This program is distributed in the hope that it will be useful,
**    but WITHOUT ANY WARRANTY; without even the implied warranty of
**    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**    GNU General Public License for more details.
**
**    You should have received a copy of the GNU General Public License
**    along with this program.  If not, see <http://www.gnu.org/licenses/>.
********************************************************************************************/
#include "global.h"
#include "debugtimer.h"

#include <QDateTime>
#include <QMutexLocker>
#include <QHash>

// static members
QHash<QString, double> DebugTimer::mTimingList;
 bool DebugTimer::m_responsive_mode = false;
 qint64 DebugTimer::ms_since_epoch = 0;
/*
double DebugTimer::m_tick_p_s=0.;

void DebugTimer::sampleClock(int ms)
{
    QTime t;
    t.start();
    ticks now = getticks();

    while (t.elapsed() < ms) {
       int t;
       double x=0.;
       for (t=0; t<100; t++)
           x+=sin(x);
    }
    int el = t.elapsed();
    double tickselapsed = elapsed(getticks(), now);
    m_tick_p_s = tickselapsed / double(el);
    qDebug() << ms << "ms -> ticks/msec" << m_tick_p_s << "ticks elapsed" << tickselapsed;

}*/
DebugTimer::~DebugTimer()
{
    --m_count;
#ifndef FONSTUDIO
    if (responsiveMode()) {
        qint64 diff = QDateTime::currentMSecsSinceEpoch() - ms_since_epoch;
        if (diff > 100) {
            ms_since_epoch = QDateTime::currentMSecsSinceEpoch();
            // qDebug() << "DebugTimer:: process events after 100ms - now" << ms_since_epoch;
            // process events only if we are currently in the main thread (GUI)
            if (QThread::currentThread() == QCoreApplication::instance()->thread())
                QCoreApplication::processEvents();
        }
    }
#endif

    double t = elapsed();
    mTimingList[m_caption]+=t;
    // show message if timer is not set to silent, and if time > 100ms (if timer is set to hideShort (which is the default))
    if (!m_silent && (!m_hideShort || t>100.))
        showElapsed();
}

QMutex timer_mutex;
DebugTimer::DebugTimer(const QString &caption, bool silent)
{
    ++m_count;
    if (responsiveMode() && m_count==1) {
        // store time of the first call (start of the year)
        ms_since_epoch = QDateTime::currentMSecsSinceEpoch();
    }

    m_caption = caption;
    m_silent=silent;
    m_hideShort=true;
    if (!mTimingList.contains(caption)) {
        QMutexLocker locker(&timer_mutex);
        if (!mTimingList.contains(caption))
            mTimingList[caption]=0.;
    }
    start();

}

void DebugTimer::clearAllTimers()
{
    QHash<QString, double>::iterator i = mTimingList.begin();
     while (i != mTimingList.end()) {
         i.value() = 0.;
         ++i;
     }
}
void DebugTimer::printAllTimers()
{
    QHash<QString, double>::iterator i = mTimingList.begin();
    qWarning() << "Total timers\n================";
    double total=0.;
    while (i != mTimingList.end()) {
         if (i.value()>0)
             qWarning() << i.key() << ":" << timeStr(i.value());
         total+=i.value();
         ++i;
     }
    qWarning() << "Sum: " << total << "ms";
}

// pretty formatting of timing information
QString DebugTimer::timeStr(double value_ms, bool exact)
{
    if (exact) {
        if (value_ms<10000)
            return QString("%1ms").arg(value_ms);
        if (value_ms<60000)
            return QString("%1s").arg(value_ms/1000);
        if (value_ms<60000*60)
            return QString("%1m %2s").arg(floor(value_ms/60000)).arg(fmod(value_ms,60000)/1000);

        return QString("%1h %2m %3s").arg(floor(value_ms/3600000)) //h
                .arg(floor(fmod(value_ms,3600000)/60000)) //m
                .arg(qRound(fmod(value_ms,60000)/1000));    //s
    } else {
        if (value_ms<60000)
            return QString("%1s").arg(qRound(value_ms/1000.));
        if (value_ms<60000*60)
            return QString("%1:%2").arg(floor(value_ms/60000), 2, 'f', 0, QLatin1Char('0'))
                    .arg(floor(fmod(value_ms,60000)/1000.), 2, 'f', 0, QLatin1Char('0'));

        return QString("%1:%2:%3").arg(floor(value_ms/3600000)) //h
                .arg(floor(fmod(value_ms,3600000)/60000), 2, 'f', 0,  QLatin1Char('0')) //m
                .arg(floor(fmod(value_ms,60000)/1000.), 2, 'f', 0, QLatin1Char('0'));    //s

    }
}

void DebugTimer::interval(const QString &text)
{
    double elapsed_time = elapsed();
    qDebug() << "Timer" << text << timeStr(elapsed_time);
    start();
}

void DebugTimer::showElapsed()
{
    if (!m_shown) {
            qDebug() << "Timer" << m_caption << ":" << timeStr(elapsed());
    }
    m_shown=true;
}
double DebugTimer::elapsed()
{
    return t.elapsed()*1000;
}

void DebugTimer::start()
{
    t.start();
    m_shown=false;
}
