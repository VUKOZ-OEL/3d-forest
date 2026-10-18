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
#include "windmodule.h"
#include "globalsettings.h"
#include "debugtimer.h"
#include "model.h"
#include "modelcontroller.h"
#include "helper.h"
#include "tree.h"
#include "species.h"
#include "speciesset.h"
#include "resourceunit.h"
#include "climate.h"
#include "gisgrid.h"
#include "outputmanager.h"

/** @defgroup windmodule iLand windmodule
  The wind module is a disturbance module within the iLand framework.

  FIXME  !!
   See https://iland-model.org/wind for the science behind the module,
  and https://iland-model.org/fire+module for the implementation/ using side.
 */


/** @class WindModule

    The WindModule is the main object of the iLand wind module. The setup() function creates the data structures,
    and the run() function is the main entry point. The main functions are detectEdges(), calculateFetch() and
    calculateImpact().
  */


/** @class WindCell
    The WindCell is the basic pixel of the wind spread algorithm. The default size is 10x10m

  */

double WindLayers::value(const WindCell &data, const int param_index) const
{
    switch(param_index){
    case 0: return data.height==9999.f?-1:data.height; // height
    case 1: return data.edge; // maximum difference to neighboring cells
    case 2: return data.cws_uproot; // critical wind speed for uprooting
    case 3: return data.cws_break; // critical wind speed for stem breakage
    case 4: return data.n_killed; // trees killed on pixel
    case 5: return data.basal_area_killed; // basal area killed on pixel
    case 6: return data.n_iteration; // iteration in processing that the current pixel is processed (and trees are killed)
    case 7: return data.crown_windspeed; // effective wind speed in the crown (m/s)
    case 8: return data.topex; // topo modifier of the current pixel
    case 9: return ruValueAt(&data, 0); // 1 if soil is frozen on the current pixel
    case 10: return data.n_affected; // number of storm events affecting the pixel
    case 11: return data.sum_volume_killed; // sum of killed volume
    case 12: return data.edge_age; // # of years that a cell is an edge
    case 13: return data.n_trees; // # of trees
    default: throw IException(QString("invalid variable index for a WindCell: %1").arg(param_index));
    }
}


const QVector<LayeredGridBase::LayerElement> &WindLayers::names()
{
    if (mNames.isEmpty())
        mNames= QVector<LayeredGridBase::LayerElement>()
                << LayeredGridBase::LayerElement(QLatin1Literal("height"), QLatin1Literal("max height at pixel (m)"), GridViewRainbow)
                << LayeredGridBase::LayerElement(QLatin1Literal("edge"), QLatin1Literal("result of edge detection"), GridViewRainbow)
                << LayeredGridBase::LayerElement(QLatin1Literal("cwsUproot"), QLatin1Literal("critical wind speed uprooting (m/s)"), GridViewRainbow)
                << LayeredGridBase::LayerElement(QLatin1Literal("cwsBreak"), QLatin1Literal("critical wind speed stem breakage (m/s)"), GridViewRainbow)
                << LayeredGridBase::LayerElement(QLatin1Literal("treesKilled"), QLatin1Literal("trees killed on pixel"), GridViewRainbow)
                << LayeredGridBase::LayerElement(QLatin1Literal("basalAreaKilled"), QLatin1Literal("killed basal area"), GridViewRainbow)
                << LayeredGridBase::LayerElement(QLatin1Literal("iteration"), QLatin1Literal("iteration # of the spread algorithm"), GridViewRainbow)
                << LayeredGridBase::LayerElement(QLatin1Literal("windSpeedCrown"), QLatin1Literal("wind speed at tree crown height (m/s)"), GridViewRainbow)
                << LayeredGridBase::LayerElement(QLatin1Literal("topo"), QLatin1Literal("the topography modifier for wind speeds"), GridViewRainbow)
                << LayeredGridBase::LayerElement(QLatin1Literal("isFrozen"), QLatin1Literal("soil (resource unit) is frozen?"), GridViewRainbow)
                << LayeredGridBase::LayerElement(QLatin1Literal("nEvents"), QLatin1Literal("number of events (total since start of simulation) that killed trees on a pixel."), GridViewReds)
                << LayeredGridBase::LayerElement(QLatin1Literal("sumVolume"), QLatin1Literal("running sum of damaged tree volume on the pixel."), GridViewReds)
                << LayeredGridBase::LayerElement(QLatin1Literal("edgeAge"), QLatin1Literal("age of an edge (consecutive number of years that a cell is an edge)."), GridViewBlues)
                << LayeredGridBase::LayerElement(QLatin1Literal("basalArea"), QLatin1Literal("sum of basal area (trees>4m) on the cel.l"), GridViewRainbow);
    return mNames;
}

// helper function (avoid a special ru-level grid and use the 10m cell resolution instead)
double WindLayers::ruValueAt(const WindCell *cell, const int what) const
{
    QPoint pos = mGrid->indexOf(cell);
    const WindRUCell &ru_cell = mRUGrid->constValueAt(mGrid->cellCenterPoint(pos));
    switch (what) {
    case 0: return ru_cell.soilIsFrozen?1.:0.;
    default: return -1.;
    }
}

