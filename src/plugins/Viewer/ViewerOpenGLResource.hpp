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

/** @file ViewerOpenGLResource.hpp */

#ifndef VIEWER_OPEN_GL_RESOURCE_HPP
#define VIEWER_OPEN_GL_RESOURCE_HPP

// Include std.
#include <vector>

// Include 3D Forest.
#include <ViewerOpenGLManager.hpp>

/** Viewer OpenGL Resource. */
class ViewerOpenGLResource
{
public:
    ViewerOpenGLResource();
    virtual ~ViewerOpenGLResource();

    /** Notification that given manager is about to be terminated. */
    virtual void releaseGL(ViewerOpenGLManager *managerGL);

    bool isInitialized(ViewerOpenGLManager *managerGL);

    /** Return the identifier.
        \return the unique identifier of this consumer. */
    const size_t &getConsumerId() const { return consumerId_; }

protected:
    /** Register that my resources can be found at given manager. */
    virtual void registerGL(ViewerOpenGLManager *managerGL);

    /** Set my resources as initialized in the manager. */
    virtual void initGL(ViewerOpenGLManager *managerGL);

    /** Deregister myself from all managers. */
    virtual void deregisterGL();

    /** Delete my resources. */
    virtual void deleteGL();

    /** The identifier number. */
    size_t consumerId_;

    /** The generator of the identifiers. */
    static size_t nextConsumerId_;

    /** My resources exist in following managers. */
    std::vector<ViewerOpenGLManager *> managers_;
    std::vector<ViewerOpenGLManager *> initManagers_;
};

#endif /* VIEWER_OPEN_GL_RESOURCE_HPP */