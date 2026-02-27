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
#include "biteoutput.h"
#include "biteagent.h"
#include "biteengine.h"
#include "bitecell.h"
#include "expression.h"

namespace BITE {

BiteOutput::BiteOutput()
{
    setName("Annual summary for each Bite agent", "bite");
    setDescription("The output provides annual statistics for each simulated biotic agent. " );
    columns() << OutputColumn::year()
              << OutputColumn("agent", "name of the biotic agent", OutString)
              << OutputColumn("NColonized", "Number of cells that were (at the end of the year, without mortality) colonized by the agent", OutInteger)
              << OutputColumn("NDispersing", "Number of cells that were actively spreading the agent in this year", OutInteger)
              << OutputColumn("NNewlyColonized", "Number of cells that were newly colonized in this year", OutInteger)
              << OutputColumn("agentBiomass", "total biomass of the agent (on all active cells, if applicable)", OutDouble)
              << OutputColumn("treesKilled", "number of host trees killed in the current year", OutInteger)
              << OutputColumn("volumeKilled", "total volume (m3) of trees killed by the agent in the current year", OutDouble)
              << OutputColumn("totalImpact", "total impact (e.g. for defoliatores foliage mass consumed)", OutDouble)
              << OutputColumn("saplingImpact", "number of sapling cohorts affected (browsing)", OutInteger)
              << OutputColumn("saplingKilled", "number of sapling cohorts killed", OutInteger);

}

void BiteOutput::exec()
{
    BiteEngine *e = BiteEngine::instance();
    for (int i=0;i<e->mAgents.count();++i) {
        BiteAgent *a = e->mAgents[i];
        *this << currentYear();
        *this << a->name();
        *this << a->stats().nActive << a->stats().nDispersal << a->stats().nNewlyColonized;
        *this << a->stats().agentBiomass << a->stats().treesKilled << a->stats().m3Killed;
        *this << a->stats().totalImpact;
        *this << a->stats().saplingsImpact << a->stats().saplingsKilled;
        writeRow();
    }
}

void BiteOutput::setup()
{

}

BiteCellOutput::BiteCellOutput()
{

}

void BiteCellOutput::exec()
{

}

void BiteCellOutput::setup()
{
setEnabled(true); // first call creates the table
}

static QMutex _runCellOut;
void BiteCellOutput::execCell(BiteCell *cell, BiteAgent *agent)
{
    QMutexLocker l(&_runCellOut);


    BiteWrapper bw(agent->wrapper(), cell);
    BACellStat *bas = agent->cellStat(cell);
    if (!bas)
        throw IException("BiteCellOutput: stats grid not available!!");
    *this << currentYear()
            << cell->index()
            << bas->nHostTrees
            << bas->nKilled
            << bas->m3Killed
            << bas->totalImpact
            << bas->nHostSaplings
            << bas->saplingsImpact
            << bas->saplingsKilled;

    for (int i=0;i<mExpressions.size();++i) {
        try {
        double result = mExpressions[i]->execute(nullptr, &bw);
        *this << result;
        } catch (IException &e) {
            // throw a nicely formatted error message
            e.add(QString("BiteOutput: in expression '%2' for cell %1.").
                  arg(cell->index()).
                  arg(mExpressions[i]->expression()) );
            throw;
        }
    }
    // bite outputs can happen concurrently, but output must not be written concurrently
    singleThreadedWriteRow();
}

void BiteCellOutput::setupBite(QStringList &cols, QString tableName)
{
    setName("Bite cell level output", tableName);
    setDescription("Bite cell level output" );
    columns() << OutputColumn::year()
              << OutputColumn("idx", "cell index (cell Id) (see 'index' variable), useful for spatial analysis", OutInteger)
              << OutputColumn("hostTrees", "number of host trees (>4m, passing the 'hostFilter') in the current year", OutInteger)
              << OutputColumn("treesKilled", "number of host trees killed (>4m) in the current year", OutInteger)
              << OutputColumn("volumeKilled", "total volume (m3) of trees killed (>4m) by the agent in the current year", OutDouble)
              << OutputColumn("totalImpact", "total impact (e.g. for defoliatores foliage mass consumed)", OutDouble)
              << OutputColumn("hostSaplings", "number of sapling cohorts that are potential host in the current year (depending on the current filtering)", OutInteger)
              << OutputColumn("saplingImpact", "number of sapling cohorts affected (browsing)", OutInteger)
              << OutputColumn("saplingKilled", "number of sapling cohorts killed", OutInteger);


    //int nKilled; ///< number of trees (>4m) killed
    //double m3Killed; ///< volume of all killed trees (>4m)
    //double totalImpact; ///< impact on tree compartments (depending on the mode)
    //int saplingsKilled; ///< number of saplings (cohorts) killed (<4m)
    //int saplingsImpact; ///< number of saplings affected (e.g. by browsing) (<4m)

    for (int i=0;i<cols.size();++i) {
        QString col=cols[i];
        Expression *expr = new Expression(col);
        mExpressions.push_back(expr);
        if (col=="index") col="idx"; // avoid invalid name
        columns() << OutputColumn(col, "", OutDouble);
    }

}


} // namespace
