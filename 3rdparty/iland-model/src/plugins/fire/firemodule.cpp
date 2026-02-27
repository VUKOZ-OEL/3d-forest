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

#include "firemodule.h"
#include "firescript.h"

#include "grid.h"
#include "model.h"
#include "modelcontroller.h"
#include "globalsettings.h"
#include "resourceunit.h"
#include "watercycle.h"
#include "permafrost.h"
#include "climate.h"
#include "soil.h"
#include "dem.h"
#include "seeddispersal.h"
#include "outputmanager.h"
#include "species.h"
#include "3rdparty/SimpleRNG.h"
#include "debugtimer.h"

/** @defgroup firemodule iLand firemodule
  The fire module is a disturbance module within the iLand framework.

  See https://iland-model.org/wildfire for the science behind the module,
  and https://iland-model.org/fire+module for the implementation/ using side.
 */

//*********************************************************************************
//******************************************** FireData ***************************
//*********************************************************************************

void FireRUData::setup(const ResourceUnit *ru)
{
    mRU = ru;
    // data items loaded here are provided per resource unit
    XmlHelper xml(GlobalSettings::instance()->settings().node("modules.fire"));
    mKBDIref = xml.valueDouble(".KBDIref", 0.3);
    mRefMgmt = xml.valueDouble(".rFireSuppression", 1.);
    mRefLand = xml.valueDouble(".rLand", 1.);
    mRefAnnualPrecipitation = xml.valueDouble(".meanAnnualPrecipitation", -1);
    mAverageFireSize = xml.valueDouble(".averageFireSize", 10000.);
    mMinFireSize = xml.valueDouble(".minFireSize", 0.);
    mMaxFireSize = xml.valueDouble(".maxFireSize", 1000000.);
    mFireReturnInterval =  xml.valueDouble(".fireReturnInterval", 100); // every x year
    if (mAverageFireSize * mFireReturnInterval == 0.)
        throw IException("Fire-setup: invalid values for 'averageFireSize' or 'fireReturnInterval' (values must not be 0).");
    double p_base = 1. / mFireReturnInterval;
    // calculate the base ignition probabiility for a cell (eg 20x20m)
    mBaseIgnitionProb = p_base * FireModule::cellsize()*FireModule::cellsize() / mAverageFireSize;
    mFireExtinctionProb = xml.valueDouble(".fireExtinctionProbability", 0.);
    mValid = true;

}

//*********************************************************************************
//****************************************** FireLayers ***************************
//*********************************************************************************


double FireLayers::value(const FireRUData &data, const int param_index) const
{
    switch(param_index){
    case 0: return data.mBaseIgnitionProb; // base ignition value
    case 1: return data.mKBDI; // KBDI values
    case 2: return data.mKBDIref; // reference KBDI value
    case 3: return data.fireRUStats.fire_id; // the ID of the last recorded fire
    case 4: return data.fireRUStats.crown_kill; // crown kill fraction (average on resource unit)
    case 5: return data.fireRUStats.died_basal_area; // basal area died in the last fire
    case 6: return data.fireRUStats.n_trees > 0 ? data.fireRUStats.n_trees_died / double(data.fireRUStats.n_trees) : 0.;
    case 7: return data.fireRUStats.fuel_dwd + data.fireRUStats.fuel_ff + data.fireRUStats.fuel_moss; // fuel load (forest floor + dwd + moss) kg/ha
    case 8: return data.fireRUStats.n_cum_fire; // cum. number of fires
    case 9: return data.fireRUStats.year_last_fire; // year of the last fire event on a cell
    case 10: { double moss, ff, dwd;   // total fuel that would burn in a fire (current climatic conditions)
               return mFireModule->calcCombustibleFuel(data, moss, ff, dwd); }
    default: throw IException(QString("invalid variable index for FireData: %1").arg(param_index));
    }
}

const QVector<LayeredGridBase::LayerElement> &FireLayers::names()
{
    if (mNames.isEmpty())
        mNames= QVector<LayeredGridBase::LayerElement>()
                << LayeredGridBase::LayerElement(QStringLiteral("baseIgnition"), QStringLiteral("base ignition rate"), GridViewRainbow)
                << LayeredGridBase::LayerElement(QStringLiteral("KBDI"), QStringLiteral("KBDI"), GridViewRainbow)
                << LayeredGridBase::LayerElement(QStringLiteral("KBDIref"), QStringLiteral("reference KBDI value"), GridViewRainbow)
                << LayeredGridBase::LayerElement(QStringLiteral("fireID"), QStringLiteral("Id of the fire"), GridViewRainbow)
                << LayeredGridBase::LayerElement(QStringLiteral("crownKill"), QStringLiteral("crown kill rate"), GridViewRainbow)
                << LayeredGridBase::LayerElement(QStringLiteral("diedBasalArea"), QStringLiteral("m2 of died basal area"), GridViewRainbow)
                << LayeredGridBase::LayerElement(QStringLiteral("diedStemsFrac"), QStringLiteral("fraction of died stems"), GridViewRainbow)
                << LayeredGridBase::LayerElement(QStringLiteral("fuel"), QStringLiteral("burned fuel (forest floor + dwd + moss) kg/ha"), GridViewRainbow)
                << LayeredGridBase::LayerElement(QStringLiteral("nFire"), QStringLiteral("cumulative count of fires"), GridViewRainbow)
                << LayeredGridBase::LayerElement(QStringLiteral("lastFireYear"), QStringLiteral("sim. year of last burn"), GridViewRainbow)
                << LayeredGridBase::LayerElement(QStringLiteral("combustibleFuel"), QStringLiteral("available combustible fuel (current KBDI, forest floor + dwd + moss) kg/ha"), GridViewRainbow);

    return mNames;

}

//*********************************************************************************
//****************************************** FireModule ***************************
//*********************************************************************************



FireModule::FireModule()
{
    mFireLayers.setData(mRUGrid, this);
    mWindSpeedMin=10.;mWindSpeedMax=10.;
    mWindDirection=45.;
    mFireId = 0;
    mFireScript = nullptr;
}

FireModule::~FireModule()
{
    GlobalSettings::instance()->controller()->removeLayers(&mFireLayers);
}

