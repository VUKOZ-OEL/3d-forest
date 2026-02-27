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

/** @class ResourceUnit
  ResourceUnit is the spatial unit that encapsulates a forest stand and links to several environmental components
  (Climate, Soil, Water, ...).
  @ingroup core
  A resource unit has a size of (currently) 100x100m. Many processes in iLand operate on the level of a ResourceUnit.
  Each resource unit has the same Climate and other properties (e.g. available nitrogen).
  Proceses on this level are, inter alia, NPP Production (see Production3PG), water calculations (WaterCycle), the modeling
  of dead trees (Snag) and soil processes (Soil).

  */
#include <QtCore>

#include "resourceunit.h"
#include "resourceunitspecies.h"
#include "speciesset.h"
#include "species.h"
#include "production3pg.h"
#include "model.h"
#include "climate.h"
#include "watercycle.h"
#include "snag.h"
#include "soil.h"
#include "helper.h"
#include "svdstate.h"
#include "statdata.h"
#include "microclimate.h"

double ResourceUnitVariables::nitrogenAvailableDelta = 0;

ResourceUnit::~ResourceUnit()
{
    if (mWater)
        delete mWater;
    mWater = 0;
    if (mSnag)
        delete mSnag;
    if (mSoil)
        delete mSoil;

    qDeleteAll(mRUSpecies);

    if (mSaplings)
        delete[] mSaplings;

    mSnag = 0;
    mSoil = 0;
    mSaplings = 0;
}

ResourceUnit::ResourceUnit(const int index)
{
    qDeleteAll(mRUSpecies);
    mSpeciesSet = nullptr;
    mClimate = nullptr;
    mPixelCount=0;
    mStockedArea = 0;
    mStockedPixelCount = 0;
    mStockableArea = 0;
    mAggregatedWLA = 0.;
    mAggregatedLA = 0.;
    mAggregatedLR = 0.;
    mEffectiveArea = 0.;
    mLRI_modification = 0.;
    mIndex = index;
    mSaplingHeightMap = 0;
    mMicroclimate = nullptr;
    mEffectiveArea_perWLA = 0.;
    mWater = new WaterCycle();
    mSnag = nullptr;
    mSoil = nullptr;
    mSaplings = nullptr;
    mID = 0;
    mCreateDebugOutput = true;
    mSVDState.clear();
}

void ResourceUnit::setup()
{
    if (mSnag)
        delete mSnag;
    mSnag=nullptr;
    if (mSoil)
        delete mSoil;
    mSoil=nullptr;
    if (Model::settings().carbonCycleEnabled) {
        mSoil = new Soil(this);
        mSnag = new Snag;
        mSnag->setup(this);
        const XmlHelper &xml=GlobalSettings::instance()->settings();

        // setup contents of the soil of the RU; use values for C and N (kg/ha)
        mSoil->setInitialState(CNPool(xml.valueDouble("model.site.youngLabileC", -1),
                                      xml.valueDouble("model.site.youngLabileN", -1),
                                      xml.valueDouble("model.site.youngLabileDecompRate", -1)),
                               CNPool(xml.valueDouble("model.site.youngRefractoryC", -1),
                                      xml.valueDouble("model.site.youngRefractoryN", -1),
                                      xml.valueDouble("model.site.youngRefractoryDecompRate", -1)),
                               CNPair(xml.valueDouble("model.site.somC", -1), xml.valueDouble("model.site.somN", -1)),
                               xml.valueDouble("model.site.youngLabileAbovegroundFraction"),
                               xml.valueDouble("model.site.youngRefractoryAbovegroundFraction"));
    }

    mWater->setup(this);

    if (mSaplings)
        delete mSaplings;
    if (Model::settings().regenerationEnabled) {
        mSaplings = new SaplingCell[cPxPerHectare];
        for (int i=0;i<cPxPerHectare;++i)
            mSaplings[i].ru = this;
    }

    if (Model::settings().microclimateEnabled) {
        mMicroclimate = new Microclimate(this);
    }

    // setup variables
    mUnitVariables.nitrogenAvailable = GlobalSettings::instance()->settings().valueDouble("model.site.availableNitrogen", 40);

    // if dynamic coupling of soil nitrogen is enabled, a starting value for available N is calculated
    if (mSoil && Model::settings().useDynamicAvailableNitrogen && Model::settings().carbonCycleEnabled) {
        mSoil->setClimateFactor(1.);
        mSoil->calculateYear();
        mUnitVariables.nitrogenAvailable = soil()->availableNitrogen();
    }
    mHasDeadTrees = false;
    mAverageAging = 0.;

}
void ResourceUnit::setBoundingBox(const QRectF &bb)
{
    mBoundingBox = bb;
    mCornerOffset = GlobalSettings::instance()->model()->grid()->indexAt(bb.topLeft());
}

