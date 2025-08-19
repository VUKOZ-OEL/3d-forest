#include "devstageout.h"

#include <QJSEngine>

#include "model.h"
#include "resourceunit.h"
#include "scriptglobal.h"
#include "statdata.h"
#include "scriptgrid.h"
#include "modelcontroller.h"
#include "spatialanalysis.h"
#include "soil.h"
#include "speciesset.h"
#include "species.h"
#include "debugtimer.h"

DevStageOut::DevStageOut()
{
    setName("Stand Development Stage", "devstage");
    setDescription(" iLand includes a special output for assessing the development stage of forested cells on the landscape. " \
                   "The rule set to derive specific development stages is flexible (using a Javascript interface) in order to accomodate different ecosystems.\n  " \
                   " see [development+stages] for the full documentation.\n " \
                   " todo.\n");

    columns() << OutputColumn::year()
            << OutputColumn("stage", "stand development stage (name, not numeric ID)", OutString)
            << OutputColumn("ncells", "Number of cells on the landscape within this stage", OutInteger)
            << OutputColumn("percent_area", "percent of the landscape covered with this stage (0..100). Not fully stockable cells are accounted for correctly.", OutDouble);

}

void DevStageOut::exec()
{
    if (!mFilter.isEmpty())
        if (!mFilter.calculateBool(GlobalSettings::instance()->currentYear()))
            return;

    DebugTimer t("DevelopmentStageOutput");
    // run spatial analysis of development stages (this updates the internal grid)
    calculateDevStages();

    if (mPostEvalFunc.isCallable()) {
        QJSValue js_result = mPostEvalFunc.callWithInstance(mEvalObj, mEvalParam);
        if (js_result.isError())
            throw IException(js_result.toString());
    }


    // summarise over categories
    QVector<int> ncells;
    ncells.resize(mStages.size());
    QVector<float> area;
    area.resize(mStages.size());

    int invalid=0;
    for (int i=0;i<mGrid.count();++i) {
        if (mGrid[i]<0)
            continue;
        if (!mStages[ mGrid[i] ].isEmpty()) {
            ncells[ mGrid[i] ]++;
            area[ mGrid[i] ] += mStockableArea[i];
        } else{
            ++invalid;
            qDebug() << "DevStageOut: Invalid class: " << mGrid[i];
        }
    }
    if (invalid>0)
        qWarning() << "DevStageOut: there were" << invalid << "invalid cell values!!!";

    // write output
    for (int i=0;i<mStages.size();++i) {
        if (!mStages[i].isEmpty()) {
            *this << currentYear() << mStages[i] << ncells[i] << area[i]/mTotalStockableArea*100.;
            writeRow();
        }
    }


    // write raster to file


}

