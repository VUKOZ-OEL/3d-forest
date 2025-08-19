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
#include "global.h"
#include "saplings.h"

#include "globalsettings.h"
#include "model.h"
#include "resourceunit.h"
#include "resourceunitspecies.h"
#include "establishment.h"
#include "species.h"
#include "seeddispersal.h"
#include "mapgrid.h"
#include "grasscover.h"

double Saplings::mRecruitmentVariation = 0.1; // +/- 10%
double Saplings::mBrowsingPressure = 0.;


Saplings::Saplings()
{

}

void Saplings::setup()
{
    //mGrid.setup(GlobalSettings::instance()->model()->grid()->metricRect(), GlobalSettings::instance()->model()->grid()->cellsize());
    FloatGrid *lif_grid = GlobalSettings::instance()->model()->grid();
    // mask out out-of-project areas
    HeightGrid *hg = GlobalSettings::instance()->model()->heightGrid();
    for (int i=0; i<lif_grid->count(); ++i) {
        SaplingCell *s = cell(lif_grid->indexOf(i), false); // false: retrieve also invalid cells
        if (s) {
            if (!hg->valueAtIndex(lif_grid->index5(i)).isValid())
                s->state = SaplingCell::CellInvalid;
            else
                s->state = SaplingCell::CellEmpty;
        }

    }

}

void Saplings::calculateInitialStatistics(const ResourceUnit *ru)
{
    SaplingCell *sap_cells = ru->saplingCellArray();
    if (!sap_cells)
        return;

    SaplingCell *s = sap_cells;

    for (int i=0; i<cPxPerHectare; ++i, ++s) {
        if (s->state != SaplingCell::CellInvalid) {
            int cohorts_on_px = s->n_occupied();
            for (int j=0;j<NSAPCELLS;++j) {
                if (s->saplings[j].is_occupied()) {
                    SaplingTree &tree=s->saplings[j];
                    ResourceUnitSpecies *rus = tree.resourceUnitSpecies(ru);
                    rus->saplingStat().mLiving++;
                    float n_repr = static_cast<float>( rus->species()->saplingGrowthParameters().representedStemNumberH(tree.height) / static_cast<double>(cohorts_on_px) );
                    if (tree.height>1.3f)
                        rus->saplingStat().mLivingSaplings += n_repr;
                    else
                        rus->saplingStat().mLivingSmallSaplings += n_repr;

                    rus->saplingStat().mAvgHeight+=tree.height;
                    rus->saplingStat().mAvgAge+=tree.age;

                }
            }
        }
    }


}

/// establishment of saplings from seeds
/// see https://iland-model.org/seed+kernel+and+seed+distribution and https://iland-model.org/establishment
void Saplings::establishment(const ResourceUnit *ru)
{
    FloatGrid *lif_grid = GlobalSettings::instance()->model()->grid();

    QPoint imap = ru->cornerPointOffset(); // offset on LIF/saplings grid
    QPoint iseedmap = QPoint(imap.x()/10, imap.y()/10); // seed-map has 20m resolution, LIF 2m -> factor 10

    for (QList<ResourceUnitSpecies*>::const_iterator i=ru->ruSpecies().constBegin(); i!=ru->ruSpecies().constEnd(); ++i) {
        float la = (*i)->saplingStat().leafArea();
        (*i)->saplingStat().clearStatistics();
        (*i)->saplingStat().setLeafArea(la); // retain the leaf area just in case the water cycle is executed during regeneration
    }

    double lif_corr[cPxPerHectare];
    for (int i=0;i<cPxPerHectare;++i)
        lif_corr[i]=-1.;


    int species_idx;
    QVector<int>::const_iterator sbegin, send;
    ru->speciesSet()->randomSpeciesOrder(sbegin, send);
    for (QVector<int>::const_iterator s_idx=sbegin; s_idx!=send;++s_idx) {

        // start from a random species (and cycle through the available species)
        species_idx = *s_idx;

        ResourceUnitSpecies *rus = ru->ruSpecies()[species_idx];
        rus->establishment().clear();

        // check if there are seeds of the given species on the resource unit
        float seeds = 0.f;
        Grid<float> &seedmap =  const_cast<Grid<float>& >(rus->species()->seedDispersal()->seedMap());
        for (int iy=0;iy<5;++iy) {
            float *p = seedmap.ptr(iseedmap.x(), iseedmap.y());
            for (int ix=0;ix<5;++ix)
                seeds += *p++;
        }
        // if there are no seeds: no need to do more
        if (seeds==0.f)
            continue;

        // calculate the abiotic environment (TACA) (this could also trigger the execution of the water cycle)
        rus->establishment().calculateAbioticEnvironment();
        double abiotic_env = rus->establishment().abioticEnvironment();
        if (abiotic_env==0.) {
            rus->establishment().writeDebugOutputs();
            continue;
        }

        // loop over all 2m cells on this resource unit
        SaplingCell *sap_cells = ru->saplingCellArray();
        SaplingCell *s;
        int isc = 0; // index on 2m cell
        for (int iy=0; iy<cPxPerRU; ++iy) {
            s = &sap_cells[iy*cPxPerRU]; // pointer to a row
            isc = lif_grid->index(imap.x(), imap.y()+iy);

            for (int ix=0;ix<cPxPerRU; ++ix, ++s, ++isc) {
                if (s->hasFreeSlots()) {
                    // is a sapling of the current species already on the pixel?
                    // * test for sapling height already in cell state
                    // * test for grass-cover already in cell state
                    SaplingTree *stree=nullptr;
                    SaplingTree *slot=s->saplings;
                    for (int i=0;i<NSAPCELLS;++i, ++slot) {
                        if (!stree && !slot->is_occupied())
                            stree=slot;
                        if (slot->species_index == species_idx) {
                            stree=nullptr;
                            break;
                        }
                    }

                    if (stree) {
                        // grass cover?
                        float seed_map_value = seedmap[lif_grid->index10(isc)];
                        if (seed_map_value==0.f)
                            continue;
                        float lif_value = (*lif_grid)[isc];

                        double &lif_corrected = lif_corr[iy*cPxPerRU+ix];
                        // calculate the LIFcorrected only once per pixel; the relative height is 0 (light level on the forest floor)
                        if (lif_corrected<0.)
                            lif_corrected = rus->species()->speciesSet()->LRIcorrection(lif_value, 0.);

                        // check for the combination of seed availability and light on the forest floor
                        if (drandom() < seed_map_value*lif_corrected*abiotic_env ) {
                            // ok, lets add a sapling at the given position (age is incremented later)
                            stree->setSapling(0.05f, 0, species_idx);
                            s->checkState();
                            rus->saplingStat().mAdded++;

                        }

                    }

                }
            }
        }
        // create debug output related to establishment
        rus->establishment().writeDebugOutputs();
    }

}

