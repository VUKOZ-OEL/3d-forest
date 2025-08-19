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
#ifndef FMTREELIST_H
#define FMTREELIST_H

#include <QObject>
#include "scriptglobal.h"
#include "grid.h"
#include "scripttree.h"

class Tree; // forward
class Expression; // forward

namespace ABE {
class FMStand;

class FMTreeList : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int standId READ standId) ///< return stand, -1 if not set
    Q_PROPERTY(int count READ count) ///< return the number of trees that are currently loaded
    Q_PROPERTY(bool simulate READ simulate WRITE setSimulate) ///< if 'simulate' is true, trees are only marked for removal
public:

    Q_INVOKABLE explicit FMTreeList(QObject *parent = nullptr);
    explicit FMTreeList(FMStand *stand, QObject *parent = nullptr);
    ~FMTreeList();
    int standId() const { return mStandId; }
    void setStand(FMStand *stand);
    bool simulate() const {return mOnlySimulate; }
    void setSimulate(bool do_simulate) {mOnlySimulate = do_simulate; }
    int count() const { return mTrees.count(); }

    /// load trees from a portion of a RU
    int loadFromRect(ResourceUnit *ru, const QRectF &rect);

    /// load all trees from a RU
    int loadFromRU(ResourceUnit *ru, bool append=false);


    /// access the list of trees
    const QVector<QPair<Tree*, double> > trees() const { return mTrees; }

    /// access to local grid (setup if necessary) - return a reference (no copy)
    Grid<float> &localStandGrid() { prepareGrids(); return mLocalGrid; }

    static void addToScriptEngine(QJSEngine* engine);

signals:

public slots:
    // loading of trees
    /// load all trees of the stand, return number of trees (living trees)
    int loadAll() { return load(QString()); }
    /// load all trees passing the filter, return number of trees (load only living trees)
    int load(const QString &filter);

    /// clear the list (this does not affect trees loaded)
    void clear() { mTrees.clear(); }

    /// apply a filter on the current tree list. Only trees for which 'filter' returns true remain in the list.
    /// returns the number of tree that remain in the list.
    int filter(QString filter);

    /// filter randomly until N trees remain from the current list of trees
    int filterRandomExclude(int N);
    /// filter randomly N trees from the current list of trees
    int filterRandom(int n_remove);

    /// load trees from a specific Patch (within a staind) (TEST)
    int loadFromPatch(int patchId, bool append=false);

    int loadFromList(FMTreeList *from, QString filter_cond=QString());

    /// spatial filter *within* the stand
    /// trees are kept in the list if the expession 'filter' returns true for the location of the tree
    int spatialFilter(QJSValue grid, QString filter);

    /// load all trees of the stand and either kill or harvest trees that are marked for that operation.
    int removeMarkedTrees();

    /// reset all marks of currently loaded trees to zero
    int resetMarks();

    /// set a given flag for all trees in the list
    void setFlag(ScriptTree::Flags flag, bool value=true);

    /// access to single trees (returns a reference)
    QJSValue tree(int index);
    /// return a copy of a tree
    QJSValue treeObject(int index);


    /** kill "number" of stems
     *  in the percentile interval "from" - "to".
     *  remove all if "number" is higher than the count.
     *  return the number of removed trees. */
//    int killPct(int pctfrom, int pctto, int number);
//    int killAll(); ///< kill all trees in the list
    /** kill 'fraction' of all trees with 'filter'=true */
//    int kill(QString filter, double fraction);
    // management
    /** kill "number" of stems
     *  in the percentile interval "from" - "to".
     *  remove all if "number" is higher than the count.
     * Use the removal fractions set by the removeStem, removeBranch and removeFoliage properties.
     *  return the number of removed trees. */
//    int managePct(int pctfrom, int pctto, int number);
//    int manageAll(); ///< manage all trees in the list
    /// kill (i.e., cut down and do not remove from the forest) the trees in the list, filtered with 'filter'.
    int kill(QString filter=QString());

    /** manage 'fraction' of all trees [0..1] with 'filter'. Return number of removed trees. */
    int harvest(QString filter=QString(), double fraction=1.);

    double percentile(int pct); ///< get value for the pct th percentile (1..100)