void DevStageOut::setup()
{
    bool enabled = settings().valueBool(".enabled", false);
    if (!enabled)
        return;

    if (!settings().isValid())
        throw IException("DevStageOut::setup(): no parameter section in init file!");
    QString filter = settings().value(".filter","");
    mFilter.setExpression(filter);

    // set up grid
    mCellSize = settings().valueInt(".cellsize", 50);
    if (mCellSize != 10 && mCellSize != 20 && mCellSize != 50 && mCellSize!= 100)
        throw IException("DevStageOut: invalid 'cellsize'! Allowed values are 10, 20, 50, 100.");

    mGrid.clear();
    mGrid.setup(GlobalSettings::instance()->model()->RUgrid().metricRect(),
                mCellSize);
    mGrid.initialize(0); // value 0: non forest

    mStockableArea.clear();
    mStockableArea.setup(GlobalSettings::instance()->model()->RUgrid().metricRect(), mCellSize);
    mStockableArea.initialize(0);
    setupStockableArea();

    // set up the cell object and link to this output
    DevStageCell *dsc = mCell.take(); // test: avoid crash?
    if (dsc)
        GlobalSettings::instance()->controller()->removePaintLayers(dsc);
    mCell.reset(new DevStageCell());
    mCell->setObj(this);

    // set up handling code
    QString handling_obj_name = settings().value(".handler");
    QJSValue handler = GlobalSettings::instance()->scriptEngine()->globalObject().property(handling_obj_name);
    if (handler.isUndefined())
        throw IException("DevStageOut: invalid Javascript object specified as 'handler': " + handling_obj_name);

    mEvalObj = handler;
    QVariant typesv = ScriptGlobal::valueFromJs(handler, "types", "", "definition of the types object").toVariant();
    QVariantMap types = qvariant_cast<QVariantMap>(typesv);

    QStringList str_keys = types.keys();
    QVector<int> keys;
    bool ok;
    for (int j=0; j<str_keys.size();++j) {
        keys.push_back( str_keys[j].toInt(&ok) );
        if (!ok)
            throw IException("setup of DevStageOut: the keys in the 'types' structure need to be numeric! wrong:" + str_keys[j]);
    }
    auto max_val = std::max_element(keys.begin(), keys.end());
    if (max_val == keys.end())
        throw IException("DevStageOutput: the 'types' object does not contain valid numeric keys > 0!");

    mStages.clear(); mColor.clear();
    for (int i=0;i<=*max_val;++i) {
        mStages.push_back(QString());
        mColor.push_back(QString());
    }

    QMapIterator<QString, QVariant> i(types);
    qDebug() << "setup of DevStageOutput:";
      while (i.hasNext()) {
          i.next();
          int key = i.key().toInt();
          QVariantMap elem = i.value().toMap();
          qDebug() << "key:" << i.key() << "name:" << elem["name"] << "color:" << elem["color"];
          mStages[key] = elem["name"].toString();
          mColor[key] = elem["color"].toString();
      }
      qDebug() << "setup of DevStageOutput finished with" << keys.size() << " stages. stages:" << mStages << ", colors: " << mColor;

      // set up classification function
      mEvalFunc = ScriptGlobal::valueFromJs(handler, "run");
      if (mEvalFunc.isCallable())
          qDebug()<< "setup of DevStageOutput: Custom code for development stage classification available";
      else
          qDebug()<< "setup of DevStageOutput: No custom code availabe, fallign back to default development stage classification (Zenner et al)";

      mPostEvalFunc = ScriptGlobal::valueFromJs(handler, "onFinished");
      QJSValue js_scriptcell = GlobalSettings::instance()->scriptEngine()->newQObject(&(*mCell));
      mEvalParam = QJSValueList() << js_scriptcell;


      // set up link to iLand (user interface) (only relevant for the GUI version of iLand)
      QStringList varlist = {"DevStage - Stage"};
      QVector<GridViewType> paint_types = {GridViewCustom};
      GlobalSettings::instance()->controller()->addPaintLayers(mCell.get(), varlist, paint_types);


}

void DevStageOut::setupStockableArea()
{
    // area per height grid cell (100m2)
    uint16_t cell_area = static_cast<uint16_t>(cHeightPixelArea);

    // aggregate stocking area for each cell
    int total=0;
    HeightGrid *hg = GlobalSettings::instance()->model()->heightGrid();
    for (HeightGridValue *hgv=hg->begin(); hgv!=hg->end(); ++hgv) {
        if (hgv->isValid()) {
            total+= cell_area;
            mStockableArea[ hg->cellCenterPoint(hgv) ] += cell_area;
        }
    }

    for (int i=0;i<mGrid.count();++i) {
        if (mStockableArea[i] == 0.f)
            mGrid[i] = -1; // non stockable areas
    }
    mTotalStockableArea = total;

    qDebug() << "setup DevStageOut: total stockable area of the landscape:" << total / 10000. << "ha";
}