/// return the sapling cell at given LIF-coordinates
SaplingCell *ResourceUnit::saplingCell(const QPoint &lifCoords) const
{
    // LIF-Coordinates are global, we here need (RU-)local coordinates
    QPoint po = lifCoords - mCornerOffset;
    int ix = po.x() % cPxPerRU;
    int iy = po.y() % cPxPerRU;
    int i = iy*cPxPerRU+ix;
    Q_ASSERT(i>=0 && i<cPxPerHectare);
    return &mSaplings[i];
}

double ResourceUnit::saplingCoveredArea(bool below130cm) const
{
    Q_ASSERT(mSaplings != 0);
    int n_covered = 0;
    if (below130cm) {
        for (int i=0;i<cPxPerHectare;++i) {
            // either grass *OR* hmax<1.3m
            if (mSaplings[i].state == SaplingCell::CellGrass) {
                ++n_covered;
            } else {
                float hmx = mSaplings[i].max_height();
                if (hmx>0.f && hmx<=1.3f)
                    ++n_covered;
            }
        }
    } else {
        // only px that have saplings > 1.3m
        for (int i=0;i<cPxPerHectare;++i) {
            if (mSaplings[i].max_height()>1.3f)
                ++n_covered;
        }
    }

    return static_cast<double>(n_covered * cPxSize*cPxSize);
}

/// set species and setup the species-per-RU-data
void ResourceUnit::setSpeciesSet(SpeciesSet *set)
{
    mSpeciesSet = set;
    qDeleteAll(mRUSpecies);

    //mRUSpecies.resize(set->count()); // ensure that the vector space is not relocated
    for (int i=0;i<set->count();i++) {
        Species *s = const_cast<Species*>(mSpeciesSet->species(i));
        if (!s)
            throw IException("ResourceUnit::setSpeciesSet: invalid index!");

        ResourceUnitSpecies *rus = new ResourceUnitSpecies();
        mRUSpecies.push_back(rus);
        rus->setup(s, this);
        /* be careful: setup() is called with a pointer somewhere to the content of the mRUSpecies container.
           If the container memory is relocated (QVector), the pointer gets invalid!!!
           Therefore, a resize() is called before the loop (no resize()-operations during the loop)! */
        //mRUSpecies[i].setup(s,this); // setup this element

    }
}

ResourceUnitSpecies &ResourceUnit::resourceUnitSpecies(const Species *species)
{
    return *mRUSpecies[species->index()];
}

const ResourceUnitSpecies *ResourceUnit::constResourceUnitSpecies(const Species *species) const
{
    return mRUSpecies[species->index()];
}

double ResourceUnit::topHeight(bool &rIrregular) const
{
    GridRunner<HeightGridValue> runner(GlobalSettings::instance()->model()->heightGrid(), boundingBox());
    int valid=0, total=0;
    QVector<double> px_heights;
    px_heights.reserve(cHeightPerRU*cHeightPerRU);
    while (runner.next()) {
        if ( runner.current()->isValid() ) {
            valid++;
            px_heights.push_back(runner.current()->stemHeight());
        }
        total++;
    }
    StatData hstat(px_heights);
    double h_top = hstat.percentile(90);
    double h_median = hstat.median();
    // irregular: 50% of the area < 50% topheight: median=50% of the area
    if (h_median < h_top*0.5)
        rIrregular = true;
    else
        rIrregular = false;
    return h_top;
}

