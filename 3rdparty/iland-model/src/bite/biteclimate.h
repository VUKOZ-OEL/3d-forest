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
#ifndef BITECLIMATE_H
#define BITECLIMATE_H

#include <QJSValue>

#include "bitewrapper.h"
class ResourceUnit; // forward
class Climate; // forward
namespace BITE {


class BiteClimate
{
public:
    BiteClimate();
    void setup(QJSValue clim_vars, BITE::BiteWrapperCore &wrapper);

    /// retrieve the climate variable with the given index (raise an exception if invalid)
    double value(int var_index, const ResourceUnit *ru) const;

    /// retrieve climate time series for variable 'type'
    /// the data is provided as vector with daily values
    QVector<double> dailyClimateTimeseries(const ResourceUnit *ru, QString type) const;
private:
    double calculateGDD(const Climate *clim, double threshold_temp) const;
    double calculateMeanWaterContent(const ResourceUnit *ru, bool only_growing_season) const;
    static QStringList mClimateVars;

};

} // namespace
#endif // BITECLIMATE_H
