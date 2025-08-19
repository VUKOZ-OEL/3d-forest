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
#include "abe_global.h"
#include "fmtreelist.h"

#include "forestmanagementengine.h"
// iLand stuff
#include "tree.h"
#include "expression.h"
#include "mapgrid.h"
#include "expressionwrapper.h"
#include "model.h"
#include "resourceunit.h"
#include "helper.h"
#include "fmstand.h"
#include "fomescript.h"
#include "saplings.h"
#include "scriptgrid.h"
#include "patch.h"

namespace ABE {

/** @class FMTreeList
    @ingroup abe
    The FMTreeList class implements low-level functionality for selecting and harvesting of trees.
    The functions of the class are usually accessed via Javascript.

  */


// TODO: fix: removal fractions need to be moved to agent/units/ whatever....
double removeFoliage()  {return 0.;}
double removeStem()  {return 1.;}
double removeBranch()  {return 0.;}

FMTreeList::FMTreeList(QObject *parent) :
    QObject(parent)
{
    mStand = nullptr;
    setStand(nullptr); // clear stand link
    mResourceUnitsLocked = false;
    mTree = new ScriptTree;
    mTreeValue = ForestManagementEngine::scriptEngine()->newQObject(mTree);

}

FMTreeList::FMTreeList(FMStand *stand, QObject *parent):
    QObject(parent)
{
    mStand = nullptr;
    setStand(stand);
    mResourceUnitsLocked = false;
    mTree = new ScriptTree;
    mTreeValue = ForestManagementEngine::scriptEngine()->newQObject(mTree);
}

FMTreeList::~FMTreeList()
{
    check_locks();
}

void FMTreeList::setStand(FMStand *stand)
{
    check_locks();
    mStand = stand;
    if (stand) {
        mStandId = stand->id();
        mNumberOfStems = stand->stems() * stand->area();
        mOnlySimulate = stand->currentActivity()?stand->currentFlags().isScheduled() : false;
        mStandRect=QRectF();
    } else {
        mStandId = -1;
        mNumberOfStems = 1000;
        mOnlySimulate = false;
    }

}

int FMTreeList::loadFromRect(ResourceUnit *ru, const QRectF &rect)
{
    mTrees.clear();
    for (const Tree &t : ru->trees()) {
        if (rect.contains(t.position()) && !t.isDead()) {
            mTrees.push_back(QPair<Tree*, double>(const_cast<Tree*>(&t), 0.));
        }
    }
    return mTrees.size();
}

int FMTreeList::loadFromRU(ResourceUnit *ru, bool append)
{
    if (!append)
        mTrees.clear();
    for (const Tree &t : ru->trees()) {
        if (!t.isDead()) {
            mTrees.push_back(QPair<Tree*, double>(const_cast<Tree*>(&t), 0.));
        }
    }
    return mTrees.size();

}

void FMTreeList::addToScriptEngine(QJSEngine *engine)
{
    QJSValue jsMetaObject = engine->newQMetaObject(&ABE::FMTreeList::staticMetaObject);
    engine->globalObject().setProperty("TreeList", jsMetaObject);

}

int FMTreeList::loadFromPatch(int patchId, bool append)
{
    if (!append)
        mTrees.clear();
    // test
    for (auto tp : mTrees) {
        int patch = mStand->patches()->patch(tp.first->positionIndex());
        qDebug() << "Tree position: " << tp.first->positionIndex() << " - patch: " << patch;
    }
    return mTrees.size();

}

int FMTreeList::loadFromList(FMTreeList *from, QString filter_cond)
{
    setStand(from->mStand);
    mTrees = from->mTrees; // copy trees from the source list. With Qt copy on write semantic, a deep copy happens when this list is changed
    return filter(filter_cond);
}



int FMTreeList::load(const QString &filter)
{
    if (standId()>-1) {
        // load all trees of the current stand
        const MapGrid *map = ForestManagementEngine::instance()->standGrid();
        if (map->isValid()) {
            map->loadTrees(mStandId, mTrees, filter, mNumberOfStems);
            mResourceUnitsLocked = true;
        } else {
            qCDebug(abe) << "FMTreeList::load: grid is not valid - no trees loaded";
        }
        return mTrees.count();

    } else {
        qCDebug(abe) << "FMTreeList::load: loading *all* trees, because stand id is -1";
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
            qDebug() << "filtering with" << filter;
            while (Tree *t=at.nextLiving()) {
                tw.setTree(t);
                if (!t->isDead() && expr.execute())
                    mTrees.push_back(QPair<Tree*, double>(t, 0.));
            }
        }
        return mTrees.count();
    }
}

