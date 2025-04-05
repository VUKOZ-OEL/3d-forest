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

/** @file ViewerOpenGLResource.cpp */

// Include 3D Forest.
#include <Util.hpp>
#include <ViewerOpenGLResource.hpp>

// Include local.
#define LOG_MODULE_NAME "ViewerOpenGLResource"
// #define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

size_t ViewerOpenGLResource::nextConsumerId_ = 0;

ViewerOpenGLResource::ViewerOpenGLResource()
{
    LOG_DEBUG(<< "Create id <" << nextConsumerId_ << ">.");
    consumerId_ = nextConsumerId_;

    // Generate the next identifier.
    nextConsumerId_++;
}

ViewerOpenGLResource::~ViewerOpenGLResource()
{
    LOG_DEBUG(<< "Destroy id <" << getConsumerId() << ">.");

    // Deregister consumer in all managers.
    deleteGL();
    deregisterGL();
}

// Notification that given manager is about to be terminated.
void ViewerOpenGLResource::releaseGL(ViewerOpenGLManager *managerGL)
{
    LOG_DEBUG(<< "Release id <" << getConsumerId() << ">.");
    remove(managers_, managerGL);
    remove(initManagers_, managerGL);
}

bool ViewerOpenGLResource::isInitialized(ViewerOpenGLManager *managerGL)
{
    if (contains(initManagers_, managerGL))
    {
        return true;
    }
    return false;
}

// Register that my resources can be found at given manager.
void ViewerOpenGLResource::registerGL(ViewerOpenGLManager *managerGL)
{
    LOG_DEBUG(<< "Register id <" << getConsumerId() << ">.");

    if (!contains(managers_, managerGL))
    {
        managers_.push_back(managerGL);
    }
}

// Set my resources as initialized in the manager.
void ViewerOpenGLResource::initGL(ViewerOpenGLManager *managerGL)
{
    LOG_DEBUG(<< "Init id <" << getConsumerId() << ">.");

    if (!contains(initManagers_, managerGL))
    {
        initManagers_.push_back(managerGL);
    }
}

// Deregister myself from all managers.
void ViewerOpenGLResource::deregisterGL()
{
    LOG_DEBUG(<< "Deregister id <" << getConsumerId() << ">.");

    size_t i = 0;
    size_t n = managers_.size();

    for (i = 0; i < n; ++i)
    {
        if (managers_[i])
        {
            managers_[i]->removeOwner(this);
        }
    }
    managers_.resize(0);
    initManagers_.resize(0);
}

// Delete my resources.
void ViewerOpenGLResource::deleteGL()
{
    LOG_DEBUG(<< "Delete id <" << getConsumerId() << ">.");

    size_t i = 0;
    size_t n = managers_.size();

    // Free the resources.
    for (i = 0; i < n; ++i)
    {
        if (managers_[i])
        {
            managers_[i]->releaseResources(this);
        }
    }

    // Set as not initialized.
    initManagers_.resize(0);
}
