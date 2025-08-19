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
#include "management.h"
#include "helper.h"
#include "model.h"
#include "resourceunit.h"
#include "tree.h"
#include "expressionwrapper.h"
#include "soil.h"
#include "species.h"
#include "seeddispersal.h"

//#include "climateconverter.h"
//#include "csvfile.h"
#include "scriptglobal.h"
#include "mapgrid.h"
//#include "modules.h"

#include <QJSEngine>
#include <QJSValue>

/** @class Management Management executes management routines.
  @ingroup core
  The actual iLand management is based on Javascript functions. This class provides
  the frame for executing the javascript as well as the functions that are called by scripts and
  that really do the work.
  See https://iland-model.org/iLand+scripting, https://iland-model.org/Object+Management for management Javascript API.
  */


// global output function
QString Management::executeScript(QString cmd)
{
    QString result = ScriptGlobal::executeScript(cmd);
    if (!ScriptGlobal::lastErrorMessage().isEmpty()) {
        Helper::msg(ScriptGlobal::lastErrorMessage());
    }

    return result;
}

Management::Management()
{
    // setup the scripting engine
    mEngine = GlobalSettings::instance()->scriptEngine();
    QJSValue objectValue = mEngine->newQObject(this);
    mEngine->globalObject().setProperty("management", objectValue);

    // default values for removal fractions
    // 100% of the stem, 0% of foliage and branches
    mRemoveFoliage = 0.;
    mRemoveBranch = 0.;
    mRemoveStem = 1.;

    mTree = new ScriptTree();
    mTreeValue = mEngine->newQObject(mTree);

}

Management::~Management()
{
}


int Management::remain(int number)
{
    if (logLevelDebug())
        qDebug() << "remain called (number): " << number;
    Model *m = GlobalSettings::instance()->model();
    AllTreeIterator at(m);
    QList<Tree*> trees;
    while (Tree *t=at.next())
        trees.push_back(t);
    int to_kill = trees.count() - number;
    if (logLevelDebug())
        qDebug() << trees.count() << " standing, targetsize" << number << ", hence " << to_kill << "trees to remove";
    for (int i=0;i<to_kill;i++) {
        int index = irandom(0, trees.count());
        trees[index]->remove();
        trees.removeAt(index);
    }
    mRemoved += to_kill;
    return to_kill;
}


int Management::killAll()
{
    int c = mTrees.count();
    for (int i=0;i<mTrees.count();i++)
        mTrees[i].first->remove();
    mTrees.clear();
    return c;
}

int Management::disturbanceKill(double stem_to_soil_fraction, double  stem_to_snag_fraction,
                                double branch_to_soil_fraction, double branch_to_snag_fraction,
                                QString agent)
{
    bool is_fire = agent==QStringLiteral("fire");
    bool is_bb = agent==QStringLiteral("barkbeetle");
    bool is_wind = agent==QStringLiteral("wind");
    bool is_cutdown = agent==QStringLiteral("cutdown");

    int c = mTrees.count();
    for (int i=0;i<mTrees.count();i++) {
        Tree *t= mTrees[i].first;
        if (is_bb) t->setDeathReasonBarkBeetle();
        if (is_wind) t->setDeathReasonWind();
        if (is_fire) t->setDeathReasonFire();
        if (is_cutdown) t->setDeathCutdown();

        t->removeDisturbance(stem_to_soil_fraction, stem_to_snag_fraction, branch_to_soil_fraction, branch_to_snag_fraction, 1.);
        if (is_fire) {
            if (t->species()->seedDispersal() && t->species()->isTreeSerotinous(t->age()) ) {
                //SeedDispersal *sd = t->species()->seedDispersal();
                t->species()->seedDispersal()->seedProductionSerotiny(t);
            }
        }
    }
    mTrees.clear();
    return c;
}

int Management::kill(QString filter, double fraction)
{
   return remove_trees(filter, fraction, false);
}
int Management::manage(QString filter, double fraction)
{
    return remove_trees(filter, fraction, true);
}

