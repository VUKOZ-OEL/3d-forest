#ifndef DEVSTAGEOUT_H
#define DEVSTAGEOUT_H

#include <QObject>
#include <QJSValue>

#include "expression.h"
#include "output.h"
#include "grid.h"

class Tree; // forward
class ResourceUnit; // forward

class DevStageCell; // forward
class DevStageOut : public Output
{
public:
    DevStageOut();
    virtual void exec();
    virtual void setup();
private:
    void setupCategories();
    /// fill the stockable grid with values 0..1 (fraction of the cell that is considered stockable)
    void setupStockableArea();

    void calculateDevStages();

    // default model: classifcation of Zenner et al
    int8_t runZennerModel();
    Expression mFilter;

    int mCellSize; ///< cell size (meters)
    Grid<int8_t> mGrid; ///< holds the current development stage as factor
    Grid<uint16_t> mStockableArea; ///< stockable area per cell (m2)
    QScopedPointer<DevStageCell> mCell; ///< the "cell" also used for JavaScript

    double mTotalStockableArea; ///< total stockable area of all cells
    QStringList mStages; ///< list of the available development stages (names)
    QStringList mColor; ///< colors for visualization of all stagse
    QJSValue mEvalObj;
    QJSValue mEvalFunc; ///< the JS function to evaluate each cell
    QJSValue mPostEvalFunc; ///< the JS function called after finsihing the map
    QJSValueList mEvalParam; ///< parameter (=cell) to the eval function

    bool mRefreshCPA;
    friend class DevStageCell;
};


class DevStageCell : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int x READ x) ///< x position of the current cell (cell index)
    Q_PROPERTY(int y READ y) ///< y position of the current cell (cell index)
    Q_PROPERTY(double DBHMax READ DBHMax)
    Q_PROPERTY(double DBHMin READ DBHMin)
    Q_PROPERTY(double DBHMean READ DBHMean)
    Q_PROPERTY(double DBHMedian READ DBHMedian)
    Q_PROPERTY(double HMax READ HMax)
    Q_PROPERTY(double HMean READ HMean)
    Q_PROPERTY(double HMedian READ HMedian)
    Q_PROPERTY(double NQD READ NQD)
    Q_PROPERTY(double deadwoodShare READ deadwoodShare)
    Q_PROPERTY(double CPA READ CPA) ///< crown projection area
    Q_PROPERTY(double N_ha READ N_ha) ///< number of trees (>4m) per ha
    Q_PROPERTY(double Pct_PMugo READ Pct_PMugo) ///< pct of the area covered by P. mugo

public:
    explicit DevStageCell(QObject *parent = nullptr);
    ~DevStageCell() {}
    void setObj(DevStageOut *dsc) { mOut = dsc; }
    int loadTrees(int8_t *grid_ptr);
    void calculateStats();

    // Location of the pixel that is currently processed (by index, given by the grid)
    int x() const {return mLocation.x(); }
    int y() const { return mLocation.y(); }

    double DBHMax() const { return sv.dbh_max; }
    double DBHMin() const { return sv.dbh_min; }
    double DBHMean() const { return sv.dbh_mean; }
    double DBHMedian() const { return sv.dbh_median; }
    double HMax() const { return sv.h_max; }
    double HMean() const { return sv.h_mean; }
    double HMedian() const { return sv.h_median; }
    double N_ha() const { return sv.n_trees; }
    /// DBH variation expressed as 100*interquartile distance/median
    double NQD() const { return sv.dbh_nqd; }

    // other variables
    double deadwoodShare(); ///< share of standing/lying deadwood relative to living biomass
    double CPA(); ///< crown projection area
    double Pct_PMugo(); ///< % area covered by P. mugo


public slots:
    QJSValue grid(); ///< return a copy of the underlying grid
    Grid<double> *paintGrid(QString what, QStringList &names, QStringList &colors); ///< function called from iLand visualization
signals:

private:
    DevStageOut *mOut;
    QVector<const Tree*> mTrees;
    QPoint mLocation; // current position (grid indices)
    const ResourceUnit *mRU; // current resource unit
    // individual variables that are available
    struct SStandVars {
        double dbh_max;
        double dbh_mean;
        double dbh_min;
        double dbh_median;
        double dbh_nqd;
        double h_max;
        double h_mean;
        double h_median;
        double n_trees;
    } sv;

    FloatGrid mCPA_grid;

    // grid for painting
    Grid<double> mPaintGrid;


};

#endif // DEVSTAGEOUT_H
