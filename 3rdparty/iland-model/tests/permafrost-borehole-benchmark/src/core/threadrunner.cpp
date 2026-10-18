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

/** @class ThreadRunner
  Encapsulates the invokation of multiple threads for paralellized tasks.
  To avoid lost updates during the light influence pattern application, all the resourceUnits
  are divided in two lists based on the index (even vs. uneven). These (for almost all cases)
  ensures, that no directly neighboring resourceUnits are processed.
  */

#include "global.h"
#include "threadrunner.h"
#include <QtCore>
#include <QtConcurrent/QtConcurrent>
bool ThreadRunner::mMultithreaded = true; // static

ThreadRunner::ThreadRunner()
{
    mMultithreaded = true;
}

void ThreadRunner::print()
{
    qDebug() << "Multithreading enabled: "<< mMultithreaded << "thread count:" << QThread::idealThreadCount();
}


void ThreadRunner::setup(const QList<ResourceUnit*> &resourceUnitList)
{
    mMap1.clear(); mMap2.clear();
    bool map=true;
    foreach(ResourceUnit *unit, resourceUnitList) {
        if (map)
            mMap1.append(unit);
        else
            mMap2.append(unit);

        map = !map;
    }

}

/// run a given function for each ressource unit either multithreaded or not.
void ThreadRunner::run(void (*funcptr)(ResourceUnit *), const bool forceSingleThreaded ) const
{
    if (mMultithreaded && mMap1.count() > 3 && forceSingleThreaded==false) {
        // execute using QtConcurrent for larger amounts of ressource units...
        QtConcurrent::blockingMap(mMap1,funcptr);
        QtConcurrent::blockingMap(mMap2,funcptr);
    } else {
        // execute serialized in main thread
        ResourceUnit *unit;
        foreach(unit, mMap1)
            (*funcptr)(unit);

        foreach(unit, mMap2)
            (*funcptr)(unit);
    }

}

/// run a given function for each species
void ThreadRunner::run(void (*funcptr)(Species *), const bool forceSingleThreaded ) const
{
    if (mMultithreaded && mSpeciesMap.count() > 3 && forceSingleThreaded==false) {
        QtConcurrent::blockingMap(mSpeciesMap, funcptr);
    } else {
        // single threaded operation
        Species *species;
        foreach(species, mSpeciesMap)
            (*funcptr)(species);
    }

}
