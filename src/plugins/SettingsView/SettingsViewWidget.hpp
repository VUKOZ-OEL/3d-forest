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

/** @file SettingsViewWidget.hpp */

#ifndef SETTINGS_VIEW_WIDGET_HPP
#define SETTINGS_VIEW_WIDGET_HPP

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
class QComboBox;
class QSlider;

/** Settings View Widget. */
class SettingsViewWidget : public QWidget
{
    Q_OBJECT

public:
    SettingsViewWidget(MainWindow *mainWindow);

    QSize sizeHint() const override { return QSize(300, 240); }

public slots:
    void slotUpdate(void *sender, const QSet<Editor::Type> &target);

    void slotSetPointSize(int v);
    void slotSetColor();
    void slotColorSourceChanged(int index);

    void slotSetDistanceBasedFadingVisible(int v);
    void slotSetSceneBoundingBoxVisible(int v);
    void slotSetTreeAttributesVisible(int v);
    void slotSetTreePositionAtBottom(int v);

private:
    MainWindow *mainWindow_;

    QSlider *pointSizeSlider_;
    ColorSwitchWidget *colorSwitchWidget_;
    QComboBox *colorSourceComboBox_;

    QCheckBox *distanceBasedFadingVisibleCheckBox_;
    QCheckBox *sceneBoundingBoxVisibleCheckBox_;
    QCheckBox *treeAttributesVisibleCheckBox_;
    QCheckBox *treePositionAtBottomCheckBox_;

    SettingsView settings_;

    void dataChanged(bool modifiers = false);
    void setViewSettings(const SettingsView &settings);
    void block();
    void unblock();
};

#endif /* SETTINGS_VIEW_WIDGET_HPP */
