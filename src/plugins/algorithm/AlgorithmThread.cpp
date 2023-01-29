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

/** @file AlgorithmThread.cpp */

#include <AlgorithmThread.hpp>
#include <AlgorithmWidgetInterface.hpp>
#include <Editor.hpp>
#include <ThreadCallbackInterface.hpp>
#include <Time.hpp>

#define LOG_MODULE_NAME "AlgorithmThread"
#include <Log.hpp>

AlgorithmThread::AlgorithmThread(Editor *editor)
    : editor_(editor),
      query_(editor_),
      algorithm_(nullptr)
{
    LOG_DEBUG(<< "Create algorithm thread.");
}

AlgorithmThread::~AlgorithmThread()
{
    LOG_DEBUG(<< "Destroy algorithm thread.");
}

void AlgorithmThread::clear()
{
    query_.clear();
    algorithm_ = nullptr;
}

void AlgorithmThread::restart(AlgorithmWidgetInterface *algorithm)
{
    LOG_DEBUG(<< "Restart the algorithm.");

    algorithm_ = algorithm;
    if (algorithm_)
    {
        algorithm_->applyParameters();
    }

    Thread::start();
}

bool AlgorithmThread::compute()
{
    LOG_DEBUG(<< "Compute the next step.");

    bool finished;

    if (algorithm_)
    {
        finished = algorithm_->step();
    }
    else
    {
        finished = true;
    }

    if (callback_)
    {
        LOG_DEBUG(<< "Call callback with argument finished <" << finished
                  << ">.");
        callback_->threadProgress(finished);
    }

    return finished;
}

int AlgorithmThread::progressPercent() const
{
    if (algorithm_)
    {
        return algorithm_->progressPercent();
    }

    return 100;
}

void AlgorithmThread::updateData()
{
    LOG_DEBUG(<< "Update data.");
    if (algorithm_)
    {
        algorithm_->updateData();
    }
}
