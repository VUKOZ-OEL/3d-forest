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
#include "bitecell.h"
#include "globalsettings.h"
#include "model.h"
#include "grid.h"
#include "resourceunit.h"
#include "biteagent.h"
#include "fmtreelist.h"
#include "fmsaplinglist.h"
#include "fmdeadtreelist.h"
#include "biteengine.h"
#include "bitelifecycle.h"
#include "biteclimate.h"

namespace BITE {

void BiteCell::setup(int cellidx, QPointF pos, BiteAgent *agent)
{
    mIndex = cellidx;
    mAgent = agent;

    if (agent->cellSize() > cRUSize) {
        // large cells need more setup
        largeCellSetup(pos);
    } else {
        // simpler case: smaller cells are always linked to a single resource unit
        mRU = GlobalSettings::instance()->model()->RUgrid().constValueAt(pos);
        mArea = agent->cellSize() * agent->cellSize() / static_cast<float>(cRUArea);
    }

}

QString BiteCell::info()
{
    return QString("[%1 - %2]").arg(index()).arg(agent()->name());
}

void BiteCell::checkTreesLoaded(ABE::FMTreeList *treelist)
{
    if (!mTreesLoaded) {
        loadTrees(treelist);
        mTreesLoaded = true;
    }

}

void BiteCell::checkSaplingsLoaded(ABE::FMSaplingList *saplist)
{
    if (!mSaplingsLoaded) {
        loadSaplings(saplist);
        mSaplingsLoaded=true;
    }
}

void BiteCell::checkDeadTreesLoaded(ABE::FMDeadTreeList *deadtreelist)
{
    if (!mDeadTreesLoaded) {
        loadDeadTrees(deadtreelist);
        mDeadTreesLoaded = true;
    }

}

double BiteCell::climateVar(int var_index) const
{
    if (!mRU)
        return 0.;
    // calculate the climate variable with 'var_index' - use the BiteClimate of the agent
    return mAgent->biteClimate().value(var_index, mRU);
}


void BiteCell::die()
{
    setActive(false);
    setSpreading(false);
    agent()->notifyItems(this, CellDied);
    mYearsLiving = 0;

    if (agent()->verbose())
        qCDebug(bite) << "cell died: " << info();

}

void BiteCell::finalize()
{
    if (isActive()) {
        agent()->stats().nActive++;
        mYearsLiving++;
        mCumYearsLiving++;

        // should the cell be active in the next iteration?
        setSpreading( agent()->lifeCycle()->shouldSpread(this) );
    }
}

void BiteCell::notify(ENotification what)
{
    switch (what) {
    case CellSpread:
        mLastSpread = BiteEngine::instance()->currentYear();
        break;
    default:
        break;
    }
}

int BiteCell::loadTrees(ABE::FMTreeList *treelist)
{
    Q_ASSERT(mRU != nullptr && mAgent != nullptr);
    if (agent()->cellSize()>cRUSize) {
        bool first = true;
        for (ResourceUnit *ru : agent()->largeCellRUs(index())) {
            treelist->loadFromRU(ru, !first);
            first = false;
        }
        return treelist->count();
    }

    QRectF rect = agent()->grid().cellRect( agent()->grid().indexOf(index()) );

    int added = treelist->loadFromRect(mRU, rect);
    return added;
}

int BiteCell::loadSaplings(ABE::FMSaplingList *saplinglist)
{
    Q_ASSERT(mRU != nullptr && mAgent != nullptr);

    if (agent()->cellSize()>cRUSize) {
        bool first = true;
        for (ResourceUnit *ru : agent()->largeCellRUs(index())) {
            saplinglist->loadFromRU(ru, !first);
            first = false;
        }
        return saplinglist->saplings().size();
    }

    QRectF rect = agent()->grid().cellRect( agent()->grid().indexOf(index()) );
    int added = saplinglist->loadFromRect(mRU, rect);
    return added;
}

int BiteCell::loadDeadTrees(ABE::FMDeadTreeList *deadtreelist)
{
    Q_ASSERT(mRU != nullptr && mAgent != nullptr);
    if (agent()->cellSize()>cRUSize) {
        bool first = true;
        for (ResourceUnit *ru : agent()->largeCellRUs(index())) {
            deadtreelist->loadFromRU(ru, ABE::FMDeadTreeList::DeadTreeType::Both,!first);
            first = false;
        }
        return deadtreelist->deadTrees().size();
    }

    QRectF rect = agent()->grid().cellRect( agent()->grid().indexOf(index()) );
    int added = deadtreelist->loadFromRect(mRU, rect);
    return added;

}

void BiteCell::largeCellSetup(QPointF pos)
{
    QRectF rect = agent()->grid().cellRect( agent()->grid().indexOf(index()) );
    rect &= GlobalSettings::instance()->model()->RUgrid().metricRect(); // intersect the grid (force valid range)
    GridRunner<ResourceUnit*> ru_runner(GlobalSettings::instance()->model()->RUgrid(), rect);
    double min_dist = std::numeric_limits<double>::max();
    QVector<ResourceUnit*> rus;
    mArea = 0.f;
    while (ResourceUnit **ru = ru_runner.next()) {
        if (*ru) {
            // save the resource unit with the smallest distance (the sqrt is not necessary)
            double dist = (ru_runner.currentCoord().x()-pos.x())*(ru_runner.currentCoord().x()-pos.x()) + (ru_runner.currentCoord().y()-pos.y())*(ru_runner.currentCoord().y()-pos.y());
            if (dist < min_dist) {
                min_dist = dist;
                mRU = *ru;
            }

            rus.push_back(*ru);
            mArea += static_cast<float>((*ru)->stockableArea()); // stockable area im m2
        }
    }

    // fix area (convert to ha)
    mArea = mArea / static_cast<float>(cRUArea);

    // save list of resource units (this copies the items from the QVector, no need to keep the container around)
    mAgent->setLargeCellRuList(index(), rus);

    qCDebug(bite) << "Setup of large cell" << index() << ": covers" << rus.size() << "RUs, total area (ha)" << mArea;
}



} // end namespace
