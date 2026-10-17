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

#include "gisgrid.h"
#include <stdexcept>
#include "helper.h"

#include "globalsettings.h"
#include "model.h"

/** @class GisGrid
  @ingroup tools
  GisGrid encapsulates a simple grid of values based on GIS data.
  GisGrid can load input files in ESRI text file format (loadFromFile()) and transforms
  coordinates to the current reference in iLand.

  */

// global transformation record:
static SCoordTrans GISCoordTrans;

// setup of global GIS transformation
// not a good place to put that code here.... please relocate!
void setupGISTransformation(double offsetx, double offsety, double offsetz, double angle_degree)
{
    GISCoordTrans.setupTransformation(offsetx, offsety, offsetz, angle_degree);
}

void worldToModel(const Vector3D &From, Vector3D &To)
{
    double x=From.x() - GISCoordTrans.offsetX;
    double y=From.y() - GISCoordTrans.offsetY;
    To.setZ( From.z() - GISCoordTrans.offsetZ );
    To.setX( x * GISCoordTrans.cosRotate - y*GISCoordTrans.sinRotate);
    To.setY( x * GISCoordTrans.sinRotate + y*GISCoordTrans.cosRotate);
    //To.setY(-To.y()); // spiegeln
}
void modelToWorld(const Vector3D &From, Vector3D &To)
{
    double x=From.x();
    double y=From.y(); // spiegeln
    To.setX( x * GISCoordTrans.cosRotateReverse - y*GISCoordTrans.sinRotateReverse + GISCoordTrans.offsetX);
    To.setY( x * GISCoordTrans.sinRotateReverse + y*GISCoordTrans.cosRotateReverse  + GISCoordTrans.offsetY);
    To.setZ( From.z() + GISCoordTrans.offsetZ );
}


GisGrid::GisGrid()
{
    mData=0;
    mNRows=0; mNCols=0;
    mCellSize = 1; // default value (for line mode)
}

GisGrid::~GisGrid()
{
    if (mData)
        delete[] mData;
}

bool GisGrid::loadFromFile(const QString &fileName)
{
    min_value = 1000000000;
    max_value = -1000000000;

    // loads from a ESRI-Grid [RasterToFile] File.
    QByteArray file_content = Helper::loadTextFile(fileName).toLatin1();
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
    do {
        if (pos>lines.count())
            throw IException("GISGrid: unexpected end of file ");
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
                mNCols=(int)value;
            else if (key=="nrows")
                mNRows=int(value);
            else if (key=="xllcorner")
                mOrigin.setX(value);
            else if (key=="yllcorner")
                mOrigin.setY(value);
            else if (key=="cellsize")
                mCellSize = value;
            else if (key=="nodata_value")
                mNODATAValue=value;
            else
                throw IException( QString("GISGrid: invalid key %1.").arg(key));
            pos++;
        }
    } while (header);

    // create data
    if (mData)
        delete[] mData;
    mDataSize = mNRows * mNCols;
    mData = new double[mDataSize];


    // loop thru datalines
    int i,j;
    char *p=0;
    char *p2;
    pos--;
    for (i=mNRows-1;i>=0;i--)
        for (j=0;j<mNCols;j++) {
        // copy next value to buffer, change "," to "."
        if (!p || *p==0) {
            pos++;
            if (pos>=lines.count())
                throw IException("GISGrid: Unexpected End of File!");
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
            if (value!=mNODATAValue) {
                min_value=std::min(min_value, value);
                max_value=std::max(max_value, value);
            }
            mData[i*mNCols + j] = value;
            // skip text...
            while (*p && !strchr(" \r\n\t", *p))
                p++;
        } else
            j--;
    }

    return true;
}

QList<double> GisGrid::distinctValues()
{
    if (!mData)
        return QList<double>();
    QMap<double, double> temp_map;
    for (int i=0;i<mDataSize;i++)
        temp_map[mData[i]] = 1.;
    temp_map.remove(mNODATAValue);
    return temp_map.keys();
}