void DevStageOut::calculateDevStages()
{
    int8_t cell_result=0;
    QJSValue js_result;

    mRefreshCPA = true; // force refresh of crown projection area

    for (int8_t *cell = mGrid.begin(); cell!=mGrid.end(); ++cell) {
        if (*cell < 0) // skip unstockable areas
            continue;

        // calculate values
        mCell->loadTrees(cell);
        mCell->calculateStats();

        // run the decision
        if (mEvalFunc.isCallable()) {
            // run the javascript function
            js_result = mEvalFunc.callWithInstance(mEvalObj, mEvalParam);
            if (!js_result.isNumber())
                throw IException("DevStageOut: custom Javascript function must return a numeric value! got: " + js_result.toString());
            cell_result = static_cast<int8_t>(js_result.toInt());
        } else {
            // no javascript function available, use the built-in default function (Zenner classification)
            cell_result = runZennerModel();
        }

        // write back result
        *cell = cell_result;
    }
}

int8_t DevStageOut::runZennerModel()
{
    // hard-coded version of the adapted Zenner approach (see "Entscheidungsbaum_Zenner_modified3.pdf")
    return static_cast<int8_t>(irandom(0, 3));
}

/********************************
 * DevStageCell
 * represents a single cell of the devstage grid
 * */

DevStageCell::DevStageCell(QObject *parent): QObject(parent)
{

}

int DevStageCell::loadTrees(int8_t *grid_ptr)
{
    QPoint location = mOut->mGrid.indexOf(grid_ptr);
    mLocation = location;
    QPointF locF = mOut->mGrid.cellCenterPoint(location);
    const ResourceUnit *ru = GlobalSettings::instance()->model()->RUgrid().constValueAt(locF);
    mRU = ru;

    mTrees.clear();

    if (!ru)
        return 0;

    QRectF cell_rect = mOut->mGrid.cellRect(location);

    // load trees that fall within the cell rectangle into the internal tree list
    const QVector<Tree> trees = ru->constTrees();
    for (QVector<Tree>::const_iterator i = trees.constBegin(); i!=trees.constEnd(); ++i)
        if (cell_rect.contains(i->position()))
            mTrees.append(&(*i));

    return mTrees.size();
}

void DevStageCell::calculateStats()
{

    QVector<double> data;
    QVector<double> heights;
    for (QVector<const Tree*>::const_iterator it=mTrees.constBegin(); it!= mTrees.constEnd(); ++it) {
        data.push_back((*it)->dbh());
    }
    for (int i=0;i<mTrees.size();++i) {
        data.push_back(mTrees[i]->dbh());
    }

    StatData stat(data);
    sv.dbh_max = stat.max();
    sv.dbh_min = stat.min();
    sv.dbh_mean = stat.mean();
    sv.dbh_median = stat.median();
    sv.dbh_nqd =  sv.dbh_median>0.? 100. * (stat.percentile75()-stat.percentile25()) / sv.dbh_median : 0.;

    //
    int i=0;
    for (QVector<const Tree*>::const_iterator it=mTrees.constBegin(); it!= mTrees.constEnd(); ++it, ++i) {
        data[i]= (*it)->height();
    }
    stat.setData(data);
    sv.h_max = stat.max();
    sv.h_mean = stat.mean();
    sv.h_median = stat.median();

    double factor = mOut->mCellSize * mOut->mCellSize / 10000.; //
    sv.n_trees = data.size() / factor;

}

double DevStageCell::deadwoodShare()
{
    if (!mRU)
        return 0.;

    double area_factor = mRU->stockableArea() / cRUArea; // conversion factor from real area to per ha values
    if (area_factor == 0.)
        return 0.;

    // aboveground living biomass (+ regeneration), kg / ha
    double living_biomass = mRU->statistics().cStem() + mRU->statistics().cBranch() + mRU->statistics().cFoliage() + mRU->statistics().cRegeneration();
    // snags pools need scaling with stockable area
    double snags = (mRU->snag()->totalSWD().C + mRU->snag()->totalOtherWood().C * mRU->snag()->otherWoodAbovegroundFraction()) / area_factor;
    // soil poils need conversion to kg / ha
    double downed = mRU->soil()->youngRefractory().C*1000. * mRU->soil()->youngRefractoryAbovegroundFraction();

    // deadwood share = deadwood / total living biomass
    if (living_biomass == 0.) {
        if (snags+downed > 0.)
            return 1.; // say, 100%
        else
            return 0.;
    }
    double share = (snags + downed) / living_biomass;
    return share;
}

