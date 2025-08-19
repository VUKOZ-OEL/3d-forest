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
#include "fmsaplinglist.h"
#include "resourceunitspecies.h"

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
    case Tending: th="tending"; break;
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
    else if (th_type=="tending") mThinningType=Tending;
    else
        throw IException(QString("Setup of thinning: invalid thinning type: %1").arg(th_type));

    switch (mThinningType) {
    case Custom: setupCustom(value); break;
    case Selection: setupSelective(value); break;
    case Tending: setupTending(value); break;
    default: throw IException("No setup defined for thinning type");
    }

    if (isRepeatingActivity())
        mBaseActivity.setIsScheduled(false);

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

    case Tending:
        return evaluateTending(stand);

    default: throw IException("ActThinning::evaluate: not available for thinning type");
    }
    return false;
}

bool ActThinning::execute(FMStand *stand)
{
    if (stand->trace()) qCDebug(abe) << stand->context() << "execute  activity" << name() << ":" << type();
    if (!stand->currentFlags().isScheduled()) {
        // if scheduling is off for this thinning activity,
        // then we need to invoke this manually.
        evaluate(stand);
    }
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
    events().setup(value, FomeScript::bridge()->activityJS(), QStringList() << "onEvaluate");
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
    mSelectiveThinning.N = FMSTP::valueFromJs(value, "N", "400");
    mSelectiveThinning.speciesProb = FMSTP::valueFromJs(value, "speciesSelectivity");
    mSelectiveThinning.rankingExpr = FMSTP::valueFromJs(value, "ranking", "");
    mSelectiveThinning.Ncompetitors = FMSTP::valueFromJs(value, "NCompetitors", "1.5");
}

