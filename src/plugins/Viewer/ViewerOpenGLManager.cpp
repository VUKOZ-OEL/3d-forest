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

/** @file ViewerOpenGLManager.cpp */

// Include 3D Forest.
#include <Util.hpp>
#include <ViewerOpenGLManager.hpp>
#include <ViewerOpenGLStrokeFont.hpp>

// Include Qt.
#include <QOpenGLFunctions>

// Include local.
#define LOG_MODULE_NAME "ViewerOpenGLManager"
#define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

size_t ViewerOpenGLManager::nextManagerId_ = 0;

ViewerOpenGLManager::ViewerOpenGLManager()
    : initialized_(false),
      lastAccess_(nullptr),
      lastAccessIdx_(-1),
      nDisplayLists_(0),
      nTextures_(0),
      nBuffers_(0),
      nPrograms_(0),
      separateSpecular_(false),
      versionMajor_(1),
      versionMinor_(0),
      glActiveTextureARB(nullptr),
      glDrawRangeElements(nullptr),
      glGenProgramsARB(nullptr),
      glDeleteProgramsARB(nullptr),
      glBindProgramARB(nullptr),
      glProgramStringARB(nullptr),
      glProgramEnvParameter4fARB(nullptr),
      glProgramEnvParameter4fvARB(nullptr),
      glBindBufferARB(nullptr),
      glDeleteBuffersARB(nullptr),
      glGenBuffersARB(nullptr),
      glIsBufferARB(nullptr),
      glBufferDataARB(nullptr)

{
    managerId_ = nextManagerId_;
    LOG_DEBUG(<< "Create manager id <" << managerId() << ">.");

    // Generate the next identifier.
    nextManagerId_++;
}

ViewerOpenGLManager::~ViewerOpenGLManager()
{
    LOG_DEBUG(<< "Destroy manager id <" << managerId() << ">.");
}

// Initialization.
void ViewerOpenGLManager::init()
{
    LOG_DEBUG(<< "Init manager id <" << managerId() << ">.");

    int i, j;
    const char *str = nullptr;
    char buffer[1024];

    // Get OpenGL version.
    str = (char *)glGetString(GL_VERSION);
    if (str)
    {
        // str: "<major>.<minor>.<release> <distributor>"
        i = 0;
        while (str[i] != '.')
        {
            buffer[i] = str[i];
            i++;
        }
        buffer[i] = '\0';
        versionMajor_ = atoi(buffer);

        i++;
        j = 0;
        while (str[i] != '.')
        {
            buffer[j] = str[i];
            i++;
            j++;
        }
        buffer[j] = '\0';
        versionMinor_ = atoi(buffer);
    }

    // Enable by version.
    if (versionMajor_ > 1 || (versionMajor_ == 1 && versionMinor_ >= 2))
    {
        // OpenGL 1.2 or greater.
        separateSpecular_ = true;
    }

    lastAccess_ = nullptr;
    initialized_ = true;

    release(false);
}

// Release.
void ViewerOpenGLManager::release(bool keep_)
{
    LOG_DEBUG(<< "Release manager id <" << managerId() << ">.");

    size_t i;
    size_t n;
    size_t iResource;
    size_t nResources;
    std::vector<Resource> *ptr = nullptr;

    // Deregister manager in all consumers.
    n = owners_.size();
    for (i = 0; i < n; ++i)
    {
        if (owners_[i])
        {
            owners_[i]->releaseGL(this);
        }
    }

    if (isInitialized())
    {
        lastAccess_ = nullptr;

        if (keep_)
        {
            // All resources will be automatically released soon as the OpenGL
            // context is gone.
        }
        else
        {
            // Put all resources to the trash.
            n = resources_.size();
            for (i = 0; i < n; ++i)
            {
                ptr = resources_[i].get();
                nResources = (*ptr).size();
                for (iResource = 0; iResource < nResources; ++iResource)
                {
                    toDelete_.push_back((*ptr)[iResource]);
                    LOG_DEBUG(<< "Prepare to delete resource number <"
                              << (*ptr)[iResource].number << ">.");
                }
            }

            // Clear all.
            resources_.resize(0);
            owners_.resize(0);
            resourcesIdx_.resize(0);

            // Call updateResources () with OpenGL context active
            // to actually free the resources.
        }
    }
}

