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

#include "treeout.h"
#include "debugtimer.h"
#include "tree.h"
#include "model.h"
#include "resourceunit.h"
#include "species.h"
#include "expressionwrapper.h"

TreeOut::TreeOut()
{
    setName("Tree Output", "tree");
    setDescription("Output of indivdual trees. Use the ''filter'' property to reduce amount of data (filter by resource-unit, year, species, ...).\n" \
                   "The output is triggered after the growth of the current season. " \
                   "Initial values (without any growth) are output as 'startyear-1'.\n" \
                   "The 'treeFlags' is a binary combination of individual flags; see the documentation of the treeremoved output for details.");
    columns() << OutputColumn::year() << OutputColumn::ru() << OutputColumn::id() << OutputColumn::species()
            << OutputColumn("id", "id of the tree", OutInteger)
            << OutputColumn("x", "position of the tree, x-direction (m)", OutDouble)
            << OutputColumn("y", "position of the tree, y-direction (m)", OutDouble)
            << OutputColumn("dbh", "dbh (cm) of the tree", OutDouble)
            << OutputColumn("height", "height (m) of the tree", OutDouble)
            << OutputColumn("basalArea", "basal area of tree in m2", OutDouble)
            << OutputColumn("volume_m3", "volume of tree (m3)", OutDouble)
            << OutputColumn("age", "tree age (years)", OutInteger)
            << OutputColumn("leafArea_m2", "current leaf area of the tree (m2)", OutDouble)
            << OutputColumn("foliageMass", "current mass of foliage (kg)", OutDouble)
            << OutputColumn("stemMass", "kg Biomass in woody department (tree stem, without reserve pool)", OutDouble)
            << OutputColumn("branchMass", "kg Biomass in branches", OutDouble)
            << OutputColumn("fineRootMass", "kg Biomass in fine-root department", OutDouble)
            << OutputColumn("coarseRootMass", "kg Biomass in coarse-root department", OutDouble)
            << OutputColumn("lri", "LightResourceIndex of the tree (raw light index from iLand, without applying resource-unit modifications)", OutDouble)
            << OutputColumn("lightResponse", "light response value (including species specific response to the light level)", OutDouble)
            << OutputColumn("stressIndex", "scalar (0..1) indicating the stress level (see [Mortality]).", OutDouble)
            << OutputColumn("reserve_kg", "NPP currently available in the reserve pool (kg Biomass). The reserve is cenceptually part of the stem and included in the stem compartment for stand level outputs.", OutDouble)
            << OutputColumn("treeFlags", "tree flags (see above)", OutInteger);


 }

void TreeOut::setup()
{
    qDebug() << "treeout::setup() called";
    if (!settings().isValid())
        throw IException("TreeOut::setup(): no parameter section in init file!");
    QString filter = settings().value(".filter","");
    mFilter.setExpression(filter);
}

void TreeOut::exec()
{
    AllTreeIterator at(GlobalSettings::instance()->model());
    DebugTimer t("TreeOut::exec()");
    TreeWrapper tw;
    mFilter.setModelObject(&tw);
    while (Tree *t=at.next()) {
        if (!mFilter.isEmpty()) { // skip fields
            tw.setTree(t);
            if (!mFilter.executeBool())
                continue;
        }
        *this << currentYear() << t->ru()->index() << t->ru()->id() << t->species()->id();
        *this << t->id() << t->position().x() << t->position().y() << t->dbh() << t->height() << t->basalArea() << t->volume() << t->age();
        *this << t->leafArea() << t->mFoliageMass << t->mStemMass << t->biomassBranch()
                               <<  t->mFineRootMass << t->mCoarseRootMass;
        *this << t->lightResourceIndex() << t->mLightResponse << t->mStressIndex << t->mNPPReserve;
        *this << t->flags();
        writeRow();
    }

}



