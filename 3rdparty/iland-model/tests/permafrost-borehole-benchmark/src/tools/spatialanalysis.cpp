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
#include "spatialanalysis.h"

#include "globalsettings.h"
#include "model.h"
#include "tree.h"
#include "stamp.h"
#include "helper.h"
#include "resourceunit.h"
#include "scriptgrid.h"

#include <QJSEngine>
#include <QJSValue>
void SpatialAnalysis::addToScriptEngine()
{
    SpatialAnalysis *spati = new SpatialAnalysis;
    QJSValue v = GlobalSettings::instance()->scriptEngine()->newQObject(spati);
    GlobalSettings::instance()->scriptEngine()->globalObject().setProperty("SpatialAnalysis", v);
}

SpatialAnalysis::~SpatialAnalysis()
{
    if (mRumple)
        delete mRumple;
}

double SpatialAnalysis::rumpleIndexFullArea()
{
    if (!mRumple)
        mRumple = new RumpleIndex;
    double rum = mRumple->value();
    return rum;
}

/// extract patches (clumps) from the grid 'src'.
/// Patches are defined as adjacent pixels (8-neighborhood)
/// Return: vector with number of pixels per patch (first element: patch 1, second element: patch 2, ...)
QList<int> SpatialAnalysis::extractPatches(Grid<double> &src, int min_size, QString fileName)
{
    mClumpGrid.setup(src.metricRect(), src.cellsize());
    mClumpGrid.wipe();

    // now loop over all pixels and run a floodfill algorithm
    QPoint start;
    QQueue<QPoint> pqueue; // for the flood fill algorithm
    QList<int> counts;
    int patch_index = 0;
    int total_size = 0;
    int patches_skipped = 0;
    for (int i=0;i<src.count();++i) {
        if (src[i]>0. && mClumpGrid[i]==0) {
            start = src.indexOf(i);
            pqueue.clear();
            patch_index++;

            // quick and dirty implementation of the flood fill algroithm.
            // based on: http://en.wikipedia.org/wiki/Flood_fill
            // returns the number of pixels colored

            pqueue.enqueue(start);
            int found = 0;
            while (!pqueue.isEmpty()) {
                QPoint p = pqueue.dequeue();
                if (!src.isIndexValid(p))
                    continue;
                if (src.valueAtIndex(p)>0. && mClumpGrid.valueAtIndex(p) == 0) {
                    mClumpGrid.valueAtIndex(p) = patch_index;
                    pqueue.enqueue(p+QPoint(-1,0));
                    pqueue.enqueue(p+QPoint(1,0));
                    pqueue.enqueue(p+QPoint(0,-1));
                    pqueue.enqueue(p+QPoint(0,1));
                    pqueue.enqueue(p+QPoint(1,1));
                    pqueue.enqueue(p+QPoint(-1,1));
                    pqueue.enqueue(p+QPoint(-1,-1));
                    pqueue.enqueue(p+QPoint(1,-1));
                    ++found;
                }
            }
            if (found<min_size) {
                // delete the patch again
                pqueue.enqueue(start);
                while (!pqueue.isEmpty()) {
                    QPoint p = pqueue.dequeue();
                    if (!src.isIndexValid(p))
                        continue;
                    if (mClumpGrid.valueAtIndex(p) == patch_index) {
                        mClumpGrid.valueAtIndex(p) = -1;
                        pqueue.enqueue(p+QPoint(-1,0));
                        pqueue.enqueue(p+QPoint(1,0));
                        pqueue.enqueue(p+QPoint(0,-1));
                        pqueue.enqueue(p+QPoint(0,1));
                        pqueue.enqueue(p+QPoint(1,1));
                        pqueue.enqueue(p+QPoint(-1,1));
                        pqueue.enqueue(p+QPoint(-1,-1));
                        pqueue.enqueue(p+QPoint(1,-1));
                    }
                }
                --patch_index;
                patches_skipped++;

            } else {
                // save the patch in the result
                counts.push_back(found);
                total_size+=found;
            }
        }
    }
    // remove the -1 again...
    mClumpGrid.limit(0,999999);

    qDebug() << "extractPatches: found" << patch_index << "patches, total valid pixels:" << total_size << "skipped" << patches_skipped;
    if (!fileName.isEmpty()) {
        qDebug() << "extractPatches: save to file:" << GlobalSettings::instance()->path(fileName);
        Helper::saveToTextFile(GlobalSettings::instance()->path(fileName), gridToESRIRaster(mClumpGrid) );
    }
    return counts;

}

