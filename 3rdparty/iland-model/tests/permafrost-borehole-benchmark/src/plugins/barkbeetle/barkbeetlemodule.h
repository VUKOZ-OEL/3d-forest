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
#ifndef BARKBEETLEMODULE_H
#define BARKBEETLEMODULE_H

#include "grid.h"
#include "layeredgrid.h"
#include "random.h"
#include "expression.h"

#include "bbgenerations.h"



/** @brief The BarkBeetleCell is the basic unit (10m pixels) of the bark beetle module.
 * Cells can be infested (i.e. beetles attacked/killed all spruce trees within its perimeter), and
 * new beetle (packages) spread from the cell to infest new cells.
 * @ingroup barkbeetle
 * */
class BarkBeetleCell
{
public:
    BarkBeetleCell() { reset(); }
    void clear() { n=0; n_total=0; killedYear=0; outbreakYear=0.f; infested=false;  p_colonize=0.f; deadtrees=NoDeadTrees; packageOutbreakYear=0.f; }
    /// full reset of the pixel
    void reset() {clear(); dbh=0.f; tree_stress=0.f; outbreakYear=0.f; n_events=0; sum_volume_killed=0.f; }
    bool isHost() const { return dbh>0.f; }
    bool isPotentialHost() const {return dbh>0.f && killedYear==0 && infested==false; }
    /// sets the 'infested' state (true: the cell is newly infested, false: the cell stops being infested, e.g. by winter mortality)
    void setInfested(bool is_infested) { infested=is_infested; if (infested) { total_infested++; killedYear=0; n=0;} }
    /// called after beetles spread out from the cell. The cell is marked as 'killed', and trees will be killed later (barkbeetleKill()).
    void finishedSpread(int iteration) { infested=false; killedYear=iteration; killed=true; max_iteration=qMax(max_iteration, iteration); ++n_events; }
    float backgroundInfestationProbability; ///< background prob. of infestation per 10m cell

    bool infested; // true for cells that are currently occupied by beetles
    bool killed; // true for cells that are killed in the current year
    float dbh; // the dbh of the biggest spruce on the pixel
    float tree_stress; // the stress rating of this tree (SI)
    float p_colonize; // the highest probability (0..1) that a pixel is killed
    int n; // number of cohorts that landed on the pixel in each iteration
    int n_total; // number of cohorts that landed on a pixel (total)
    int killedYear; // year (iteration) at which pixel was killed ??
    float outbreakYear; // year in which the outbreak started (this information is preserved by spreading beatles)
    float packageOutbreakYear; // outbreak year of packages landing on a cell
    int n_events; // total number of events on the pixel since the start of the simulation
    float sum_volume_killed; // total killed volume (since start of the simulation) on a pixel (m3)
    enum DeadTrees { NoDeadTrees=0, StormDamage=10, SinkInVicinity=5, BeetleTrapTree=8 };
    DeadTrees deadtrees;
    /// return true if either storm damaged trees or trap trees are on the pixel or in the Moore neighborhood of the cell
    bool isNeutralized() const { return deadtrees!=NoDeadTrees; }

    static void resetCounters() { total_infested=0; max_iteration=0; }
    static int total_infested;
    static int max_iteration;

};

/**
 * @brief The BarkBeetleRUCell class collects information on resource unit (100m pixel) level.
 * This includes the number of bark beetle generations that are possible on given the climate and leaf area on the cell.
 * @ingroup barkbeetle
 */
class BarkBeetleRUCell
{
public:
    BarkBeetleRUCell(): scanned(false), generations(0.), add_sister(false),
        cold_days(0), cold_days_late(0), killed_trees(false),
        killed_pixels(0), host_pixels(0),
        infested(0.) {}
    /// relative damage: fraction of host pixels that died in the current or the last year
    double currentDamageFraction() { return host_pixels+killed_pixels>0? (killed_pixels)/double(host_pixels+killed_pixels): 0.; }
    bool scanned;
    double generations; // filial generations + 0.5 if full sisterbrood develops for last filial generation
    bool add_sister;
    int cold_days; // number of days in the winter season with t_min below a given threshold (-15 degree Celsius)
    int cold_days_late;
    bool killed_trees;
    int killed_pixels;
    int host_pixels;
    int infested; // number of pixels that are currently infested

};

