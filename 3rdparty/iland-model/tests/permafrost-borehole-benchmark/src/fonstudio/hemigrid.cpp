
#include "hemigrid.h"

//#include <algorithm>
#include <QtCore>
// needed for drawing: image & color
#include <QImage>
#include <QColor>

//////////////////////////////////////////////////////
// Setup memory for the Grid.
// @param size of a single pixel in degree
//////////////////////////////////////////////////////
void HemiGrid::setup(double cellsize_degree)
{
      // setup grid...
      mMatrixCountAzimuth = int(360 / cellsize_degree);
      mMatrixCountElevation = int(90 / cellsize_degree);
      mCellSizeDegree = cellsize_degree;
      // size occupied by one pixel in rad
      mMatrixCellSize = cellsize_degree * M_PI / 180.;
      if (mMatrix)
        delete[] mMatrix;
      mMatrix = new double[mMatrixCountAzimuth*mMatrixCountElevation];

      clear();

}


//////////////////////////////////////////////////////
// Clear the Grid.
// @param SetWith value used to fill (default 0.)
//////////////////////////////////////////////////////
void HemiGrid::clear(double SetWith)
{
    if (mMatrix) {
      // blank matrix...
      for (int i=0;i<mMatrixCountAzimuth*mMatrixCountElevation;i++)
         mMatrix[i]=SetWith;
    }
}

void HemiGrid::matrixMinMax(double &rMatrixMin, double &rMatrixMax) const
{
    rMatrixMin = 100000000.;
    rMatrixMax = -1000000000;
    if (mMatrix) {
      // blank matrix...
      for (int i=0;i<mMatrixCountAzimuth*mMatrixCountElevation;i++) {
         rMatrixMin = std::min(mMatrix[i], rMatrixMin);
         rMatrixMax = std::max(mMatrix[i], rMatrixMax);
      }
    }
}
double HemiGrid::sum(const double elevation) const
{
    int ie=indexElevation(elevation);
    int emax=matrixCountElevation();
    int amax=matrixCountAzimuth();
    double value = 0;
    for (int e=ie;e<emax;e++)
        for (int a=0;a<amax;a++)
            value+=rGetByIndex(a,e);
    return value;
}

//////////////////////////////////////////////////////
// Dump the grid into a TStringList
//////////////////////////////////////////////////////
//void HemiGrid::DumpGrid(TStrings* List)
//{
//    AnsiString Line;
//    for (int i=0;i<mMatrixCountAzimuth;i++)  {
//       Line=AnsiString(i);
//       for (int j=0;j<mMatrixCountElevation;j++)  {
//           Line+=";"+AnsiString(rGetByIndex(i,j));
//       }
//       List->Add(Line);
//    }
//
//}

//////////////////////////////////////////////////////
// Get Gridvalue by integer internal grid indices
// @param iAzimuth index of azimuth (0..count(pixels)-1
// @param iElevation index of elevatin (0..count-1)
// @return ref. to grid-value
//////////////////////////////////////////////////////
double& HemiGrid::rGetByIndex(const int iAzimuth, const int iElevation) const
{
if (iAzimuth < mMatrixCountAzimuth && iElevation < mMatrixCountElevation
        && iAzimuth>=0 && iElevation>=0)
       return mMatrix[iAzimuth*mMatrixCountElevation + iElevation];
    else
       throw QString("TSolar::Rad::Get() - invalid indices");
}

//////////////////////////////////////////////////////
// Get Gridvalue
// @param Azimuth azimuth angle (-pi .. +pi, 0=south)
// @param Elevation elevation angle (0=horizon, pi/2: zenith)
// @return ref. to grid-value
//////////////////////////////////////////////////////
double& HemiGrid::rGet(const double Azimuth, const double Elevation) const
{
    // Azimuth goes from -pi .. +pi -> move to 0..2pi, scale to 0..1 and convert to integer indices
    int iAzimuth = indexAzimuth(Azimuth);
    // Elevation goes from 0..90° = 0..pi/2
    int iElevation = indexElevation(Elevation);

    return rGetByIndex(iAzimuth, iElevation);
}

void HemiGrid::modify(const HemiGrid &Source, const ModifyMode mode)
{
        for (int i=0; i<mMatrixCountAzimuth*mMatrixCountElevation; i++) {
            switch (mode) {
                case Add: mMatrix[i]+=Source.mMatrix[i]; break;
                case Multiply: mMatrix[i] *= Source.mMatrix[i]; break;
                case SetTo: mMatrix[i] = Source.mMatrix[i]; break;
            }
        }

}

/** retrieve total sum of the hemigrid.
  @param Weighter if available (non null) each cell value of this grid is multiplied with the weighter grid (note: no additional calculations are performed) */
double HemiGrid::getSum(const HemiGrid *Weighter) const
{
    double Sum=0.;
    if (Weighter) {
        if (Weighter->matrixCountAzimuth()!=this->matrixCountAzimuth()
            || Weighter->matrixCountElevation() != this->matrixCountElevation())
                throw QString("HemiGrid::getSum: invalid weighing object!");

        for (int i=0; i<mMatrixCountAzimuth*mMatrixCountElevation; i++) {
            Sum += mMatrix[i]*Weighter->mMatrix[i];
        }
        return Sum;
    }
    for (int i=0; i<mMatrixCountAzimuth*mMatrixCountElevation; i++) {
        Sum += mMatrix[i];
    }
    return Sum;
}