QPointF GisGrid::modelToWorld(QPointF model_coordinates)
{
    Vector3D to;
    ::modelToWorld(Vector3D(model_coordinates.x(), model_coordinates.y(), 0.), to);
    return QPointF(to.x(), to.y());
}

QPointF GisGrid::worldToModel(QPointF world_coordinates)
{
    Vector3D to;
    ::worldToModel(Vector3D(world_coordinates.x(), world_coordinates.y(), 0.), to);
    return QPointF(to.x(), to.y());

}

/*
void GISGrid::GetDistinctValues(TStringList *ResultList, double x_m, double y_m)
{
   // alle "distinct" values in einem rechteck (picus-koordinaten)
   // herauslesen. geht nur mit integers.
    double stepsize=CellSize/2; //  default stepsize, die haelfte der Cellsize, damit sollten alle pixel ueberstrichen werden.
    double x=0, y=0;
    int v;
    TList *List=new TList;
    while (x<=x_m) {
       y=0;
       while (y<=y_m) {
          v=value(x,y);
          if (List->IndexOf((void*)v)==-1)
             List->Add((void*)v);
          y+=stepsize;
       }
       x+=stepsize;
    }
    ResultList->Clear();
    for (int i=0;i<List->Count;i++)
       ResultList->Add(AnsiString((int)List->Items[i]));
    delete List;

}*/

/// get value of grid at index positions
double GisGrid::value(const int indexx, const int indexy) const
{
    if (indexx>=0 && indexx < mNCols && indexy>=0 && indexy<mNRows)
        return mData[indexy*mNCols + indexx];
    return -1.;  // out of scope
}

/// get value of grid at index positions
double GisGrid::value(const int Index) const
{
    if (Index>=0 && Index<mDataSize)
        return mData[Index];
    return -1.;  // out of scope
}

double GisGrid::value(const double X, const double Y) const
{

    Vector3D model;
    model.setX(X);
    model.setY(Y);
    model.setZ(0.);
    Vector3D world;
    ::modelToWorld(model, world);


    world.setX(world.x() - mOrigin.x());
    world.setY(world.y() - mOrigin.y());


    // get value out of grid.
    // double rx = Origin.x + X * xAxis.x + Y * yAxis.x;
    // double ry = Origin.y + X * xAxis.y + Y * yAxis.y;
    if (world.x()<0. || world.y()<0.)
        return -1.;
    int ix = world.x() / mCellSize;
    int iy = world.y() / mCellSize;
    if (ix>=0 && ix<mNCols && iy>=0 && iy<mNRows) {
        double value = mData[iy*mNCols + ix];
        if (value!=mNODATAValue)
            return value;
    }
    return -1.; // the ultimate NODATA- or ErrorValue
}

Vector3D GisGrid::coord(const int indexx, const int indexy) const
{
    Vector3D world((indexx+0.5)*mCellSize + mOrigin.x(),
                    (indexy+0.5)*mCellSize + mOrigin.y(),
                    0.);
    Vector3D model;
    ::worldToModel(world, model);
    return model;
}

QRectF GisGrid::rectangle(const int indexx, const int indexy) const
{
    Vector3D world(indexx*mCellSize + mOrigin.x(),
                    indexy*mCellSize + mOrigin.y(),
                    0.);
    Vector3D model;
    ::worldToModel(world, model);
    QRectF rect(model.x(), // left
                model.y(), // top
                mCellSize, // width
                mCellSize); // height
    return rect;
}

Vector3D GisGrid::coord(const int Index) const
{
    if (Index<0 || Index>=mDataSize)
        throw IException("gisgrid:coord: invalid index.");
    int ix = Index%mNCols;
    int iy = Index / mNCols;
    return coord(ix,iy);

}


