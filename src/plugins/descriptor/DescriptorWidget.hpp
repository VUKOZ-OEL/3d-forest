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

/** @file DescriptorWidget.hpp */

#ifndef DESCRIPTOR_WIDGET_HPP
#define DESCRIPTOR_WIDGET_HPP

#include <DescriptorAction.hpp>
class MainWindow;
class SliderWidget;
class InfoDialog;

#include <QWidget>
class QPushButton;
class QRadioButton;
class QCheckBox;

/** Descriptor Widget. */
class DescriptorWidget : public QWidget
{
    Q_OBJECT

public:
    DescriptorWidget(MainWindow *mainWindow);

protected slots:
    void slotApply();
    void slotHelp();

protected:
    void hideEvent(QHideEvent *event) override;

private:
    MainWindow *mainWindow_;
    InfoDialog *infoDialog_;

    DescriptorAction descriptor_;

    SliderWidget *radiusSlider_;
    SliderWidget *voxelSizeSlider_;
    QCheckBox *groundCheckBox_;
    std::vector<QRadioButton *> methodRadioButton_;

    QPushButton *helpButton_;
    QPushButton *applyButton_;
};

#endif /* DESCRIPTOR_WIDGET_HPP */
