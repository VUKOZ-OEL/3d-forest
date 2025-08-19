/********************************************************************************************
**    iLand - an individual based forest landscape and disturbance model
**    http://iland.boku.ac.at
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

#include "sapling.h"
#include "model.h"
#include "species.h"
#include "resourceunit.h"
#include "resourceunitspecies.h"
#include "tree.h"

/** @class Sapling
  @ingroup core
    Sapling stores saplings per species and resource unit and computes sapling growth (before recruitment).
    http://iland.boku.ac.at/sapling+growth+and+competition
    Saplings are established in a separate step (@sa Regeneration). If sapling reach a height of 4m, they are recruited and become "real" iLand-trees.
    Within the regeneration layer, a cohort-approach is applied.

  */

double Sapling::mRecruitmentVariation = 0.1; // +/- 10%
double Sapling::mBrowsingPressure = 0.;

Sapling::Sapling()
{
    mRUS = 0;
    clearStatistics();
    mAdded = 0;
}

// reset statistics, called at newYear
void Sapling::clearStatistics()
{
    // mAdded: removed
    mRecruited=mDied=mLiving=0;
    mSumDbhDied=0.;
    mAvgHeight=0.;
    mAvgAge=0.;
    mAvgDeltaHPot=mAvgHRealized=0.;
}

void Sapling::updateBrowsingPressure()
{
    if (GlobalSettings::instance()->settings().valueBool("model.settings.browsing.enabled"))
        Sapling::mBrowsingPressure = GlobalSettings::instance()->settings().valueDouble("model.settings.browsing.browsingPressure");
    else
        Sapling::mBrowsingPressure = 0.;
}

/// get the *represented* (Reineke's Law) number of trees (N/ha)
double Sapling::livingStemNumber(double &rAvgDbh, double &rAvgHeight, double &rAvgAge) const
{
    double total = 0.;
    double dbh_sum = 0.;
    double h_sum = 0.;
    double age_sum = 0.;
    const SaplingGrowthParameters &p = mRUS->species()->saplingGrowthParameters();
    for (QVector<SaplingTreeOld>::const_iterator it = mSaplingTrees.constBegin(); it!=mSaplingTrees.constEnd(); ++it) {
        float dbh = it->height / p.hdSapling * 100.f;
        if (dbh<1.) // minimum size: 1cm
            continue;
        double n = p.representedStemNumber(dbh); // one cohort on the pixel represents that number of trees
        dbh_sum += n*dbh;
        h_sum += n*it->height;
        age_sum += n*it->age.age;
        total += n;
    }
    if (total>0.) {
        dbh_sum /= total;
        h_sum /= total;
        age_sum /= total;
    }
    rAvgDbh = dbh_sum;
    rAvgHeight = h_sum;
    rAvgAge = age_sum;
    return total;
}

double Sapling::representedStemNumber(float height) const
{
    const SaplingGrowthParameters &p = mRUS->species()->saplingGrowthParameters();
    float dbh = height / p.hdSapling * 100.f;
    double n = p.representedStemNumber(dbh);
    return n;
}

/// maintenance function to clear dead/recruited saplings from storage
void Sapling::cleanupStorage()
{
    QVector<SaplingTreeOld>::iterator forw=mSaplingTrees.begin();
    QVector<SaplingTreeOld>::iterator back;

    // seek last valid
    for (back=mSaplingTrees.end()-1; back>=mSaplingTrees.begin(); --back)
        if ((*back).isValid())
            break;

    if (back<mSaplingTrees.begin()) {
        mSaplingTrees.clear(); // no valid trees available
        return;
    }

    while (forw < back) {
        if (!(*forw).isValid()) {
            *forw = *back; // copy (fill gap)
            while (back>forw) // seek next valid
                if ((*--back).isValid())
                    break;
        }
        ++forw;
    }
    if (back != mSaplingTrees.end()-1) {
        // free resources...
        mSaplingTrees.erase(back+1, mSaplingTrees.end());
    }
}