int FMTreeList::filter(QString filter)
{
    if (filter.isEmpty())
        return mTrees.size();

    TreeWrapper tw;
    Expression expression(filter, &tw);
    expression.enableIncSum();
    QPair<Tree*, double> empty_tree(nullptr,0.);

    for (int i=0;i<mTrees.size();++i) {
        tw.setTree(mTrees[i].first);
        if (mTrees[i].first->isDead() || !expression.execute())
            mTrees[i] = empty_tree; // mark for removal
    }
    int n_rem = mTrees.removeAll(empty_tree);
    if (logLevelDebug())
        qDebug() << "apply filter" << filter << ", removed" << n_rem;
    return mTrees.size();
}

int FMTreeList::filterRandomExclude(int N)
{
    int to_remove = mTrees.size() - N;
    return filterRandom(to_remove);
}

int FMTreeList::filterRandom(int n_remove)
{
    QPair<Tree*, double> empty_tree(nullptr,0.);

    if (n_remove <= 0)
        return 0;
    double p_remove = n_remove / double(mTrees.size());
    int removed = 0;
    int n_loops = 0;
    while (removed < n_remove) {
        for (int i=0;i<mTrees.size() && removed<n_remove;++i) {
            if (mTrees[i].first) {
                if (drandom() < p_remove) {
                    mTrees[i] = empty_tree;
                    ++removed;
                }
            }
        }
        ++n_loops;
        if (n_loops > 10)
            break;
    }
    mTrees.removeAll(empty_tree);
    if (logLevelDebug())
        qDebug() << "random selection: number of loops: " << n_loops << ", to remove: " << n_remove << ", removed" << removed;
    return removed;

}

int FMTreeList::spatialFilter(QJSValue grid, QString filter)
{
    QObject *o = grid.toQObject();
    ScriptGrid *sg = qobject_cast<ScriptGrid*>(o);
    if (!o) {
       qDebug() << "ERROR: Invalid grid in call to treelist::spatialFilter()!";
       return -1;
    }
    Expression expr;
    double *var = expr.addVar( sg->name() );
    try {
        expr.setExpression(filter);
        expr.parse();
    } catch(const IException &e) {
        qDebug() << "JS - treelist::spatialFilter(): expression ERROR: " << e.message();
        return -1;
    }

    QPair<Tree*, double> empty_tree(nullptr,0.);
    QPointF tc;
    for (int i=0;i<mTrees.size();++i) {
        tc = mTrees[i].first->position();
        if (sg->grid()->coordValid(tc)) {
            // set variable of expression
            *var = sg->grid()->valueAt(tc);
            if (mTrees[i].first->isDead() || !expr.execute())
                mTrees[i] = empty_tree; // mark
        } else {
            mTrees[i] = empty_tree;
        }
    }
    int n_rem = mTrees.removeAll(empty_tree);
    if (logLevelDebug())
        qDebug() << "apply spatial filter" << filter << ", removed" << n_rem;
    return mTrees.size();

}

int FMTreeList::removeMarkedTrees()
{
    loadAll();
    int n_removed = 0;
    for (QVector<QPair<Tree*, double> >::const_iterator it = mTrees.constBegin(); it!=mTrees.constEnd(); ++it) {
        Tree *t = const_cast<Tree*>((*it).first);
        if (t->isMarkedForCut()) {
            t->remove();
            n_removed++;
        } else if (t->isMarkedForHarvest()) {
            t->remove(removeFoliage(), removeBranch(), removeStem());
            n_removed++;
        }
    }
    if (mStand->trace())
        qCDebug(abe) << mStand->context() << "removeMarkedTrees: n=" << n_removed;

    return n_removed;
}

