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

/** @file RenderThread.hpp */

#ifndef RENDER_THREAD_HPP
#define RENDER_THREAD_HPP

#include <Camera.hpp>
#include <ThreadLoop.hpp>
class Editor;

#include <ExportEditor.hpp>
#include <WarningsDisable.hpp>

/** Render Thread. */
class EXPORT_EDITOR RenderThread : public ThreadLoop
{
public:
    RenderThread(Editor *editor);

    void render(size_t viewportId, const Camera &camera);

    virtual bool next();

protected:
    Editor *editor_;
    size_t viewportId_;
    Camera camera_;
    bool initialized_;
};

#include <WarningsEnable.hpp>

#endif /* RENDER_THREAD_HPP */
