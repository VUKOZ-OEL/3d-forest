/********************************************************************************************
**    iLand - an individual based forest landscape and disturbance model
**    http://iland-model.org
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

#ifndef GRID_H
#define GRID_H

#include <QtCore>


#include <stdexcept>
#include <limits>
#include <cstring>

#include "global.h"

/** Grid class (template).
@ingroup tools
Orientation
The grid is oriented as typically coordinates on the northern hemisphere: higher y-values -> north, higher x-values-> east.
The projection is reversed for drawing on screen (Viewport).
          N
  (0/2) (1/2) (2/2)
W (0/1) (1/1) (2/1)  E
  (0/0) (1/0) (2/0)
          S
*/
template <class T>
class Grid {
public:

    Grid();
    Grid(float cellsize, int sizex, int sizey) { mData=0; setup(cellsize, sizex, sizey); }
    /// create from a metric rect
    Grid(const QRectF rect_metric, const float cellsize) { mData=0; setup(rect_metric,cellsize); }
    /// load a grid from an ASCII grid file
    /// the coordinates and cell size remain as in the grid file.
    bool loadGridFromFile(const QString &fileName);
    // copy ctor
    Grid(const Grid<T>& toCopy);
    ~Grid() { clear(); }
    void clear() { if (mData) delete[] mData; mData=0; }

    bool setup(const float cellsize, const int sizex, const int sizey);
    bool setup(const QRectF& rect, const double cellsize);
    bool setup(const Grid<T>& source) { clear();  mRect = source.mRect; return setup(source.mRect, source.mCellsize); }
    void initialize(const T& value) {for( T *p = begin();p!=end(); ++p) *p=value; }
    void wipe(); ///< write 0-bytes with memcpy to the whole area
    void wipe(const T value); ///< overwrite the whole area with "value" size of T must be the size of "int" ERRORNOUS!!!
    /// copies the content of the source grid to this grid.
    /// no operation, if the grids are not of the same size.
    void copy(const Grid<T> &source) { if (source.count()==count()) memcpy(mData, source.mData, count()*sizeof(T)); }
    /// create a double grid (same size as this grid) and convert this grid to double values.
    /// NOTE: caller is responsible for freeing memory!
    Grid<double> *toDouble() const;

    // get the number of cells in x and y direction
    int sizeX() const { return mSizeX; }
    int sizeY() const { return mSizeY; }
    // get the size of the grid in metric coordinates (x and y direction)
    float metricSizeX() const { return mSizeX*mCellsize; }
    float metricSizeY() const { return mSizeY*mCellsize; }
    /// get the metric rectangle of the grid
    QRectF metricRect() const { return mRect; }
    /// set the metric rectangle. Use with care! No further checks are executed!
    void setMetricRect(QRectF rect) { mRect = rect; }
    /// get the rectangle of the grid in terms of indices
    QRect rectangle() const { return QRect(QPoint(0,0), QPoint(sizeX(), sizeY())); }
    /// get the length of one pixel of the grid
    float cellsize() const { return mCellsize; }
    int count() const { return mCount; } ///< returns the number of elements of the grid
    bool isEmpty() const { return mData==NULL; } ///< returns false if the grid was not setup
    // operations
    // query
    /// access (const) with index variables. use int.
    inline const T& operator()(const int ix, const int iy) const { return constValueAtIndex(ix, iy); }
    /// access (const) using metric variables. use float.
    inline const T& operator()(const float x, const float y) const { return constValueAt(x, y); }
    /// access value of grid with a QPoint
    inline const T& operator[](const QPoint &p) const { return constValueAtIndex(p); }
    /// use the square brackets to access by index
    inline T& operator[](const int idx) const { return mData[idx]; }
    /// use the square bracket to access by QPointF
    inline T& operator[] (const QPointF &p) { return valueAt(p); }
    /// use the square bracket to access by QPoint
    inline T& operator[] (const QPoint &p) { return valueAtIndex(p); }