void ResourceUnit::notifyDisturbance(ERUDisturbanceType source, double info) const
{
    if (!mSVDState.disturbanceEvents) // do nothing if SVD states are not used
        return;

    // events are stored with newest events first. Oldest event is removed
    // when maximum number of events reached
    mSVDState.disturbanceEvents->prepend(RUSVDState::SVDDisturbanceEvent(
                                            Globals->currentYear(),
                                            source,
                                            info));
    if (mSVDState.disturbanceEvents->size() > 3 )
        mSVDState.disturbanceEvents->pop_back();
}

Tree &ResourceUnit::newTree()
{
    // start simple: just append to the vector...
    if (mTrees.isEmpty())
        mTrees.reserve(100); // reserve a junk of memory for trees

    mTrees.append(Tree());
    return mTrees.back();
}
int ResourceUnit::newTreeIndex()
{
    newTree();
    return mTrees.count()-1; // return index of the last tree
}

/// remove dead trees from tree list
/// reduce size of vector if lots of space is free
/// tests showed that this way of cleanup is very fast,
/// because no memory allocations are performed (simple memmove())
/// when trees are moved.
void ResourceUnit::cleanTreeList()
{
    if (!mHasDeadTrees)
        return;

    QVector<Tree>::iterator last=mTrees.end()-1;
    QVector<Tree>::iterator current = mTrees.begin();
    while (last>=current && (*last).isDead())
        --last;

    while (current<last) {
        if ((*current).isDead()) {
            *current = *last; // copy data!
            --last; //
            while (last>=current && (*last).isDead())
                --last;
        }
        ++current;
    }
    ++last; // last points now to the first dead tree

    // free ressources
    if (last!=mTrees.end()) {
        mTrees.erase(last, mTrees.end());
        if (mTrees.capacity()>100) {
            if (mTrees.count() / double(mTrees.capacity()) < 0.2) {
                //int target_size = mTrees.count()*2;
                //qDebug() << "reduce size from "<<mTrees.capacity() << "to" << target_size;
                //mTrees.reserve(qMax(target_size, 100));
                if (logLevelDebug())
                    qDebug() << "reduce tree storage of RU" << index() << " from " << mTrees.capacity() << "to" << mTrees.count();
                mTrees.squeeze();
            }
        }
    }
    mHasDeadTrees = false; // reset flag
}

void ResourceUnit::newYear()
{
    mAggregatedWLA = 0.;
    mAggregatedLA = 0.;
    mAggregatedLR = 0.;
    mEffectiveArea = 0.;
    mPixelCount = mStockedPixelCount = 0;

    if (GlobalSettings::instance()->model()->ruList().first()==this && GlobalSettings::instance()->settings().hasNode("model.site.deltaAvailableNitrogen")) {
        mUnitVariables.nitrogenAvailableDelta = GlobalSettings::instance()->settings().valueDouble("model.site.deltaAvailableNitrogen",0.);
        if (mUnitVariables.nitrogenAvailableDelta != 0.)
            qDebug() << "applying a global delta to available Nitrogen:" << mUnitVariables.nitrogenAvailableDelta << "kg N/ha/yr";
    }

    snagNewYear();
    if (mSoil)
        mSoil->newYear();
    // clear statistics global and per species...
    QList<ResourceUnitSpecies*>::const_iterator i;
    QList<ResourceUnitSpecies*>::const_iterator iend = mRUSpecies.constEnd();
    mStatistics.clear();
    for (i=mRUSpecies.constBegin(); i!=iend; ++i) {
        (*i)->statisticsDead().clear();
        (*i)->statisticsMgmt().clear();
    }



}

/** production() is the "stand-level" part of the biomass production (3PG).
    - The amount of radiation intercepted by the stand is calculated
    - the water cycle is calculated
    - statistics for each species are cleared
    - The 3PG production for each species and ressource unit is called (calculates species-responses and NPP production)
    see also: https://iland-model.org/individual+tree+light+availability */