WindModule::WindModule()
{
    mWindDirection = 0.;
    mWindSpeed = 0.;
    mSimulationMode = false;
    mMaxIteration = 10;
    mWindDirectionVariation = 0.;
    mGustModifier = 1.;
    mIterationsPerMinute = 1.;
    mEdgeDetectionThreshold = 10.;
    mFactorEdge = 5.;
    mTopexFactorModificationType = gfMultiply;
}

/// setup of general settings from the project file.
/// the function is invoked from the Plugin.
void WindModule::setup()
{
    // setup the grid (using the size/resolution)
    mRUGrid.setup(GlobalSettings::instance()->model()->RUgrid().metricRect(),
                  GlobalSettings::instance()->model()->RUgrid().cellsize());
    // setup the wind grid
    mGrid.clear(); // force a recreate (incl. c'tor)
    mGrid.setup(GlobalSettings::instance()->model()->heightGrid()->metricRect(), cellsize());
    //mFireId = 0;

    // set some global settings
    XmlHelper xml(GlobalSettings::instance()->settings().node("modules.wind"));
    mWindDirectionVariation = xml.valueDouble(".directionVariation", 0.) * M_PI/180.;
    mWindDirection = xml.valueDouble(".direction", 0.)*M_PI/180.;
    mWindSpeed = 0.; // set the wind speed explicitely to 0
    mGustModifier = xml.valueDouble(".gustModifier", 1.);
    mEdgeDetectionThreshold = xml.valueDouble(".edgeDetectionThreshold", 10.);
    mFactorEdge = xml.valueDouble(".factorEdge", 5.);
    QString topex_mod_typ = xml.value(".topexModifierType", "multiplicative");
    mTopexFactorModificationType = gfMultiply;
    if (topex_mod_typ ==  "additive")
        mTopexFactorModificationType = gfAdd;

    mIterationsPerMinute = 1. / xml.valueDouble(".durationPerIteration", 10.); // default: 10mins/iteration is 60m/h
    mWindDayOfYear = xml.valueInt(".dayOfYear", 100.);
    mLRITransferFunction.setAndParse(xml.value(".LRITransferFunction", "max(min(3.733-6.467*LRI,3.41),0.5)"));

    // edge age
    mEdgeProbability.setAndParse(xml.value(".edgeProbability", "1")); // default value: all pixels are tested always
    mEdgeAgeBaseValue = xml.valueInt(".edgeAgeBaseValue", 10);
    mEdgeBackgroundProbability = xml.valueDouble(".edgeBackgroundProbability", 0.);
    // topographic topex modifier
    mTopexFromGrid = false;
    QString topex_grid_file = xml.value(".topoGridFile");
    if (!topex_grid_file.isEmpty()) {
        // load the topex-grid from file and assign the values
        GisGrid topex_grid;
        topex_grid_file = GlobalSettings::instance()->path(topex_grid_file);
        if (topex_grid.loadFromFile(topex_grid_file)) {
            for (int i=0;i<mGrid.count();i++) {
                mGrid.valueAtIndex(i).topex = static_cast<float>( topex_grid.value(mGrid.cellCenterPoint(mGrid.indexOf(i))) );
            }
            mTopexFromGrid = true;
        }
    }

    // soil freeze state
    mSoilFreezeMode=esfInvalid;
    QString soil_freeze = xml.value(".soilFreezeMode", "auto");
    if (soil_freeze=="yes") mSoilFreezeMode=esfFrozen;
    if (soil_freeze=="no") mSoilFreezeMode=esfNotFrozen;
    if (soil_freeze=="auto") mSoilFreezeMode=esfAuto;
    if (mSoilFreezeMode==esfInvalid)
        throw IException("WindModule::setup: parameter 'soilFreezeMode' has invalid value. Allowed: yes, no, auto.");

    mWindLayers.setGrid(mGrid);
    mWindLayers.setRUGrid(&mRUGrid);
    GlobalSettings::instance()->controller()->addLayers(&mWindLayers, "wind");

    mAfterExecEvent = xml.value(".onAfterWind");

    // setup the species parameters that are specific to the wind module
    QString parameter_table_name = xml.value(".speciesParameter", "wind");
    loadSpeciesParameter(parameter_table_name);
}

/// setup of spatial explicit variables (e.g. the wind speed modifier)
/// the function is called from the plugin-object.
void WindModule::setupResourceUnit(const ResourceUnit *ru)
{
    if (!mTopexFromGrid) {
        float topo_value =  static_cast<float>( GlobalSettings::instance()->settings().valueDouble("modules.wind.topoModifier", 1.) );
        GridRunner<WindCell> runner(mGrid, ru->boundingBox());
        while (WindCell *p=runner.next()) {
            p->topex = topo_value;
        }
    }
}

