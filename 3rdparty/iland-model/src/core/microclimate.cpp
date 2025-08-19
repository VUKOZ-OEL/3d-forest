
#include "microclimate.h"

#include "resourceunit.h"
#include "species.h"
#include "tree.h"
#include "modelcontroller.h"
#include "dem.h"
#include "climate.h"

Microclimate::MicroClimateSettings Microclimate::mSettings = {true, true, true};


Microclimate::Microclimate(const ResourceUnit *ru)
{
    mRU = ru;
    mCells = new MicroclimateCell[cHeightPerRU*cHeightPerRU];
    mIsSetup = false;
    // setup of effect switches (static variable)
    mSettings.barkbeetle_effect = GlobalSettings::instance()->settings().valueBool("model.climate.microclimate.barkbeetle");
    mSettings.decomposition_effect = GlobalSettings::instance()->settings().valueBool("model.climate.microclimate.decomposition");
    mSettings.establishment_effect = GlobalSettings::instance()->settings().valueBool("model.climate.microclimate.establishment");
}

Microclimate::~Microclimate()
{
    delete[] mCells;
}

void Microclimate::calculateVegetation()
{
    QVector<double> ba_total(cHeightPerRU*cHeightPerRU, 0.);
    QVector<double> lai_total(cHeightPerRU*cHeightPerRU, 0.);
    QVector<double> shade_tol(cHeightPerRU*cHeightPerRU, 0.);

    if (!mIsSetup)
        // calculate (only once) northness and other factors that only depend on elevation model
        calculateFixedFactors();


    // loop over trees and calculate aggregate values
    for ( QVector<Tree>::const_iterator t = mRU->constTrees().constBegin(); t != mRU->constTrees().constEnd(); ++t) {
        int idx = cellIndex(t->position());
        ba_total[idx] += t->basalArea();
        lai_total[idx] += t->leafArea();
        // shade-tolerance uses species parameter light response class
        shade_tol[idx] += t->species()->lightResponseClass() * t->basalArea();
    }

    // now write back to the microclimate store
    for (int i=0;i<cHeightPerRU*cHeightPerRU; ++i) {
        double lai = limit(lai_total[i] / cHeightPixelArea, 0.3, 9.4); // m2/m2
        double stol = limit(ba_total[i] > 0. ? shade_tol[i] / ba_total[i] : 0., 1., 5.);
        cell(i).setLAI( lai ); // calculate m2/m2
        cell(i).setShadeToleranceMean( stol);
    }

    // do additionally calculate and buffer values on RU resolution for performance reasons
    calculateRUMeanValues();

}


void Microclimate::calculateRUMeanValues()
{


    // run over the year
    double buffer_min;
    double buffer_max;
    int n;
    const ClimateDay *cday;
    double mean_tmin[12];
    double mean_tmax[12];

    for (int i=0;i<12;++i) {
        mean_tmin[i] = 0.;
        mean_tmax[i] = 0.;
    }
    // calculate mean min / max temperature
    for (cday = mRU->climate()->begin(); cday != mRU->climate()->end(); ++cday) {
        mean_tmin[cday->month - 1] += cday->min_temperature;
        mean_tmax[cday->month - 1] += cday->max_temperature;
    }
    for (int i=0;i<12;++i) {
        mean_tmin[i] /= mRU->climate()->days(i);
        mean_tmax[i] /= mRU->climate()->days(i);
        // limit to values in statistical model
        mean_tmin[i] = limit(mean_tmin[i], -12.4, 16.5);
        mean_tmax[i] = limit(mean_tmax[i], -5.4, 44.9);
    }

    // run calculations
    for (int m=0;m<12;++m) {
        // loop over all cells and calculate buffering
        buffer_min=0.;
        buffer_max=0.;
        n=0;
        for (int i=0;i < cHeightPerRU*cHeightPerRU; ++i) {
            if (mCells[i].valid()) {

                buffer_min += mCells[i].minimumMicroclimateBuffering(mean_tmin[m]);
                buffer_max += mCells[i].maximumMicroclimateBuffering(mean_tmax[m]);
                ++n;
            }
        }

        // calculate mean values for RU and save for later
        buffer_min = n>0 ? buffer_min / n : 0.;
        buffer_max = n>0 ? buffer_max / n : 0.;
        if (abs(buffer_min) > 15 || abs(buffer_max)>15) {
            qDebug() << "Microclimate: dubious buffering: RU: " << mRU->id() << ", buffer_min:" << buffer_min << ", buffermax:" << buffer_max;
            buffer_min = 0.;
            buffer_max = 0.; // just make sure nothing bad happens downstream
        }
        // make sure that min-temp is not exceeding max temp of the month
        //double buffer_mean = (buffer_min + buffer_max) / 2.;
        //if (buffer_min > buffer_max) {
        //    buffer_min = buffer_mean;
        //    buffer_max = buffer_mean;
        //}


        mRUvalues[m] = QPair<float, float>(static_cast<float>(buffer_min),
                                             static_cast<float>(buffer_max));

    }

}

