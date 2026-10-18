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

#include "bbgenerations.h"

#include "resourceunit.h"
#include "climate.h"
/** @class BBGenerations
    @ingroup beetlemodule
    BBGenerations calculates potential bark beetle generations based on climate data (including bark temperature).
  */
BBGenerations::BBGenerations()
{
}

/**
 * @brief BBGenerations::calculateGenerations
 * @param ru
 * @return the number of filial generation (i.e, main generations) + 0.5 if a sister brood develops also for the last generation
 */
double BBGenerations::calculateGenerations(const ResourceUnit *ru)
{
    calculateBarkTemperature(ru);

    // start at the 1. of April, and wait for 140.3 degree days (with a threhsold of 8.3 degrees)
    const ClimateDay *clim = ru->climate()->day(4-1,1-1); // 0-based indices
    const ClimateDay *last_day = ru->climate()->day(10-1,31-1); // 0-based indices -> Oct 31
    const ClimateDay * day_too_short = ru->climate()->dayOfYear(ru->climate()->sun().dayShorter14_5hrs()); // the first doy where the day is shorter than 14.5 hours

    double dd=0.;
    while (dd<140.3 && clim<last_day) {
        dd+=std::max(clim->max_temperature-8.3, 0.);
        ++clim;
    }
    // now wait for a decent warm day with tmax > 16.5 degrees
    while (clim->max_temperature<=16.5 && clim<last_day)
        ++clim;

    mGenerations.clear();
    // start with the first generation....
    BBGeneration bbgen;
    bbgen.start_day = ru->climate()->whichDayOfYear(clim);
    bbgen.is_sister_brood = false;
    bbgen.gen = 1; // the first generation
    mGenerations.append(bbgen);

    // process the generations
    int i=0;
    while (i<mGenerations.count()) {
        BBGeneration bb = mGenerations[i]; // deliberately make a copy and not a ref
        const ClimateDay *c = ru->climate()->dayOfYear(bb.start_day);
        int doy = bb.start_day;
        double base_temp = mEffectiveBarkTemp[doy];

        double t_sum=0.;
        bool added_sister_brood = false;
        while (c < last_day) {
            t_sum = (mEffectiveBarkTemp[doy]-base_temp) / 557.;
            if (t_sum>=1.) {
                if (c<day_too_short) {
                    // start a new parental generation (a full cycle), only if the current
                    // generation is also a filial generation.
                    if (bb.is_sister_brood)
                        mGenerations.append(BBGeneration(doy, true, bb.gen)); // sisterbrood: (true), keep counter of filial generation
                    else
                        mGenerations.append(BBGeneration(doy, false, bb.gen+1)); // new filial brood (false), start a new gen

                }
                break;
            } else if (t_sum>0.5 && !added_sister_brood) {
                // start a sister brood, *if* the maximum air temperature is high enough, and if the
                // length of the day > 14.5 hours
                if (c->max_temperature>16.5 && c<day_too_short) {
                    mGenerations.append(BBGeneration(doy, true, bb.gen)); // add a sister brood generation (true), keep gen. of originating brood
                    added_sister_brood = true;
                }
            }
            ++c; ++doy;
        }
        mGenerations[i].value = std::min(t_sum, 1.);
        ++i; // now process the next generation
    }


    mNSisterBroods = 0; mNFilialBroods = 0;

    for (i=0;i<mGenerations.count();++i) {
        if (mGenerations[i].value>0.6 && mGenerations[i].is_sister_brood==true)
            mNSisterBroods = std::max( mGenerations[i].gen, mNSisterBroods );

        if (mGenerations[i].value>0.6 && mGenerations[i].is_sister_brood==false)
            mNFilialBroods = mGenerations[i].gen;
    }

    // return the number of filial broods, and increase by 0.5 if a sister brood of the last filial generation has successfully developed.
    return mNFilialBroods + ( hasSisterBrood() ? 0.5 : 0.);

//    // now accumulate the generations
//    double filial_generations = 0.;
//    double sister_generations = 0.;
//    // accumulate possible number of offspring
//    const double offspring_factor = 25.; // assuming sex-ratio of 1:1 and 50 offspring per female (see p. 59)
//    int n=0;
//    int total_offspring = 0;
//    for (i=0;i<mGenerations.count();++i) {
//        if (mGenerations[i].value>0.6)  {
//            ++n;
//            total_offspring+= pow(offspring_factor, mGenerations[i].gen-1)*2.*offspring_factor;
//        }
//        if (mGenerations[i].value>0.6 && mGenerations[i].is_sister_brood==true)
//            sister_generations+=mGenerations[i].value;
//        if (mGenerations[i].value>0.6 && mGenerations[i].is_sister_brood==false)
//            filial_generations+=mGenerations[i].value;

//    }
//    if (logLevelDebug())
//        qDebug() << "rid" <<ru->id() << "filial/sister:" << filial_generations << sister_generations << "offspring:" << total_offspring << "started generations:" << n;
//    mNSisterBroods = sister_generations;
//    mNFilialBroods = filial_generations;

//    return filial_generations + (sister_generations>0?0.5:0);
}


/**
 * Calculate the bark temperatures for this year and a given resource unit.
 * Input: climate data (tmax (C), tmean (C), radiation (MJ/m2))
 * the LAI to estimate the radiation on the ground (Wh/m2)
 * Output: calculates for each day of the year the "effective" bark-temperature and saves a cumulative sum
 * Source: Schopf et al 2004: Risikoabschaetzung von Borkenkaefermassenkalamitaeten im Nationalpark Kalkalpen
 */
void BBGenerations::calculateBarkTemperature(const ResourceUnit *ru)
{
    // estimate the fraction of light on the ground (multiplier from 0..1)
    const double k = 0.5; // constant for the beer lambert function
    double ground_light_fraction = exp(-k * ru->leafAreaIndex() );

    mFrostDaysEarly=0;
    mFrostDaysLate=0;

    for (int i=0;i<ru->climate()->daysOfYear();++i) {
        const ClimateDay *clim = ru->climate()->dayOfYear(i);
        // radiation: MJ/m2/day -> the regression uses Wh/m2/day -> conversion-factor: 1/0.0036
        double rad_wh = clim->radiation*ground_light_fraction/0.0036;
        // calc. maximum bark temperature
        double bt_max=1.656 + 0.002955*rad_wh + 0.534*clim->max_temperature + 0.01884 * clim->max_temperature*clim->max_temperature;
        double diff_bt=0.;


        if (bt_max>=30.4)
            diff_bt=std::max(-310.667 + 9.603 * bt_max, 0.); // degree * hours

        // mean bark temperature
        double bt_mean=-0.173+0.0008518*rad_wh+1.054*clim->mean_temp();

        // effective:
        double bt_sum = std::max(bt_mean - 8.3, 0.)*24.; // degree * hours

        // corrected for days with very high bark temperature (>30.4 degrees)
        double bt_sum_eff = (bt_sum - diff_bt) / 24.; // degree * days

        mEffectiveBarkTemp[i] = (i>0? mEffectiveBarkTemp[i-1] : 0.) + bt_sum_eff;

        // frost days:
        if (clim->min_temperature < -15.) {
            if (i < ru->climate()->sun().longestDay())
                mFrostDaysEarly++;
            else
                mFrostDaysLate++;
        }

    }

}