// access data element
FireRUData &FireModule::data(const ResourceUnit *ru)
{
    QPointF p = ru->boundingBox().center();
    return mRUGrid.valueAt(p.x(), p.y());
}
void FireModule::setup()
{
    // setup the grid (using the size/resolution)
    mRUGrid.clear(); // force recreation (and clearing of statistics)
    mRUGrid.setup(GlobalSettings::instance()->model()->RUgrid().metricRect(),
                  GlobalSettings::instance()->model()->RUgrid().cellsize());
    // setup the fire spread grid
    mGrid.setup(mRUGrid.metricRect(), cellsize());
    mGrid.initialize(0.f);
    mFireId = 0;

    // set some global settings
    XmlHelper xml(GlobalSettings::instance()->settings().node("modules.fire"));
    mWindSpeedMin = xml.valueDouble(".wind.speedMin", 5.);
    mWindSpeedMax = xml.valueDouble(".wind.speedMax", 10.);
    mWindDirection = xml.valueDouble(".wind.direction", 270.); // defaults to "west"
    mMinimumFuel = xml.valueDouble(".minimumFuel", 0.05); // minimum fuel in kgBM/m2
    mMinimumFuel = mMinimumFuel * 10000.; // convert to kgBM/ha

    mOnlyFireSimulation = xml.valueBool(".onlySimulation", false);

    // fuel parameters
    mFuelkFC1 = xml.valueDouble(".fuelKFC1", 0.8);
    mFuelkFC2 = xml.valueDouble(".fuelKFC2", 0.2);
    mFuelkFC3 = xml.valueDouble(".fuelKFC3", 0.4);

    // parameters for crown kill
    mCrownKillkCK1 = xml.valueDouble(".crownKill1", 0.21111);
    mCrownKillkCK2 = xml.valueDouble(".crownKill2", -0.00445);
    mCrownKillDbh = xml.valueDouble(".crownKillDbh", 40.);

    QString formula = xml.value(".mortalityFormula", "1/(1 + exp(-1.466 + 1.91*bt - 0.1775*bt*bt - 5.41*ck*ck))");
    mFormula_bt = mMortalityFormula.addVar("bt");
    mFormula_ck = mMortalityFormula.addVar("ck");
    mMortalityFormula.setExpression(formula);

    mBurnSoilBiomass = xml.valueDouble(".burnSOMFraction", 0.);
    mBurnStemFraction = xml.valueDouble(".burnStemFraction", 0.1);
    mBurnBranchFraction = xml.valueDouble(".burnBranchFraction", 0.5);
    mBurnFoliageFraction = xml.valueDouble(".burnFoliageFraction", 1.);

    mAfterFireEvent = xml.value(".onAfterFire");

    mAllowBurnIn = xml.valueBool(".allowBurnIn", false);
    if (mAllowBurnIn) {
        // set up the grid of border flags
        mBorderGrid.setup(mRUGrid.metricRect(), cellsize());
        mBorderGrid.initialize(0);
        setupBorderGrid();
    }

    // setup of the visualization of the grid
    GlobalSettings::instance()->controller()->addLayers(&mFireLayers, "fire");
    GlobalSettings::instance()->controller()->addGrid(&mGrid, "fire spread", GridViewRainbow,0., 50.);

    // check if we have a DEM in the system
    if (!GlobalSettings::instance()->model()->dem())
        throw IException("FireModule:setup: a digital elevation model is required for the fire module!");
}

void FireModule::setup(const ResourceUnit *ru)
{
    if (mRUGrid.isEmpty())
        throw IException("FireModule: grid not properly setup!");
    FireRUData &fire_data = data(ru);
    fire_data.setup(ru);
}

/** yearBegin is called at the beginnig of every year.
    do some cleanup here.
  */
void FireModule::yearBegin()
{
// setting KBDI=0 is not really necessary; in addition: kbdi-grids are emtpy if grid export is called during management (between yearBegin() and run())
//for (FireRUData *fd = mRUGrid.begin(); fd!=mRUGrid.end(); ++fd)
//    fd->reset(); // reset drought index
}

/** main function of the fire module.

*/
void FireModule::run()
{
    if (GlobalSettings::instance()->settings().valueBool("modules.fire.enabled") == false)
        return;
    // run handler to call external / forced ignitions
    if (mFireScript->onIgnition().isCallable()) {
        mFireScript->onIgnition().call();
    }

    // ignition() calculates ignition and calls 'spread()' if a new fire is created.
    ignition();
}


/** perform the calculation of the KBDI drought index.
    see https://iland-model.org/wildfire#fire_ignition
  */
void FireModule::calculateDroughtIndex(const ResourceUnit *resource_unit, const WaterCycleData *water_data)
{
    FireRUData &fire_data = data(resource_unit);
    const ClimateDay *end = resource_unit->climate()->end();
    int iday = 0;
    double kbdi = 100.; // starting value of the year
    const double mean_ap = fire_data.mRefAnnualPrecipitation; // reference mean annual precipitation
    double dp, dq, tmax;

//  debug!!!
//    QFile dump("e:/kbdidump.txt");
//    dump.open(QFile::WriteOnly);
//    QTextStream ts(&dump);

    double kbdi_sum = 0.;
    for (const ClimateDay *day = resource_unit->climate()->begin(); day!=end; ++day, ++iday) {
        dp = water_data->water_to_ground[iday]; // water reaching the ground for this day
        double wetting = - dp/25.4 * 100.;
        kbdi += wetting;
        if (kbdi<0.) kbdi=0.;

        tmax = day->max_temperature;
        // drying is only simulated, if:
        // * the temperature > 10 degrees Celsius
        // * there is no snow cover
        if (tmax > 10. && water_data->snow_cover[iday]==0.) {
            // calculate drying: (kbdi already includes current wetting!)
            dq = 0.001*(800.-kbdi)*( (0.9676*exp(0.0486*(tmax*9./5.+32.))-8.299) / (1 + 10.88*exp(-0.0441*mean_ap/25.4)) );

            kbdi += dq;
        }
        kbdi_sum += kbdi;
//        // debug
//        ts << iday << ";" << water_data->water_to_ground[iday] << ";" << water_data->snow_cover[iday] << ";" << tmax << ";" << kbdi << endl;
    }
    // the effective relative KBDI is calculated
    // as the year sum related to the maximum value (800*365)
    fire_data.mKBDI = kbdi_sum / (365. * 800.);
}