/** Helper class manage and visualize data layers related to the barkbeetle module.
  @ingroup barkbeetle
*/
class BarkBeetleLayers: public LayeredGrid<BarkBeetleCell> {
  public:
    void setGrid(const Grid<BarkBeetleCell> &grid) { mGrid = &grid; }
    double value(const BarkBeetleCell& data, const int index) const;
    const QVector<LayeredGridBase::LayerElement> &names();
    bool onClick(const QPointF &world_coord) const;
private:
    QVector<LayeredGridBase::LayerElement> mNames;
};

class BarkBeetleRULayers: public LayeredGrid<BarkBeetleRUCell> {
  public:
    void setGrid(const Grid<BarkBeetleRUCell> &grid) { mGrid = &grid; }
    double value(const BarkBeetleRUCell& data, const int index) const;
    const QVector<LayeredGridBase::LayerElement> &names();
    bool onClick(const QPointF &world_coord) const;
private:
    QVector<LayeredGridBase::LayerElement> mNames;
};



class ResourceUnit; // forward
class Tree; // forward
class BarkBeetleOut; // forward
class Climate; // forward
/** The BarkBeetleModule class is the main class of the bark beetle module.
 * The module simulates the spruce bark beetle (Ips typographus) spatially explicit on the landscape.
 * The number of possible bark beetle generations is calculated based on climate data (BBGenerations)
  @ingroup barkbeetle
*/
class BarkBeetleModule
{
public:
    BarkBeetleModule();
    ~BarkBeetleModule();
    static double cellsize() { return 10.; }

    void setup(); ///< general setup
    void setup(const ResourceUnit *ru); ///< setup for a specific resource unit
    void loadParameters(bool do_reset=true); ///< load params from XML
    void clearGrids(); ///< reset the state of the internal grids (used for javascript based tests)
    void loadAllVegetation(); ///< scan the state of the vegetation of the full landscape

    /// main function to execute the bark beetle module (iteration can be non-0 when called from Javascript)
    void run(int iteration=0);

    /// function that is called whenever a tree dies somewhere in iLand
    void treeDeath(const Tree *tree);

    void yearBegin(); ///< called automatically
    /// call from script (from script)
    int manualYearBegin() { int y=mYear; yearBegin(); mYear = y + 1; return mYear; }
    int internalYear() const { return mYear; }
    // properties
    void setSimulate(bool do_simulate) { mSimulate = do_simulate; }
    bool simulate() const {return mSimulate; }

