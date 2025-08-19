/********************************************************************************************
**    iLand - an individual based forest landscape and disturbance model
**    https://iland-model.org
**    Copyright (C) 2009-  Werner Rammer, Rupert Seidl
**
**    This program is free software: you can redistribute it and/or modify
**    it under the terms of the GNU General Public License as published by
**    the Free Software Foundation, either version 3 of the License, or
**    (at your option) any later version.
**
**    This program is distributed in the hope that it will be useful,
**    but WITHOUT ANY WARRANTY; without even the implied warranty of
**    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**    GNU General Public License for more details.
**
**    You should have received a copy of the GNU General Public License
**    along with this program.  If not, see <http://www.gnu.org/licenses/>.
********************************************************************************************/
#include "customaggout.h"

#include "debugtimer.h"
#include "statdata.h"
#include "model.h"
#include "resourceunit.h"
#include "species.h"
#include "expressionwrapper.h"
#include "mapgrid.h"

CustomAggOut::CustomAggOut()
{
    setName("custom aggregation of saplings, trees, RUs on user defined intervals", "customagg");

    setDescription("See https://iland-model.org/dynamic+outputs for details. ");
    columns() << OutputColumn::year() << OutputColumn::ru()  << OutputColumn::id() << OutputColumn::species();
    // other colums are added during setup...
}

CustomAggOut::~CustomAggOut()
{
    qDeleteAll(mLevels);
}

void CustomAggOut::exec()
{
    // run all enabled levels
    foreach(Output *out, mLevels) {
        if (out->isEnabled())
            out->exec();
    }
}

void CustomAggOut::setup()
{

    qDeleteAll(mLevels);
    mLevels.clear();
    XmlHelper &xml = const_cast<XmlHelper&>(GlobalSettings::instance()->settings());

    for (int i=0;i<10;++i) {
        QString outname = QString("output.customagg.output%1").arg(i);
        if (settings().hasNode(outname)) {
            QString tab_name = settings().value(QString("%1.tablename").arg(outname));
            bool enabled = settings().valueBool(QString("%1.enabled").arg(outname));
            qDebug() << "CustomAgg output: found output tablename" << tab_name << "; output enabled=" << enabled;
            CustomAggOutLevel *out = new CustomAggOutLevel();
            mLevels.push_back(out);
            // set up individual level
            xml.setCurrentNode(outname); // -> output.custumagg.outputX

            out->setup();
        }
    }
}

void CustomAggOut::setStandGrid(MapGrid *mapgrid)
{
    foreach (Output *l, mLevels)
        dynamic_cast<CustomAggOutLevel*>(l)->setStandGrid(mapgrid);
}

const QStringList aggList = { "mean", "sum", "min", "max",
                             "p25", "p50", "p75", "p5", "p10", "p90", "p95",
                             "sd", "p80","p85"};

