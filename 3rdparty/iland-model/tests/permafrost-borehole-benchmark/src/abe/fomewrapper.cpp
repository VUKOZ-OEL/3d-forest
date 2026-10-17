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
#include "fomewrapper.h"

#include "activity.h"
#include "fmstand.h"
#include "fmunit.h"
#include "forestmanagementengine.h"
#include "fmstp.h"
namespace ABE {

// definition of variables
// (1) variables of activites

// (2) stand variables
QStringList standVarList=QStringList() << "basalArea" << "age"  << "absoluteAge" << "nspecies"
                                       << "volume" << "dbh" << "height"
                                       << "annualIncrement" << "elapsed" << "topHeight" << "area" << "year";

// (3) site variables
QStringList siteVarList=QStringList() << "annualIncrement" << "harvestMode" << "U";
int siteVarListOffset = standVarList.count(); // stand vars start here...


// finally: combine all varibles:
QStringList allVarList;

// setup the variable names
// we use "__" internally (instead of .)
void FOMEWrapper::buildVarList()
{
    allVarList.clear();

    foreach(QString var, standVarList)
        allVarList.append(QString("stand__%1").arg(var));

    foreach(QString var, siteVarList)
        allVarList.append(QString("site__%1").arg(var));
}


const QStringList FOMEWrapper::getVariablesList()
{
    if (allVarList.isEmpty())
        buildVarList();
    return allVarList;
}

double FOMEWrapper::value(const int variableIndex)
{
    // dispatch
    if (variableIndex > siteVarListOffset)
        return valueSite(variableIndex - siteVarListOffset);


    return valueStand(variableIndex);
}



double FOMEWrapper::valueStand(const int variableIndex)
{
    //<< "basalArea" << "age"  << "absoluteAge" << "speciesCount" << "volume" << dbh << height
    switch (variableIndex) {
    case 0: return mStand->basalArea(); // "basalArea"
    case 1: return mStand->age(); // mean age, "age"
    case 2: return mStand->absoluteAge(); // years since begin of rotation, "absoluteAge"
    case 3: return mStand->nspecies(); // species richness, "nspecies"
    case 4: return mStand->volume(); // total standing volume, m3/ha, "volume"
    case 5: return mStand->dbh(); // mean dbh
    case 6: return mStand->height(); // "height" (m)
    case 7: return mStand->meanAnnualIncrementTotal(); // annual increment (since beginning of the rotation) m3/ha
    case 8: return ForestManagementEngine::instance()->currentYear() - mStand->lastExecution(); // years since last execution of an activity for the stand (yrs)
    case 9: return mStand->topHeight(); // top height (m)
    case 10: return mStand->area(); // stand area (ha)
    case 11: return GlobalSettings::instance()->currentYear(); // the current year
    default: return 0;
    }
}

double FOMEWrapper::valueSite(const int variableIndex)
{
    switch (variableIndex) {
    case 0: return mStand->unit()->annualIncrement(); // annualIncrement
    case 2: return mStand->U(); // just testing
    default: return 0;
    }
}

} // namespace