/// load specific species parameter for the wind module.
/// wind related species parameter are located in a separate sqlite-table.
void WindModule::loadSpeciesParameter(const QString &table_name)
{
    QSqlQuery query(GlobalSettings::instance()->dbin());
    QString sql = QString("select * from %1").arg(table_name);
    query.exec(sql);
    if (query.lastError().isValid()){
        throw IException(QString("Error loading species parameters for wind module: %1 \n %2").arg(sql, query.lastError().text()) );
    }
    mSpeciesParameters.clear();
    int iID=query.record().indexOf("shortName");
    int iCreg = query.record().indexOf("CReg");
    int iCrownArea = query.record().indexOf("crownAreaFactor");
    int iCrownLength = query.record().indexOf("crownLength");
    int iMOR = query.record().indexOf("MOR");
    int iWet = query.record().indexOf("wetBiomassFactor");
    if (iID==-1 || iCreg==-1 || iCrownArea==-1 || iCrownLength==-1 || iMOR==-1 || iWet==-1) {
        throw IException(QString("Error in wind parameter table '%1'. A required column was not found.").arg(table_name));
    }
    QString species_id;
    while (query.next()) {
        species_id = query.value(iID).toString();
        Species *s = GlobalSettings::instance()->model()->speciesSet()->species(species_id);
        if (s) {
            WindSpeciesParameters &p = mSpeciesParameters[s];
            p.Creg = query.value(iCreg).toDouble();
            p.crown_area_factor = query.value(iCrownArea).toDouble();
            p.crown_length = query.value(iCrownLength).toDouble();
            p.MOR = query.value(iMOR).toDouble();
            p.wet_biomass_factor = query.value(iWet).toDouble();
        }
    }
    qDebug() << "wind:"  << mSpeciesParameters.count() << "species parameter vectors loaded.";
}

const WindSpeciesParameters &WindModule::speciesParameter(const Species *s)
{
    if (!mSpeciesParameters.contains(s))
        throw IException(QString("WindModule: no wind species parameter for species '%1'").arg(s->id()));
    return mSpeciesParameters[s];
}

void WindModule::initializeEdgeAge(const int years)
{
    detectEdges(true); // in startup-mode
    int val = qMax(years,0);
    int n=0;
    for (WindCell *p=mGrid.begin(); p!=mGrid.end(); ++p) {
        if (p->edge_age>=0 && p->edge==1.f) {
            // skip out-of-project area
            p->edge_age = val;
            ++n;
        }
    }
    qDebug() << "Wind:initializeEdgeAge: set" << n << "edges to" << years << "years.";
}

void WindModule::incrementEdgeAge()
{
    int n_background=0;
    int n_disabled=0;
    for (WindCell *p=mGrid.begin(); p!=mGrid.end(); ++p) {
        if (p->edge_age>0) {
            if (!p->tree || p->height<10.f) {
                p->edge_age = 0;
            } else {

                if (mEdgeBackgroundProbability>0.)
                    if (drandom() < mEdgeBackgroundProbability) {
                        p->edge = 10.f; //10m is the minium upwind gap size for edges activated by background probability
                        ++n_background;
                    }
                if (p->edge<=1.f) {
                    // calculate probability that the pixel is disabled for the event
                    // (only if this pixel is not a random-edge)
                    double p_edge = mEdgeProbability.calculate(static_cast<double>(p->edge_age));
                    if (drandom() < p_edge) {
                        p->edge = -2.f; // the edge is disabled for this event
                        ++n_disabled;
                    }
                }
                p->edge_age++;
            }
        }
    }
    qDebug() << "Wind:incrementEdgeAge: background activation of" << n_background << "potential start edges (with trees), protection not active for" << n_disabled << "px.";

}

/// the run() function executes the wind events
void WindModule::run(const int iteration, const bool execute_from_script)
{
    // initialize things in the first iteration
    if (iteration<=0) {

        if (!execute_from_script) {
            setSimulationMode(false);
            initWindGrid();
            if (GlobalSettings::instance()->currentYear() == 1) {
                initializeEdgeAge(mEdgeAgeBaseValue - 1);
            } else {
                detectEdges(); //  needed for the increment
            }
            // edges get one year older (even if no wind event is triggered this year)
            incrementEdgeAge();
        }

        if (!execute_from_script) {
            // check if we have a wind event this year
            if (!eventTriggered())
                return;
        }

    }


    bool finished = false;
    mCurrentIteration = 1;
    if (iteration>=0) mCurrentIteration = iteration;
    DebugTimer ttotal("wind:total");
    while (!finished) {
        // check for edges in the stand
        DebugTimer titeration("wind:Cycle");
        // detect current edges in the forest
        detectEdges();

        // calculate the gap sizes/fetch for the current structure
        calculateFetch();
        // wind speed of the current iteration
        mCurrentGustFactor = 1. + nrandom(-mGustModifier, mGustModifier);
        // derive the impact of wind (i.e. calculate critical wind speeds and the effect of wind on the forest)
        int pixels = calculateWindImpact();
        mPixelAffected += pixels;
        if (++mCurrentIteration > mMaxIteration)
            finished = true;
        if (iteration>=0) // step by step execution
            finished = true;
        qDebug() << "wind module: iteration" << mCurrentIteration-1
                 << "this round affected:" << pixels
                 << "total:" << mPixelAffected
                 << "totals: killed trees:" << mTreesKilled
                 << "basal-area:" << mTotalKilledBasalArea
                 << "gustfactor:" << mCurrentGustFactor;
    }
    qDebug() << "iterations: " << mCurrentIteration << "total pixels affected:" << mPixelAffected << "totals: killed trees:" << mTreesKilled << "basal-area:" << mTotalKilledBasalArea;

    GlobalSettings::instance()->outputManager()->execute("wind");
    GlobalSettings::instance()->outputManager()->save();

    // execute the after wind event
    if (!mAfterExecEvent.isEmpty()) {
        // evaluate the javascript function...
        GlobalSettings::instance()->executeJavascript(mAfterExecEvent);
    }

}

