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
    @file Forest3dEditor.hpp
*/

#ifndef FOREST_3D_EDITOR_HPP
#define FOREST_3D_EDITOR_HPP

#include <Editor.hpp>
#include <Forest3dThreadRender.hpp>
#include <QWidget>

/** Forest 3d Editor. */
class Forest3dEditor
{
    //    Q_OBJECT

public:
    Editor editor_; // TBD
    Forest3dThreadRender thread_; // TBD

    Forest3dEditor();
    ~Forest3dEditor();

    void cancel();
    void render();

protected:
};

#endif /* FOREST_3D_EDITOR_HPP */
