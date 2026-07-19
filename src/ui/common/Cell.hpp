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

/** @file Cell.hpp */

#ifndef CELL_HPP
#define CELL_HPP

// Include 3D Forest.
#include <Brush.hpp>
#include <Color.hpp>
#include <Ui.hpp>
#include <Util.hpp>

// Include local.
#include <ExportUiCommon.hpp>
#include <WarningsDisable.hpp>

/** Cell. */
class EXPORT_UI_COMMON Cell
{
public:
    Cell();

    void setText(const std::string &str);
    std::string text() const { return text_; }

    void setSelected(bool b);
    bool isSelected() const { return selected_; }

    void setNumeric(bool b);
    bool isNumeric() const { return numeric_; }

    void setCheckState(Ui::CheckState state);
    Ui::CheckState checkState() const { return state_; }

    void setBackground(const Brush &brush);
    Brush background() const { return background_; }

    void setFlags(int flags);
    int flags() const { return flags_; }

    bool operator<(const Cell &other) const
    {
        if (isNumeric())
        {
            return toDouble(text()) < toDouble(other.text());
        }

        return text() < other.text();
    }

private:
    std::string text_;
    bool selected_{false};
    bool numeric_{false};
    int flags_{0};
    Ui::CheckState state_{Ui::Unchecked};
    Brush background_;
};

#include <WarningsEnable.hpp>

#endif /* CELL_HPP */
