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
#ifndef BITEIMPACT_H
#define BITEIMPACT_H

#include "biteitem.h"
#include "bitecellscript.h"

namespace BITE {


class BiteImpact: public BiteItem
{
    Q_OBJECT

public:
    Q_INVOKABLE BiteImpact(QJSValue obj);
    void setup(BiteAgent *parent_agent);
    QString info();
    // void notify(BiteCell *cell, BiteCell::ENotification what);

public slots:
    void afterSetup();
    void runCell(BiteCell *cell, ABE::FMTreeList *treelist, ABE::FMSaplingList *saplist);

protected:
    QStringList allowedProperties();
private:
    struct BiteImpactItem {
      void setup(QJSValue obj, int index, BiteAgent *parent_agent);
      enum ImpactTarget {Tree, Foliage, Root, Sapling, Browsing, Invalid};
      ImpactTarget target;
      DynamicExpression fractionOfTrees;
      DynamicExpression fractionPerTree;
      DynamicExpression maxTrees;
      DynamicExpression maxBiomass;
      Expression treeFilter;
      double fineRootMultiplier;
      bool hasMaxTrees() const { return maxTrees.isValid(); }
      bool hasMaxBiomass() const { return maxBiomass.isValid(); }
      bool hasFractionOfTrees() const { return fractionOfTrees.isValid(); }
      bool hasFractionPerTree() const { return fractionPerTree.isValid(); }
      QString order;
      int id;
    };
    bool runImpact(BiteImpactItem *item, BiteCell *cell, ABE::FMTreeList *treelist, ABE::FMSaplingList *saplist);
    bool runImpactTrees(BiteImpactItem *item, BiteCell *cell, ABE::FMTreeList *treelist);
    bool runImpactSaplings(BiteImpactItem *item, BiteCell *cell, ABE::FMSaplingList *saplist);

    DynamicExpression mImpactFilter;
    QString mHostTreeFilter;
    bool mSimulate;
    Events mEvents;
    QString mImportOrder;
    bool mVerbose;

    QVector<BiteImpactItem *> mItems;

};


} // end namespace

#endif // BITEIMPACT_H
