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
#include "unitout.h"
#include "globalsettings.h"

#include "forestmanagementengine.h"
#include "fmunit.h"
#include "scheduler.h"

namespace ABE {

UnitOut::UnitOut()
{
    setName("Annual harvests and harvest plan on unit level.", "abeUnit");
    setDescription("The output provides planned and realized harvests on the level of planning units. " \
                   "Note that the planning unit area, mean age, mean volume and MAI are only updated every 10 years. "\
                   "Harvested timber is given as 'realizedHarvest', which is the sum of 'finalHarvest' and 'thinningHarvest.' "\
                   "The 'salvageHarvest' is provided extra, but already accounted for in the 'finalHarvest' column (note that salvageHarvest is not available when the scheduler is disabled).");
    columns() << OutputColumn::year()
              << OutputColumn("id", "unique identifier of the planning unit", OutString)
              << OutputColumn("area", "total area of the unit (ha)", OutDouble)
              << OutputColumn("age", "mean stand age (area weighted) (updated every 10yrs)", OutDouble)
              << OutputColumn("U", "default rotation length for stands of the unit (years)", OutInteger)
              << OutputColumn("thinningIntensity", "default thinning intensity for the unit", OutDouble)
              << OutputColumn("volume", "mean standing volume (updated every 10yrs), m3/ha", OutDouble)
              << OutputColumn("MAI", "mean annual increment (updated every 10yrs), m3/ha*yr", OutDouble)
              << OutputColumn("decadePlan", "planned mean harvest per year for the decade (m3/ha*yr)", OutDouble)
              << OutputColumn("annualPlan", "updated annual plan for the year, m3/ha*yr", OutDouble)
              << OutputColumn("runningDelta", "current aggregated difference between planned (10yr period) and realised harvests (+decay); positive: more realized than planned harvests, m3/ha*yr", OutDouble)
              << OutputColumn("realizedHarvest", "total harvested timber volume, m3/ha*yr", OutDouble)
              << OutputColumn("finalHarvest", "total harvested timber of planned final harvests, m3/ha*yr", OutDouble)
              << OutputColumn("thinningHarvest", "total harvested timber due to tending and thinning operations, m3/ha*yr", OutDouble)
              << OutputColumn("salvageHarvest", "total harvested timber due to salvage operations, m3/ha*yr", OutDouble);


}


void UnitOut::exec()
{
    FMUnit *unit;
    double salvage_harvest, annual_target;
    foreach(unit, ForestManagementEngine::instance()->mUnits) {
        *this << currentYear() << unit->id(); // keys
        *this << unit->area();
        *this << unit->mMeanAge;
        *this << unit->U() << unit->thinningIntensity();
        *this << unit->mTotalVolume/unit->area() << unit->mMAI;
        *this << unit->mAnnualHarvestTarget;

        salvage_harvest = unit->scheduler()->mExtraHarvest / unit->area();

        if (unit->scheduler()->enabled()) {
            annual_target = unit->scheduler()->mFinalCutTarget;
        } else {
            annual_target = 0.;

        }
        double thin_h = unit->annualThinningHarvest()/unit->area();
        *this << annual_target << unit->mTotalPlanDeviation
                               << unit->annualTotalHarvest()/unit->area() // total realized
                               << unit->annualTotalHarvest()/unit->area() - thin_h - salvage_harvest  // final
                               << thin_h // thinning
                               << salvage_harvest; // salvaging

        writeRow();
     }
}

void ABE::UnitOut::setup()
{

}


} // namespace