    inline T& valueAtIndex(const QPoint& pos) {return valueAtIndex(pos.x(), pos.y());}  ///< value at position defined by a QPoint defining the two indices (x,y)
    T& valueAtIndex(const int ix, const int iy) { return mData[iy*mSizeX + ix];  } ///< const value at position defined by indices (x,y)
    T& valueAtIndex(const int index) {return mData[index]; } ///< get a ref ot value at (one-dimensional) index 'index'.
    inline int index(const int ix, const int iy) { return iy*mSizeX + ix; } ///< get the 0-based index of the cell with indices ix and iy.
    inline int index(const QPoint &pos) { return pos.y()*mSizeX + pos.x(); } ///< get the 0-based index of the cell at 'pos'.
    /// value at position defined by a (integer) QPoint
    inline const T& constValueAtIndex(const QPoint& pos) const {return constValueAtIndex(pos.x(), pos.y()); }
    /// value at position defined by a pair of integer coordinates
    inline const T& constValueAtIndex(const int ix, const int iy) const { return mData[iy*mSizeX + ix];  }
    /// value at position defined by the index within the grid
    const T& constValueAtIndex(const int index) const {return mData[index]; } ///< get a ref ot value at (one-dimensional) index 'index'.

    T& valueAt(const QPointF& posf); ///< value at position defined by metric coordinates (QPointF)
    const T& constValueAt(const QPointF& posf) const; ///< value at position defined by metric coordinates (QPointF)

    T& valueAt(const float x, const float y); ///< value at position defined by metric coordinates (x,y)
    const T& constValueAt(const float x, const float y) const; ///< value at position defined by metric coordinates (x,y)


    bool coordValid(const float x, const float y) const { return x>=mRect.left() && x<mRect.right()  && y>=mRect.top() && y<mRect.bottom(); }
    bool coordValid(const QPointF &pos) const { return coordValid(pos.x(), pos.y()); }

    QPoint indexAt(const QPointF& pos) const { return QPoint(int((pos.x()-mRect.left()) / mCellsize),  int((pos.y()-mRect.top())/mCellsize)); } ///< get index of value at position pos (metric)
    /// get index (x/y) of the (linear) index 'index' (0..count-1)
    QPoint indexOf(const int index) const {return QPoint(index % mSizeX,  index / mSizeX); }
    bool isIndexValid(const QPoint& pos) const { return (pos.x()>=0 && pos.x()<mSizeX && pos.y()>=0 && pos.y()<mSizeY); } ///< return true, if position is within the grid
    bool isIndexValid(const int x, const int y) const {return (x>=0 && x<mSizeX && y>=0 && y<mSizeY); } ///< return true, if index is within the grid

    /// returns the index of an aligned grid (with the same size and matching origin) with the double cell size (e.g. to scale from a 10m grid to a 20m grid)
    int index2(int idx) const {return ((idx/mSizeX)/2)*(mSizeX/2) + (idx%mSizeX)/2; }
    /// returns the index of an aligned grid (the same size) with the 5 times bigger cells (e.g. to scale from a 2m grid to a 10m grid)
    int index5(int idx) const {return ((idx/mSizeX)/5)*(mSizeX/5) + (idx%mSizeX)/5; }
    /// returns the index of an aligned grid (the same size) with the 10 times bigger cells (e.g. to scale from a 2m grid to a 20m grid)
    int index10(int idx) const {return ((idx/mSizeX)/10)*(mSizeX/10) + (idx%mSizeX)/10; }

    /// force @param pos to contain valid indices with respect to this grid.
    void validate(QPoint &pos) const{ pos.setX( qMax(qMin(pos.x(), mSizeX-1), 0) );  pos.setY( qMax(qMin(pos.y(), mSizeY-1), 0) );} ///< ensure that "pos" is a valid key. if out of range, pos is set to minimum/maximum values.
    /// get the (metric) centerpoint of cell with index @p pos
    QPointF cellCenterPoint(const QPoint &pos) const { return QPointF( (pos.x()+0.5)*mCellsize+mRect.left(), (pos.y()+0.5)*mCellsize + mRect.top());} ///< get metric coordinates of the cells center
    /// get the metric cell center point of the cell given by index 'index'
    QPointF cellCenterPoint(const int &index) const { QPoint pos=indexOf(index); return QPointF( (pos.x()+0.5)*mCellsize+mRect.left(), (pos.y()+0.5)*mCellsize + mRect.top());}
    /// get the metric cell center point of the cell given py the pointer
    QPointF cellCenterPoint(T* ptr) { QPoint pos = indexOf(ptr); return cellCenterPoint(pos); }
    /// get the metric rectangle of the cell with index @pos
    QRectF cellRect(const QPoint &pos) const { QRectF r( QPointF(mRect.left() + mCellsize*pos.x(), mRect.top() + pos.y()*mCellsize),
                                                   QSizeF(mCellsize, mCellsize)); return r; } ///< return coordinates of rect given by @param pos.

