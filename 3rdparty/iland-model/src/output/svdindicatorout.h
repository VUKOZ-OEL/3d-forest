#ifndef SVDINDICATOROUT_H
#define SVDINDICATOROUT_H

#include "output.h"
#include "grid.h"
#include <bitset>
// Qt5.12 bug with MSVC compiler
// https://bugreports.qt.io/browse/QTBUG-72073
// created an updated version of "bitset" including a lil fix
//#ifdef Q_OS_WIN // <- not working for GCC on Windows
#ifdef Q_CC_MSVC
//#include "../3rdparty/bitset.h"
#else
#include <bitset>
#endif

class ResourceUnit; // forward

/// SVDIndicatorOut saves (compressed) indicator data for SVD.
/// Data is collected for each resource unit and for every year
class SVDIndicatorOut: public Output
{
public:
    SVDIndicatorOut();
    virtual void exec();
    virtual void setup();
private:
    // list of active indicators
    enum Indicators {EshannonIndex, EabovegroundCarbon, EtotalCarbon, Evolume, EcrownCover,
                      ELAI, EbasalArea, EstemDensity, EsaplingDensity, EIBP};
    std::bitset<32> mIndicators;
    QVector<QPair<QString, int> > mSpecies; // store species IDs and corresponding index
    int mNDisturbanceHistory;

    Grid<float> mCrownCoverGrid;

    // indicator calculators
    double calcShannonIndex(const ResourceUnit *ru);
    double calcCrownCover(const ResourceUnit *ru);
    double calcTotalCarbon(const ResourceUnit *ru);
    double calcIBP(const ResourceUnit *ru);

    void addSpeciesProportions(const ResourceUnit *ru);
    void addDisturbanceHistory(const ResourceUnit *ru);

};


#endif // SVDINDICATOROUT_H
