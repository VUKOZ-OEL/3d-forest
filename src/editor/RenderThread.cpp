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
// #define LOG_MODULE_DEBUG_ENABLED 1
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
            std::unique_lock<std::mutex> mutexlock(editor_->editorMutex_);
            editor_->viewports().applyCamera(cameraList_);
        }
        initialized_ = true;
        return true;
    }

    LOG_DEBUG_RENDER(<< "Compute next state.");

    double t1 = Time::realTime();
    double t2 = 0;
    double msec = 0;
    bool finished = false;
    bool lruL0Ready = true;
    {
        std::unique_lock<std::mutex> mutexlock(editor_->editorMutex_);
        do
        {
            finished = !editor_->viewports().nextState(&lruL0Ready);
            t2 = Time::realTime();
            msec = (t2 - t1) * 1000.0;
        } while (!finished && (!lruL0Ready || msec < 10.0));
    }

    if (msec > 40.0)
    {
        LOG_DEBUG_RENDER(<< "Extra worker time.");
    }

    if (callback_)
    {
        LOG_DEBUG_RENDER(<< "Call callback argument finished <" << finished
                         << "> ms <" << msec << ">.");
        callback_->threadProgress(finished);
    }

    return !finished;
}