    inline  T* begin() const { return mData; } ///< get "iterator" pointer
    inline  T* end() const { return mEnd; } ///< get iterator end-pointer
    inline QPoint indexOf(const T* element) const; ///< retrieve index (x/y) of the pointer element. returns -1/-1 if element is not valid.
    // special queries
    T max() const; ///< retrieve the maximum value of a grid
    T min() const; ///< retrieve the minimum value of a grid
    T sum() const; ///< retrieve the sum of the grid
    T avg() const; ///< retrieve the average value of a grid
    // modifying operations
    void add(const T& summand);
    void multiply(const T& factor);
    /// limit each cell value to (including) min_value and (including) max_value
    void limit(const T min_value, const T max_value);

    /// creates a grid with lower resolution and averaged cell values.
    /// @param factor factor by which grid size is reduced (e.g. 3 -> 3x3=9 pixels are averaged to 1 result pixel)
    /// @param offsetx, offsety: start averaging with an offset from 0/0 (e.g.: x=1, y=2, factor=3: -> 1/2-3/4 -> 0/0)
    /// @return Grid with size sizeX()/factor x sizeY()/factor
    Grid<T> averaged(const int factor, const int offsetx=0, const int offsety=0) const;
    /// normalized returns a normalized grid, in a way that the sum()  = @param targetvalue.
    /// if the grid is empty or the sum is 0, no modifications are performed.
    Grid<T> normalized(const T targetvalue) const;
    T* ptr(int x, int y) { return &(mData[y*mSizeX + x]); } ///< get a pointer to the element indexed by "x" and "y"
    inline double distance(const QPoint &p1, const QPoint &p2); ///< distance (metric) between p1 and p2
    const QPoint randomPosition() const; ///< returns a (valid) random position within the grid
    /// applies a flood fill algorithm to the grid starting at 'start'
    /// fills the contingent area with value 'old_color' with 'color' (or stops when 'max_fill' pixels have been filled)
    /// returns the number of filled pixels
    int floodFill(QPoint start, T old_color, T color, int max_fill=-1);
private:

    T* mData;
    T* mEnd; ///< pointer to 1 element behind the last
    QRectF mRect;
    float mCellsize; ///< size of a cell in meter
    int mSizeX; ///< count of cells in x-direction
    int mSizeY; ///< count of cells in y-direction
    int mCount; ///< total number of cells in the grid
};


typedef Grid<float> FloatGrid;

enum GridViewType { GridViewRainbow=0, GridViewRainbowReverse=1, GridViewGray=2, GridViewGrayReverse=3, GridViewHeat=4,
                    GridViewGreens=5, GridViewReds=6, GridViewBlues=7, GridViewTurbo=8,
                    GridViewBrewerDiv=10, GridViewBrewerQual=11, GridViewTerrain=12, GridViewCustom=14  };

