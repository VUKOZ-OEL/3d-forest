#include "geotiff.h"


#include "grid.h"

// #include "../3rdparty/FreeImage/FreeImage.h"

enum FREE_IMAGE_FORMAT
{
    FIF_UNKNOWN= -1,
    FIF_BMP    = 0,
    FIF_ICO    = 1,
    FIF_JPEG= 2,
    FIF_JNG    = 3,
    FIF_KOALA= 4,
    FIF_LBM    = 5,
    FIF_IFF= FIF_LBM,
    FIF_MNG    = 6,
    FIF_PBM    = 7,
    FIF_PBMRAW= 8,
    FIF_PCD    = 9,
    FIF_PCX    = 10,
    FIF_PGM    = 11,
    FIF_PGMRAW= 12,
    FIF_PNG    = 13,
    FIF_PPM    = 14,
    FIF_PPMRAW= 15,
    FIF_RAS    = 16,
    FIF_TARGA= 17,
    FIF_TIFF= 18,
    FIF_WBMP= 19,
    FIF_PSD    = 20,
    FIF_CUT    = 21,
    FIF_XBM    = 22,
    FIF_XPM    = 23,
    FIF_DDS    = 24,
    FIF_GIF    = 25,
    FIF_HDR    = 26,
    FIF_FAXG3= 27,
    FIF_SGI    = 28,
    FIF_EXR    = 29,
    FIF_J2K    = 30,
    FIF_JP2    = 31,
    FIF_PFM    = 32,
    FIF_PICT= 33,
    FIF_RAW    = 34,
    FIF_WEBP= 35,
    FIF_JXR        = 36
};

enum FREE_IMAGE_MDMODEL
{
    FIMD_NODATA            = -1,
    FIMD_COMMENTS        = 0,    //! single comment or keywords
    FIMD_EXIF_MAIN        = 1,    //! Exif-TIFF metadata
    FIMD_EXIF_EXIF        = 2,    //! Exif-specific metadata
    FIMD_EXIF_GPS        = 3,    //! Exif GPS metadata
    FIMD_EXIF_MAKERNOTE = 4,    //! Exif maker note metadata
    FIMD_EXIF_INTEROP    = 5,    //! Exif interoperability metadata
    FIMD_IPTC            = 6,    //! IPTC/NAA metadata
    FIMD_XMP            = 7,    //! Abobe XMP metadata
    FIMD_GEOTIFF        = 8,    //! GeoTIFF metadata
    FIMD_ANIMATION        = 9,    //! Animation metadata
    FIMD_CUSTOM            = 10,    //! Used to attach other metadata types to a dib
    FIMD_EXIF_RAW        = 11    //! Exif metadata as a raw buffer
};

enum FREE_IMAGE_MDTYPE
{
    FIDT_NOTYPE        = 0,    //! placeholder 
    FIDT_BYTE        = 1,    //! 8-bit unsigned integer 
    FIDT_ASCII        = 2,    //! 8-bit bytes w/ last byte null 
    FIDT_SHORT        = 3,    //! 16-bit unsigned integer 
    FIDT_LONG        = 4,    //! 32-bit unsigned integer 
    FIDT_RATIONAL    = 5,    //! 64-bit unsigned fraction 
    FIDT_SBYTE        = 6,    //! 8-bit signed integer 
    FIDT_UNDEFINED    = 7,    //! 8-bit untyped data 
    FIDT_SSHORT        = 8,    //! 16-bit signed integer 
    FIDT_SLONG        = 9,    //! 32-bit signed integer 
    FIDT_SRATIONAL    = 10,    //! 64-bit signed fraction 
    FIDT_FLOAT        = 11,    //! 32-bit IEEE floating point 
    FIDT_DOUBLE        = 12,    //! 64-bit IEEE floating point 
    FIDT_IFD        = 13,    //! 32-bit unsigned integer (offset) 
    FIDT_PALETTE    = 14,    //! 32-bit RGBQUAD 
    FIDT_LONG8        = 16,    //! 64-bit unsigned integer 
    FIDT_SLONG8        = 17,    //! 64-bit signed integer
    FIDT_IFD8        = 18    //! 64-bit unsigned integer (offset)
};

