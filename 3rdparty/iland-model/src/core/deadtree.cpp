#include "deadtree.h"

#include "tree.h"
#include "species.h"
#include "snag.h"

DeadTree::DeadTree(const Tree *tree)
{
    mSpecies = tree->species();
    QPointF tree_pos = tree->position();
    mX = tree_pos.x();
    mY = tree_pos.y();
    mVolume = tree->volume();
    mInititalBiomass = tree->biomassStem();
    mBiomass = tree->biomassStem();
    mCrownRadius = tree->crownRadius();
    if (mInititalBiomass <= 0.)
        throw IException("DeadTree: invalid stem biomass of <=0!");
    // death reason:
    if (tree->isDead()) mDeathReason = 1; // mortality
    if (tree->isDeadBarkBeetle()) mDeathReason = 2;
    if (tree->isDeadWind()) {
        mDeathReason = 3;
        mIsStanding = false; // wind disturbed trees go to the ground immediately
    }
    if (tree->isDeadFire()) mDeathReason = 4;
    if (tree->isCutdown()) {
        mDeathReason = 5;
        mIsStanding = false; // drop down to the ground immediately
    }
}

bool DeadTree::calculate(double climate_factor, CNPair &rFlux_to_atmosphere, CNPair &rFlux_to_refr)
{
    if (mYearsStandingDead == 0 && !isStanding()) {
        // special case: snags start as downed -> immediately transfer all biomass to DWD pools
        rFlux_to_refr.C += mBiomass * biomassCFraction;
        rFlux_to_refr.N += mInititalBiomass * biomassCFraction / species()->cnWood();
    }

    if (isStanding()) {
        mYearsStandingDead++;
        calculateSnag(climate_factor, rFlux_to_atmosphere, rFlux_to_refr);
    } else {
        // lying deadwood
        mYearsDowned++;
        return calculateDWD(climate_factor, rFlux_to_atmosphere, rFlux_to_refr);
    }
    return true;
}

bool DeadTree::calculateSnag(double climate_factor_re, CNPair &rFlux_to_atmosphere, CNPair &rFlux_to_refr)
{

    // update biomass, use decomposition rate for snags
    double decay_factor = exp(-species()->snagKsw() * climate_factor_re);
    rFlux_to_atmosphere.C += mBiomass * (1. - decay_factor) * biomassCFraction;
    mBiomass *= decay_factor;

    updateDecayClass();

    // calculate probability of falling down
    double p_fall;
    p_fall = log(2.) / ( species()->snagHalflife() / climate_factor_re );

    // transfer to DWD?
    if (drandom() < p_fall) {
        mIsStanding = false;
        // explict transfer of biomass to DWD pool of the soil
        // Important for tracking biomass and carbon balance:
        // the "real" tracking of DWD biomass is in the soil pools (Yr). Upon falling, biomass
        // is transffered to Yr (and also reported in carbon outputs).
        // here we continue to track individual DWD pieces, but that does *not* affect
        // carbon pools and is only for tracking decay classes!
        rFlux_to_refr.C += mBiomass * biomassCFraction;
        rFlux_to_refr.N += mInititalBiomass * biomassCFraction / species()->cnWood();

        return false; // changed to DWD
    }
    return true;
}

bool DeadTree::calculateDWD(double climate_factor_re, CNPair &rFlux_to_atmosphere, CNPair &rFlux_to_refr)
{
    Q_UNUSED(rFlux_to_refr)
    // update biomass... use the decomposition rate for woody biomass on the ground
    // Note: carbon calculation for DWD is only "for fun" - the actual tracking of
    // biomass/carbon is done in Soil-pools! (all BM is transferred when the stem is downed)
    double decay_factor = exp(-species()->snagKyr() * climate_factor_re);
    rFlux_to_atmosphere.C += mBiomass * (1. - decay_factor) * biomassCFraction;
    mBiomass *= decay_factor;

    updateDecayClass();

    // drop out?
    if (proportionBiomass() < 0.05) {
        // set ptr to 0 -> mark to be cleared
        mSpecies = nullptr;
        return false;
    }
    return true;
}

void DeadTree::updateDecayClass()
{
    double remaining = proportionBiomass();
    mDecayClass = 5;
    auto thresholds = Snag::decayClassThresholds();
    if (remaining > thresholds[0]) mDecayClass = 4;
    if (remaining > thresholds[1]) mDecayClass = 3;
    if (remaining > thresholds[2]) mDecayClass = 2;
    if (remaining > thresholds[3]) mDecayClass = 1;
    //if (mDecayClass == 5)
    //    mVolume = 0.;
}
