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

/** @file ProgressBar.hpp */

#ifndef PROGRESS_BAR_HPP
#define PROGRESS_BAR_HPP

// Include 3D Forest.
#include <Widget.hpp>

// Include local.
#include <ExportUiCommon.hpp>
#include <WarningsDisable.hpp>

/** Progress Bar. */
class EXPORT_UI_COMMON ProgressBar : public Widget
{
public:
    ProgressBar();
    virtual ~ProgressBar();

    void setRange(int min, int max);
    int minimum() const { return min_; }
    int maximum() const { return max_; }

    void setValue(int value);
    void setLabelText(const std::string &str);

private:
    int min_;
    int max_;
};

#include <WarningsEnable.hpp>

#endif /* PROGRESS_BAR_HPP */