// Create new resource.
GLuint ViewerOpenGLManager::createResource(
    ViewerOpenGLResource *owner,
    size_t id,
    ViewerOpenGLManager::ResourceType res,
    GLsizei range)
{
    LOG_DEBUG(<< "Create resource id <" << id << ">");
    ASSERT(owner);

    size_t n;
    size_t ownerIdx;
    size_t idx;
    GLuint idGL = 0;

    if (lastAccess_ == owner)
    {
        LOG_DEBUG(<< "Use resource cache index <" << lastAccessIdx_ << ">.");
        ownerIdx = lastAccessIdx_;
    }
    else
    {
        ownerIdx = binarySearch(owners_, owner);
    }

    LOG_DEBUG(<< "Resource ownerIdx <" << ownerIdx << ">.");

    if (ownerIdx == SIZE_MAX)
    {
        ownerIdx = insert(owners_, owner);
        LOG_DEBUG(<< "Resource insert index <" << ownerIdx << ">.");

        std::shared_ptr<std::vector<Resource>> res;
        res = std::make_shared<std::vector<Resource>>();
        idx = resources_.size();
        resources_.push_back(res);
        insertAt(resourcesIdx_, ownerIdx, idx);

        LOG_DEBUG(<< "New resource insert index <" << idx << ">.");
    }

    if (ownerIdx != SIZE_MAX)
    {
        // Update cache.
        lastAccess_ = owner;
        lastAccessIdx_ = ownerIdx;

        LOG_DEBUG(<< "Create resource index <" << resourcesIdx_[ownerIdx]
                  << ">.");

        // Resize resources.
        std::vector<Resource> *ptr = resources_[resourcesIdx_[ownerIdx]].get();
        n = (*ptr).size();
        if (id >= n)
        {
            LOG_DEBUG(<< "Resize resources to <" << id + 1 << ">.");
            (*ptr).resize(id + 1);
        }

        LOG_DEBUG(<< "Old resource id <" << (*ptr)[id].number << ">.");

        // Create.
        if (res == ViewerOpenGLManager::DisplayList)
        {
            idGL = glGenLists(range);
            nDisplayLists_++;
            LOG_DEBUG(<< "New list id <" << idGL << "> nDisplayLists <"
                      << nDisplayLists_ << ">.");
        }
        else if (res == ViewerOpenGLManager::Texture)
        {
            ASSERT(range == 1);
            SAFE_GL(glGenTextures(1, (GLuint *)&idGL));
            nTextures_++;
            LOG_DEBUG(<< "New texture id <" << idGL << "> nTextures <"
                      << nTextures_ << ">.");
        }
        else if (res == ViewerOpenGLManager::BufferARB)
        {
            ASSERT(range == 1);
            if (glGenBuffersARB)
            {
                LOG_DEBUG(<< "glGenBuffersARB");
                SAFE_GL(glGenBuffersARB(1, &idGL));
                nBuffers_++;
            }
            LOG_DEBUG(<< "New BufferARB id <" << idGL << "> nBuffers <"
                      << nBuffers_ << ">.");
        }
        else if (res == ViewerOpenGLManager::ProgramARB)
        {
            ASSERT(range == 1);
            if (glGenProgramsARB)
            {
                LOG_DEBUG(<< "glGenProgramsARB");
                SAFE_GL(glGenProgramsARB(1, &idGL));
                nPrograms_++;
            }
            LOG_DEBUG(<< "New program id <" << idGL << "> nPrograms <"
                      << nPrograms_ << ">.");
        }

        (*ptr)[id].number = idGL;
        (*ptr)[id].resource = res;
        (*ptr)[id].range = range;

        LOG_DEBUG(<< "New resource id <" << (*ptr)[id].number << ">.");
        return (*ptr)[id].number;
    }

    return 0;
}

