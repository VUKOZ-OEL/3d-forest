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

/** @file WindowSettingsView.hpp */

#ifndef WINDOW_SETTINGS_VIEW_HPP
#define WINDOW_SETTINGS_VIEW_HPP

#include <EditorSettings.hpp>
#include <QWidget>
#include <vector>

class QCheckBox;
class QPushButton;
class QSlider;
class QTabWidget;
class QTreeWidget;
class QTreeWidgetItem;

/** Window Settings View. */
class WindowSettingsView : public QWidget
{
    Q_OBJECT

public:
    /** Window Settings View Column. */
    enum Column
    {
        COLUMN_CHECKED,
        COLUMN_ID,
        COLUMN_LABEL,
        COLUMN_OPACITY,
        COLUMN_LAST,
    };

    explicit WindowSettingsView(QWidget *parent = nullptr);
    ~WindowSettingsView();

    const EditorSettingsView &settings() const { return settings_; }
    void setSettings(const EditorSettingsView &settings);

public slots:
    void clearSelection();
    void itemChanged(QTreeWidgetItem *item, int column);

    void setPointSize(int v);
    void setFogEnabled(int v);

signals:
    void settingsChanged();
    void settingsChangedApply();

protected:
    QTabWidget *tabWidget_;

    QTreeWidget *tree_;
    QPushButton *deselectButton_;
    QSlider *pointSizeSlider_;
    QCheckBox *fogCheckBox_;

    EditorSettingsView settings_;

    size_t index(const QTreeWidgetItem *item);
    void updateTree();
    void block();
    void unblock();
    void addItem(size_t i);
    void setColorSource(const EditorSettingsView &settings);
};

#endif /* WINDOW_SETTINGS_VIEW_HPP */