/** evaluates the probability that a fire starts for each cell (20x20m)
    see https://iland-model.org/wildfire#fire_ignition

*/
double FireModule::ignition(bool only_ignite)
{
    DebugTimer t("Fire:ignition");
    int cells_per_ru = (cRUSize / cellsize()) * (cRUSize / cellsize()); // number of fire cells per resource unit
    bool has_handler = mFireScript->hasIgnitionRUHandler();

    int fires_count = 0;
    double total_area = 0.;

    for (FireRUData *fd = mRUGrid.begin(); fd!=mRUGrid.end(); ++fd)
        if (fd->allowIgnition() && fd->kbdi()>0.) {
            // calculate the probability that a fire ignites within this resource unit
            // the climate factor is the current drought index relative to the reference drought index
            double odds_base = fd->mBaseIgnitionProb / (1. - fd->mBaseIgnitionProb);
            double r_climate = fd->mKBDI / fd->mKBDIref;
            double management_effect = fd->mRefMgmt;
            if (has_handler)
                management_effect = mFireScript->calcDyanmicManagementEffect(fd);
            double odds = odds_base * r_climate / management_effect;
            // p_cell is the ignition probability for one 20x20m cell
            double p_cell = odds / (1. + odds);
            // p_cell is the probability of ignition for a "fire"-pixel. We scale that to RU-level by multiplying with the number of pixels per RU.
            // for small probabilities this yields almost the same results as the more correct 1-(1-p)^cells_per_ru [for p=0.0000001 and cells=25 the difference is 0.000000000003]
            // the probability is scaled down with to the actually stockable area of the resource unit
            //
            double cells = cells_per_ru * fd->mRU->stockableArea() / cRUArea;
            p_cell *= cells;
            if (!p_cell)
                continue;

            double p = drandom();

            if (p < p_cell) {
                // We have a fire event on the particular resource unit
                // now randomly select a pixel within the resource unit as the starting point
                int pixel_index = irandom(0, cells_per_ru);
                int ix = pixel_index % (int((cRUSize / cellsize())));
                int iy = pixel_index / (int((cRUSize / cellsize())));
                QPointF startcoord = mRUGrid.cellRect(mRUGrid.indexOf(fd)).bottomLeft();
                fireStats.startpoint = QPointF(startcoord.x() + (ix+0.5)*cellsize(), startcoord.y() + (iy+0.5)*cellsize() );
                QPoint startpoint = mGrid.indexAt(fireStats.startpoint);

                // check if we have enough fuel to start the fire: done in the spread routine
                // in this case "empty" fires (with area=0) are in the output

                // now start the fire!!!
                mFireId++; // this fire gets a new id
                qDebug() << "*** New fire event #"<< mFireId << " (#" << ++fires_count << " this year) ***";

                if (only_ignite) {
                    int idx, gen, refill;
                    RandomGenerator::debugState(idx, gen, refill);

                    //qDebug() << "test: rand-sum" << test_rand_sum << "n" << test_rand_n << pixel_index << startpoint << p_cell<< "rng:" << idx << gen << refill;
                    return p; // no real fire spread
                }

                spread(startpoint);

                // finalize statistics after fire event
                afterFire();

                // provide outputs: This calls the FireOut::exec() function
                GlobalSettings::instance()->outputManager()->execute("fire");

                total_area += fireStats.fire_size_realized_m2;
            }

        }
    // return the cumulative burned area in the current year
    if (fires_count>0)
        return total_area;

    return -1.; // nothing burnt

}

/** calculate the actual fire spread.
*/
void FireModule::spread(const QPoint &start_point, const bool prescribed)
{
    if (!mGrid.isIndexValid(start_point))
        return;

    qDebug() << "fire event starting at position" << start_point;

    mGrid.initialize(0.f);
    mGrid.valueAtIndex(start_point) = 1.f;
    for (FireRUData *fds = mRUGrid.begin(); fds!=mRUGrid.end(); ++fds)
        fds->fireRUStats.clear();

    if (!prescribed) {
        // randomly choose windspeed and wind direction
        mCurrentWindSpeed = nrandom(mWindSpeedMin, mWindSpeedMax);
        mCurrentWindDirection = fmod(mWindDirection + nrandom(-45., 45.)+360., 360.);
        mPrescribedFiresize = -1;
    }

    // choose spread algorithm
    probabilisticSpread(start_point);


}

/// Estimate fire size (m2) from a fire size distribution.
double FireModule::calculateFireSize(const FireRUData *data)
{
    // calculate fire size based on a negative exponential firesize distribution
    // to sample from a distribution function, use the inverse function:
    // https://en.wikipedia.org/wiki/Inverse_transform_sampling
    double size = -log(drandom()) * data->mAverageFireSize;
    size = qMin(size, data->mMaxFireSize);
    size = qMax(size, data->mMinFireSize);
    if (mFireScript->hasCalculateFireSizeHandler()) {
        double script_size = mFireScript->calculateFireSize(data, size);
        qDebug() << "Calculated fire size in 'onCalculateFireSize()' handler. Old value:" << size << ", new value (from JS):" << script_size;
        return script_size;
    }
    return size;

    // old code: uses a log normal distribution -- currently not used:
//    SimpleRNG rng;
//    rng.SetState(irandom(0, std::numeric_limits<unsigned int>::max()-1), irandom(0, std::numeric_limits<unsigned int>::max()-1));
//    double size = rng.GetLogNormal(log(average_fire_size), mFireSizeSigma);
//    return size;
}

