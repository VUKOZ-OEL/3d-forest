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

/** @file HeightMapWindow.hpp */

#ifndef HEIGHT_MAP_WINDOW_HPP
#define HEIGHT_MAP_WINDOW_HPP

#include <QDockWidget>

class MainWindow;
class HeightMapModifier;

class QSpinBox;
class QComboBox;
class QCheckBox;
class QPushButton;
class QCloseEvent;

/** Height Map Window.

    This class represents Height Map GUI as view-controller for
    HeightMapModifier.

    GUI could be provided directly by the top HeightMapPlugin if it was derived
    from QDialog instead of QObject.
*/
class HeightMapWindow : public QDockWidget
{
    Q_OBJECT

public:
    HeightMapWindow(MainWindow *mainWindow, HeightMapModifier *modifier);

protected slots:
    void colorCountChanged(int i);
    void colormapChanged(int index);
    void sourceChanged(int index);
    void previewChanged(int index);
    void apply();

protected:
    MainWindow *mainWindow_;
    HeightMapModifier *modifier_;
    QWidget *widget_;
    QSpinBox *colorCountSpinBox_;
    QComboBox *colormapComboBox_;
    QComboBox *sourceComboBox_;
    QCheckBox *previewCheckBox_;
    QPushButton *applyButton_;

    void closeEvent(QCloseEvent *event) override;
};

#endif /* HEIGHT_MAP_WINDOW_HPP */