void SpatialAnalysis::saveRumpleGrid(QString fileName)
{
    if (!mRumple)
        mRumple = new RumpleIndex;

    Helper::saveToTextFile(GlobalSettings::instance()->path(fileName), gridToESRIRaster(mRumple->rumpleGrid()) );

}

void SpatialAnalysis::saveCrownCoverGrid(QString fileName)
{
    calculateCrownCoverRU();
    Helper::saveToTextFile(GlobalSettings::instance()->path(fileName), gridToESRIRaster(mCrownCoverGrid) );

}

void SpatialAnalysis::saveCrownCoverGrid(QString fileName, QJSValue grid)
{
    ScriptGrid *sg = qobject_cast<ScriptGrid*>(grid.toQObject());
    if (!sg) {
        qDebug() << "ERROR: saveCrownCoverGrid got invalid reference grid!";
        return;
    }
    // make a copy of the underlying grid
    Grid<double> gr;
    gr.setup( *sg->grid() );

    // calculate crown cover by looping over all trees and mis-use the LIF grid
    runCrownProjection2m();


    FloatGrid *lifgrid = GlobalSettings::instance()->model()->grid();
    Model *model = GlobalSettings::instance()->model();
    double cell_area = gr.cellsize() * gr.cellsize();

    for (double *rg = gr.begin(); rg!=gr.end();++rg) {

        float cc_sum = 0.f;
        GridRunner<float> runner(lifgrid, gr.cellRect(gr.indexOf(rg)));
        while (float *gv = runner.next()) {
            if (model->heightGridValue(runner.currentIndex().x(), runner.currentIndex().y()).isValid())
                if (*gv >= 0.5f) // 0.5: half of a 2m cell is covered by a tree crown; is a bit pragmatic but seems reasonable (and works)
                    cc_sum++;
        }
        double value = cc_sum * cPxSize*cPxSize / cell_area;
        *rg = limit(value, 0., 1.);
    }
     Helper::saveToTextFile(GlobalSettings::instance()->path(fileName), gridToESRIRaster(gr) );

}

QJSValue SpatialAnalysis::patches(QJSValue grid, int min_size)
{
    ScriptGrid *sg = qobject_cast<ScriptGrid*>(grid.toQObject());
    if (sg) {
        // extract patches (keep patches with a size >= min_size
        mLastPatches = extractPatches(*sg->grid(), min_size, QString());
        // create a (double) copy of the internal clump grid, and return this grid
        // as a JS value
        QJSValue v = ScriptGrid::createGrid(mClumpGrid.toDouble(),"patch");
        return v;
    }
    return QJSValue();
}

void SpatialAnalysis::calculateCrownCoverRU()
{
    mCrownCoverGrid.setup(GlobalSettings::instance()->model()->RUgrid().metricRect(),
                          GlobalSettings::instance()->model()->RUgrid().cellsize());

    // calculate crown cover by looping over all trees and mis-use the LIF grid
    runCrownProjection2m();

    FloatGrid *grid = GlobalSettings::instance()->model()->grid();
    // now aggregate values for each resource unit
    Model *model = GlobalSettings::instance()->model();
    for (float *rg = mCrownCoverGrid.begin(); rg!=mCrownCoverGrid.end();++rg) {
        ResourceUnit *ru =  model->RUgrid().constValueAtIndex(mCrownCoverGrid.indexOf(rg));
        if (!ru) {
            *rg=0.f;
            continue;
        }
        float cc_sum = 0.f;
        GridRunner<float> runner(grid, mCrownCoverGrid.cellRect(mCrownCoverGrid.indexOf(rg)));
        while (float *gv = runner.next()) {
            if (model->heightGridValue(runner.currentIndex().x(), runner.currentIndex().y()).isValid())
                if (*gv >= 0.5f) // 0.5: half of a 2m cell is covered by a tree crown; is a bit pragmatic but seems reasonable (and works)
                    cc_sum++;
        }
        if (ru->stockableArea()>0.) {
            double value = cPxSize*cPxSize*cc_sum/ru->stockableArea();
            *rg = limit(value, 0., 1.);
        }
    }
}