// not a very good way of checking if sapling is present
// maybe better: use also a (local) maximum sapling height grid
// maybe better: use a bitset:
// position: index of pixel on LIF (absolute index)
bool Sapling::hasSapling(const QPoint &position) const
{
    const QPoint &offset = mRUS->ru()->cornerPointOffset();
    int index = (position.x()- offset.x())*cPxPerRU + (position.y() - offset.y());
    if (index<0)
        qDebug() << "Sapling error";
    return mSapBitset[index];
    /*
    float *target = GlobalSettings::instance()->model()->grid()->ptr(position.x(), position.y());
    QVector<SaplingTree>::const_iterator it;
    for (it = mSaplingTrees.constBegin(); it!=mSaplingTrees.constEnd(); ++it) {
        if (it->pixel==target)
            return true;
    }
    return false;
    */
}

/// retrieve the height of the sapling at the location 'position' (given in LIF-coordinates)
/// this is quite expensive and only done for initialization
double Sapling::heightAt(const QPoint &position) const
{
    if (!hasSapling(position))
        return 0.;
    // ok, we'll have to search through all saplings
    QVector<SaplingTreeOld>::const_iterator it;
    float *lif_ptr = GlobalSettings::instance()->model()->grid()->ptr(position.x(), position.y());
    for (it = mSaplingTrees.constBegin(); it!=mSaplingTrees.constEnd(); ++it) {
        if (it->isValid() && it->pixel == lif_ptr)
            return it->height;
    }
    return 0.;

}


void Sapling::setBit(const QPoint &pos_index, bool value)
{
    int index = (pos_index.x() - mRUS->ru()->cornerPointOffset().x()) * cPxPerRU +(pos_index.y() - mRUS->ru()->cornerPointOffset().y());
    mSapBitset.set(index,value); // set bit: now there is a sapling there
}

/// add a sapling at the given position (index on the LIF grid, i.e. 2x2m)
int Sapling::addSapling(const QPoint &pos_lif, const float height, const int age)
{
    // adds a sapling...
    mSaplingTrees.push_back(SaplingTreeOld());
    SaplingTreeOld &t = mSaplingTrees.back();
    t.height = height; // default is 5cm height
    t.age.age = age;
    Grid<float> &lif_map = *GlobalSettings::instance()->model()->grid();
    t.pixel = lif_map.ptr(pos_lif.x(), pos_lif.y());
    setBit(pos_lif, true);
    mAdded++;
    return mSaplingTrees.count()-1; // index of the newly added tree.
}

/// clear  saplings on a given position (after recruitment)
void Sapling::clearSaplings(const QPoint &position)
{
    float *target = GlobalSettings::instance()->model()->grid()->ptr(position.x(), position.y());
    QVector<SaplingTreeOld>::const_iterator it;
    for (it = mSaplingTrees.constBegin(); it!=mSaplingTrees.constEnd(); ++it) {
        if (it->pixel==target) {
            // trick: use a const iterator to avoid a deep copy of the vector; then do an ugly const_cast to actually write the data
            //const SaplingTree &t = *it;
            //const_cast<SaplingTree&>(t).pixel=0;
            clearSapling(const_cast<SaplingTreeOld&>(*it), false); // kill sapling and move carbon to soil
        }
    }
    setBit(position, false); // clear bit: now there is no sapling on this position
    //int index = (position.x() - mRUS->ru()->cornerPointOffset().x()) * cPxPerRU +(position.y() - mRUS->ru()->cornerPointOffset().y());
    //mSapBitset.set(index,false); // clear bit: now there is no sapling on this position

}

/// clear  saplings within a given rectangle
void Sapling::clearSaplings(const QRectF &rectangle, const bool remove_biomass)
{
    QVector<SaplingTreeOld>::const_iterator it;
    FloatGrid *grid = GlobalSettings::instance()->model()->grid();
    for (it = mSaplingTrees.constBegin(); it!=mSaplingTrees.constEnd(); ++it) {
        if (rectangle.contains(grid->cellCenterPoint(it->coords()))) {
            clearSapling(const_cast<SaplingTreeOld&>(*it), remove_biomass);
        }
    }
}

