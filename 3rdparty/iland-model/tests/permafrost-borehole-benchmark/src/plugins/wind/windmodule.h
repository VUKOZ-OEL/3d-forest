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
#ifndef WINDMODULE_H
#define WINDMODULE_H
#include "grid.h"
#include "layeredgrid.h"
#include "expression.h"
#include <QHash>

class Tree; // forward
class Species; // forward
class ResourceUnit; // forward

/** data structure for a single wind cell (usually 10x10m).
  @ingroup windmodule

*/
class WindCell {
public:
    WindCell() { topex = 0; n_affected=0; sum_volume_killed=0.; edge_age=0; clear(); }
    void clear() {height = edge = 0.f; n_trees=0.f; tree=0; n_killed = 0; basal_area_killed = 0.f; cws_uproot = 0.; cws_break= crown_windspeed= 0.; n_iteration = 0;}
    bool isValid() const { return height<9999.f; } ///< returns true if the pixel is on the valid project area
    float topex; ///< topographic modifier for wind speed (-)
    float height; ///< top height (m).
    float n_trees; ///< number of trees on pixel
    const Tree *tree; ///< pointer to the tallest tree on the pixel (if already populated)
    float edge; ///< maximum difference to neighboring cells (m)
    // statistics
    int n_iteration; ///< number of iteration this pixel is processed (and trees are killed)
    int n_killed; ///< number of trees killed on the pixel
    int edge_age; ///< age of an edge (consecutive number of years of being an edge)
    double basal_area_killed; ///< basal area of trees that died (m2) (during an event)
    double cws_uproot; ///< critital wind speed for uprooting (m/s)
    double cws_break; ///< critical wind speed for tree breakage (m/s)
    double crown_windspeed; ///< wind speed (m/s) on the cecll
    int n_affected; ///< number of storm that killed trees on that pixel.
    double sum_volume_killed; ///< running sum of killed tree volume on that pixel (m3)
};
// data structure for a resource unit
class WindRUCell {
public:
    WindRUCell(): flag(0), soilIsFrozen(false) {}
    int flag; // flag to indicate that the trees of the current resource are already processed
    bool soilIsFrozen;
};

/** Helper class manage and visualize data layers related to fire.
  @ingroup windmodule
*/
class WindLayers: public LayeredGrid<WindCell> {
  public:
    void setGrid(const Grid<WindCell> &grid) { mGrid = &grid; }
    double value(const WindCell& data, const int index) const;
    const QVector<LayerElement> &names();
    // specifics for wind layers
    void setRUGrid(const Grid<WindRUCell> *grid) { mRUGrid = grid; }
private:
    QVector<LayerElement> mNames;
    double ruValueAt(const WindCell *cell, const int what) const; // get resource-unit level factor at cell "cell"
    const Grid<WindRUCell> *mRUGrid;
};

/** Species parameters that are specific to the wind module
  */
struct WindSpeciesParameters
{
    WindSpeciesParameters():  crown_area_factor(0.5), crown_length(0.5), Creg(111), MOR(30.6), wet_biomass_factor(1.86) {}
    double crown_area_factor; // empirical factor related to the crown shape (fraction of crown shape compared to rectangle)
    double crown_length; // crown length of the tree (fraction of tree height)
    double Creg; // Nm/kg, critical turning coefficient from tree pulling
    double MOR; // MPa, modulus of rupture
    double wet_biomass_factor; // conversion factor between dry and wet biomass (wet = dry*factor)
};

/** @class WindModule
    @ingroup windmodule
    The WindModule is the disturbance module for simulation wind and windthrow in iLand.  */
class WindModule
{
public:
    WindModule();
    static double cellsize() { return 10.; }
    /// the general setup routine after starting iland
    void setup();
    void setupResourceUnit(const ResourceUnit* ru);
    WindLayers &layers() { return mWindLayers; }

    /// main function of the disturbance module
    void run(const int iteration=-1, const bool execute_from_script=false);

    // test functions
    void setWindProperties(const double direction_rad, const double speed_ms) { mWindDirection = direction_rad; mWindSpeed = speed_ms; }
    void setSimulationMode(const bool mode) { mSimulationMode = mode; }
    bool simulationMode() const { return mSimulationMode; }
    void setMaximumIterations(const double maxit) { mMaxIteration = maxit; }

