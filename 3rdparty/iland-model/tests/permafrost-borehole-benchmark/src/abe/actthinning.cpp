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
#include "actthinning.h"

#include "fmstand.h"
#include "fmtreelist.h"
#include "fmstp.h"
#include "forestmanagementengine.h"
#include "fomescript.h"
#include "model.h"
#include "speciesset.h"
#include "species.h"

#include "tree.h"

#include <QJSValueIterator>
namespace ABE {

/** @class ActThinning
    @ingroup abe
    The ActThinning class implements a very general interface to thinning activties.

  */

// statics
QStringList ActThinning::mSyntaxCustom;


ActThinning::ActThinning(FMSTP *parent): Activity(parent)
{
    mBaseActivity.setIsScheduled(true); // use the scheduler
    mBaseActivity.setDoSimulate(true); // simulate per default
    mThinningType = Invalid;
    if (mSyntaxCustom.isEmpty())
        mSyntaxCustom = QStringList()  << Activity::mAllowedProperties
                                       << "percentile" << "removal" << "thinning"
                                       << "relative" << "remainingStems" << "minDbh"
                                       << "filter" << "targetVariable" << "targetRelative"
                                       << "targetValue" << "classes" << "onEvaluate" << "onExecute";
}

QString ActThinning::type() const
{
    QString th;
    switch (mThinningType) {
    case Invalid: th="Invalid"; break;
    case FromBelow: th="from below"; break;
    case FromAbove: th="from above"; break;
    case Custom: th = "custom"; break;
    case Selection: th = "selection"; break;
    }

    return QString("thinning (%1)").arg(th);
}

void ActThinning::setup(QJSValue value)
{
    Activity::setup(value); // setup base events
    mThinningType = Invalid;
    QString th_type = FMSTP::valueFromJs(value, "thinning").toString();
    if (th_type=="fromBelow") mThinningType=FromBelow;
    else if (th_type=="fromAbove") mThinningType=FromAbove;
    else if (th_type=="custom") mThinningType=Custom;
    else if (th_type=="selection") mThinningType=Selection;
    else
        throw IException(QString("Setup of thinning: invalid thinning type: %1").arg(th_type));

    switch (mThinningType) {
    case Custom: setupCustom(value); break;
    case Selection: setupSelective(value); break;
    default: throw IException("No setup defined for thinning type");
    }

}

bool ActThinning::evaluate(FMStand *stand)
{
    bool return_value = true;
    switch (mThinningType) {
    case Custom:
        for (int i=0;i<mCustomThinnings.count();++i)
            return_value = return_value && evaluateCustom(stand, mCustomThinnings[i]);
        return return_value; // false if one fails

    case Selection:
        return evaluateSelective(stand);
    default: throw IException("ActThinning::evaluate: not available for thinning type");
    }
    return false;
}

bool ActThinning::execute(FMStand *stand)
{
    if (stand->trace()) qCDebug(abe) << stand->context() << "execute  activity" << name() << ":" << type();
    if (events().hasEvent(QStringLiteral("onExecute"))) {
        // switch off simulation mode
        stand->currentFlags().setDoSimulate(false);
        // execute this event
        bool result =  Activity::execute(stand);
        stand->currentFlags().setDoSimulate(true);
        return result;
    } else {
        // default behavior: process all marked trees (harvest / cut)
        if (stand->trace()) qCDebug(abe) << stand->context() << "activity" << name() << "remove all marked trees.";
        FMTreeList trees(stand);
        trees.removeMarkedTrees();
        return true;
    }

}

void ActThinning::setupCustom(QJSValue value)
{
    events().setup(value, QStringList() << "onEvaluate");
    mCustomThinnings.clear();
    if (value.hasProperty("thinnings") && value.property("thinnings").isArray()) {
        QJSValueIterator it(value.property("thinnings"));
        while (it.hasNext()) {
            it.next();
            if (it.name()==QStringLiteral("length"))
                continue;
            mCustomThinnings.push_back(SCustomThinning());
            setupSingleCustom(it.value(), mCustomThinnings.back());
        }
    } else {
        mCustomThinnings.push_back(SCustomThinning());
        setupSingleCustom(value, mCustomThinnings.back());
    }
}

void ActThinning::setupSelective(QJSValue value)
{
    mSelectiveThinning.N = FMSTP::valueFromJs(value, "N", "400").toInt();
}

// setup of the "custom" thinning operation
void ActThinning::setupSingleCustom(QJSValue value, SCustomThinning &custom)
{
    FMSTP::checkObjectProperties(value, mSyntaxCustom, "setup of 'custom' thinning:" + name());

    custom.usePercentiles = FMSTP::boolValueFromJs(value, "percentile", true);
    custom.removal = FMSTP::boolValueFromJs(value, "removal", true);
    custom.relative = FMSTP::boolValueFromJs(value, "relative", true);
    custom.remainingStems = FMSTP::valueFromJs(value, "remainingStems", "0");
    custom.minDbh = FMSTP::valueFromJs(value, "minDbh", "0");
    QJSValue filter = FMSTP::valueFromJs(value, "filter", "");
    if (filter.isString())
        custom.filter = filter.toString();
    else
        custom.filter = QString();
    custom.targetVariable = FMSTP::valueFromJs(value, "targetVariable", "stems").toString();
    if (custom.targetVariable != "stems" &&
            custom.targetVariable != "basalArea" &&
            custom.targetVariable != "volume")
        throw IException(QString("setup of custom Activity: invalid targetVariable: %1").arg(custom.targetVariable));

    custom.targetRelative = FMSTP::boolValueFromJs(value, "targetRelative", true);
    custom.targetValue = FMSTP::valueFromJs(value, "targetValue", "30");

    QJSValue values = FMSTP::valueFromJs(value, "classes",  "", "setup custom acitvity");
    if (!values.isArray())
        throw IException("setup of custom activity: the 'classes' is not an array.");
    custom.classValues.clear();
    custom.classPercentiles.clear();
    QJSValueIterator it(values);
    while (it.hasNext()) {
        it.next();
        if (it.name()==QStringLiteral("length"))
            continue;
        custom.classValues.push_back(it.value().toNumber());
    }
    if (custom.classValues.size()==0)
        throw IException("setup of custom thinnings: 'classes' has no elements.");

    // check if sum is 100 for relative classes
    if (custom.relative) {
        double sum=0.;
        for (int i=0;i<custom.classValues.size();++i)
            sum+=custom.classValues[i];
        if (fabs(sum-100.)>0.000001)
            throw IException("setup of custom thinnings: 'classes' do not add up to 100 (relative=true).");
    }

    // span the range between 0..100: from e.g. 10,20,30,20,20 -> 0,10,30,60,80,100
    double f = 100. / custom.classValues.size();
    double p = 0.;
    for (int i=0;i<custom.classValues.size();++i, p+=f)
        custom.classPercentiles.push_back(qRound(p));
    custom.classPercentiles.push_back(100);
}

bool ActThinning::evaluateCustom(FMStand *stand, SCustomThinning &custom)
{
    // fire onEvaluate event and collect probabilities
    QJSValue eval_result = events().run(QStringLiteral("onEvaluate"), stand);
    if (eval_result.isBool() && eval_result.toBool()==false)
        return false; // do nothing
    bool species_selective = false;

    if( eval_result.isObject()) {
        // expecting a list of probabilities....
        // create list if not present
        if (mSpeciesSelectivity.isEmpty()) {
            foreach(const Species *s, GlobalSettings::instance()->model()->speciesSet()->activeSpecies())
                mSpeciesSelectivity[s] = 1.;
        }
        // fetch from javascript
        double rest_val = eval_result.property(QStringLiteral("rest")).isNumber() ?  eval_result.property(QStringLiteral("rest")).toNumber() : 1.;
        foreach(const Species *s, mSpeciesSelectivity.keys()) {
            mSpeciesSelectivity[s] = limit( eval_result.property(s->id()).isNumber() ? eval_result.property(s->id()).toNumber() : rest_val, 0., 1.);
        }
        species_selective = true;
    }
    // process the dynamic variables
    double target_value = FMSTP::evaluateJS(custom.targetValue).toNumber(); // evaluate dynamically at runtime
    double min_dbh = FMSTP::evaluateJS(custom.minDbh).toNumber();
    double remaining_stems = FMSTP::evaluateJS(custom.remainingStems).toNumber();

    if (custom.targetRelative && (target_value>100. || target_value<0.))
        throw IException(QString("Thinning activity: invalid relative targetValue (0-100): %1").arg(target_value));


    FMTreeList trees(stand);
    QString filter = custom.filter;
    if (min_dbh>0.) {
        if (!filter.isEmpty())
            filter += " and ";
        filter += QString("dbh>%1").arg(min_dbh);
    }

    if (!filter.isEmpty())
        trees.load(filter);
    else
        trees.loadAll();

    if (remaining_stems>0 && remaining_stems>=trees.trees().size())
        return false;

    if (trees.trees().size()==0)
        return false;

    // remove harvest flags.
    clearTreeMarks(&trees);

    // sort always by target variable (if it is stems, then simply by dbh)
    bool target_dbh = custom.targetVariable=="stems";
    if (target_dbh)
        trees.sort("dbh");
    else
        trees.sort(custom.targetVariable);

    // count trees and values (e.g. volume) in the defined classes
    QVector<double> values = custom.classValues; // make a copy
    QVector<double> tree_counts = custom.classValues; // make a copy
    QVector<int> percentiles = custom.classPercentiles; // make a copy

    QVector<QPair<Tree*, double> >::const_iterator it;
    for (int i=0;i<values.count();++i) {
        tree_counts[i]=0.;
    }
    int class_index = 0;
    int n=0;

    percentiles.first()=0;
    double tree_count=trees.trees().count();
    double total_value = 0.;
    for (it = trees.trees().constBegin(); it!=trees.trees().constEnd(); ++it, ++n) {
        if (n/tree_count*100. > custom.classPercentiles[class_index+1]) {
            ++class_index;
            percentiles[class_index] = n; // then n'th tree
        }
        tree_counts[class_index]++;
        total_value += target_dbh?1.:it->second; // e.g., sum of volume in the class, or simply count
    }
    while (++class_index<percentiles.size())
        percentiles[class_index]=n+1;

    double calc_target_value=0.;
    if (custom.targetRelative)
        calc_target_value = target_value * total_value / 100.;
    else
        calc_target_value = target_value * stand->area();

    if (!custom.relative) {
        // TODO: does not work now!!! redo!!
        // class values are given in absolute terms, e.g. 40m3/ha.
        // this needs to be translated to relative classes.
        // if the demand in a class cannot be met (e.g. planned removal of 40m3/ha, but there are only 20m3/ha in the class),
        // then the "miss" is distributed to the other classes (via the scaling below).
        for (int i=0;i<values.size();++i) {
            if (values[i]>0){
                if (values[i]<=custom.classValues[i]*stand->area()) {
                    values[i] = 1.; // take all from the class
                } else {
                    values[i] = custom.classValues[i]*stand->area() / values[i];
                }
            }
        }
        // scale to 100
        double sum=0.;
        for (int i=0;i<values.size();++i)
            sum+=values[i];
        if (sum>0.){
            for (int i=0;i<values.size();++i)
                values[i] *= 100./sum;
        }
    }

    // *****************************************************************
    // ***************    Main loop
    // *****************************************************************
    for (int i=0;i<values.size();++i)
        values[i] = 0;

    bool finished = false;
    int cls;
    double p;
    int removed_trees = 0;
    double removed_value = 0.;
    int no_tree_found = 0;
    bool target_value_reached=false;
    do {
        // look up a random number: it decides in which class to select a tree:
        p = nrandom(0,100);
        for (cls=0;cls<values.size();++cls) {
            if (p < custom.classPercentiles[cls+1])
                break;
        }
        // select a tree:
        int tree_idx = selectRandomTree(&trees, percentiles[cls], percentiles[cls+1]-1, species_selective);
        if (tree_idx>=0) {
            // stop harvesting, when the target size is reached: if the current tree would surpass the limit,
            // a random number decides whether the tree should be included or not.
            double tree_value = target_dbh?1.:trees.trees()[tree_idx].second;
            if (target_value>0.) {
                if (removed_value + tree_value > calc_target_value) {
                    if (drandom()>0.5 || target_value_reached)
                        break;
                    else
                        target_value_reached = true;
                }

            }
            trees.remove_single_tree(tree_idx, true);
            removed_trees++;
            removed_value += tree_value;
            values[cls]++;

        } else {
            // tree_idx = -1: no tree found in list, -2: tree found but is not selected
            no_tree_found += tree_idx == -1 ? 100 : 1; // empty list counts much more
            if (no_tree_found > 1000)
                finished=true;
        }
        // stop harvesting, when the minimum remaining number of stems is reached
        if (trees.trees().size()-removed_trees <= remaining_stems*stand->area())
            finished = true;

        if (target_value>0. && removed_value > calc_target_value)
            finished = true;

    } while (!finished);

    if (stand->trace()) {
        qCDebug(abe) << stand->context() << "custom-thinning: removed" << removed_trees << ". Reached cumulative 'value' of:" << removed_value << "(planned value:" << calc_target_value << "). #of no trees found:" << no_tree_found << "; stand-area:" << stand->area();
        for (int i=0;i<values.count();++i)
            qCDebug(abe) << stand->context() << "class" << i << ": removed" << values[i] << "of" << percentiles[i+1]-percentiles[i];
    }

    return true;

}

int ActThinning::selectRandomTree(FMTreeList *list, const int pct_min, const int pct_max, const bool selective)
{
    // pct_min, pct_max: the indices of the first and last tree in the list to be looked for, including pct_max
    // seek a tree in the class 'cls' (which has not already been removed);
    int idx;
    if (pct_max<pct_min)
        return -1;
    // search randomly for a couple of times
    for (int i=0;i<5;i++) {
        idx = irandom(pct_min, pct_max);
        Tree *tree = list->trees().at(idx).first;
        if (!tree->isDead() && !tree->isMarkedForHarvest() && !tree->isMarkedForCut())
            return selectSelectiveSpecies(list, selective, idx);
    }
    // not found, now walk in a random direction...
    int direction = 1;
    if (drandom()>0.5) direction=-1;
    // start in one direction from the last selected random position
    int ridx=idx;
    while (ridx>=pct_min && ridx<pct_max) {
        Tree *tree = list->trees().at(ridx).first;
        if (!tree->isDead() && !tree->isMarkedForHarvest() && !tree->isMarkedForCut())
            return selectSelectiveSpecies(list, selective, ridx);

        ridx+=direction;
    }
    // now look in the other direction
    direction = -direction;
    ridx = idx;
    while (ridx>=pct_min && ridx<pct_max) {
        Tree *tree = list->trees().at(ridx).first;
        if (!tree->isDead() && !tree->isMarkedForHarvest() && !tree->isMarkedForCut())
            return selectSelectiveSpecies(list, selective, ridx);

        ridx+=direction;
    }

    // no tree found in the entire range
    return -1;


}

int ActThinning::selectSelectiveSpecies(FMTreeList *list, const bool is_selective, const int index)
{
    if (!is_selective)
        return index;
    // check probability for species [0..1, 0: 0% chance to take a tree of that species] against a random number
    if (mSpeciesSelectivity[list->trees()[index].first->species()] < drandom())
        return index; // take the tree

    // a tree was found but is not going to be removed
    return -2;
}

void ActThinning::clearTreeMarks(FMTreeList *list)
{
    QVector<QPair<Tree*, double> >::const_iterator it;
    for (it=list->trees().constBegin(); it!=list->trees().constEnd(); ++it) {
        Tree *tree = it->first;
        if (tree->isMarkedForHarvest())
            tree->markForHarvest(false);
        if (tree->isMarkedForCut())
            tree->markForCut(false);

    }
}

bool ActThinning::evaluateSelective(FMStand *stand)
{
    markCropTrees(stand);
    return true;
}

bool ActThinning::markCropTrees(FMStand *stand)
{
    // tree list from current exeution context
    FMTreeList *treelist = ForestManagementEngine::instance()->scriptBridge()->treesObj();
    treelist->setStand(stand);
    treelist->loadAll();
    clearTreeMarks(treelist);

    // get the 2x2m grid for the current stand
    Grid<float> &grid = treelist->localGrid();
    // clear (except the out of "stand" pixels)
    for (float *p=grid.begin(); p!=grid.end(); ++p)
        if (*p > -1.f)
            *p = 0.f;

    int target_n = mSelectiveThinning.N * stand->area();

    if (target_n>=treelist->trees().count())
        target_n = treelist->trees().count();

    int max_target_n = qMax(target_n * 1.5, treelist->trees().count()/2.);
    if (max_target_n>=treelist->trees().count())
        max_target_n = treelist->trees().count();
    // we have 2500 px per ha (2m resolution)
    // if each tree dominates its Moore-neighborhood, 2500/9 = 267 trees are possible (/ha)
    // if *more* trees should be marked, some trees need to be on neighbor pixels:
    // pixels = 2500 / N; if 9 px are the Moore neighborhood, the "overprint" is N*9 / 2500.
    // N*)/2500 -1 = probability of having more than zero overlapping pixels
    double overprint = (mSelectiveThinning.N * 9) / double(cPxPerHectare) - 1.;

    // order the list of trees according to tree height
    treelist->sort("-height");

    // start with a part of N and 0 overlap
    int n_found = 0;
    int tests=0;
    for (int i=0;i<target_n/3;i++) {
        float f=testPixel(treelist->trees().at(i).first->position(), grid); ++tests;
        if (f==0.f) {
            setPixel(treelist->trees().at(i).first->position(), grid);
            treelist->trees()[i].first->markCropTree(true);
            ++n_found;
        }
    }
    // continue with a higher probability --- incr
    for (int run=0;run<4;++run) {
        for (int i=0; i<max_target_n;++i) {
            if (treelist->trees().at(i).first->isMarkedAsCropTree())
                continue;

            float f=testPixel(treelist->trees().at(i).first->position(), grid); ++tests;

            if ( (f==0.f) ||
                 (f<=2.f && drandom()<overprint) ||
                 (run==1 && f<=4 && drandom()<overprint) ||
                 (run==2 && drandom()<overprint) ||
                 (run==3) ) {

                setPixel(treelist->trees().at(i).first->position(), grid);
                treelist->trees()[i].first->markCropTree(true);
                ++n_found;
                if (n_found == target_n)
                    break;
            }
        }
        if (n_found==target_n)
            break;
    }

    // now mark the competitors:
    // competitors are trees up to 75th percentile of the tree population that
    int n_competitor=0;
    for (int run=0;run<3;++run) {
        for (int i=0; i<max_target_n;++i) {
            Tree *tree = treelist->trees().at(i).first;
            if (tree->isMarkedAsCropTree() || tree->isMarkedAsCropCompetitor())
                continue;

            float f=testPixel(treelist->trees().at(i).first->position(), grid); ++tests;

            if ( (f>12.f) ||
                 (run==1 && f>8) ||
                 (run==2 && f>4) ) {
                tree->markCropCompetitor(true);
                n_competitor++;
            }
        }
    }


    if (FMSTP::verbose()) {
        qCDebug(abe) << stand->context() << "Thinning::markCropTrees: marked" << n_found << "(plan:" << target_n << ") from total" << treelist->trees().count()
                     << ". Tests performed:" << tests << "marked as competitors:" << n_competitor;
    }
    return n_found==target_n;

}

float ActThinning::testPixel(const QPointF &pos, Grid<float> &grid)
{
    // check Moore neighborhood
    int x=grid.indexAt(pos).x();
    int y=grid.indexAt(pos).y();

    float sum = 0.f;
    sum += grid.isIndexValid(x-1,y-1) ? grid.valueAtIndex(x-1, y-1) : 0;
    sum += grid.isIndexValid(x,y-1) ? grid.valueAtIndex(x, y-1) : 0;
    sum += grid.isIndexValid(x+1,y-1) ? grid.valueAtIndex(x+1, y-1) : 0;

    sum += grid.isIndexValid(x-1,y) ? grid.valueAtIndex(x-1, y) : 0;
    sum += grid.isIndexValid(x,y) ? grid.valueAtIndex(x, y) : 0;
    sum += grid.isIndexValid(x+1,y) ? grid.valueAtIndex(x+1, y) : 0;

    sum += grid.isIndexValid(x-1,y+1) ? grid.valueAtIndex(x-1, y+1) : 0;
    sum += grid.isIndexValid(x,y+1) ? grid.valueAtIndex(x, y+1) : 0;
    sum += grid.isIndexValid(x+1,y+1) ? grid.valueAtIndex(x+1, y+1) : 0;

    return sum;
}

void ActThinning::setPixel(const QPointF &pos, Grid<float> &grid)
{
    // check Moore neighborhood
    int x=grid.indexAt(pos).x();
    int y=grid.indexAt(pos).y();

    if (grid.isIndexValid(x-1,y-1)) grid.valueAtIndex(x-1, y-1)++;
    if (grid.isIndexValid(x,y-1)) grid.valueAtIndex(x, y-1)++;
    if (grid.isIndexValid(x+1,y-1)) grid.valueAtIndex(x+1, y-1)++;

    if (grid.isIndexValid(x-1,y)) grid.valueAtIndex(x-1, y)++;
    if (grid.isIndexValid(x,y)) grid.valueAtIndex(x, y) += 3; // more impact on center pixel
    if (grid.isIndexValid(x+1,y)) grid.valueAtIndex(x+1, y)++;

    if (grid.isIndexValid(x-1,y+1)) grid.valueAtIndex(x-1, y+1)++;
    if (grid.isIndexValid(x,y+1)) grid.valueAtIndex(x, y+1)++;
    if (grid.isIndexValid(x+1,y+1)) grid.valueAtIndex(x+1, y+1)++;
}


} // namespace