double Microclimate::minimumMicroclimateBufferingRU(int month) const
{
    return mRUvalues[month].first;
}


double Microclimate::maximumMicroclimateBufferingRU(int month) const
{
    return mRUvalues[month].second;
 }

double Microclimate::meanMicroclimateBufferingRU(int month) const
{
    // calculate mean value of min and max buffering
    double buffer = ( minimumMicroclimateBufferingRU(month) +
                     maximumMicroclimateBufferingRU(month) ) / 2.;
    return buffer;
}



int Microclimate::cellIndex(const QPointF &coord)
{
    // convert to index
    QPointF local = coord - mRU->boundingBox().topLeft();
    Q_ASSERT(local.x()>=0 && local.x()<100 && local.y()>=0 && local.y() < 100);

    int index = ( int( local.y() / cHeightPerRU) ) * cHeightPerRU + ( int( local.x() / cHeightPerRU ) );
    return index;
}

QPointF Microclimate::cellCoord(int index)
{
    QPointF local( ( (index % cHeightPerRU) + 0.5) * cHeightPerRU, ((index/cHeightPerRU) + 0.5) * cHeightPerRU );
    return local + mRU->boundingBox().topLeft();
}

void Microclimate::calculateFixedFactors()
{
    if (!GlobalSettings::instance()->model()->dem())
        throw IException("The iLand Microclimate module requires a digital elevation model (DEM).");


    // extract fixed factors from DEM
    const DEM *dem =  GlobalSettings::instance()->model()->dem();

    HeightGrid *hg = GlobalSettings::instance()->model()->heightGrid();

    for (int i=0;i<cHeightPerRU*cHeightPerRU; ++i) {
        QPointF p = cellCoord(i);

        double aspect = dem->aspectGrid()->constValueAt(p)*M_PI / 180.;
        double northness = cos(aspect);

        // slope
        //double slope = dem->slopeGrid()->constValueAt(p); // percentage of degrees, i.e. 1 = 45 degrees
        //slope = atan(slope) * 180./M_PI; // convert degree, thanks Kristin for spotting the error in a previous version

        // topographic position
        const int radius = 500;
        double tpi = dem->topographicPositionIndex(p, radius);

        // limit values to range of predictors in statistical model
        //northness = limit(northness, -1, 1)
        tpi = limit(tpi, -105., 67.);

        cell(i).setNorthness(northness);
        cell(i).setTopographicPositionIndex(tpi);

        // we only process cells that are stockable
        if (!hg->constValueAt(p).isValid())
            cell(i).setInvalid();

    }

    mIsSetup = true;
}

MicroclimateVisualizer *MicroclimateVisualizer::mVisualizer = nullptr;

MicroclimateVisualizer::MicroclimateVisualizer(QObject *parent)
    :QObject(parent)
{
    Q_UNUSED(parent);
}

MicroclimateVisualizer::~MicroclimateVisualizer()
{
    GlobalSettings::instance()->controller()->removePaintLayers(mVisualizer);
    mVisualizer = nullptr;
}