void Management::cutAndDrop()
{
    //int c = mTrees.count();
    for (int i=0;i<mTrees.count();i++) {
        mTrees[i].first->setDeathCutdown(); // set flag that tree is cut down
        mTrees[i].first->die();
    }
    mTrees.clear();
}

int Management::remove_percentiles(int pctfrom, int pctto, int number, bool management)
{
    if (mTrees.isEmpty())
        return 0;
    int index_from = limit(int(pctfrom/100. * mTrees.count()), 0, mTrees.count());
    int index_to = limit(int(pctto/100. * mTrees.count()), 0, mTrees.count());
    if (index_from>=index_to)
        return 0;
    if (logLevelDebug())
        qDebug() << "attempting to remove" << number << "trees between indices" << index_from << "and" << index_to;
    int i;
    int count = number;
    if (index_to-index_from <= number)  {
        // kill all
        if (management) {
            // management
            for (i=index_from; i<index_to; i++)
                mTrees.at(i).first->remove(removeFoliage(), removeBranch(), removeStem());
        } else {
            // just kill...
            for (i=index_from; i<index_to; i++)
                mTrees.at(i).first->remove();
        }
        count = index_to - index_from;
    } else {
        // kill randomly the provided number
        int cancel = 1000;
        while(number>=0) {
            int rnd_index = irandom(index_from, index_to);
            if (mTrees[rnd_index].first->isDead()) {
                if (--cancel<0) {
                    qDebug() << "Management::kill: canceling search." << number << "trees left.";
                    count-=number; // not all trees were killed
                    break;
                }
                continue;
            }
            cancel = 1000;
            number--;
            if (management) {
                mTrees[rnd_index].first->remove( removeFoliage(), removeBranch(), removeStem() );
            } else {
                mTrees[rnd_index].first->remove();
            }
        }
    }
    if (logLevelDebug())
        qDebug() << count << "removed.";
    // clean up the tree list...
    for (int i=mTrees.count()-1; i>=0; --i) {
        if (mTrees[i].first->isDead())
            mTrees.removeAt(i);
    }
    return count; // killed or manages
}

/** remove trees from a list and reduce the list.

  */
int Management::remove_trees(QString expression, double fraction, bool management)
{
    TreeWrapper tw;
    Expression expr(expression,&tw);
    expr.enableIncSum();
    int n = 0;
    QPair<Tree*, double> empty_tree(nullptr,0.);
    QList<QPair<Tree*, double> >::iterator tp=mTrees.begin();
    try {
        for (;tp!=mTrees.end();++tp) {
            tw.setTree(tp->first);
            // if expression evaluates to true and if random number below threshold...
            if (expr.calculate(tw) && drandom() <=fraction) {
                // remove from system
                if (management)
                    tp->first->remove(removeFoliage(), removeBranch(), removeStem()); // management with removal fractions
                else
                    tp->first->remove(); // kill

                // remove from tree list
                *tp = empty_tree;
                n++;
            }
        }
        mTrees.removeAll(empty_tree);
    } catch(const IException &e) {
        ScriptGlobal::throwError(e.message());
    }
    return n;
}

// calculate aggregates for all trees in the internal list
double Management::aggregate_function(QString expression, QString filter, QString type)
{
    QList<QPair<Tree*, double> >::iterator tp=mTrees.begin();
    TreeWrapper tw;
    Expression expr(expression,&tw);

    double sum = 0.;
    int n=0;
    try {

        if (filter.isEmpty()) {
            // without filtering
            while (tp!=mTrees.end()) {
                tw.setTree(tp->first);
                sum += expr.calculate();
                ++n;
                ++tp;
            }
        } else {
            // with filtering
            Expression filter_expr(filter,&tw);
            filter_expr.enableIncSum();
            while (tp!=mTrees.end()) {
                tw.setTree(tp->first);
                if (filter_expr.calculate()) {
                    sum += expr.calculate();
                    ++n;
                }
                ++tp;
            }
        }

    } catch(const IException &e) {
         ScriptGlobal::throwError(e.message());
    }
    if (type=="sum")
        return sum;
    if (type=="mean")
        return n>0?sum/double(n):0.;
    return 0.;
}