int FMTreeList::resetMarks()
{
    int n=0;
    for (QVector<QPair<Tree*, double> >::const_iterator it = mTrees.constBegin(); it!=mTrees.constEnd(); ++it) {
        Tree *t = const_cast<Tree*>((*it).first);
        t->markCropCompetitor(false);
        t->markCropTree(false);
        t->markForCut(false);
        t->markForHarvest(false);
        ++n;
    }
    return n;
}

void FMTreeList::setFlag(ScriptTree::Flags flag, bool value)
{
    for (QVector<QPair<Tree*, double> >::const_iterator it = mTrees.constBegin(); it!=mTrees.constEnd(); ++it) {
        Tree *t = const_cast<Tree*>((*it).first);
        ScriptTree::setTreeFlag(t, flag, value);
    }
}

QJSValue FMTreeList::tree(int index)
{
    if (index<0 || index>=count())
        mTree->clear();
    else
        mTree->setTree( mTrees[index].first );
    return mTreeValue;

}

QJSValue FMTreeList::treeObject(int index)
{
    Tree *tree = nullptr;
    if (index>=0 && index<count())
        tree = mTrees[index].first;

    ScriptTree *tobj = new ScriptTree();
    tobj->setTree(tree);
    QJSValue val = ForestManagementEngine::instance()->scriptEngine()->newQObject(tobj);
    return val;

}

int FMTreeList::kill(QString filter)
{
    return remove_trees(filter, 1. /* all trees, 100%*/, false);
}

int FMTreeList::harvest(QString filter, double fraction)
{
    return remove_trees(filter, fraction, true);

}

bool FMTreeList::trace() const
{
    return FomeScript::bridge()->standObj()->trace();
}


int FMTreeList::remove_percentiles(int pctfrom, int pctto, int number, bool management)
{
    if (mTrees.isEmpty())
        return 0;
    int index_from = limit(int(pctfrom/100. * mTrees.count()), 0, mTrees.count());
    int index_to = limit(int(pctto/100. * mTrees.count()), 0, mTrees.count()-1);
    if (index_from>=index_to)
        return 0;
    //qDebug() << "attempting to remove" << number << "trees between indices" << index_from << "and" << index_to;
    int i;
    int count = number;
    if (index_to-index_from <= number)  {
        // kill all
        if (management) {
            // management
            for (i=index_from; i<index_to; i++)
                if (simulate()) {
                    mTrees.at(i).first->markForHarvest(true);
                    mStand->addScheduledHarvest(mTrees.at(i).first->volume());
                } else {
                    mTrees.at(i).first->remove(removeFoliage(), removeBranch(), removeStem());
                }
        } else {
            // just kill...
            for (i=index_from; i<index_to; i++)
                if (simulate()) {
                    mTrees.at(i).first->markForCut(true);
                    mStand->addScheduledHarvest(mTrees.at(i).first->volume());
                } else
                    mTrees.at(i).first->remove();
        }
        count = index_to - index_from;
    } else {
        // kill randomly the provided number
        int cancel = 1000;
        while(number>=0) {
            int rnd_index = irandom(index_from, index_to);
            Tree *tree = mTrees[rnd_index].first;
            if (tree->isDead() || tree->isMarkedForHarvest() || tree->isMarkedForCut()) {
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
                if (simulate()) {
                    tree->markForHarvest(true);
                    mStand->addScheduledHarvest( tree->volume());
                } else
                    tree->remove( removeFoliage(), removeBranch(), removeStem() );
            } else {
                if (simulate()) {
                    tree->markForCut(true);
                    mStand->addScheduledHarvest( tree->volume());
                } else
                    tree->remove();
            }
        }
    }
    if (mStand && mStand->trace())
        qCDebug(abe) << "FMTreeList::remove_percentiles:" << count << "removed.";
    // clean up the tree list...
    for (int i=mTrees.count()-1; i>=0; --i) {
        if (mTrees[i].first->isDead())
            mTrees.removeAt(i);
    }
    return count; // killed or manages

}