void MicroclimateVisualizer::setupVisualization()
{
    // add agent to UI
    if (mVisualizer)
        delete mVisualizer;

    mVisualizer = new MicroclimateVisualizer();

    QStringList varlist = {"Microclimate - LAI", "Microclimate - ShadeTol", // 0,1
                           "Microclimate - TPI", "Microclimate - Northness",  // 2,3
                           "Microclimate - Min.Buffer(June)", "Microclimate - Min.Buffer(Dec)", // 4,5
                           "Microclimate - Max.Buffer(June)", "Microclimate - Max.Buffer(Dec)"};  // 6,7

    QVector<GridViewType> paint_types = {GridViewTurbo, GridViewTurbo, GridViewTurbo,
                                         GridViewTurbo, GridViewTurbo,
                                         GridViewTurbo, GridViewTurbo,
                                         GridViewTurbo, GridViewTurbo};
    GlobalSettings::instance()->controller()->addPaintLayers(mVisualizer, varlist, paint_types);

}

Grid<double> *MicroclimateVisualizer::paintGrid(QString what, QStringList &names, QStringList &colors)
{
    Q_UNUSED(names)
    Q_UNUSED(colors)

    if (mGrid.isEmpty()) {
        // setup grid with the dimensions of the iLand height grid
        mGrid.setup(GlobalSettings::instance()->model()->heightGrid()->metricRect(),
                    GlobalSettings::instance()->model()->heightGrid()->cellsize());
        mGrid.wipe(0.);
    }
    int index = 0;
    if (what == "Microclimate - LAI") index = 0;
    if (what == "Microclimate - ShadeTol") index = 1;
    if (what == "Microclimate - TPI") index = 2;
    if (what == "Microclimate - Northness") index = 3;
    if (what == "Microclimate - Min.Buffer(June)") index=4;
    if (what == "Microclimate - Min.Buffer(Dec)") index=5;
    if (what == "Microclimate - Max.Buffer(June)") index=6;
    if (what == "Microclimate - Max.Buffer(Dec)") index=7;

    // fill the grid with the expected variable

    foreach (ResourceUnit *ru, GlobalSettings::instance()->model()->ruList()) {
        const Microclimate *clim = ru->microClimate();
        GridRunner<double> runner(mGrid, ru->boundingBox());
        int cell_index = 0;
        double value;
        while (double *gridptr = runner.next()) {
            switch (index) {
            case 0: value = clim->constCell(cell_index).LAI(); break;
            case 1: value = clim->constCell(cell_index).shadeToleranceMean(); break;
            case 2: value = clim->constCell(cell_index).topographicPositionIndex(); break;
            case 3: value = clim->constCell(cell_index).northness(); break;
                // buffering capacity: minimum summer
            case 4:  value = clim->constCell(cell_index).minimumMicroclimateBuffering(ru, 5); break;
                // minimum winter
            case 5:  value = clim->constCell(cell_index).minimumMicroclimateBuffering(ru, 0); break;
                // buffering capacity: max summer
            case 6:  value = clim->constCell(cell_index).maximumMicroclimateBuffering(ru, 5); break;
                // max winter
            case 7:  value = clim->constCell(cell_index).maximumMicroclimateBuffering(ru, 0); break;
            }

            *gridptr = value;
            ++cell_index;
        }
    }
    return &mGrid;
}

Grid<double> *MicroclimateVisualizer::grid(QString what, int month)
{
    Grid<double> *grid = new Grid<double>(GlobalSettings::instance()->model()->heightGrid()->metricRect(),
                                         GlobalSettings::instance()->model()->heightGrid()->cellsize());
    grid->wipe(0.);
    int index = -1;
    if (what == "LAI") index = 0;
    if (what == "ShadeTol") index = 1;
    if (what == "TPI") index = 2;
    if (what == "Northness") index = 3;
    if (what == "MinTBuffer") index=4;
    if (what == "MaxTBuffer") index=5;

    if (index < 0)
        throw IException("Microclimate: invalid grid name");


    // fill the grid with the expected variable

    foreach (ResourceUnit *ru, GlobalSettings::instance()->model()->ruList()) {
        const Microclimate *clim = ru->microClimate();
        GridRunner<double> runner(grid, ru->boundingBox());
        int cell_index = 0;
        double value;
        while (double *gridptr = runner.next()) {
            switch (index) {
            case 0: value = clim->constCell(cell_index).LAI(); break;
            case 1: value = clim->constCell(cell_index).shadeToleranceMean(); break;
            case 2: value = clim->constCell(cell_index).topographicPositionIndex(); break;
            case 3: value = clim->constCell(cell_index).northness(); break;
            case 4:  value = clim->constCell(cell_index).minimumMicroclimateBuffering(ru, month); break;
            case 5:  value = clim->constCell(cell_index).maximumMicroclimateBuffering(ru, month); break;
            }

            *gridptr = value;
            ++cell_index;
        }
    }
    return grid;
}

