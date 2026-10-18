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

#ifndef FIREMODULE_H
#define FIREMODULE_H

#include "grid.h"
#include "layeredgrid.h"
#include "expression.h"

class WaterCycleData;
class ResourceUnit;
class FireModule;
class FireOut;
class FireScript;

/** FireRUData contains data items for resource units.
    @ingroup firemodule
    Data items include:
    * parameters (KBDIref, ...)
    * fuel values
*/
class FireRUData
{
public:
    FireRUData(): mKBDIref(0.), mRefMgmt(0.), mRefAnnualPrecipitation(0.), mKBDI(0.), mValid(false), mRU(nullptr) { fireRUStats.clear(); fireRUStats.n_cum_fire=0; fireRUStats.year_last_fire=0; }
    void setup(const ResourceUnit *ru);
    bool valid() const { return mValid; } ///< returns false if the cell is not a valid resource unit (outside of project area)
    bool allowIgnition() const { return mRefMgmt>0.; }
    void reset() { mKBDI = 0.; }
    double kbdi() const { return mKBDI; }
    double kbdiRef() const { return mKBDIref; }
    double baseIgnitionProbability() const { return mBaseIgnitionProb; }
    double managementSuppression() const { return mRefMgmt; }
    const ResourceUnit *ru() const { return mRU; }

    // access data

    // statistics for the resource unit
    struct {
        int fire_id;
        // statistics for:
        int n_trees_died; ///< number of trees that are killed
        int n_trees; ///< number of trees that are on burning cells
        int n_cells; ///< number of burning cells
        int n_cum_fire; ///< number of fire events affecting the resource unit
        int year_last_fire; ///< the year of the last fire event on the RU
        double died_basal_area; ///< basal area (m2) of died trees
        double basal_area; ///< basal area (m2) of all trees on burning pixels
        double fuel_moss; ///< average affected moss biomass (kg/ha)
        double fuel_ff; ///< average fuel fine material (kg/ha)
        double fuel_dwd; ///< average fuel dead wood (kg/ha)
        double crown_kill; ///< average crown kill percent
        double avg_dbh; ///< average dbh (arithmetic mean) and limited to threshold
        // enter() can be called multiple times
        void enter(const int this_fire_id) {
            if (fire_id!=this_fire_id) {
                fire_id = this_fire_id;
                // clear all stats
                n_trees_died = n_trees = n_cells = 0;
                died_basal_area = basal_area = fuel_moss = fuel_ff = fuel_dwd = crown_kill = avg_dbh = 0.;
            }
        }
        // call once after fire is finished
        void calculate(const int this_fire_id, int current_year) {
            if (fire_id==this_fire_id) {
                // calculate averages
                if (n_cells>0) {
                    crown_kill /= double(n_cells);
                    //fuel_moss /= double(n_cells);
                    //fuel_ff /= double(n_cells);
                    //fuel_dwd /= double(n_cells);
                    avg_dbh /= double(n_cells);
                    n_cum_fire++;
                    year_last_fire = current_year;
                }
            }
        }
        void clear() { fire_id=-1;  enter(0);}
    } fireRUStats;
private:
    // parameters
    double mKBDIref; ///< reference value for KBDI drought index
    double mRefMgmt; ///< r_mgmt (fire suppression value)
    double mRefLand; ///< fixed multiplier for the fire spread probabilites (e.g. for riparian land) [0..1], default 1
    double mRefAnnualPrecipitation; ///< mean annual precipitation (mm)
    double mFireReturnInterval; ///< mean fire return interval (yrs)
    double mAverageFireSize; ///< mean average fire size (m2)
    double mMinFireSize; ///< minimum fire size (m2)
    double mMaxFireSize; ///< maximum fire size (m2)
    double mBaseIgnitionProb; ///< ignition probabilty for r_climate = r_mgmt = 1 (value is for the prob. for a cell, e.g. 20x20m)
    double mFireExtinctionProb; ///< gives the probabilty that a fire extincts on a pixel without having a chance to spread further
    // variables
    double mKBDI; ///< Keetch Byram Drought Index (0..800, in 1/100 inch of water)
    bool mValid; /// correctly set up?
    const ResourceUnit *mRU; ///< link to corresponding resource unit

    friend class FireModule; // allow access to member values
    friend class FireLayers;
};

/** Helper class manage and visualize data layers related to fire.
  @ingroup firemodule
*/
class FireLayers: public LayeredGrid<FireRUData> {
  public:
    void setData(const Grid<FireRUData> &grid, FireModule *module) { mGrid = &grid; mFireModule=module; }
    double value(const FireRUData& data, const int index) const;
    const QVector<LayeredGridBase::LayerElement> &names();
private:
    QVector<LayeredGridBase::LayerElement> mNames;
    FireModule *mFireModule;
};
/** FireModule is the main class of the fire sub module.
    @ingroup firemodule
    FireModule  holds all the relevant data/actions for the iLand fire module.
    See https://iland-model.org/wildfire and https://iland-model.org/fire+module

    The fire module has conceptually three parts that stand more or less on its own:
     * Fire ignition
     * Fire spread
     * Fire severity/effect
*/
class FireModule
{
public:
    FireModule();
    /// the setup function sets up the grids and parameters used for the fire module.
    /// this should be called when the main model is already created.
    void setup(); ///< general setup
    void setup(const ResourceUnit *ru); ///< setup for a specific resource unit
    void setScriptObj(FireScript *sm) {mFireScript = sm; }