void Saplings::saplingGrowth(const ResourceUnit *ru)
{
    HeightGrid *height_grid = GlobalSettings::instance()->model()->heightGrid();
    FloatGrid *lif_grid = GlobalSettings::instance()->model()->grid();

    QPoint imap = ru->cornerPointOffset();
    bool need_check=false;
    SaplingCell *sap_cells = ru->saplingCellArray();

    for (int iy=0; iy<cPxPerRU; ++iy) {
        SaplingCell *s = &sap_cells[iy*cPxPerRU]; // ptr to row
        int isc = lif_grid->index(imap.x(), imap.y()+iy);

        for (int ix=0;ix<cPxPerRU; ++ix, ++s, ++isc) {
            if (s->state != SaplingCell::CellInvalid) {
                need_check=false;
                int n_on_px = s->n_occupied();
                for (int i=0;i<NSAPCELLS;++i) {
                    if (s->saplings[i].is_occupied()) {
                        // growth of this sapling tree
                        HeightGridValue &hgv = height_grid->valueAtIndex(lif_grid->index5(isc));
                        float lif_value = (*lif_grid)[isc];

                        need_check |= growSapling(ru, *s, s->saplings[i], isc, hgv, lif_value, n_on_px);
                    }
                }
                if (need_check)
                    s->checkState();

            }
        }
    }


    // store statistics on saplings/regeneration
    for (QList<ResourceUnitSpecies*>::const_iterator i=ru->ruSpecies().constBegin(); i!=ru->ruSpecies().constEnd(); ++i) {
        (*i)->saplingStat().calculate((*i)->species(), const_cast<ResourceUnit*>(ru));
        (*i)->statistics().add(&((*i)->saplingStat()));
    }

    // debug output related to saplings
    if (GlobalSettings::instance()->isDebugEnabled(GlobalSettings::dSaplingGrowth) && ru->shouldCreateDebugOutput()) {

        // establishment details
        for (QList<ResourceUnitSpecies*>::const_iterator it=ru->ruSpecies().constBegin();it!=ru->ruSpecies().constEnd();++it) {
            if ((*it)->saplingStat().livingCohorts() == 0)
                continue;
            DebugList &out = GlobalSettings::instance()->debugList(ru->index(), GlobalSettings::dSaplingGrowth);
            out << (*it)->species()->id() << ru->index() <<ru->id();
            out << (*it)->saplingStat().livingCohorts() << (*it)->saplingStat().averageHeight() << (*it)->saplingStat().averageAge()
                << (*it)->saplingStat().averageDeltaHPot() << (*it)->saplingStat().averageDeltaHRealized();
            out << (*it)->saplingStat().newSaplings() << (*it)->saplingStat().newSaplingsVegetative() << (*it)->saplingStat().diedSaplings()
                << (*it)->saplingStat().recruitedSaplings() <<(*it)->species()->saplingGrowthParameters().referenceRatio;
            out << (*it)->saplingStat().carbonLiving().C << (*it)->saplingStat().carbonGain().C;

        }
    }

}

