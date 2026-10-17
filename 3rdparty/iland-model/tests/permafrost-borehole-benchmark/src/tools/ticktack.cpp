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

#include "ticktack.h"
#include <QtCore>

/* TODO: this is purely WINDOWS - provide a version for other BS, change the build-system
   to only use this on Win.
*/
#ifdef Q_OS_WIN
#include <windows.h>

class TTickTack
{
  private:
    _LARGE_INTEGER starttime;

_LARGE_INTEGER getCount()
{
  _LARGE_INTEGER value;
  QueryPerformanceCounter(&value);
  return  value;
}
  public:
    TTickTack() { reset(); }
    void reset() { starttime = getCount(); }
    double elapsed() {
        _LARGE_INTEGER stoptime = getCount();
        __int64 start = starttime.QuadPart;
        __int64 end = stoptime.QuadPart;
        __int64 elapsed = end - start;
        _LARGE_INTEGER freq;
        QueryPerformanceFrequency(&freq);
        double seconds = elapsed / double(freq.QuadPart);
        return seconds;

    }
};
#else
// non windows implementation
#include <QElapsedTimer>
class TTickTack
{
public:
    TTickTack() { reset(); }
    void reset() { t.start(); }
    double elapsed() { return t.elapsed()/1000.; }
private:
    QElapsedTimer t;
};
#endif

TickTack::TickTack()
{
    d = new TTickTack();
    d->reset();
}
TickTack::~TickTack()
{
    delete d;
}

void TickTack::start()
{
    d->reset();
}

double TickTack::elapsed()
{
    return d->elapsed();
}