    static double cellsize() { return 20.; }

    // actions
    /// main function that is executed yearly (called by the plugin)
    /// performs the major processes (ignition, spread, fire effect)
    void run();
    /// called yearly from the plugin to perform some
    /// cleanup.
    void yearBegin();
    /// called from the plugin to perform calculations (drought indices)
    /// during the water cycle routine.
    void calculateDroughtIndex(const ResourceUnit *resource_unit, const WaterCycleData *water_data);

    // main fire functions
    /// calculate the start and starting point of a possible fire. return: burnt area (-1 if nothing burnt)
    double ignition(bool only_ignite = false);
    ///  spread a fire starting from 'start_point' (index of the 20m grid)
    void spread(const QPoint &start_point, const bool prescribed = false);
    void severity();

    // helper functions
    int fireId() const { return mFireId; } ///< return the ID of the last fire event
    double fireX() const { return fireStats.startpoint.x(); } ///< coordinates of the ignition point
    double fireY() const { return fireStats.startpoint.y(); } ///< coordinates of the ignition point
    void testSpread();
    double prescribedIgnition(const double x_m, const double y_m, const double firesize, const double windspeed, const double winddirection);
    /// triggers a burn-in at 'x' and 'y' (iLand coordinate system) with a length (along the border) of 'length'. If 'simulate' is true, no fire spread happens
    /// returns the realized fire (m2)
    double burnInIgnition(const double x_m, const double y_m, const double length, double max_fire_size, bool simulate=false);

private:
    /// estimate fire size from a distribution
    double calculateFireSize(const FireRUData *data);

    // functions for the cellular automata
    void probabilisticSpread(const QPoint &start_point, QRect burn_in=QRect(), int burn_in_cells=0);
    /// calculates the probabibilty of spreading the fire from \p pixel_from to \p pixel_to.
    /// the \p direction provides encodes the cardinal direction.
    void calculateSpreadProbability(const FireRUData &fire_data, const double height, const float *pixel_from, float *pixel_to, const int direction);

    /// calc the effect of slope on the fire spread
    double calcSlopeFactor(const double slope) const;

    /// calc the effect of wind on the fire spread
    double calcWindFactor(const double direction) const;

    /// calculate the "severity", i.e. burn individual trees within the pixels
    bool burnPixel(const QPoint &pos, FireRUData &ru_data);
    /// calculate statistics, burn snags, soil of the resource units
    void afterFire();

    /// calculate combustible fuel
    /// returns the total combustible fuel (kg/ha), and sets the reference variables for forest floor and deadwood
    double calcCombustibleFuel(const FireRUData &ru_data, double &rMoss_kg_ha, double &rForestFloor_kg_ha, double &rDWD_kg_ha);

    /// prepare the necessary data grid to allow burn ins
    void setupBorderGrid();

    int mFireId; ///< running id of a fire event
    // parameters
    double mWindSpeedMin;
    double mWindSpeedMax;
    double mWindDirection;
    double mCurrentWindSpeed;
    double mCurrentWindDirection;
    double mPrescribedFiresize; // fire size from javascript
    double mMinimumFuel; ///< minimum fuel for fire (kg biomass/ha)
    // fuel parameters
    double mFuelkFC1; ///< params (Schumacher 2006) to calculate amount of fuel
    double mFuelkFC2;
    double mFuelkFC3;
    // crown kill
    double mCrownKillkCK1; ///< parameter to calculate fraction of crowns killed by the fire (Schumacher)
    double mCrownKillkCK2;
    double mCrownKillDbh; ///< dbh threshold (cm) for crown kill calculation
    Expression mMortalityFormula; ///< formula to calculate mortality caused by fire
    double *mFormula_bt;
    double *mFormula_ck;
    double mBurnSoilBiomass; ///< fraction of soil biomass that is to be removed when burning
    double mBurnStemFraction; ///< fraction of stem biomass burned by fire (if a tree dies)
    double mBurnBranchFraction; ///< fraction of branch biomass burned by fire (if a tree dies)
    double mBurnFoliageFraction; ///< fraction of foliage biomass burned by fire (if a tree dies)

    bool mOnlyFireSimulation; ///< if true, trees/snags etc. are not really affected by fire
    bool mAllowBurnIn; ///< true if burn ins at specific locations is enabled
    // event handler
    QString mAfterFireEvent; ///< javascript event after fire

    // data
    Grid<FireRUData> mRUGrid; ///< grid with data values per resource unit
    Grid<float> mGrid; ///< fire grid (20x20m)
    Grid<char> mBorderGrid; ///< 20x20m grid where border pixels are flagged
    FireLayers mFireLayers;
    FireScript *mFireScript;

    // functions
    FireRUData &data(const ResourceUnit *ru); ///< get ref to data element (FireData)

    // statistics
    struct {
        int iterations;
        int fire_size_plan_m2;
        int fire_size_realized_m2;
        double fire_psme_total; ///< psme (doug fir) on burning pixels (m2 basal area)
        double fire_psme_died; ///<  psme (doug fir) that died during the fire (based on m2)
        QPointF startpoint;
    } fireStats;
    friend class FireOut;
    friend class FireScript;
    friend class FireLayers;

};

#endif // FIREMODULE_H
