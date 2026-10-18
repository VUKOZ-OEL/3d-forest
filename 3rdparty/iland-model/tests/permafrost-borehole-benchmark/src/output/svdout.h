#ifndef SVDOUT_H
#define SVDOUT_H
#include "output.h"
#include "expression.h"
// #include <bitset>
// Qt5.12 bug:
// https://bugreports.qt.io/browse/QTBUG-72073
// created an updated version of "bitset" including a lil fix
#ifdef Q_OS_WIN
#include "../3rdparty/bitset.h"
#else
#include <bitset>
#endif

class ResourceUnit; // forward

/// An auxiliary output which saves
/// GPP per resource unit and year
/// (for training GPP DNNs)
class SVDGPPOut: public Output
{
public:
    SVDGPPOut();
    virtual void exec();
    virtual void setup();
private:
    QStringList mSpeciesList;
    int mSpeciesIndex[10];
};

/// SVDStateOut saves state changes for SVD.
/// The output includes also info about the neighborhood
/// of each resource unit (i.e. species composition)
class SVDStateOut: public Output
{
public:
    SVDStateOut();
    virtual void exec();
    virtual void setup();
private:

};

/// SVDUniqueStateOut saves the list of unique states.
/// This should be done at the end of the simulation
class SVDUniqueStateOut: public Output
{
public:
    SVDUniqueStateOut();
    virtual void exec();
    virtual void setup();
private:
    Expression mCondition;

};




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
    enum Indicators {EshannonIndex, EabovegroundCarbon, EtotalCarbon, Evolume, EcrownCover};
    std::bitset<32> mIndicators;
    // indicator calculators
    double calcShannonIndex(const ResourceUnit *ru);
    double calcCrownCover(const ResourceUnit *ru);
    double calcTotalCarbon(const ResourceUnit *ru);
};


#endif // SVDOUT_H