/// calculate effect of slope on fire spread
/// for upslope following Keene and Albini 1976
///  It was designed by RKeane (2/2/99) (calc.c)
/// the downslope function is "not based on empirical data" (Keane in calc.c)
/// return is the metric distance to spread (and not number of pixels)
double FireModule::calcSlopeFactor(const double slope) const
{
    double slopespread;       /* Slope spread rate in pixels / timestep   */
    static double firebgc_cellsize = 30.; /* cellsize for which this functions were originally designed */

    if (slope < 0.) {
        // downslope effect
        slopespread = 1.0 - ( 20.0 * slope * slope );

    } else {
        // upslope effect
        static double alpha = 4.0; /* Maximum number of pixels to spread      */
        static double beta  = 3.5; /* Scaling coeff for inflection point      */
        static double gamma = 10.0;/* Scaling coeff for graph steepness       */
        static double zeta  = 0.0; /* Scaling coeff for y intercept           */

        slopespread = zeta + ( alpha / ( 1.0 + ( beta * exp( -gamma * slope ) ) ) );
    }


    return( slopespread ) * firebgc_cellsize;

}

/// calculate the effect of wind on the spread.
/// function designed by R. Keane, 2/2/99
/// @param direction direction (in degrees) of spread (0=north, 90=east, ...)
/// @return spread (in meters)
double FireModule::calcWindFactor(const double direction) const
{
    const double firebgc_cellsize = 30.; /* cellsize for which this functions were originally designed */
    double windspread;         /* Wind spread rate in pixels / timestep   */
    double coeff;              /* Coefficient that reflects wind direction*/
    double lwr;                /* Length to width ratio                   */
    const double alpha = 0.6; /* Wind spread power coeffieicnt           */
    const double MPStoMPH = 1. / 0.44704;

    /* .... If zero wind speed return 1.0 for the factor .... */
    if ( mCurrentWindSpeed <= 0.5 )
        return ( 1.0 ) * firebgc_cellsize; // not 0????

    /* .... Change degrees to radians .... */
    coeff = fabs( direction - mCurrentWindDirection ) * M_PI/180.;

    /* .... If spread direction equal zero, then spread direction = wind direct */
    if ( direction <= 0.01 )
        coeff = 0.0;

    /* .... Compute the length:width ratio from Andrews (1986) .....  */

    lwr = 1.0 + ( 0.125 * mCurrentWindSpeed * MPStoMPH );

    /* .... Scale the difference between direction between 0 and 1.0 .....  */
    coeff = ( cos( coeff ) + 1.0 ) / 2.0;

    /* .... Scale the function based on windspeed between 1 and 10...  */
    windspread = pow( coeff, pow( (mCurrentWindSpeed * MPStoMPH ), alpha ) ) * lwr;

    return( windspread ) * firebgc_cellsize;

}


/** calculates probability of spread from one pixel to one neighbor.
    In this functions the effect of the terrain, the wind and others are used to estimate a probability.
    @param fire_data reference to the variables valid for the current resource unit
    @param height elevation (m) of the origin point
    @param pixel_from pointer to the origin point in the fire grid
    @param pixel_to pointer to the target pixel
    @param direction codes the direction from the origin point (1..8, N, E, S, W, NE, SE, SW, NW)
  */
void FireModule::calculateSpreadProbability(const FireRUData &fire_data, const double height, const float *pixel_from, float *pixel_to, const int direction)
{
    const double directions[8]= {0., 90., 180., 270., 45., 135., 225., 315. };
    (void) pixel_from; // remove 'unused parameter' warning
    double spread_metric; // distance that fire supposedly spreads

    // calculate the slope from the curent point (pixel_from) to the spreading cell (pixel_to)
    double h_to = GlobalSettings::instance()->model()->dem()->elevation(mGrid.cellCenterPoint(mGrid.indexOf(pixel_to)));
    if (h_to==-1) {
        // qDebug() << "invalid elevation for pixel during fire spread: " << mGrid.cellCenterPoint(mGrid.indexOf(pixel_to));
        // the pixel is "outside" the project area. No spread is possible.
        return;
    }
    double pixel_size = cellsize();
    // if we spread diagonal, the distance is longer:
    if (direction>4)
        pixel_size *= 1.41421356;

    double slope = (h_to - height) / pixel_size;

    double r_wind, r_slope; // metric distance for spread
    r_slope = calcSlopeFactor( slope ); // slope factor (upslope / downslope)

    r_wind = calcWindFactor(directions[direction-1]); // metric distance from wind

    spread_metric = r_slope + r_wind;

    double spread_pixels = spread_metric / pixel_size;
    if (spread_pixels<=0.)
        return;

    double p_spread = pow(0.5, 1. / spread_pixels);
    // apply the r_land factor that accounts for different land types
    p_spread *= fire_data.mRefLand;
    // add probabilites
    *pixel_to = static_cast<float>(1. - (1. - *pixel_to)*(1. - p_spread));

}

