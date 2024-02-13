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

/** @file ExplorerClippingBoxWidget.hpp */

#ifndef EXPLORER_CLIPPING_BOX_WIDGET_HPP
#define EXPLORER_CLIPPING_BOX_WIDGET_HPP

// Include 3D Forest.
#include <Range.hpp>
#include <Region.hpp>
class DoubleRangeSliderWidget;
class MainWindow;

// Include Qt.
#include <QWidget>

/** Explorer Clipping Box Widget. */
class ExplorerClippingBoxWidget : public QWidget
{
    Q_OBJECT

public:
    ExplorerClippingBoxWidget(MainWindow *mainWindow);

    void setRegion(const Region &region);

signals:
    void signalRegionChanged(const Region &region);

public slots:
    void slotRangeIntermediateMinimumValue();
    void slotRangeIntermediateMaximumValue();

protected:
    DoubleRangeSliderWidget *rangeInput_[3];
    Range<double> clipRange_[3];

    void updateRegion();
};

#endif /* EXPLORER_CLIPPING_BOX_WIDGET_HPP */