void CustomAggOutLevel::setup()
{
    QString tab_name = settings().value(".tablename");
    bool enabled = settings().valueBool(".enabled");
    // set up output
    setName(QString("Custom output for %1").arg(tab_name), tab_name);


    // set up fixed columns


    QString entity_filter = settings().value(".entityfilter","");
    QString level_filter = settings().value(".levelfilter","");
    QString fieldList = settings().value(".columns", "");
    QString condition = settings().value(".filter", "");

    QString aggtype = settings().value(".entity", "tree").toLower();
    mEntity = CustomAggOut::Invalid;
    if (aggtype == "tree") mEntity = CustomAggOut::Trees;
    if (aggtype == "ru") mEntity = CustomAggOut::RU;
    if (aggtype == "sapling") mEntity = CustomAggOut::Saplings;
    if (aggtype == "snag") mEntity = CustomAggOut::Snags;
    if (mEntity == CustomAggOut::Invalid)
        throw IException(QString("CustomAggOut: invalid value for 'entity': '%1'. Allowed are: ru/tree/sapling/snag").arg(aggtype));

    mLevel = CustomAggOut::sInvalid;
    aggtype = settings().value(".level", "ru").toLower();
    if (aggtype == "ru") mLevel = CustomAggOut::sRU;
    if (aggtype == "stand") mLevel = CustomAggOut::sStand;
    if (aggtype == "landscape") mLevel = CustomAggOut::sLandscape;
    if (mLevel == CustomAggOut::sInvalid)
        throw IException(QString("CustomAggOut: invalid value for 'level' (spatial aggregation level): '%1'. Allowed are: ru/stand/landscape").arg(aggtype));

    if (fieldList.isEmpty())
        return;
    mEntityFilter.setExpression(entity_filter);
    mCondition.setExpression(condition);
    mLevelFilter.setExpression(level_filter);

    mStandGrid = GlobalSettings::instance()->model()->standGrid();

    // clear columns
    columns().clear();
    //qDeleteAll(mFieldList);
    mFieldList.clear();


    switch (mEntity) {
    case CustomAggOut::Trees: columns() << OutputColumn::year() <<  OutputColumn::species(); break;
    case CustomAggOut::Saplings: columns() << OutputColumn::year()  << OutputColumn::species(); break;
    case CustomAggOut::RU: columns() << OutputColumn::year(); break;
    case CustomAggOut::Snags: columns() << OutputColumn::year() << OutputColumn::species(); break;
    case CustomAggOut::Invalid: break;
    }
    switch (mLevel) {
    case CustomAggOut::sLandscape: break;
    case CustomAggOut::sStand: columns() << OutputColumn("stand_id", "Id of the stand", OutInteger) << OutputColumn("area", "area of the stand (ha)", OutDouble); break;
    case CustomAggOut::sRU: columns() << OutputColumn::ru() << OutputColumn::id(); break;
    case CustomAggOut::sInvalid: break;
    }

    // setup fields
    if (!fieldList.isEmpty()) {
        QRegularExpression re("([^\\.]+).(\\w+)[,\\s]*"); // two parts: before dot and after dot, and , + whitespace at the end

        QString field, aggregation;
        TreeWrapper tw;
        SaplingWrapper sw;
        RUWrapper rw;
        DeadTreeWrapper dw;
        QRegularExpressionMatchIterator i = re.globalMatch(fieldList);
        while (i.hasNext()) {
            QRegularExpressionMatch match = i.next();
            field = match.captured(1);
            aggregation = match.captured(2);

            SDynamicField *dfield = new SDynamicField;
            mFieldList.append( dfield );
            // parse field
            if (field.size()>0 && !field.contains('(')) {
                // simple expression: extract index from wrappers
                int var_index = 0;

                switch (mEntity) {
                case CustomAggOut::Trees: var_index = tw.variableIndex(field); break;
                case CustomAggOut::Saplings: var_index = sw.variableIndex(field); break;
                case CustomAggOut::RU: var_index = rw.variableIndex(field); break;
                case CustomAggOut::Snags: var_index = dw.variableIndex(field); break;
                default: throw IException("Invalid aggregation in custom agg output!");
                }

                dfield->var_index = var_index;
            } else {
                // complex expression
                dfield->var_index=-1;
                dfield->expression.setExpression(field);
                //mFieldList.back().expression = QScopedPointer<Expression>(new Expression(field));
            }

            dfield->agg_index = aggList.indexOf(aggregation);
            if (dfield->agg_index==-1)
                throw IException(QString("Invalid aggregate expression for dynamic output: %1\nallowed:%2")
                                 .arg(aggregation).arg(aggList.join(" ")));

            QString stripped_field=QString("%1_%2").arg(field, aggregation);
            stripped_field.replace(QRegularExpression("[\\[\\]\\,\\(\\)<>=!\\-\\+/\\*\\s]"), "_");
            stripped_field.replace("__", "_");
            columns() << OutputColumn(stripped_field, field, OutDouble);
        }
    }

    // enable (and open output table/file)
    setEnabled(enabled);
}

void CustomAggOutLevel::exec()
{
    if (mFieldList.count()==0)
        return;
    // filter for years
    if (!mCondition.isEmpty())
        if (!mCondition.calculate(GlobalSettings::instance()->currentYear()))
            return;

    DebugTimer t("customagg output");

    switch (mEntity) {
    case CustomAggOut::Trees: runTrees(); break;
    case CustomAggOut::Saplings: runSaplings(); break;
    case CustomAggOut::Snags: runSnags(); break;
    default: throw IException("Invalid aggregation level in custom agg output!");
    }

}





