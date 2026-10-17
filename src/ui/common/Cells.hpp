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

/** @file Cells.hpp */

#ifndef CELLS_HPP
#define CELLS_HPP

// Include std.
#include <vector>

// Include 3D Forest.
#include <Cell.hpp>

// Include local.
#include <ExportUiCommon.hpp>
#include <WarningsDisable.hpp>

/** Cells. */
class EXPORT_UI_COMMON Cells
{
public:
    Cells();

    void resize(size_t index);

    void setText(size_t index, const std::string &str);
    std::string text(size_t index) const;

    void setSelected(size_t index, bool b);
    bool isSelected(size_t index) const;

    void setCheckState(size_t index, Ui::CheckState state);
    Ui::CheckState checkState(size_t index) const;

    void setBackground(size_t index, const Brush &brush);
    Brush background(size_t index) const;

    void setSelected(bool b);
    bool isSelected() const { return selected_; }

private:
    std::vector<Cell> cells_;
    bool selected_{false};
};

#include <WarningsEnable.hpp>

#endif /* CELLS_HPP */
