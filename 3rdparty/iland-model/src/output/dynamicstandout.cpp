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
#include "dynamicstandout.h"

#include "debugtimer.h"
#include "statdata.h"
#include "model.h"
#include "resourceunit.h"
#include "species.h"
#include "expressionwrapper.h"

DynamicStandOut::DynamicStandOut()
{
    setName("dynamic stand output by species/RU", "dynamicstand");
    setDescription("Userdefined outputs for tree aggregates for each stand or species.\n"\
                   "Technically, each field is calculated 'live', i.e. it is looped over all trees, and eventually the statistics (percentiles) "\
                   "are calculated. The aggregated values are not scaled to any area unit.\n" \
                   "!!!Specifying the aggregation\n" \
                   "The ''by_species'' and ''by_ru'' option allow to define the aggregation level. When ''by_species'' is set to ''true'', " \
                   "a row for each species will be created, otherwise all trees of all species are aggregated to one row. " \
                   "Similarly, ''by_ru''=''true'' means outputs for each resource unit, while a value of ''false'' aggregates over the full project area.\n" \
                   "Even if ''by_ru'' is false, the calculation of RU level outputs can be triggered by the ''conditionRU'' switch (variable='year'). Note  " \
                   " that in this case landscape level outputs are generated always, RU-level outputs only for certain years.\n" \
                   "!!!Specifying filters\n" \
                   "You can use the 'rufilter' and 'treefilter' XML settings to reduce the limit the output to a subset of resource units / trees. " \
                   "Both filters are valid expressions (for resource unit level and tree level, respectively). For example, a ''treefilter'' of 'speciesindex=0' reduces the output to just one species.\n" \
                   "The ''condition'' filter is (when present) evaluated and the output is only executed when ''condition'' is true (variable='year') This can be used to constrain the output to specific years (e.g. 'in(year,100,200,300)' produces output only for the given year.\n" \
                   "!!!Specifying data columns\n"
                   "Each field is defined as: ''field.aggregation'' (separated by a dot). A ''field'' is a valid [Expression]. ''Aggregation'' is one of the following:  " \
                   "mean, sum, min, max, p25, p50, p75, p5, 10, p80, p85, p90, p95 (pXX=XXth percentile), sd (std.dev.).\n" \
                   "Complex expression are allowed, e.g: if(dbh>50,1,0).sum (-> counts trees with dbh>50)\n" \
                   "Note that the column names in the output table may be slightly different, as dots (and other special characsters) are not allowed in column names und substituted.\n" \
                   "Note also, that `customagg` is another highly customizable output (https://iland-model.org/dynamic+outputs).");
    columns() << OutputColumn::year() << OutputColumn::ru()  << OutputColumn::id() << OutputColumn::species();
    // other colums are added during setup...
}

const QStringList aggList = QStringList() << "mean" << "sum" << "min" << "max" << "p25" << "p50" << "p75" << "p5"<< "p10" << "p90" << "p95" << "sd" << "p80" << "p85";

void DynamicStandOut::setup()
{
    QString filter = settings().value(".rufilter","");
    QString tree_filter = settings().value(".treefilter","");
    QString fieldList = settings().value(".columns", "");
    QString condition = settings().value(".condition", "");
    QString conditionRU = settings().value(".conditionRU", "");

    if (fieldList.isEmpty())
        return;
    mRUFilter.setExpression(filter);
    mTreeFilter.setExpression(tree_filter);
    mCondition.setExpression(condition);
    mConditionRU.setExpression(conditionRU);
    // clear columns
    columns().erase(columns().begin()+4, columns().end());
    mFieldList.clear();

    // setup fields
    if (!fieldList.isEmpty()) {
        QRegularExpression re("([^\\.]+).(\\w+)[,\\s]*"); // two parts: before dot and after dot, and , + whitespace at the end

        QString field, aggregation;
        TreeWrapper tw;
        QRegularExpressionMatchIterator i = re.globalMatch(fieldList);
        while (i.hasNext()) {
            QRegularExpressionMatch match = i.next();
            field = match.captured(1);
            aggregation = match.captured(2);

            mFieldList.append(SDynamicField());
            // parse field
            if (field.size()>0 && !field.contains('(')) {
                // simple expression
                mFieldList.back().var_index = tw.variableIndex(field);
            } else {
                // complex expression
                mFieldList.back().var_index=-1;
                mFieldList.back().expression = field;
            }

            mFieldList.back().agg_index = aggList.indexOf(aggregation);
            if (mFieldList.back().agg_index==-1)
                throw IException(QString("Invalid aggregate expression for dynamic output: %1\nallowed:%2")
                                 .arg(aggregation).arg(aggList.join(" ")));

            QString stripped_field=QString("%1_%2").arg(field, aggregation);
            stripped_field.replace(QRegularExpression("[\\[\\]\\,\\(\\)<>=!\\-\\+/\\*\\s]"), "_");
            stripped_field.replace("__", "_");
            columns() << OutputColumn(stripped_field, field, OutDouble);
        }
    }
}