void Saplings::simplifiedGrassCover(const ResourceUnit *ru)
{
    if (!ru)
        return;

    float threshold = GlobalSettings::instance()->model()->grassCover()->lifThreshold();
    FloatGrid *lif_grid = GlobalSettings::instance()->model()->grid();

    QPoint imap = ru->cornerPointOffset();
    SaplingCell *sap_cells = ru->saplingCellArray();

    for (int iy=0; iy<cPxPerRU; ++iy) {
        SaplingCell *s = &sap_cells[iy*cPxPerRU]; // ptr to row
        int isc = lif_grid->index(imap.x(), imap.y()+iy);

        for (int ix=0;ix<cPxPerRU; ++ix, ++s, ++isc) {
            if (s->state == SaplingCell::CellEmpty || s->state== SaplingCell::CellGrass) {
                s->state =  (*lif_grid)[isc] > threshold ? SaplingCell::CellGrass : SaplingCell::CellEmpty;
            }
        }
    }
}

double Saplings::topHeight(const ResourceUnit *ru) const
{
    // top height
    SaplingCell *sap_cell = ru->saplingCellArray();
    //int n_cells = ru->stockableArea() / (cPxSize*cPxSize);
    float h_max = 0.f;
    for (int iy=0; iy<cPxPerRU; ++iy, ++sap_cell) {
        if (sap_cell->state!=SaplingCell::CellInvalid) {
            h_max = qMax(sap_cell->max_height(), h_max);
        }
    }
    return static_cast<double>(h_max);

}

SaplingCell *Saplings::cell(QPoint lif_coords, bool only_valid, ResourceUnit **rRUPtr)
{
    FloatGrid *lif_grid = GlobalSettings::instance()->model()->grid();

    // in this case, getting the actual cell is quite cumbersome: first, retrieve the resource unit, then the
    // cell based on the offset of the given coordiantes relative to the corner of the resource unit.
    ResourceUnit *ru = GlobalSettings::instance()->model()->ru(lif_grid->cellCenterPoint(lif_coords));
    if (rRUPtr)
        *rRUPtr = ru;

    if (ru) {
        QPoint local_coords = lif_coords - ru->cornerPointOffset();
        int idx = local_coords.y() * cPxPerRU + local_coords.x();
        DBGMODE( if (idx<0 || idx>=cPxPerHectare)
                 qDebug("invalid coords in Saplings::cell");
                    );
        SaplingCell *s=&ru->saplingCellArray()[idx];
        if (s && (!only_valid || s->state!=SaplingCell::CellInvalid))
            return s;
    }
    return nullptr;
}

QPointF Saplings::coordOfCell(const ResourceUnit *ru, int cell_index)
{
    QPoint imap = ru->cornerPointOffset();
    int x = imap.x() + cell_index % cPxPerRU;
    int y = imap.y() + cell_index/cPxPerRU;
    return GlobalSettings::instance()->model()->grid()->cellCenterPoint(QPoint(x,y));
}

QPoint Saplings::coordOfCellLIF(const ResourceUnit *ru, int cell_index)
{
    QPoint imap = ru->cornerPointOffset();
    int x = imap.x() + cell_index % cPxPerRU;
    int y = imap.y() + cell_index/cPxPerRU;
    QPointF coord = GlobalSettings::instance()->model()->grid()->cellCenterPoint(QPoint(x,y));
    return GlobalSettings::instance()->model()->grid()->indexAt(coord);
}



void Saplings::clearSaplings(const QRectF &rectangle, const bool remove_biomass, bool resprout)
{
    GridRunner<float> runner(GlobalSettings::instance()->model()->grid(), rectangle);
    ResourceUnit *ru;
    while (runner.next()) {
        SaplingCell *s = cell(runner.currentIndex(), true, &ru);
        if (s) {
            clearSaplings(s, ru, remove_biomass, resprout);
        }

    }
}

void Saplings::clearSaplings(SaplingCell *s, ResourceUnit *ru, const bool remove_biomass, bool resprout)
{
    if (s) {
        for (int i=0;i<NSAPCELLS;++i)
            if (s->saplings[i].is_occupied()) {
                ResourceUnitSpecies *rus = s->saplings[i].resourceUnitSpecies(ru);
                if (!remove_biomass) {
                    if (!rus && !rus->species()) {
                        qDebug() << "Saplings::clearSaplings(): invalid resource unit!!!";
                        return;
                    }
                    rus->saplingStat().addCarbonOfDeadSapling( s->saplings[i].height / rus->species()->saplingGrowthParameters().hdSapling * 100.f );
                }

                // check for resprout (e.g. after being killed by fire)
                bool do_clear = true;
                if (resprout) {
                    if (rus && rus->species() && rus->species()->saplingGrowthParameters().sproutGrowth > 0. && s->saplings[i].height>1.) {
                        s->saplings[i].clear();
                        do_clear=false;
                        SaplingTree *st=s->addSapling(0.05f, 0, rus->species()->index());
                        if (st)
                            st->set_sprout(true);
                    }
                }
                if (do_clear)
                    s->saplings[i].clear();

            }
        s->checkState();

    }

}

