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
#ifndef DEBUGTIMER_H
#define DEBUGTIMER_H
#include "ticktack.h"
#include <QAtomicInt>

/** Timer class that writes timings to the Debug-Output-Channel

The class writes the elapsed time to qDebug() when either destructed, or when explicitely showElapsed() is called.
  elapsed() queries the elapsed time in milliseconds since construction or start() is called. Using interval() one can
  write a message with the time elapsed up the calling time, and the clock is reset afterwards. The name of the timer is
  set during construction. This message is printed when showElapsed() is called or durig destruction.
  Additionally, elapsed times of timers sharing the same caption are aggregated. Use clearAllTimers() to reset and printAllTimers()
  print the sums to the debug console. "Silent" DebugOutputs (setSilent() don't print timings for each iteration, but are still
    counted in the sums. If setAsWarning() is issued, the debug messages are print as warning, thus also visible
  when debug messages are disabled.
  @code void foo() {
     DebugTimer t("foo took [ms]:");
     <some lengthy operation>
 } // will e.g. print "foo took [ms]: 123" to debug console

 void bar() {
    DebugTimer::clearAllTimers(); // set all timers to 0
    for (i=0;i<1000;i++)
       foo();
    DebugTimer::printAllTimers(); // print the sum of the timings.
 }
 @endcode
 For Windows, the "TickTack"-backend is used.

*/
class DebugTimer
{
public:
    DebugTimer() { m_hideShort=false; m_silent=false; start();  ++m_count; }
    DebugTimer(const QString &caption, bool silent=false);
    void setSilent() { m_silent=true; }
    void setHideShort(bool hide_short_messages) { m_hideShort = hide_short_messages; }
    ~DebugTimer();
    void showElapsed();
    double elapsed(); // elapsed time in milliseconds
    void start();
    void interval(const QString &text);
    static void clearAllTimers();
    static void printAllTimers();
    static QString timeStr(double value_ms, bool exact=true);
    static void setResponsiveMode(bool mode) { m_responsive_mode = mode; }
    static bool responsiveMode()  { return m_responsive_mode; }
private:
    static QHash<QString, double> mTimingList;
    static bool m_responsive_mode;
    static qint64 ms_since_epoch; // milliseconds since epoch for the first call
    TickTack t;
    bool m_hideShort; // if true, hide messages for short operations (except an explicit call to showElapsed())
    bool m_shown;
    bool m_silent;
    QString m_caption;
    QAtomicInt m_count; // counts how many DebugTimer are currently alive
};

#endif // DEBUGTIMER_H
