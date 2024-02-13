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

/** @file HeightMapWidget.hpp */

#ifndef HEIGHT_MAP_WIDGET_HPP
#define HEIGHT_MAP_WIDGET_HPP

// Include 3D Forest.
class MainWindow;
class HeightMapModifier;

// Include Qt.
#include <QWidget>
class QSpinBox;
class QComboBox;
class QCheckBox;
class QPushButton;

/** Height Map Widget.

    This class represents Height Map GUI as view-controller for
    HeightMapModifier.
*/
class HeightMapWidget : public QWidget
{
    Q_OBJECT

public:
    HeightMapWidget(MainWindow *mainWindow, HeightMapModifier *modifier);

    void closeModifier();

protected slots:
    void colorCountChanged(int i);
    void colormapChanged(int index);
    void sourceChanged(int index);
    void previewChanged(int index);
    void apply();

protected:
    MainWindow *mainWindow_;
    HeightMapModifier *modifier_;

    QSpinBox *colorCountSpinBox_;
    QComboBox *colormapComboBox_;
    QComboBox *sourceComboBox_;
    QCheckBox *previewCheckBox_;

    QPushButton *applyButton_;
};

#endif /* HEIGHT_MAP_WIDGET_HPP */
