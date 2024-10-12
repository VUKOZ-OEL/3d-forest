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

/** @file TreeAttributesWidget.hpp */

#ifndef TREE_ATTRIBUTES_WIDGET_HPP
#define TREE_ATTRIBUTES_WIDGET_HPP

// Include 3D Forest.
#include <TreeAttributesAction.hpp>
#include <TreeAttributesParameters.hpp>
class MainWindow;
class DoubleSliderWidget;

// Include Qt.
#include <QWidget>
class QPushButton;

/** Tree Attributes Widget. */
class TreeAttributesWidget : public QWidget
{
    Q_OBJECT

public:
    TreeAttributesWidget(MainWindow *mainWindow);

protected slots:
    void slotApply();

protected:
    void hideEvent(QHideEvent *event) override;

private:
    MainWindow *mainWindow_;

    TreeAttributesParameters parameters_;
    TreeAttributesAction treeAttributesAction_;

    DoubleSliderWidget *treePositionHeightRangeSlider_;
    DoubleSliderWidget *dbhElevationSlider_;
    DoubleSliderWidget *dbhElevationRangeSlider_;
    DoubleSliderWidget *maximumValidCalculatedDbhSlider_;

    QPushButton *applyButton_;
};

#endif /* TREE_ATTRIBUTES_WIDGET_HPP */
