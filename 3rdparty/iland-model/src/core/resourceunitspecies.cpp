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
#include "resourceunitspecies.h"

#include "species.h"
#include "resourceunit.h"
#include "model.h"
#include "watercycle.h"
#include "debugtimer.h"

/** @class ResourceUnitSpecies
  @ingroup core
    The class contains data available at ResourceUnit x Species scale.
    Data stored is either statistical (i.e. number of trees per species) or used
    within the model (e.g. fraction of utilizable Radiation).
    Important submodules are:
    * 3PG production (Production3PG)
    * Establishment
    * Growth and Recruitment of Saplings
    * Snag dynamics
  */
ResourceUnitSpecies::~ResourceUnitSpecies()
{
}



void ResourceUnitSpecies::setup(Species *species, ResourceUnit *ru)
{
    mSpecies = species;
    mRU = ru;
    mResponse.setup(this);
    m3PG.setResponse(&mResponse);
    if (GlobalSettings::instance()->model()->settings().regenerationEnabled)
        mEstablishment.setup(ru->climate(), this);
    mStatistics.setResourceUnitSpecies(this);
    mStatisticsDead.setResourceUnitSpecies(this);
    mStatisticsMgmt.setResourceUnitSpecies(this);

    mRemovedGrowth = 0.;
    mLastYear = -1;

    DBGMODE( if(mSpecies->index()>1000 || mSpecies->index()<0)
             qDebug() << "suspicious species?? in RUS::setup()";
                );

}


void ResourceUnitSpecies::calculate(const bool fromEstablishment)
{

    // if *not* called from establishment, clear the species-level-stats
    bool has_leaf_area = statistics().leafAreaIndex() > 0.;
    if (!fromEstablishment)
        statistics().clear();

    // if already processed in this year, do not repeat
    if (mLastYear == GlobalSettings::instance()->currentYear())
        return;

    if (has_leaf_area || fromEstablishment==true) {
        // execute the water calculation...
        if (fromEstablishment)
            const_cast<WaterCycle*>(mRU->waterCycle())->run(); // run the water sub model (only if this has not be done already)
        DebugTimer rst("response+3pg");
        mResponse.calculate();// calculate environmental responses per species (vpd, temperature, ...)
        m3PG.calculate();// production of NPP
        mLastYear = GlobalSettings::instance()->currentYear(); // mark this year as processed
    } else {
        // if no LAI is present, then just clear the respones.
        mResponse.clear();
        m3PG.clear();
    }
}


void ResourceUnitSpecies::updateGWL()
{
    // removed growth is the running sum of all removed
    // tree volume. the current "GWL" therefore is current volume (standing) + mRemovedGrowth.
    // important: statisticsDead() and statisticsMgmt() need to calculate() before -> volume() is already scaled to ha
    mRemovedGrowth+=statisticsDead().volume() + statisticsMgmt().volume();
}

double ResourceUnitSpecies::leafAreaIndexSaplings() const {
    return mRU->stockableArea()>0.? constSaplingStat().leafArea() /mRU->stockableArea() : 0.;
}



