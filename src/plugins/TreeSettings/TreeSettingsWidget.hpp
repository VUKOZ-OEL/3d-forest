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

/** @file TreeSettingsWidget.hpp */

#ifndef TREE_SETTINGS_WIDGET_HPP
#define TREE_SETTINGS_WIDGET_HPP

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

/** Tree Settings Widget. */
class TreeSettingsWidget : public QWidget
{
    Q_OBJECT

public:
    TreeSettingsWidget(MainWindow *mainWindow);

    QSize sizeHint() const override { return QSize(300, 240); }

public slots:
    void slotUpdate(void *sender, const QSet<Editor::Type> &target);

    void slotSetTreeAttributesVisible(int v);
    void slotSetTreePositionAtBottom(int v);
    void slotSetConvexHullVisible(int v);
    void slotSetConvexHullProjectionVisible(int v);

private:
    MainWindow *mainWindow_;

    QCheckBox *treeAttributesVisibleCheckBox_;
    QCheckBox *treePositionAtBottomCheckBox_;
    QCheckBox *convexHullVisibleCheckBox_;
    QCheckBox *convexHullProjectionVisibleCheckBox_;

    TreeSettings settings_;

    void dataChanged(bool modifiers = false);
    void setTreeSettings(const TreeSettings &settings);
    void block();
    void unblock();
};

#endif /* TREE_SETTINGS_WIDGET_HPP */
