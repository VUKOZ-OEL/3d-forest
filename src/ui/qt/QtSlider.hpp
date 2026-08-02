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

/** @file QtSlider.hpp */

#ifndef QT_SLIDER_HPP
#define QT_SLIDER_HPP

// Include 3D Forest.
#include <Slider.hpp>

// Include Qt.
#include <QSlider>

// Include local.
#include <ExportUiQt.hpp>
#include <WarningsDisable.hpp>

/** QtSlider. */
class EXPORT_UI_QT QtSlider : public QSlider
{
public:
    explicit QtSlider(Slider *slider, QWidget *parent = nullptr);
    virtual ~QtSlider();

private:
    Slider *slider_;
};

#include <WarningsEnable.hpp>

#endif /* QT_SLIDER_HPP */
