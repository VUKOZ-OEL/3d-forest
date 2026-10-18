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

#ifndef STANDLOADER_H
#define STANDLOADER_H
#include <QtCore/QString>

#include "csvfile.h"

class Model;
class ResourceUnit;
class RandomCustomPDF;
class Species;
class MapGrid;
class Expression;

class StandLoader
{
public:
    StandLoader(Model *model): mModel(model), mRandom(0), mCurrentMap(0), mInitHeightGrid(0), mHeightGridResponse(0) {}
    ~StandLoader();
    /// define a stand grid externally
    void setMap(const MapGrid *map) { mCurrentMap = map; }
    /// set a constraining height grid (10m resolution)
    void setInitHeightGrid(const MapGrid *height_grid) { mInitHeightGrid = height_grid; }

    /// main function of stand initialization
    /// the function loads - depending on the XML project file - inits for a single resource unit, for polygons or a snapshot from a database.
    void processInit();
    /// this function is called *after* the init and after initial LIF-calculations.
     void processAfterInit();

    /// load a single tree file (picus or iland style). return number of trees loaded.
    int loadPicusFile(const QString &fileName, ResourceUnit *ru=NULL);
    /// load a tree distribution based on dbh classes. return number of trees loaded.
    int loadiLandFile(const QString &fileName, ResourceUnit *ru=NULL, int stand_id=0);

    /// worker function to load a file containing single trees
    int loadSingleTreeList(const QString &content, ResourceUnit*ru = NULL, const QString &fileName="");
    /// worker function to load a file containing rows with dbhclasses
    int loadDistributionList(const QString &content, ResourceUnit *ru = NULL, int stand_id=0, const QString &fileName="");
    // load regeneration in stands
    int loadSaplings(const QString &content, int stand_id, const QString &fileName=QString());
    // load regen in stand but consider also the light conditions on the ground
    int loadSaplingsLIF(int stand_id, const CSVFile &init, int low_index, int high_index);
private:
    struct InitFileItem
    {
        Species *species;
        double count;
        double dbh_from, dbh_to;
        double hd;
        int age;
        double density;
    };
    /// load tree initialization from a file. return number of trees loaded.
    int loadInitFile(const QString &fileName, const QString &type, int stand_id=0, ResourceUnit *ru=NULL);
    void executeiLandInit(ResourceUnit *ru); ///< shuffle tree positions
    void executeiLandInitStand(int stand_id); ///< shuffle tree positions
    void copyTrees(); ///< helper function to quickly fill up the landscape by copying trees
    void evaluateDebugTrees(); ///< set debug-flag for trees by evaluating the param-value expression "debug_tree"
    int parseInitFile(const QString &content, const QString &fileName, ResourceUnit *ru=0); ///< creates a list of InitFileItems from the init files' content
    Model *mModel;
    RandomCustomPDF *mRandom;
    QVector<InitFileItem> mInitItems;
    QHash<int, QVector<InitFileItem> > mStandInitItems;
    const MapGrid *mCurrentMap;
    const MapGrid *mInitHeightGrid; ///< grid with tree heights
    Expression *mHeightGridResponse; ///< response function to calculate fitting of pixels with pre-determined height
    int mHeightGridTries; ///< maximum number of tries to land at pixel with fitting height
};

#endif // STANDLOADER_H
