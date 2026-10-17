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
#include "bitecolonization.h"
#include "biteengine.h"
#include "bitecell.h"
#include "fmtreelist.h"

namespace BITE {

BiteColonization::BiteColonization()
{

}

BiteColonization::BiteColonization(QJSValue obj): BiteItem(obj)
{

}

void BiteColonization::setup(BiteAgent *parent_agent)
{
    BiteItem::setup(parent_agent);

    try {

        checkProperties(mObj);
        QJSValue disp_filter = BiteEngine::valueFromJs(mObj, "dispersalFilter", "1", "required property.");
        mDispersalFilter.setup(disp_filter, DynamicExpression::CellWrap, parent_agent);


        QJSValue species_filter = BiteEngine::valueFromJs(mObj, "speciesFilter");
        if (!species_filter.isUndefined()) {
            qCDebug(biteSetup) << "species filter: " << species_filter.toString();
        }
        QJSValue cell_filter = BiteEngine::valueFromJs(mObj, "cellFilter");
        if (!cell_filter.isUndefined()) {
            qCDebug(biteSetup) << "cell filter: " << cell_filter.toString();
            mCellConstraints.setup(cell_filter, DynamicExpression::CellWrap, parent_agent);
        }
        QJSValue tree_filter = BiteEngine::valueFromJs(mObj, "treeFilter");
        if (!tree_filter.isUndefined()) {
            qCDebug(biteSetup) << "tree filter: " << tree_filter.toString();
            mTreeConstraints.setup(tree_filter, DynamicExpression::TreeWrap, parent_agent);
        }
        QJSValue sap_filter = BiteEngine::valueFromJs(mObj, "saplingFilter");
        if (!sap_filter.isUndefined()) {
            qCDebug(biteSetup) << "sapling filter: " << sap_filter.toString();
            mSaplingConstraints.setup(sap_filter, DynamicExpression::SaplingWrap, parent_agent);
        }


        QJSValue init_biomass = BiteEngine::valueFromJs(mObj, "initialAgentBiomass");
        if (!init_biomass.isUndefined()) {
            qCDebug(biteSetup) << "initial agent biomass: " << init_biomass.toString();
            mInitialAgentBiomass.setup(init_biomass, DynamicExpression::TreeWrap, parent_agent);
        }

        mThis = BiteEngine::instance()->scriptEngine()->newQObject(this);
        BiteAgent::setCPPOwnership(this);

        mEvents.setup(mObj, QStringList() << "onCalculate" << "onSetup", agent());

        QJSValueList eparam = QJSValueList() << thisJSObj();
        mEvents.run("onSetup", nullptr, &eparam);

    } catch (const IException &e) {
        QString error = QString("An error occured in the setup of BiteColonization item '%1': %2").arg(name()).arg(e.message());
        qCInfo(biteSetup) << error;
        BiteEngine::instance()->error(error);

    }


}

void BiteColonization::afterSetup()
{
    iAgentBiomass = BiteWrapper(agent()->wrapper()).variableIndex("agentBiomass");
    if (mInitialAgentBiomass.isValid() && iAgentBiomass<0)
        throw IException("BiteColonization: initial agent biomass requires that the 'agentBiomass' variable is available");
}

void BiteColonization::runCell(BiteCell *cell, ABE::FMTreeList *treelist, ABE::FMSaplingList *saplist)
{
    // no colonization if agent is already living on the cell
    if (cell->isActive())
        return;

    if (mDispersalFilter.evaluateBool(cell)==false) {
        return;
    }

    if (agent()->verbose())
        qCDebug(bite) << "BiteCol:runCell:" << cell->info();

    ++agent()->stats().nColonizable;

    double result = mCellConstraints.evaluate(cell);
    if (result == 0.) {
        return; // no colonization
    }

    // now we need to load the trees and saplings, and evaluate
    bool check_tree = !mTreeConstraints.isConst();
    bool check_sap = !mSaplingConstraints.isConst();
    bool pass_tree = true, pass_sap = true;
    if (check_tree) {
        cell->checkTreesLoaded(treelist);
        result = mTreeConstraints.evaluate(treelist);
        if (result == 0.)
            pass_tree = false; // constraint not met!
    }
    if (check_sap) {
        cell->checkSaplingsLoaded(saplist);
        result = mSaplingConstraints.evaluate(saplist);
        if (result==0.)
            pass_sap = false; // constraint not met!
    }

    if ( check_tree && check_sap ) {
        // if both constraints are checked, both must fail to stop
        if (pass_sap == false && pass_tree == false)
            return;
    } else {
        // if only one constraint is evaluted, we stop if it evaluates to false
        if (pass_sap && pass_tree == false)
            return;
    }



    QJSValue event_res = mEvents.run("onCalculate", cell);
    if (event_res.isBool() && event_res.toBool()==false) {
        return; // event returned false
    }
    // successfully colonized
    cell->setActive(true);
    if (mInitialAgentBiomass.isValid()) {
        BiteWrapper bitewrap(agent()->wrapper(), cell);
        double init_value = 0.;
        if (mInitialAgentBiomass.type() == DynamicExpression::ftExpression) {
        for (int i=0;i<treelist->count(); ++i)
            init_value  += mInitialAgentBiomass.evaluate(treelist->trees()[i].first);
        } else {
            init_value = mInitialAgentBiomass.evaluate(cell);
        }

        bitewrap.setValue(iAgentBiomass, init_value);
    }
    agent()->notifyItems(cell, BiteCell::CellColonized);
    ++agent()->stats().nNewlyColonized;

}

QStringList BiteColonization::allowedProperties()
{
    QStringList l = BiteItem::allowedProperties();
    l << "speciesFilter" << "cellFilter" << "treeFilter" << "dispersalFilter" << "saplingFilter" << "initialAgentBiomass";
    return l;

}


} // end namespace