/** a cellular automaton spread algorithm.
    @param start_point the starting point of the fire spread as index of the fire grid
*/
void FireModule::probabilisticSpread(const QPoint &start_point, QRect burn_in, int burn_in_cells)
{
    QRect max_spread = burn_in;
    if (burn_in.isNull()) {
        max_spread = QRect(start_point, start_point+QPoint(1,1));
        // grow the rectangle by one row/column but ensure validity
        max_spread.setCoords(qMax(start_point.x()-1,0),qMax(start_point.y()-1,0),
                             qMin(max_spread.right()+1,mGrid.sizeX()),qMin(max_spread.bottom()+1,mGrid.sizeY()) );
    }

    FireRUData *rudata = &mRUGrid.valueAt( mGrid.cellCenterPoint(start_point) );
    double fire_size_m2 = calculateFireSize(rudata);

    // for test cases, the size of the fire is predefined.
    if (mPrescribedFiresize>=0)
        fire_size_m2 = mPrescribedFiresize;

    fireStats.fire_size_plan_m2 = qRound(fire_size_m2);
    fireStats.iterations = 0;
    fireStats.fire_size_realized_m2 = 0;
    fireStats.fire_psme_died = 0.;
    fireStats.fire_psme_total = 0.;

    // calculate a factor describing how much larger/smaller the selected fire is compared to the average
    // fire size of the ignition cell
    double fire_scale_factor = fire_size_m2 / rudata->mAverageFireSize;

    int total_cells_to_burn = static_cast<int>(fire_size_m2 / (cellsize() * cellsize()));
    int cells_burned = 1;
    if (burn_in_cells>0)
        cells_burned = burn_in_cells; // cells already burnt during start of burn in

    int last_round_burned = cells_burned;
    double cum_fire_size = fire_size_m2 * cells_burned; // running sum of fire size per cell
    double fire_size_target = fire_size_m2; // running mean target fire size
    int iterations = 1;
    // main loop
    float *neighbor[8];
    float *p;

    rudata->fireRUStats.enter(mFireId);
    if (burn_in.isNull() && !burnPixel(start_point, *rudata)) {
        // no fuel / no trees on the starting pixel (don't run burn for burn ins)
        return;
    }
    while (cells_burned < total_cells_to_burn) {
        // scan the current spread area
        // and calcuate for each pixel the probability of spread from a burning
        // pixel to a non-burning pixel
        GridRunner<float> runner(mGrid, max_spread);
        while ((p = runner.next())) {
            if (*p == 1.f) {
                // p==1: pixel is burning in this iteration and might spread fire to neighbors
                const QPointF pt = mGrid.cellCenterPoint(mGrid.indexOf(p));
                FireRUData &fire_data = mRUGrid.valueAt(pt);
                fire_data.fireRUStats.enter(mFireId); // setup/clear statistics if this is the first pixel in the resource unit
                double h = GlobalSettings::instance()->model()->dem()->elevation(pt);
                if (h==-1) {
                    qDebug() << "Fire-Spread: invalid elevation at " << pt.x() << "/" << pt.y();
                    qDebug() << "value is: " << GlobalSettings::instance()->model()->dem()->elevation(pt);
                    return;
                }

                // current cell is burning.
                // check the neighbors: get an array with neighbors
                // 1-4: north, east, west, south
                // 5-8: NE/NW/SE/SW
                runner.neighbors8(neighbor);
                if (neighbor[0] && *(neighbor[0])<1.f)
                    calculateSpreadProbability(fire_data, h, p, neighbor[0], 1);
                if (neighbor[1] && *(neighbor[1])<1.f)
                    calculateSpreadProbability(fire_data, h, p, neighbor[1], 2);
                if (neighbor[2] && *(neighbor[2])<1.f)
                    calculateSpreadProbability(fire_data, h, p, neighbor[2], 3);
                if (neighbor[3] && *(neighbor[3])<1.f)
                    calculateSpreadProbability(fire_data, h, p, neighbor[3], 4);
                if (neighbor[4] && *(neighbor[4])<1.f)
                    calculateSpreadProbability(fire_data, h, p, neighbor[4], 5);
                if (neighbor[5] && *(neighbor[5])<1.f)
                    calculateSpreadProbability(fire_data, h, p, neighbor[5], 6);
                if (neighbor[6] && *(neighbor[6])<1.f)
                    calculateSpreadProbability(fire_data, h, p, neighbor[6], 7);
                if (neighbor[7] && *(neighbor[7])<1.f)
                    calculateSpreadProbability(fire_data, h, p, neighbor[7], 8);
                *p = iterations + 1;
            }
        }
        // now draw random numbers and calculate the real spread
        runner.reset();
        while ((p = runner.next())) {
            if (*p<1.f && *p>0.f) {
                if (drandom() < *p) {
                    // the fire spreads:
                    *p = 1.f;
                    FireRUData &fire_data = mRUGrid.valueAt(mGrid.cellCenterPoint(mGrid.indexOf(p)));
                    if (!fire_data.valid()) {
                        *p = 0.f; // reset
                        continue;
                    }
                    fire_data.fireRUStats.enter(mFireId);
                    cells_burned++;
                    // do the severity calculations:
                    // the function returns false if no trees are on the pixel
                    bool really_burnt = burnPixel(mGrid.indexOf(p), fire_data);
                    // update the fire size
                    cum_fire_size += fire_data.mAverageFireSize * fire_scale_factor;
                    //qDebug() << runner.currentIndex() << "avgfiresize" << fire_data.mAverageFireSize << "cum(ha)" << cum_fire_size / 10000. << " cells:" << cells_burned << "target(ha)" << cum_fire_size / static_cast<double>(cells_burned) / 10000.;
                    // the fire stops
                    //    (*) if no trees were on the pixel, or
                    //    (*) if the fire extinguishes
                    bool spread = really_burnt;
                    if (spread && fire_data.mFireExtinctionProb>0.) {
                        // exinguishing of fire is only effective, when at least the minimum fire size is already reached
                        if (cells_burned*cellsize()*cellsize() > fire_data.mMinFireSize) {
                            if (drandom() < fire_data.mFireExtinctionProb)
                                spread = false;
                        }

                    }
                    if (!spread)
                        *p = iterations + 1;

                } else {
                    *p = 0.f; // if the fire does note spread to the cell, the value is cleared again.
                }
            }
        }

        // update the cells to burn by factoring in different fire sizes within the fire-perimeter
        // see https://iland-model.org/wildfire+spread

        // weighted fire size = sum(fire_size_per_cell) / cells_burned
        fire_size_target = cum_fire_size / static_cast<double>(cells_burned);

        // total number of cells to burn for the fire (including the effects of changing mean fire size)
        total_cells_to_burn = static_cast<int>( fire_size_target / (cellsize() * cellsize()) );
        if (total_cells_to_burn <= cells_burned)
            break;

        // now determine the maximum extent with burning pixels...
        runner.reset();
        int left = mGrid.sizeX(), right = 0, top = mGrid.sizeY(), bottom = 0;
        while ((p = runner.next())) {
            if (*p == 1.f) {
                QPoint pt = mGrid.indexOf(p);
                left = qMin(left, pt.x()-1);
                right = qMax(right, pt.x()+2); // coord of right is never reached
                top = qMin(top, pt.y()-1);
                bottom = qMax(bottom, pt.y()+2); // coord bottom never reacher
            }
        }
        max_spread.setCoords(qMax(left,0),
                             qMax(top,0),
                             qMin(right, mGrid.sizeX()),
                             qMin(bottom, mGrid.sizeY()) );

        qDebug() << "Iter: " << iterations << "cells burned:" << cells_burned << "(from " << total_cells_to_burn << "), spread-rect:" << max_spread;
        iterations++;
        if (last_round_burned == cells_burned) {
            qDebug() << "Firespread: a round without new burning cells - exiting!";
            break;
        }
        last_round_burned = cells_burned;
        if (iterations > 10000) {
            qDebug() << "Firespread: maximum number of iterations (10000) reached!";
            break;
        }
    }
    qDebug() << "Fire:probabilistic spread: used " << iterations
             << "iterations. Planned (ha/cells):" << fire_size_target/10000. << "/" << total_cells_to_burn
             << "burned (ha/cells):" << cells_burned*cellsize()*cellsize()/10000. << "/" << cells_burned;

    fireStats.iterations = iterations-1;
    fireStats.fire_size_realized_m2 = qRound(cells_burned*cellsize()*cellsize());

}