enum FREE_IMAGE_TYPE
{
    FIT_UNKNOWN = 0,    //! unknown type
    FIT_BITMAP  = 1,    //! standard image            : 1-, 4-, 8-, 16-, 24-, 32-bit
    FIT_UINT16    = 2,    //! array of unsigned short    : unsigned 16-bit
    FIT_INT16    = 3,    //! array of short            : signed 16-bit
    FIT_UINT32    = 4,    //! array of unsigned long    : unsigned 32-bit
    FIT_INT32    = 5,    //! array of long            : signed 32-bit
    FIT_FLOAT    = 6,    //! array of float            : 32-bit IEEE floating point
    FIT_DOUBLE    = 7,    //! array of double            : 64-bit IEEE floating point
    FIT_COMPLEX    = 8,    //! array of FICOMPLEX        : 2 x 64-bit IEEE floating point
    FIT_RGB16    = 9,    //! 48-bit RGB image            : 3 x 16-bit
    FIT_RGBA16    = 10,    //! 64-bit RGBA image        : 4 x 16-bit
    FIT_RGBF    = 11,    //! 96-bit RGB float image    : 3 x 32-bit IEEE floating point
    FIT_RGBAF    = 12    //! 128-bit RGBA float image    : 4 x 32-bit IEEE floating point
};

#define TIFF_DEFAULT        0

struct FIBITMAP
{
    void *data;
};

struct FITAG
{
    void *data;
};

static void FreeImage_Unload(FIBITMAP *dib)
{
    if (dib)
    {
        dib->data = nullptr;
    }
}

static FIBITMAP *FreeImage_Load(FREE_IMAGE_FORMAT fif, const char *filename, int flags = 0)
{
    return nullptr;
}

static FIBITMAP *FreeImage_Allocate(int width, int height, int bpp, unsigned red_mask = 0, unsigned green_mask = 0, unsigned blue_mask =0)
{
    return nullptr;
}

static bool FreeImage_CloneMetadata(FIBITMAP *dst, FIBITMAP *src)
{
    return false;
}

static unsigned FreeImage_GetMetadataCount(FREE_IMAGE_MDMODEL model, FIBITMAP *dib)
{
    return 0;
}

static bool FreeImage_GetMetadata(FREE_IMAGE_MDMODEL model, FIBITMAP *dib, const char *key, FITAG **tag)
{
    return false;
}

static FREE_IMAGE_MDTYPE FreeImage_GetTagType(FITAG *tag)
{
    return FIDT_NOTYPE;
}

static size_t FreeImage_GetTagCount(FITAG *tag)
{
    return 0;
}

static const void *FreeImage_GetTagValue(FITAG *tag)
{
    return nullptr;
}

static unsigned FreeImage_GetBPP(FIBITMAP *dib)
{
    return 0;
}

static unsigned FreeImage_GetWidth(FIBITMAP *dib)
{
    return 0;
}

static unsigned FreeImage_GetHeight(FIBITMAP *dib)
{
    return 0;
}

static FREE_IMAGE_TYPE FreeImage_GetImageType(FIBITMAP *dib)
{
    return FIT_UNKNOWN;
}

static unsigned char *FreeImage_GetScanLine(FIBITMAP *dib, int scanline)
{
    return nullptr;
}

static bool FreeImage_Save(FREE_IMAGE_FORMAT fif, FIBITMAP *dib, const char *filename, int flags = 0)
{
    return false;
}

static FIBITMAP *FreeImage_AllocateT(FREE_IMAGE_TYPE type, int width, int height, int bpp = 8, unsigned red_mask = 0, unsigned green_mask = 0, unsigned blue_mask = 0)
{
    return nullptr;
}

FIBITMAP *GeoTIFF::mProjectionBitmap = nullptr;

GeoTIFF::GeoTIFF()
{
    dib = nullptr;
    mOx = mOy = mCellsize = 0.;
    mNcol = mNrow = 0;
    mNoDataValue = 0.;
}

GeoTIFF::~GeoTIFF()
{
    if (dib) {
        FreeImage_Unload(dib);
        dib = nullptr;
    }
}

void GeoTIFF::clearProjection()
{
    if (mProjectionBitmap) {
        FreeImage_Unload(mProjectionBitmap);
        mProjectionBitmap = nullptr;
    }
}

