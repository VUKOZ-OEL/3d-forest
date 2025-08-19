#ifndef GEOTIFF_H
#define GEOTIFF_H

#include <QString>


struct FIBITMAP;
template <typename T>
class Grid; // forward

/**
 * @brief The GeoTIFF class is a wrapper for handling GeoTIFF format in
 * the FreeImage library.
 * The class supports reading TIFF files (loadImage()) and writing
 * TIFF files (initialize() + setValue() + saveToFile()).
 */


class GeoTIFF
{
public:
    GeoTIFF();
    ~GeoTIFF();
    // constants from FREE_IMAGE_TYPE (FreeImage.h)
    enum TIFDatatype { DTUNKNOWN = 0,
                       DTSINT16		= 3,	//!  16-bit signed integer
                       DTSINT32		= 5,	//!  32-bit signed integer
                       DTFLOAT		= 6,	//! 32-bit IEEE floating point
                       DTDOUBLE		= 7    //! 64-bit IEEE floating point
    };

    static void clearProjection();

    int loadImage(const QString &fileName);

    void copyToIntGrid(Grid<int> *grid);
    void copyToDoubleGrid(Grid<double> *grid);
    void copyToFloatGrid(Grid<float> *grid);

    // write Grid to file + free memory
    bool saveToFile(const QString &fileName);
    /// create a bitmap with the the size of the full grid, and provide the data type
    void initialize(uint width, uint height, TIFDatatype dtype=DTDOUBLE);
    /// set value at ix/iy to *double* value
    void setValue(uint ix, uint iy, double value);
    /// set value at ix/iy to *float* value
    void setValue(uint ix, uint iy, float value) {setValue(ix, iy, static_cast<double>(value));}
    /// set value at ix/iy to int* value
    void setValue(uint ix, uint iy, int value) {setValue(ix, iy, static_cast<double>(value));}
    /// set value at ix/iy to *short int* value
    void setValue(uint ix, uint iy, short int value) {setValue(ix, iy, static_cast<double>(value));}

    double noDataValue() const { return mNoDataValue; }
    // getters
    double ox() const { return mOx; }
    double oy() const { return mOy; }
    double cellsize() const { return mCellsize; }
    uint ncol() const { return mNcol; }
    uint nrow() const { return mNrow; }
private:
    static short int noDataShort() { return std::numeric_limits<short int>::lowest(); }
    static  int noDataInt() { return std::numeric_limits<int>::lowest(); }
    static  float noDataFloat() { return std::numeric_limits<float>::lowest(); }
    static  double noDataDouble() { return std::numeric_limits<double>::lowest(); }
    static FIBITMAP *mProjectionBitmap;
    FIBITMAP *dib;
    TIFDatatype mDType;

    double mOx, mOy;
    double mCellsize;
    uint mNcol, mNrow;

    double mNoDataValue;
};

#endif // GEOTIFF_H
