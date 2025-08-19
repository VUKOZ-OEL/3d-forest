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

#ifndef ESTABLISHMENT_H
#define ESTABLISHMENT_H
#include <QtCore/QPoint>
class Climate;
class ResourceUnitSpecies;

class Establishment
{
public:
    Establishment();
    Establishment(const Climate *climate, const ResourceUnitSpecies *rus);
    /// setup function that links to a climate and the resource unit / species
    void setup(const Climate *climate, const ResourceUnitSpecies *rus);
    void clear();
    void calculateAbioticEnvironment(); ///< calculate the abiotic environment (TACA model)
    void writeDebugOutputs();
    // some informations after execution
    double abioticEnvironment() const {return mPAbiotic; } ///< integrated value of abiotic environment (i.e.: TACA-climate + total iLand environment)
    int numberEstablished() const { return mNumberEstablished; } ///< return number of newly established trees in the current year
    bool TACAminTemp() const { return mTACA_min_temp;} ///< TACA flag for minimum temperature
    bool TACAchill() const { return mTACA_chill;} ///< TACA flag chilling requirement
    bool TACgdd() const { return mTACA_gdd;} ///< TACA flag for growing degree days
    int growingDegreeDays() const { return mGDD; } ///< the groing degree days (species specific)
    bool TACAfrostFree() const { return mTACA_frostfree;} ///< TACA flag for number of frost free days
    int TACAfrostDaysAfterBudBirst() const { return mTACA_frostAfterBuds; } ///< number of frost days after bud birst
    double avgLIFValue() const { return mLIFcount>0?mSumLIFvalue/double(mLIFcount):0.; } ///< average LIF value of LIF pixels where establishment is tested
    double waterLimitation() const { return mWaterLimitation; } ///< scalar value between 0 and 1 (1: no limitation, 0: no establishment)

private:
    double mPAbiotic; ///< abiotic probability for establishment (climate)
    /// calculate effect of water limitation on establishment, returns scalar [0..1]
    double calculateWaterLimitation();
    /// limitation if the depth of the soil organic layer is high (e.g. boreal forests)
    double calculateSOLDepthLimitation();

    const Climate *mClimate; ///< link to the current climate
    const ResourceUnitSpecies *mRUS; ///< link to the resource unit species (links to production data and species respones)
    // some statistics
    int mNumberEstablished; // number of established trees in the current year
    // TACA switches
    bool mTACA_min_temp; // minimum temperature threshold
    bool mTACA_chill;  // (total) chilling requirement
    bool mTACA_gdd;   // gdd-thresholds
    int mGDD; // growing degree days
    bool mTACA_frostfree; // frost free days in vegetation period
    int mTACA_frostAfterBuds; // frost days after bud birst
    double mSumLIFvalue;
    double mWaterLimitation; // scalar 0..1 signifying the drought limitation of establishment
    int mLIFcount;

};

#endif // ESTABLISHMENT_H