void WindModule::initWindGrid()
{
    DebugTimer t("wind:init");
    mWindDayOfYear = GlobalSettings::instance()->settings().valueInt("modules.wind.dayOfYear");
    // reset some statistics
    mTotalKilledBasalArea = 0.;
    mTotalKilledVolume = 0.;
    mTreesKilled = 0;
    mPixelAffected = 0;
    // as long as we have 10m -> easy!
    if (cellsize()==cHeightPerRU) {
        WindCell *p = mGrid.begin();
        for (const HeightGridValue *hgv=GlobalSettings::instance()->model()->heightGrid()->begin(); hgv!=GlobalSettings::instance()->model()->heightGrid()->end(); ++hgv, ++p) {
            p->clear();
            if (hgv->isValid()) {
                // p->height = hgv->height;
                //p->n_trees = hgv->count();

            } else {
                // the "height" of pixels not in the project area depends on a flag provided with the "stand map"
                if (hgv->isForestOutside()) {
                    p->height = 9999.f;
                    p->edge_age = -1;
                } else {
                    p->height = 0.f;
                    p->edge_age = -1;
                }
            }
        }
        // reset resource unit grid...
        for (WindRUCell *cell=mRUGrid.begin(); cell!=mRUGrid.end(); ++cell) {
            cell->flag = 0;
            scanResourceUnitTrees(mGrid.indexAt(mRUGrid.cellCenterPoint(mRUGrid.indexOf(cell))));
        }
        return;
    }
    throw IException("WindModule::initWindGrid: not 10m of windpixels...");
}

/** mark all pixels that are at stand edges, i.e. pixels with trees that are much taller (treeheight) than their neighbors.
  */
void WindModule::detectEdges(bool at_startup)
{
    DebugTimer t("wind:edges");
    WindCell *p_above, *p, *p_below;
    int dy = mGrid.sizeY();
    int dx = mGrid.sizeX();
    int x,y;

    const float threshold = static_cast<float>( mEdgeDetectionThreshold );
    for (y=1;y<dy-1;++y){
        p = mGrid.ptr(1,y);
        p_above = p - dx; // one line above
        p_below = p + dx; // one line below
        for (x=1;x<dx-1;++x,++p,++p_below, ++p_above) {
            if (at_startup)
                p->edge=0.f;
            if (p->n_trees>0) {
                float min_h = p->height - threshold; // max_h: if no surrounding pixel is lower than this value, then there is no edge here
                // edges are only detected if trees are >10m high
                if (p->height>10.f && min_h > 0) {
                    // check 8-neighborhood. if one of those pixels exceed the threshold, then the current
                    // pixel is not an "edge".
                    int edge = ((p_above-1)->height < min_h) +
                            ((p_above)->height < min_h) +
                            ((p_above+1)->height < min_h) +
                            ((p-1)->height < min_h)  +
                            ((p+1)->height < min_h) +
                            ((p_below-1)->height < min_h)+
                            ((p_below)->height < min_h) +
                            ((p_below+1)->height < min_h) ;

                    if (edge>2) {
                        // set flag for gap size determination (randomly activated edges are overwritten here,
                        // but edges disabled due to the age of the edge not (=-2)
                        if (at_startup) {
                            p->edge = 1.f;
                        } else {
                            // we exclude age-protected pixels only in the first iteration
                            //if (mCurrentIteration!=1 || p->edge>-1.f)
                            if (p->edge>-1.f)
                                p->edge = qMax(p->edge, 1.f);
                            // start couting the edge age for new edges
                            if (!mSimulationMode && p->edge_age==0)
                                p->edge_age=1;
                        }
                    }
                }
            }
            // reset age counter
            if (!mSimulationMode && p->edge_age>0.f && p->edge==0.f)
                p->edge_age = 0;
        }
    }
}

static QMutex wind_mt_counter;
static WindModule *wind_module=0;
static int impact_c;

// multithreading enabled main function for fetch calculations
void nc_calculateFetch(WindCell *begin, WindCell *end)
{
    int calculated = 0;
    double current_direction;
    for (WindCell *p=begin; p!=end; ++p) {
        if (p->edge >= 1.f) {
            QPoint pt=wind_module->mGrid.indexOf(p);
            current_direction = wind_module->mWindDirection + (wind_module->mWindDirectionVariation>0.?nrandom(-wind_module->mWindDirectionVariation, wind_module->mWindDirectionVariation):0);
            float old_edge = p->edge;
            wind_module->checkFetch(pt.x(), pt.y(), current_direction, p->height * 10., p->height - wind_module->mEdgeDetectionThreshold);
            ++calculated;
            // only simulate edges with gapsize > 20m
            // this skips small gaps (e.g. areas marked as "stones")
            if (old_edge>1.f) {
                // for random starts only increase the distance
                p->edge = qMax(p->edge, old_edge);
            } else if (p->edge < 10.f) {
                // minimum fetch distance for edges
                p->edge = 0.f;
            }
        }
    }
    QMutexLocker lock(&wind_mt_counter);
    impact_c += calculated;

}

