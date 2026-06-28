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

/** @file ProjectFilePlugin.hpp */

#ifndef PROJECT_FILE_PLUGIN_HPP
#define PROJECT_FILE_PLUGIN_HPP

// Include 3D Forest.
#include <Plugin.hpp>
#include <ProjectFileInterface.hpp>

class Action;

#if defined(_MSC_VER)
    #if defined(EXPORT_3DForestProjectFilePlugin)
        #define EXPORT_PROJECT_FILE_PLUGIN __declspec(dllexport)
    #else
        #define EXPORT_PROJECT_FILE_PLUGIN __declspec(dllimport)
    #endif
#else
    #define EXPORT_PROJECT_FILE_PLUGIN
#endif

/** Project File Plugin. */
class ProjectFilePlugin : public Plugin, public ProjectFileInterface
{
public:
    ProjectFilePlugin();

    const char *name() const override { return "ProjectFilePlugin"; }
    void initialize(Application *app) override;
    void release() override { delete this; }

    virtual bool closeProject();

    void slotNewProject();
    void slotOpenProject();
    void slotSaveProject();
    void slotSaveAsProject();
    void slotReloadProject();

private:
    Application *app_{nullptr};

    Action *newProjectAction_;
    Action *openProjectAction_;
    Action *saveProjectAction_;
    Action *saveAsProjectAction_;
    Action *reloadProjectAction_;
};

extern "C" EXPORT_PROJECT_FILE_PLUGIN Plugin *createPlugin()
{
    return new ProjectFilePlugin();
}

#endif /* PROJECT_FILE_PLUGIN_HPP */