/** @class GridRunner is a helper class to iterate over a rectangular fraction of a grid
*/
template <class T>
class GridRunner {
public:
    // constructors with a QRectF (metric coordinates)
    GridRunner(Grid<T> &target_grid, const QRectF &rectangle) {setup(&target_grid, rectangle);}
    GridRunner(const Grid<T> &target_grid, const QRectF &rectangle) {setup(&target_grid, rectangle);}
    GridRunner(Grid<T> *target_grid, const QRectF &rectangle) {setup(target_grid, rectangle);}
    // constructors with a QRect (indices within the grid)
    GridRunner(Grid<T> &target_grid, const QRect &rectangle) {setup(&target_grid, rectangle);}
    GridRunner(const Grid<T> &target_grid, const QRect &rectangle) {setup(&target_grid, rectangle);}
    GridRunner(Grid<T> *target_grid, const QRect &rectangle) {setup(target_grid, rectangle);}
    GridRunner(Grid<T> *target_grid) {setup(target_grid, target_grid->rectangle()); }
    T* next(); ///< to to next element, return NULL if finished
    /// return the current element, or NULL
    T* current() const { return mCurrent; }
    /// return the first element
    T* first() const { return mFirst; }
    /// return the last element (not one element behind the last element!)
    T* last() const { return mLast; }
    /// checks if the state of the GridRunner is valid, returns false if out of scope
    bool isValid() const {return mCurrent>=mFirst && mCurrent<=mLast; }
    /// return the (index) - coordinates of the current position in the grid
    QPoint currentIndex() const { return mGrid->indexOf(mCurrent); }
    /// return the coordinates of the cell center point of the current position in the grid.
    QPointF currentCoord() const {return mGrid->cellCenterPoint(mGrid->indexOf(mCurrent));}
    void reset() { mCurrent = mFirst-1; mCurrentCol = -1; }
    /// set the internal pointer to the pixel at index 'new_index'. The index is relative to the base grid!
    void setPosition(QPoint new_index) { if (mGrid->isIndexValid(new_index)) mCurrent = const_cast<Grid<T> *>(mGrid)->ptr(new_index.x(), new_index.y()); else mCurrent=0; }
    // helpers
    /// fill array with pointers to neighbors (north, east, west, south)
    /// or Null-pointers if out of range.
    /// the target array (rArray) is not checked and must be valid!
    void neighbors4(T** rArray);
    void neighbors8(T** rArray);
private:
    void setup(const Grid<T> *target_grid, const QRectF &rectangle);
    void setup(const Grid<T> *target_grid, const QRect &rectangle);
    const Grid<T> *mGrid;
    T* mFirst; // points to the first element of the grid
    T* mLast; // points to the last element of the grid
    T* mCurrent;
    size_t mLineLength;
    size_t mCols;
    int mCurrentCol;
};

/** @class Vector3D is a simple 3d vector.
  QVector3D (from Qt) is in QtGui so we needed a replacement.
*/
class Vector3D
{
 public:
    Vector3D(): mX(0.), mY(0.), mZ(0.) {}
    Vector3D(const double x, const double y, const double z): mX(x), mY(y), mZ(z) {}
    double x() const { return mX; } ///< get x-coordinate
    double y() const { return mY; } ///< get y-coordinate
    double z() const { return mZ; } ///< get z-coordinate
    // set variables
    void setX(const double x) { mX=x; } ///< set value of the x-coordinate
    void setY(const double y) { mY=y; } ///< set value of the y-coordinate
    void setZ(const double z) { mZ=z; } ///< set value of the z-coordinate
private:
    double mX;
    double mY;
    double mZ;
};

// copy constructor
template <class T>
Grid<T>::Grid(const Grid<T>& toCopy)
{
    mData = 0;
    mRect = toCopy.mRect;
    setup(toCopy.metricRect(), toCopy.cellsize());
    //setup(toCopy.cellsize(), toCopy.sizeX(), toCopy.sizeY());
    const T* end = toCopy.end();
    T* ptr = begin();
    for (T* i= toCopy.begin(); i!=end; ++i, ++ptr)
       *ptr = *i;
}

// normalize function
template <class T>
Grid<T> Grid<T>::normalized(const T targetvalue) const
{
    Grid<T> target(*this);
    T total = sum();
    T multiplier;
    if (total)
        multiplier = targetvalue / total;
    else
        return target;
    for (T* p=target.begin();p!=target.end();++p)
        *p *= multiplier;
    return target;
}


template <class T>
Grid<T> Grid<T>::averaged(const int factor, const int offsetx, const int offsety) const
{
    Grid<T> target;
    target.setup(metricRect(), cellsize()*factor);
    int x,y;
    T sum=0;
    target.initialize(sum);
    // sum over array of 2x2, 3x3, 4x4, ...
    for (x=offsetx;x<mSizeX;x++)
        for (y=offsety;y<mSizeY;y++) {
            target.valueAtIndex((x-offsetx)/factor, (y-offsety)/factor) += constValueAtIndex(x,y);
        }
    // divide
    double fsquare = factor*factor;
    for (T* p=target.begin();p!=target.end();++p)
        *p /= fsquare;
    return target;
}


template <class T>
T&  Grid<T>::valueAt(const float x, const float y)
{
    return valueAtIndex( indexAt(QPointF(x,y)) );
}

template <class T>
const T&  Grid<T>::constValueAt(const float x, const float y) const
{
    return constValueAtIndex( indexAt(QPointF(x,y)) );
}

