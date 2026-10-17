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

/** @class OutputManager
   Global container that handles data output.

  */

#include "global.h"
#include "outputmanager.h"
#include "debugtimer.h"
#include <QtCore>

// tree outputs
#include "treeout.h"
#include "standout.h"
#include "landscapeout.h"
#include "standdeadout.h"
#include "managementout.h"
#include "dynamicstandout.h"
#include "productionout.h"
#include "saplingout.h"
#include "carbonout.h"
#include "carbonflowout.h"
#include "soilinputout.h"
#include "waterout.h"
#include "svdout.h"
#include "devstageout.h"
#include "ecovizout.h"


// on creation of the output manager
// an instance of every iLand output
// must be added to the list of outputs.
OutputManager::OutputManager()
{
    mTransactionOpen = false;
    // add all the outputs
    mOutputs.append(new TreeOut);
    mOutputs.append(new TreeRemovedOut);
    mOutputs.append(new StandOut);
    mOutputs.append(new LandscapeOut);
    mOutputs.append(new LandscapeRemovedOut);
    mOutputs.append(new DynamicStandOut);
    mOutputs.append(new ProductionOut);
    mOutputs.append(new StandDeadOut);
    mOutputs.append(new ManagementOut);
    mOutputs.append(new SaplingOut);
    mOutputs.append(new SaplingDetailsOut);
    mOutputs.append(new CarbonOut);
    mOutputs.append(new CarbonFlowOut);
    mOutputs.append(new SoilInputOut);
    mOutputs.append(new WaterOut);
    mOutputs.append(new SVDGPPOut);
    mOutputs.append(new SVDStateOut);
    mOutputs.append(new SVDIndicatorOut);
    mOutputs.append(new SVDUniqueStateOut);
    mOutputs.append(new DevStageOut);
    mOutputs.append(new EcoVizOut);
}

void OutputManager::addOutput(Output *output)
{
    mOutputs.append(output);
}

void OutputManager::removeOutput(const QString &tableName)
{
    Output *o = find(tableName);
    if (o) {
        mOutputs.removeAt(mOutputs.indexOf(o));
        delete o;
    }
}


OutputManager::~OutputManager()
{
    qDeleteAll(mOutputs);
}

void OutputManager::setup()
{
    //close();
    XmlHelper &xml = const_cast<XmlHelper&>(GlobalSettings::instance()->settings());
    QString nodepath;
    foreach(Output *o, mOutputs) {
        nodepath = QString("output.%1").arg(o->tableName());
        xml.setCurrentNode(nodepath);
        qDebug() << "setup of output" << o->name() << "(" << o->tableName() << ")";
        o->setup();
        bool enabled = xml.valueBool(".enabled", false);
        bool file_mode = false;
        if (xml.hasNode(".mode"))
            file_mode = xml.value(".mode") == "file";
        if (file_mode)
            o->setMode(OutFile);
        o->setEnabled(enabled);
        if (enabled)
            o->open();
    }
    endTransaction(); // just to be sure
}

Output *OutputManager::find(const QString& tableName)
{
    foreach(Output* p,mOutputs)
        if (p->tableName()==tableName)
            return p;
    return nullptr;
}

void OutputManager::save()
{
    endTransaction();
}

void OutputManager::close()
{
    qDebug() << "outputs closed";
    foreach(Output *p, mOutputs)
        p->close();
}

/** start a database transaction.
    do nothing if transaction is already open. */
void OutputManager::startTransaction()
{
    //return; // test without transactions
    if (!mTransactionOpen && GlobalSettings::instance()->dbout().isValid()) {
        if (GlobalSettings::instance()->dbout().transaction()) {
            qDebug() << "opening transaction";
            mTransactionOpen = true;
        }
    }
}
void OutputManager::endTransaction()
{
    //return; // test without transactions
    if (mTransactionOpen && GlobalSettings::instance()->dbout().isValid()) {
        if (GlobalSettings::instance()->dbout().commit()) {
            mTransactionOpen = false;
            qDebug() << "database transaction commited";
        }
    }
}

bool OutputManager::execute(const QString& tableName)
{
    DebugTimer t("OutputManager::execute()");
    t.setSilent();
    Output *p = find(tableName);
    if (p) {
        if (!p->isEnabled())
            return false;
        if(!p->isOpen())
            return false;
        if (!p->isRowEmpty()) {
            qWarning() << "Output" << p->name() << "invalid (not at new row)!!!";
            return false;
        }

        startTransaction(); // just assure a transaction is open.... nothing happens if already inside a transaction
        p->exec();

        return true;
    }
    qDebug() << "output" << tableName << "not found!";
    return false; // no output found
}

QString OutputManager::wikiFormat()
{
    QString result;
    foreach(const Output *o, mOutputs)
        result+=o->wikiFormat() + "\n\n";
    return result;
}