/** remove trees from a list and reduce the list.

  */
int FMTreeList::remove_trees(QString expression, double fraction, bool management)
{
    TreeWrapper tw;
    if (expression.isEmpty())
        expression="true";
    Expression expr(expression,&tw);
    expr.enableIncSum();
    int n = 0;
    QPair<Tree*, double> empty_tree(nullptr,0.);
    QVector<QPair<Tree*, double> >::iterator tp=mTrees.begin();
    try {
        for (;tp!=mTrees.end();++tp) {
            // we skip trees that are explicitly marked as
            // "NoHarvest".
            if (tp->first->isMarkedNoHarvest())
                continue;

            tw.setTree(tp->first);
            // if expression evaluates to true and if random number below threshold...
            if (expr.calculate(tw) && drandom() <=fraction) {
                // remove from system
                if (management) {
                    if (simulate()) {
                        tp->first->markForHarvest(true);
                        mStand->addScheduledHarvest(tp->first->volume());
                    } else {
                        tp->first->markForHarvest(true);
                        tp->first->remove(removeFoliage(), removeBranch(), removeStem()); // management with removal fractions
                    }
                } else {
                    if (simulate()) {
                        tp->first->markForCut(true);
                        tp->first->setDeathCutdown();
                        mStand->addScheduledHarvest(tp->first->volume());
                    } else {
                        tp->first->markForCut(true);
                        tp->first->setDeathCutdown();
                        tp->first->remove(); // kill
                    }
                }
                // remove from tree list
                *tp = empty_tree;
                n++;
            }
        }
        mTrees.removeAll(empty_tree);
    } catch(const IException &e) {
        qCWarning(abe) << "treelist: remove_trees: expression:" << expression << ", msg:" << e.message();
    }
    return n;

}

double FMTreeList::aggregate_function(QString expression, QString filter, QString type)
{
    QVector<QPair<Tree*, double> >::iterator tp=mTrees.begin();
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
        qCWarning(abe) << "Treelist: aggregate function: expression:" << expression << ", filter:" << filter << ", msg:" <<e.message();
        //throwError(e.message());
    }
    if (type=="sum")
        return sum;
    if (type=="mean")
        return n>0?sum/double(n):0.;
    return 0.;

}

double FMTreeList::aggregate_function_sapling(QString expression, QString filter, QString type)
{
    SaplingWrapper sw;
    Expression filter_expr(filter.isEmpty() ? "true" : filter, &sw);
    Expression expr(expression,&sw);

    SaplingCellRunner scr(mStandId, GlobalSettings::instance()->model()->ABEngine()->standGrid());
    double sum = 0.;
    int n=0;
    try{
    while (SaplingCell *sc = scr.next()) {
        if (sc){
            for (int i=0;i<NSAPCELLS;++i) {
                if (sc->saplings[i].is_occupied()) {
                    sw.setSaplingTree(&sc->saplings[i], sc->ru);
                    if (filter_expr.execute()) {
                        sum += expr.calculate();
                        ++n;
                    }
                }
            }
        }
    }
    } catch(const IException &e) {
        qCWarning(abe) << "Treelist: aggregate function for saplings: expression:" << expression << ", filter:" << filter << ", msg:" <<e.message();
    }
    if (type=="sum")
        return sum;
    if (type=="mean")
        return n>0?sum/double(n):0.;
    qCWarning(abe) << "invalid aggregate function for saplings (allowed: 'mean', 'sum'): " << type;
    return 0.;
}