// from the range percentile range pctfrom to pctto (each 1..100)
int Management::killPct(int pctfrom, int pctto, int number)
{
    return remove_percentiles(pctfrom, pctto, number, false);
}

// from the range percentile range pctfrom to pctto (each 1..100)
int Management::managePct(int pctfrom, int pctto, int number)
{
    return remove_percentiles(pctfrom, pctto, number, true);
}

int Management::manageAll()
{
    int c = mTrees.count();
    for (int i=0;i<mTrees.count();i++)
        mTrees[i].first->remove(removeFoliage(),
                                removeBranch(),
                                removeStem()); // remove with current removal fractions
    mTrees.clear();
    return c;
}



void Management::run()
{
    ExprExceptionAsScriptError no_expression; // turn expression errors to JS exceptions

    mTrees.clear();
    mRemoved=0;
    qDebug() << "Management::run() called";
    QJSValue mgmt = mEngine->globalObject().property("manage");
    int year = GlobalSettings::instance()->currentYear();
    //mgmt.call(QJSValue(), QScriptValueList()<<year);
    QJSValue result = mgmt.call(QJSValueList() << year);
    if (result.isError())
        qDebug() << "Script Error occured: " << result.toString();//  << "\n" << mEngine->uncaughtExceptionBacktrace();

}

void Management::loadScript(const QString &fileName)
{
    mScriptFile = fileName;
    ScriptGlobal::loadScript(fileName);
}

QJSValue Management::tree(int index)
{
    if (index<0 || index>=count())
        mTree->clear();
    else
        mTree->setTree( mTrees[index].first );
    return mTreeValue;
}

QJSValue Management::treeObject(int index)
{
    Tree *tree = nullptr;
    if (index>=0 && index<count())
        tree = mTrees[index].first;

    ScriptTree *tobj = new ScriptTree();
    tobj->setTree(tree);
    QJSValue val = mEngine->newQObject(tobj);
    return val;
}

int Management::filterIdList(QVariantList idList)
{
    QVector<int> ids;
    foreach(const QVariant &v, idList)
        if (!v.isNull())
            ids << v.toInt();
//    QHash<int, int> ids;
//    foreach(const QVariant &v, idList)
//        ids[v.toInt()] = 1;

    QList<QPair<Tree*, double> >::iterator tp=mTrees.begin();
    while (tp!=mTrees.end()) {
        if (!ids.contains(tp->first->id()) )
            tp = mTrees.erase(tp);
        else
            ++tp;
    }
    if (logLevelDebug())
        qDebug() << "Management::filter by id-list:" << mTrees.count();
    return mTrees.count();
}

int Management::filter(QString filter)
{
    TreeWrapper tw;
    Expression expr(filter,&tw);
    expr.enableIncSum();
    int n_before = mTrees.count();
    QPair<Tree*, double> empty_tree(nullptr,0.);
    try {
        for (QList< QPair<Tree*, double> >::iterator it=mTrees.begin(); it!=mTrees.end(); ++it) {

            tw.setTree(it->first); // iterate

            double value = expr.calculate(tw);
            // keep if expression returns true (1)
            bool keep = value==1.;
            // if value is >0 (i.e. not "false"), then draw a random number
            if (!keep && value>0.)
                keep = drandom() < value;

            if (!keep)
                *it = empty_tree;

        }
        mTrees.removeAll(empty_tree);
    } catch(const IException &e) {
        ScriptGlobal::throwError(e.message());
    }

    if (logLevelDebug())
        qDebug() << "filtering with" << filter << "N=" << n_before << "/" << mTrees.count()  << "trees (before/after filtering).";
    return mTrees.count();
}

int Management::loadResourceUnit(int ruindex)
{
    Model *m = GlobalSettings::instance()->model();
    ResourceUnit *ru = m->ru(ruindex);
    if (!ru)
        return -1;
    mTrees.clear();
    for (int i=0;i<ru->trees().count();i++)
        if (!ru->tree(i)->isDead())
            mTrees.push_back(QPair<Tree*,double>(ru->tree(i), 0.));
    return mTrees.count();
}