int GeoTIFF::loadImage(const QString &fileName)
{
    qDebug() << "Loading TIF file" << fileName;
    dib = FreeImage_Load(FIF_TIFF, fileName.toStdString().c_str());
    if (!mProjectionBitmap) {

        mProjectionBitmap = FreeImage_Allocate(10,10,24);
        FreeImage_CloneMetadata(mProjectionBitmap, dib);
        qDebug() << "GeoTIFF: meta data (incl. projection) for writing TIFs is copied from" << fileName;
    }


    if (FreeImage_GetMetadataCount(FIMD_GEOTIFF, dib) >0) {
        FITAG *tagMake = nullptr;
        FreeImage_GetMetadata(FIMD_GEOTIFF, dib, "GeoTiePoints", &tagMake);
        if (!tagMake)
            throw IException(QString("GeoTIF '%1' does not contain required tags (tie points).").arg(fileName));

        if (FreeImage_GetTagType(tagMake) != FIDT_DOUBLE)
            throw IException(QString("GeoTIF '%1' invalid datatype (tie points).").arg(fileName));

        size_t tag_count = FreeImage_GetTagCount(tagMake);
        const double *values = static_cast<const double*>(FreeImage_GetTagValue(tagMake));
        if (!values)
            throw IException(QString("GeoTIF '%1' does not contain required tags (tie points).").arg(fileName));

        if (logLevelDebug())
            for (size_t i=0; i<tag_count;++i) {
                qDebug() << "TIFF: TiePoints value #" << i << ":" << values[i];
        }
        mOx = values[0];
        mOy = values[1];
        if (mOx == 0. && mOy == 0.) {
            mOx = values[3];
            mOy = values[4];
        }


        FreeImage_GetMetadata(FIMD_GEOTIFF, dib, "GeoPixelScale", &tagMake);
        if (!tagMake)
            return -1;
        if (FreeImage_GetTagType(tagMake) != FIDT_DOUBLE)
            throw IException(QString("GeoTIF '%1' does not contain required tags (pixel scale).").arg(fileName));

        tag_count = FreeImage_GetTagCount(tagMake);
        values = static_cast<const double*>(FreeImage_GetTagValue(tagMake));
        if (!values)
            throw IException(QString("GeoTIF '%1' does not contain required tags (pixel scale).").arg(fileName));

        mCellsize = values[0];
        if (fabs(mCellsize-values[1])>0.001) {
            throw IException(QString("GeoTIF '%1' pixel scale in x and y do not match (x: %2, y: %3).").arg(fileName).arg(mCellsize).arg(values[1]));

        }
        mNcol = FreeImage_GetWidth(dib);
        mNrow = FreeImage_GetHeight(dib);


        switch (FreeImage_GetImageType(dib)) {
        case FIT_INT16: mNoDataValue = std::numeric_limits<short int>::lowest(); mDType = DTSINT16; break;
        case FIT_INT32: mNoDataValue = std::numeric_limits<int>::lowest(); mDType = DTSINT32; break;
        case FIT_FLOAT: mNoDataValue = std::numeric_limits<float>::lowest(); mDType = DTFLOAT; break;
        case FIT_DOUBLE: mNoDataValue = std::numeric_limits<double>::lowest(); mDType = DTDOUBLE; break;
        default:
            throw IException(QString("GeoTiff: The TIF file '%1' has an invalid datatype. \n" \
                                     "Currently valid are: int16 (INT2S), int32 (INT4S), float (FLT4S), double (FLT8S).").arg(fileName));
        }

        QString info_str = QString("Loaded TIF '%1', x/y: %2/%3, cellsize: %4, width: %5, height: %6, datatype %7, %8 bits per cell")
                               .arg(fileName).arg(mOx).arg(mOy)
                               .arg(mCellsize).arg(mNcol).arg(mNrow)
                               .arg(FreeImage_GetImageType(dib))
                               .arg(FreeImage_GetBPP(dib));
        qInfo() << info_str;
        return 0;

    } else {
        throw IException(QString("GeoTIF '%1' does not contain meta data.").arg(fileName));
    }

}

