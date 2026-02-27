#ifndef DEADTREE_H
#define DEADTREE_H
#include <cstdint>
class Species; // forward
class Tree; // forward
class CNPair; // forward
/**
 * @brief The DeadTree class encapsulates a single dead tree
 * (either standing or lying) that is tracked as invidual element.
 */
class DeadTree
{
public:
    DeadTree() {};
    DeadTree(const Tree *tree);
    /// main update function for both snags and DWD
    /// decomposition of C is tracked in rFlux_to_atmosphere, flow of matter to soil pool in rFlux_to_refr
    /// return false if tracking stops
    bool calculate(double climate_factor, CNPair &rFlux_to_atmosphere, CNPair &rFlux_to_refr);
    /// mark a tree to be removed on next call to Snag::packDeadTrees
    void setToBeRemoved() { mSpecies = nullptr; }

    // properties
    /// x-coordinate (metric, center of 2m cell)
    double x() const { return mX; }
    /// x-coordinate (metric, center of 2m cell)
    double y() const { return mY; }
    /// true if standing, false if downed dead wood
    bool isStanding() const { return mIsStanding; }
    /// tree volume of the stem at the time of death
    double volume() const { return mVolume; }
    /// current biomass (kg)
    double biomass() const { return mBiomass; }
    /// crown radius of the living tree
    double crownRadius()const { return mCrownRadius; }
    /// proportion of remaining biomass (0..1)
    double proportionBiomass() const { return mBiomass / mInititalBiomass; }
    /// initial biomass (i.e. stem biomass at time of death)
    double initialBiomass() const { return mInititalBiomass; }
    /// decayClass: 1..5
    int decayClass() const { return mDecayClass; }
    /// years since death (standing as snag)
    int yearsStanding() const  { return mYearsStandingDead; }
    /// years since downed (on the ground)
    int yearsDowned() const { return mYearsDowned; }
    /// species ptr
    const Species *species() const { return mSpecies; }
    /// reason of death: 1: "normal" mortality, 2: bb, 3: wind, 4: fire, 5: mgmt
    int reason() const { return mDeathReason; }
private:
    bool calculateSnag(double climate_factor_re, CNPair &rFlux_to_atmosphere, CNPair &rFlux_to_refr); // process standing snag
    bool calculateDWD(double climate_factor_re, CNPair &rFlux_to_atmosphere, CNPair &rFlux_to_refr); // process lying deadwood
    /// set decay class (I to V) based on the
    /// proportion of remaining biomass
    void updateDecayClass();
    float mX {0};
    float mY {0};
    const Species *mSpecies {nullptr };
    bool mIsStanding {true};
    std::uint8_t mDeathReason {0};
    short int mYearsStandingDead {0};
    short int mYearsDowned {0};
    short int mDecayClass {0};
    float mVolume {0};
    float mInititalBiomass {0}; // kg biomass at time of death
    float mBiomass {0}; // kg biomass currently
    float mCrownRadius {0}; // crown radius (m)

    friend class Snapshot;
};

#endif // DEADTREE_H
