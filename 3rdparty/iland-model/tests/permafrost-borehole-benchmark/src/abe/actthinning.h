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
    enum ThinningType { Invalid, FromBelow, FromAbove, Custom, Selection};
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
        QVector<int> classPercentiles; ///< percentiles [0..100] for the classes (count = count(classValues) + 1
        QJSValue minDbh; ///< only trees with dbh > minDbh are considered (default: 0)
        QJSValue remainingStems; ///< minimum remaining stems/ha (>minDbh)
    };
    struct SSelectiveThinning {
        int N; // stems pro ha target
    };

    SSelectiveThinning mSelectiveThinning;

    QVector<SCustomThinning> mCustomThinnings;
    /// setup function for custom thinnings
    void setupCustom(QJSValue value);
    /// setup function for selective thinnings ("auslesedurchforstung")
    void setupSelective(QJSValue value);

    // setup a single thinning definition
    void setupSingleCustom(QJSValue value, SCustomThinning &custom);
    bool evaluateCustom(FMStand *stand, SCustomThinning &custom);
    int selectRandomTree(FMTreeList *list, const int pct_min, const int pct_max, const bool selective);
    int selectSelectiveSpecies(FMTreeList *list, const bool is_selective, const int index);
    void clearTreeMarks(FMTreeList *list);
    QHash<const Species*, double> mSpeciesSelectivity;

    // selective
    bool evaluateSelective(FMStand *stand);
    bool markCropTrees(FMStand* stand);
    float testPixel(const QPointF &pos,  Grid<float> &grid);
    void setPixel(const QPointF &pos,  Grid<float> &grid);
    ThinningType mThinningType;

    // syntax checking
    static QStringList mSyntaxCustom;
    static QStringList mSyntaxSelective;

};


} // namespace
#endif // ACTTHINNING_H