// Get resource.
GLuint ViewerOpenGLManager::resource(ViewerOpenGLResource *owner, size_t id)
{
    LOG_DEBUG(<< "Get resource manager <" << managerId() << "> id <" << id
              << ">.");

    size_t n;
    size_t idx;

    if (lastAccess_ == owner)
    {
        LOG_DEBUG(<< "Using cache index <" << lastAccessIdx_ << ">.");
        idx = lastAccessIdx_;
    }
    else
    {
        idx = binarySearch(owners_, owner);
    }

    LOG_DEBUG(<< "Resource ownerIdx <" << idx << ">.");

    if (idx != SIZE_MAX)
    {
        // Update cache.
        lastAccess_ = owner;
        lastAccessIdx_ = idx;

        LOG_DEBUG(<< "Resources idx <" << resourcesIdx_[idx] << ">.");

        std::vector<Resource> *ptr = resources_[resourcesIdx_[idx]].get();

        n = (*ptr).size();
        LOG_DEBUG(<< "Resources size <" << n << ">.");
        if (id < n)
        {
            LOG_DEBUG(<< "Return resource number <" << (*ptr)[id].number
                      << ">.");
            return (*ptr)[id].number;
        }
    }

    return 0;
}

// Release resource.
void ViewerOpenGLManager::releaseResource(ViewerOpenGLResource *owner,
                                          size_t id)
{
    LOG_DEBUG(<< "Release resource id <" << id << ">.");

    size_t idx;

    if (lastAccess_ == owner)
    {
        LOG_DEBUG(<< "Use cache index <" << lastAccessIdx_ << ">.");
        idx = lastAccessIdx_;
    }
    else
    {
        idx = binarySearch(owners_, owner);
    }

    LOG_DEBUG(<< "Resource ownerIdx <" << idx << ">.");

    if (idx != SIZE_MAX)
    {
        // Update cache.
        lastAccess_ = owner;
        lastAccessIdx_ = idx;

        LOG_DEBUG(<< "Resources index <" << resourcesIdx_[idx] << ">.");

        std::vector<Resource> *ptr = resources_[resourcesIdx_[idx]].get();
        if (ptr)
        {
            if ((*ptr)[id].number > 0)
            {
                toDelete_.push_back((*ptr)[id]);
                LOG_DEBUG(<< "Resource toDelete <" << (*ptr)[id].number
                          << ">.");
                (*ptr)[id].number = 0;
            }
        }
    }
}

// Release all owners resources and remove the entry.
void ViewerOpenGLManager::releaseResources(ViewerOpenGLResource *owner)
{
    LOG_DEBUG(<< "Release resources.");

    size_t i;
    size_t n;
    size_t idx;

    if (lastAccess_ == owner)
    {
        LOG_DEBUG(<< "Use cache index <" << lastAccessIdx_ << ">.");
        idx = lastAccessIdx_;
    }
    else
    {
        idx = binarySearch(owners_, owner);
    }

    LOG_DEBUG(<< "Resource ownerIdx <" << idx << ">.");

    if (idx != SIZE_MAX)
    {
        // Update cache.
        lastAccess_ = owner;
        lastAccessIdx_ = idx;

        LOG_DEBUG(<< "Resources index <" << resourcesIdx_[idx] << ">.");

        std::vector<Resource> *ptr = resources_[resourcesIdx_[idx]].get();
        n = (*ptr).size();
        for (i = 0; i < n; ++i)
        {
            if ((*ptr)[i].number > 0)
            {
                toDelete_.push_back((*ptr)[i]);
                LOG_DEBUG(<< "Resource toDelete <" << (*ptr)[i].number << ">.");
                (*ptr)[i].number = 0;
            }
        }

        // Remove owner entry:
        //      resources_.removeRow[resourcesIdx_[idx]];
        //      owners_.removeRow[idx];
        //      resourcesIdx_.removeRow[idx];
    }
}