void Sapling::clearSapling(SaplingTreeOld &tree, const bool remove)
{
    QPoint p=tree.coords();
    tree.pixel=0;
    setBit(p, false); // no tree left
    if (!remove) {
        // killing of saplings:
        // if remove=false, then remember dbh/number of trees (used later in calculateGrowth() to estimate carbon flow)
        mDied++;
        mSumDbhDied+=tree.height / mRUS->species()->saplingGrowthParameters().hdSapling * 100.;
    }

}

//
void Sapling::clearSapling(int index, const bool remove)
{
    Q_ASSERT(index < mSaplingTrees.count());
    clearSapling(mSaplingTrees[index], remove);
}

/// growth function for an indivudal sapling.
/// returns true, if sapling survives, false if sapling dies or is recruited to iLand.
/// see also http://iland.boku.ac.at/recruitment
bool Sapling::growSapling(SaplingTreeOld &tree, const double f_env_yr, Species* species)
{
    QPoint p=GlobalSettings::instance()->model()->grid()->indexOf(tree.pixel);
    //GlobalSettings::instance()->model()->heightGrid()[Grid::index5(tree.pixel-GlobalSettings::instance()->model()->grid()->begin())];

    // (1) calculate height growth potential for the tree (uses linerization of expressions...)
    double h_pot = species->saplingGrowthParameters().heightGrowthPotential.calculate(tree.height); // TODO check if this can be source of crashes (race condition)
    double delta_h_pot = h_pot - tree.height;

    // (2) reduce height growth potential with species growth response f_env_yr and with light state (i.e. LIF-value) of home-pixel.
    double lif_value = *tree.pixel;
    double h_height_grid = GlobalSettings::instance()->model()->heightGrid()->valueAtIndex(p.x()/cPxPerHeight, p.y()/cPxPerHeight).height;
    if (h_height_grid==0.)
        throw IException(QString("growSapling: height grid at %1/%2 has value 0").arg(p.x()).arg(p.y()));

    double rel_height = tree.height / h_height_grid;

    double lif_corrected = mRUS->species()->speciesSet()->LRIcorrection(lif_value, rel_height); // correction based on height

    double lr = mRUS->species()->lightResponse(lif_corrected); // species specific light response (LUI, light utilization index)

    double delta_h_factor = f_env_yr * lr; // relative growth

    if (h_pot<0. || delta_h_pot<0. || lif_corrected<0. || lif_corrected>1. || delta_h_factor<0. || delta_h_factor>1. )
        qDebug() << "invalid values in Sapling::growSapling";

    // check browsing
    if (mBrowsingPressure>0. && tree.height<=2.f) {
        double p = mRUS->species()->saplingGrowthParameters().browsingProbability;
        // calculate modifed annual browsing probability via odds-ratios
        // odds = p/(1-p) -> odds_mod = odds * browsingPressure -> p_mod = odds_mod /( 1 + odds_mod) === p*pressure/(1-p+p*pressure)
        double p_browse = p*mBrowsingPressure / (1. - p + p*mBrowsingPressure);
        if (drandom() < p_browse) {
            delta_h_factor = 0.;
        }
    }

    // check mortality of saplings
    if (delta_h_factor < species->saplingGrowthParameters().stressThreshold) {
        tree.age.stress_years++;
        if (tree.age.stress_years > species->saplingGrowthParameters().maxStressYears) {
            // sapling dies...
            clearSapling(tree, false); // false: put carbon to the soil
            return false;
        }
    } else {
        tree.age.stress_years=0; // reset stress counter
    }
    DBG_IF(delta_h_pot*delta_h_factor < 0.f || delta_h_pot*delta_h_factor > 2., "Sapling::growSapling", "inplausible height growth.");

    // grow
    tree.height += delta_h_pot * delta_h_factor;
    tree.age.age++; // increase age of sapling by 1

    // recruitment?
    if (tree.height > 4.f) {
        mRecruited++;

        ResourceUnit *ru = const_cast<ResourceUnit*> (mRUS->ru());
        float dbh = tree.height / species->saplingGrowthParameters().hdSapling * 100.f;
        // the number of trees to create (result is in trees per pixel)
        double n_trees = species->saplingGrowthParameters().representedStemNumber(dbh);
        int to_establish = (int) n_trees;

        // if n_trees is not an integer, choose randomly if we should add a tree.
        // e.g.: n_trees = 2.3 -> add 2 trees with 70% probability, and add 3 trees with p=30%.
        if (drandom() < (n_trees-to_establish) || to_establish==0)
            to_establish++;

        // add a new tree
        for (int i=0;i<to_establish;i++) {
            Tree &bigtree = ru->newTree();
            bigtree.setPosition(p);
            // add variation: add +/-10% to dbh and *independently* to height.
            bigtree.setDbh(dbh * nrandom(1. - mRecruitmentVariation, 1. + mRecruitmentVariation));
            bigtree.setHeight(tree.height * nrandom(1. - mRecruitmentVariation, 1. + mRecruitmentVariation));
            bigtree.setSpecies( species );
            bigtree.setAge(tree.age.age,tree.height);
            bigtree.setRU(ru);
            bigtree.setup();
            const Tree *t = &bigtree;
            mRUS->statistics().add(t, 0); // count the newly created trees already in the stats
        }
        // clear all regeneration from this pixel (including this tree)
        clearSapling(tree, true); // remove this tree (but do not move biomass to soil)
//        ru->clearSaplings(p); // remove all other saplings on the same pixel

        return false;
    }
    // book keeping (only for survivors)
    mLiving++;
    mAvgHeight+=tree.height;
    mAvgAge+=tree.age.age;
    mAvgDeltaHPot+=delta_h_pot;
    mAvgHRealized += delta_h_pot * delta_h_factor;
    return true;

}


