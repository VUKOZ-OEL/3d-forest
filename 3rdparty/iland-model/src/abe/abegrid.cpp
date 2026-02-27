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
#include "abegrid.h"
#include "fmstand.h"
#include "fmunit.h"
#include "modelcontroller.h"
#include "scheduler.h"
#include "agent.h"
#include "fmstp.h"
#include "forestmanagementengine.h"
#include "fomescript.h"


/** @class ABELayers
    @ingroup abe
    ABELayers is a helper class for spatial visualization of ABE data.

  */

ABELayers::~ABELayers()
{
    GlobalSettings::instance()->controller()->removeLayers(this);
}

double ABELayers::value(const FMStandPtr &data, const int index) const
{
    if (data == nullptr && index<2) return -1; // for classes
    if (data == nullptr) return 0;
    switch (index) {
    case 0:
        if(!mStandIndex.contains(data->id()))
            mStandIndex[data->id()] = mStandIndex.count();
        return mStandIndex[data->id()]; // "id"
    case 1:
        if(!mUnitIndex.contains(data->unit()->id()))
            mUnitIndex[data->unit()->id()] = mUnitIndex.count();
        return mUnitIndex[data->unit()->id()]; // unit
    case 2:
        if(!mAgentIndex.contains(data->unit()->agent()))
            mAgentIndex[data->unit()->agent()] = mAgentIndex.count();
        return mAgentIndex[data->unit()->agent()]; // unit

    case 3: return data->volume(); // "volume"
    case 4: return data->meanAnnualIncrement(); // mean annual increment m3/ha
    case 5: return data->meanAnnualIncrementTotal(); // mean annual increment m3/ha
    case 6: return data->basalArea(); // "basalArea"
    case 7: return data->age(); // "age"
    case 8: return data->lastExecution(); // "last evaluation"
    case 9: return data->sleepYears(); // "next evaluation"
    case 10: return data->lastUpdate(); // last update
    case 11: return data->unit()->constScheduler()?data->unit()->constScheduler()->scoreOf(data->id()) : -1.; // scheduler score
    case 12: if (!mSTPIndex.contains(data->stp()->name())) // stand treatment program
                mSTPIndex[data->stp()->name()] = mSTPIndex.count();
             return mSTPIndex[data->stp()->name()];
    case 13: return
                data->property(ABE::ForestManagementEngine::instance()->scriptBridge()->standVisualization()).toNumber();
    case 14: if (!mActivityIndex[data->currentActivity() ? data->currentActivity()->name() : QStringLiteral("invalid")])
                mActivityIndex[data->currentActivity() ? data->currentActivity()->name() : QStringLiteral("invalid")] = mActivityIndex.count();
            return mActivityIndex[data->currentActivity() ? data->currentActivity()->name() : QStringLiteral("invalid")];


    default: throw IException("ABELayers:value(): Invalid index");
    }
}

const QVector<LayeredGridBase::LayerElement> &ABELayers::names()
{
    if (mNames.isEmpty())
        mNames= QVector<LayeredGridBase::LayerElement>()
                << LayeredGridBase::LayerElement(QStringLiteral("id"), QStringLiteral("stand ID"), GridViewBrewerDiv)
                << LayeredGridBase::LayerElement(QStringLiteral("unit"), QStringLiteral("ID of the management unit"), GridViewBrewerDiv)
                << LayeredGridBase::LayerElement(QStringLiteral("agent"), QStringLiteral("managing agent"), GridViewBrewerDiv)
                << LayeredGridBase::LayerElement(QStringLiteral("volume"), QStringLiteral("stocking volume (m3/ha)"), GridViewRainbow)
                << LayeredGridBase::LayerElement(QStringLiteral("MAI"), QStringLiteral("mean annual increment (of the decade) (m3/ha)"), GridViewRainbow)
                << LayeredGridBase::LayerElement(QStringLiteral("MAITotal"), QStringLiteral("mean annual increment (full rotation) (m3/ha)"), GridViewRainbow)
                << LayeredGridBase::LayerElement(QStringLiteral("basalArea"), QStringLiteral("stocking basal area (m2/ha)"), GridViewRainbow)
                << LayeredGridBase::LayerElement(QStringLiteral("age"), QStringLiteral("stand age"), GridViewRainbow)
                << LayeredGridBase::LayerElement(QStringLiteral("last execution"), QStringLiteral("years since the last execution of an activity on the stand."), GridViewHeat)
                << LayeredGridBase::LayerElement(QStringLiteral("next evaluation"), QStringLiteral("year of the last execution"), GridViewHeat)
                << LayeredGridBase::LayerElement(QStringLiteral("last update"), QStringLiteral("year of the last update of the forest state."), GridViewRainbowReverse)
                << LayeredGridBase::LayerElement(QStringLiteral("scheduler score"), QStringLiteral("score of a stand in the scheduler (higher scores: higher prob. to be executed)."), GridViewRainbow)
                << LayeredGridBase::LayerElement(QStringLiteral("stp"), QStringLiteral("Stand treatment program currently active"), GridViewBrewerDiv)
                << LayeredGridBase::LayerElement(QStringLiteral("user"), QStringLiteral("stand property (fmengine.visualizationProperty)"), GridViewRainbow)
                << LayeredGridBase::LayerElement(QStringLiteral("activity"), QStringLiteral("Currently active activity for the stand"), GridViewBrewerDiv);
    return mNames;
}

const QString ABELayers::labelvalue(const int value, const int index) const
{
    switch(index) {
    case 0: // stand id
        return QString::number(mStandIndex.key(value));
    case 1: // unit
        return mUnitIndex.key(value);
    case 2: // agent
        return mAgentIndex.key(value)->name();
    case 12: // stp
        return mSTPIndex.key(value);
    case 14:
        return mActivityIndex.key(value);
    default: return QString::number(value);
    }
}

void ABELayers::registerLayers()
{
    GlobalSettings::instance()->controller()->addLayers(this, "ABE");
}

void ABELayers::clearClasses()
{
    mAgentIndex.clear();
    mStandIndex.clear();
    mUnitIndex.clear();
}
