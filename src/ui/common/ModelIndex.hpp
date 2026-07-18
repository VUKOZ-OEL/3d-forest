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

/** @file ModelIndex.hpp */

#ifndef MODEL_INDEX_HPP
#define MODEL_INDEX_HPP

// Include 3D Forest.

// Include local.
#include <ExportUiCommon.hpp>
#include <WarningsDisable.hpp>

/** ModelIndex Bar. */
class EXPORT_UI_COMMON ModelIndex
{
public:
    ModelIndex();

    int row() const { return row_; }
    int column() const { return column_; }

    int columnCount() const { return columnCount_; }
    int rowCount() const { return rowCount_; }

    bool isValid() const;

private:
    int row_{0};
    int column_{0};
    int rowCount_{0};
    int columnCount_{0};
};

#include <WarningsEnable.hpp>

#endif /* MODEL_INDEX_HPP */