void Saplings::clearAllSaplings()
{
    GridRunner<float> runner(GlobalSettings::instance()->model()->grid(), GlobalSettings::instance()->model()->grid()->metricRect());
    ResourceUnit *ru;
    while (runner.next()) {
        SaplingCell *s = cell(runner.currentIndex(), true, &ru);
        if (s) {
            clearSaplings(s, ru, true, false);
        }
    }
}

int Saplings::addSaplings(const QRectF &rectangle, QString species, double height, int age)
{
    GridRunner<float> runner(GlobalSettings::instance()->model()->grid(), rectangle);
    Species *s = GlobalSettings::instance()->model()->speciesSet()->species(species);
    if (!s) {
        throw IException("addSaplings: invalid species: " + species);
    }
    short signed int species_index = static_cast<short signed int>(s->index());
    int nadded = 0;
    while (runner.next()) {
        SaplingCell *s = cell(runner.currentIndex(), true);
        if (s) {
            if (s->addSapling(height, age, species_index))
                ++nadded;

        }
    }
    return nadded;
}

int Saplings::addSprout(const Tree *t, bool tree_is_removed)
{
    if (t->species()->saplingGrowthParameters().sproutGrowth==0.)
        return 0;
    ResourceUnit *ru;
    SaplingCell *sc = cell(t->positionIndex(), true, &ru);
    if (!sc)
        return 0;


    if (tree_is_removed) {
        // if the host tree is removed (disturbance, harvest), a resprout is happening on the current pixel ( and pixels that are
        // (partially) covered by the tree crown
        clearSaplings(sc, const_cast<ResourceUnit*>(t->ru()), false, false );
        SaplingTree *st=sc->addSapling(0.05f, 0, t->species()->index());
        if (st)
            st->set_sprout(true);
        sc->checkState();

/*
 *      // neighboring cells
        double crown_area = t->crownRadius()*t->crownRadius() * M_PI; //m2
        // calculate how many cells on the ground are covered by the crown (this is a rather rough estimate)
        // n_cells: in addition to the original cell
        int n_cells = static_cast<int>(round( crown_area / static_cast<double>(cPxSize*cPxSize) - 1.));
        if (n_cells>0) {
            ResourceUnit *ru;
            int s=irandom(0,8);
            while(n_cells) {
                sc = cell(t->positionIndex()+QPoint(offsets_x[s], offsets_y[s]),true,&ru);
                if (sc && !sc->sapling(t->species()->index())) {
                    // add sapling only if cell not already occupied by a sapling of the species
                    SaplingTree *st=sc->addSapling(0.05f, 0, t->species()->index());
                    if (st)
                        st->set_sprout(true);
                }

                s = (s+1)%8; --n_cells;
            }
        } */
    } else {

        // sprouts spread from a living tree with
        // a low probability in adjacent cells
        if (t->species()->saplingGrowthParameters().adultSproutProbability > 0. && t->age() > t->species()->maturityAge())
            vegetativeSprouting(t->species(), *sc, t->positionIndex());

    }
    return 1;
}

void Saplings::updateBrowsingPressure()
{
    if (GlobalSettings::instance()->settings().valueBool("model.settings.browsing.enabled"))
        Saplings::mBrowsingPressure = GlobalSettings::instance()->settings().valueDouble("model.settings.browsing.browsingPressure");
    else
        Saplings::mBrowsingPressure = 0.;
}