/*

void GISGrid::CountOccurence(int intID, int & Count, int & left, int & upper, int &right, int &lower, QRectF *OuterBox)
{
        // zaehlt, wie of intID im Grid vorkommt,
        // ausserdem das rectangle, in dem es vorkommt.
        // rectangle ist durch indices [z.b. 0..NRows-1] und nicht laengen definiert!
        int ix,iy;
        Count=0;
        left=100000;
        right=-1;
        upper=100000;
        lower=-1;
        QVector3D akoord;
        for (ix=0;ix<mNCols;ix++)
           for (iy=0;iy<mNRows;iy++)
               if (mData[iy*mNCols + ix]==intID) {
                    // gefunden!
                    // innerhalb der Box?
                    if (OuterBox) {
                       akoord = koord(iy*mNCols + ix);
                       if (akoord.x<OuterBox->x1 || akoord.x>OuterBox->x2 || akoord.y<OuterBox->y1 || akoord.y>OuterBox->y2)
                           continue; // nicht zaehlen, falls punkt ausserhalb rect.
                    }
                    Count++;
                    left=ix<left?ix:left;
                    upper=iy<upper?iy:upper;
                    right=ix>right?ix:right;
                    lower=iy>lower?iy:lower;
              }
        if (Count==0)
           left=upper=right=lower=-1; // if not found.

}
*/
/*
QVector3D GISGrid::GetNthOccurence(int ID, int N, int left, int upper, int right, int lower)
{
        // aus dem (index-)rectangle left/upper..right/lower
        // das N-te vorkommen von "ID" heraussuchen.
        // das ergebnis sind die koordinaten des mittelpunktes der grid-zelle.
        int ix,iy;
        int Counter=0;
        for (ix=left;ix<=right;ix++)
           for (iy=upper;iy<=lower;iy++)
               if (mData[iy*mNCols+ix]==ID) {
                   Counter++;
                   if (Counter==N) {  // N-tes vorkommen gefunden!!!
                       // Picus-Koordinaten zurueckgeben.
                       return koord(iy*mNCols + ix);
                   }
               }
        // n-tes vorkommen nicht gefunden!!
        throw Exception("GISGrid:getNthOccurence. ID="+AnsiString(ID)+", N="+AnsiString(N)+" nicht gefunden!");
}
*/
/*
bool GISGrid::GetBoundingBox(int LookFor, QRectF &Result, double x_m, double y_m)
{
     // alle "distinct" values in einem rechteck (picus-koordinaten)
     // herauslesen. geht nur mit integers.
      double stepsize=CellSize/2; //  default stepsize, die haelfte der Cellsize, damit sollten alle pixel ueberstrichen werden.
      double x=0, y=0;
      int v;
      Result.x1 = 1000000; Result.x2 = -10000000;
      Result.y1 = 1000000; Result.y2 = -10000000;
      bool Found = false;
      while (x<=x_m) {
         y=0;
         while (y<=y_m) {
            v=value(x,y);
            if (v==LookFor) {
               Result.x1 = Min(Result.x1, x);
               Result.x2 = Max(Result.x2, x);
               Result.y1 = Min(Result.y1, y);
               Result.y2 = Max(Result.y2, y);
               Found = true;
            }
            y+=stepsize;
         }
         x+=stepsize;
      }
      return Found;
}
*/

void GisGrid::clip(const QRectF & box)
{
    // auf das angegebene Rechteck zuschneiden, alle
    // werte draussen auf -1 setzen.
    int ix,iy;
    Vector3D akoord;
    for (ix=0;ix<mNCols;ix++)
        for (iy=0;iy<mNRows;iy++) {
        akoord = coord(iy*mNCols + ix);
        if (!box.contains(akoord.x(), akoord.y()))
            mData[iy*mNCols + ix]=-1.;
        }

}


/*
void GISGrid::ExportToTable(AnsiString OutFileName)
{
    TStringList *Result = new TStringList();
    AnsiString Line;
    int ix,iy;
    double Value;
    for (ix=0;ix<mNCols;ix++)
        for (iy=0;iy<mNRows;iy++) {
           Value = mData[iy*mNCols + ix];
           if (Value != mNODATAValue) {
             Line.sprintf("%d;%d;%f", ix, iy, Value);
             Result->Add(Line);
           }
        }
    Result->SaveToFile(OutFileName);
    delete Result;
}
*/
