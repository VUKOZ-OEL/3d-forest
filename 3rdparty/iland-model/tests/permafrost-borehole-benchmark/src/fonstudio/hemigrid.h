//---------------------------------------------------------------------------

#ifndef HemiGrid_H
#define HemiGrid_H

/** HemiGrid represents a grid covering the hemisphehre as well as some operations.
    The sky is modeled as equally sized tiles.
    Coordinate system:
    azimuth: angle between -pi .. +pi, where 0=south direction
    elevation: 0: horizon - pi/2 (90°)
  */
class HemiGrid
{
 public:
    HemiGrid(): mMatrix(0), mMatrixCountAzimuth(-1),
                     mMatrixCountElevation(-1) {}
    ~HemiGrid() { if (mMatrix) delete[] mMatrix; }
    HemiGrid(const HemiGrid& grid) { setup(mCellSizeDegree); modify(grid, HemiGrid::SetTo); }

    void setup(double cellsize_degree);
    void clear(double SetWith = 0.);
    //void DumpGrid(TStrings* List);

    double& rGetByIndex(const int iAzimuth, const int iElevation) const;
    double& rGet(const double Azimuth, const double Elevation) const;

    /// get azimuth index of given azimuth angle. angle should be -pi .. pi.
    int indexAzimuth(double Azimuth) const { int x=int( (Azimuth + M_PI) / (2.*M_PI) * mMatrixCountAzimuth ); if (x==mMatrixCountAzimuth) --x; return x;}
    /// get index of elevation (0..pi/2)
    int indexElevation(double Elevation) const { int x=int( Elevation / (M_PI / 2.) * mMatrixCountElevation ); if (x==mMatrixCountElevation) --x; return x;}

    /// get azimuth angle associated with given index. @return from -pi .. pi.
    double azimuth(const int iAzimuth) { return iAzimuth/double(mMatrixCountAzimuth) * M_PI*2 - M_PI; }
    /// get azimuth angle associated with given index. @return 0 (North) -> pi/2 (West) -> pi (South) -> 3pi/2 (East)
    double azimuthNorth(const int iAzimuth) { double d = iAzimuth/double(mMatrixCountAzimuth) * M_PI*2 - M_PI/2.; return d; }
    /// get elevation angle associated with given index. @return from 0..pi/2
    double elevation(const int iElevation) { return iElevation/double(mMatrixCountElevation) * M_PI_2; }

    int matrixCountAzimuth() const { return mMatrixCountAzimuth; }
    int matrixCountElevation() const  { return mMatrixCountElevation; }
    /// get mininum and maximum value in the grid.
    void matrixMinMax(double &rMatrixMin, double &rMatrixMax) const;
    /// get sum of all cell values with elevation >= "elevation".
    double sum(const double elevation=0) const;

    static void projectLine(const double &x, const double &y, const double &deltah, const double &r,
        double &elevation, double &azimuth1, double &azimuth2);

    enum ModifyMode { Add, Multiply, SetTo };
    /** combine two Hemigrids.
        use "Source" and combine it with curernt grid using the combine mode "mode".
        Available modes are: Add, Multiply, Set (=copy).
        @param Source Hemigrid used as source (is const, and not modified).
        @param mode Combine mode (Add, Multiply, SetTo) */
    void modify(const HemiGrid &Source, const ModifyMode mode);

    void projectCylinder(const double &deltax, const double &deltay,
                         const double &offsetz, const double &height, const double &r,
                         const ModifyMode mode, const double &value);
    void modifyAngleRect( const double &elow, const double &alow1, const double &alow2,
                    const double &ehigh, const double &ahigh1, const double &ahigh2,
                    const ModifyMode mode, const double &value);
    double getSum(const HemiGrid *Weighter=NULL) const;
    // drawing/output
    void paintGrid(QImage &image) const;
    QString dumpGrid() const;

 private:
    double* mMatrix;
    int mMatrixCountAzimuth;
    int mMatrixCountElevation;
    double mMatrixCellSize;
    double mCellSizeDegree;
};

//---------------------------------------------------------------------------
#endif