bool FMTreeList::remove_single_tree(int index, bool harvest)
{
    if (!mStand || index<0 || index>=mTrees.size())
        return false;
    Tree *tree = mTrees.at(index).first;
    if (harvest) {
        if (simulate()) {
            tree->markForHarvest(true);
            mStand->addScheduledHarvest( tree->volume());
        } else
            tree->remove( removeFoliage(), removeBranch(), removeStem() );
    } else {
        if (simulate()) {
            tree->markForCut(true);
            mStand->addScheduledHarvest(  tree->volume());
        } else
            tree->remove();
    }
    return true;
}


bool treePairValue(const QPair<Tree*, double> &p1, const QPair<Tree*, double> &p2)
{
    return p1.second < p2.second;
}

void FMTreeList::sort(QString statement)
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

double FMTreeList::percentile(int pct)
{
    if (mTrees.count()==0)
        return -1.;
    int idx = int( (pct/100.) * mTrees.count());
    if (idx == mTrees.count())
        return mTrees.at(idx-1).second; // special case pct=100 -> last entry
    if (idx>=0 && idx<mTrees.count())
        return mTrees.at(idx).second;
    else
        return -1;
}

/// random shuffle of all trees in the list
void FMTreeList::randomize()
{
    // fill the "value" part of the tree storage with a random value for each tree
    for (int i=0;i<mTrees.count(); ++i) {
        mTrees[i].second = drandom();
    }
    // now sort the list....
    std::sort(mTrees.begin(), mTrees.end(), treePairValue);

}


void FMTreeList::prepareGrids()
{
    QRectF box = ForestManagementEngine::instance()->standGrid()->boundingBox(mStand->id());
    if (mStandRect==box)
        return;
    mStandRect = box;
    // the memory of the grids is only reallocated if the current box is larger then the previous...
    mStandGrid.setup(box, cHeightSize);
    mTreeCountGrid.setup(box, cHeightSize);
    mLocalGrid.setup(box, cPxSize);
    // mark areas outside of the grid...
    GridRunner<int> runner(ForestManagementEngine::instance()->standGrid()->grid(), box);
    float *p=mStandGrid.begin();
    while (runner.next()) {
        if (*runner.current()!=mStand->id())
            *p=-1.f;
        else
            *p=0.f;
        ++p;
    }
    // copy stand limits to the grid
    for (int iy=0;iy<mLocalGrid.sizeY();++iy)
        for (int ix=0;ix<mLocalGrid.sizeX();++ix)
            mLocalGrid.valueAtIndex(ix,iy) = mStandGrid.valueAtIndex(ix/cPxPerHeight, iy/cPxPerHeight)==-1.f ? -1.f: 0.f;
}

void FMTreeList::runGrid(void (*func)(float &, int &, const Tree *, const FMTreeList *))
{
    if (mStandRect.isNull())
        prepareGrids();

    // set all values to 0 (within the limits of the stand grid)
    for (float *p=mStandGrid.begin(); p!=mStandGrid.end(); ++p)
        if (*p!=-1.f)
            *p=0.f;
    mTreeCountGrid.initialize(0);
    int invalid_index = 0;
    for (QVector<QPair<Tree*, double> >::const_iterator it=mTrees.constBegin(); it!=mTrees.constEnd(); ++it) {
        const Tree* tree = it->first;
        QPoint p = mStandGrid.indexAt(tree->position());
        if (mStandGrid.isIndexValid(p) && !tree->isDead())
            (*func)(mStandGrid.valueAtIndex(p), mTreeCountGrid.valueAtIndex(p), tree, this);
        else
            ++invalid_index;
    }
    if (invalid_index)
        qDebug() << "FMTreeList::runGrid: invalid index: n=" << invalid_index;

    // finalization: call again for each *cell*
    for (int i=0;i<mStandGrid.count();++i)
        (*func)(mStandGrid.valueAtIndex(i), mTreeCountGrid.valueAtIndex(i), nullptr, this);

}

