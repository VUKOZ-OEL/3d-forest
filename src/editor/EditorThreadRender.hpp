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

/** @file EditorThreadRender.hpp */

#ifndef EDITOR_THREAD_RENDER_HPP
#define EDITOR_THREAD_RENDER_HPP

#include <EditorCamera.hpp>
#include <Thread.hpp>

class Editor;

/** Editor Thread Render. */
class EditorThreadRender : public Thread
{
public:
    EditorThreadRender(Editor *editor);

    void render(size_t viewportId, const EditorCamera &camera);

    virtual bool compute();

protected:
    Editor *editor_;
    size_t viewportId_;
    EditorCamera camera_;
    bool initialized_;
};

#endif /* EDITOR_THREAD_RENDER_HPP */