void ResourceUnit::production()
{

    if (mAggregatedWLA==0. || mPixelCount==0) {
        // clear statistics of resourceunitspecies
        for ( QList<ResourceUnitSpecies*>::const_iterator i=mRUSpecies.constBegin(); i!=mRUSpecies.constEnd(); ++i) {
            (*i)->statistics().clear();
        }
        mEffectiveArea = 0.;
        mStockedArea = 0.;
        return;
    }

    // the pixel counters are filled during the height-grid-calculations
    mStockedArea = cHeightPerRU*cHeightPerRU * mStockedPixelCount; // m2 (1 height grid pixel = 10x10m)
    if (leafAreaIndex()<3.) {
        // estimate stocked area based on crown projections
        double crown_area = 0.;
        for (int i=0;i<mTrees.count();++i)
            crown_area += mTrees.at(i).isDead() ? 0. : mTrees.at(i).stamp()->reader()->crownArea();

        if (logLevelDebug())
            qDebug() << "crown area: lai" << leafAreaIndex() << "stocked area (pixels)" << mStockedArea << " area (crown)" << crown_area;
        if (leafAreaIndex()<1.) {
            mStockedArea = std::min(crown_area, mStockedArea);
        } else {
            // for LAI between 1 and 3:
            // interpolate between sum of crown area of trees (at LAI=1) and the pixel-based value (at LAI=3 and above)
            double px_frac = (leafAreaIndex()-1.)/2.; // 0 at LAI=1, 1 at LAI=3
            mStockedArea = mStockedArea * px_frac + std::min(crown_area, mStockedArea) * (1. - px_frac);
        }
        if (mStockedArea==0.)
            return;
    }

    // calculate the leaf area index (LAI)
    double LAI = mAggregatedLA / mStockedArea;
    // calculate the intercepted radiation fraction using the law of Beer Lambert
    const double k = Model::settings().lightExtinctionCoefficient;
    double interception_fraction = 1. - exp(-k * LAI);
    mEffectiveArea = mStockedArea * interception_fraction; // m2

    // calculate the total weighted leaf area on this RU:
    mLRI_modification = interception_fraction *  mStockedArea / mAggregatedWLA; // p_WLA
    if (mLRI_modification == 0.)
        qDebug() << "lri modifaction==0!";

    if (logLevelDebug()) {
    DBGMODE(qDebug() << QString("production: LAI: %1 (intercepted fraction: %2, stocked area: %4). LRI-Multiplier: %3")
            .arg(LAI)
            .arg(interception_fraction)
            .arg(mLRI_modification)
            .arg(mStockedArea);
    );
    }

    // calculate LAI fractions
    QList<ResourceUnitSpecies*>::const_iterator i;
    QList<ResourceUnitSpecies*>::const_iterator iend = mRUSpecies.constEnd();

    // soil water model - this determines soil water contents needed for response calculations
    mWater->run();

    // invoke species specific calculation (3PG)
    for (i=mRUSpecies.constBegin(); i!=iend; ++i) {

        (*i)->calculate(); // CALCULATE 3PG

        // debug output related to production
        if (GlobalSettings::instance()->isDebugEnabled(GlobalSettings::dStandGPP) && shouldCreateDebugOutput() && (*i)->leafAreaIndex()>0.) {
            DebugList &out = GlobalSettings::instance()->debugList(index(), GlobalSettings::dStandGPP);
            out << (*i)->species()->id() << index() << id();
            out << (*i)->leafAreaIndex() << (*i)->prod3PG().GPPperArea() << productiveArea()*(*i)->leafAreaIndex()/(leafAreaIndex()==0.?1.:leafAreaIndex()) *(*i)->prod3PG().GPPperArea() << averageAging() << (*i)->prod3PG().fEnvYear() ;

        }
    }
}

void ResourceUnit::calculateInterceptedArea()
{
    if (mAggregatedLR==0) {
        mEffectiveArea_perWLA = 0.;
        return;
    }
    Q_ASSERT(mAggregatedLR>0.);
    mEffectiveArea_perWLA = mEffectiveArea / mAggregatedLR;
    if (logLevelDebug()) qDebug() << "RU: aggregated lightresponse:" << mAggregatedLR  << "eff.area./wla:" << mEffectiveArea_perWLA;
}

