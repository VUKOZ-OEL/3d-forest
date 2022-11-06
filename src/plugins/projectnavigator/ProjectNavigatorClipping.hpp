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

/** @file ProjectNavigatorClipping.hpp */

#ifndef PROJECT_NAVIGATOR_CLIPPING_HPP
#define PROJECT_NAVIGATOR_CLIPPING_HPP

#include <Range.hpp>

#include <QWidget>

class MainWindow;
class RangeSliderWidget;

class QCheckBox;
class QPushButton;

/** Project Navigator Clipping. */
class ProjectNavigatorClipping : public QWidget
{
    Q_OBJECT

public:
    ProjectNavigatorClipping(MainWindow *mainWindow);

public slots:
    void slotUpdate(const QSet<Editor::Type> &target);

    void slotRangeIntermediateMinimumValue();
    void slotRangeIntermediateMaximumValue();
    void setEnabled(int state);
    void reset();

protected:
    MainWindow *mainWindow_;
    RangeSliderWidget *rangeInput_[3];
    QCheckBox *enabledCheckBox_;
    QPushButton *resetButton_;
    Range<double> clipRange_[3];

    void filterChanged();
};

#endif /* PROJECT_NAVIGATOR_CLIPPING_HPP */
