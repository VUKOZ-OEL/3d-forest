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
    @file Forest3dThreadRender.hpp
*/

#ifndef FOREST_3D_THREAD_RENDER_HPP
#define FOREST_3D_THREAD_RENDER_HPP

#include <Camera.hpp>
#include <QObject>
#include <Thread.hpp>

class Editor;

class Forest3dThreadRender : public Thread
{
    Q_OBJECT

public:
    Forest3dThreadRender(QObject *parent = nullptr);
    virtual ~Forest3dThreadRender();

    void start(const Camera &camera, Editor *editor, bool finished);
    void restart();

    virtual bool compute();

signals:
    void statusChanged();

protected:
    Editor *editor_;
    Camera camera_;
    bool interactionFinished_;
    bool initialized_;
};

#endif /* FOREST_3D_THREAD_RENDER_HPP */