void HemiGrid::projectLine(const double &x, const double &y, const double &deltah, const double &r, double &elevation, double &azimuth1, double &azimuth2)
{
   // transform coordinates....
   // distance to footing point (x/y)
   double distance = sqrt(x*x + y*y);
   elevation = atan2(deltah, distance);
   double azimuth = atan2(x,y);
   // distance to point (x/y/z)
   double dist3 = sqrt(x*x+y*y+deltah*deltah);
   double azimuth_delta = atan2(r, dist3);
   azimuth1 = azimuth - azimuth_delta;
   azimuth2 = azimuth + azimuth_delta;
}

//////////////////////////////////////////////////////
// Modify a rect defined by coordinates, a mode and a value
// @param  elow, ehigh: elevation angles
// @param alow1, alow2: azimuth angles at base, alow1 must be < alow2
// @param ahigh1, ahigh2: azimuth angles at the top, ahigh1 must be < ahigh2
// @param ModifyMode multiply or add to the matrix
// @param Value value used for multiply/add,...
//////////////////////////////////////////////////////
void HemiGrid::modifyAngleRect( const double &elow, const double &alow1, const double &alow2,
                    const double &ehigh, const double &ahigh1, const double &ahigh2,
                    const ModifyMode mode, const double &value)
{
    int i_e_low = indexElevation(elow);
    int i_e_high = indexElevation(ehigh);
    int i_a_low1 = indexAzimuth(alow1);
    int i_a_low2 = indexAzimuth(alow2);
    int i_a_high1 = indexAzimuth(ahigh1);
    int i_a_high2 = indexAzimuth(ahigh2);
    int i_a_min = std::min(i_a_low1, i_a_high1);
    int i_a_max = std::max(i_a_low2, i_a_high2);

    for (int e = i_e_low; e<=i_e_high; e++) {
       for (int a = i_a_min; a<=i_a_max; a++) {
          double &ref = rGetByIndex((a+mMatrixCountAzimuth)%mMatrixCountAzimuth, e);
          switch (mode) {
            case Multiply: ref*=value; break;
            case Add: ref+=value; break;
            case SetTo: ref=value; break;
          }
       }
    }

}


//////////////////////////////////////////////////////
// Project a cylinder onto the grid.
// @param deltax, deltay: relative position
// @param offsetz: relative height of lower edge
// @param height, r: height and radius of cylinder
// @param mode, value: mode and value of change
// @param  elow, ehigh: elevation angles
//////////////////////////////////////////////////////
void HemiGrid::projectCylinder(const double &deltax, const double &deltay,
                     const double &offsetz, const double &height, const double &r,
                     const ModifyMode mode, const double &value)
{
    double elow, ehigh; // elevations...
    double al1, al2, ah1, ah2;
    projectLine(deltax, deltay, std::max(offsetz+height, 0.), r, ehigh, ah1, ah2);
    // ignore if top is t
    if (ehigh < 5.*M_PI/180.)
       return;
    projectLine(deltax, deltay, std::max(offsetz, 0.), r, elow, al1, al2);
    modifyAngleRect(elow, al1, al2, ehigh, ah1, ah2, mode, value);
}


void HemiGrid::paintGrid(QImage &image) const
{
    int dx = image.width();
    int dy = image.height();
    int ix,iy;
    double x,y;
    int maxsize = std::min(dx,dy);
    double mmin=0, mmax=0; // min and max values of matrix
    //if (mmax==0) mmax=1;
    matrixMinMax(mmin, mmax);
    QColor col;
    double phi, r, elevation, value;
    for (ix=0;ix<maxsize;ix++){
        for (iy=0;iy<maxsize;iy++){
            // to -1..1
            x = 2*(ix-maxsize/2.) / maxsize;
            y = 2*(iy-maxsize/2.) / maxsize;
            // get phi,r (polar coords)
            if (x==0 && y==0)
                continue;

            r = sqrt(x*x + y*y);
            if (r>=1)
                continue;
            phi = atan2(y,x);
            elevation = M_PI_2 - (r * M_PI_2);
            value = rGet(phi, elevation);
            value /= mmax;
            col = QColor::fromHsvF((1.-value)*0.666666 ,0.9, 0.9); // hue from 0..240 = red to blue
            image.setPixel(ix,maxsize-iy,col.rgb());
        }
    }
}

QString HemiGrid::dumpGrid() const
{
    QString text;
    text="rows: azimuth steps, cols: elevation. (first value in row: index of azimuth)\n";
    for (int i=0;i<mMatrixCountAzimuth;i++)  {
       text+=QString::number(i);
       for (int j=0;j<mMatrixCountElevation;j++)  {
           text+=";"+QString::number(rGetByIndex(i,j));
       }
       text+="\n";
    }
    return text;
}