// process tree based aggregations
void CustomAggOutLevel::runTrees()
{

    // data to (temporarily) store values: per species vector (dim=fields) of vectors (dim=trees)
    QMap<QString, QVector<QVector<double> > > data;
    TreeWrapper tw;
    bool do_filter = !mEntityFilter.isEmpty();

    switch (mLevel) {
    case CustomAggOut::sLandscape: {
        // loop over all trees in the landsacpe
        AllTreeIterator ati(GlobalSettings::instance()->model());
        while (Tree *t = ati.next()) {
            if (do_filter) {
                tw.setTree(t);
                if (!mEntityFilter.calculateBool(tw))
                    continue; // skip
            }
            processTree(t, data);
        }
        writeResults(data, nullptr, 0);

    }
    case CustomAggOut::sRU: {
        const QList<ResourceUnit*> &ru_list = GlobalSettings::instance()->model()->ruList();
        for (int i=0;i<ru_list.size();++i) {
            data.clear();
            if (!mLevelFilter.isEmpty()) {
                if (!mLevelFilter.calculateBool(ru_list[i]->id()))
                    continue;
            }

            // loop over all trees
            QVector<Tree> &trees = ru_list[i]->trees();
            for (int j=0; j<trees.size();++j) {
                if (do_filter) {
                    tw.setTree(&trees[j]);
                    if (!mEntityFilter.calculateBool(tw))
                        continue; // skip
                }
                processTree(&trees[j], data);
            }

            writeResults(data, ru_list[i], 0);

        }
        break;
    }
    case CustomAggOut::sStand: {
        if (!mStandGrid || !mStandGrid->isValid())
            throw IException("CustomAggOut: aggregation per stand, but no valid standgrid available / set!");

        QList<int> ids = mStandGrid->mapIds();
        for (int i=0;i<ids.size();++i) {
            if (!mLevelFilter.isEmpty()) {
                if (!mLevelFilter.calculateBool(ids[i]))
                    continue;
            }

            data.clear();
            // skip stands with Ids < 1 (empty, out of project area)
            if (ids[i] <= 0)
                continue;

            // loop over all trees of each stand
            QVector<Tree*> trees = mStandGrid->trees(ids[i]);
            for (int j=0; j<trees.size();++j) {
                if (do_filter) {
                    tw.setTree(trees[j]);
                    if (!mEntityFilter.calculateBool(tw))
                        continue; // skip
                }
                processTree(trees[j], data);
            }

            writeResults(data, nullptr, ids[i]);

        }
        break;

    }
    default: return;

    }

}



// process tree based aggregations
void CustomAggOutLevel::runSnags()
{
    if (!Globals->model()->settings().carbonCycleEnabled)
        throw IException("CustomAgg: should process Snags, but carbon cycle is not enabled in the model!");

    // data to (temporarily) store values: per species vector (dim=fields) of vectors (dim=trees)
    QMap<QString, QVector<QVector<double> > > data;
    DeadTreeWrapper tw;
    bool do_filter = !mEntityFilter.isEmpty();

    switch (mLevel) {
    case CustomAggOut::sLandscape: {
        // loop over all trees in the landsacpe
        for (const auto &ru : Globals->model()->ruList()) {
            for (const auto &dt : ru->snag()->deadTrees()) {
                if (do_filter) {
                    tw.setDeadTree(&dt);
                    if (!mEntityFilter.calculateBool(tw))
                        continue;
                }
                processSnag(&dt, data);
            }
        }

        writeResults(data, nullptr, 0);

    }
    case CustomAggOut::sRU: {

        for (const auto &ru : Globals->model()->ruList()) {

            data.clear();
            if (!mLevelFilter.isEmpty()) {
                if (!mLevelFilter.calculateBool(ru->id()))
                    continue;
            }

            // loop over all snags
            for (const auto &dt : ru->snag()->deadTrees()) {
                if (do_filter) {
                    tw.setDeadTree(&dt);
                    if (!mEntityFilter.calculateBool(tw))
                        continue; // skip
                }
                processSnag(&dt, data);
            }

            writeResults(data, ru, 0);

        }
        break;
    }
    case CustomAggOut::sStand: {
        if (!mStandGrid || !mStandGrid->isValid())
            throw IException("CustomAggOut: aggregation per stand, but no valid standgrid available / set!");

        QList<int> ids = mStandGrid->mapIds();
        QVector<DeadTree*> dead_trees;
        for (int i=0;i<ids.size();++i) {
            if (!mLevelFilter.isEmpty()) {
                if (!mLevelFilter.calculateBool(ids[i]))
                    continue;
            }

            data.clear();
            // skip stands with Ids < 1 (empty, out of project area)
            if (ids[i] <= 0)
                continue;

            // loop over all trees of each stand
            mStandGrid->loadDeadTrees(ids[i], dead_trees);
            for (const auto &dt : dead_trees) {
                if (do_filter) {
                    tw.setDeadTree(dt);
                    if (!mEntityFilter.calculateBool(tw))
                        continue; // skip
                }
                processSnag(dt, data);
            }

            writeResults(data, nullptr, ids[i]);

        }
        break;

    }
    default: return;

    }

}