void WindModule::calculateFetch()
{
    DebugTimer t("wind:fetch");
    wind_module = this;
    impact_c=0;
    GlobalSettings::instance()->model()->threadExec().runGrid(nc_calculateFetch, mGrid.begin(), mGrid.end());
    qDebug() << "calculated fetch for" << impact_c << "pixels";
    return;
//    int calculated = 0;
//    double current_direction;
//    WindCell *end = mGrid.end();
//    for (WindCell *p=mGrid.begin(); p!=end; ++p) {
//        if (p->edge == 1.f) {
//            QPoint pt=mGrid.indexOf(p);
//            current_direction = mWindDirection + (mWindDirectionVariation>0.?nrandom(-mWindDirectionVariation, mWindDirectionVariation):0);
//            checkFetch(pt.x(), pt.y(), current_direction, p->height * 10., p->height - mEdgeDetectionThreshold);
//            ++calculated;
//            // only simulate edges with gapsize > 20m
//            // this skips small gaps (e.g. areas marked as "stones")
//            if (p->edge < 20.f) {
//                p->edge = 0.f;
//            }
//        }
//   }
//    qDebug() << "calculated fetch for" << calculated << "pixels";
}

static int effective_c;
void nc_calculateWindImpact(ResourceUnit *unit)
{
    GridRunner<WindCell> runner(wind_module->mGrid, unit->boundingBox());
    int calculated=0;
    int effective=0;
    while (WindCell *p = runner.next()) {
        if (p->edge >= 1.f) {
            QPoint pt=wind_module->mGrid.indexOf(p);
            if (wind_module->windImpactOnPixel(pt, p))
                ++effective;
            ++calculated;
        }
    }
    QMutexLocker lock(&wind_mt_counter);

    impact_c += calculated;
    effective_c+=effective;
}

/** calculate for each pixel the impact of wind
  @return number of pixels with killed trees
  */
int WindModule::calculateWindImpact()
{
    DebugTimer t("wind:impact");
    wind_module = this;
    impact_c=0; effective_c=0;
    GlobalSettings::instance()->model()->executePerResourceUnit(nc_calculateWindImpact);
    qDebug() << "calculated impact for" << impact_c << "pixels, affected" << effective_c;
    return effective_c;
//    int calculated = 0;
//    int effective = 0;
//    WindCell *end = mGrid.end();
//    for (WindCell *p=mGrid.begin(); p!=end; ++p) {
//        if (p->edge >= 1.f) {
//            QPoint pt=mGrid.indexOf(p);
//            if (windImpactOnPixel(pt, p))
//                ++effective;
//            ++calculated;
//        }
//    }
//    qDebug() << "calculated impact for" << calculated << "pixels";
//    return effective;
}



// test function
void WindModule::testFetch(double degree_direction)
{
//    for (int i=0;i<350;i+=10) {
//        checkFetch(100,100, i*M_PI/180., 500+i);
//    }
    double direction = degree_direction*M_PI/180.;
    int calculated = 0;

    WindCell *end = mGrid.end();
    for (WindCell *p=mGrid.begin(); p!=end; ++p) {
        if (p->edge == 1.f) {
            QPoint pt=mGrid.indexOf(p);
            checkFetch(pt.x(), pt.y(), direction, p->height * 10., p->height - 10.);
            ++calculated;
        }
   }
    qDebug() << "calculated fetch for" << calculated << "pixels";
}

void WindModule::testEffect()
{
    int calculated = 0;
    WindCell *end = mGrid.end();
    for (WindCell *p=mGrid.begin(); p!=end; ++p) {
        if (p->edge >= 1.f) {
            QPoint pt=mGrid.indexOf(p);
            windImpactOnPixel(pt, p);
            ++calculated;
        }
    }
    qDebug() << "calculated fetch for" << calculated << "pixels";
}


/** determines whether a wind event should be triggered in the current year.
    returns true if so and sets all relevant properties of the event (speed, direction).
  */
bool WindModule::eventTriggered()
{
    // when using time events, a wind event is triggered by the time event mechanism
    // by setting the wind speed of the event > 0

    XmlHelper xml(GlobalSettings::instance()->settings().node("modules.wind"));
    mWindSpeed = xml.valueDouble(".speed");
    if (mWindSpeed == 0.)
        return false;
    // reset the wind speed in the xml structure (avoid execution next year)
    xml.setNodeValue(".speed", "0");
    // get duration of the event
    double duration = xml.valueDouble(".duration", 0.); // duration of the event (minutes)
    mMaxIteration = duration * mIterationsPerMinute + 0.5;
    if (mMaxIteration<=0)
        return false;

    // get wind direction and date of the event
    mWindDirection = xml.valueDouble(".direction", 0.)*M_PI/180.;
    mWindDayOfYear = xml.valueDouble(".dayOfYear", 100.);


    qDebug() << "Wind: start event. Speed:" << mWindSpeed << "m/s, Duration (iterations):" << mMaxIteration << ", direction (deg):" << mWindDirection/M_PI*180.;
    return true;
}

/** find distance to the next shelter pixel
  @param startx x-index of the starting pixel (index)
  @param starty y-index of the starting pixel (index)
  @param direction direction to look (rad, 0: north, pi/2: east, pi: south, 3/2pi: west)
  @param max_distance maximum distance (meters) to look for
  @param threshold algorithm terminates if a pixel with a height higher than threshold is found
  */
