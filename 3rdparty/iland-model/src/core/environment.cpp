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

#include "global.h"
#include "environment.h"
#include "debugtimer.h"
#include "helper.h"
#include "csvfile.h"
#include "gisgrid.h"

#include "climate.h"
#include "speciesset.h"


/** Represents the input of various variables with regard to climate, soil properties and more.
  @ingroup tools
    Data is read from various sources and presented to the core model with a standardized interface.
    see https://iland-model.org/simulation+extent
*/
Environment::Environment()
{
    mInfile = 0;
    mGrid = 0;
    mGridMode = false;
    mCurrentSpeciesSet = 0;
    mCurrentClimate = 0;
    mCurrentID = 0;
}
Environment::~Environment()
{
    if (mInfile) {
        delete mInfile;
    }
    if (mGrid)
        delete mGrid;
}

bool Environment::loadFromFile(const QString &fileName)
{
    QStringList source = Helper::loadTextFileLines(GlobalSettings::instance()->path(fileName));
    if (source.isEmpty())
        throw IException(QString("Environment: input file does not exist or is empty (%1)").arg(fileName));
    return loadFromString(source);
}

// ******** specific keys *******
const QString speciesKey = "model.species.source";
const QString climateKey = "model.climate.tableName";

bool Environment::loadFromString(const QStringList &source)
{
    try {
        if (mInfile)
            delete mInfile;
        mInfile = new CSVFile();

        mInfile->loadFromStringList(source);
        mKeys = mInfile->captions();

        XmlHelper xml(GlobalSettings::instance()->settings());
        mSpeciesSets.clear(); // note: the objects are not destroyed - potential memory leak.
        mClimate.clear();
        mRowCoordinates.clear();
        mCreatedObjects.clear();
        mCurrentID = 0;

        int index;
        if (mGridMode) {
            int id = mInfile->columnIndex("id");
            if (id<0)
                throw IException("Environment:: (grid mode) input file has no 'id' column!");
            for (int row=0;row<mInfile->rowCount();row++) {
                mRowCoordinates[mInfile->value(row, id).toString()] = row;
            }

        } else {
            // ***  Matrix mode ******
            // each row must contain 'x' and 'y' coordinates
            // setup coordinates (x,y)
            int ix,iy;
            ix = mInfile->columnIndex("x");
            iy = mInfile->columnIndex("y");
            if (ix<0 || iy<0)
                throw IException("Environment:: (matrix mode) input file has no x/y coordinates!");
            for (int row=0;row<mInfile->rowCount();row++) {
                QString key=QString("%1_%2")
                        .arg(mInfile->value(row, ix).toString())
                        .arg(mInfile->value(row, iy).toString());
                mRowCoordinates[key] = row;
            }
        }



        // ******** setup of Species Sets *******
        if ((index = mKeys.indexOf(speciesKey))>-1) {
            DebugTimer t("environment:load species");
            QStringList speciesNames = mInfile->column(index);
            speciesNames.removeDuplicates();
            qDebug() << "creating species sets:" << speciesNames;
            foreach (const QString &name, speciesNames) {
                xml.setNodeValue(speciesKey,name); // set xml value
                // create species sets
                SpeciesSet *set = new SpeciesSet();
                mSpeciesSets.push_back(set);
                mCreatedObjects[name] = (void*)set;
                set->setup();
            }
            qDebug() << mSpeciesSets.count() << "species sets created.";
        } else {
            // no species sets specified
            SpeciesSet *speciesSet = new SpeciesSet();
            mSpeciesSets.push_back(speciesSet);
            speciesSet->setup();
            mCurrentSpeciesSet = speciesSet;
        }

        // ******** setup of Climate *******
        if ((index = mKeys.indexOf(climateKey))>-1) {
            DebugTimer t("environment:load climate");
            QStringList climateNames = mInfile->column(index);
            climateNames.removeDuplicates();
            if (logLevelDebug())
                qDebug() << "creating climatae: " << climateNames;
            qDebug() << "Environment: climate: # of climate tables in environment file:" << climateNames.count();
            foreach (QString name, climateNames) {
                // create an entry in the list of created objects, but
                // really create the climate only if required (see setPosition() )
                mCreatedObjects[name]=(void*)0;
                xml.setNodeValue(climateKey,name); // set xml value
            }
        } else {
            // no climate defined - setup default climate
            Climate *c = new Climate();
            mClimate.push_back(c);
            c->setup();
            mCurrentClimate = c;
        }
        if (!mCurrentClimate && mClimate.count()>0)
            mCurrentClimate = mClimate[0];
        if (!mCurrentSpeciesSet && mSpeciesSets.count()>0)
            mCurrentSpeciesSet = mSpeciesSets[0];
        return true;

    } catch(const IException &e) {
        QString addMsg;
        if (!mClimate.isEmpty())
            addMsg = QString("last Climate: %1 ").arg(mClimate.last()->name());
        if (!mSpeciesSets.isEmpty())
            addMsg += QString("last Speciesset table: %1").arg(mSpeciesSets.last()->name());
        QString error_msg = QString("An error occured during the setup of the environment: \n%1\n%2").arg(e.message()).arg(addMsg);
        qDebug() << error_msg;
        Helper::msg(error_msg);
        return false;
    }
}

