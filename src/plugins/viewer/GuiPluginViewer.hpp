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

/** @file GuiPluginViewer.hpp */

#ifndef GUI_PLUGIN_VIEWER_HPP
#define GUI_PLUGIN_VIEWER_HPP

#include <GuiViewports.hpp>

#include <QObject>

class GuiWindowMain;
class QAction;

/** Gui Plugin Viewer. */
class GuiPluginViewer : public QObject
{
    Q_OBJECT

public:
    GuiPluginViewer(GuiWindowMain *window);

    GuiViewports *viewports() { return guiViewports_; }

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
    void slotViewLayout(GuiViewports::ViewLayout layout);

protected:
    GuiWindowMain *window_;

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

    GuiViewports *guiViewports_;

    void updateViewer();
};

#endif /* GUI_PLUGIN_VIEWER_HPP */
