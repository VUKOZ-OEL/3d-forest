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

/**
    @file WindowViewports.hpp
*/

#ifndef WINDOW_VIEWPORTS_HPP
#define WINDOW_VIEWPORTS_HPP

#include <Camera.hpp>
#include <QWidget>
#include <vector>

class Editor;
class GLWidget;

/** Window Viewports. */
class WindowViewports : public QWidget
{
    Q_OBJECT

public:
    enum ViewLayout
    {
        VIEW_LAYOUT_SINGLE,
        VIEW_LAYOUT_TWO_COLUMNS,
        VIEW_LAYOUT_GRID,
        VIEW_LAYOUT_THREE_ROWS_RIGHT
    };

    explicit WindowViewports(QWidget *parent = nullptr);
    ~WindowViewports();

    void setLayout(ViewLayout viewLayout);

    void setViewOrthographic();
    void setViewPerspective();
    void setViewTop();
    void setViewFront();
    void setViewRight();
    void setView3d();
    void setViewResetDistance();
    void setViewResetCenter();

    void selectViewport(GLWidget *viewport);
    size_t selectedViewportId() const;
    Camera camera(size_t viewportId) const;

    void updateScene(Editor *editor);
    void resetScene(Editor *editor);
    void resetScene(Editor *editor, size_t viewportId);

signals:
    void cameraChanged(size_t viewportId);

protected:
    std::vector<GLWidget *> viewports_;

    void initializeViewer();
    GLWidget *createViewport(size_t viewportId);
    GLWidget *selectedViewport();
    const GLWidget *selectedViewport() const;
};

#endif /* WINDOW_VIEWPORTS_HPP */
