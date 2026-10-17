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
#include "biteimpact.h"
#include "biteengine.h"
#include "fmtreelist.h"
#include "fmsaplinglist.h"

namespace BITE {

BiteImpact::BiteImpact(QJSValue obj): BiteItem(obj)
{

}

void BiteImpact::setup(BiteAgent *parent_agent)
{
    BiteItem::setup(parent_agent);
    try {

        checkProperties(mObj);

        QJSValue filter =  BiteEngine::valueFromJs(mObj, "hostTrees", "");
        if (!filter.isUndefined())
            mHostTreeFilter = filter.toString();
        mSimulate = BiteEngine::valueFromJs(mObj, "simulate").toBool(); // default false
        mVerbose = BiteEngine::valueFromJs(mObj, "verbose").toBool();

        QJSValue impacts = BiteEngine::valueFromJs(mObj, "impact", "", "The 'impact' is required!");
        if (!impacts.isArray())
            throw IException("The 'impact' is no array.");
        QJSValueIterator it(impacts);
        int index=0;
        while (it.hasNext()) {
            it.next();
            if (it.name()==QStringLiteral("length"))
                continue;

            BiteImpactItem *bi = new BiteImpactItem();
            bi->setup(it.value(), ++index, parent_agent);
            mItems.push_back(bi);
        }

        filter = BiteEngine::valueFromJs(mObj, "impactFilter");
        if (!filter.isUndefined())
            mImpactFilter.setup(filter, DynamicExpression::CellWrap, parent_agent);



        mEvents.setup(mObj, QStringList() << "onImpact" << "onAfterImpact" << "onExit" , agent());



    } catch (const IException &e) {
        QString error = QString("An error occured in the setup of BiteImpact item '%1': %2").arg(name()).arg(e.message());
        qCInfo(biteSetup) << error;
        BiteEngine::instance()->error(error);

    }

}

QString BiteImpact::info()
{
    QString res = QString("Type: BiteImpact\nDesc: %1").arg(description());
    return res;

}

void BiteImpact::afterSetup()
{
    BiteWrapper bitewrap(agent()->wrapper());

}

void BiteImpact::runCell(BiteCell *cell, ABE::FMTreeList *treelist, ABE::FMSaplingList *saplist)
{
    if (!cell->isActive())
        return;

    bool filter = mImpactFilter.evaluateBool(cell);
    if (verbose())
        qCDebug(bite) << "Impact: result of impactFilter:" << filter;
    if (!filter)
        return;

    // filter host trees
    cell->checkTreesLoaded(treelist); // load trees (if this has not happened before)
    BACellStat *stat = agent()->cellStat(cell);
    if (!mHostTreeFilter.isEmpty()) {
        int before = treelist->count();
        int after = treelist->filter(mHostTreeFilter);
        if (verbose())
            qCDebug(bite) << "Impact: filter trees with" << mHostTreeFilter << "N before:" << before << ", after: " << after;
        // cell level stats
        if (stat) {
            stat->nHostTrees = after;
        }

    } else {
        // no explicit filter, number of host trees is the number of all trees on the cell
        if (stat) {
            stat->nHostTrees = treelist->count();
        }
    }
    if (stat && cell->areSaplingsLoaded())
        stat->nHostSaplings = saplist->saplings().size();

    bool had_impact = false;
    for (int i=0;i<mItems.length();++i) {
        if (mVerbose)
            qCDebug(bite) << "run impact item" << i+1 << ":";
        had_impact |= runImpact(mItems[i], cell, treelist, saplist);
    }


    int killed = mEvents.run("onImpact", cell).toInt();
    if (verbose())
        qCDebug(bite) << "Impact: called 'onImpact', #trees killed (=return value): " << killed;

    if (killed>0 || had_impact) {
        agent()->notifyItems(cell, BiteCell::CellImpacted);
    }


}

QStringList BiteImpact::allowedProperties()
{
    QStringList l = BiteItem::allowedProperties();
    l << "impactFilter" << "hostTrees" << "impactTarget"  << "impactOrder" <<  "impactMode" << "verbose" << "simulate" << "impact";
    return l;

}

bool BiteImpact::runImpact(BiteImpact::BiteImpactItem *item, BiteCell *cell, ABE::FMTreeList *treelist, ABE::FMSaplingList *saplist)
{
    bool saplings = item->target==BiteImpactItem::Sapling || item->target==BiteImpactItem::Browsing;

    if (saplings) {
        return runImpactSaplings(item, cell, saplist);
    } else {
        return runImpactTrees(item, cell, treelist);
    }

}

bool BiteImpact::runImpactTrees(BiteImpact::BiteImpactItem *item, BiteCell *cell, ABE::FMTreeList *treelist)
{
    BiteImpactItem::ImpactTarget target = item->target;
    double total_biomass = 0.; // available biomass (in compartment)
    bool select_random = false;
    double random_fraction = 1.;
    double fraction_per_tree = 1.;
    int max_trees = std::numeric_limits<int>().max();
    double max_biomass = std::numeric_limits<double>().max();
    int n_trees=0; // number of living trees

    TreeWrapper tw;
    for (int i=0;i<treelist->count();++i) {
        Tree *t = treelist->trees()[i].first;
        if (!t->isDead()) {
            if (!item->treeFilter.isEmpty()) {
                tw.setTree(t);
                if (item->treeFilter.execute(nullptr, &tw)==0.)
                    continue;
            }
            ++n_trees;
            switch (target) {
            case BiteImpactItem::Foliage: total_biomass+=t->biomassFoliage(); break;
            case BiteImpactItem::Root: total_biomass+=t->biomassCoarseRoot() + t->biomassFineRoot(); break;
            default: break;
            }
        }
    }

    if (item->hasMaxTrees())
        max_trees = static_cast<int>(item->maxTrees.evaluate(cell));

    if (item->hasMaxBiomass())
        max_biomass = item->maxBiomass.evaluate(cell);

    if (item->hasFractionOfTrees()) {
        random_fraction = item->fractionOfTrees.evaluate(cell);
        if (random_fraction < 0. || random_fraction > 1.)
            throw IException(QString("BiteImpact: invalid 'fractionOfTrees': %1 in item %2").arg(random_fraction).arg(item->id));
        if (random_fraction<1.) {
            max_trees = std::min(max_trees, static_cast<int>(random_fraction * n_trees + drandom())); // calc. number of trees + rounding (and truncating to int)
            select_random = true;
        }

    }
    if (item->hasFractionPerTree()) {
        fraction_per_tree = item->fractionPerTree.evaluate(cell);
    }


    // sort trees according to the given order criterion
    if (!item->order.isEmpty()) {
        treelist->sort(item->order);
        select_random = false;
    }

    if (mVerbose) {
        QString details = QString("Impact %1 (#%2): Trees: %3, Biomass: %4, fractionOfTrees: %5. Affect: %6, pick random: %7, maxBiomass: %8")
                .arg(cell->info()).arg(item->id)
                .arg(n_trees).arg(total_biomass).arg(random_fraction).arg(std::min(max_trees, n_trees)).arg(select_random ? "true" : "false").arg(max_biomass);
        qCDebug(bite) << details;
    }
    if (max_trees==0) {
        if (mVerbose)
            qCDebug(bite) << "no trees are affected.";
        return false; // nothing to do!
    }

    // Main loop
    int n_affected = 0, n_killed=0;
    double removed_biomass = 0.;
    double killed_m3 = 0.;
    for (int i=0;i<treelist->count();++i) {
        Tree *t = treelist->trees()[i].first;
        if (t->isDead())
            continue;

        if (!item->treeFilter.isEmpty()) {
            tw.setTree(t);
            if (item->treeFilter.execute(nullptr, &tw)==0.)
                continue;
        }

        if (!select_random || drandom() < random_fraction) {
            // affect tree
            switch (target) {
            case BiteImpactItem::Foliage: {
                double remove_biomass = t->biomassFoliage() * fraction_per_tree;
                if (removed_biomass + remove_biomass > max_biomass) {
                    // cap with maximum biomass
                    fraction_per_tree = (max_biomass - removed_biomass + 0.01) / t->biomassFoliage(); // 0.01: avoid rounding problems
                }
                removed_biomass += t->biomassFoliage() * fraction_per_tree;
                if (!mSimulate) {
                    t->removeBiomassOfTree(fraction_per_tree, 0., 0.);
                    t->setAffectedBite();
                }
                break;
            }
            case BiteImpactItem::Root: {
                double fine_root_fraction = qMin(fraction_per_tree * item->fineRootMultiplier, 1.);
                double remove_biomass = t->biomassCoarseRoot() * fraction_per_tree + t->biomassFineRoot() * fine_root_fraction;
                if (removed_biomass + remove_biomass > max_biomass) {
                    // scale removal: removal = coarse_root * frac + fine_root * frac * finerootmult -> frac = removal_new / (coarse_root + finerootmult*fine_root)
                    fraction_per_tree = (max_biomass - removed_biomass) / (t->biomassCoarseRoot() + item->fineRootMultiplier*t->biomassFineRoot());
                    fine_root_fraction = qMin(fraction_per_tree * item->fineRootMultiplier, 1.);
                    remove_biomass = max_biomass;
                } else {
                    removed_biomass += remove_biomass;
                }
                if (!mSimulate) {
                    t->removeRootBiomass(fine_root_fraction, fraction_per_tree);
                    t->setAffectedBite();
                }
                break;
            }
            case BiteImpactItem::Tree: {
                // kill the tree
                killed_m3+=t->volume();
                ++n_killed;
                if (!mSimulate) {
                    t->setAffectedBite();
                    t->die();
                }
                break;
            }
            default: break;
            }

            ++n_affected;
            if (n_affected >= max_trees || removed_biomass >= max_biomass)
                break;

        }
    }
    if (mVerbose || agent()->verbose()) {
        QString details = QString("Impact %1 (#%2): #affected: %3, rem.biomass: %4, killed: %5, killed.vol: %6")
                .arg(cell->info()).arg(item->id)
                .arg(n_affected).arg(removed_biomass).arg(n_killed).arg(killed_m3);
        qCDebug(bite) << details;
    }


    //agent()->notifyItems(cell, BiteCell::CellImpacted);
    agent()->stats().treesKilled += n_killed;
    agent()->stats().m3Killed += killed_m3;
    agent()->stats().totalImpact += removed_biomass;

    // cell level stats
    BACellStat *stat = agent()->cellStat(cell);
    if (stat) {
        stat->m3Killed += killed_m3;
        stat->totalImpact += removed_biomass;
        stat->nKilled += n_killed;
    }

    return n_killed>0 || removed_biomass>0.;

}

bool BiteImpact::runImpactSaplings(BiteImpact::BiteImpactItem *item, BiteCell *cell, ABE::FMSaplingList *saplist)
{
    cell->checkSaplingsLoaded(saplist);
    bool select_random = false;
    double random_fraction = 1.;
    int max_trees = std::numeric_limits<int>().max();
    int n_saplings = 0;

    SaplingWrapper sw;
    QVector<QPair<SaplingTree*, SaplingCell*> >::iterator it;

    if (!item->treeFilter.isEmpty()) {
        // count all sapling cohorts:
        for (it = saplist->saplings().begin(); it!=saplist->saplings().end(); ++it) {
            sw.setSaplingTree(it->first, it->second->ru);
            if (item->treeFilter.execute(nullptr, &sw)==0.)
                continue;
            ++n_saplings;
        }
    } else {
        n_saplings = saplist->length();
    }

    if (n_saplings==0)
        return false; // nothing to do


    // number of tree cohorts
    if (item->hasMaxTrees())
        max_trees = static_cast<int>(item->maxTrees.evaluate(cell));


    if (item->hasFractionOfTrees()) {
        random_fraction = item->fractionOfTrees.evaluate(cell);
        if (random_fraction < 0. || random_fraction > 1.)
            throw IException(QString("BiteImpact: invalid 'fractionOfTrees': %1 in item %2").arg(random_fraction).arg(item->id));
        if (random_fraction<1.) {
            max_trees = std::min(max_trees, static_cast<int>(random_fraction * n_saplings + drandom())); // calc. number of saplings + rounding (and truncating to int)
            select_random = true;
        }

    }

    int n_affected = 0;
    for (it = saplist->saplings().begin(); it!=saplist->saplings().end(); ++it) {
        SaplingTree *stree = it->first;
        SaplingCell *sc = it->second;
        // filtering?
        if (!item->treeFilter.isEmpty()) {
            sw.setSaplingTree(stree, sc->ru);
            if (item->treeFilter.execute(nullptr, &sw)==0.)
                continue;
        }
        if (!select_random || drandom() < random_fraction) {
            if (item->target == BiteImpactItem::Browsing) {
                stree->set_browsed(true);
            } else {
                stree->clear();
                sc->checkState();
            }
            ++n_affected;
        }

    }

    if (mVerbose || agent()->verbose()) {
        QString details = QString("Impact %1 (#%2): #affected: %3 from %4 sapling cohorts")
                .arg(cell->info()).arg(item->id)
                .arg(n_affected).arg(n_saplings);
        qCDebug(bite) << details;
    }
    if (item->target == BiteImpactItem::Browsing)
        agent()->stats().saplingsImpact += n_affected;
    else
        agent()->stats().saplingsKilled += n_affected;


    // cell level stats
    BACellStat *stat = agent()->cellStat(cell);
    if (stat) {
        if (item->target == BiteImpactItem::Browsing)
            stat->saplingsImpact += n_affected;
        else
            stat->saplingsKilled += n_affected;
    }

    return n_affected>0;
}


void BiteImpact::BiteImpactItem::setup(QJSValue obj, int index, BiteAgent *parent_agent)
{
    id = index;
    // check allowed properties
    QStringList allowed = QStringList() << "target" << "fractionOfTrees" << "fractionPerTree" << "maxTrees" << "maxBiomass" << "order" << "treeFilter" << "fineRootFactor";
    if (obj.isObject()) {
        QJSValueIterator it(obj);
        while (it.hasNext()) {
            it.next();
            if (!it.name().startsWith("on") &&  !it.name().startsWith("user") && !allowed.contains(it.name())) {
                qCDebug(biteSetup) << it.name() << "is not a valid property for ImpactItem " << index << "! Allowed are: " << allowed;
            }
        }
    }

    // setup the properties
    QString target_str = BiteEngine::valueFromJs(obj, "target").toString();
    int idx = (QStringList() << "tree"<< "foliage"<<"roots"<<"sapling"<<"browsing").indexOf(target_str);
    if (idx<0)
        throw IException("Invalid target: " + target_str);
    target = static_cast<ImpactTarget>(idx);

    QJSValue filter = BiteEngine::valueFromJs(obj, "fractionOfTrees");
    if (!filter.isUndefined())
        fractionOfTrees.setup(filter, DynamicExpression::CellWrap, parent_agent);

    filter = BiteEngine::valueFromJs(obj, "fractionPerTree");
    if (!filter.isUndefined())
        fractionPerTree.setup(filter, DynamicExpression::CellWrap, parent_agent);

    filter = BiteEngine::valueFromJs(obj, "maxTrees");
    if (!filter.isUndefined())
        maxTrees.setup(filter, DynamicExpression::CellWrap, parent_agent);

    filter = BiteEngine::valueFromJs(obj, "maxBiomass");
    if (!filter.isUndefined())
        maxBiomass.setup(filter, DynamicExpression::CellWrap, parent_agent);

    filter = BiteEngine::valueFromJs(obj, "order");
    if (!filter.isUndefined())
        order = BiteEngine::valueFromJs(obj, "order").toString();

    filter = BiteEngine::valueFromJs(obj, "treeFilter");
    if (!filter.isUndefined())
        treeFilter.setExpression(BiteEngine::valueFromJs(obj, "treeFilter").toString());

    fineRootMultiplier = BiteEngine::valueFromJs(obj, "fineRootMultiplier", "1").toNumber();


}




} // end namespace