    void testFetch(double degree_direction);
    void testEffect();
private:
    // main functions
    bool eventTriggered(); ///< determine details of this years' wind event (and return false if no event happens)
    void initWindGrid(); ///< load state from iland main module
    void detectEdges(bool at_startup=false); ///< detect all pixels that are higher than the surrounding and therefore are likely candidates for damage
    void calculateFetch(); ///< calculate maximum gap sizes in upwind direction
    int calculateWindImpact(); ///< do one round of wind effect calculations

    // details
    /// find distance to the next pixels that give shelter
    bool checkFetch(const int startx, const int starty, const double direction, const double max_distance, const double threshold) ;
    /// perform the wind effect calculations for a given grid cell
    bool windImpactOnPixel(const QPoint position, WindCell *cell);
    ///
    double calculateCrownWindSpeed(const Tree *tree, const WindSpeciesParameters &params, const double n_trees, const double wind_speed_10);
    double calculateCrititalWindSpeed(const Tree *tree, const WindSpeciesParameters &params, const double gap_length, double &rCWS_uproot, double &rCWS_break);
    /// check if the soil on current resource unit is frozen or not
    bool isSoilFrozen(const ResourceUnit *ru, const int day_of_year) const;

    // helping functions
    void scanResourceUnitTrees(const QPoint &position);
    void loadSpeciesParameter(const QString &table_name);
    const WindSpeciesParameters &speciesParameter(const Species *s);
    void initializeEdgeAge(const int years);
    void incrementEdgeAge();

    // variables
    bool mTopexFromGrid; ///< indicates if topex grid is a real grid or by ressource unit
    double mWindDirection; ///< direction of the current wind event (rad)
    double mWindDirectionVariation; ///< random variation in wind direction
    double mWindSpeed; ///< wind speed (TODO: per resource unit!)
    double mEdgeDetectionThreshold; ///< minimum height difference of height-grid pixels to be detected as edges (default is 10m)
    double mFactorEdge; ///< constant ratio between the maximujm turning moments at the stand edge and conditions well inside the forest (default: 5)
    int mWindDayOfYear; ///< day of year of the wind event (0..365)
    bool mSimulationMode; ///< if true, no trees are removed (test mode)
    int mCurrentIteration; ///< current iteration (1..n)
    int mMaxIteration; ///< maximum number of iterations
    double mGustModifier; ///< Modification range accounting for differences in wind speed between iterations (0..1)
    double mCurrentGustFactor; ///< gustfactor of the current year (multiplier)
    enum ETopexFactorModificationType {gfMultiply, gfAdd} mTopexFactorModificationType; ///< determines if topo-modifier is added multiplicatively or additively.
    double mIterationsPerMinute; ///< number of iterations per minute of the events' duration
    int mEdgeAgeBaseValue; ///< initial value for the age of edges in the initial state of the forest
    Expression mEdgeProbability; ///< function that determines the probability that an edge is calculated (old edges are less likely to being tested)
    double mEdgeBackgroundProbability; ///< probability that a 10x10m pixel is marked as edge during a wind event (randomly)
    // some statistics
    int mPixelAffected; ///< total number of pixels that are impacted
    int mTreesKilled; ///< total number of killed trees
    double mTotalKilledBasalArea; ///< total basal area of killed trees
    double mTotalKilledVolume; ///< total tree volume (m3) killed
    enum ESoilFreezeMode {esfFrozen, esfNotFrozen, esfAuto, esfInvalid} mSoilFreezeMode; ///< if "esfAuto", soil-freeze-state is derived from climate
    Grid<WindCell> mGrid; ///< wind grid (10x10m)
    Grid<WindRUCell> mRUGrid; ///< grid for resource unit data
    WindLayers mWindLayers; ///< helping structure
    // species parameters for the wind module
    QHash<const Species*, WindSpeciesParameters> mSpeciesParameters;
    /// formula for the transfer function LRI
    Expression mLRITransferFunction;

    QString mAfterExecEvent;

    friend class WindScript;
    friend class WindOut;
    friend void nc_calculateFetch(WindCell *begin, WindCell *end);
    friend void nc_calculateWindImpact(ResourceUnit *unit);

};



#endif // WINDMODULE_H
