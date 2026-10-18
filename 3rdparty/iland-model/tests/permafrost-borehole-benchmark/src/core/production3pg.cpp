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
#include "production3pg.h"

#include "resourceunit.h"
#include "species.h"
#include "speciesresponse.h"
#include "model.h"

Production3PG::Production3PG()
{
    clear();
    mResponse=0;
    mEnvYear = 0.;

}

/**
  This is based on the utilizable photosynthetic active radiation.
  @sa https://iland-model.org/primary+production
  The resulting radiation is MJ/m2       */
inline double Production3PG::calculateUtilizablePAR(const int month) const
{
    // calculate the available radiation. This is done at SpeciesResponse-Level (SpeciesResponse::calculate())
    // see Equation (3)
    // multiplicative approach: responses are averaged one by one and multiplied on a monthly basis
//    double response = mResponse->absorbedRadiation()[month] *
//                      mResponse->vpdResponse()[month] *
//                      mResponse->soilWaterResponse()[month] *
//                      mResponse->tempResponse()[month];
    // minimum approach: for each day the minimum aof vpd, temp, soilwater is calculated, then averaged for each month
    //double response = mResponse->absorbedRadiation()[month] *
    //                  mResponse->minimumResponses()[month];
    double response = mResponse->utilizableRadiation()[month];

    return response;
}
/** calculate the alphac (=photosynthetic efficiency) for the given month.
   this is based on a global efficiency, and modified per species.
   epsilon is in gC/MJ Radiation
  */
inline double Production3PG::calculateEpsilon(const int month) const
{
    double epsilon = Model::settings().epsilon; // maximum radiation use efficiency
    epsilon *= mResponse->nitrogenResponse() *
               mResponse->co2Response()[month];
    return epsilon;
}

inline double Production3PG::abovegroundFraction() const
{
    double utilized_frac = 1.;
    if (Model::settings().usePARFractionBelowGroundAllocation) {
        // the Landsberg & Waring formulation takes into account the fraction of utilizeable to total radiation (but more complicated)
        // we originally used only nitrogen and added the U_utilized/U_radiation
        utilized_frac = mResponse->totalUtilizeableRadiation() / mResponse->yearlyRadiation();
    }
    double harsh =  1. - 0.8/(1. + 2.5 * mResponse->nitrogenResponse() * utilized_frac);
    return harsh;
}

void Production3PG::clear()
{
    for (int i=0;i<12;i++) {
        mGPP[i] = 0.; mUPAR[i]=0.;
    }
    mEnvYear = 0.;
    mGPPperArea = 0.;
    mRootFraction = 0.;
}

/** calculate the stand-level NPP
  @ingroup core
  Standlevel (i.e ResourceUnit-level) production (NPP) following the 3PG approach from Landsberg and Waring.
  @sa https://iland-model.org/primary+production */
double Production3PG::calculate()
{
    Q_ASSERT(mResponse!=0);
    // Radiation: sum over all days of each month with foliage
    double year_raw_gpp = 0.;
    clear();
    double utilizable_rad, epsilon;
    // conversion from gC to kg Biomass: C/Biomass=0.5
    const double gC_to_kg_biomass = 1. / (biomassCFraction * 1000.);
    for (int i=0;i<12;i++) {
        utilizable_rad = calculateUtilizablePAR(i); // utilizable radiation of the month ... (MJ/m2)
        epsilon = calculateEpsilon(i); // ... photosynthetic efficiency ... (gC/MJ)
        mUPAR[i] = utilizable_rad ;
        mGPP[i] =utilizable_rad * epsilon * gC_to_kg_biomass; // ... results in GPP of the month kg Biomass/m2 (converted from gC/m2)
        year_raw_gpp += mGPP[i]; // kg Biomass/m2
    }

    // calculate f_env,yr: see https://iland-model.org/sapling+growth+and+competition
    double f_sum = 0.;
    for (int i=0;i<12;i++)
        f_sum += mGPP[i] / gC_to_kg_biomass; // == uAPar * epsilon_eff

    //  the factor f_ref: parameter that scales response values to the range 0..1 (1 for best growth conditions) (species parameter)
    const double perf_factor = mResponse->species()->saplingGrowthParameters().referenceRatio;
    // f_env,yr=(uapar*epsilon_eff) / (APAR * epsilon_0 * fref)
    mEnvYear = f_sum / (Model::settings().epsilon * mResponse->yearlyRadiation() * perf_factor);
    if (mEnvYear > 1.) {
        if (logLevelDebug() &&  mEnvYear>1.5) // warning for large deviations
            qDebug() << "WARNING: fEnvYear > 1 for " << mResponse->species()->id() << mEnvYear << "f_sum, epsilon, yearlyRad, refRatio" <<  f_sum << Model::settings().epsilon <<  mResponse->yearlyRadiation() << perf_factor
                     << "check calibration of the sapReferenceRatio (fref) for this species!";
        mEnvYear = 1.;
    }

    // calculate fraction for belowground biomass
    mRootFraction = 1. - abovegroundFraction();

    // global value set?
    double dbg = GlobalSettings::instance()->settings().paramValue("gpp_per_year",0);
    if (dbg>0.) {
        year_raw_gpp = dbg;
        mRootFraction = 0.4;
    }

    // year GPP/rad: kg Biomass/m2
    mGPPperArea = year_raw_gpp;
    return mGPPperArea; // yearly GPP in kg Biomass/m2
}
