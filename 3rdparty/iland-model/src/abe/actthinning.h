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
#ifndef ACTTHINNING_H
#define ACTTHINNING_H
#include "activity.h"
#include "grid.h"

class Species; // forward

namespace ABE {

class FMSTP; // forward
class FMStand; // forward
class FMTreeList; // forward


class ActThinning : public Activity
{
public:
    ActThinning(FMSTP *parent);
    enum ThinningType { Invalid, FromBelow, FromAbove, Custom, Selection, Tending};
    QString type() const;
    void setup(QJSValue value);
    bool evaluate(FMStand *stand);
    bool execute(FMStand *stand);
private:
    struct SCustomThinning {
        QString filter; ///< additional filter
        bool usePercentiles; ///< if true, classes relate to percentiles, if 'false' classes relate to relative dbh classes
        bool removal; ///< if true, classes define removals, if false trees should *stay* in the class
        bool relative; ///< if true, values are per cents, if false, values are absolute values per hectare
        QJSValue targetValue; ///< the number (per ha) that should be removed, see targetVariable
        bool targetRelative; ///< if true, the target variable is relative to the stock, if false it is absolute
        QString targetVariable; ///< target variable ('volume', 'basalArea', 'stems') / ha
        QVector<double> classValues; ///< class values (the number of values defines the number of classes)
        QVector<int> cumClassPercentiles; ///< cumulative percentiles [0..100] of the relative classes
        QVector<int> cumSelectPercentiles; ///< cumulative probability weight of the selectoin
        QJSValue minDbh; ///< only trees with dbh > minDbh are considered (default: 0)
        QJSValue remainingStems; ///< minimum remaining stems/ha (>minDbh)
    };
    struct SSelectiveThinning {
        QJSValue N; ///< stems pro ha target
        QJSValue speciesProb; ///< probability [0...1] for each species to get picked as crop tree (second order after ranking)
        QJSValue rankingExpr; ///< ranking expression to order trees for selecting crop trees (default: 'height', but can be e.g. 'height + 10*(species=fasy)')
        QJSValue Ncompetitors; ///< number of competitors to mark per crop tree (default: 1.5)
    };

    SSelectiveThinning mSelectiveThinning;

    struct STendingThinning {
        QJSValue speciesProb; ///< probability [0...1] for each species to get picked as crop tree (second order after ranking)
        double intensity; ///< factor that defines tending intensity. 0.0001 - 100
    } mTendingThinning;

    QVector<SCustomThinning> mCustomThinnings;
    /// setup function for custom thinnings
    void setupCustom(QJSValue value);
    /// setup function for selective thinnings ("auslesedurchforstung")
    void setupSelective(QJSValue value);
    /// setup function for tending ("mischwuchsregulierung")
    void setupTending(QJSValue value);

    // setup a single thinning definition
    void setupSingleCustom(QJSValue value, SCustomThinning &custom);
    bool evaluateCustom(FMStand *stand, SCustomThinning &custom);
    int selectRandomTree(FMTreeList *list, const int pct_min, const int pct_max, const bool selective);
    int selectSelectiveSpecies(FMTreeList *list, const bool is_selective, const int index);
    void clearTreeMarks(FMTreeList *list);
    QHash<const Species*, double> mSpeciesSelectivity;

    // selective
    bool evaluateSelective(FMStand *stand);
    bool markCropTrees(FMStand* stand, bool selective_species=false);
    float testPixel(const QPointF &pos,  Grid<float> &grid);
    void setPixel(const QPointF &pos,  Grid<float> &grid);

    // tending

    bool evaluateTending(FMStand *stand);
    bool runTending(FMStand* stand);



    ThinningType mThinningType;

    // general
    bool populateSpeciesSelectivity(QJSValue value, double default_value=1.);
    // syntax checking
    static QStringList mSyntaxCustom;
    static QStringList mSyntaxSelective;

};


} // namespace
#endif // ACTTHINNING_H