bool WindModule::checkFetch(const int startx, const int starty, const double direction, const double max_distance, const double threshold)
{
    int endx = startx + (max_distance/cellsize()+0.5)*sin(direction);
    int endy = starty + (max_distance/cellsize()+0.5)*cos(direction);

    // "draw" a line using bresenhems algorithm (http://en.wikipedia.org/wiki/Bresenham's_line_algorithm)
    int dx = abs(endx-startx);
    int dy = abs(endy-starty);
    int sx = startx<endx?1:-1;
    int sy = starty<endy?1:-1;
    int err = dx - dy;

    int x = startx,y = starty;
    while (true) {
        if (mGrid.isIndexValid(x,y)) {
            if ((x!=startx || y!=starty) && (mGrid.valueAtIndex(x,y).height > threshold)) {
                double dist = sqrt(cellsize()*((x-startx)*(x-startx) + (y-starty)*(y-starty)));
                mGrid.valueAtIndex(startx, starty).edge = dist;
                return true;
            }
            // mGrid.valueAtIndex(x,y).edge = 10; // plot...
        } else
            break;
        if (x==endx && y==endy)
            break;
        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x += sx;
        }
        if (e2 < dx) {
            err += dx;
            y += sy;
        }
    }

    mGrid.valueAtIndex(startx, starty).edge = max_distance;
    return false;
/*
function line(x0, y0, x1, y1)
   dx := abs(x1-x0)
   dy := abs(y1-y0)
   if x0 < x1 then sx := 1 else sx := -1
   if y0 < y1 then sy := 1 else sy := -1
   err := dx-dy

   loop
     setPixel(x0,y0)
     if x0 = x1 and y0 = y1 exit loop
     e2 := 2*err
     if e2 > -dy then
       err := err - dy
       x0 := x0 + sx
     end if
     if e2 <  dx then
       err := err + dx
       y0 := y0 + sy
     end if
   end loop
*/
}


static QMutex tree_kill_lock;

/** Main function of the wind module
  @param position the integer index (x/y) of the grid cell for which the wind effect should be calculated
  @param cell the content of the current wind cell (for convenience)
    @return true, if trees were killed (thrown, broken)
  */
bool WindModule::windImpactOnPixel(const QPoint position, WindCell *cell)
{
    QRectF pixel_rect = mGrid.cellRect(position);
    ResourceUnit *ru = GlobalSettings::instance()->model()->ru(pixel_rect.center());
    if (!ru)
        return false;

    // ************************************************
    // scan the trees of the current resource unit: select the largest tree per 10m pixel
    // ************************************************
    scanResourceUnitTrees(position);
    if (!cell->tree) {
        // this should actually not happen any more
        cell->height = 0.f;
        cell->edge = 0.f;
        return false;
    }

    // *****************************************************************************
    // Calculate the wind speed at the crown top and the critical wind speeds
    // *****************************************************************************
    // first, calculate the windspeed in the crown
    DebugTimer t2("wind:impact:speed");
    const WindSpeciesParameters &params = speciesParameter(cell->tree->species());
    const WindRUCell &ru_cell = mRUGrid.valueAt(pixel_rect.center());
    double topo_mod = cell->topex;
    // the wind speed (10m above the canopy) is the global wind speed modified with the topography modifier
    // and with some added variation.
    double wind_speed_10;
    if (mTopexFactorModificationType == gfMultiply)
        wind_speed_10 = mWindSpeed * topo_mod * mCurrentGustFactor; // wind speed on current resource unit 10m above the canopy, factor applied multiplicatively
    else
        wind_speed_10 =( mWindSpeed + topo_mod) * mCurrentGustFactor; // wind speed on current resource unit 10m above the canopy, topo modifier calculated additively

    // cell->n_trees: sum of basal area of all trees on the pixel
    double n_trees = cell->n_trees / cell->tree->basalArea(); // number of trees with the dimension of the focal tree

    double u_crown = calculateCrownWindSpeed(cell->tree, params, n_trees, wind_speed_10);

    // now calculate the critical wind speed for the tallest tree on the pixel (i.e. the speed at which stem breakage/uprooting occurs)
    double cws_uproot, cws_break;
    calculateCrititalWindSpeed(cell->tree, params, cell->edge, cws_uproot, cws_break);
    cell->cws_break = cws_break;
    cell->cws_uproot = cws_uproot;
    cell->crown_windspeed = u_crown;

    // whether uprooting or breaking occurs depend on the wind speed and the state of the soil:
    // if the soil is frozen, than trees break, if not trees uproot
    bool do_break;
    if (ru_cell.soilIsFrozen) {
        if (u_crown < cws_break)
            return false; // wind speed is too low
        do_break = true;
    } else {
        if (u_crown < qMin(cws_uproot, cws_break))
            return false; // wind speed is to low
        do_break = cws_break < cws_uproot; // either break or uproot depending on the critical wind speeds
    }

    QMutexLocker locker(&tree_kill_lock); // make sure that the following code is not run in parallel

    // *****************************************************************************
    // Kill the trees that are thrown/uprooted by the wind
    // *****************************************************************************
    if (!do_break) {
        // regeneration is killed in case of uprooting
        if (GlobalSettings::instance()->model()->saplings())
            GlobalSettings::instance()->model()->saplings()->clearSaplings(pixel_rect, false, true);
    }
    QVector<Tree>::const_iterator tend = ru->trees().constEnd();
    for (QVector<Tree>::const_iterator  t=ru->trees().constBegin(); t!=tend; ++t) {
        if (!t->isDead() &&
                t->positionIndex().x()/cPxPerHeight == position.x() &&
                t->positionIndex().y()/cPxPerHeight == position.y() ) {
            if (!mSimulationMode) {
                // all trees > 4m are killed on the cell
                Tree *tree = const_cast<Tree*>(&(*t));
                tree->setDeathReasonWind();
                if (do_break) {
                    // the tree is breaking
                    // half of the stem as well as foliage/branches are moved to the soil. The other half
                    // of the stem remains as snag.
                    tree->removeDisturbance(0.5, 0.5, // 50% of the stem to soil, 50% to snag
                                            1., 0.,   // 100% of branches to soil
                                            1.);      // 100% of foliage to soil

                } else {
                    // uprooting
                    // all biomass of the tree is moved to the soil
                    tree->removeDisturbance(1., 0., // 100% of stem -> soil
                                            1., 0., // 100% of branch -> soil
                                            1.);    // 100% of foliage -> soil
                }
            }
            // statistics
            cell->basal_area_killed += t->basalArea();
            cell->sum_volume_killed += t->volume();
            cell->n_killed ++;
            mTotalKilledBasalArea += t->basalArea();
            mTotalKilledVolume += t->volume();
            mTreesKilled ++;

        }
    }

    // reset the current cell
    cell->height = 0.f;
    cell->edge = 0.f;
    cell->n_iteration = mCurrentIteration;
    cell->tree = 0;
    cell->n_trees = 0.f; // no more trees on the pixel
    if (!mSimulationMode)
        cell->edge_age = 0; // ... and no edge
    cell->n_affected++; // pixel has been cleared this time...
    return true;
}