void rungrid_heightmax(float &cell, int &n, const Tree *tree, const FMTreeList *list)
{
    Q_UNUSED(n); Q_UNUSED(list);
    if (tree)
        cell = qMax(cell, tree->height());
}
void rungrid_basalarea(float &cell, int &n, const Tree *tree, const FMTreeList *list)
{
    Q_UNUSED(list);
    if (tree) {
        cell += tree->basalArea();
        ++n;
    } else {
        if (n>0)
            cell /= float(n);
    }
}
void rungrid_volume(float &cell, int &n, const Tree *tree, const FMTreeList *list)
{
    Q_UNUSED(list);
    if (tree) {
        cell += tree->volume();
        ++n;
    } else {
        if (n>0)
            cell /= float(n);
    }
}

void rungrid_custom(float &cell, int &n, const Tree *tree, const FMTreeList *list)
{
    if (tree) {
        *list->mRunGridCustomCell = cell;
        TreeWrapper tw(tree);
        cell = static_cast<float>(list->mRunGridCustom->calculate(tw));
        ++n;
    }
}
void FMTreeList::prepareLocalGrid(QString type, QString custom_expression)
{
    if(!mStand){
        qCDebug(abe) << "Error: FMTreeList: no current stand defined.";
        return;
    }

    if (type==QStringLiteral("height")) {
        return runGrid(&rungrid_heightmax);
    }

    if (type==QStringLiteral("basalArea"))
        return runGrid(&rungrid_basalarea);

    if (type==QStringLiteral("volume"))
        return runGrid(&rungrid_volume);

    if (type==QStringLiteral("custom")) {
        mRunGridCustom = new Expression(custom_expression);
        mRunGridCustomCell = mRunGridCustom->addVar("cell");
        runGrid(&rungrid_custom);
        delete mRunGridCustom;
        mRunGridCustom = nullptr;
        return;
    }
    qCDebug(abe) << "FMTreeList: invalid type for prepareStandGrid: " << type;
}

void FMTreeList::exportStandGrid(QString file_name)
{
    file_name = GlobalSettings::instance()->path(file_name);
    Helper::saveToTextFile(file_name, gridToESRIRaster(mStandGrid) );
    qCDebug(abe) << "saved grid to file" << file_name;
}

QJSValue FMTreeList::localGrid()
{
    Grid<double> *dgrid = new Grid<double>(mLocalGrid.metricRect(), mLocalGrid.cellsize());
    double *p=dgrid->begin();
    for (float *s=mLocalGrid.begin(); s!=mLocalGrid.end(); ++s,++p)
        *p = *s;

    QJSValue g = ScriptGrid::createGrid(dgrid, "local");
    return g;
}

QJSValue FMTreeList::local10Grid()
{
    Grid<double> *dgrid = new Grid<double>(mStandGrid.metricRect(), mStandGrid.cellsize());
    double *p=dgrid->begin();
    for (float *s=mStandGrid.begin(); s!=mStandGrid.end(); ++s,++p)
        *p = *s;

    QJSValue g = ScriptGrid::createGrid(dgrid, "local10");
    return g;

}

int FMTreeList::killSaplings(QString expression)
{
    int nsap_removed=0;

    SaplingWrapper sw;
    Expression expr(expression.isEmpty() ? "true" : expression, &sw);

    SaplingCellRunner scr(mStandId, GlobalSettings::instance()->model()->ABEngine()->standGrid());
    while (SaplingCell *sc = scr.next()) {
        if (sc){
            for (int i=0;i<NSAPCELLS;++i) {
                if (sc->saplings[i].is_occupied()) {
                    sw.setSaplingTree(&sc->saplings[i], sc->ru);
                    if (expr.execute()) {
                        sc->saplings[i].clear();
                        nsap_removed++;
                    }
                }
            }
            sc->checkState();
        }
    }
    return nsap_removed;
}

void FMTreeList::check_locks()
{
    // removed the locking code again, WR20140821
//    if (mStand && mResourceUnitsLocked) {
//        const MapGrid *map = ForestManagementEngine::instance()->standGrid();
//        if (map->isValid()) {
//            map->freeLocksForStand(mStandId);
//            mResourceUnitsLocked = false;
//        }
//    }
}

} // namespace
