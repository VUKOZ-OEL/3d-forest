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

/** @file SettingsUnitsWidget.hpp */

#ifndef SETTINGS_UNITS_WIDGET_HPP
#define SETTINGS_UNITS_WIDGET_HPP

// Include std.
#include <vector>

// Include 3D Forest.
#include <Editor.hpp>
#include <Settings.hpp>
class MainWindow;
class ColorSwitchWidget;

// Include Qt.
#include <QWidget>
class QCheckBox;
class QDoubleSpinBox;

/** Settings Units Widget. */
class SettingsUnitsWidget : public QWidget
{
    Q_OBJECT

public:
    SettingsUnitsWidget(MainWindow *mainWindow);

public slots:
    void slotUpdate(void *sender, const QSet<Editor::Type> &target);

protected slots:
    void slotIntermediateLas(double v);
    void slotIntermediateUser(double v);
    void slotUserDefined(int v);

private:
    MainWindow *mainWindow_;

    QDoubleSpinBox *ppmLasSpinBox_;
    QDoubleSpinBox *ppmUserSpinBox_;
    QCheckBox *userDefinedCheckBox_;

    SettingsUnits settings_;

    void setSettingsOut();
    void setSettingsIn(const SettingsUnits &settings);
    void block();
    void unblock();
};

#endif /* SETTINGS_UNITS_WIDGET_HPP */
