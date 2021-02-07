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
    @file GLViewer.hpp
*/

#ifndef GL_VIEWER_HPP
#define GL_VIEWER_HPP

#include <Editor.hpp>
#include <QWidget>
#include <Scene.hpp>
#include <vector>

class GLWidget;

/** OpenGL Viewer. */
class GLViewer : public QWidget
{
    Q_OBJECT

public:
    enum ViewLayout
    {
        VIEW_LAYOUT_SINGLE,
        VIEW_LAYOUT_TWO_COLUMNS
    };

    explicit GLViewer(QWidget *parent = nullptr);
    ~GLViewer();

    void setViewLayout(ViewLayout viewLayout);
    void selectViewport(GLWidget *viewport);

    void updateScene(const Scene &scene);
    void updateScene(Editor *editor);

protected:
    std::vector<GLWidget *> viewports_;

    void initializeViewer();
    GLWidget *createViewport();
};

#endif /* GL_VIEWER_HPP */