/** calculate the windspeed at the top of the crown.

  @param tree the tallest tree on the cell
  @param n_trees number of trees that are on the 10x10m cell
  @param wind_speed_10 wind speed in 10m above canopy (m/s)
  */
double WindModule::calculateCrownWindSpeed(const Tree *tree, const WindSpeciesParameters &params, const double n_trees, const double wind_speed_10)
{
     // frontal area index
    const double porosity = 0.5;
    double lambda = 2. * tree->crownRadius() * (tree->height() * params.crown_length )* params.crown_area_factor * porosity / (cellsize()*cellsize()/n_trees);
    // calculate zero-plane-displacement height (Raupachs drag partitioning model (Raupach 1992, 1994))
    // the zero plane displacement is the virtual "ground" height in the canopy; it is usually at about 80% of the tree height
    const double cdl = 7.5;
    double d0 = tree->height() * ( 1. - (1-exp(-sqrt(cdl*lambda)))/(sqrt(cdl*lambda)));

    const double surface_drag_coefficient = 0.003;
    const double element_drag_coefficient = 0.3;
    const double kaman_constant = 0.4;

    // drag coefficient gamma
    double lambda_drag = std::min(lambda, 0.6); // the lambda for the drag calculation is max 0.6
    double gamma = 1./sqrt(surface_drag_coefficient + element_drag_coefficient*lambda_drag/2.);

    // surface roughness
    double z0 = (tree->height() - d0)*exp(-kaman_constant*gamma + 0.193);

    // now we calculate the windspeed at the crown top. Our input is a windspeed 10m above the zero-plane-displacement (U10)
    // if U10 is from a weather station above open ground, a transformation to the wind speed 10m above the forest would be necessary:
    // U10 = U10_ref * log(1000/z0_ref) * log(10/z0) / ( log(10/z0_ref)*log(1000/z0) ), with z0_ref = z0 above open ground (e.g. 0.05), and z0 a typical roughness for forests (e.g. 0.3)
    // we assume a logarithmic wind profie
    double u_factor = log((tree->height()-d0)/z0) / log(10./z0);
    double u_crown = wind_speed_10 * u_factor;

    return u_crown;
}

/** calculate the critital windspeed taking into account the sheltering from upwind vegetation and the competetive state of the tree.
  The calculation is performed for the largest tree on the cell.
  @param tree the tallest tree on the cell
  @param gap_length size of the gap in upwind direction (m)
  @param rCWS_uproot OUT parameter with the critical windspeed for uprooting
  @param rCWS_uproot OUT parameter with the critical windspeed for stem breakage
  @return the (lower) critital windspeed

  */