//    void killSaplings(MapGridWrapper *wrap, int key); ///< kill all saplings that are on the area denoted by 'key' of the given grid (script access)

    /** hacky access function to resource units covered by a polygon.
     the parameters are "remove-fractions": i.e. value=0: no change, value=1: set to zero. */
//    void removeSoilCarbon(MapGridWrapper *wrap, int key, double SWDfrac, double DWDfrac, double litterFrac, double soilFrac);
    /** slash snags (SWD and otherWood-Pools) of polygon 'key' on the map 'wrap'.
      @param slash_fraction 0: no change, 1: 100%
       */
//    void slashSnags(MapGridWrapper *wrap, int key, double slash_fraction);
    /**  sort the list according to 'statement'. Note that sorting is in ascending order. To
     *   have e.g. tallest trees first in the list, use '-height'.
    */
    void sort(QString statement); ///< sort trees in the list according to a criterion
//    int filter(QString filter); ///< apply a filter on the list of trees (expression), return number of remaining trees.
//    int filterIdList(QVariantList idList); ///< apply filter in form of a list of ids, return number of remaining trees
    void randomize(); ///< random shuffle of all trees in the list

    /// calculate the mean value for all trees in the internal list for 'expression' (filtered by the filter criterion)
    double mean(QString expression, QString filter=QString()) { return aggregate_function( expression, filter, "mean"); }
    /// calculate the sum for all trees in the internal list for the 'expression' (filtered by the filter criterion)
    double sum(QString expression, QString filter=QString()) { return aggregate_function( expression, filter, "sum"); }

    /// set up internally a map (10m grid cells) of the stand
    /// with a given grid type or using a custom expression.
    void prepareLocalGrid(QString type, QString custom_expression=QString());
    void exportStandGrid(QString file_name);
    /// get access to the locally prepared grid
    QJSValue localGrid();
    QJSValue local10Grid();

    /// modify sapling
    int killSaplings(QString expression);
    /// calculate the mean value for all trees in the internal list for 'expression' (filtered by the filter criterion)
    double meanSaplings(QString expression, QString filter=QString()) { return aggregate_function_sapling(expression, filter, "mean"); }
    /// calculate the sum for all trees in the internal list for the 'expression' (filtered by the filter criterion)
    double sumSaplings(QString expression, QString filter=QString()) { return aggregate_function_sapling(expression, filter, "sum"); }



private:
    bool trace() const;
    ///
    int remove_percentiles(int pctfrom, int pctto, int number, bool management);
    int remove_trees(QString expression, double fraction, bool management);
    double aggregate_function(QString expression, QString filter, QString type);
    double aggregate_function_sapling(QString expression, QString filter, QString type);
    bool remove_single_tree(int index, bool harvest=true);
    void check_locks();

    // grid functions
    void prepareGrids();
    /// run function 'func' for all trees in the current tree list of the stand.
    /// signature: function(&ref_to_float, &ref_to_int, *tree);
    /// after all trees are processed, func is called again (aggregations, ...) with tree=0.
    void runGrid(void (*func)(float &, int &, const Tree *, const FMTreeList *) );

    QVector<QPair<Tree*, double> > mTrees; ///< store a Tree-pointer and a value (e.g. for sorting)
    bool mResourceUnitsLocked;
    int mRemoved;
    FMStand *mStand; /// the stand the list is currently connected
    int mStandId; ///< link to active stand
    int mNumberOfStems; ///< estimate for the number of trees in the stand
    bool mOnlySimulate; ///< mode
    QRectF mStandRect;
    FloatGrid mStandGrid; ///< local stand grid (10m pixel)
    Grid<int> mTreeCountGrid; ///< tree counts on local stand grid (10m)
    Grid<float> mLocalGrid; ///< 2m grid of the stand
    Expression *mRunGridCustom;
    double *mRunGridCustomCell;
    friend void rungrid_custom(float &cell, int &n, const Tree *tree, const FMTreeList *list);

    QJSValue mTreeValue;
    ScriptTree *mTree;


    friend class ActThinning;
};

} // namespace
#endif // FMTREELIST_H