    void setEnabled(bool do_set_enabled) { mEnabled = do_set_enabled; }
    bool enabled() const { return mEnabled; }
private:
    void calculateGenerations(); ///< calculate on Resource Unit level the number of potential generations
    void calculateOutbreakFactor(); ///< calculate 'rc'-factor (climate sensitive outbreak sensitivity)
    void startSpread(); ///< beginning of a calculation
    int clumpedBackgroundActivation(QPoint start_idx); ///< start infestations clumped
    void prepareInteractions(bool update_interaction=false); ///< effect of dead trees (wind interactions), etc.
    void barkbeetleSpread(); ///< main function of bark beetle spread
    void barkbeetleKill(); ///< kill the trees on pixels marked as killed
    void scanResourceUnitTrees(const QPointF &position); ///< load tree data of the resource unit 'position' (metric) lies inside
    bool sanitationTreatment(QPointF coord) const; ///< returns true if no beetles should spread from a cell due to sanitation treatments
    //void calculateMeanDamage(); ///< calculate the mean damage percentage (fraction of killed pixels to host pixels)
    int mIteration;
    QString mAfterExecEvent;
    struct SBBParams {
        SBBParams(): minDbh(10.f), cohortsPerGeneration(30), cohortsPerSisterbrood(50),
            spreadKernelMaxDistance(100.), backgroundInfestationProbability(0.0001), initialInfestationProbability(0.),
            stormInfestationProbability(1.), winterMortalityBaseLevel(0.),
            outbreakDurationMin(0.), outbreakDurationMax(0.), deadTreeSelectivity(1.), sanitationTreatmentProb(0.) {}
        float minDbh; ///< minimum dbh of spruce trees that are considered as potential hosts
        int cohortsPerGeneration; ///< 'packages' of beetles that spread from an infested pixel
        int cohortsPerSisterbrood; ///< cohorts that spread from a pixel when a full sister brood developed
        QString spreadKernelFormula; ///< formula of the PDF for the BB-spread
        double spreadKernelMaxDistance; ///< upper limit for the spread distance (the kernel is cut at this distance)
        double backgroundInfestationProbability; ///< p that a pixel gets spontaneously infested each year
        double initialInfestationProbability; ///< p that a pixel is infested at startup (as a result of pre-simulation dynamics)
        double stormInfestationProbability; ///< p that a pixel with storm damage gets infested
        double winterMortalityBaseLevel; ///< p that a infested pixel dies out over the winter (due to antagonists, bad luck, ...)
        double outbreakDurationMin; ///< minimum value for the duration of a barkbeetle outbreak
        double outbreakDurationMax; ///< maximum value for the duration of a barkbeetle outbreak#
        double deadTreeSelectivity; ///< how effectively beetles are attracted by dead trees (e.g. windthrown) (5x5 pixel). 1: all beetles go into dead trees, 0: no effect of dead trees
        double sanitationTreatmentProb; ///< probability (0..1) that a sanitation treatment is effective for a cell (if so, no beetles spread from the cell)

    } params;
    struct SBBStats {
        void clear() { infestedStart=0;infestedBackground=0; infestedStorm=0; maxGenerations=0;NCohortsLanded=0;NPixelsLanded=0;NCohortsSpread=0;NInfested=0;NWinterMortality=0;NAreaKilled=0;NTreesKilled=0;BasalAreaKilled=0.; VolumeKilled=0.;}
        int infestedStart; // # of pixels that are infested at the beginning of an iteration
        int infestedBackground; // # of pixels that are getting active
        int infestedStorm; // # of pixels that are activated due to storm damage
        int maxGenerations; // maxium number of generations found this year
        int NCohortsLanded; // number of cohorts that landed on new potential host pixels
        int NPixelsLanded; // number of potential host pixels that received at least one cohort
        int NCohortsSpread; // number of pixels that are spread from infested cells
        int NInfested; // number of newly infested pixels (a subset of those who 'landed')
        int NWinterMortality; // number of (infested) pixels that died off during winter
        int NAreaKilled; // number of pixels on which trees were killed
        int NTreesKilled; // number of spruce trees killed
        double BasalAreaKilled; // sum of basal area of killed trees
        double VolumeKilled; // sum of killed tree volumes (m3)
    } stats;


    bool mSimulate; ///< true if bark beetle are only simulated, i.e. no trees get killed
    bool mEnabled; ///< if false, no bark beetles are simulates
    int mYear; ///< year (usually synchronized with iLand clock, but for testing purposes the module has a separate year)
    BBGenerations mGenerations;
    RandomCustomPDF mKernelPDF;
    Expression mColonizeProbability; ///< function that calculates probability of infestation for one landed beetle package given the trees' stress level
    Expression mWinterMortalityFormula; ///< temperature dependent winter mortality (more beetle die if there are more cold days)
    /// equation calculating the 'r_c' factor (modifying probability of outbreak linked to climate means)
    /// variables: <Var><season>; Var: [T|P]: T current temperature - average temperature, P: current precipitation / average precipitation, <season>: [spring, summer, autumn, winter]; e.g.: Tspring, Psummer
    Expression mOutbreakClimateSensitivityFormula;
    Expression mOutbreakDurationFormula;
    Grid<BarkBeetleCell> mGrid;
    Grid<BarkBeetleRUCell> mRUGrid;
    BarkBeetleLayers mLayers;
    BarkBeetleRULayers mRULayers;
    // reference climate
    QVector<double> mRefClimateAverages; ///< vector containing 4 reference temperatures, and 4 reference seasonal precipitation values (MAM, JJA, SON, DJF): Pspring, Psummer, Pautumn, Pwinter, Tspring, Tsummer, Tautumn, Twinter
    double *mClimateVariables[8]; // pointer to the variables
    const Climate *mRefClimate;
    double mRc; ///< climate sensitive outbreak probability: 1: this scales the backgroundOutbreakProbability, and is calculated by the respective sensitivity-Formula.

    friend class BarkBeetleScript;
    friend class BarkBeetleOut;

};



#endif // BARKBEETLEMODULE_H
