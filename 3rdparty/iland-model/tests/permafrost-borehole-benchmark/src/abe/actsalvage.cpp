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
#include "actsalvage.h"

#include "fmstp.h"
#include "fmstand.h"
#include "fomescript.h"
#include "scheduler.h"
#include "forestmanagementengine.h"
#include "fmtreelist.h"

#include "tree.h"
#include "expression.h"
#include "expressionwrapper.h"
#include "mapgrid.h"
#include "helper.h"

namespace ABE {

/** @class ActSalvage
    @ingroup abe
    The ActSalvage class handles salvage logging after disturbances.

  */

ActSalvage::ActSalvage(FMSTP *parent): Activity(parent)
{
    mCondition = nullptr;
    mSanitationCondition = nullptr;
    mMaxPreponeActivity = 0;

    mBaseActivity.setIsSalvage(true);
    mBaseActivity.setIsRepeating(true);
    mBaseActivity.setExecuteImmediate(true);

}

ActSalvage::~ActSalvage()
{
    if (mCondition)
        delete mCondition;
}

void ActSalvage::setup(QJSValue value)
{
    Activity::setup(value); // setup base events
    events().setup(value, QStringList() << "onBarkBeetleAttack");

    QString condition = FMSTP::valueFromJs(value, "disturbanceCondition").toString();
    if (!condition.isEmpty() && condition!="undefined") {
        mCondition = new Expression(condition);
    }

    condition = FMSTP::valueFromJs(value, "sanitationCondition").toString();
    if (!condition.isEmpty() && condition!="undefined") {
        mSanitationCondition = new Expression(condition);
    }

    mMaxPreponeActivity = FMSTP::valueFromJs(value, "maxPrepone", "0").toInt();
    mThresholdSplit = FMSTP::valueFromJs(value, "thresholdSplitStand", "0.1").toNumber();
    mThresholdClear = FMSTP::valueFromJs(value, "thresholdClearStand", "0.9").toNumber();
    mThresholdMinimal = FMSTP::valueFromJs(value, "thresholdIgnoreDamage", "5").toNumber();
    mDebugSplit = FMSTP::boolValueFromJs(value, "debugSplit", false);

    mJSConditionFunc = FMSTP::valueFromJs(value, "onDisturbanceCondition", "");
    mJSCondition = mJSConditionFunc.isCallable();

}

bool ActSalvage::execute(FMStand *stand)
{


    if (stand->property("_run_salvage").toBool()) {
        // 2nd phase: do the after disturbance cleanup of a stand.
        bool simu = stand->currentFlags().isDoSimulate();
        bool is_final = stand->currentFlags().isFinalHarvest();
        stand->currentFlags().setDoSimulate(false);
        stand->currentFlags().setFinalHarvest(true); // this should be accounted as "final harvest"
        // execute the "onExecute" event
        bool result =  Activity::execute(stand);
        stand->currentFlags().setDoSimulate(simu);
        stand->currentFlags().setFinalHarvest(is_final);
        stand->setProperty("_run_salvage", false);
        return result;
    }

    // the salvaged timber is already accounted for - so nothing needs to be done here.
    // however, we check if there is a planned activity for the stand which could be executed sooner
    // than planned.
    bool preponed = const_cast<FMUnit*>(stand->unit())->scheduler()->forceHarvest(stand, mMaxPreponeActivity);
    if (stand->trace())
        qCDebug(abe) << "Salvage activity executed. Changed scheduled activites (preponed): " << preponed;

    // now in forestmanagementengine: const_cast<FMUnit*>(stand->unit())->scheduler()->addExtraHarvest(stand, stand->salvagedTimber(), Scheduler::Salvage);
    // check if we should re-assess the stand grid (after large disturbances)
    // as a preliminary check we only look closer, if we have more than  x m3/ha of damage.
    if (stand->disturbedTimber()/stand->area() > mThresholdMinimal)
        checkStandAfterDisturbance(stand);

    // the harvest happen(ed) anyways.
    //stand->resetHarvestCounter(); // set back to zero...
    return true;
}

QStringList ActSalvage::info()
{
    QStringList lines = Activity::info();
    lines << QString("condition: %1").arg(mCondition?mCondition->expression():"-");
    lines << QString("sanitationCondition: %1").arg(mSanitationCondition?mSanitationCondition->expression():"-");
    lines << QString("maxPrepone: %1").arg(mMaxPreponeActivity);
    return lines;
}

bool ActSalvage::evaluateRemove(Tree *tree) const
{
    bool result = true; // remove all trees is the default
    if (mCondition) {
        TreeWrapper tw(tree);
        result = static_cast<bool>( mCondition->execute(nullptr, &tw) );
    }
    if (result && mJSCondition) {
        // a second test based on JS
        QJSValue t = ForestManagementEngine::instance()->scriptBridge()->treeRef(tree);
        QJSValue func = mJSConditionFunc;
        result = func.call(QJSValueList() << t).toBool();

    }
    return result;
}

bool ActSalvage::barkbeetleAttack(FMStand *stand, double generations, int infested_px_ha)
{

    //QJSValue params;
    QJSValueList params=QJSValueList() << QJSValue(generations) << QJSValue(infested_px_ha);

    QJSValue result = events().run(QStringLiteral("onBarkBeetleAttack"), stand, &params);
    if (!result.isBool())
        qCDebug(abe) << "Salvage-Activity:onBarkBeetleAttack: expecting a boolean return";
    return result.toBool();
}

bool ActSalvage::checkSanitation(FMStand *stand)
{
   if (standFlags().enabled() && standFlags(stand).enabled()) {
       // activity is enabled. Now check
       if (mSanitationCondition) {
           bool result = static_cast<bool>( mSanitationCondition->execute() );
           if (stand->trace())
               qCDebug(abe) << "Sanitation for stand" << stand->id() << ": result:" << result;
           return result;
       }
   }
   return false;
}

void ActSalvage::checkStandAfterDisturbance(FMStand *stand)
{
    //
    FMTreeList *trees = ForestManagementEngine::instance()->scriptBridge()->treesObj();
    //trees->runGrid();
    trees->prepareStandGrid(QStringLiteral("height"), QString());

    const int min_split_size = 50; // min size (100=1ha)
    FloatGrid &grid = trees->standGrid();
    static int no_split = 0;
    if (mDebugSplit)
        trees->exportStandGrid(QString("temp/height_%1.txt").arg(++no_split));

    float h_max = grid.max();

    double r_low;
    int h_lower = 0, h_higher=0;
    if (h_max==0.f) {
        // total disturbance...
        r_low = 1.;
    } else {
        // check coverage of disturbed area.
        for (float *p=grid.begin(); p!=grid.end(); ++p) {
            if (*p>=0.f) {
                if (*p < h_max*0.33)
                    ++h_lower;
                else
                    ++h_higher;
            }
        }
        if (h_lower==0 && h_higher==0)
            return;
        // r_low is the fraction of 10m cells that are below 1/3rd of the stand top height
        r_low = h_lower / double(h_lower+h_higher);
    }


    // restart if a large fraction is cleared, or if the remaining forest is <0.5ha
    if (r_low > mThresholdClear || (r_low>0.5 && h_higher<min_split_size)) {
        // total disturbance: restart rotation...
        qCDebug(abe) << "ActSalvage: total damage for stand" << stand->id() << "Restarting rotation.";
        stand->setProperty("_run_salvage", true);
        stand->reset(stand->stp());
        return;
    }

    if (r_low < mThresholdSplit || (r_low<0.5 && h_lower<min_split_size)) {
        // no big damage: return and do nothing
        return;
    }

    // medium disturbance: check if need to split the stand area:
    Grid<int> my_map(grid.cellsize(), grid.sizeX(), grid.sizeY());
    GridRunner<float> runner(&grid);
    GridRunner<int> id_runner(&my_map);
    float *neighbors[8];
    int n_empty=0;
    while (runner.next() && id_runner.next()) {
        if (*runner.current()==-1.f) {
            *id_runner.current() = -1;
            continue;
        }
        runner.neighbors8(neighbors);
        double empty = 0.;
        int valid = 0;
        for (int i=0;i<8;++i) {
            if (neighbors[i] && *neighbors[i]<h_max*0.33)
                empty++;
            if (neighbors[i])
                valid++;
        }
        if (valid)
            empty /= double(valid);
        // empty cells are marked with 0; areas covered by forest set to stand_id; -1: out-of-stand areas
        // if a cell is empty, some neighbors (i.e. >50%) need to be empty too;
        // if a cell is *not* empty, it has to be surrounded by a larger fraction of empty points (75%)
        if ( (*runner.current()<h_max*0.33 && empty>0.5)
             || (empty>=0.75) ) {
            *id_runner.current() = 0;
            n_empty++;
        } else {
            *id_runner.current() = stand->id();
        }
    }
    if (mDebugSplit)
        Helper::saveToTextFile(GlobalSettings::instance()->path(QString("temp/split_before_%1.txt").arg(no_split)), gridToESRIRaster(my_map) );


    // now flood-fill 0ed areas....
    // if the "new" areas are too small (<0.25ha), then nothing happens.
    QVector<QPair<int,int> > cleared_small_areas; // areas of cleared "patches"
    QVector<QPair<int,int> > stand_areas; // areas of remaining forested "patches"
    int fill_color = -1;
    int stand_fill_color = stand->id() + 1000;
    id_runner.reset();
    while (id_runner.next()) {
        if (*id_runner.current()==0) {
            int s = floodFillHelper(my_map, id_runner.currentIndex(), 0, --fill_color);
            cleared_small_areas.push_back(QPair<int,int>(fill_color, s)); // patch size
        } else if (*id_runner.current()==stand->id()) {
            int s=floodFillHelper(my_map, id_runner.currentIndex(), stand->id(), stand_fill_color);
            stand_areas.push_back(QPair<int,int>(stand_fill_color,s));
            stand_fill_color++;
        }
    }
    if (mDebugSplit)
        Helper::saveToTextFile(GlobalSettings::instance()->path(QString("temp/split_stands_%1.txt").arg(no_split)), gridToESRIRaster(my_map) );


    // special case: remainnig forest are only small patches
    int max_size=0;
    for (int i=0;i<stand_areas.size();++i)
        max_size=std::max(max_size, stand_areas[i].second);
    if (max_size<min_split_size) {
        // total disturbance: restart rotation...
        qCDebug(abe) << "ActSalvage: total damage for stand" << stand->id() << "(remaining patch too small). Restarting rotation.";
        stand->setProperty("_run_salvage", true);
        stand->reset(stand->stp());
        return;
    }

    // clear small areas
    QVector<int> neighbor_ids;
    bool finished=false;
    int iter=100;
    while (!finished && cleared_small_areas.size()>0 && --iter>0) {

        // find smallest area....
        int i_min=-1;
        for (int i=0;i<cleared_small_areas.size();++i) {
            if (cleared_small_areas[i].second<min_split_size) {
                if (i_min==-1 || (i_min>-1 && cleared_small_areas[i].second<cleared_small_areas[i_min].second))
                    i_min = i;
            }
        }
        if (i_min==-1) {
            finished = true;
            continue;
        }

        // loook for neighbors of the area
        // attach to largest "cleared" neighbor (if such a neighbor exists)
        neighborFinderHelper(my_map,neighbor_ids, cleared_small_areas[i_min].first);
        if (neighbor_ids.size()==0) {
            // patch fully surrounded by "out of project area". We'll add it to the *first* stand map entry
            neighbor_ids.append(stand_areas.first().first);
        }
        // look for "empty patches" first
        int i_empty=-1; int max_size=0;
        for (int i=0;i<cleared_small_areas.size();++i) {
            if (neighbor_ids.contains(cleared_small_areas[i].first))
                if (cleared_small_areas[i].second>max_size) {
                    i_empty=i;
                    max_size=cleared_small_areas[i].second;
                }
        }
        if (i_empty>-1) {
            // replace "i_min" with "i_empty"
            int r = replaceValueHelper(my_map, cleared_small_areas[i_min].first, cleared_small_areas[i_empty].first);
            cleared_small_areas[i_empty].second += r;
            cleared_small_areas.remove(i_min);
            continue;
        }


        if (stand_areas.size()>0) {
            // attach to largest stand part which is a neighbor

            int i_empty=-1; int max_size=0;
            for (int i=0;i<stand_areas.size();++i) {
                if (neighbor_ids.contains(stand_areas[i].first))
                    if (stand_areas[i].second>max_size) {
                        i_empty=i;
                        max_size=stand_areas[i].second;
                    }
            }
            if (i_empty>-1) {
                // replace "i_min" with "i_empty"
                int r = replaceValueHelper(my_map, cleared_small_areas[i_min].first, stand_areas[i_empty].first);
                stand_areas[i_empty].second += r;
                cleared_small_areas.remove(i_min);
            }
        }
        if (iter==3)
            qCDebug(abe) << "ActSalvage:Loop1: no solution.";
    }


    // clear small forested stands
    finished = false;
    iter = 100;
    while (!finished && --iter>0) {
        finished=true;
        for (int i=0;i<stand_areas.size();++i) {
            if (stand_areas[i].second < min_split_size) {
                neighborFinderHelper(my_map, neighbor_ids, stand_areas[i].first);

                if (neighbor_ids.size()>0) {
                    int r = replaceValueHelper(my_map, stand_areas[i].first, neighbor_ids[0]);
                    if (neighbor_ids[0]>0) {
                        // another stand
                        for (int j=0;j<stand_areas.size();++j)
                            if (stand_areas[j].first == neighbor_ids[0]) {
                                stand_areas[j].second += r;
                            }
                    } else {
                        // clearing
                        for (int j=0;j<cleared_small_areas.size();++j)
                            if (cleared_small_areas[j].first == neighbor_ids[0]) {
                                cleared_small_areas[j].second += r;
                            }
                    }

                    stand_areas.remove(i);
                    finished=false;
                    break;
                }
            }
        }
    }
    if (iter==0)
        qCDebug(abe) << "ActSalvage:Loop2: no solution.";
    if (mDebugSplit)
        Helper::saveToTextFile(GlobalSettings::instance()->path(QString("temp/split_final_%1.txt").arg(no_split)), gridToESRIRaster(my_map) );


    // determine final new stands....
    QVector<int> new_stands; // internal ids that should become new stands

    for (int i=0;i<cleared_small_areas.size();++i)
        new_stands.push_back(cleared_small_areas[i].first);

    // only add new stands - keep the old stand as is
    //    if (new_stands.size()>0) {
//        // if there are no "cleared" parts, we keep the stand as is.
//        for (int i=0;i<stand_areas.size();++i)
//            if (stand_areas[i].first != stand->id()+1000)
//                new_stands.push_back(stand_areas[i].first);
//    }


    for (int i=0;i<new_stands.size(); ++i) {
        // ok: we have new stands. Now do the actual splitting
        FMStand *new_stand = ForestManagementEngine::instance()->splitExistingStand(stand);
        // copy back to the stand grid
        GridRunner<int> sgrid(ForestManagementEngine::instance()->standGrid()->grid(), grid.metricRect());
        id_runner.reset();
        int n_px=0;
        while (sgrid.next() && id_runner.next()) {
            if (*id_runner.current() == new_stands[i]) {
                *sgrid.current() = new_stand->id();
                ++n_px;
            }
        }

        // the new stand  is prepared.
        // at the end of this years execution, the stand will be re-evaluated.
        new_stand->setInitialId(stand->id());
        // year of splitting: all the area of the stand is still accounted for for the "old" stand
        // in the next year (after the update of the stand grid), the old stand shrinks and the new
        // stands get their correct size.
        // new_stand->setArea(n_px / (cHeightSize*cHeightSize));
        new_stand->setProperty("_run_salvage", true);
        new_stand->reset(stand->stp());
        qCDebug(abe) << "ActSalvage: new stand" << new_stand->id() << "parent stand" << stand->id() << "#split:" << no_split;
    }
}

// quick and dirty implementation of the flood fill algroithm.
// based on: http://en.wikipedia.org/wiki/Flood_fill
// returns the number of pixels colored
int ActSalvage::floodFillHelper(Grid<int> &grid, QPoint start, int old_color, int color)
{
    QQueue<QPoint> pqueue;
    pqueue.enqueue(start);
    int found = 0;
    while (!pqueue.isEmpty()) {
        QPoint p = pqueue.dequeue();
        if (!grid.isIndexValid(p))
            continue;
        if (grid.valueAtIndex(p)==old_color) {
            grid.valueAtIndex(p) = color;
            pqueue.enqueue(p+QPoint(-1,0));
            pqueue.enqueue(p+QPoint(1,0));
            pqueue.enqueue(p+QPoint(0,-1));
            pqueue.enqueue(p+QPoint(0,1));
            pqueue.enqueue(p+QPoint(1,1));
            pqueue.enqueue(p+QPoint(1,-1));
            pqueue.enqueue(p+QPoint(-1,1));
            pqueue.enqueue(p+QPoint(-1,-1));
            ++found;
        }
    }
    return found;
}

// find all neigbors of color 'stand_id' and save in the 'neighbors' vector
int ActSalvage::neighborFinderHelper(Grid<int> &grid, QVector<int> &neighbors, int stand_id)
{
    GridRunner<int> id_runner(&grid);
    neighbors.clear();
    int *nb[8];
    while (id_runner.next()) {
        if (*id_runner.current()==stand_id) {
            id_runner.neighbors8(nb);
            for (int i=0;i<8;++i)
                if (nb[i] && *nb[i]!=-1 && *nb[i]!=stand_id) {
                    if (!neighbors.contains(*nb[i]))
                        neighbors.push_back(*nb[i]);
                }
        }
    }
    return neighbors.size();
}

int ActSalvage::replaceValueHelper(Grid<int> &grid, int old_value, int new_value)
{
    int n=0;
    for (int *p=grid.begin(); p!=grid.end(); ++p)
        if (*p == old_value) {
            *p = new_value;
            ++n;
        }
    return n;
}


} // namespace