bool Saplings::growSapling(const ResourceUnit *ru, SaplingCell &scell, SaplingTree &tree, int isc, HeightGridValue &hgv, float lif_value, int cohorts_on_px)
{
    ResourceUnitSpecies *rus = tree.resourceUnitSpecies(ru);
    if (!rus) {
        return false;
    }
    const Species *species = rus->species();
    if (!species) {
        return false;
    }


    // (1) calculate height growth potential for the tree (uses linerization of expressions...)
    double h_pot = species->saplingGrowthParameters().heightGrowthPotential.calculate(tree.height);
    double delta_h_pot = h_pot - tree.height;

    // (2) reduce height growth potential with species growth response f_env_yr and with light state (i.e. LIF-value) of home-pixel.
    if (hgv.height==0.f)
        throw IException(QString("growSapling: height grid at %1/%2 has value 0").arg(isc));

    double rel_height = tree.height / hgv.height;

    double lif_corrected = species->speciesSet()->LRIcorrection(lif_value, rel_height); // correction based on height

    double lr = species->lightResponse(lif_corrected); // species specific light response (LUI, light utilization index)

    rus->calculate(true); // calculate the 3pg module (this is done only once per RU); true: call comes from regeneration
    double f_env_yr = rus->prod3PG().fEnvYear();

    double delta_h_factor = f_env_yr * lr; // relative growth

    if (h_pot<0. || delta_h_pot<0. || lif_corrected<0. || lif_corrected>1. || delta_h_factor<0. || delta_h_factor>1. )
        qDebug() << "invalid values in Sapling::growSapling";

    // sprouts grow faster. Sprouts therefore are less prone to stress (threshold), and can grow higher than the growth potential.
    if (tree.is_sprout())
        delta_h_factor = delta_h_factor *species->saplingGrowthParameters().sproutGrowth;

    // check browsing
    if (mBrowsingPressure>0. && tree.height<=2.f) {
        double p = rus->species()->saplingGrowthParameters().browsingProbability;
        // calculate modifed annual browsing probability via odds-ratios
        // odds = p/(1-p) -> odds_mod = odds * browsingPressure -> p_mod = odds_mod /( 1 + odds_mod) === p*pressure/(1-p+p*pressure)
        double p_browse = p*mBrowsingPressure / (1. - p + p*mBrowsingPressure);
        if (drandom() < p_browse) {
            delta_h_factor = 0.;
        }
    }
    // check browsing due to "management" or BITE
    if (tree.is_browsed()) {
        delta_h_factor = 0.;
        tree.set_browsed(false);
    }

    // intrinsic mortality of saplings: use the same approach as adult trees (probability based on the maximum age species parameter)
    bool sapling_dies = false;
    if (drandom() < species->deathProb_intrinsic()) {
        sapling_dies = true;
    }
    // check stress mortality of saplings
    if (delta_h_factor < species->saplingGrowthParameters().stressThreshold) {
        tree.stress_years++;
        if (tree.stress_years > species->saplingGrowthParameters().maxStressYears) {
            sapling_dies = true;
        }
    } else {
        tree.stress_years=0; // reset stress counter
    }

    if (sapling_dies) {
        // sapling dies either due to stress or due to age related mortality
        rus->saplingStat().addCarbonOfDeadSapling( tree.height / species->saplingGrowthParameters().hdSapling * 100.f );
        tree.clear();
        return true; // need cleanup

    }
    DBG_IF(delta_h_pot*delta_h_factor < 0.f || (!tree.is_sprout() && delta_h_pot*delta_h_factor > 2.), "Sapling::growSapling",
           QString("implausible height growth: species: %1, h: %2, deltaH: %3").arg( species->id()).arg(tree.height).arg(delta_h_pot*delta_h_factor).toLocal8Bit() );

    // grow
    tree.height += static_cast<float>(delta_h_pot * delta_h_factor);
    tree.age++; // increase age of sapling by 1

    // recruitment?
    double total_carbon_added=0.;
    if (tree.height > cSapHeight) {
        rus->saplingStat().mRecruited++;

        float dbh = tree.height / species->saplingGrowthParameters().hdSapling * 100.f;
        // the number of trees to create (result is in trees per pixel)
        double n_trees = species->saplingGrowthParameters().representedStemNumber(dbh);
        int to_establish = static_cast<int>( n_trees );

        // if n_trees is not an integer, choose randomly if we should add a tree.
        // e.g.: n_trees = 2.3 -> add 2 trees with 70% probability, and add 3 trees with p=30%.
        if (drandom() < (n_trees-to_establish) || to_establish==0)
            to_establish++;

        // add a new tree
        for (int i=0;i<to_establish;i++) {
            Tree &bigtree = const_cast<ResourceUnit*>(ru)->newTree();

            bigtree.setPosition(GlobalSettings::instance()->model()->grid()->indexOf(isc));
            // add variation: add +/-N% to dbh and *independently* to height.
            bigtree.setDbh(static_cast<float>(dbh * nrandom(1. - mRecruitmentVariation, 1. + mRecruitmentVariation)));
            bigtree.setHeight(static_cast<float>(tree.height * nrandom(1. - mRecruitmentVariation, 1. + mRecruitmentVariation)));
            bigtree.setSpecies( const_cast<Species*>(species) );
            bigtree.setAge(tree.age,tree.height);
            bigtree.setRU(const_cast<ResourceUnit*>(ru));
            bigtree.setup();
            const Tree *t = &bigtree;
            const_cast<ResourceUnitSpecies*>(rus)->statistics().add(t, nullptr); // count the newly created trees already in the stats
            // account for the carbon that is *added* by the new trees
            total_carbon_added += (bigtree.biomassStem()+bigtree.biomassBranch()+bigtree.biomassFoliage()+bigtree.biomassCoarseRoot()+bigtree.biomassFineRoot())*biomassCFraction;
        }
        // clear all regeneration from this pixel (including this tree)
        double woody_bm = species->biomassStem(dbh) + species->biomassBranch(dbh) + species->biomassRoot(dbh);
        double foliage = species->biomassFoliage(dbh);
        double fineroot = foliage*species->finerootFoliageRatio();
        // the 'gap' between sapling and trees is: sum(C recruited trees) - C sapling
        total_carbon_added -= total_carbon_added - (woody_bm + foliage + fineroot)*biomassCFraction;

        tree.clear(); // clear this tree (no carbon flow to the ground)
        for (int i=0;i<NSAPCELLS;++i) {
            if (scell.saplings[i].is_occupied()) {
                // add carbon to the ground
                ResourceUnitSpecies *srus = scell.saplings[i].resourceUnitSpecies(ru);
                srus->saplingStat().addCarbonOfDeadSapling( scell.saplings[i].height / srus->species()->saplingGrowthParameters().hdSapling * 100.f );
                scell.saplings[i].clear();
            }
        }
        rus->saplingStat().mCarbonOfRecruitedTrees += static_cast<float>(total_carbon_added);
        return true; // need cleanup
    }
    // book keeping (only for survivors) for the sapling of the resource unit / species
    SaplingStat &ss = rus->saplingStat();
    float n_repr = static_cast<float>( species->saplingGrowthParameters().representedStemNumberH(tree.height) / static_cast<double>(cohorts_on_px) );
    if (tree.height>1.3f) {
        ss.mLivingSaplings += n_repr;
        ss.mCohortsWithDbh++;
    } else
        ss.mLivingSmallSaplings += n_repr;
    ss.mLiving++;
    ss.mAvgHeight+=tree.height;
    ss.mAvgAge+=tree.age;
    ss.mAvgDeltaHPot+= static_cast<float>( delta_h_pot );
    ss.mAvgHRealized += static_cast<float>( delta_h_pot * delta_h_factor );


    // seed dispersal: the saplings produce seed if they are are old enough
    // important e.g. for Pinus mugo
    if (species->seedDispersal() && tree.age > species->maturityAge()) {
        float leaf_area = cPxSize*cPxSize * 2.5f; // assume for now a fixed LAI of 2.5
        float dbh = tree.height / species->saplingGrowthParameters().hdSapling * 100.f;
        double foliage = species->biomassFoliage(dbh);
        leaf_area = static_cast<float>( foliage * n_repr );
        species->seedDispersal()->setSaplingTree(GlobalSettings::instance()->model()->grid()->indexOf(isc), leaf_area);
    }

    // sprouting from regeneration: this requires a minimum height (and lateral sprouting being enabled)
    if (species->saplingGrowthParameters().adultSproutProbability > 0. && tree.age > species->maturityAge()) {
        vegetativeSprouting(species, scell, GlobalSettings::instance()->model()->grid()->indexOf(isc));
    }


    // update stem height
    //float sh_before = hgv.stemHeight();
    if (tree.height > hgv.stemHeight()) {
        hgv.setStemHeight(tree.height);
        //qDebug()<< "sapheihgt:updated at:"<<GlobalSettings::instance()->model()->heightGrid()->cellCenterPoint(GlobalSettings::instance()->model()->heightGrid()->indexOf(&hgv));
    }
    //qDebug() << "sapheight:" << tree.height << "hgv:before:" << sh_before << "after:"  << hgv.stemHeight() << "hgv:" << &hgv;
    return false;
}

