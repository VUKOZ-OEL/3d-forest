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

/** @file Editor.cpp */

#include <Editor.hpp>

Editor::Editor(QObject *parent) : QObject(parent), thread_(this)
{
    connect(&thread_, SIGNAL(statusChanged()), this, SLOT(render()));

    thread_.init();
}

Editor::~Editor()
{
    thread_.stop();
}

// void Editor::renderRequest()
// {
//     emit renderRequested();
// }

void Editor::lock()
{
    mutex_.lock();
}

void Editor::unlock()
{
    mutex_.unlock();
}

void Editor::cancelThreads()
{
    thread_.cancel();
}

void Editor::restartThreads()
{
    thread_.restart();
}

void Editor::render(size_t viewportId, const Camera &camera)
{
    thread_.start(viewportId, camera);
}

void Editor::render()
{
    emit renderRequested();
}