TreeRemovedOut::TreeRemovedOut()
{
    setName("Tree Removed Output", "treeremoved");
    setDescription("Output of removed indivdual trees. Use the ''filter'' property to reduce amount of data (filter by resource-unit, year, species, ...).\n" \
                   "The output is triggered immediately when a tree is removed due to mortality or management.\n " \
                   "\n the column 'treeFlags' is a binary combination of the following values: \n\n" \
                   "||__Flag__|__description__\n" \
                   "TreeDead|1\n"\
                   "TreeDebugging|2\n"\
                   "TreeDeadBarkBeetle|16\nTreeDeadWind|32\n TreeDeadFire|64\n TreeDeadKillAndDrop|128\n TreeHarvested|256\n"\
                   "MarkForCut|512\n"\
                   "MarkForHarvest|1024\n"\
                   "MarkCropTree|2048\n"\
                   "MarkCropCompetitor|4096\n"\
                   "TreeAffectedBite|8192||");
    columns() << OutputColumn::year() << OutputColumn::ru() << OutputColumn::id() << OutputColumn::species()
            << OutputColumn("id", "id of the tree", OutInteger)
            << OutputColumn("reason", "reason of removal: 0: mortality, 1: management, 2: disturbance ", OutInteger)
            << OutputColumn("x", "position of the tree, x-direction (m)", OutDouble)
            << OutputColumn("y", "position of the tree, y-direction (m)", OutDouble)
            << OutputColumn("dbh", "dbh (cm) of the tree", OutDouble)
            << OutputColumn("height", "height (m) of the tree", OutDouble)
            << OutputColumn("basalArea", "basal area of tree in m2", OutDouble)
            << OutputColumn("volume_m3", "volume of tree (m3)", OutDouble)
            << OutputColumn("age", "tree age (yrs)", OutInteger)
            << OutputColumn("leafArea_m2", "current leaf area of the tree (m2)", OutDouble)
            << OutputColumn("foliageMass", "current mass of foliage (kg)", OutDouble)
            << OutputColumn("stemMass", "kg Biomass in the stem", OutDouble)
            << OutputColumn("branchMass", "kg Biomass in branches", OutDouble)
            << OutputColumn("fineRootMass", "kg Biomass in fine-root department", OutDouble)
            << OutputColumn("coarseRootMass", "kg Biomass in coarse-root department", OutDouble)
            << OutputColumn("lri", "LightResourceIndex of the tree (raw light index from iLand, without applying resource-unit modifications)", OutDouble)
            << OutputColumn("lightResponse", "light response value (including species specific response to the light level)", OutDouble)
            << OutputColumn("stressIndex", "scalar (0..1) indicating the stress level (see [Mortality]).", OutDouble)
            << OutputColumn("reserve_kg", "NPP currently available in the reserve pool (kg Biomass)", OutDouble)
            << OutputColumn("treeFlags", "tree flags (see above)", OutInteger);

}

static QMutex protect_output;
void TreeRemovedOut::execRemovedTree(const Tree *t, int reason)
{
    if (!mFilter.isEmpty()) { // skip trees if filter is present
        TreeWrapper tw;
        mFilter.setModelObject(&tw);
        tw.setTree(t);
        if (!mFilter.executeBool())
            return;
    }
    QMutexLocker protector(&protect_output); // output creation can come from many threads

    *this << currentYear() << t->ru()->index() << t->ru()->id() << t->species()->id();
    *this << t->id()  << reason;
    *this << t->position().x() << t->position().y() << t->dbh() << t->height() << t->basalArea() << t->volume() << t->age();
    *this << t->leafArea() << t->mFoliageMass << t->mStemMass << t->mBranchMass <<  t->mFineRootMass << t->mCoarseRootMass;
    *this << t->lightResourceIndex() << t->mLightResponse << t->mStressIndex << t->mNPPReserve;
    *this << t->flags();
    singleThreadedWriteRow(); // make sure that only one thread writes to the database


}

void TreeRemovedOut::exec()
{
    // do nothing here
    return;
}

void TreeRemovedOut::setup()
{
    QString filter = settings().value(".filter","");
    mFilter.setExpression(filter);
    Tree::setTreeRemovalOutput(this);

}
