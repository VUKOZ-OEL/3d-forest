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


#ifndef GISGRID_H
#define GISGRID_H

#include <QString>
#include <QPointF>
#include <QRectF>

#include "grid.h"

struct SCoordTrans {
    SCoordTrans() { setupTransformation(0.,0.,0.,0.); }
    double RotationAngle;
    double sinRotate, cosRotate;
    double sinRotateReverse, cosRotateReverse;
    double offsetX, offsetY, offsetZ;
    void setupTransformation(double new_offsetx, double new_offsety, double new_offsetz, double angle_degree)
    {
        offsetX = new_offsetx;
        offsetY = new_offsety;
        offsetZ = new_offsetz;
        RotationAngle=angle_degree * M_PI / 180.;
        sinRotate=sin(RotationAngle);
        cosRotate=cos(RotationAngle);
        sinRotateReverse=sin(-RotationAngle);
        cosRotateReverse=cos(-RotationAngle);
    }
};

/** GIS Transformation
    The transformation is defined by three offsets (x,y,z) and a rotation information.
    the (x/y) denotes the real-world coordinates of the left lower edge of the grid (at least for the northern hemisphere), i.e. the (0/0).
    The z-offset is currently not used.

  */
// setup routine -- give a vector with offsets [m] and rotation angle.
void setupGISTransformation(const double offsetx,
                            const double offsety,
                            const double offsetz,
                            const double angle_degree);
// transformation routines.
void worldToModel(const Vector3D &From, Vector3D &To);
void modelToWorld(const Vector3D &From, Vector3D &To);
QPointF modelToWorld(QPointF model_coordinates);
QPointF worldToModel(QPointF world_coordinates);


class GisGrid
{
public:
    GisGrid();
    ~GisGrid();
    // maintenance
    bool loadFromFile(const QString &fileName); ///< load ESRI style text file
    // access
    int dataSize() const { return mDataSize; }   ///< number of data items (rows*cols)
    int rows() const { return mNRows; } ///< number of rows
    int cols() const { return mNCols; } ///< number of columns
    QPointF origin() const { return mOrigin; } ///< coordinates of the lower left corner of the grid
    double cellSize() const { return mCellSize; } ///< size of a cell (meters)
    double minValue() const { return min_value; } ///< minimum data value
    double maxValue() const { return max_value; } ///< maximum data value
    int noDataValue() const { return mNODATAValue; } ///< no data value of the grid
    /// get grid value at local coordinates (X/Y); returs NODATAValue if out of range
    /// @p X and @p Y are local coordinates.
    double value(const QPointF &p) const {return value(p.x(), p.y());}
    double value(const double X, const double Y) const;
    double value(const int indexx, const int indexy) const; ///< get value of grid at index positions
    double value(const int Index) const; ///< get value of grid at index positions
    /// get coordinates of the center of the cell with 'Index'
    Vector3D coord(const int Index) const;
    Vector3D coord(const int indexx, const int indexy) const;
    QRectF rectangle(const int indexx, const int indexy) const;
    void clip(const QRectF & box); ///< clip the grid to 'Box' (set values outside to -1)

    // special operations
    //QRectF GetBoundingBox(int LookFor, SBoundingBox &Result, double x_m, double y_m);
    QList<double> distinctValues(); ///< returns a list of distinct values
    //void GetDistinctValues(TStringList *ResultList, double x_m, double y_m);
    //void CountOccurence(int intID, int & Count, int & left, int &upper, int &right, int &lower, SBoundingBox *OuterBox);
    //S3dVector GetNthOccurence(int ID, int N, int left, int upper, int right, int lower);
    //void ExportToTable(AnsiString OutFileName);

    // global conversion functions between local and world coordinates
    // the world-context is created by calling setupGISTransformation() (once).
    /// convert model to world coordinates (metric)
    static QPointF modelToWorld(QPointF model_coordinates);
    /// convert world (i.e. GIS) to model coordinates (metric) (with 0/0 at lower left edge of project area)
    static QPointF worldToModel(QPointF world_coordinates);


private:

    int mDataSize;  ///< number of data items (rows*cols)
    double mCellSize;   // size of cells [m]
    double max_value;
    double min_value;
    QPointF mOrigin; // lowerleftcorner
    QPointF xAxis;  // transformed axis (moved, rotated)
    QPointF yAxis;
    int mNRows;
    int mNCols;     // count of rows and cols
    double *mData;
    int mNODATAValue;
};


// Cached GIS - Dataset
// template class.
/*
template <class T> class TGISData
{
public:
        TTypedList<T> *Items;

        TGISGrid *Grid;
        TGISData() {
           Items = new TTypedList<T>(true);
           Grid=0;
        }
        ~TGISData() {
           delete Items;
           //if (Grid)
           //  delete Grid;
        }
        int Add(T* Item) {
            return Items->Add(Item);
        }
        void Clear() { Items->Clear(); }
//        template <class T> * Select(int ID);
        T* Select(int ID) {
            for (int i=0;i<Items->Count;i++)
                      if (Items->Items[i]->ID == ID)
                         return Items->Items[i];
                   return 0;
        }
        T* SelectAt(double X, double Y) {
           if (!Grid)
              return 0;
           int CellValue = Grid->value(X,Y);
           return Select(CellValue);
        }
        //bool Select(int ID);
};

*/
//---------------------------------------------------------------------------
#endif