template <class T>
T&  Grid<T>::valueAt(const QPointF& posf)
{
    return valueAtIndex( indexAt(posf) );
}

template <class T>
const T&  Grid<T>::constValueAt(const QPointF& posf) const
{
    return constValueAtIndex( indexAt(posf) );
}

template <class T>
Grid<T>::Grid()
{
    mData = 0; mCellsize=0.f;
    mEnd = 0;
    mSizeX=0; mSizeY=0; mCount=0;
}

template <class T>
bool Grid<T>::setup(const float cellsize, const int sizex, const int sizey)
{
    mSizeX=sizex; mSizeY=sizey;
    if (mRect.isNull()) // only set rect if not set before (e.g. by call to setup(QRectF, double))
        mRect.setCoords(0., 0., cellsize*sizex, cellsize*sizey);

    if (mData) {
        // test if we can re-use the allocated memory.
        if (mSizeX*mSizeY > mCount || mCellsize != cellsize) {
            // we cannot re-use the memory - create new data
            delete[] mData; mData=NULL;
        }
    }
    mCellsize=cellsize;
    mCount = mSizeX*mSizeY;
    if (mCount==0)
        return false;
    if (mData==NULL)
        mData = new T[mCount];
    mEnd = &(mData[mCount]);
    return true;
}

template <class T>
bool Grid<T>::setup(const QRectF& rect, const double cellsize)
{
    mRect = rect;
    int dx = int(rect.width()/cellsize);
    if (mRect.left()+cellsize*dx<rect.right())
        dx++;
    int dy = int(rect.height()/cellsize);
    if (mRect.top()+cellsize*dy<rect.bottom())
        dy++;
    return setup(cellsize, dx, dy);
}

/** retrieve from the index from an element reversely from a pointer to that element.
    The internal memory layout is (for dimx=6, dimy=3):
0  1  2  3  4  5
6  7  8  9  10 11
12 13 14 15 16 17
Note: north and south are reversed, thus the item with index 0 is located in the south-western edge of the grid! */
template <class T> inline
QPoint Grid<T>::indexOf(const T* element) const
{
//    QPoint result(-1,-1);
    if (element==NULL || element<mData || element>=end())
        return QPoint(-1, -1);
    int idx = element - mData;
    return QPoint(idx % mSizeX,  idx / mSizeX);
//    result.setX( idx % mSizeX);
//    result.setY( idx / mSizeX);
//    return result;
}

template <class T>
T  Grid<T>::max() const
{
    T maxv = -std::numeric_limits<T>::max();
    T* p;
    T* pend = end();
    for (p=begin(); p!=pend;++p)
       maxv = std::max(maxv, *p);
    return maxv;
}

template <class T>
T  Grid<T>::min() const
{
    T maxv = std::numeric_limits<T>::max();
    T* p;
    T* pend = end();
    for (p=begin(); p!=pend;++p)
       maxv = std::min(maxv, *p);
    return maxv;
}

template <class T>
T  Grid<T>::sum() const
{
    T* pend = end();
    T total = 0;
    for (T *p=begin(); p!=pend;++p)
       total += *p;
    return total;
}

template <class T>
T  Grid<T>::avg() const
{
    if (count())
        return sum() / T(count());
    else return 0;
}

template <class T>
void Grid<T>::add(const T& summand)
{
    T* pend = end();
    for (T *p=begin(); p!=pend;*p+=summand,++p)
       ;
}

template <class T>
void Grid<T>::multiply(const T& factor)
{
    T* pend = end();
    for (T *p=begin(); p!=pend;*p*=factor,++p)
        ;
}

template <class T>
void Grid<T>::limit(const T min_value, const T max_value)
{
    T* pend = end();
    for (T *p=begin(); p!=pend;++p)
        *p = *p < min_value ? min_value : (*p>max_value? max_value : *p);
}



template <class T>
void  Grid<T>::wipe()
{
    memset(mData, 0, mCount*sizeof(T));
}
template <class T>
void  Grid<T>::wipe(const T value)
{
    /* this does not work properly !!! */
    if (sizeof(T)==sizeof(int)) {
        float temp = value;
        float *pf = &temp;

        memset(mData, *((int*)pf), mCount*sizeof(T));
    } else
        initialize(value);
}

