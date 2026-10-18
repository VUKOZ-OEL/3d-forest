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
#include "abe_global.h"
#include "global.h"

#include "fmunit.h"

#include "forestmanagementengine.h"
#include "fmstand.h"
#include "scheduler.h"
#include "agent.h"
#include "agenttype.h"
#include "fomescript.h"
#include "fmtreelist.h"

namespace ABE {

/** @class FMUnit
    @ingroup abe
    The FMUnit class encapsulates a forest management unit, comprised of forest stands. Units are the base level at which
    the scheduling works.

  */


void FMUnit::aggregate()
{
    // loop over all stands
    // collect some data....
    double age=0.;
    double volume = 0.;
    double harvest = 0.;
    double totalarea = 0.;
    const QMultiMap<FMUnit*, FMStand*> &stands = ForestManagementEngine::instance()->stands();
    QMultiMap<FMUnit*, FMStand*>::const_iterator it = stands.constFind(this);
    while (it != stands.constEnd() && it.key()==this) {
        const FMStand *s = it.value();
        age += s->age() * s->area();
        volume += s->volume() * s->area();

        totalarea += s->area();
        ++it;
    }
    if (totalarea>0.) {
        age /= totalarea;
        volume /= totalarea;
        harvest /= totalarea;
    }
    qCDebug(abe) << "unit" << id() << "volume (m3/ha)" << volume << "age" << age << "planned harvest: todo";

}

QStringList FMUnit::info() const
{
    return QStringList() << QString("(accumulated) harvest: %1").arg(mRealizedHarvest)
                         << QString("MAI: %1").arg(mMAI)
                         << QString("HDZ: %1").arg(mHDZ)
                         << QString("average age: %1").arg(mMeanAge)
                         << QString("decadal plan: %1").arg(mAnnualHarvestTarget)
                         << QString("current plan: %1").arg(constScheduler()!=nullptr?constScheduler()->harvestTarget():0.);

}

double FMUnit::annualThinningHarvest() const
{
    const QMultiMap<FMUnit*, FMStand*> &stands = ForestManagementEngine::instance()->stands();
    QMultiMap<FMUnit*, FMStand*>::const_iterator it = stands.constFind(const_cast<FMUnit*>(this));
    double harvested=0.;
    while (it != stands.constEnd() && it.key()==this) {
        FMStand *stand = it.value();
        harvested += stand->totalThinningHarvest();
        ++it;
    }
    return harvested;
}

FMUnit::FMUnit(const Agent *agent)
{
    mAgent = agent;
    mScheduler = nullptr;
    mAnnualHarvestTarget = -1.;
    mRealizedHarvest = 0.;
    mMAI = 0.; mHDZ = 0.; mMeanAge = 0.;
    mTotalArea = 0.; mTotalPlanDeviation = 0.;
    mTotalVolume = 0.;
    mAnnualHarvest = 0.;
    mNumberOfStands = 0;
    mU = 100; mThinningIntensityClass = 2; mSpeciesCompositionIndex = 0;
    mAverageMAI = 0.;
    mForceUpdateManagementPlan=false;


    //if (agent->type()->schedulerOptions().useScheduler)
    // explicit scheduler only for stands/units that include more than one stand
    mScheduler = new Scheduler(this);
    //mScheduler->setEnabled( agent->schedulerOptions().useScheduler ); // enabled is switched on during agent init
}

FMUnit::~FMUnit()
{
    if (mScheduler)
        delete mScheduler;
}

void FMUnit::setId(const QString &id)
{
    mId = id;
}

void FMUnit::resetHarvestCounter()
{
    if (scheduler())
        scheduler()->resetHarvestCounter();
}

void FMUnit::managementPlanUpdate()
{
    const double period_length = 10.;
    // calculate the planned harvest in the next planning period (i.e., 10yrs).
    // this is the sum of planned operations that are already in the scheduler.
    double plan_thinning, plan_final;
    mScheduler->plannedHarvests(plan_final, plan_thinning);
    // the actual harvests of the last planning period
    //double realized = mRealizedHarvest;

    mRealizedHarvest = 0.; // reset
    mRealizedHarvestLastYear = 0.;
    mForceUpdateManagementPlan = false;


    // preparations:
    // MAI-calculation for all stands:
    double total_area = 0.;
    double age = 0.;
    double mai = 0.;
    double hdz = 0.;
    double volume = 0.;
    const QMultiMap<FMUnit*, FMStand*> &stands = ForestManagementEngine::instance()->stands();
    QMultiMap<FMUnit*, FMStand*>::const_iterator it = stands.constFind(this);
    while (it != stands.constEnd() && it.key()==this) {
        FMStand *stand = it.value();
        stand->reload();
        stand->calculateMAI();
        // calculate sustainable total harvest (following Breymann)
        double area = stand->area();
        mai += stand->meanAnnualIncrementTotal() * area; // m3/yr
        age += stand->absoluteAge() * area;
        volume += stand->volume() * area;
        // HDZ: "haubarer" average increment: timber that is ready for final harvest
        if (stand->readyForFinalHarvest())
            hdz += stand->volume() / stand->absoluteAge() * area; //(0.1* stand->U()) * area; // note: changed!!!! was: volume/age * area
        total_area += area;
        ++it;
    }
    // reset
    ForestManagementEngine::instance()->scriptBridge()->treesObj()->setStand(nullptr);
    mTotalArea = total_area;
    if (total_area==0.)
        return;

    mai /= total_area; // m3/ha*yr area weighted average of annual increment
    age /= total_area; // area weighted mean age
    hdz /= total_area; // =sum(Vol/age * share)

    mMAI = mai;
    //mMAI = mMAI * 1.15; // 15% increase, hack WR
    mHDZ = hdz;
    mMeanAge = age;
    mTotalVolume = volume;

    double rotation_length = U();
    double h_tot = mai * 2.*age / rotation_length;  //
    double h_reg = hdz * 2.*age / rotation_length;
    h_reg = h_tot * 0.85; // hack!
    h_reg *= agent()->schedulerOptions().harvestIntensity;
    h_tot *= agent()->schedulerOptions().harvestIntensity;
    double h_thi = qMax(h_tot - h_reg, 0.);

    qCDebug(abe) << "plan-update for unit" << id() << ": h-tot:" << h_tot << "h_reg:" << h_reg << "h_thi:" << h_thi << "of total volume:" << volume;
    double sf = mAgent->useSustainableHarvest();
    // we do not calculate sustainable harvest levels.
    // do a pure bottom up calculation
    double bottom_up_harvest = (plan_final / period_length) / total_area; // m3/ha*yr

    // the sustainable harvest yield is the current yield and some carry over from the last period
    double sustainable_harvest = h_reg;
//    if (mAnnualHarvestTarget>0.) {
//        double delta = realized/(total_area*period_length) - mAnnualHarvestTarget;
//        // if delta > 0: timber removal was too high -> plan less for the current period, and vice versa.
//        sustainable_harvest -= delta;
//    }
    mAnnualHarvestTarget = sustainable_harvest * sf + bottom_up_harvest * (1.-sf);
    mAnnualHarvestTarget = qMax(mAnnualHarvestTarget, 0.);

    mAnnualThinningTarget = (plan_thinning / period_length) / total_area; // m3/ha*yr


    if (scheduler()->enabled())
        scheduler()->setHarvestTarget(mAnnualHarvestTarget, mAnnualThinningTarget);
}

static QMutex _protect_agent_exec;
void FMUnit::runAgent()
{
    QMutexLocker m(&_protect_agent_exec); // avoid parallel execution of agent-code....

    // we need to set an execution context
    FMStand *stand = *ForestManagementEngine::instance()->stands().find(this);
    if (!stand)
        throw IException("Invalid stand in FMUnit::runAgent");

    FomeScript::setExecutionContext(stand, true); // true: add also agent as 'agent'

    QJSValue val;
    QJSValue agent_type = agent()->type()->jsObject();
    if (agent_type.property("run").isCallable()) {
        val = agent_type.property("run").callWithInstance(agent_type);
        qCDebug(abe) << "running agent-function 'run' for unit" <<  id() << ":" << val.toString();
    } else {
        if (stand->trace())
            qCDebug(abe) << "function 'run' is not a valid function of agent-type" << agent()->type()->name();
    }

}

void FMUnit::updatePlanOfCurrentYear()
{

    if (mTotalArea==0.)
        throw IException("FMUnit:updatePlan: unit area = 0???");

    // compare the harvests of the last year to the plan:
    double harvests = mRealizedHarvest - mRealizedHarvestLastYear;
    mRealizedHarvestLastYear = mRealizedHarvest;
    mAnnualHarvest = harvests;

    // we are done here if the scheduler is disabled
    if (!scheduler()->enabled())
        return;

    // difference in m3/ha
    double delta = harvests/mTotalArea - mAnnualHarvestTarget;
    mTotalPlanDeviation += delta;

    // apply decay function for deviation
    mTotalPlanDeviation *= mAgent->schedulerOptions().deviationDecayRate;
    qDebug() << "ABE-plan-deviation: annual harvest:" << mAnnualHarvest << "target: " << mAnnualHarvestTarget << "delta:" << delta << "total deviation:" << mTotalPlanDeviation;

    // relative deviation: >0: too many harvests
    double rel_deviation = mAnnualHarvestTarget? mTotalPlanDeviation / mAnnualHarvestTarget : 0.;

    // the current deviation is reduced to 50% in rebounce_yrs years.
    double rebounce_yrs = mAgent->schedulerOptions().scheduleRebounceDuration;

    double new_harvest = mAnnualHarvestTarget * (1. - rel_deviation/rebounce_yrs);

    // limit to minimum/maximum parameter
    new_harvest = qMax(new_harvest, mAgent->schedulerOptions().minScheduleHarvest);
    new_harvest = qMin(new_harvest, mAgent->schedulerOptions().maxScheduleHarvest);
    scheduler()->setHarvestTarget(new_harvest, mAnnualThinningTarget);

}

} // namesapce