// function is called immediately before the growth of individuals
void ResourceUnit::beforeGrow()
{
    mAverageAging = 0.;
}

// function is called after finishing the indivdual growth / mortality.
void ResourceUnit::afterGrow()
{
    mAverageAging = leafArea()>0.?mAverageAging/leafArea():0; // calculate aging value (calls to addAverageAging() by individual trees)
    if (mAverageAging>0. && mAverageAging<0.00001)
        qDebug() << "ru" << mIndex << "aging <0.00001";
    if (mAverageAging<0. || mAverageAging>1.)
        qDebug() << "Average aging invalid: (RU, LAI):" << index() << mStatistics.leafAreaIndex();
}

void ResourceUnit::yearEnd()
{
    // calculate statistics for all tree species of the ressource unit
    int c = mRUSpecies.count();
    for (int i=0;i<c; i++) {
        mRUSpecies[i]->statisticsDead().calculate(); // calculate the dead trees
        mRUSpecies[i]->statisticsMgmt().calculate(); // stats of removed trees
        mRUSpecies[i]->updateGWL(); // get sum of dead trees (died + removed)
        mRUSpecies[i]->statistics().calculate(); // calculate the living (and add removed volume to gwl)
        mStatistics.add(mRUSpecies[i]->statistics());
    }
    mStatistics.calculate(); // aggreagte on stand level

    // update carbon flows
    if (soil() && GlobalSettings::instance()->model()->settings().carbonCycleEnabled) {
        double area_factor = stockableArea() / cRUArea; //conversion factor
        mUnitVariables.carbonUptake = statistics().npp() * biomassCFraction;
        mUnitVariables.carbonUptake += statistics().nppSaplings() * biomassCFraction;

        double to_atm = snag()->fluxToAtmosphere().C / area_factor; // from snags, kgC/ha
        to_atm += soil()->fluxToAtmosphere().C *cRUArea/10.; // soil: t/ha -> t/m2 -> kg/ha
        mUnitVariables.carbonToAtm = to_atm;

        double to_dist = snag()->fluxToDisturbance().C / area_factor;
        to_dist += soil()->fluxToDisturbance().C * cRUArea/10.;
        double to_harvest = snag()->fluxToExtern().C / area_factor;

        mUnitVariables.NEP = mUnitVariables.carbonUptake - to_atm - to_dist - to_harvest; // kgC/ha

        // incremental values....
        mUnitVariables.cumCarbonUptake += mUnitVariables.carbonUptake;
        mUnitVariables.cumCarbonToAtm += mUnitVariables.carbonToAtm;
        mUnitVariables.cumNEP += mUnitVariables.NEP;

    }

    // SVD States: update state
    updateSVDState();


}

void ResourceUnit::updateSVDState()
{
    if (GlobalSettings::instance()->model()->svdStates()){
        if (!mSVDState.localComposition) {
            // create vectors on the heap only when really needed
            int nspecies = GlobalSettings::instance()->model()->speciesSet()->activeSpecies().size();
            mSVDState.localComposition = new QVector<float>(nspecies, 0.f);
            mSVDState.midDistanceComposition = new QVector<float>(nspecies, 0.f);
            // create history vector
            mSVDState.disturbanceEvents = new QVector<RUSVDState::SVDDisturbanceEvent>();
        }
        int stateId=GlobalSettings::instance()->model()->svdStates()->evaluateState(this);
        if (mSVDState.stateId==stateId)
            mSVDState.time++;
        else {
            mSVDState.previousTime = mSVDState.time;
            mSVDState.previousStateId = mSVDState.stateId;
            mSVDState.stateId=stateId;
            mSVDState.time=1;
        }

    }

}

void ResourceUnit::addTreeAgingForAllTrees()
{
    mAverageAging = 0.;
    foreach(const Tree &t, mTrees) {
        addTreeAging(t.leafArea(), t.species()->aging(t.height(), t.age()));
    }

}