int Management::load(QString filter)
{
    TreeWrapper tw;
    Model *m = GlobalSettings::instance()->model();
    mTrees.clear();
    AllTreeIterator at(m);
    if (filter.isEmpty()) {
        while (Tree *t=at.nextLiving())
            if (!t->isDead())
                mTrees.push_back(QPair<Tree*, double>(t, 0.));
    } else {
        Expression expr(filter,&tw);
        expr.enableIncSum();
        if (logLevelDebug())
            qDebug() << "filtering with" << filter;
        while (Tree *t=at.nextLiving()) {
            tw.setTree(t);
            if (!t->isDead() && expr.execute())
                mTrees.push_back(QPair<Tree*, double>(t, 0.));
        }
    }
    return mTrees.count();
}

/**
*/
void Management::loadFromTreeList(QList<Tree*>tree_list, bool do_append)
{
    if (!do_append)
        mTrees.clear();
    for (int i=0;i<tree_list.count();++i)
        mTrees.append(QPair<Tree*, double>(tree_list[i], 0.));
}

// loadFromMap: script access
int Management::loadFromMap(MapGridWrapper *wrap, int key, bool do_append)
{
    if (!wrap) {
         ScriptGlobal::throwError("loadFromMap called with invalid map object!");
        return 0;
    }
    loadFromMap(wrap->map(), key, do_append);
    return count();
}

void Management::killSaplings(MapGridWrapper *wrap, int key, QString filter)
{

    QRectF box = wrap->map()->boundingBox(key);
    GridRunner<float> runner(GlobalSettings::instance()->model()->grid(), box);
    ResourceUnit *ru;

    SaplingWrapper sw;
    Expression expr(filter.isEmpty() ? "true" : filter, &sw);

    int nsap_removed=0;
    while (runner.next()) {
        if (wrap->map()->standIDFromLIFCoord(runner.currentIndex()) == key) {
            SaplingCell *sc=GlobalSettings::instance()->model()->saplings()->cell(runner.currentIndex(),true, &ru);
            if (sc) {
                if (filter.isEmpty()) {
                    // clear all saplings on the cell
                    GlobalSettings::instance()->model()->saplings()->clearSaplings(sc,ru,false, true);

                } else {
                    for (int i=0;i<NSAPCELLS;++i) {
                        if (sc->saplings[i].is_occupied()) {
                            sw.setSaplingTree(&sc->saplings[i], sc->ru);
                            if (expr.executeBool()) {
                                sc->saplings[i].clear();
                                nsap_removed++;
                            }
                        }
                    }
                    sc->checkState();
                }
            }
        }
    }
}

void Management::killSaplingsResourceUnit(int ruindex)
{
    ResourceUnit *ru = GlobalSettings::instance()->model()->ru(ruindex);
    if (ru) {
        GridRunner<float> runner(GlobalSettings::instance()->model()->grid(), ru->boundingBox());
        while (runner.next()) {
            SaplingCell *sc=GlobalSettings::instance()->model()->saplings()->cell(runner.currentIndex(),true);
            if (sc)
                GlobalSettings::instance()->model()->saplings()->clearSaplings(sc,ru,false, true);
        }
    }
}