void Saplings::vegetativeSprouting(const Species *species, SaplingCell &scell, QPoint tree_pos)
{
    // sprouts spread from a living tree with
    // a low probability in adjacent cells
    static const int offsets_x[8] = {1,1,0,-1,-1,-1,0,1};
    static const int offsets_y[8] = {0,1,1,1,0,-1,-1,-1};

    const double p_resprout = species->saplingGrowthParameters().adultSproutProbability;
    if (p_resprout>0. && drandom() < p_resprout) {
        // select a neighbor randomly
        int s=irandom(0,8);
        ResourceUnit *ru_new;

        for (int i=0;i<8;++i) {
            SaplingCell *sc_new = cell(tree_pos +QPoint(offsets_x[s], offsets_y[s]), true, &ru_new);
            if (sc_new && !sc_new->saplingOfSpecies(species->index())) {
                if (GlobalSettings::instance()->model()->settings().torusMode) {
                    // in torus mode we make sure not to grow saplings in an adjacent resource unit
                    if (scell.ru != ru_new) {
                        s = (s+1)%8; // move on...
                        continue;
                    }
                }
                // the species is not yet on the cell, so let us spread there....
                SaplingTree *st=sc_new->addSapling(0.05f, 0, species->index());
                sc_new->checkState();
                if (st) {
                    st->set_sprout(true);
                    ru_new->resourceUnitSpecies(species).saplingStat().mAddedVegetative++;
                }
                break;  // stop searching when one sprout was added

            }
            s = (s+1)%8; // move on...
        }
    }
}