// process sapling based aggregation
void CustomAggOutLevel::runSaplings()
{

    // data to (temporarily) store values: per species vector (dim=fields) of vectors (dim=trees)
    QMap<QString, QVector<QVector<double> > > data;

    switch (mLevel) {
    case CustomAggOut::sLandscape: {
        // loop over all trees in the landsacpe
        foreach (ResourceUnit *ru, GlobalSettings::instance()->model()->ruList()) {
            SaplingCell *s = ru->saplingCellArray();
            for (int px=0;px<cPxPerHectare;++px, ++s) {
                int n_on_px = s->n_occupied();
                if (n_on_px>0) {
                    processSaplingCell(s, ru, data);
                }
            }
        }
        writeResults(data, nullptr, 0);
        break;

    }

    case CustomAggOut::sRU: {
        // loop over all trees in the landsacpe
        foreach (ResourceUnit *ru, GlobalSettings::instance()->model()->ruList()) {
            if (!mLevelFilter.isEmpty()) {
                if (!mLevelFilter.calculateBool(ru->id()))
                    continue;
            }
            data.clear(); // reset data for each resource unit

            SaplingCell *s = ru->saplingCellArray();
            for (int px=0;px<cPxPerHectare;++px, ++s) {
                int n_on_px = s->n_occupied();
                if (n_on_px>0) {
                    processSaplingCell(s, ru, data);
                }
            }
            writeResults(data, ru, 0);
        }
        break;
    }

    case CustomAggOut::sStand: {
        if (!mStandGrid || !mStandGrid->isValid())
            throw IException("CustomAggOut: aggregation per stand, but no valid standgrid available / set!");

        QList<int> ids = mStandGrid->mapIds();
        for (int i=0;i<ids.size();++i) {
            if (!mLevelFilter.isEmpty()) {
                if (!mLevelFilter.calculateBool(ids[i]))
                    continue;
            }

            data.clear();
            // skip stands with Ids < 1 (empty, out of project area)
            if (ids[i] <= 0)
                continue;

            SaplingCellRunner scr(ids[i], mStandGrid);
            SaplingWrapper sw;

            while (SaplingCell *sc = scr.next()) {
                processSaplingCell(sc, scr.ru(), data);
            }
            writeResults(data, nullptr, ids[i]);
        }
        break;

    }
    default: return;
    }

}



void CustomAggOutLevel::processTree(const Tree *t, QMap<QString, QVector< QVector<double> > > &data)
{
    TreeWrapper tw;
    tw.setTree(t);

    if (!data.contains(t->species()->id()))
        data[t->species()->id()] = QVector<QVector<double> >(mFieldList.size(), QVector<double>(0));

    QVector< QVector<double> >  &dat = data[t->species()->id()];

    // retrieve values for all fields for the tree
    for (int i=0;i<mFieldList.size();++i) {
        SDynamicField *field = mFieldList[i];
        if (field->var_index>-1) {
            dat[i].push_back(tw.value(field->var_index));
        } else {
            dat[i].push_back( field->expression.calculate(tw) );
        }
    }
}

void CustomAggOutLevel::processSaplingCell(const SaplingCell *sc, const ResourceUnit *ru, QMap<QString, QVector<QVector<double> > > &data)
{
    bool do_filter = !mEntityFilter.isEmpty();

    SaplingWrapper sw;
    for (int i=0;i<NSAPCELLS;++i) {
        if (sc->saplings[i].is_occupied()) {

            if (do_filter) {
                sw.setSaplingTree(&sc->saplings[i], ru);
                if (!mEntityFilter.calculateBool(sw))
                    continue; //
            }
            processSapling(&sc->saplings[i],
                           ru,
                           sc->saplings[i].resourceUnitSpecies(ru)->species()->id(), //
                           data);

        }
    }
}

void CustomAggOutLevel::processSapling(const SaplingTree *t, const ResourceUnit *ru, const QString &speciesId, QMap<QString, QVector<QVector<double> > > &data)
{
    SaplingWrapper sw;
    sw.setSaplingTree(t, ru);

    if (!data.contains(speciesId))
        data[speciesId] = QVector<QVector<double> >(mFieldList.size(), QVector<double>(0));

    QVector< QVector<double> >  &dat = data[speciesId]; // get a reference to the underlying data

    // retrieve values for all fields for the tree
    for (int i=0;i<mFieldList.size();++i) {
        SDynamicField *field = mFieldList[i];
        if (field->var_index>-1) {
            dat[i].push_back(sw.value(field->var_index));
        } else {
            dat[i].push_back( field->expression.calculate(sw) );
        }
    }
}