void ActThinning::setupTending(QJSValue value)
{
    mTendingThinning.speciesProb = FMSTP::valueFromJs(value, "speciesSelectivity");
    mTendingThinning.intensity = FMSTP::valueFromJs(value, "intensity", "1", "Intensity required!").toNumber();

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
            custom.targetVariable != "basalarea" &&
            custom.targetVariable != "volume")
        throw IException(QString("setup of custom Activity: invalid targetVariable: %1").arg(custom.targetVariable));

    custom.targetRelative = FMSTP::boolValueFromJs(value, "targetRelative", true);
    custom.targetValue = FMSTP::valueFromJs(value, "targetValue", "30");

    QJSValue values = FMSTP::valueFromJs(value, "classes",  "", "setup custom acitvity");
    if (!values.isArray())
        throw IException("setup of custom activity: the 'classes' is not an array.");
    custom.classValues.clear();
    custom.cumClassPercentiles.clear();
    custom.cumSelectPercentiles.clear();

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

    // percentiles are calculated cumulatively, both the buckets (cumClassPercentiles)
    // and the probabilities (cumSelectPercentiles)
    double p_pct = 0.;
    double p_select = 0.;
    double f = 100. / custom.classValues.size();
    for (int i=0;i<custom.classValues.size();++i) {
        custom.cumClassPercentiles.push_back(qRound(p_pct));
        custom.cumSelectPercentiles.push_back(qRound(p_select));
        p_select +=custom.classValues[i];
        p_pct += f;
    }
    custom.cumClassPercentiles.push_back(100);
    custom.cumSelectPercentiles.push_back(100);
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

    if (target_value < 0. || remaining_stems < 0. || min_dbh < 0.)
        throw IException(QString("Thinning activity, error: target_value or min_dbh or remaining_stems < 0."));

    // load trees (considering the filter)
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

    // if not enough trees are on the stand, do nothing
    if (remaining_stems>0 && remaining_stems*stand->area()>=trees.trees().size())
        return false;

    if (trees.trees().size()==0)
        return false;

    // remove harvest flags.
    clearTreeMarks(&trees);

    // sort always by target variable (if it is stems, then simply by dbh)
    bool target_dbh = custom.targetVariable=="stems";
    double total_value;
    if (target_dbh) {
        trees.sort("dbh");
        total_value = trees.count();
    } else {
        trees.sort(custom.targetVariable);
        total_value = trees.sum(custom.targetVariable);
    }


    QVector<double> values = QVector<double>(custom.classValues.size(), 0.);
    QVector<double> tree_counts = QVector<double>(custom.classValues.size(), 0.);
    QVector<int> percentiles = QVector<int>(custom.cumClassPercentiles.size(), 0);

    // set percentiles to actual tree-indices
    // (e.g., if there are 200 trees, the 10th percentiles is tree #20)
    for (int i=0;i<percentiles.size();++i) {
        percentiles[i] = custom.cumClassPercentiles[i] * trees.trees().size() / 100;
    }

    double calc_target_value=0.;
    if (custom.targetRelative)
        calc_target_value = target_value * total_value / 100.;
    else
        calc_target_value = target_value * stand->area();

    if (!custom.relative) {
        throw IException("custom thinning: relative=false currently not supported.");
        // TODO: does not work now!!! redo!!
        // class values are given in absolute terms, e.g. 40m3/ha.
        // this needs to be translated to relative classes.
        // if the demand in a class cannot be met (e.g. planned removal of 40m3/ha, but there are only 20m3/ha in the class),
        // then the "miss" is distributed to the other classes (via the scaling below).
/*        for (int i=0;i<values.size();++i) {
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
        }*/
    }

    // *****************************************************************
    // ***************    Main loop
    // *****************************************************************

    bool finished = false;
    int removed_trees = 0;
    double removed_value = 0.;
    int no_tree_found = 0;
    bool target_value_reached=false;
    do {
        // look up a random number: it decides in which class to select a tree
        // the trick is that in cumSelectPercentiles the probability mass is not evenly
        // distributed
        int cls; // selected class
        double p = nrandom(0,100);
        for (cls=0;cls<values.size();++cls) {
            if (p < custom.cumSelectPercentiles[cls+1])
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
            no_tree_found += tree_idx == -1 ? 20 : 1; // empty list counts much more
            if (no_tree_found > 2000)
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
    QJSValue result = FMSTP::evaluateJS(mSelectiveThinning.speciesProb);
    bool selective_species = populateSpeciesSelectivity(result);

    markCropTrees(stand, selective_species);
    return true;
}

bool ActThinning::markCropTrees(FMStand *stand, bool selective_species)
{
    // tree list from current exeution context
    FMTreeList *treelist = ForestManagementEngine::instance()->scriptBridge()->treesObj();
    treelist->setStand(stand);
    treelist->loadAll();
    clearTreeMarks(treelist);

    // evaluate dynamic variables
    double selective_n = FMSTP::evaluateJS(mSelectiveThinning.N).toInt();
    if (selective_n == 0. || isnan(selective_n))
        throw IException(QString("Invalid value for 'N' in selective Thinning: '%1'").arg(selective_n));
    double selective_competitor = FMSTP::evaluateJS(mSelectiveThinning.Ncompetitors).toNumber();
    if (selective_competitor == 0. || isnan(selective_competitor))
        throw IException(QString("Invalid value for 'NCompetitors' in selective Thinning: '%1'").arg(selective_competitor));

    QString selective_ranking_expr = FMSTP::evaluateJS(mSelectiveThinning.rankingExpr).toString();
    if (selective_ranking_expr == "undefined")
        selective_ranking_expr.clear();

    // get the 2x2m grid for the current stand
    Grid<float> &grid = treelist->localStandGrid();
    // clear (except the out of "stand" pixels)
    for (float *p=grid.begin(); p!=grid.end(); ++p)
        if (*p > -1.f)
            *p = 0.f;

    int target_n = selective_n * stand->area();

    if (target_n>=treelist->trees().count())
        target_n = treelist->trees().count();

    qCDebug(abe) << "using user-defined number of competitors: " << selective_competitor;

    int max_target_n = qMax(target_n * 1.5, treelist->trees().count()/2.);
    if (max_target_n>=treelist->trees().count())
        max_target_n = treelist->trees().count();
    // we have 2500 px per ha (2m resolution)
    // if each tree dominates its Moore-neighborhood, 2500/9 = 267 trees are possible (/ha)
    // if *more* trees should be marked, some trees need to be on neighbor pixels:
    // pixels = 2500 / N; if 9 px are the Moore neighborhood, the "overprint" is N*9 / 2500.
    // N*9/2500 -1 = probability of having more than zero overlapping pixels
    //double overprint = (mSelectiveThinning.N * 9) / double(cPxPerHectare) - 1.;
    //double overprint = (mSelectiveThinning.N * 49) / double(cPxPerHectare) - 1.; // JM: Adjusted, since we have a 7x7 Kernle now instead of 3x3

    // rank the trees according to their ranking
    if (selective_ranking_expr.isEmpty()) {
        // order the list of trees according to tree height
        treelist->sort("-height");
    } else {
        // order the list of trees according to a user defined ranking expression
        treelist->sort(QString("-(%1)").arg(selective_ranking_expr));
        qCDebug(abe) << "using user-defined ranking for selective thinning: " << selective_ranking_expr;

    }

    qCDebug(abe) << "Target number of crop trees: " << target_n;


    // start with a part of N and 0 overlap
    int n_found = 0;
    int tests=0;
    int i=0;
    while (n_found < target_n/3 && i<target_n/2) {
        float f=testPixel(treelist->trees().at(i).first->position(), grid); ++tests;
        if (f==0.f) {
            // no neighbors: check species
            if (!selective_species ||
                drandom() < mSpeciesSelectivity[treelist->trees().at(i).first->species()]) {

                // found! Now mark as crop trees
                setPixel(treelist->trees().at(i).first->position(), grid);
                treelist->trees()[i].first->markCropTree(true);
                ++n_found;

            }
        }
        ++i;
    }



    qCDebug(abe) << "numbers found in first round: " << n_found;




    // continue with a higher probability
    for (int run=0;run<4;++run) {
        for (int i=0; i<max_target_n;++i) {
            if (treelist->trees().at(i).first->isMarkedAsCropTree())
                continue;

            float f=testPixel(treelist->trees().at(i).first->position(), grid); ++tests;

            if ((f==0.f) ||
                (f<=(0.0805*selective_n-2.4256)) ||                  // JM: define kernel thresholds here     scaled: max(0, 0.0805*selective_n-2.4256)
                (run==1 && f<=(0.1484*selective_n-5.4919)) ||        // JM: define kernel thresholds here     scaled: max(0, 0.1484*selective_n-5.4919)
                (run==2 && f<=(0.1679*selective_n-4.8988)) ||        // JM: define kernel thresholds here     scaled: max(0, 0.1679*selective_n-4.8988)
                ((run==3) && f<=(4*(0.1679*selective_n-4.8988)))) {  // JM: define kernel thresholds here     scaled: max(0, 0.0805*selective_n-2.4256) or 4*(0.1679*selective_n-4.8988)

                if (selective_species & !( drandom() < mSpeciesSelectivity[treelist->trees().at(i).first->species()]) )
                    continue;

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
    int target_competitors = std::round(selective_competitor * target_n);

    int max_target_n_competitor = qMax(target_competitors * 1.5, treelist->trees().count()/2.);
    if (max_target_n_competitor>=treelist->trees().count())
        max_target_n_competitor = treelist->trees().count();


    for (int run=0;run<3 && n_competitor<target_competitors;++run) {
        for (int i=0; i<max_target_n_competitor;++i) {
            Tree *tree = treelist->trees().at(i).first;
            if (tree->isMarkedAsCropTree() || tree->isMarkedAsCropCompetitor())
                continue;

            float f=testPixel(treelist->trees().at(i).first->position(), grid); ++tests;

            if ( (f>1.f) ||            // 12.f
                 (run==1 && f>0.5) ||     // 8.f
                 (run==2)) {    // 4.f
                tree->markCropCompetitor(true);
                n_competitor++;
                if (n_competitor >= target_competitors)
                    break;
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
    for (int i=-3;i<=3;++i){
        for (int j=-3;j<=3;++j){
            sum += grid.isIndexValid(x+i,y+j) ? grid.valueAtIndex(x+i, y+j) : 0;
        }
    }

    return sum;
}


QVector<QPair<QPoint, float> > rel_positions = { //calculated using 9 minus squared distance to center
    {{-3, -3}, 1/19.f},
    {{-3, -2}, 6/19.f},
    {{-3, -1}, 9/19.f},
    {{-3,  0}, 10/19.f},
    {{-3,  1}, 9/19.f},
    {{-3,  2}, 6/19.f},
    {{-3,  3}, 1/19.f},
    {{-2, -3}, 6/19.f},
    {{-2, -2}, 11/19.f},
    {{-2, -1}, 14/19.f},
    {{-2,  0}, 15/19.f},
    {{-2,  1}, 14/19.f},
    {{-2,  2}, 11/19.f},
    {{-2,  3}, 6/19.f},
    {{-1, -3}, 9/19.f},
    {{-1, -2}, 14/19.f},
    {{-1, -1}, 17/19.f},
    {{-1,  0}, 18/19.f},
    {{-1,  1}, 17/19.f},
    {{-1,  2}, 14/19.f},
    {{-1,  3}, 9/19.f},
    {{ 0, -3}, 10/19.f},
    {{ 0, -2}, 15/19.f},
    {{ 0, -1}, 18/19.f},
    {{ 0,  0}, 19/19.f},
    {{ 0,  1}, 18/19.f},
    {{ 0,  2}, 15/19.f},
    {{ 0,  3}, 10/19.f},
    {{ 1, -3}, 9/19.f},
    {{ 1, -2}, 14/19.f},
    {{ 1, -1}, 17/19.f},
    {{ 1,  0}, 18/19.f},
    {{ 1,  1}, 17/19.f},
    {{ 1,  2}, 14/19.f},
    {{ 1,  3}, 9/19.f},
    {{ 2, -3}, 6/19.f},
    {{ 2, -2}, 11/19.f},
    {{ 2, -1}, 14/19.f},
    {{ 2,  0}, 15/19.f},
    {{ 2,  1}, 14/19.f},
    {{ 2,  2}, 11/19.f},
    {{ 2,  3}, 6/19.f},
    {{ 3, -3}, 1/19.f},
    {{ 3, -2}, 6/19.f},
    {{ 3, -1}, 9/19.f},
    {{ 3,  0}, 10/19.f},
    {{ 3,  1}, 9/19.f},
    {{ 3,  2}, 6/19.f},
    {{ 3,  3}, 1/19.f}
};

void ActThinning::setPixel(const QPointF &pos, Grid<float> &grid)
{
    // check Moore neighborhood
    QPoint center_point(grid.indexAt(pos).x(),
                        grid.indexAt(pos).y());

    for (auto &p : rel_positions) {
        QPoint pt = center_point + p.first;
        if (grid.isIndexValid(pt))
            grid.valueAtIndex(pt) += p.second;
    }

}

bool ActThinning::evaluateTending(FMStand *stand)
{
    // species probabilities
    QJSValue result = FMSTP::evaluateJS(mTendingThinning.speciesProb);
    populateSpeciesSelectivity(result, 0.);

    runTending(stand);
    return true;

}

/// this is a 32 bit structure (same size as float)
/// to store structured information for the tending process
/// hacky as hell, but I dont want to create a new grid just
/// for this purpose.
struct STendingIndex {
    std::int8_t flag; // 0: unused, 1: a tree, 2: a sapling, 3: do not process
    std::uint8_t selectivity_byte; // 0..255: higher = more selectivity
    std::uint16_t index; // index to tree list or sapling array
    void set(bool is_tree, double dbl_selectivity, int index) {
        flag = is_tree ? 1 : 2;
        selectivity_byte = 255 * limit(dbl_selectivity, 0., 1.);
        this->index = index;
    }
    void setSelectivity(double dbl_sel) { selectivity_byte = 255 * limit(dbl_sel, 0., 1.); }
    double selectivity() { return selectivity_byte / 255.; }
    void lock() { flag = 3; }
    bool isLocked() { return flag == 3; }
    bool isEmpty() { return flag == 0; }
    bool isTree() { return flag == 1; }
    bool isSapling() { return flag == 2; }
    bool isAffected() { return flag < 0; }
};

bool ActThinning::runTending(FMStand* stand)
{
    // tree list from current exeution context, and create a new sapling list
    FMTreeList *treelist = ForestManagementEngine::instance()->scriptBridge()->treesObj();
    FMSaplingList saplinglist;
    saplinglist.loadFromStand(stand->id());

    treelist->setStand(stand);
    treelist->loadAll();
    clearTreeMarks(treelist);

    if (treelist->count() > 65535)
        throw IException("Tending operation: the number of trees on the stand is too high (>2^16). This is awkward and due to a implementation detail of the tending operation. Use smaller stands?");

    // get the 2x2m grid for the current stand
    Grid<float> &grid = treelist->localStandGrid();
    // clear (except the out of "stand" pixels)
    for (float *p=grid.begin(); p!=grid.end(); ++p)
        if (*p > -1.f)
            *p = 0.f;

    // pass 1: mark all positions where treees with selectivity > 0.5 are located
    // save max. one tree (with the highest selectivity) per 2m cell
    int trees_to_tend = 0;
    for (int i=0;i< treelist->trees().size(); ++i) {
        // get the grid cell occupied by the tree
        Tree *t = treelist->trees().at(i).first;
        QPointF tree_location = t->position();
        QPoint lc = grid.indexAt(tree_location);
        double selectivity = mSpeciesSelectivity[t->species()];
        STendingIndex *ti = reinterpret_cast<STendingIndex *>(&grid[lc]);
        if (ti->isLocked())
            continue;
        if (t->height() >= 10.) {
            // skip tall trees
            ti->lock();
            continue;
        }
        if (t->height() < 10. && selectivity> 0.5) {
            // relative height between 4m and 10m:
            double rel_height = std::max((t->height() - 4.) / 6., 0.);
            double eff_selectivity = pow(selectivity, 1. - rel_height);
            // select the focal tree if its effective selectivity is higher
            if (ti->flag == 0 || eff_selectivity > ti->selectivity()) {
                ti->set(true, eff_selectivity, i);
                ++trees_to_tend;
            }
        }
    }
    // pass 1.5: we used the "effective selectivity" in the grid, but later on we continue
    // with the species selectivity. So we need to reset the values in the grid to
    // the species selectivity:
    // We skip for now, as *higher* values (effective selectivity) make it less
    // probable that a tree is removed to promote saplings
    // for (float *g = grid.begin(); g!=grid.end(); ++g) {
    //     if (*g != -1.) {
    //         STendingIndex *ti = reinterpret_cast<STendingIndex *>(g);
    //         if (ti->isTree()) {
    //             ti->setSelectivity( mSpeciesSelectivity[treelist->trees()[ti->index].first->species()] );
    //         }
    //     }
    // }

    // pass 2: mark positions with favorable saplings
    // for saplings the selectivity is calculated as a function that reaches species selectivity close to 4m
    // larger saplings (with slightly less selectivity) are therefore favored more strongly
    float *p = grid.begin();
    int saps_to_tend = 0;
    SaplingCellRunner scr(stand->id(), GlobalSettings::instance()->model()->standGrid());
    while (SaplingCell *sc = scr.next()) {
        while (p!=grid.end() && *p == -1.f) ++p; // skip pixels outside of the stand
        if (p == grid.end()) {
            //QString fileName = GlobalSettings::instance()->path("debuggrid.csv");
            //gridToFile<float>(grid, fileName);
            throw IException("activity tending: grid reached end");
        }
        // look only at pixels that have not yet a favorable tree
        if (sc && *p == 0.f){
            if (scr.currentCoord() != grid.cellCenterPoint(p)) {
                qDebug() << "problem: " << scr.currentCoord() << grid.cellCenterPoint(p);
            }
            double max_sel = 0.0;
            int which_max = -1;
            for (int i=0;i<NSAPCELLS;++i) {
                if (sc->saplings[i].is_occupied()) {

                    double selectivity = mSpeciesSelectivity[sc->saplings[i].resourceUnitSpecies(scr.ru())->species()];
                    double eff_selectivity = selectivity / (1. + exp(-20*(sc->saplings[i].height / 4. - (1.-selectivity))));

                    if (selectivity > 0.5 && eff_selectivity > max_sel) {
                        max_sel = eff_selectivity;
                        which_max = i;
                    }

                }
            }
            if (which_max > -1) {
                // sapling to save found!
                STendingIndex *ti = reinterpret_cast<STendingIndex *>(p);
                double selectivity = mSpeciesSelectivity[sc->saplings[which_max].resourceUnitSpecies(scr.ru())->species()];
                ti->set(false, selectivity, which_max);
                ++saps_to_tend;
            }
        }
        ++p;
    }

    // pass 3: now we have marked the grid positions that have either trees
    // or saplings to be favored.
    // In this pass we mark 2m cells thare are in the vicinity of a tending target
    for (float *g = grid.begin(); g!=grid.end(); ++g) {
        if (*g != -1.) {
           STendingIndex *ti = reinterpret_cast<STendingIndex *>(g);
            if (ti->isTree() || ti->isSapling()) {
                // we have a tree or sapling to protect here
                QPoint p = grid.indexOf(g);
                // look at the 5x5 neighborhood (~5m radius around)
                for (int i=-2;i<=2;++i){
                    for (int j=-2;j<=2;++j){
                        QPoint pt(p.x()+i,p.y()+j);
                        if ((i!=0 && j!=0) && grid.isIndexValid(pt) && grid[pt] != -1.f) {
                            STendingIndex *ttest = reinterpret_cast<STendingIndex *>(&grid[pt]);
                            if (ttest->flag == 0) {
                                // pixels closer to the source pixel will be stronger impacted
                                if (abs(i)<=1 && abs(j) <=1) {
                                    ttest->flag = -1;
                                    ttest->setSelectivity( ti->selectivity() * 0.8 );
                                } else {
                                    ttest->flag = -2;
                                    ttest->setSelectivity( ti->selectivity() * 0.5 );
                                }
                            }
                        }
                    }
                }
            }
        }
    }


    // pass 4:
    // time to do the actual tending!
    const double impact_strength = mTendingThinning.intensity;
    int tree_marked = 0;
    for (int i=0;i< treelist->trees().size(); ++i) {
        // get the grid cell occupied by the tree
        Tree *t = treelist->trees().at(i).first;
        QPointF tree_location = t->position();
        QPoint lc = grid.indexAt(tree_location);
        double selectivity = mSpeciesSelectivity[t->species()];
        STendingIndex *ti = reinterpret_cast<STendingIndex *>(&grid[lc]);
        if (ti->isEmpty() || ti->isLocked() || t->height() > 10.)
            continue; // nothing to do here (in any case, we do not cut trees above 10m)
        if (ti->isTree()) {
            if (ti->index == i) {
                continue;
            } else {// this is a tree on a pixel where *another* tree is to be favored.
                t->markForCut(true);
                ++tree_marked;
            }
        } else {
            // calculate how much we want to favor this spot: higher (closer to 1): higher likelyhood to clear
            double favor =  (1. - selectivity) * ti->selectivity();
            // we translate the favor rating (0..1)
            double p_cut = pow(favor,  (1. / impact_strength));
            if (drandom() < p_cut) {
                t->markForCut(true);
                ++tree_marked;
            }
        }
    }

    // now the saplings:
    p = grid.begin();
    int sap_removed = 0;
    SaplingCellRunner scr_impact(stand->id(), GlobalSettings::instance()->model()->standGrid());
    while (SaplingCell *sc = scr_impact.next()) {
        while (p!=grid.end() && *p == -1.f) ++p; // skip pixels outside of the stand
        if (p == grid.end())
            throw IException("activity tending: grid reached end");


        STendingIndex *ti = reinterpret_cast<STendingIndex *>(p);

        if (ti->isSapling()) {
            // a sapling to be favored: remove all other saplings from the cell
            for (int i=0;i<NSAPCELLS;++i) {
                if (sc->saplings[i].is_occupied()) {
                    if (i != ti->index) {
                        sc->saplings[i].clear();
                        ++sap_removed;
                    }
                }
            }
            sc->checkState();
        }
        if (ti->isAffected()) {
            // neighborhood of favored cells.
            for (int i=0;i<NSAPCELLS;++i) {
                if (sc->saplings[i].is_occupied()) {
                    double selectivity = mSpeciesSelectivity[sc->saplings[i].resourceUnitSpecies(scr.ru())->species()];
                    // calculate how much we want to favor this spot: higher (closer to 1): higher likelyhood to clear
                    double favor =  (1. - selectivity) * ti->selectivity();
                    // we translate the favor rating (0..1)
                    double p_cut = pow(favor,  (1. / impact_strength));
                    if (drandom() < p_cut) {
                        sc->saplings[i].clear();
                        ++sap_removed;
                    }

                }
            }
            sc->checkState();
        }

        ++p;
    }

    qCDebug(abe) << "Tending. Found " <<  trees_to_tend << "trees and" << saps_to_tend << "saplings to promote. Removed" << tree_marked << " trees, and" << sap_removed << "saplings from stand" << stand->id();

    return true;
}


bool ActThinning::populateSpeciesSelectivity(QJSValue value, double default_value)
{
    // fill with all active species in the simulation (this list does not change)
    if (mSpeciesSelectivity.isEmpty()) {
        foreach(const Species *s, GlobalSettings::instance()->model()->speciesSet()->activeSpecies())
            mSpeciesSelectivity[s] = default_value;
    }
    if (value.isUndefined() || value.isNull())
        return false;

    // fetch from javascript object
    double rest_val = value.property(QStringLiteral("rest")).isNumber() ?  value.property(QStringLiteral("rest")).toNumber() : default_value;
    foreach(const Species *s, mSpeciesSelectivity.keys()) {
        mSpeciesSelectivity[s] = limit( value.property(s->id()).isNumber() ? value.property(s->id()).toNumber() : rest_val, 0., 1.);
    }
    return true;
}


} // namespace