double DevStageCell::CPA()
{
    if (mOut->mRefreshCPA) {
        // do a crown projection for all trees on the landscape
        // use a 2m grid for this purpose
        if (mCPA_grid.isEmpty())
            mCPA_grid.setup(*GlobalSettings::instance()->model()->grid());

        SpatialAnalysis::runCrownProjection2m(&mCPA_grid);
        qDebug() << "crown projection grid: sum: " << mCPA_grid.sum();
        mOut->mRefreshCPA = false;
    }

    // retrieve from the CPA grid the CPA covered by the cell
    Model *model = GlobalSettings::instance()->model();

    GridRunner<float> runner(mCPA_grid, mOut->mGrid.cellRect(mLocation));
    int cc_sum = 0;
    while (float *gv = runner.next()) {
        if (model->heightGridValue(runner.currentIndex().x(),
                                   runner.currentIndex().y()).isValid())
            if (*gv >= 0.5f) // 0.5: half of a 2m cell is covered by a tree crown; is a bit pragmatic but seems reasonable (and works)
                cc_sum++;
    }
    if (mOut->mStockableArea[mLocation]>0) {
        double value = cPxSize*cPxSize*cc_sum / static_cast<double>(mOut->mStockableArea[mLocation]);
        return value;
    }

    return 0.;
}

double DevStageCell::Pct_PMugo()
{
    // count the number of cells in the regeneration layer where P. mugo is present.

    // check if the species is available
    Species *p_mugo_species_index = GlobalSettings::instance()->model()->speciesSet()->species("pimu");
    if (!p_mugo_species_index)
        throw IException("Development stages: Pinus mugo ('pimu') expected but not available!");

    // the rectangle of the current cell:
    QRectF cell_rect = mOut->mGrid.cellRect(mLocation);

    GridRunner<float> runner(GlobalSettings::instance()->model()->grid(), cell_rect);
    int n_not_stockable=0;
    int n_mugo=0, n_nomugo=0;
    while (runner.next()) {
        SaplingCell *sc=GlobalSettings::instance()->model()->saplings()->cell(runner.currentIndex(), true);
        if (sc) {
            SaplingTree *t = sc->saplingOfSpecies(p_mugo_species_index->index());
            // count all pixels with P. mugo .... count a pixel if P. mugo >1.3m is present ( && t->height > 1.3f )
            if (t)
                n_mugo++;
            else
                n_nomugo++; // no Pimu or Pimu not tall enough

        } else {
            n_not_stockable++; // cell not stockable / outside of project area
        }

    }
    if (n_mugo == 0)
        return 0.;

    // perctange mugo relativ to *stockable* area
    return 100. * n_mugo / (n_mugo + n_nomugo);
}

QJSValue DevStageCell::grid()
{

    // create a grid with the same size and copy the data (converted to double)
    Grid<double> *grid = mOut->mGrid.toDouble();

    // create a Javascript Wrapper around the grid and return
    QJSValue g = ScriptGrid::createGrid(grid, "devstage");
    return g;

}

Grid<double> *DevStageCell::paintGrid(QString what, QStringList &names, QStringList &colors)
{
    Q_UNUSED(what)

    if (mPaintGrid.isEmpty()) {
        // setup the grid
        mPaintGrid.setup(mOut->mGrid.metricRect(), mOut->mGrid.cellsize());
    }
    // copy data from the current internal grid
    double *p = mPaintGrid.begin();
    for (int8_t *iv = mOut->mGrid.begin(); iv!=mOut->mGrid.end(); ++iv, ++p)
        *p = *iv;

    names = mOut->mStages;
    colors = mOut->mColor;

    return &mPaintGrid;

}