void CustomAggOutLevel::processRU(const ResourceUnit *ru)
{
    RUWrapper rw;
    rw.setResourceUnit(ru);

    // retrieve values for all fields for the tree
    for (int i=0;i<mFieldList.size();++i) {
        SDynamicField *field = mFieldList[i];
        if (field->var_index>-1) {
            //field.data.push_back(rw.value(field.var_index));
            // TODO
        } else {
            //field.data.push_back( field.expression->calculate(rw) );
            // TODO
        }
    }
}

void CustomAggOutLevel::processSnag(const DeadTree *dt, QMap<QString, QVector<QVector<double> > > &data)
{
    DeadTreeWrapper tw;
    tw.setDeadTree(dt);

    if (!data.contains(dt->species()->id()))
        data[dt->species()->id()] = QVector<QVector<double> >(mFieldList.size(), QVector<double>(0));

    QVector< QVector<double> >  &dat = data[dt->species()->id()];

    // retrieve values for all fields for the tree
    for (int i=0;i<mFieldList.size();++i) {
        SDynamicField *field = mFieldList[i];
        if (field->var_index>-1) {
            dat[i].push_back(tw.value(field->var_index));
        } else {
            dat[i].push_back( field->expression.calculate(tw) );
        }
    }
}

void CustomAggOutLevel::writeResults(QMap<QString, QVector<QVector<double> > > &data, ResourceUnit *ru, int stand_id)
{

    foreach( QString species, data.keys()) {

        // data: field - trees
        QVector<QVector<double> > &dat = data[species];

        writeFirstCols(species, ru, stand_id);

        for (int i=0;i<mFieldList.size();++i) {
            // summarize according to the definition
            double value = aggregate(*mFieldList[i], dat[i] );
            // add to output stream
            *this << value;
        }

        writeRow();
    }

}



void CustomAggOutLevel::writeFirstCols(QString &species_id, ResourceUnit *ru, int stand_id)
{
    *this << currentYear(); // year in all outputs

    *this << species_id;  // species level for all outputs

    switch (mLevel) {
    case CustomAggOut::sLandscape: break;
    case CustomAggOut::sStand: *this << stand_id << mStandGrid->area(stand_id) / 10000.;  break;
    case CustomAggOut::sRU:
        if (!ru)
            throw IException("CustomAggLevel: expected ResourceUnit, but got none!");
        *this << ru->index() << ru->id();
        break;
    case CustomAggOut::sInvalid: break;
    }

}


double CustomAggOutLevel::aggregate(const SDynamicField &field, QVector<double> &data)
{
    StatData stat(data);
    // aggregate
    double value;
    switch (field.agg_index) {
    case 0: value = stat.mean(); break;
    case 1: value = stat.sum(); break;
    case 2: value = stat.min(); break;
    case 3: value = stat.max(); break;
    case 4: value = stat.percentile25(); break;
    case 5: value = stat.median(); break;
    case 6: value = stat.percentile75(); break;
    case 7: value = stat.percentile(5); break;
    case 8: value = stat.percentile(10); break;
    case 9: value = stat.percentile(90); break;
    case 10: value = stat.percentile(95); break;
    case 11: value = stat.standardDev(); break;
    case 12: value = stat.percentile(80); break;
    case 13: value = stat.percentile(85); break;
    default: value = 0.; break;
    }
    return value;
}

void CustomAggOutLevel::populateSaplingData(QMap<QString, QVector<QPair<SaplingTree *, ResourceUnit *> > > &data, Expression &filter, SaplingCell *sapcell, bool by_species)
{

    if (sapcell){
        SaplingWrapper sw;
        for (int i=0;i<NSAPCELLS;++i) {
            if (sapcell->saplings[i].is_occupied()) {

                if (!filter.isEmpty()) {
                    sw.setSaplingTree(&sapcell->saplings[i], sapcell->ru);
                    if (!filter.execute())
                        continue; //
                }
                // store a pointer to the sapling tree in the data structure
                if (by_species) {

                    data[ sapcell->saplings[i].resourceUnitSpecies(sapcell->ru)->species()->id() ].push_back( QPair<SaplingTree*, ResourceUnit*> (&sapcell->saplings[i], sapcell->ru) );
                } else {
                    data[ "" ].push_back( QPair<SaplingTree*, ResourceUnit*> (&sapcell->saplings[i], sapcell->ru) );
                }

            }
        }
    }


}