void FireModule::testSpread()
{
//    QPoint pt = mGrid.indexAt(QPointF(1000., 600.));
//    spread( pt );
    SimpleRNG rng;
    rng.SetState(irandom(0, std::numeric_limits<unsigned int>::max()), irandom(0, std::numeric_limits<unsigned int>::max()));
    int bins[20];
    for(int i=0;i<20;i++) bins[i]=0;
    for (int i=0;i<10000;i++) {
        double value = rng.GetLogNormal(log(2000.),0.25);
        if (value>=0 && value<10000.)
            bins[(int)(value/500.)]++;
    }
    for(int i=0;i<20;i++)
        qDebug() << bins[i];

    for (int r=0;r<360;r+=90) {
        mWindDirection = r;
        for (int i=0;i<5;i++) {
            QPoint pt = mGrid.indexAt(QPointF(730., 610.)); // was: 1100/750
            mFireId++; // this fire gets a new id

            spread( pt );
            // stats
            for (FireRUData *fds = mRUGrid.begin(); fds!=mRUGrid.end(); ++fds)
                fds->fireRUStats.calculate(mFireId, GlobalSettings::instance()->currentYear());

            GlobalSettings::instance()->controller()->repaint();
            GlobalSettings::instance()->controller()->saveScreenshot(GlobalSettings::instance()->path(QString("%1_%2.png").arg(r).arg(i), "temp"));
        }
    }
}


double FireModule::prescribedIgnition(const double x_m, const double y_m, const double firesize, const double windspeed, const double winddirection)
{
    QPoint pt = mGrid.indexAt(QPointF(x_m, y_m));
    if (!mGrid.isIndexValid(pt)) {
        qDebug() << "Fire starting point is not valid!";
        return -1.;
    }
    mFireId++; // this fire gets a new id

    mPrescribedFiresize = firesize; // if -1, then a fire size is estimated

    if (windspeed>=0) {
        mCurrentWindSpeed = windspeed;
        mCurrentWindDirection = winddirection;
    }
    DebugTimer t("Fire:prescribedIgnition");
    spread( pt, true );

    afterFire();
    mPrescribedFiresize = -1; // reset

    // provide outputs: This calls the FireOut::exec() function
    GlobalSettings::instance()->outputManager()->execute("fire");
    GlobalSettings::instance()->outputManager()->save();

    return fireStats.fire_size_realized_m2;
}

double FireModule::burnInIgnition(const double x_m, const double y_m, const double length, double max_fire_size, bool simulate)
{
    // step 1: find the closest edge pixel to the given coordinates
    if (!mBorderGrid.coordValid(static_cast<float>(x_m), static_cast<float>(y_m)))
        throw IException(QString("FireModule:burnInIgnition: invalid coordinates! x=%1 y=%2").arg(x_m).arg(y_m));
    QPoint pos = mBorderGrid.indexAt(QPointF(x_m, y_m));
    QRectF search_rect = mBorderGrid.cellRect(pos);
    search_rect.adjust(-100., -100., 100., 100.); // 220m rectangle
    GridRunner<char> runner(mBorderGrid, search_rect);
    QPoint closest;
    bool found=false;
    double min_dist=9999999.;
    while (runner.next()) {
        if (*runner.current() == 1) {
            found = true;
            QPointF cell = runner.currentCoord();
            double dist = (cell.x() - x_m)*(cell.x() - x_m) + (cell.y() - y_m)*(cell.y() - y_m);
            if (dist < min_dist) {
                min_dist = dist;
                closest = runner.currentIndex();
            }
        }
    }
    if (!found)
        throw IException(QString("FireModule:burnInIgnition: no edge found close to: x=%1 y=%2").arg(x_m).arg(y_m));

    // step 2: apply a flood fill algorithm to the border
    int px_to_fill = qRound(length / cellsize());
    int filled = mBorderGrid.floodFill(closest, 1, 2, px_to_fill);
    qDebug() << "burnInIgnition: Starting point" << x_m << "/" << y_m << " closest cell" << closest << "to fill" << px_to_fill << "filled:" << filled;


    // in simulation mode we are done here (and keep the updated border grid)
    if (simulate)
        return static_cast<double>(filled);

    // step 3: start the fire
    mFireId++;
    mPrescribedFiresize = max_fire_size;

    // determine bounding box and execute
    // burn for all pixels of the burn-in fire front
    mGrid.initialize(0.f);
    for (FireRUData *fds = mRUGrid.begin(); fds!=mRUGrid.end(); ++fds)
        fds->fireRUStats.clear();
    float *f = mGrid.begin();
    int init_burned = 0;
    int left = mGrid.sizeX(), right = 0, top = mGrid.sizeY(), bottom = 0;
    for (char *c = mBorderGrid.begin(); c!=mBorderGrid.end(); ++c, ++f) {
        if (*c == 2) {
            *f = 1.f; // set as burning
            QPoint pt = mGrid.indexOf(f);
            FireRUData &fire_data = mRUGrid.valueAt(mGrid.cellCenterPoint(pt)); // get the RU
            fire_data.fireRUStats.enter(mFireId);
            bool really_burnt = burnPixel(pt, fire_data); // burn the px
            if (really_burnt)
                ++init_burned;
            left = qMin(left, pt.x()-1);
            right = qMax(right, pt.x()+2); // coord of right is never reached
            top = qMin(top, pt.y()-1);
            bottom = qMax(bottom, pt.y()+2); // coord bottom never reacher
            *c = 1; // reset the border state
        }
    }
    QRect burn_box;
    burn_box.setCoords(qMax(left,0),
                       qMax(top,0),
                       qMin(right, mGrid.sizeX()),
                       qMin(bottom, mGrid.sizeY()) );

    qDebug() << "burn-in:" << init_burned << "of" << filled << "px could burn (enough veg/fuel)";

    probabilisticSpread(closest, burn_box, init_burned);
    afterFire();

    mPrescribedFiresize = -1; // reset

    return fireStats.fire_size_realized_m2;

}

