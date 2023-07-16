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

/** @file SettingsColorWidget.hpp */

#ifndef SETTINGS_COLOR_WIDGET_HPP
#define SETTINGS_COLOR_WIDGET_HPP

#include <vector>

#include <Settings.hpp>
class MainWindow;
class ColorSwitchWidget;

#include <QWidget>
class QCheckBox;
class QComboBox;
class QSlider;

/** Settings Color Widget. */
class SettingsColorWidget : public QWidget
{
    Q_OBJECT

public:
    SettingsColorWidget(MainWindow *mainWindow);

public slots:
    void slotUpdate(void *sender, const QSet<Editor::Type> &target);
    void slotColorSourceChanged(int index);
    void slotSetPointSize(int v);
    void slotSetFogEnabled(int v);
    void slotSetColor();

private:
    MainWindow *mainWindow_;

    ColorSwitchWidget *colorSwitchWidget_;
    QCheckBox *fogCheckBox_;
    QComboBox *colorSourceComboBox_;
    QSlider *pointSizeSlider_;

    SettingsView settings_;

    void setSettingsOut(bool modifiers = false);
    void setSettingsIn(const SettingsView &settings);
    void block();
    void unblock();
};

#endif /* SETTINGS_COLOR_WIDGET_HPP */
