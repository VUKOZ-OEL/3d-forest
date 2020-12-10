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
    @file Viewer.hpp
*/

#ifndef VIEWER_HPP
#define VIEWER_HPP

#include <Node.hpp>
#include <QWidget>
#include <vector>

class GLWidget;

/** Viewer. */
class Viewer : public QWidget
{
    Q_OBJECT

public:
    enum ViewportLayout
    {
        VIEWPORT_LAYOUT_SINGLE,
        VIEWPORT_LAYOUT_TWO_COLUMNS
    };

    explicit Viewer(QWidget *parent = nullptr);
    ~Viewer();

    void setViewportLayout(ViewportLayout viewportLayout);
    void selectViewport(GLWidget *viewport);

    void update(const std::vector<std::shared_ptr<Node>> &scene);

protected:
    std::vector<GLWidget *> viewports_;

    void initializeViewer();
    GLWidget *createViewport();
};

#endif /* VIEWER_HPP */