/// refresh of tree based statistics.
/// WARNING: this function is only called once (during startup).
/// see function "yearEnd()" above!!!
void ResourceUnit::createStandStatistics()
{
    // clear statistics (ru-level and ru-species level)
    mStatistics.clear();
    for (int i=0;i<mRUSpecies.count();i++) {
        mRUSpecies[i]->statistics().clear();
        mRUSpecies[i]->statisticsDead().clear();
        mRUSpecies[i]->statisticsMgmt().clear();
        mRUSpecies[i]->saplingStat().clearStatistics();
    }

    // add all trees to the statistics objects of the species
    foreach(const Tree &t, mTrees) {
        if (!t.isDead())
            resourceUnitSpecies(t.species()).statistics().add(&t, 0);
    }
    // summarise sapling stats
    GlobalSettings::instance()->model()->saplings()->calculateInitialStatistics(this);

    // summarize statistics for the whole resource unit
    for (int i=0;i<mRUSpecies.count();i++) {
        mRUSpecies[i]->saplingStat().calculate(mRUSpecies[i]->species(), this);
        mRUSpecies[i]->statistics().add(&mRUSpecies[i]->saplingStat());
        mRUSpecies[i]->statistics().calculate();
        mStatistics.add(mRUSpecies[i]->statistics());
    }
    mStatistics.calculate();
    mAverageAging = mStatistics.leafAreaIndex()>0.?mAverageAging / (mStatistics.leafAreaIndex()*stockableArea()):0.;
    if (mAverageAging<0. || mAverageAging>1.)
        qDebug() << "Average aging invalid: (RU, LAI):" << index() << mStatistics.leafAreaIndex();

    updateSVDState(); // initial state (if SVD enabled)
}

/** recreate statistics. This is necessary after events that changed the structure
    of the stand *after* the growth of trees (where stand statistics are updated).
    An example is after disturbances.  */
void ResourceUnit::recreateStandStatistics(bool recalculate_stats)
{
    // when called after disturbances (recalculate_stats=false), we
    // clear only the tree-specific variables in the stats (i.e. we keep NPP, and regen carbon),
    // and then re-add all trees (since TreeGrowthData is NULL no NPP is available).
    // The statistics are not summarised here, because this happens for all resource units
    // in the yearEnd function of RU.
    for (int i=0;i<mRUSpecies.count();i++) {
        if (recalculate_stats)
            mRUSpecies[i]->statistics().clear();
        else
            mRUSpecies[i]->statistics().clearOnlyTrees();
    }
    foreach(const Tree &t, mTrees) {
        resourceUnitSpecies(t.species()).statistics().add(&t, 0);
    }

    if (recalculate_stats) {
        for (int i=0;i<mRUSpecies.count();i++) {
            mRUSpecies[i]->statistics().calculate();
        }
    }
}

void ResourceUnit::analyzeMicroclimate()
{
    if (mMicroclimate)
        mMicroclimate->calculateVegetation();
}




void ResourceUnit::calculateCarbonCycle()
{
    if (!snag())
        return;

    // (1) calculate the snag dynamics
    // because all carbon/nitrogen-flows from trees to the soil are routed through the snag-layer,
    // all soil inputs (litter + deadwood) are collected in the Snag-object.
    snag()->calculateYear();

    soil()->setClimateFactor( snag()->climateFactor() ); // the climate factor is only calculated once

    soil()->setSoilInput( snag()->labileFlux(), snag()->refractoryFlux(),
                          snag()->labileFluxAbovegroundCarbon(), snag()->refractoryFluxAbovegroundCarbon());

    soil()->calculateYear(); // update the ICBM/2N model
    // use available nitrogen?
    if (Model::settings().useDynamicAvailableNitrogen)
        mUnitVariables.nitrogenAvailable = soil()->availableNitrogen();

    // debug output
    if (GlobalSettings::instance()->isDebugEnabled(GlobalSettings::dCarbonCycle) && shouldCreateDebugOutput() && !snag()->isEmpty()) {
        DebugList &out = GlobalSettings::instance()->debugList(index(), GlobalSettings::dCarbonCycle);
        out << index() << id(); // resource unit index and id
        out << snag()->debugList(); // snag debug outs
        out << soil()->debugList(); // ICBM/2N debug outs
    }

}


