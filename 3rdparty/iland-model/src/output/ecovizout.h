#ifndef ECOVIZOUT_H
#define ECOVIZOUT_H

#include "output.h"
#include "expression.h"

/** EcoVizOut is as special output to store data for use in a visualization tool */

class EcoVizOut: public Output
{
public:
    EcoVizOut();
    virtual void exec();
    virtual void setup();
private:
    // pdb creation
    bool writePDBFile(QString fileName, int n_trees, int n_cohorts, int year);
    bool writePDBBinaryFile(QString fileName, int n_trees, int n_cohorts, int year);
    Expression mCondition;
    QString mFilePattern;
    bool mBinaryMode {false};

};

#endif // ECOVIZOUT_H