/** sets the "pointer" to a "position" (metric coordinates).
    All specified values are set (also the climate/species-set pointers).
*/
void Environment::setPosition(const QPointF position)
{
    // no changes occur, when the "environment" is not loaded
    if (!isSetup())
        return;
    QString key;
    int ix=-1, iy=-1, id=-1;
    if (mGridMode) {
        // grid mode
        id = mGrid->value(position);
        mCurrentID = id;
        key = QString::number(id);
        if (id==-1)
            return; // no data for the resource unit
    } else {
        // access data in the matrix by resource unit indices
        ix = int(position.x() / 100.); // suppose size of 1 ha for each coordinate
        iy = int(position.y() / 100.);
        mCurrentID++; // to have Ids for each resource unit

        key=QString("%1_%2").arg(ix).arg(iy);
    }

    if (mRowCoordinates.contains(key)) {
        XmlHelper xml(GlobalSettings::instance()->settings());
        int row = mRowCoordinates[key];
        QString value;
        if (logLevelInfo()) qDebug() << "settting up point" << position << "with row" << row;
        for (int col=0;col<mInfile->colCount(); col++) {
            if (mKeys[col]=="id") {
                mCurrentID = mInfile->value(row, col).toInt();
                continue;
            }
            if (mKeys[col]=="x" || mKeys[col]=="y") // ignore "x" and "y" keys
                continue;
            value = mInfile->value(row,col).toString();
            if (logLevelInfo()) qDebug() << "set" << mKeys[col] << "to" << value;
            if (!xml.hasNode(mKeys[col])) {
                throw IException("Setup of the environment: tried to set the value of the xml-key '" + mKeys[col] + "', but the node does not exist.");
            }
            if (!xml.setNodeValue(mKeys[col], value)) {
                throw IException("Setup of the environment: tried to set the value of the xml-key '" + mKeys[col] + "', but the node is empty (Note that nodes must not be empty in the XML file, even if they are to be overwritten).");
            }
            // special handling for constructed objects:
            if (mKeys[col]==speciesKey)
                mCurrentSpeciesSet = (SpeciesSet*)mCreatedObjects[value];
            if (mKeys[col]==climateKey) {
                mCurrentClimate = (Climate*)mCreatedObjects[value];
                if (mCurrentClimate==nullptr) {
                    // create only those climate sets that are really used in the current landscape
                    Climate *climate = new Climate();
                    mClimate.push_back(climate);
                    mCreatedObjects[value]=(void*)climate;
                    climate->setup(mClimate.size()<2); // debug log only for the first climate
                    mCurrentClimate = climate;

                }
            }


        }

    } else {
        if (mGridMode)
            throw IException(QString("Environment:setposition: invalid grid id (or not present in input file): %1m/%2m (mapped to id %3).\n Check that id values in 'model.world.environmentGrid' are also present in 'model.world.environmentFile'!")
                             .arg(position.x()).arg(position.y()).arg(id));
        else
            throw IException(QString("Environment:setposition: invalid coordinates (or not present in input file): %1m/%2m (mapped to indices %3/%4). \nYou may miss these indices in the environment file.")
                             .arg(position.x()).arg(position.y()).arg(ix).arg(iy));
    }
}

bool Environment::setGridMode(const QString &grid_file_name)
{
    mGrid = new GisGrid();
    mGrid->loadFromFile(grid_file_name);
    mGridMode = true;
    return true;
}
