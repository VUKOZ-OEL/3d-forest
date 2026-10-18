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

#ifndef MANAGEMENT_H
#define MANAGEMENT_H

#include <QObject>
#include <QList>
#include <QtCore/QVariantList>
#include "scriptglobal.h"
#include "scripttree.h"

class Tree;
class QJSEngine;
class Management : public QObject //, protected QScriptable
{
    Q_OBJECT
    Q_PROPERTY(int count READ count)
    Q_PROPERTY(double removeFoliage READ removeFoliage WRITE setRemoveFoliage)
    Q_PROPERTY(double removeBranch READ removeBranch WRITE setRemoveBranch)
    Q_PROPERTY(double removeStem READ removeStem WRITE setRemoveStem)
public:
    Management();
    ~Management();
    void run();
    void loadScript(const QString &fileName);
    QString scriptFile() const { return mScriptFile; }

    // property getter & setter for removal fractions
    /// removal fraction foliage: 0: 0% will be removed, 1: 100% will be removed from the forest by management operations (i.e. calls to manage() instead of kill())
    double removeFoliage() const { return mRemoveFoliage; }
    /// removal fraction branch biomass: 0: 0% will be removed, 1: 100% will be removed from the forest by management operations (i.e. calls to manage() instead of kill())
    double removeBranch() const { return mRemoveBranch; }
    /// removal fraction stem biomass: 0: 0% will be removed, 1: 100% will be removed from the forest by management operations (i.e. calls to manage() instead of kill())
    double removeStem() const { return mRemoveStem; }

    void setRemoveFoliage(const double fraction)  { mRemoveFoliage = fraction; }
    void setRemoveBranch(const double fraction)  { mRemoveBranch = fraction; }
    void setRemoveStem(const double fraction)  { mRemoveStem = fraction; }

    int count() const {return mTrees.count();} ///< return number of trees currently in list

public slots:
    /// access to single trees (returns a reference)
    QJSValue tree(int index);
    /// return a copy of a tree
    QJSValue treeObject(int index);

    /// calculate the mean value for all trees in the internal list for 'expression' (filtered by the filter criterion)
    double mean(QString expression, QString filter=QString()) { return aggregate_function( expression, filter, "mean"); }
    /// calculate the sum for all trees in the internal list for the 'expression' (filtered by the filter criterion)
    double sum(QString expression, QString filter=QString()) { return aggregate_function( expression, filter, "sum"); }
    /// remove randomly trees until only 'number' of trees remain.
    /// return number of removed trees
    int remain(int number);
    /** kill "number" of stems
     *  in the percentile interval "from" - "to".
     *  remove all if "number" is higher than the count.
     *  return the number of removed trees. */
    int killPct(int pctfrom, int pctto, int number);
    int killAll(); ///< kill all trees in the list
    /// kill all trees (disturbance related)
    /// @param stem_to_soil_fraction (0..1) of stem biomass that is routed to the soil
    /// @param stem_to_snag_fraction (0..1) of the stem biomass continues as standing dead
    /// @param branch_to_soil_fraction (0..1) of branch biomass that is routed to the soil
    /// @param branch_to_snag_fraction (0..1) of the branch biomass continues as standing dead
    /// @param foliage_to_soil_fraciton (0..1) fraction of biomass that goes directly to the soil. The rest (1.-fraction) is removed.
    /// @param agent (string): disturbance agent ('fire' 'wind', 'bb', ...)
    int disturbanceKill(double stem_to_soil_fraction, double stem_to_snag_fraction, double branch_to_soil_fraction, double branch_to_snag_fraction, QString agent);
    /** kill 'fraction' of all trees with 'filter'=true */
    int kill(QString filter, double fraction);
    // management
    /** kill "number" of stems
     *  in the percentile interval "from" - "to".
     *  remove all if "number" is higher than the count.
     * Use the removal fractions set by the removeStem, removeBranch and removeFoliage properties.
     *  return the number of removed trees. */
    int managePct(int pctfrom, int pctto, int number);
    int manageAll(); ///< manage all trees in the list
    /** manage 'fraction' of all trees with 'filter'=true */
    int manage(QString filter, double fraction);
    /// kill trees, cut down to the ground
    void cutAndDrop();

    double percentile(int pct); ///< get value for the pct th percentile (1..100)
    int loadAll() { return load(QString()); } ///< load all trees, return number of trees
    int load(QString filter); ///< load all trees passing the filter in a list, return number of trees
    int loadResourceUnit(int ruindex); ///< load all trees of a resource index
    void loadFromTreeList(QList<Tree*>tree_list); ///< load a previously present tree list
    void loadFromMap(const MapGrid *map_grid, int key); ///< load all trees that are on the area denoted by 'key' of the given grid
    int loadFromMap(MapGridWrapper *wrap, int key); ///< load all trees that are on the area denoted by 'key' of the given grid (script access)
    /// kill all saplings that are on the area denoted by 'key' of the given grid (script access)
    void killSaplings(MapGridWrapper *wrap, int key, QString filter=QString());
    /// kill all saplings that are on a given resource unit (given by 'ruindex')
    void killSaplingsResourceUnit(int ruindex);
    /** hacky access function to resource units covered by a polygon.
     the parameters are "remove-fractions": i.e. value=0: no change, value=1: set to zero. */
    void removeSoilCarbon(MapGridWrapper *wrap, int key, double SWDfrac, double DWDfrac, double litterFrac, double soilFrac);
    /** slash snags (SWD and otherWood-Pools) of polygon 'key' on the map 'wrap'.
      @param slash_fraction 0: no change, 1: 100%
       */
    void slashSnags(MapGridWrapper *wrap, int key, double slash_fraction);
    void sort(QString statement); ///< sort trees in the list according to a criterion
    int filter(QString filter); ///< apply a filter on the list of trees (expression), return number of remaining trees.
    int filterIdList(QVariantList idList); ///< apply filter in form of a list of ids, return number of remaining trees
    void randomize(); ///< random shuffle of all trees in the list
private:
    QString executeScript(QString cmd="");
    int remove_percentiles(int pctfrom, int pctto, int number, bool management);
    int remove_trees(QString expression, double fraction, bool management);
    double aggregate_function(QString expression, QString filter, QString type);
    void throwError(const QString &errormessage);

    // removal fractions
    double mRemoveFoliage, mRemoveBranch, mRemoveStem;
    QString mScriptFile;
    QList<QPair<Tree*, double> > mTrees;
    QJSEngine *mEngine;
    int mRemoved;
    QJSValue mTreeValue;
    ScriptTree *mTree;
};

#endif // MANAGEMENT_H