/// specify removal fractions
/// @param SWDFrac 0: no change, 1: remove all of standing woody debris
/// @param DWDfrac 0: no change, 1: remove all of downed woody debris
/// @param litterFrac 0: no change, 1: remove all of soil litter
/// @param soilFrac 0: no change, 1: remove all of soil organic matter
void Management::removeSoilCarbon(MapGridWrapper *wrap, int key, double SWDfrac, double DWDfrac, double litterFrac, double soilFrac)
{
    if (!(SWDfrac>=0. && SWDfrac<=1. && DWDfrac>=0. && DWDfrac<=1. && soilFrac>=0. && soilFrac<=1. && litterFrac>=0. && litterFrac<=1.)) {
         ScriptGlobal::throwError(QString("removeSoilCarbon called with invalid parameters!!\nArgs: ---"));
        return;
    }
    if (!wrap || !wrap->map())
        return;

    //QList<QPair<ResourceUnit*, double> > ru_areas = wrap->map()->resourceUnitAreas(key);

    auto it_pair = wrap->map()->resourceUnitAreasIterator(key);
    auto it = it_pair.first; // the iterator
    auto itend = it_pair.second; // the end
    double total_area = 0.;
    while (it!=itend && it.key() == key) {
        ResourceUnit *ru = it->first;
        double area_factor = it->second; // 0..1
        total_area += area_factor;
        // swd
        if (SWDfrac>0. && ru->snag())
            ru->snag()->removeCarbon(SWDfrac*area_factor);
        // soil pools
        if (ru->soil())
            ru->soil()->disturbance(DWDfrac*area_factor, litterFrac*area_factor, soilFrac*area_factor);
        // qDebug() << ru->index() << area_factor;

        ++it;
    }


    if (logLevelDebug())
        qDebug() << "total area" << total_area << "of" << wrap->map()->area(key);
}

/** slash snags (SWD and otherWood-Pools) of polygon \p key on the map \p wrap.
  The factor is scaled to the overlapping area of \p key on the resource unit.
  @param wrap MapGrid to use together with \p key
  @param key ID of the polygon.
  @param slash_fraction 0: no change, 1: 100%
   */
void Management::slashSnags(MapGridWrapper *wrap, int key, double slash_fraction)
{
    if (slash_fraction<0 || slash_fraction>1) {
         ScriptGlobal::throwError(QString("slashSnags called with invalid parameters!!\nArgs: ...."));
        return;
    }
    if (!wrap || !wrap->map())
        return;
    auto it_pair = wrap->map()->resourceUnitAreasIterator(key);
    auto it = it_pair.first; // the iterator
    auto itend = it_pair.second; // the end

    double total_area = 0.;
    while (it!=itend && it.key() == key) {
        ResourceUnit *ru = it->first;
        double area_factor = it->second; // 0..1
        total_area += area_factor;
        ru->snag()->management(slash_fraction * area_factor);
        ++it;
    }
    if (logLevelDebug())
        qDebug() << "total area" << total_area << "of" << wrap->map()->area(key);

}

/** loadFromMap selects trees located on pixels with value 'key' within the grid 'map_grid'.
*/
void Management::loadFromMap(const MapGrid *map_grid, int key, bool do_append)
{
    if (!map_grid) {
        qDebug() << "invalid parameter for Management::loadFromMap: Map expected!";
        return;
    }
    if (map_grid->isValid()) {
        QList<Tree*> tree_list = map_grid->trees(key);
        loadFromTreeList( tree_list, do_append );
    } else {
        qDebug() << "Management::loadFromMap: grid is not valid - no trees loaded";
    }

}

bool treePairValue(const QPair<Tree*, double> &p1, const QPair<Tree*, double> &p2)
{
    return p1.second < p2.second;
}

void Management::sort(QString statement)
{
    TreeWrapper tw;
    Expression sorter(statement, &tw);
    // fill the "value" part of the tree storage with a value for each tree
    for (int i=0;i<mTrees.count(); ++i) {
        tw.setTree(mTrees.at(i).first);
        mTrees[i].second = sorter.execute();
   }
   // now sort the list....
   std::sort(mTrees.begin(), mTrees.end(), treePairValue);
}

double Management::percentile(int pct)
{
    if (mTrees.count()==0)
        return -1.;
    int idx = int( (pct/100.) * mTrees.count());
    if (idx>=0 && idx<mTrees.count())
        return mTrees.at(idx).second;
    else
        return -1;
}

/// random shuffle of all trees in the list
void Management::randomize()
{
    // fill the "value" part of the tree storage with a random value for each tree
    for (int i=0;i<mTrees.count(); ++i) {
        mTrees[i].second = drandom();
    }
    // now sort the list....
    std::sort(mTrees.begin(), mTrees.end(), treePairValue);

}