void SaplingStat::clearStatistics()
{
    mRecruited=mDied=mLiving=mCohortsWithDbh=0;
    mLivingSaplings=0.; mLivingSmallSaplings=0.;
    mSumDbhDied=0.;
    mAvgHeight=0.;
    mAvgAge=0.;
    mAvgDeltaHPot=mAvgHRealized=0.;
    mAdded=0; mAddedVegetative = 0;
    mLeafArea=0.; mLeafAreaIndex=0.;
    mBasalArea = 0.;
    mCarbonOfRecruitedTrees=0.;

}

void SaplingStat::calculate(const Species *species, ResourceUnit *ru)
{
    if (mLiving) {
        mAvgHeight /= static_cast<float>(mLiving);
        mAvgAge /= static_cast<float>(mLiving);
        mAvgDeltaHPot /= static_cast<float>(mLiving);
        mAvgHRealized /= static_cast<float>(mLiving);
    }

    // calculate carbon balance
    CNPair old_state = mCarbonLiving;
    mCarbonLiving.clear();

    CNPair dead_wood, dead_fine; // pools for mortality
    double dead_wood_ag = 0., dead_fine_ag=0.; // carbon aboveground
    double c_turnover = 0.;
    // average dbh
    if (mLiving>0) {
        // calculate the avg dbh and number of stems
        double avg_dbh = mAvgHeight / species->saplingGrowthParameters().hdSapling * 100.;
        // the number of "real" stems is given by the Reineke formula
        double n = mLivingSaplings; // total number of saplings (>1.3m) (=represented stems, result of the Reineke equation)

        // woody parts: stem, branchse and coarse roots
        double woody_bm = species->biomassStem(avg_dbh) + species->biomassBranch(avg_dbh) + species->biomassRoot(avg_dbh);
        double foliage = species->biomassFoliage(avg_dbh);
        double fineroot = foliage*species->finerootFoliageRatio();
        mLeafArea = static_cast<float>( foliage * n * species->specificLeafArea() ); // calculate leaf area on n saplings using the species specific SLA
        mLeafAreaIndex = static_cast<float>( ru->stockableArea()>0. ? mLeafArea / ru->stockableArea() : 0. );
        mBasalArea = static_cast<float>( (avg_dbh / 200.)*(avg_dbh/200.)*M_PI * n );

        // get living carbon.
        mCarbonLiving.addBiomass( woody_bm*n, species->cnWood()  );
        mCarbonLiving.addBiomass( foliage* n , species->cnFoliage()  );
        mCarbonLiving.addBiomass( fineroot* n, species->cnFineroot()  );
        c_turnover = (foliage*species->turnoverRoot() + fineroot*species->turnoverRoot())* n * biomassCFraction;

        DBGMODE(
        if (isnan(mCarbonLiving.C))
            qDebug("carbon NaN in SaplingStat::calculate (living trees).");
                );

        // turnover
        if (ru->snag())
            ru->snag()->addTurnoverLitter(species, n*foliage*species->turnoverLeaf(), n*fineroot*species->turnoverRoot());

        // calculate the "mortality from competition", i.e. carbon that stems from reduction of stem numbers
        // from Reinekes formula.
        //
        if (avg_dbh>1.) {
            // compare only with cohorts >1.3m
            double avg_dbh_before = (mAvgHeight - mAvgHRealized) / species->saplingGrowthParameters().hdSapling * 100.;
            double n_before = mCohortsWithDbh * species->saplingGrowthParameters().representedStemNumber( qMax(1.,avg_dbh_before) );
            if (n<n_before) {
                dead_wood.addBiomass( woody_bm * (n_before-n), species->cnWood() );
                dead_fine.addBiomass( foliage * (n_before-n), species->cnFoliage()  );
                dead_fine.addBiomass( fineroot * (n_before-n), species->cnFineroot()  );
                DBGMODE(
                if (isnan(dead_fine.C))
                    qDebug("carbon NaN in SaplingStat::calculate (self thinning).");
                        );

            }
        }

    } else {
        mLeafArea = 0.; // leaf area is not cleared at the beginning of the regen loop (for the water cycle)
    }

    if (GlobalSettings::instance()->currentYear()==0)
        return; // no need for carbon flows in initial run


    if (mDied) {
        double avg_dbh_dead = mSumDbhDied / double(mDied);
        double n = mDied * species->saplingGrowthParameters().representedStemNumber( avg_dbh_dead );
        // woody parts: stem, branchse and coarse roots

        double bm_above = (species->biomassStem(avg_dbh_dead) + species->biomassBranch(avg_dbh_dead) ) * n;
        dead_wood.addBiomass(  species->biomassRoot(avg_dbh_dead) * n + bm_above, species->cnWood()  );
        dead_wood_ag += bm_above * biomassCFraction;

        double foliage = species->biomassFoliage(avg_dbh_dead)*n;

        dead_fine.addBiomass( foliage, species->cnFoliage()  );
        dead_fine.addBiomass( foliage*species->finerootFoliageRatio(), species->cnFineroot()  );
        dead_fine_ag += foliage * biomassCFraction;
        DBGMODE(
        if (isnan(dead_fine.C))
            qDebug("carbon NaN in SaplingStat::calculate (died trees).");
                );

    }
    if (!dead_wood.isEmpty() || !dead_fine.isEmpty())
        if (ru->snag())
            ru->snag()->addToSoil(species, dead_wood, dead_fine, dead_wood_ag, dead_fine_ag);

    // calculate net growth:
    // delta of stocks
    mCarbonGain = mCarbonLiving + dead_fine + dead_wood - old_state;
    mCarbonGain.C += c_turnover;
    mCarbonGain.C += mCarbonOfRecruitedTrees; // correction for newly created trees
    if (mCarbonGain.C < 0)
        mCarbonGain.clear();


    GlobalSettings::instance()->systemStatistics()->saplingCount+=mLiving;
    GlobalSettings::instance()->systemStatistics()->newSaplings+=mAdded;

}

