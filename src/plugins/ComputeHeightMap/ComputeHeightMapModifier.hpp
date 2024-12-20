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

/** @file ComputeHeightMapModifier.hpp */

#ifndef COMPUTE_HEIGHT_MAP_MODIFIER_HPP
#define COMPUTE_HEIGHT_MAP_MODIFIER_HPP

// Include std.
#include <vector>

// Include 3D Forest.
#include <Vector3.hpp>
class Editor;
class Page;
class MainWindow;

// Include Qt.
#include <QMutex>
class QWidget;

/** Compute Height Map Modifier.

    This class represents Compute Height Map model.

    An instance of this class is shared memory resource between main window GUI
    thread and editor thread. Most of the code must not be run by multiple
    threads at once. A mutex is used to protect concurrent read/write access to
    shared memory.
*/
class ComputeHeightMapModifier
{
public:
    /** Compute Height Map Modifier Source. */
    enum Source
    {
        SOURCE_Z_POSITION,
        SOURCE_ELEVATION
    };

    ComputeHeightMapModifier();

    void initialize(MainWindow *mainWindow);
    void setSource(Source source);
    void setColormap(const QString &name, int colorCount);
    void setPreviewEnabled(bool enabled,
                           bool update = true,
                           bool reload = false);
    bool previewEnabled();
    void applyModifier(Page *page);
    void apply(QWidget *widget);

protected:
    MainWindow *mainWindow_;
    Editor *editor_;
    bool previewEnabled_;
    Source source_;
    std::vector<Vector3<double>> colormap_;
    QMutex mutex_;

    std::vector<Vector3<double>> createColormap(const QString &name,
                                                int colorCount);
};

#endif /* COMPUTE_HEIGHT_MAP_MODIFIER_HPP */