void SpatialAnalysis::runCrownProjection2m(FloatGrid *agrid)
{
    // calculate the crown cover per resource unit. We use the "reader"-stamps of the individual trees
    // as they represent the crown (size). We also simply hijack the LIF grid for our calculations if no grid is provided

    FloatGrid *grid = agrid;
    if (agrid == nullptr)
        grid = GlobalSettings::instance()->model()->grid();

    grid->initialize(0.f);
    // we simply iterate over all trees of all resource units (not bothering about multithreading here)
    int x,y;
    AllTreeIterator ati(GlobalSettings::instance()->model());
    while (Tree *t = ati.nextLiving()) {
        // apply the reader-stamp
        const Stamp *reader = t->stamp()->reader();
        QPoint pos_reader = t->positionIndex(); // tree position
        pos_reader-=QPoint(reader->offset(), reader->offset());
        int reader_size = reader->size();
        int rx = pos_reader.x();
        int ry = pos_reader.y();
        // the reader stamps are stored such as to have a sum of 1.0 over all pixels
        // (i.e.: they express the percentage for each cell contributing to the full crown).
        // we thus calculate a the factor to "blow up" cell values; a fully covered cell has then a value of 1,
        // and values between 0-1 are cells that are partially covered by the crown.
        double crown_factor = reader->crownArea()/double(cPxSize*cPxSize);

        // add the reader-stamp values: multiple (partial) crowns can add up to being fully covered
        for (y=0;y<reader_size; ++y) {
            for (x=0;x<reader_size;++x) {
                 grid->valueAtIndex(rx+x, ry+y) += (*reader)(x,y)*crown_factor;
            }
        }
    }

}


/****************************************************************************************/
/********************************* RumpleIndex class ************************************/
/****************************************************************************************/


void RumpleIndex::setup()
{
    mRumpleGrid.clear();
    if (!GlobalSettings::instance()->model()) return;

    // the rumple grid hast the same dimensions as the resource unit grid (i.e. 100 meters)
    mRumpleGrid.setup(GlobalSettings::instance()->model()->RUgrid().metricRect(),
                    GlobalSettings::instance()->model()->RUgrid().cellsize());

}

void RumpleIndex::calculate()
{
    if (mRumpleGrid.isEmpty())
        setup();

    mRumpleGrid.initialize(0.f);
    HeightGrid *hg = GlobalSettings::instance()->model()->heightGrid();

    // iterate over the resource units and calculate the rumple index / surface area for each resource unit
    HeightGridValue* hgv_8[8]; // array holding pointers to height grid values (neighborhood)
    float heights[9];  // array holding heights (8er neighborhood + center pixel)
    int total_valid_pixels = 0;
    float total_surface_area = 0.f;
    for (float *rg = mRumpleGrid.begin(); rg!=mRumpleGrid.end();++rg) {
        int valid_pixels = 0;
        float surface_area_sum = 0.f;
        GridRunner<HeightGridValue> runner(hg, mRumpleGrid.cellRect(mRumpleGrid.indexOf(rg)));
        while (runner.next()) {
            if (runner.current()->isValid()) {
                runner.neighbors8(hgv_8);
                bool valid = true;
                float *hp = heights;
                *hp++ = runner.current()->height;
                // retrieve height values from the grid
                for (int i=0;i<8;++i) {
                    *hp++ = hgv_8[i] ? hgv_8[i]->height: 0;
                    if (hgv_8[i] && !hgv_8[i]->isValid()) valid = false;
                    if (!hgv_8[i]) valid = false;
                }
                // calculate surface area only for cells which are (a) within the project area, and (b) all neighboring pixels are inside the forest area
                if (valid) {
                   valid_pixels++;
                   float surface_area = calculateSurfaceArea(heights, hg->cellsize());
                   surface_area_sum += surface_area;
                }
            }
        }
        if (valid_pixels>0) {
            float rumple_index = surface_area_sum / ( (float) valid_pixels * hg->cellsize()*hg->cellsize());
            *rg = rumple_index;
            total_valid_pixels += valid_pixels;
            total_surface_area += surface_area_sum;
        }
    }
    mRumpleIndex = 0.;
    if (total_valid_pixels>0) {
        float rumple_index = total_surface_area / ( (float) total_valid_pixels * hg->cellsize()*hg->cellsize());
        mRumpleIndex = rumple_index;
    }
    mLastYear = GlobalSettings::instance()->currentYear();
}

