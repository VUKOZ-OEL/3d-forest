/*
    Copyright 2020 VUKOZ

    This file is part of 3D Forest.

    3D Forest is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    3D Forest is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with 3D Forest.  If not, see <https://www.gnu.org/licenses/>.
*/

/** @file TreeTableExportProperties.hpp */

#ifndef TREE_TABLE_EXPORT_PROPERTIES_HPP
#define TREE_TABLE_EXPORT_PROPERTIES_HPP

// Include Std.
#include <string>

/** Tree Table Export Properties. */
class TreeTableExportProperties
{
public:
    void setFileName(const std::string &fileName) { fileName_ = fileName; }
    const std::string &fileName() const { return fileName_; }

    void setExportValidValuesOnly(bool b) { exportValidValuesOnly_ = b; }
    bool exportValidValuesOnly() const { return exportValidValuesOnly_; }

    void setPointsPerMeter(double ppm) { pointsPerMeter_ = ppm; }
    double pointsPerMeter() const { return pointsPerMeter_; }

private:
    std::string fileName_;
    bool exportValidValuesOnly_{true};
    double pointsPerMeter_{1000.0};
};

#endif /* TREE_TABLE_EXPORT_PROPERTIES_HPP */
