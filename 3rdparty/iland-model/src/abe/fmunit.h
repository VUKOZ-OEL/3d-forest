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

#ifndef FMUNIT_H
#define FMUNIT_H
#include <QString>

namespace ABE {


class Agent; // forward
class Scheduler;
/** The FMUnit represents a management unit, i.e. a collection of stands.
 *  */
class FMUnit
{
public:
    FMUnit(const Agent *agent);
    ~FMUnit();
    void setId(const QString &id);
    const QString &id() const {return mId; }
    Scheduler *scheduler() {return mScheduler; }
    const Scheduler *constScheduler() const { return mScheduler; }
    const Agent* agent() const { return mAgent; }
    double area() const { return mTotalArea; } ///< total area of the unit (ha)
    int numberOfStands() const { return mNumberOfStands; } ///< the total number of stands
    void setNumberOfStands(int new_number) { mNumberOfStands = new_number; } ///< set the number of stands
    double volume() const { return mTotalVolume/area(); } ///< total volume of the unit (m3/ha)
    double annualIncrement() const { return mMAI; } ///< mean annual increment (m3/ha)
    void resetHarvestCounter();
    // agent properties
    /// rotation period (years)
    double U() const { return mU; }
    /// thinning intensity (class); 1: low, 2: medium, 3: high
    int thinningIntensity() const { return mThinningIntensityClass; }
    /// species composition key
    int targetSpeciesIndex() const { return mSpeciesCompositionIndex; }
    const QString &harvestMode() const { return mHarvestMode; }

    void setU(const double rotation_length) { mU = rotation_length; }
    void setThinningIntensity(const int th_class) { mThinningIntensityClass = th_class; }
    void setTargetSpeciesCompositionIndex(const int index) { mSpeciesCompositionIndex = index; }
    void setHarvestMode(const QString new_mode) { mHarvestMode = new_mode; }

    void setAverageMAI(const double avg_mai) { mAverageMAI = avg_mai; }
    double averageMAI() const { return mAverageMAI; }
    // actions

    /// update decadal management objectives for the planning unit.
    void managementPlanUpdate();

    /// tell the unit to update management plan next year
    void setForceUpdateManagementPlan() { mForceUpdateManagementPlan = true; }
    bool forceUpdateManagementPlan() const { return mForceUpdateManagementPlan; }

    /// run the agent main function
    void runAgent();

    /// update objectives of the current year.
    void updatePlanOfCurrentYear();

    /// record realized harvests on the unit (all harvests)
    void addRealizedHarvest(const double harvest_m3) { mRealizedHarvest+=harvest_m3; }

    void aggregate();
    QStringList info() const;

private:
    double annualTotalHarvest() const {return mRealizedHarvest-mRealizedHarvestLastYear; } ///< total m3 produced in final harvests in this year
    double annualThinningHarvest() const; ///< return the total m3 of thinning harvests (m3)
    QString mId;
    const Agent *mAgent;
    Scheduler *mScheduler;
    int mNumberOfStands; ///< the number of stands
    double mAnnualHarvestTarget; ///< planned annual harvest (final harvests) (m3)
    double mAnnualThinningTarget; ///< planned annual harvests (thinnings and tendings) (m3)
    double mRealizedHarvest; ///< sum of realized harvest in the current planning period (final harvests) (m3)
    double mRealizedHarvestLastYear; ///< the sum of harvests up to the last year (final harvests) (m3)
    double mAnnualHarvest; ///< suf of the harvest of the current year (final harvests)
    double mMAI; ///< mean annual increment (m3/ha)
    double mHDZ; ///< mean "haubarer" annual increment (m3/ha)
    double mMeanAge; ///< mean age of the planning unit
    double mTotalArea; ///< total area of the unit (ha)
    double mTotalVolume; ///< total standing volume (m3)
    double mTotalPlanDeviation; ///< cumulative deviation from the planned harvest (m3/ha)

    double mU; ///< rotation length
    int mSpeciesCompositionIndex; ///< index of the active target species composition
    int mThinningIntensityClass; ///< currently active thinning intensity level
    QString mHarvestMode; ///< type of applicable harvesting technique (e.g. skidder, cablecrane)

    double mAverageMAI; ///< reference value for mean annual increment
    bool mForceUpdateManagementPlan; ///<


    friend class UnitOut;
};

} // namespace
#endif // FOMEUNITS_H
