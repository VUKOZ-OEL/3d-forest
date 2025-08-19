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

#ifndef MODELSETTINGS_H
#define MODELSETTINGS_H
#include <QtCore>
#include "expression.h"
class ModelSettings
{
public:
    ModelSettings();
    void loadModelSettings();
    void print();
    // list of settings
    // general on/off switches
    bool growthEnabled; ///< if false, trees will apply/read light patterns, but do not grow
    bool mortalityEnabled; ///< if false, no natural (intrinsic+stress) mortality occurs
    bool regenerationEnabled; ///< if true, seed dispersal, establishment, ... is modelled
    bool carbonCycleEnabled; ///< if true, snag dynamics and soil CN cycle is modelled
    // light
    double lightExtinctionCoefficient; ///< "k" parameter (beer lambert) used for calc. of absorbed light on resourceUnit level
    double lightExtinctionCoefficientOpacity; ///< "k" for beer lambert used for opacity of single trees
    bool torusMode; ///< special mode that treats each resource unit as a "torus" (light calculation, seed distribution)
    // climate
    double temperatureTau; ///< "tau"-value for delayed temperature calculation acc. to Maekela 2008
    bool microclimateEnabled; ///< if true, iLand calculates microclimate variables at 10m resolution
    // water
    double airDensity; // density of air [kg / m3]
    double laiThresholdForClosedStands; // for calculation of max-canopy-conductance
    double boundaryLayerConductance; // 3pg-evapotranspiration
    // nitrogen and soil model
    bool useDynamicAvailableNitrogen; ///< if true, iLand utilizes the dynamically calculated NAvailable
    // site variables (for now!)
    double latitude; ///< latitude of project site in radians
    // production
    double epsilon; ///< maximum light use efficency used for the 3PG model
    bool usePARFractionBelowGroundAllocation; ///< if true, the 'correct' version of the calculation of belowground allocation is used (default=true)


};

#endif // MODELSETTINGS_H