// Remove owner from the manager.
void ViewerOpenGLManager::removeOwner(ViewerOpenGLResource *owner)
{
    size_t idxRes;
    size_t idx;
    size_t i;
    size_t n;

    if (lastAccess_ == owner)
    {
        LOG_DEBUG(<< "Remove owner using cache index <" << lastAccessIdx_
                  << ">.");
        idx = lastAccessIdx_;
    }
    else
    {
        idx = binarySearch(owners_, owner);
    }

    if (idx != SIZE_MAX)
    {
        // Update cache.
        lastAccess_ = nullptr;

        LOG_DEBUG(<< "Remove owner index <" << resourcesIdx_[idx] << ">.");

        // Remove owner entry.
        idxRes = resourcesIdx_[idx];
        removeAt(resources_, idxRes);
        removeAt(owners_, idx);
        removeAt(resourcesIdx_, idx);

        // Shift indices to the resources.
        n = resourcesIdx_.size();
        for (i = 0; i < n; ++i)
        {
            if (resourcesIdx_[i] > idxRes)
            {
                resourcesIdx_[i]--;
            }
        }
    }
    else
    {
        LOG_DEBUG(<< "Remove owner index not found.");
    }
}

// Release all resources from the trash.
void ViewerOpenGLManager::updateResources()
{
    size_t i = 0;
    size_t n = toDelete_.size();

    // if (!models)  {
    //    updateGL ();
    // }
    if (!strokeFont_)
    {
        updateGL();
    }

    if (n > 0)
    {
        for (i = 0; i < n; ++i)
        {
            if (toDelete_[i].number > 0)
            {
                if (toDelete_[i].resource == ViewerOpenGLManager::DisplayList)
                {
                    SAFE_GL(
                        glDeleteLists(toDelete_[i].number, toDelete_[i].range));
                    nDisplayLists_--;
                    LOG_DEBUG(<< "Update resources nLists <" << nDisplayLists_
                              << ">.");
                }
                else if (toDelete_[i].resource == ViewerOpenGLManager::Texture)
                {
                    SAFE_GL(glDeleteTextures(toDelete_[i].range,
                                             &toDelete_[i].number));
                    nTextures_--;
                    LOG_DEBUG(<< "Update resources nTextures <" << nTextures_
                              << ">.");
                }
                else if (toDelete_[i].resource ==
                         ViewerOpenGLManager::BufferARB)
                {
                    ASSERT(glDeleteBuffersARB);
                    SAFE_GL(glDeleteBuffersARB(toDelete_[i].range,
                                               &toDelete_[i].number));
                    nBuffers_--;
                    LOG_DEBUG(<< "Update resources nBuffers <" << nBuffers_
                              << ">.");
                }
                else if (toDelete_[i].resource ==
                         ViewerOpenGLManager::ProgramARB)
                {
                    ASSERT(glDeleteProgramsARB);
                    SAFE_GL(glDeleteProgramsARB(toDelete_[i].range,
                                                &toDelete_[i].number));
                    nPrograms_--;
                    LOG_DEBUG(<< "Update resources nPrograms <" << nPrograms_
                              << ">.");
                }
            }
        }

        toDelete_.resize(0);
    }
}

GLuint ViewerOpenGLManager::font()
{
    GLuint displayList;

    ASSERT(strokeFont_);

    displayList = strokeFont_->list(this);

    return displayList;
}

std::string ViewerOpenGLManager::cleanText(const std::string &text)
{
    std::string cleanText(text);
    for (size_t i = 0; i < cleanText.size(); i++)
    {
        if (cleanText[i] < 32 || cleanText[i] > 125)
        {
            cleanText[i] = ' ';
        }
    }

    return cleanText;
}

float ViewerOpenGLManager::textWidth(const std::string &text)
{
    ASSERT(strokeFont_);

    return strokeFont_->textWidth(text);
}

float ViewerOpenGLManager::textHeight()
{
    ASSERT(strokeFont_);

    return strokeFont_->textHeight();
}

void ViewerOpenGLManager::updateGL()
{
    strokeFont_ = std::make_shared<ViewerOpenGLStrokeFont>();
}