template <class T>
Grid<double> *Grid<T>::toDouble() const
{
    Grid<double> *g = new Grid<double>();
    g->setup(metricRect(), cellsize());
    if (g->isEmpty())
        return g;
    double *dp = g->begin();
    for (T *p=begin(); p!=end(); ++p, ++dp)
        *dp = *p;
    return g;
}

template <class T>
double Grid<T>::distance(const QPoint &p1, const QPoint &p2)
{
    QPointF fp1=cellCenterPoint(p1);
    QPointF fp2=cellCenterPoint(p2);
    double distance = sqrt( (fp1.x()-fp2.x())*(fp1.x()-fp2.x()) + (fp1.y()-fp2.y())*(fp1.y()-fp2.y()));
    return distance;
}

template <class T>
const QPoint Grid<T>::randomPosition() const
{
    return QPoint(irandom(0,mSizeX), irandom(0, mSizeY));
}


// quick and dirty implementation of the flood fill algroithm.
// based on: http://en.wikipedia.org/wiki/Flood_fill
// returns the number of pixels colored
template<class T>
int Grid<T>::floodFill(QPoint start, T old_color, T color, int max_fill)
{

    QQueue<QPoint> pqueue;
    pqueue.enqueue(start);
    int found = 0;
    while (!pqueue.isEmpty()) {
        QPoint p = pqueue.dequeue();
        if (!isIndexValid(p))
            continue;
        if (valueAtIndex(p)==old_color) {
            valueAtIndex(p) = color;
            pqueue.enqueue(p+QPoint(-1,0));
            pqueue.enqueue(p+QPoint(1,0));
            pqueue.enqueue(p+QPoint(0,-1));
            pqueue.enqueue(p+QPoint(0,1));
            pqueue.enqueue(p+QPoint(1,1));
            pqueue.enqueue(p+QPoint(1,-1));
            pqueue.enqueue(p+QPoint(-1,1));
            pqueue.enqueue(p+QPoint(-1,-1));
            ++found;
            if (max_fill > 0 && found>=max_fill)
                break;
        }
    }
    return found;

}

////////////////////////////////////////////////////////////
// grid runner
////////////////////////////////////////////////////////////
template <class T>
void GridRunner<T>::setup(const Grid<T> *target_grid, const QRect &rectangle)
{
    QPoint upper_left = rectangle.topLeft();
    // due to the strange behavior of QRect::bottom() and right():
    QPoint lower_right = rectangle.bottomRight();
    mCurrent = const_cast<Grid<T> *>(target_grid)->ptr(upper_left.x(), upper_left.y());
    mFirst = mCurrent;
    mCurrent--; // point to first element -1
    mLast = const_cast<Grid<T> *>(target_grid)->ptr(lower_right.x()-1, lower_right.y()-1);
    mCols = lower_right.x() - upper_left.x(); //
    mLineLength =  target_grid->sizeX() - mCols;
    mCurrentCol = -1;
    mGrid = target_grid;
//    qDebug() << "GridRunner: rectangle:" << rectangle
//             << "upper_left:" << target_grid.cellCenterPoint(target_grid.indexOf(mCurrent))
//             << "lower_right:" << target_grid.cellCenterPoint(target_grid.indexOf(mLast));
}

template <class T>
void GridRunner<T>::setup(const Grid<T> *target_grid, const QRectF &rectangle_metric)
{
    QRect rect(target_grid->indexAt(rectangle_metric.topLeft()),
               target_grid->indexAt(rectangle_metric.bottomRight()) );
    setup (target_grid, rect);
}

template <class T>
T* GridRunner<T>::next()
{
    if (mCurrent>mLast)
        return NULL;
    mCurrent++;
    mCurrentCol++;

    if (mCurrentCol >= int(mCols)) {
        mCurrent += mLineLength; // skip to next line
        mCurrentCol = 0;
    }
    if (mCurrent>mLast)
        return NULL;
    else
        return mCurrent;
}

template <class T>
/// get pointers the the 4-neighborhood
/// north, east, west, south
/// 0-pointers are returned for edge pixels.
void GridRunner<T>::neighbors4(T** rArray)
{
    // north:
    rArray[0] = mCurrent + mCols + mLineLength > mLast?0: mCurrent + mCols + mLineLength;
    // south:
    rArray[3] = mCurrent - (mCols + mLineLength) < mFirst?0: mCurrent -  (mCols + mLineLength);
    // east / west
    rArray[1] = mCurrentCol+1<int(mCols)? mCurrent + 1 : 0;
    rArray[2] = mCurrentCol>0? mCurrent-1 : 0;
}