double RumpleIndex::value(const bool force_recalculate)
{
    if (force_recalculate ||  mLastYear != GlobalSettings::instance()->currentYear())
        calculate();
    return mRumpleIndex;
}

double RumpleIndex::test_triangle_area()
{
    // check calculation: numbers for Jenness paper
    float hs[9]={165, 170, 145, 160, 183, 155,122,175,190};
    double area = calculateSurfaceArea(hs, 100);
    return area;
}

inline double surface_length(float h1, float h2, float l)
{
    return sqrt((h1-h2)*(h1-h2) + l*l);
}
inline double heron_triangle_area(float a, float b, float c)
{
    float s=(a+b+c)/2.f;
    return sqrt(s * (s-a) * (s-b) * (s-c) );
}

/// calculate the surface area of a pixel given its height value, the height of the 8 neigboring pixels, and the cellsize
/// the algorithm is based on http://www.jennessent.com/downloads/WSB_32_3_Jenness.pdf
double RumpleIndex::calculateSurfaceArea(const float *heights, const float cellsize)
{
    // values in the height array [0..8]: own height / north/east/west/south/ NE/NW/SE/SW
    // step 1: calculate length on 3d surface between all edges
    //   8(A) * 1(B) * 5(C)       <- 0: center cell, indices in the "heights" grid, A..I: codes used by Jenness
    //   4(D) * 0(E) * 2(F)
    //   7(G) * 3(H) * 6(I)

    float slen[16]; // surface lengths (divided by 2)
    // horizontal
    slen[0] = surface_length(heights[8], heights[1], cellsize)/2.f;
    slen[1] = surface_length(heights[1], heights[5], cellsize)/2.f;
    slen[2] = surface_length(heights[4], heights[0], cellsize)/2.f;
    slen[3] = surface_length(heights[0], heights[2], cellsize)/2.f;
    slen[4] = surface_length(heights[7], heights[3], cellsize)/2.f;
    slen[5] = surface_length(heights[3], heights[6], cellsize)/2.f;
    // vertical
    slen[6] = surface_length(heights[8], heights[4], cellsize)/2.f;
    slen[7] = surface_length(heights[1], heights[0], cellsize)/2.f;
    slen[8] = surface_length(heights[5], heights[2], cellsize)/2.f;
    slen[9] = surface_length(heights[4], heights[7], cellsize)/2.f;
    slen[10] = surface_length(heights[0], heights[3], cellsize)/2.f;
    slen[11] = surface_length(heights[2], heights[6], cellsize)/2.f;
    // diagonal
    float cellsize_diag = cellsize * M_SQRT2;
    slen[12] = surface_length(heights[0], heights[8], cellsize_diag)/2.f;
    slen[13] = surface_length(heights[0], heights[5], cellsize_diag)/2.f;
    slen[14] = surface_length(heights[0], heights[7], cellsize_diag)/2.f;
    slen[15] = surface_length(heights[0], heights[6], cellsize_diag)/2.f;

    // step 2: combine the three sides of all the 8 sub triangles using Heron's formula
    double surface_area = 0.;
    surface_area += heron_triangle_area(slen[12], slen[0], slen[7]); // i
    surface_area += heron_triangle_area(slen[7], slen[1], slen[13]); // ii
    surface_area += heron_triangle_area(slen[6], slen[2], slen[12]); // iii
    surface_area += heron_triangle_area(slen[13], slen[8], slen[3]); // iv
    surface_area += heron_triangle_area(slen[2], slen[9], slen[14]); // v
    surface_area += heron_triangle_area(slen[3], slen[11], slen[15]); // vi
    surface_area += heron_triangle_area(slen[14], slen[10], slen[4]); // vii
    surface_area += heron_triangle_area(slen[10], slen[15], slen[5]); // viii

    return surface_area;
}

/* *************************************************************************************** */
/* ******************************* Spatial Layers **************************************** */
/* *************************************************************************************** */
void SpatialLayeredGrid::setup()
{
    addGrid("rumple", 0);
}

void SpatialLayeredGrid::createGrid(const int grid_index)
{
    Q_UNUSED(grid_index); // TODO: what should happen here?
}


int SpatialLayeredGrid::addGrid(const QString name, FloatGrid *grid)
{
    mGridNames.append(name);
    mGrids.append(grid);
    return mGrids.size();
}



