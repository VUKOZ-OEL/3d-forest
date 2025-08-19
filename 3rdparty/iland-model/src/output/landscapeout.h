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

#ifndef LANDSCAPEOUT_H
#define LANDSCAPEOUT_H

#include "output.h"
#include "expression.h"
#include "standstatistics.h"
#include <QMap>

/** LandscapeOut is aggregated output for the total landscape per species. All values are per hectare values. */
class LandscapeOut : public Output
{
public:
    LandscapeOut();
    virtual void exec();
    virtual void setup();
private:
    Expression mCondition;
    QMap<QString,StandStatistics> mLandscapeStats;
};


/** LandscapeRemovedOut is aggregated output for removed trees on the full landscape. All values are per hectare values. */
class LandscapeRemovedOut : public Output
{
public:
    LandscapeRemovedOut();
    void execRemovedTree(const Tree *t, int reason);
    virtual void exec();
    virtual void setup();
private:
    const int mMaxDbh = 200;
    int dbhClass(float dbh); ///< return the dbh class of 'dbh' (starting with 0)
    void setupDbhClasses(QString cls_string);
    bool mIncludeDeadTrees;
    bool mIncludeHarvestTrees;
    Expression mCondition;
    struct LROdata {
        LROdata() { clear(); }
        void clear() { volume=0.; basal_area=0.; carbon=0.; n=0.; cstem=0.; cbranch=0.; cfoliage=0.; }
        double volume;
        double basal_area;
        double carbon;
        double n;
        double cstem;
        double cbranch;
        double cfoliage;
    };
    QVector<float> mDBHThreshold;
    QVector<int> mDBHClass;

    QHash<int,LROdata> mLandscapeRemoval;
};

#endif // LANDSCAPEOUT_H