/** main growth function for saplings.
    Statistics are cleared at the beginning of the year.
*/
void Sapling::calculateGrowth()
{
    Q_ASSERT(mRUS);
    if (mSaplingTrees.count()==0)
        return;

    ResourceUnit *ru = const_cast<ResourceUnit*> (mRUS->ru() );
    Species *species = const_cast<Species*>(mRUS->species());

    // calculate necessary growth modifier (this is done only once per year)
    mRUS->calculate(true); // calculate the 3pg module (this is done only if that did not happen up to now); true: call comes from regeneration
    double f_env_yr = mRUS->prod3PG().fEnvYear();

    mLiving=0;
    QVector<SaplingTreeOld>::const_iterator it;
    for (it = mSaplingTrees.constBegin(); it!=mSaplingTrees.constEnd(); ++it) {
        const SaplingTreeOld &tree = *it;
        if (tree.height<0)
            qDebug() << "Sapling::calculateGrowth(): h<0";
        // if sapling is still living check execute growth routine
        if (tree.isValid()) {
            // growing (increases mLiving if tree did not die, mDied otherwise)
            if (growSapling(const_cast<SaplingTreeOld&>(tree), f_env_yr, species)) {
                // set the sapling height to the maximum value on the current pixel
//                ru->setMaxSaplingHeightAt(tree.coords(),tree.height);
            }
        }
    }
    if (mLiving) {
        mAvgHeight /= double(mLiving);
        mAvgAge /= double(mLiving);
        mAvgDeltaHPot /= double(mLiving);
        mAvgHRealized /= double(mLiving);
    }
    // calculate carbon balance
    CNPair old_state = mCarbonLiving;
    mCarbonLiving.clear();

    CNPair dead_wood, dead_fine; // pools for mortality
    // average dbh
    if (mLiving) {
        // calculate the avg dbh and number of stems
        double avg_dbh = mAvgHeight / species->saplingGrowthParameters().hdSapling * 100.;
        double n = mLiving * species->saplingGrowthParameters().representedStemNumber( avg_dbh );
        // woody parts: stem, branchse and coarse roots
        double woody_bm = species->biomassWoody(avg_dbh) + species->biomassBranch(avg_dbh) + species->biomassRoot(avg_dbh);
        double foliage = species->biomassFoliage(avg_dbh);
        double fineroot = foliage*species->finerootFoliageRatio();

        mCarbonLiving.addBiomass( woody_bm*n, species->cnWood()  );
        mCarbonLiving.addBiomass( foliage*n, species->cnFoliage()  );
        mCarbonLiving.addBiomass( fineroot*n, species->cnFineroot()  );

        // turnover
        if (mRUS->ru()->snag())
            mRUS->ru()->snag()->addTurnoverLitter(species, foliage*species->turnoverLeaf(), fineroot*species->turnoverRoot());

        // calculate the "mortality from competition", i.e. carbon that stems from reduction of stem numbers
        // from Reinekes formula.
        //
        if (avg_dbh>1.) {
            double avg_dbh_before = (mAvgHeight - mAvgHRealized) / species->saplingGrowthParameters().hdSapling * 100.;
            double n_before = mLiving * species->saplingGrowthParameters().representedStemNumber( qMax(1.,avg_dbh_before) );
            if (n<n_before) {
                dead_wood.addBiomass( woody_bm * (n_before-n), species->cnWood() );
                dead_fine.addBiomass( foliage * (n_before-n), species->cnFoliage()  );
                dead_fine.addBiomass( fineroot * (n_before-n), species->cnFineroot()  );
            }
        }

    }
    if (mDied) {
        double avg_dbh_dead = mSumDbhDied / double(mDied);
        double n = mDied * species->saplingGrowthParameters().representedStemNumber( avg_dbh_dead );
        // woody parts: stem, branchse and coarse roots

        dead_wood.addBiomass( ( species->biomassWoody(avg_dbh_dead) + species->biomassBranch(avg_dbh_dead) + species->biomassRoot(avg_dbh_dead)) * n, species->cnWood()  );
        double foliage = species->biomassFoliage(avg_dbh_dead)*n;

        dead_fine.addBiomass( foliage, species->cnFoliage()  );
        dead_fine.addBiomass( foliage*species->finerootFoliageRatio(), species->cnFineroot()  );
    }
    if (!dead_wood.isEmpty() || !dead_fine.isEmpty())
        if (mRUS->ru()->snag())
            mRUS->ru()->snag()->addToSoil(species, dead_wood, dead_fine);

    // calculate net growth:
    // delta of stocks
    mCarbonGain = mCarbonLiving + dead_fine + dead_wood - old_state;
    if (mCarbonGain.C < 0)
        mCarbonGain.clear();

    if (mSaplingTrees.count() > mLiving*1.3)
        cleanupStorage();

//    mRUS->statistics().add(this);
    GlobalSettings::instance()->systemStatistics()->saplingCount+=mLiving;
    GlobalSettings::instance()->systemStatistics()->newSaplings+=mAdded;
    mAdded = 0; // reset

    //qDebug() << ru->index() << species->id()<< ": (living/avg.height):" <<  mLiving << mAvgHeight;
}

/// fill a grid with the maximum height of saplings per pixel (2x2m).
/// this function is used for visualization only
void Sapling::fillMaxHeightGrid(Grid<float> &grid) const
{
    QVector<SaplingTreeOld>::const_iterator it;
    for (it = mSaplingTrees.begin(); it!=mSaplingTrees.end(); ++it) {
        if (it->isValid()) {
             QPoint p=it->coords();
             if (grid.valueAtIndex(p)<it->height)
                 grid.valueAtIndex(p) = it->height;
        }
    }

}




