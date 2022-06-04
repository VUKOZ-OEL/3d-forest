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

/** @file ViewerPlugin.hpp */

#ifndef VIEWER_PLUGIN_HPP
#define VIEWER_PLUGIN_HPP

#include <ExportGui.hpp>
#include <ViewerViewports.hpp>

#include <QObject>

class MainWindow;
class QAction;

/** Viewer Plugin. */
class EXPORT_GUI ViewerPlugin : public QObject
{
    Q_OBJECT

public:
    ViewerPlugin();

    void initialize(MainWindow *mainWindow);

    ViewerViewports *viewports() { return viewports_; }

public slots:
    void slotViewOrthographic();
    void slotViewPerspective();

    void slotViewTop();
    void slotViewFront();
    void slotViewRight();
    void slotView3d();

    void slotViewResetDistance();
    void slotViewResetCenter();

    void slotViewLayoutSingle();
    void slotViewLayout2Columns();
    void slotViewLayoutGrid();
    void slotViewLayout3RowsRight();
    void slotViewLayout(ViewerViewports::ViewLayout layout);

protected:
    MainWindow *mainWindow_;

    QAction *actionViewOrthographic_;
    QAction *actionViewPerspective_;

    QAction *actionViewTop_;
    QAction *actionViewFront_;
    QAction *actionViewRight_;
    QAction *actionView3d_;

    QAction *actionViewResetDistance_;
    QAction *actionViewResetCenter_;

    QAction *actionViewLayoutSingle_;
    QAction *actionViewLayoutTwoColumns_;
    QAction *actionViewLayoutGrid_;
    QAction *actionViewLayoutGridRight_;

    ViewerViewports *viewports_;

    void updateViewer();
};

#endif /* VIEWER_PLUGIN_HPP */
