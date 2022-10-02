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

#include <QWidget>

class MainWindow;

class QCheckBox;
class QPushButton;
class QTabWidget;
class QTreeWidget;
class QTreeWidgetItem;
class QSlider;

/** Settings Color Widget. */
class SettingsColorWidget : public QWidget
{
    Q_OBJECT

public:
    /** Settings Color Widget Column. */
    enum Column
    {
        COLUMN_CHECKED,
        COLUMN_ID,
        COLUMN_LABEL,
        COLUMN_OPACITY,
        COLUMN_LAST
    };

    SettingsColorWidget(MainWindow *mainWindow);

public slots:
    void slotUpdate(const QSet<Editor::Type> &target);

    void slotSetPointSize(int v);
    void slotSetFogEnabled(int v);
    void slotSetColorFg();
    void slotSetColorBg();
    void slotItemChanged(QTreeWidgetItem *item, int column);

protected:
    MainWindow *mainWindow_;
    QTreeWidget *treeWidget_;
    QCheckBox *fogCheckBox_;
    QPushButton *colorFgButton_;
    QPushButton *colorBgButton_;
    QSlider *pointSizeSlider_;
    SettingsView settings_;

    void settingsChanged();
    void settingsChangedApply();

    size_t index(const QTreeWidgetItem *item);
    void updateTree();
    void block();
    void unblock();
    void addItem(size_t i);

    void setColorSource(const SettingsView &settings);
    void setSettings(const SettingsView &settings);

    bool colorDialog(Vector3<float> &rgb);
    void setColor(QPushButton *button, const Vector3<float> &rgb);
};

#endif /* SETTINGS_COLOR_WIDGET_HPP */
