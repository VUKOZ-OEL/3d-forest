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

/** @file EditorThreadRender.cpp */

#include <Editor.hpp>
#include <EditorThreadRender.hpp>
#include <Log.hpp>
#include <ThreadCallbackInterface.hpp>
#include <Time.hpp>

#define LOG_LOCAL(msg)
//#define LOG_LOCAL(msg) LOG_MODULE("EditorThreadRender", msg)

EditorThreadRender::EditorThreadRender(Editor *editor)
    : editor_(editor),
      viewportId_(0),
      initialized_(false)
{
}

void EditorThreadRender::render(size_t viewportId, const EditorCamera &camera)
{
    LOG_LOCAL("viewportId=" << viewportId);
    cancel();

    viewportId_ = viewportId;
    camera_ = camera;
    initialized_ = false;

    Thread::start();
}

bool EditorThreadRender::compute()
{
    LOG_LOCAL("initialized=" << initialized_);
    if (!initialized_)
    {
        editor_->lock();
        editor_->viewports().selectCamera(viewportId_, camera_);
        editor_->unlock();
        initialized_ = true;
        return false;
    }

    LOG_LOCAL("nextState");
    double t1 = getRealTime();
    bool finished;
    editor_->lock();
    finished = editor_->viewports().nextState();
    editor_->unlock();
    double t2 = getRealTime();
    double msec = (t2 - t1) * 1000.;

    if (callback_)
    {
        LOG_LOCAL("callback finished=" << finished << ", ms=" << msec);
        callback_->threadProgress(finished);
    }

    if (msec < 20.)
    {
        msleep(1);
    }

    return finished;
}
