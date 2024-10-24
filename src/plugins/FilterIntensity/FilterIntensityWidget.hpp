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

/** @file FilterIntensityWidget.hpp */

#ifndef FILTER_INTENSITY_WIDGET_HPP
#define FILTER_INTENSITY_WIDGET_HPP

// Include 3D Forest.
#include <Editor.hpp>
#include <Range.hpp>
#include <Region.hpp>
class DoubleRangeSliderWidget;
class MainWindow;

// Include Qt.
#include <QWidget>

/** Filter Intensity Widget. */
class FilterIntensityWidget : public QWidget
{
    Q_OBJECT

public:
    FilterIntensityWidget(MainWindow *mainWindow);

    void setFilterEnabled(bool b);

    QSize sizeHint() const override { return QSize(300, 120); }

public slots:
    void slotUpdate(void *sender, const QSet<Editor::Type> &target);

    void slotRangeIntermediateMinimumValue();
    void slotRangeIntermediateMaximumValue();

protected:
    MainWindow *mainWindow_;
    DoubleRangeSliderWidget *intensityInput_;
    Range<double> intensityRange_;

    void setIntensity(const Range<double> &intensityRange);
    void filterChanged();
};

#endif /* FILTER_INTENSITY_WIDGET_HPP */