/// get pointers to the 8-neighbor-hood
/// north/east/west/south/NE/NW/SE/SW
/// 0-pointers are returned for edge pixels.
template <class T>
void GridRunner<T>::neighbors8(T** rArray)
{
    neighbors4(rArray);
    // north-east
    rArray[4] = rArray[0] && rArray[1]? rArray[0]+1: 0;
    // north-west
    rArray[5] = rArray[0] && rArray[2]? rArray[0]-1: 0;
    // south-east
    rArray[6] = rArray[3] && rArray[1]? rArray[3]+1: 0;
    // south-west
    rArray[7] = rArray[3] && rArray[2]? rArray[3]-1: 0;

}

////////////////////////////////////////////////////////////
// global functions
////////////////////////////////////////////////////////////

/// dumps a FloatGrid to a String.
/// rows will be y-lines, columns x-values. (see grid.cpp)
QString gridToString(const FloatGrid &grid, const QChar sep=QChar(';'), const int newline_after=-1);

/// creates and return a QImage from Grid-Data.
/// @param black_white true: max_value = white, min_value = black, false: color-mode: uses a HSV-color model from blue (min_value) to red (max_value), default: color mode (false)
/// @param min_value, max_value min/max bounds for color calcuations. values outside bounds are limited to these values. defaults: min=0, max=1
/// @param reverse if true, color ramps are inversed (to: min_value = white (black and white mode) or red (color mode). default = false.
/// @return a QImage with the Grids size of pixels. Pixel coordinates relate to the index values of the grid.
QImage gridToImage(const FloatGrid &grid,
                   bool black_white=false,
                   double min_value=0., double max_value=1.,
                   bool reverse=false);


/** load into 'rGrid' the content of the image pointed at by 'fileName'.
    Pixels are converted to grey-scale and then transformend to a value ranging from 0..1 (black..white).
  */
bool loadGridFromImage(const QString &fileName, FloatGrid &rGrid);

/// template version for non-float grids (see also version for FloatGrid)
/// @param sep string separator
/// @param newline_after if <>-1 a newline is added after every 'newline_after' data values
template <class T>
        QString gridToString(const Grid<T> &grid, const QChar sep=QChar(';'), const int newline_after=-1)
{
    QString res;
    QTextStream ts(&res);

    int newl_counter = newline_after;
    for (int y=grid.sizeY()-1;y>=0;--y){
        for (int x=0;x<grid.sizeX();x++){
            ts << grid.constValueAtIndex(x,y) << sep;
            if (--newl_counter==0) {
                ts << "\r\n";
                newl_counter = newline_after;
            }
        }
        ts << "\r\n";
    }

    return res;
}

/// template version for non-float grids (see also version for FloatGrid)
/// @param valueFunction pointer to a function with the signature: QString func(const T&) : this should return a QString
/// @param sep string separator
/// @param newline_after if <>-1 a newline is added after every 'newline_after' data values
template <class T>
        QString gridToString(const Grid<T> &grid, QString (*valueFunction)(const T& value), const QChar sep=QChar(';'), const int newline_after=-1 )
        {
            QString res;
            QTextStream ts(&res);

            int newl_counter = newline_after;
            for (int y=grid.sizeY()-1;y>=0;--y){
                for (int x=0;x<grid.sizeX();x++){
                    ts << (*valueFunction)(grid.constValueAtIndex(x,y)) << sep;

                    if (--newl_counter==0) {
                        ts << "\r\n";
                        newl_counter = newline_after;
                    }
                }
                ts << "\r\n";
            }

            return res;
        }
void modelToWorld(const Vector3D &From, Vector3D &To);