void GeoTIFF::copyToIntGrid(Grid<int> *grid)
{
    if (!dib)
        throw std::logic_error("Copy TIF to grid: tif not loaded!");
    auto dtype = FreeImage_GetImageType(dib);
    if (dtype != FIT_INT32 && dtype != FIT_UINT16 && dtype != FIT_INT16) {
        throw IException(QString("Copy TIF to grid: wrong data type, INT32, UINT16 or INT16 expected, got type {}").arg(FreeImage_GetImageType(dib)));
    }
    // the null value of grids (at least for INT) is weird; it is not the smallest possible value (−2,147,483,648), but instead −2,147,483,647.
    throw IException("Internal: fix handling of null values!");
    //int null_value = grid->nullValue();
    //int value_null = std::numeric_limits<LONG>::min()+2;
    int null_value = -1;
    int value_null = -1;

    if (dtype == FIT_INT32) {
        for(unsigned int y = 0; y < FreeImage_GetHeight(dib); y++) {
            int32_t *bits = (int32_t*)FreeImage_GetScanLine(dib, y);
            for(unsigned int x = 0; x < FreeImage_GetWidth(dib); x++) {
                grid->valueAtIndex(x,y) = bits[x] < value_null ? null_value : bits[x];
            }
        }
    }

    if (dtype == FIT_UINT16) {
        value_null = std::numeric_limits<uint16_t>::max();
        for(unsigned int y = 0; y < FreeImage_GetHeight(dib); y++) {
            uint16_t *bits = (uint16_t*)FreeImage_GetScanLine(dib, y);
            for(unsigned int x = 0; x < FreeImage_GetWidth(dib); x++) {
                grid->valueAtIndex(x,y) = bits[x] == value_null ? null_value : bits[x];
            }
        }
    }

    if (dtype == FIT_INT16) {
        value_null = std::numeric_limits<int16_t>::min();
        for(uint y = 0; y < FreeImage_GetHeight(dib); y++) {
            int16_t *bits = (int16_t*)FreeImage_GetScanLine(dib, y);
            for(uint x = 0; x < FreeImage_GetWidth(dib); x++) {
                grid->valueAtIndex(x,y) = bits[x] == value_null ? null_value : bits[x];
            }
        }
    }


}

void GeoTIFF::copyToDoubleGrid(Grid<double> *grid)
{
    if (!dib)
        throw IException("Copy TIF to grid: tif not loaded!");

    if (FreeImage_GetImageType(dib) != FIT_DOUBLE
        && FreeImage_GetImageType(dib) != FIT_FLOAT
        && FreeImage_GetImageType(dib) != FIT_INT16
        && FreeImage_GetImageType(dib) != FIT_INT32) {
        throw IException("Copy TIF to grid: wrong data type, double, float, int16, int32 expected!");
    }
    switch (FreeImage_GetImageType(dib)) {
    case FIT_DOUBLE: {

        for(uint y = 0; y < FreeImage_GetHeight(dib); y++) {
            double *bits = (double*)FreeImage_GetScanLine(dib, y);
            for(uint x = 0; x < FreeImage_GetWidth(dib); x++) {
                grid->valueAtIndex(x,y) = bits[x] == noDataDouble() ? noDataDouble() : bits[x];
            }
        }
        return;
    }
    case FIT_FLOAT: {
        for(uint y = 0; y < FreeImage_GetHeight(dib); y++) {
            float *bits = (float*)FreeImage_GetScanLine(dib, y);
            for(uint x = 0; x < FreeImage_GetWidth(dib); x++) {
                grid->valueAtIndex(x,y) = bits[x] == noDataFloat() ? noDataDouble() : bits[x];
            }
        }
        return;
    }
    case FIT_INT16:  {
        for(uint y = 0; y < FreeImage_GetHeight(dib); y++) {
            short int *bits = (short int *)FreeImage_GetScanLine(dib, y);
            for(uint x = 0; x < FreeImage_GetWidth(dib); x++) {
                grid->valueAtIndex(x,y) = bits[x] == noDataShort() ? noDataDouble() : bits[x];
            }
        }
        return;
    }
    case FIT_INT32:  {
        for(uint y = 0; y < FreeImage_GetHeight(dib); y++) {
            int *bits = (int *)FreeImage_GetScanLine(dib, y);
            for(uint x = 0; x < FreeImage_GetWidth(dib); x++) {
                grid->valueAtIndex(x,y) = bits[x] == noDataInt() ? noDataDouble() : bits[x];
            }
        }
        return;
    }

    default:
        throw IException("Geotiff::copyToDoubleGrid: invalid data type.");
    }


}