/** burning of a single 20x20m pixel. see https://iland-model.org/wildfire.
   The function is called from the fire spread function.
   @return boolean true, if any trees were burned on the pixel

  */
bool FireModule::burnPixel(const QPoint &pos, FireRUData &ru_data)
{
    // extract a list of trees that are within the pixel boundaries
    QRectF pixel_rect = mGrid.cellRect(pos);
    ResourceUnit *ru = GlobalSettings::instance()->model()->ru(pixel_rect.center());
    if (!ru)
        return false;

    // retrieve a list of trees within the active pixel
    // NOTE: the check with isDead() is necessary because dead trees could be already in the trees list
    QVector<Tree*> trees;
    QVector<Tree>::iterator tend = ru->trees().end();
    for (QVector<Tree>::iterator t = ru->trees().begin(); t!=tend; ++t) {
        if ( pixel_rect.contains( (*t).position() ) && !(*t).isDead()) {
            trees.push_back(&(*t));
        }
    }

    // calculate mean values for dbh
    double sum_dbh = 0.;
    double sum_ba = 0.;
    double avg_dbh = 0.;
    foreach (const Tree* t, trees) {
        sum_dbh += t->dbh();
        sum_ba += t->basalArea();
    }

    if(trees.size()>0)
        avg_dbh = sum_dbh / static_cast<double>( trees.size() );

    // (1) calculate fuel
    double fuel_moss,fuel_ff, fuel_dwd;
    double fuel = calcCombustibleFuel(ru_data, fuel_moss, fuel_ff, fuel_dwd); // kg BM/ha

    // if fuel level is below 0.05kg BM/m2 (=500kg/ha), then no burning happens!
    // note that it is not necessary that trees are on the pixel, as long as there is enough fuel on the ground.
    if (fuel < mMinimumFuel)
        return false;

    const double cell_fraction = cellsize()*cellsize() / cRUArea;
    ru_data.fireRUStats.n_cells++; // number of cells burned in the resource unit

    // we add a fraction of the total fuel of the RU (in kg/ha) to the fire-stats
    ru_data.fireRUStats.fuel_ff += fuel_ff * cell_fraction; // fuel in kg/cell Biomass

    ru_data.fireRUStats.fuel_moss += fuel_moss * cell_fraction; // kg/cell moss biomass

    if (ru->soil() && ru_data.fireRUStats.fuel_ff > ru->soil()->youngLabile().biomass()*1000.)
        qWarning() << "!!!burnPixel: invalid fuel. now: " << ru_data.fireRUStats.fuel_ff <<  ", this px: " << fuel_ff*cell_fraction << "labile: " << ru->soil()->youngLabile().biomass()*1000. << ", RU-index: " << ru->index();

    ru_data.fireRUStats.fuel_dwd += fuel_dwd * cell_fraction; // fuel in kg/cell Biomass
    ru_data.fireRUStats.n_trees += trees.size();
    ru_data.fireRUStats.basal_area += sum_ba;

    //qDebug() << "fireid" << mFireId << "RU-index:" << ru->index() << "pos" << pos << "ncells" << ru_data.fireRUStats.n_cells << "comb.fuel.ff" << fuel_ff << "fuel.ru.ff" << ru_data.fireRUStats.fuel_ff << "ru.ff.max" << ru->soil()->youngLabile().biomass()*1000.*ru->soil()->youngLabileAbovegroundFraction();

    if (!mOnlyFireSimulation) {
        //  effect of forest fire on saplings: all saplings are killed.
        //  As regeneration happens before the fire routine, any newly regenarated saplings are killed as well.
        //  Note: re-sprouting after fire from adult trees happens after killing the regeneration
        if (GlobalSettings::instance()->model()->saplings())
            GlobalSettings::instance()->model()->saplings()->clearSaplings(pixel_rect, true, true);
        //ru->clearSaplings(pixel_rect, true); [old version]
    }

    // (2) calculate the "crownkill" fraction
    const double dbh_trehshold = mCrownKillDbh; // dbh
    const double kck1 = mCrownKillkCK1;
    const double kck2 = mCrownKillkCK2;
    if (avg_dbh > dbh_trehshold)
        avg_dbh = dbh_trehshold;

    double crown_kill_fraction = (kck1+kck2*avg_dbh)*fuel/1000.; // fuel: to t/ha
    crown_kill_fraction = limit(crown_kill_fraction, 0., 1.); // limit to 0..1


    // (3) derive mortality of single trees
    double p_mort;
    int died = 0;
    double died_basal_area=0.;
    bool tree_is_psme;
    foreach (Tree* t, trees) {
        // the mortality probability depends on the thickness of the bark:
        *mFormula_bt = t->barkThickness(); // cm
        *mFormula_ck = crown_kill_fraction; // fraction of crown that is killed (0..1)
        p_mort = mMortalityFormula.execute();
        // note: 5.41 = 0.000541*10000, (fraction*fraction) = 10000 * pct*pct
        //p_mort = 1. / (1. + exp(-1.466 + 1.91*bt - 0.1775*bt*bt - 5.41*crown_kill_fraction*crown_kill_fraction));
        tree_is_psme = t->species()->id()=="Psme";
        if (tree_is_psme)
            fireStats.fire_psme_total += t->basalArea();
        if (drandom() < p_mort) {
            // the tree actually dies.
            died_basal_area += t->basalArea();
            if (tree_is_psme)
                fireStats.fire_psme_died += t->basalArea();

            if (t->species()->seedDispersal() && t->species()->isTreeSerotinous(t->age()) ) {
                //SeedDispersal *sd = t->species()->seedDispersal();
                t->species()->seedDispersal()->seedProductionSerotiny(t);
            }

            if (!mOnlyFireSimulation) {
                // before tree biomass is transferred to the snag-state, a part of the biomass is combusted:
                t->setDeathReasonFire();
                t->removeBiomassOfTree(mBurnFoliageFraction, mBurnBranchFraction, mBurnStemFraction);
                // kill the tree and calculate flows to soil/snags
                t->removeDisturbance(0., 1., // 100% of the remaining stem goes to snags
                                     0., 1., // 100% of the remaining branches go to snags
                                     1.); // the remaining foliage goes to soil

            }
            ++died;
            // some statistics???
        }
    }

    // update statistics
    ru_data.fireRUStats.n_trees_died += died;
    ru_data.fireRUStats.died_basal_area += died_basal_area;
    ru_data.fireRUStats.crown_kill += crown_kill_fraction;
    ru_data.fireRUStats.avg_dbh += avg_dbh;

    return true;
}

