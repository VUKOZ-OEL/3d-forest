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

#ifndef CUSTOMAGGOUT_H
#define CUSTOMAGGOUT_H

#include "output.h"
#include "expression.h"

struct SaplingTree; // forward
struct SaplingCell; // forward
class ResourceUnit; // forward
class Tree; // forward
class MapGrid; // forward
class DeadTree; // forward

class CustomAggOut : public Output
{
public:
    enum AggregationEntity {Invalid, RU, Trees, Saplings, Snags};
    enum AggregationLevel {sInvalid, sRU, sStand, sLandscape};

    CustomAggOut();
    ~CustomAggOut();
    virtual void exec();
    virtual void setup();

    // special functions
    void setStandGrid(MapGrid* mapgrid);
private:

    QList<Output*> mLevels;


};

class CustomAggOutLevel : public Output {
public:
    virtual void exec();
    virtual void setup();
    void setStandGrid(MapGrid* m) { mStandGrid = m; }

private:
    CustomAggOut::AggregationEntity mEntity; ///< aggregation entity (ru, trees, saplings)
    CustomAggOut::AggregationLevel mLevel; ///< spatial level on which data is aggregated (ru, stand, landscape)

    Expression mEntityFilter; ///< filter for entities (trees, ...)
    Expression mLevelFilter; ///< filter for stands, resource-unit-ids, ...
    Expression mCondition; ///< filter for years

    // data structure for a field
    // (is public bc Q_DECLARE_TYPEINFO below)
    struct SDynamicField {
        SDynamicField(): agg_index(-1), var_index(-1), expression(nullptr){}
        int agg_index;
        int var_index;
        Expression expression;
    };

    QVector<SDynamicField*> mFieldList;
    const MapGrid *mStandGrid;

    // helper function to aggregate values of a vector(means, medians, percentiles)
    double aggregate(const SDynamicField &field, QVector<double> &data);
    void populateSaplingData(QMap<QString, QVector<QPair<SaplingTree*, ResourceUnit*> > > &data, Expression &filter, SaplingCell *sapcell, bool by_species);

    void extractByResourceUnit(const bool by_species);

    void runSaplings(); ///< process saplings
    void runTrees(); ///< process on tree level
    void runSnags(); ///< process on snag level

    // functions that process a single element (tree, sapling, ru)
    // and populate the data in fieldlist
    void processTree(const Tree *t, QMap<QString, QVector<QVector<double> > > &data);
    void processSapling(const SaplingTree *t, const ResourceUnit *ru, const QString &speciesId, QMap<QString, QVector<QVector<double> > > &data);
    void processSaplingCell(const SaplingCell *sc, const ResourceUnit *ru, QMap<QString, QVector<QVector<double> > > &data);
    void processRU(const ResourceUnit *ru);
    void processSnag(const DeadTree *dt, QMap<QString, QVector<QVector<double> > > &data);

    // aggregation & write outputs functions
    void writeResults(QMap<QString, QVector<QVector<double> > >  &data, ResourceUnit *ru, int stand_id);
    void writeFirstCols(QString &species_id, ResourceUnit *ru, int stand_id);
};

// declare as relocatable: this tells the QVector container
// that the actual bytes can be copied (moved to another location)
// without invoking the destructor. Without, iLand crashed (Expression-ptr destroyed)
//Q_DECLARE_TYPEINFO(CustomAggOutLevel::SDynamicField, Q_RELOCATABLE_TYPE);

#endif // DYNAMICSTANDOUT_H