MicroclimateCell::MicroclimateCell(double lai,
                                   double shade_tol,
                                   double tpi,
                                   double northness)
{
    setLAI(lai);
    setShadeToleranceMean(shade_tol);
    setTopographicPositionIndex(tpi);
    setNorthness(northness);
}

//double MicroclimateCell::growingSeasonIndex(const ResourceUnit *ru, int dayofyear) const
//{
//    const int pheno_broadleaved = 1;
//    const Phenology &pheno = ru->climate()->phenology(pheno_broadleaved );

//    int rDay, rMonth;
//    ru->climate()->toDate(dayofyear, &rDay, &rMonth);

//    double gsi = pheno.monthArray()[rMonth];
//    return gsi;
//}

double MicroclimateCell::minimumMicroclimateBuffering(const ResourceUnit *ru, int month) const
{
    Q_ASSERT(month>=0 && month<12);
    int n_days = ru->climate()->days(month);
    double mean_temp = 0.;
    for (int i=0;i< n_days;++i)
        mean_temp += ru->climate()->day(month, i)->min_temperature;
    mean_temp /= static_cast<double>(n_days);

    return minimumMicroclimateBuffering( mean_temp);
}

double MicroclimateCell::maximumMicroclimateBuffering(const ResourceUnit *ru, int month) const
{
    Q_ASSERT(month>=0 && month<12);
    int n_days = ru->climate()->days(month);
    double mean_temp = 0.;
    for (int i=0;i< n_days;++i)
        mean_temp += ru->climate()->day(month, i)->max_temperature;
    mean_temp /= static_cast<double>(n_days);

    return maximumMicroclimateBuffering(mean_temp);
}

double MicroclimateCell::minimumMicroclimateBuffering(double macro_t_min) const
{
    // old: "Minimum temperature buffer ~ -1.7157325 - 0.0187969*North + 0.0161997*RelEmin500 + 0.0890564*lai + 0.3414672*stol + 0.8302521*GSI + 0.0208083*prop_evergreen - 0.0107308*GSI:prop_evergreen"
    // Buffer_minT = 0.6077 – 0.0088 * Macroclimate_minT + 0.3548 * Northness  + 0.0872 * Slope + 0.0202 * TPI - 0.0330 * LAI + 0.0502 * STol – 0.7601 * Evergreen – 0.8385 * GSI:Evergreen
    // version nov 2023: Tminbuffer = 1.4570 - 0.0248 × Tminmacroclimate + 0.2627 × Northness + 0.0158 × TPI + 0.0227 × LAI - 0.2031 × STol
    double buf = 1.4570 +
                 -0.0248 *macro_t_min +
                 0.2627*northness() +
                 0.0158*topographicPositionIndex() +
                 0.0227*LAI() +
                 -0.2031*shadeToleranceMean() ;
    if (abs(buf)>10)
        buf=0;
    return buf;

}

double MicroclimateCell::maximumMicroclimateBuffering(double macro_t_max) const
{
    // old: "Maximum temperature buffer ~ 1.9058391 - 0.2528409*North - 0.0027037*RelEmin500 - 0.1549061*lai - 0.3806543*stol - 1.2863341*GSI - 0.8070951*prop_evergreen + 0.5004421*GSI:prop_evergreen"
    // Buffer_maxT = 2.7839 – 0.2729 * Macroclimate_maxT - 0.5403 * Northness  - 0.1127 * Slope + 0.0155 * TPI – 0.3182 * LAI + 0.1403 * STol – 1.1039 * Evergreen + 6.9670 * GSI:Evergreen
    // version nov 23: Tmaxbuffer = 0.9767 - 0.1932 × Tmaxmacroclimate - 0.5729 × Northness + 0.0140 × TPI - 0.3948 × LAI + 0.4419 × STol
    double buf = 0.9767 +
                 -0.1932*macro_t_max +
                 -0.5729*northness() +
                 0.0140*topographicPositionIndex() +
                 -0.3948*LAI() +
                 0.4419*shadeToleranceMean();

    if (abs(buf)>10)
        buf=0;
    return buf;
}
