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
    @file Forest3dThreadRender.cpp
*/

#include <Editor.hpp>
#include <Forest3dThreadRender.hpp>
#include <QDebug>
#include <Time.hpp>

Forest3dThreadRender::Forest3dThreadRender(QObject *parent)
    : Thread(parent), editor_(nullptr), interactionFinished_(false),
      initialized_(false)
{
}

Forest3dThreadRender::~Forest3dThreadRender()
{
}

void Forest3dThreadRender::start(const Camera &camera,
                                 Editor *editor,
                                 bool finished)
{
    cancel();

    editor_ = editor;
    camera_ = camera;
    interactionFinished_ = finished;
    initialized_ = false;

    Thread::start();
}

void Forest3dThreadRender::restart()
{
    // TBD
    cancel();
    initialized_ = false;
    interactionFinished_ = false;
    Thread::start();
}

bool Forest3dThreadRender::compute()
{
    if (!initialized_)
    {
        editor_->lock();
        editor_->updateCamera(camera_, interactionFinished_);
        editor_->unlock();
        initialized_ = true;
        return false;
    }

    double t1 = getRealTime();
    bool ret;
    editor_->lock();
    ret = editor_->loadView();
    editor_->unlock();

    emit statusChanged();

    double t2 = getRealTime();
    if (t2 - t1 < 0.02)
    {
        msleep(1);
    }

    return ret;
}