void GeoTIFF::copyToFloatGrid(Grid<float> *grid)
{
    if (!dib)
        throw IException("Copy TIF to grid: tif not loaded!");

    if (FreeImage_GetImageType(dib) != FIT_DOUBLE
        && FreeImage_GetImageType(dib) != FIT_FLOAT
        && FreeImage_GetImageType(dib) != FIT_INT16
        && FreeImage_GetImageType(dib) != FIT_INT32) {
        throw IException("Copy TIF to grid: wrong data type, double, float, int16, int32 expected!");
    }

    switch (FreeImage_GetImageType(dib)) {
    case FIT_DOUBLE: {

        for(uint y = 0; y < FreeImage_GetHeight(dib); y++) {
            double *bits = (double*)FreeImage_GetScanLine(dib, y);
            for(uint x = 0; x < FreeImage_GetWidth(dib); x++) {
                grid->valueAtIndex(x,y) = bits[x];
            }
        }
        return;
    }
    case FIT_FLOAT: {
        for(uint y = 0; y < FreeImage_GetHeight(dib); y++) {
            float *bits = (float*)FreeImage_GetScanLine(dib, y);
            for(uint x = 0; x < FreeImage_GetWidth(dib); x++) {
                grid->valueAtIndex(x,y) = bits[x];
            }
        }
        return;
    }
    case FIT_INT16:  {
        for(uint y = 0; y < FreeImage_GetHeight(dib); y++) {
            short int *bits = (short int *)FreeImage_GetScanLine(dib, y);
            for(uint x = 0; x < FreeImage_GetWidth(dib); x++) {
                grid->valueAtIndex(x,y) = bits[x];
            }
        }
        return;
    }
    case FIT_INT32:  {
        for(uint y = 0; y < FreeImage_GetHeight(dib); y++) {
            int *bits = (int *)FreeImage_GetScanLine(dib, y);
            for(uint x = 0; x < FreeImage_GetWidth(dib); x++) {
                grid->valueAtIndex(x,y) = bits[x];
            }
        }
        return;
    }

    default:
        throw IException("Geotiff::copyToFloatGrid: invalid data type.");
    }


}

bool GeoTIFF::saveToFile(const QString &fileName)
{
    if (!dib)
        return false;
    bool success = FreeImage_Save(FIF_TIFF, dib, fileName.toStdString().c_str(), TIFF_DEFAULT);
    FreeImage_Unload(dib);
    dib = nullptr;
    return success;

}

void GeoTIFF::initialize(uint width, uint height, TIFDatatype dtype)
{
    if (!mProjectionBitmap)
        throw IException("GeoTif: init write: no projection information is available. You need to load at least one TIF including projection info before writing a TIF.");

    mDType = dtype;

    switch (mDType) {
    case DTSINT16: mNoDataValue = std::numeric_limits<short int>::lowest(); break;
    case DTSINT32: mNoDataValue = std::numeric_limits<int>::lowest(); break;
    case DTFLOAT: mNoDataValue = std::numeric_limits<float>::lowest(); break;
    case DTDOUBLE: mNoDataValue = std::numeric_limits<double>::lowest(); break;
    default:
        throw IException("GeoTif: init write: invalid data type!");
    }

    FREE_IMAGE_TYPE fit = FREE_IMAGE_TYPE(dtype);
    dib = FreeImage_AllocateT(fit , width, height );
    FreeImage_CloneMetadata(dib, mProjectionBitmap);

    //if (!FreeImage_SetMetadataKeyValue(FIMD_GEOTIFF, dib, "GDAL_NODATA", "-32768"))
    //   throw std::logic_error("GeoTif: set metadata (NODATA) to xyz not successful!");
}

void GeoTIFF::setValue(uint ix, uint iy, double value)
{
    if (!dib)
        return;
    if (ix > FreeImage_GetWidth(dib) || iy > FreeImage_GetHeight(dib))
        return;
    switch (mDType) {
    case DTFLOAT: {
        float flt_value = static_cast<float>(value);
        ((float*)FreeImage_GetScanLine(dib, iy))[ix] = flt_value;
        return;
    }
    case DTDOUBLE:
        ((double*)FreeImage_GetScanLine(dib, iy))[ix] = value;
        return;
    case DTSINT16: {
        short int short_value = static_cast<short int>(value);
        ((short int*)FreeImage_GetScanLine(dib, iy))[ix] = short_value;
        return;
    }
    case DTSINT32: {
        int int_value = static_cast<int>(value);
        ((int*)FreeImage_GetScanLine(dib, iy))[ix] = int_value;
        return;
    }
    default:
        throw IException(QString("GeoTif:setValue(): invalid type of TIF: %1").arg(mDType));
    }


}

