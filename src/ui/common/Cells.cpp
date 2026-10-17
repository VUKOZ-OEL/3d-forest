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

/** @file Cells.cpp */

// Include 3D Forest.
#include <Cells.hpp>

// Include local.
#define LOG_MODULE_NAME "Cells"
#include <Log.hpp>

Cells::Cells()
{
}

void Cells::resize(size_t index)
{
    if (index >= cells_.size())
    {
        cells_.resize(index + 1);
    }
}

void Cells::setText(size_t index, const std::string &str)
{
    resize(index);
    cells_[index].setText(str);
}

std::string Cells::text(size_t index) const
{
    if (index >= cells_.size())
    {
        return "";
    }

    return cells_[index].text();
}

void Cells::setSelected(size_t index, bool b)
{
    resize(index);
    cells_[index].setSelected(b);
}

bool Cells::isSelected(size_t index) const
{
    if (index >= cells_.size())
    {
        return false;
    }

    return cells_[index].isSelected();
}

void Cells::setCheckState(size_t index, Ui::CheckState state)
{
    resize(index);
    cells_[index].setCheckState(state);
}

Ui::CheckState Cells::checkState(size_t index) const
{
    if (index >= cells_.size())
    {
        return Ui::Unchecked;
    }

    return cells_[index].checkState();
}

void Cells::setBackground(size_t index, const Brush &brush)
{
    resize(index);
    cells_[index].setBackground(brush);
}

Brush Cells::background(size_t index) const
{
    if (index >= cells_.size())
    {
        return Brush();
    }

    return cells_[index].background();
}

void Cells::setSelected(bool b)
{
    selected_ = b;
}
