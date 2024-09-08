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

/** @file RenderThread.cpp */

// Include 3D Forest.
#include <Editor.hpp>
#include <RenderThread.hpp>
#include <ThreadCallbackInterface.hpp>
#include <Time.hpp>

// Include local.
#define LOG_MODULE_NAME "RenderThread"
#include <Log.hpp>

RenderThread::RenderThread(Editor *editor)
    : editor_(editor),
      initialized_(false)
{
}

void RenderThread::render(const std::vector<Camera> &cameraList)
{
    LOG_DEBUG_RENDER(<< "Render viewports n <" << cameraList.size() << ">.");

    cancel();

    cameraList_ = cameraList;
    initialized_ = false;

    ThreadLoop::start();
}

bool RenderThread::next()
{
    LOG_DEBUG_RENDER(<< "Render next step, initialized <" << initialized_
                     << ">.");

    if (!initialized_)
    {
        LOG_DEBUG_RENDER(<< "Apply camera to viewports.");
        {
            std::unique_lock<std::mutex> mutexlock(editor_->mutex_);
            editor_->viewports().applyCamera(cameraList_);
        }
        initialized_ = true;
        return true;
    }

    LOG_DEBUG_RENDER(<< "Compute next state.");

    double t1 = Time::realTime();
    bool finished = false;
    {
        std::unique_lock<std::mutex> mutexlock(editor_->mutex_);
        finished = !editor_->viewports().nextState();
    }
    double t2 = Time::realTime();
    double msec = (t2 - t1) * 1000.;

    if (callback_)
    {
        LOG_DEBUG_RENDER(<< "Call callback argument finished <" << finished
                         << "> ms <" << msec << ">.");
        callback_->threadProgress(finished);
    }

    if (msec < 20.)
    {
        Time::msleep(1);
    }

    return !finished;
}