/// do some cleanup / statistics after the fire.
/// apply the effect of fire on dead wood pools and soil pools of the resource units
/// biomass of living trees is consumed in the burnPixel() routine.
void FireModule::afterFire()
{
    DebugTimer t("Fire:afterFire");
    const double pixel_fraction = cellsize()*cellsize() / cRUArea; // fraction of one pixel, default: 0.04 (20x20 / 100x100)

    int year = GlobalSettings::instance()->currentYear();
    for (FireRUData *fds = mRUGrid.begin(); fds!=mRUGrid.end(); ++fds) {
        fds->fireRUStats.calculate(mFireId, year);
        if (fds->fireRUStats.fire_id == mFireId && fds->fireRUStats.n_cells>0) {
            // a fire happened on this resource unit,
            // so we need to update snags/soil pools
            if (!mOnlyFireSimulation) {
                //ResourceUnit *ru = GlobalSettings::instance()->model()->ru(ru_idx);
                const ResourceUnit *ru = fds->ru();
                double ru_fraction = fds->fireRUStats.n_cells * pixel_fraction; // fraction of RU burned (0..1)

                if (ru && ru->soil()) {
                    // (1) effect of forest fire on the dead wood pools. fuel_dwd and fuel_ff is the amount of fuel
                    //     available on the pixels that are burnt. The assumption is: all of it was burnt.
                    ru->soil()->disturbanceBiomass(fds->fireRUStats.fuel_dwd, fds->fireRUStats.fuel_ff, 0.);

                    // (2) remove also a fixed fraction of the biomass that is in the soil
                    if (mBurnSoilBiomass>0.) {
                        ru->soil()->disturbance(0.,0., mBurnSoilBiomass*ru_fraction);
                    }
                    // (3) effect on the snags
                    ru->snag()->removeCarbon(mBurnStemFraction*ru_fraction);

                    // /(4) effect on life moss
                    if (ru->waterCycle()->permafrost())
                        const_cast<Water::Permafrost*>(ru->waterCycle()->permafrost())->burnMoss(fds->fireRUStats.fuel_moss);
                }
            }
            // notify iLand that a fire happened. info = proportion of area burned on the RU
            fds->ru()->notifyDisturbance(ResourceUnit::dtFire, fds->fireRUStats.n_cells * pixel_fraction);
        }
    }

    // execute the after fire event
    if (!mAfterFireEvent.isEmpty()) {
        // evaluate the javascript function...
        GlobalSettings::instance()->executeJavascript(mAfterFireEvent);
    }
}

double FireModule::calcCombustibleFuel(const FireRUData &ru_data, double &rMoss_kg_ha, double &rForestFloor_kg_ha, double &rDWD_kg_ha)
{
    // (1) calculate fuel
    const double kfc1 = mFuelkFC1;
    const double kfc2 = mFuelkFC2;
    const double kfc3 = mFuelkFC3;

    const ResourceUnit *ru = ru_data.ru();
    if (!ru){
        rMoss_kg_ha=rForestFloor_kg_ha=rDWD_kg_ha=0.;
        return 0.;
    }


    // retrieve values for fuel.
    // forest_floor: sum of leaves and twigs (t/ha) = aboveground fraction of the yR pool
    // DWD: downed woody debris (t/ha) = yL pool

    // fuel per ha (kg biomass): derive available fuel using the KBDI as estimate for humidity.
    double fuel_ff = (kfc1 + kfc2*ru_data.kbdi()) * (ru->soil()? ru->soil()->youngLabile().biomass() * ru->soil()->youngLabileAbovegroundFraction() * 1000. : 0.);

    // life moss biomass - Note: moss in kg/m2; use the same fraction as for forest floor litter
    double fuel_moss = 0.;
    if (ru->waterCycle()->permafrost())
        fuel_moss = (kfc1 + kfc2*ru_data.kbdi()) * ru->waterCycle()->permafrost()->mossBiomass() * 10000.;

    // Note: when carbon cycle is disabled (and Soil/Snags etc not available), we
    // assume a non-zero amount of DWD (10t/ha), to allow the fire module to run
    double fuel_dwd = kfc3*ru_data.kbdi() * (ru->soil() ? ru->soil()->youngRefractory().biomass() * ru->soil()->youngRefractoryAbovegroundFraction() * 1000. : 10000. );
    // calculate fuel (kg biomass / ha)
    double fuel = fuel_ff + fuel_dwd + fuel_moss;

    rMoss_kg_ha = fuel_moss;
    rForestFloor_kg_ha = fuel_ff;
    rDWD_kg_ha = fuel_dwd;
    return fuel;

}

void FireModule::setupBorderGrid()
{
    HeightGrid *hg = GlobalSettings::instance()->model()->heightGrid();
    GridRunner<HeightGridValue> runner(*hg, hg->metricRect());
    HeightGridValue* neighbors[8];
    while (runner.next()) {
        if (runner.current()->isValid()) {
            runner.neighbors8(neighbors);
            for (int i=0;i<8;++i)
                if (neighbors[i] &&  !neighbors[i]->isValid()) {
                    //runner.current()->setIsRadiating();
                    // this is a pixel at the edge to non-project area
                    char &bgv = mBorderGrid.valueAt(runner.currentCoord());
                    if (bgv == '\0') {
                        bgv = 1; // mark cell as a border
                    }

                }

        }
    }

}










