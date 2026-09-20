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
    ~ProgressBar() override;

    void setRange(int minimum, int maximum);
    int minimum() const { return minimum_; }
    int maximum() const { return maximum_; }

    void setValue(int value);
    int value() const { return value_; }

    void reset();

    void setTextVisible(bool visible);
    bool isTextVisible() const { return textVisible_; }

    Signal<int, int> rangeChanged;
    Signal<int> valueChanged;
    Signal<> resetRequested;
    Signal<bool> textVisibleChanged;

private:
    int minimum_{0};
    int maximum_{100};
    int value_{-1};
    bool textVisible_{true};
};

#include <WarningsEnable.hpp>

#endif /* PROGRESS_BAR_HPP */
