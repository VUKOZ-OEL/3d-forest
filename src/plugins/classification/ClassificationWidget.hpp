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

/** @file ClassificationWidget.hpp */

#ifndef CLASSIFICATION_WIDGET_HPP
#define CLASSIFICATION_WIDGET_HPP

// Include 3D Forest.
#include <ClassificationAction.hpp>
#include <ClassificationParameters.hpp>
class MainWindow;
class DoubleSliderWidget;
class InfoDialog;

// Include Qt.
#include <QWidget>
class QPushButton;
class QCheckBox;

/** Classification Widget. */
class ClassificationWidget : public QWidget
{
    Q_OBJECT

public:
    ClassificationWidget(MainWindow *mainWindow);

protected slots:
    void slotApply();
    void slotHelp();

protected:
    void hideEvent(QHideEvent *event) override;

private:
    MainWindow *mainWindow_;
    InfoDialog *infoDialog_;

    ClassificationParameters parameters_;
    ClassificationAction classification_;

    DoubleSliderWidget *voxelSlider_;
    DoubleSliderWidget *radiusSlider_;
    DoubleSliderWidget *angleSlider_;
    QCheckBox *cleanGroundCheckBox_;
    QCheckBox *cleanAllCheckBox_;

    QPushButton *helpButton_;
    QPushButton *applyButton_;
};

#endif /* CLASSIFICATION_WIDGET_HPP */