void DynamicStandOut::exec()
{
    if (mFieldList.count()==0)
        return;
    if (!mCondition.isEmpty())
        if (!mCondition.calculate(GlobalSettings::instance()->currentYear()))
            return;

    DebugTimer t("dynamic stand output");

    bool per_species = GlobalSettings::instance()->settings().valueBool("output.dynamicstand.by_species", true);
    bool per_ru = GlobalSettings::instance()->settings().valueBool("output.dynamicstand.by_ru", true);
    bool per_ru_cond=false;

    if (!mConditionRU.isEmpty() && mConditionRU.calculate(GlobalSettings::instance()->currentYear()))
        per_ru_cond = true;


    if (per_ru) {
        // when looping over resource units, do it differently (old way)
        extractByResourceUnit(per_species);
        return;
    }
    if (per_ru_cond) {
        // in this case the RU level outputs are *in addition* to the landscape means
        extractByResourceUnit(per_species);
    }

    Model *m = GlobalSettings::instance()->model();
    QVector<double> data; //statistics data
    TreeWrapper tw;
    Expression custom_expr;

    StatData stat; // statistcs helper class
    // grouping
    QVector<const Tree*> trees;
    for (QList<Species*>::const_iterator species = m->speciesSet()->activeSpecies().constBegin();species!=m->speciesSet()->activeSpecies().constEnd();++species) {
        trees.clear();
        AllTreeIterator all_trees(m);
        while (const Tree *t = all_trees.nextLiving()) {
            if (per_species && t->species() != *species)
                continue;
            trees.push_back(t);
        }
        if (trees.size()==0)
            continue;
        // dynamic calculations
        foreach (const SDynamicField &field, mFieldList) {

            if (!field.expression.isEmpty()) {
                // setup dynamic dynamic expression if present
                custom_expr.setExpression(field.expression);
                custom_expr.setModelObject(&tw);
            }

            // fetch data values from the trees
            data.clear();
            foreach(const Tree *t, trees) {
                tw.setTree(t);
                if (field.var_index>=0)
                    data.push_back(tw.value(field.var_index));
                else
                    data.push_back(custom_expr.execute());
            }
            // constant values (if not already present)
            if (isRowEmpty()) {
                *this << currentYear() << -1 << -1;
                if (per_species)
                    *this << (*species)->id();
                else
                    *this << "";
            }

            // calculate statistics
            stat.setData(data);
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
            // add current value to output
            *this << value;
        }
        if (!isRowEmpty())
            writeRow();

        if (!per_species)
            break;
    }

}


void DynamicStandOut::extractByResourceUnit(const bool by_species)
{

    if (mFieldList.count()==0)
        return;

    Model *m = GlobalSettings::instance()->model();
    QVector<double> data; //statistics data
    StatData stat; // statistcs helper class
    TreeWrapper tw;
    RUWrapper ruwrapper;
    mRUFilter.setModelObject(&ruwrapper);

    Expression custom_expr;


    foreach(ResourceUnit *ru, m->ruList()) {
        if (ru->id()==-1)
            continue; // do not include if out of project area

        // test filter
        if (!mRUFilter.isEmpty()) {
            ruwrapper.setResourceUnit(ru);
            if (!mRUFilter.execute())
                continue;
        }
        foreach(const ResourceUnitSpecies *rus, ru->ruSpecies()) {
            if (by_species && rus->constStatistics().count()==0)
                continue;


            // dynamic calculations
            foreach (const SDynamicField &field, mFieldList) {

                if (!field.expression.isEmpty()) {
                    // setup dynamic dynamic expression if present
                    custom_expr.setExpression(field.expression);
                    custom_expr.setModelObject(&tw);
                }
                data.clear();
                bool has_trees = false;
                foreach(const Tree &tree, ru->trees()) {
                    if (by_species && tree.species()->index()!=rus->species()->index())
                        continue;
                    if (tree.isDead())
                        continue;
                    tw.setTree(&tree);

                    // apply treefilter
                    if (!mTreeFilter.isEmpty()) {
                        mTreeFilter.setModelObject(&tw);
                        if (!mTreeFilter.execute())
                            continue;
                    }
                    has_trees = true;

                    if (field.var_index>=0)
                        data.push_back(tw.value(field.var_index));
                    else
                        data.push_back(custom_expr.execute());
                }

                // do nothing if no trees are avaiable
                if (!has_trees)
                    continue;


                if (isRowEmpty()) {
                    *this << currentYear()  << ru->index() << ru->id();
                    if (by_species)
                        *this << rus->species()->id();
                    else
                        *this << "";
                 }

                // calculate statistics
                stat.setData(data);
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
                // add current value to output
                *this << value;

            } // foreach (field)
            if (!isRowEmpty())
                writeRow();
            if (!by_species)
                break;
        } //foreach species
    } // foreach ressource unit

}