template <class T>
    QString gridToESRIRaster(const Grid<T> &grid, QString (*valueFunction)(const T& value) )
{
        Vector3D model(grid.metricRect().left(), grid.metricRect().top(), 0.);
        Vector3D world;
        modelToWorld(model, world);
        QString result = QString("ncols %1\r\nnrows %2\r\nxllcorner %3\r\nyllcorner %4\r\ncellsize %5\r\nNODATA_value %6\r\n")
                                .arg(grid.sizeX())
                                .arg(grid.sizeY())
                                .arg(world.x(),0,'f').arg(world.y(),0,'f')
                                .arg(grid.cellsize()).arg(-9999);
        QString line =  gridToString(grid, valueFunction, QChar(' ')); // for special grids
        return result + line;
}

    template <class T>
        QString gridToESRIRaster(const Grid<T> &grid )
{
            Vector3D model(grid.metricRect().left(), grid.metricRect().top(), 0.);
            Vector3D world;
            modelToWorld(model, world);
            QString result = QString("ncols %1\r\nnrows %2\r\nxllcorner %3\r\nyllcorner %4\r\ncellsize %5\r\nNODATA_value %6\r\n")
                    .arg(grid.sizeX())
                    .arg(grid.sizeY())
                    .arg(world.x(),0,'f').arg(world.y(),0,'f')
                    .arg(grid.cellsize()).arg(-9999);
            QString line = gridToString(grid, QChar(' ')); // for normal grids (e.g. float)
            return result + line;
}


template <class T>
        bool Grid<T>::loadGridFromFile(const QString &fileName)
        {
            double min_value = 1000000000;
            double max_value = -1000000000;

            // loads from a ESRI-Grid [RasterToFile] File.
            QFile file(fileName);

            if (!file.open(QIODevice::ReadOnly)) {
                qDebug() << "Grid::loadGridFromFile: " << fileName << "does not exist!";
                return false;
            }
            QTextStream s(&file);
            //s.setCodec("UTF-8");
            QByteArray file_content=s.readAll().toLatin1();

            if (file_content.isEmpty()) {
                qDebug() << "GISGrid: file" << fileName << "not present or empty.";
                return false;
            }
            QList<QByteArray> lines = file_content.split('\n');

            // processing of header-data
            bool header=true;
            int pos=0;
            QString line;
            QString key;
            double value;
            int ncol=0, nrow=0;
            double cellsize=0;
            double ox=0., oy=0.;
            double no_data_val=0.;
            do {
                if (pos>lines.count())
                    throw IException("Grid load from ASCII file: unexpected end of file. File: " + fileName);
                line=lines[pos].simplified();
                if (line.length()==0 || line.at(0)=='#') {
                    pos++; // skip comments
                    continue;
                }
                key=line.left(line.indexOf(' ')).toLower();
                if (key.length()>0 && (key.at(0).isNumber() || key.at(0)=='-')) {
                    header=false;
                } else {
                    value = line.mid(line.indexOf(' ')).toDouble();
                    if (key=="ncols")
                        ncol=(int)value;
                    else if (key=="nrows")
                        nrow=int(value);
                    else if (key=="xllcorner")
                        ox = value;
                    else if (key=="yllcorner")
                       oy = value;
                    else if (key=="cellsize")
                        cellsize = value;
                    else if (key=="nodata_value")
                        no_data_val=value;
                    else
                        throw IException( QString("GISGrid: invalid key %1.").arg(key));
                    pos++;
                }
            } while (header);

            // create the grid
            QRectF rect(ox, oy, ncol*cellsize, nrow*cellsize);
            setup( rect, cellsize );


            // loop thru datalines
            int i,j;
            char *p=nullptr;
            char *p2;
            pos--;
            for (i=nrow-1;i>=0;i--)
                for (j=0;j<ncol;j++) {
                // copy next value to buffer, change "," to "."
                if (!p || *p==0) {
                    pos++;
                    if (pos>=lines.count())
                        throw std::logic_error("GISGrid: Unexpected End of File!");
                    p=lines[pos].data();
                    // replace chars
                    p2=p;
                    while (*p2) {
                        if (*p2==',')
                            *p2='.';
                        p2++;
                    }
                }
                // skip spaces
                while (*p && strchr(" \r\n\t", *p))
                    p++;
                if (*p) {
                    value = atof(p);
                    if (value!=no_data_val) {
                        min_value=std::min(min_value, value);
                        max_value=std::max(max_value, value);
                    }
                    valueAtIndex(j,i) = value;
                    // skip text...
                    while (*p && !strchr(" \r\n\t", *p))
                        p++;
                } else
                    j--;
            }

            return true;
        }


#endif // GRID_H
