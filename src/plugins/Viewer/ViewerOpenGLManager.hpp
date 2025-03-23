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

/** @file ViewerOpenGLManager.hpp */

#ifndef VIEWER_OPEN_GL_MANAGER_HPP
#define VIEWER_OPEN_GL_MANAGER_HPP

// Include std.
#include <vector>

// Include 3D Forest.
#include <ViewerOpenGL.hpp>

class ViewerOpenGLResource;
class ViewerOpenGLStrokeFont;

/** Viewer OpenGL Manager. */
class ViewerOpenGLManager
{
public:
    enum Models
    {
        Sphere = 0,
        Box = 1,
        SelectionBox = 2,
        Circle = 3,
        CircleFill = 4,
        Cylinder = 5,
        Cone = 6,
        Billboard = 7,
        SelectionRect = 8,
        SphereWires = 9
    };

    enum Fonts
    {
        FontStroke = 0
    };

    enum ResourceType
    {
        Unknown = 0,
        DisplayList = 1,
        Texture = 2,
        BufferARB = 3,
        ProgramARB = 4
    };

    ViewerOpenGLManager();
    virtual ~ViewerOpenGLManager();

    /** Initialization. */
    virtual void init();

    /** Release the manager.
        \param keep Set to false to force delete on all resources.
                    All resources will be automatically released soon as
                    the OpenGL context is gone. This can be usefull if there
                    are more managers per one GL context. */
    virtual void release(bool keep = true);

    /** Create new resource.
        \param owner Specifies the owner by a pointer.
        \param id Specifies the resource to create.
        \param res Specifies the type of the resource.
        \param range Specifies the range for multi resources with a single
                     identifier.
        \return OpenGL identifier of new resource. */
    GLuint createResource(ViewerOpenGLResource *owner,
                          size_t id,
                          ViewerOpenGLManager::ResourceType res,
                          GLsizei range = 1);

    /** Get resource.
        \param owner Specifies the owner by a pointer.
        \param id Specifies the resource to obtain.
        \return OpenGL identifier of requested resource. */
    GLuint resource(ViewerOpenGLResource *owner, size_t id);

    /** Release resource.
        \param owner Specifies the owner by a pointer.
        \param id Specifies the resource to release. */
    void releaseResource(ViewerOpenGLResource *owner, size_t id);

    /** Release all owners resources and remove the entry.
        \param owner Specifies the owner by a pointer. */
    void releaseResources(ViewerOpenGLResource *owner);

    /** Remove owner from the manager.
        \param owner Specifies the owner by a pointer. */
    void removeOwner(ViewerOpenGLResource *owner);

    /** Release all resources from the trash.

        The OpenGL context where the resources were created should be
        active to have the effect. */
    void updateResources();

    /** Return manager's identifier.
        \return id number of this manager. */
    const bool &isInitialized() const { return initialized_; }

    /** Return the identifier.
        \return the unique identifier of this manager. */
    const size_t &managerId() const { return managerId_; }

    GLuint font();
    std::string cleanText(const std::string &text);
    float textWidth(const std::string &text);
    float textHeight();

protected:
    class Resource
    {
    public:
        GLuint number{0};
        ResourceType resource{ResourceType::Unknown};
        GLsizei range{0};
        GLuint *names{nullptr};
    };

    void updateGL();

    /** Manager initialized flag. */
    bool initialized_;

    /** The identifier number. */
    size_t managerId_;

    /** The generator of the identifiers. */
    static size_t nextManagerId_;

    /** Cache: last owner. */
    void *lastAccess_;

    /** Cache: the result for last owner. */
    size_t lastAccessIdx_;

    /** Resources: map owners-resourcesIdx. */
    std::vector<ViewerOpenGLResource *> owners_;

    /** Resources: map owners-resourcesIdx. */
    std::vector<size_t> resourcesIdx_;

    /** OpenGL resources. */
    std::vector<std::shared_ptr<std::vector<Resource>>> resources_;

    /** The number of display lists. */
    size_t nDisplayLists_;

    size_t nTextures_;

    size_t nBuffers_;

    size_t nPrograms_;

    /** Resources that are planned to be deleted (trash). */
    std::vector<Resource> toDelete_;

    bool separateSpecular_;

    int versionMajor_;
    int versionMinor_;

    std::shared_ptr<ViewerOpenGLStrokeFont> strokeFont_;

public:
    // texture
    ViewerOpenGL::PFNGLACTIVETEXTUREARBPROC glActiveTextureARB;

    // misc
    ViewerOpenGL::PFNGLDRAWRANGEELEMENTSPROC glDrawRangeElements;

    // shader
    ViewerOpenGL::PFNGLGENPROGRAMSARBPROC glGenProgramsARB;
    ViewerOpenGL::PFNGLDELETEPROGRAMSARBPROC glDeleteProgramsARB;
    ViewerOpenGL::PFNGLBINDPROGRAMARBPROC glBindProgramARB;
    ViewerOpenGL::PFNGLPROGRAMSTRINGARBPROC glProgramStringARB;
    ViewerOpenGL::PFNGLPROGRAMENVPARAMETER4FARBPROC glProgramEnvParameter4fARB;
    ViewerOpenGL::PFNGLPROGRAMENVPARAMETER4FVARBPROC
        glProgramEnvParameter4fvARB;

    // VBO
    ViewerOpenGL::PFNGLBINDBUFFERARBPROC glBindBufferARB;
    ViewerOpenGL::PFNGLDELETEBUFFERSARBPROC glDeleteBuffersARB;
    ViewerOpenGL::PFNGLGENBUFFERSARBPROC glGenBuffersARB;
    ViewerOpenGL::PFNGLISBUFFERARBPROC glIsBufferARB;
    ViewerOpenGL::PFNGLBUFFERDATAARBPROC glBufferDataARB;
};

#endif /* VIEWER_OPEN_GL_MANAGER_HPP */