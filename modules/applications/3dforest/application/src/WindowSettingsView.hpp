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

/** Window Settings View. */
class WindowSettingsView : public QWidget
{
    Q_OBJECT

public:
    explicit WindowSettingsView(QWidget *parent = nullptr);
    ~WindowSettingsView();

    const EditorSettingsView &settings() const { return settings_; }
    void setSettings(const EditorSettingsView &settings);

public slots:
    void setPointColor();
    void setColorSourceEnabled(int v);
    void setPointSize(int v);
    void setFogEnabled(int v);

signals:
    void settingsChanged();
    void settingsChangedApply();

protected:
    EditorSettingsView settings_;

    QTabWidget *tabWidget_;
    QPushButton *colorSourceButton_;
    std::vector<QCheckBox *> colorSourceCheckBox_;
    QSlider *pointSizeSlider_;
    QCheckBox *fogCheckBox_;
};

#endif /* WINDOW_SETTINGS_VIEW_HPP */
