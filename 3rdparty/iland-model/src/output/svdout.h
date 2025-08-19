#ifndef SVDOUT_H
#define SVDOUT_H
#include "output.h"
#include "expression.h"


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




#endif // SVDOUT_H
