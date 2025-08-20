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

#ifndef BARKBEETLESCRIPT_H
#define BARKBEETLESCRIPT_H

#include <QObject>
#include <QJSValue>

#include "scriptglobal.h"

class BarkBeetleModule; // forward

class BarkBeetleScript : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QJSValue onClick READ onClick WRITE setOnClick)
    Q_PROPERTY(bool simulate READ simulate WRITE setSimulate)
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled)

public:
    explicit BarkBeetleScript(QObject *parent = 0);
    void setBBModule(BarkBeetleModule *module) { mBeetle = module; }
    QJSValue onClick() const { return mOnClick; }
    void setOnClick(QJSValue handler) { mOnClick = handler; }
    // properties
    bool simulate();
    void setSimulate(bool do_simulate);
    bool enabled();
    void setEnabled(bool do_set_enable);
signals:

public slots:
    void test(QString value);

    void init(QJSValue fun);
    void run(QJSValue fun);
    double pixelValue(int ix, int iy);
    void setPixelValue(int ix, int iy, double val);

    /// access the number of bark beetle generation at position ix/iy (indices on the 10m grid)
    double generations(int ix, int iy);

    // the real thing
    void reloadSettings(); ///< reload the BB-Module settings from the XML-File
    void newYear(); ///< start a new "year" (not synced with iLand clock)
    void runBB(int iteration); ///< run a full cycle of the bark beetle module
    void clear(); ///< reset the barkbeetle module (clear damage and spread data - makes only sense if in simulation mode)
    bool gridToFile(QString type, QString filename);

    /// get a JS representation of the grid given by 'type'
    QJSValue grid(QString type);
    /// number of damaged pixels with a patchsize>'threshold'; save to grid to "fileName" (if not empty)
    int damagedArea(int threshold, QString fileName);

    /// clear the 'infested' flags on the stand 'stand_id' (on the given 'standmap').
    /// this does not remove trees!
    int clearInfestedPixels(QJSValue standmap, int stand_id, double fraction);

    /// initiate damage
    /// sets pixel at (x/y) (grid indices) as infested, return true if pixel was a potential host pixel
    bool setInfested(int x, int y);
    /// set all pixels on stand 'key' on the map 'grid' (with probability 'probability') as infested
    int setInfestedFromMap(MapGridWrapper *grid, int key, double probability, int agerange);

    /// set a new value for background infestation probability (this overwrites
    /// the value given in the project file)
    void setBackgroundInfestationProbability(double new_value);

private:

    QJSValue mOnClick;
    BarkBeetleModule *mBeetle;

};

#endif // BARKBEETLESCRIPT_H