double WindModule::calculateCrititalWindSpeed(const Tree *tree, const WindSpeciesParameters &params, const double gap_length, double &rCWS_uproot, double &rCWS_break)
{
    // relate the gap size to tree length and calcualte the f_gap factor
    double rel_gap = gap_length / tree->height();
    if (rel_gap>10) rel_gap=10;

    // formulation from Peltola et al. (1999), based on Gardiner et al. (1997) 0.2158... = scale to situation with gapsize = 0
    double f_gap = (0.001+0.001*pow(rel_gap,0.562))/(0.00465) ;

    // calculate the wet stem weight (iLand internally uses always dry weights)
    double stem_mass = tree->biomassStem() * params.wet_biomass_factor;

    // the competitveness index of Hegyi (1974) is derived from the iLand LRI.
    // it has a value of 3.41 for very dense stands (i.e. LRI<0.05), and a minimum index of 0.5 (when LRI>0.5)
    double c_hegyi;
    c_hegyi = mLRITransferFunction.calculate(tree->lightResourceIndex());
    // the turning moment coefficient incorporating the competition state
    double tc = 4.42+122.1*(tree->dbh()*tree->dbh()/cRUArea)*tree->height()-0.141*c_hegyi-14.6*(tree->dbh()*tree->dbh()/cRUArea)*tree->height()*c_hegyi;
    // now derive the critital wind speeds for uprooting and breakage
    const double f_knot = 1.; // a reduction factor accounting for the presence of knots, currenty no reduction.
    // a factor to scale average wind speeds to gust, which transport much more energy. Orignially, the factor depends on the distance from the edge;
    // since we calculate only on the edge, the factor is fixed (see Byrne (2011) and Gardiner)

    // Turning moments at stand edges are significantly higher at stand edges compared to conditions
    // well inside the forest. Data from Gardiner(1997) and recalculations from Byrne (2011) indicate,
    // that the maximum turning moment at the edge is about 5 times as high as "well inside" the forest.
    const double f_edge = mFactorEdge;

    rCWS_uproot = sqrt((params.Creg*stem_mass) / (tc*f_gap*f_edge)); // critical windspeed for uprooting
    rCWS_break = sqrt(params.MOR*pow(tree->dbh(),3)*f_knot*M_PI / (32.*tc*f_gap*f_edge)); // critical windspeed for breakage

    // debug info
    // qDebug() << "f_gap, bal, tc, cws_uproot, cws_break:" << f_gap << bal << tc << rCWS_uproot << rCWS_break;
    return qMin(rCWS_uproot, rCWS_break);
}


/** calculate the temperature of the soil and return true if the soil is frozen.
  The algorithm uses the model of Paul et al (2004).
  This model calculates soil temperature based on mean annual temperature, summer temperature and the temperature
  of the current day, i.e. does not depend on floating averages. However, it takes into account the LAI and the mass of litter on the soil.
  @param ru ResourceUnit for which the calculations are done
  @param day_of_year index of the day of the year (0..365)  */
bool WindModule::isSoilFrozen(const ResourceUnit *ru, const int day_of_year) const
{
    const double soil_depth = 10.; // we use a default soil depth of 10cm
    const double litter_mass = 30.;  // 30 Mg BM/ha, is approx. the average value for Austrias forest soils (cf. Seidl et al. 2009)
    const double weed_cover = 0.2; // we also assume a constant weed cover of 20%

    double mean_annual_temp = ru->climate()->meanAnnualTemperature();
    // mean temperature of june/july/august
    double summer_temp = (ru->climate()->temperatureMonth()[5] + ru->climate()->temperatureMonth()[6] + ru->climate()->temperatureMonth()[7]) / 3.;
    double lai = ru->statistics().leafAreaIndex();
    double temp_day = ru->climate()->dayOfYear(day_of_year)->temperature;

    double t_x = 297. - day_of_year;
    double as = mean_annual_temp*1.23*exp(-0.06*(lai+weed_cover));
    double pa = summer_temp - mean_annual_temp;
    double ps = pa*1.12*exp(-0.15*(lai+weed_cover))*exp(-0.01*litter_mass)+2.22;
    double ds = ((mean_annual_temp + pa*sin(2*M_PI/365.*t_x))-temp_day)*exp(-0.08*soil_depth);
    double ts = as + ps*sin(2*M_PI/365.*t_x) - ds;

    if (ts>0.)
        return false; // soil is not frozen
    else
        return true; // soil is frozen

}

// scan the content of the trees container of the resource unit
// and extract the tallest tree & species per wind pixel
void WindModule::scanResourceUnitTrees(const QPoint &position)
{
    QPointF p_m = mGrid.cellCenterPoint(position);
    // if this resource unit was already scanned in this wind event, then do nothing
    // the flags are reset during initWindGrid()
    if (mRUGrid.valueAt(p_m).flag)
        return;

    ResourceUnit *ru = GlobalSettings::instance()->model()->ru(p_m);

    if (ru) {
        QVector<Tree>::const_iterator tend = ru->trees().constEnd();
        for (QVector<Tree>::const_iterator t = ru->trees().constBegin(); t!=tend; ++t) {
            if (!t->isDead()) {
                const QPoint &tp = t->positionIndex();
                QPoint pwind(tp.x()/cPxPerHeight, tp.y()/cPxPerHeight);
                WindCell &wind=mGrid.valueAtIndex(pwind);
                wind.n_trees += t->basalArea();
                if (!wind.tree || t->height()>wind.tree->height()) {
                    wind.height = t->height();
                    wind.tree = &(*t);
                }
            }
        }
        // check if the soil on the resource unit is frozen
        switch(mSoilFreezeMode){
        case esfAuto: mRUGrid.valueAt(p_m).soilIsFrozen = isSoilFrozen(ru, mWindDayOfYear); break;
        case esfFrozen: mRUGrid.valueAt(p_m).soilIsFrozen = true; break;
        case esfNotFrozen: mRUGrid.valueAt(p_m).soilIsFrozen = false; break;
        default: break;
        }
    }

    // set the "processed" flag
    mRUGrid.valueAt(p_m).flag = 1;
}

