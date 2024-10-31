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

/** @file ComputeHeightMapWidget.hpp */

#ifndef COMPUTE_HEIGHT_MAP_WIDGET_HPP
#define COMPUTE_HEIGHT_MAP_WIDGET_HPP

// Include 3D Forest.
class MainWindow;
class ComputeHeightMapModifier;

// Include Qt.
#include <QWidget>
class QSpinBox;
class QComboBox;
class QCheckBox;
class QPushButton;

/** Compute Height Map Widget.

    This class represents Compute Height Map GUI as view-controller for
    ComputeHeightMapModifier.
*/
class ComputeHeightMapWidget : public QWidget
{
    Q_OBJECT

public:
    ComputeHeightMapWidget(MainWindow *mainWindow,
                           ComputeHeightMapModifier *modifier);

    void closeModifier();

protected slots:
    void colorCountChanged(int i);
    void colormapChanged(int index);
    void sourceChanged(int index);
    void previewChanged(int index);
    void apply();

protected:
    MainWindow *mainWindow_;
    ComputeHeightMapModifier *modifier_;

    QSpinBox *colorCountSpinBox_;
    QComboBox *colormapComboBox_;
    QComboBox *sourceComboBox_;
    QCheckBox *previewCheckBox_;

    QPushButton *applyButton_;
};

#endif /* COMPUTE_HEIGHT_MAP_WIDGET_HPP */