double SaplingStat::livingStemNumber(const Species *species, double &rAvgDbh, double &rAvgHeight, double &rAvgAge) const
{
     rAvgHeight = averageHeight();
     rAvgDbh = rAvgHeight / species->saplingGrowthParameters().hdSapling * 100.f;
     rAvgAge = averageAge();
     double n= species->saplingGrowthParameters().representedStemNumber(rAvgDbh);
     return n;
// *** old code (sapling.cpp) ***
//    double total = 0.;
//    double dbh_sum = 0.;
//    double h_sum = 0.;
//    double age_sum = 0.;
//    const SaplingGrowthParameters &p = mRUS->species()->saplingGrowthParameters();
//    for (QVector<SaplingTreeOld>::const_iterator it = mSaplingTrees.constBegin(); it!=mSaplingTrees.constEnd(); ++it) {
//        float dbh = it->height / p.hdSapling * 100.f;
//        if (dbh<1.) // minimum size: 1cm
//            continue;
//        double n = p.representedStemNumber(dbh); // one cohort on the pixel represents that number of trees
//        dbh_sum += n*dbh;
//        h_sum += n*it->height;
//        age_sum += n*it->age.age;
//        total += n;
//    }
//    if (total>0.) {
//        dbh_sum /= total;
//        h_sum /= total;
//        age_sum /= total;
//    }
//    rAvgDbh = dbh_sum;
//    rAvgHeight = h_sum;
//    rAvgAge = age_sum;
//    return total;
}

ResourceUnitSpecies *SaplingTree::resourceUnitSpecies(const ResourceUnit *ru) const
{
    if (!ru || !is_occupied())
        return nullptr;
    if (species_index  < 0)
        return nullptr;
    ResourceUnitSpecies *rus = ru->resourceUnitSpecies(species_index);
    return rus;
}

SaplingCellRunner::SaplingCellRunner(const int stand_id, const MapGrid *stand_grid)
{
    mRunner = nullptr;
    mRU = nullptr;
    mStandId = stand_id;
    mStandGrid = stand_grid ? stand_grid : GlobalSettings::instance()->model()->standGrid();
    QRectF box = mStandGrid->boundingBox(stand_id);
    mRunner = new GridRunner<float>(GlobalSettings::instance()->model()->grid(), box);

}


SaplingCellRunner::~SaplingCellRunner()
{
    if (mRunner)
        delete mRunner;
}

SaplingCell *SaplingCellRunner::next()
{
    if (!mRunner)
        return nullptr;
    while (float *n = mRunner->next()) {
        if (!n)
            return nullptr; // end of the bounding box
        if (mStandGrid->standIDFromLIFCoord(mRunner->currentIndex()) != mStandId)
            continue; // pixel does not belong to the target stand
        mRU = GlobalSettings::instance()->model()->ru(mRunner->currentCoord());
        SaplingCell *sc=nullptr;
        if (mRU)
            sc=mRU->saplingCell(mRunner->currentIndex());
        if (sc)
            return sc;
        qDebug() << "SaplingCellRunner::next(): unexected missing SaplingCell!";
        return nullptr;
    }
    return nullptr;
}

QPointF SaplingCellRunner::currentCoord() const
{
    return mRunner->currentCoord();
}
