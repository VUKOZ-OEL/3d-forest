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

/** @file Splitter.hpp */

#ifndef SPLITTER_HPP
#define SPLITTER_HPP

// Include 3D Forest.
#include <Ui.hpp>
#include <Widget.hpp>

// Include local.
#include <ExportUiCommon.hpp>
#include <WarningsDisable.hpp>

/** Splitter. */
class EXPORT_UI_COMMON Splitter : public Widget
{
public:
    Splitter();

    void addWidget(Widget *widget);
    void setOrientation(Ui::Orientation v);
    void setSizes(const std::vector<int> &sizes);

private:
};

#include <WarningsEnable.hpp>

#endif /* SPLITTER_HPP */
