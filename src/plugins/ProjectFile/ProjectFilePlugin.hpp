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
#include <PluginInterface.hpp>
#include <ProjectFileInterface.hpp>

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
class EXPORT_PROJECT_FILE_PLUGIN ProjectFilePlugin : public QObject,
                                                     public PluginInterface,
                                                     public ProjectFileInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID PluginInterface_iid)
    Q_INTERFACES(PluginInterface)

public:
    ProjectFilePlugin();

    virtual void initialize(MainWindow *mainWindow);

    virtual bool closeProject();

public slots:
    void slotNewProject();
    void slotOpenProject();
    void slotSaveProject();
    void slotSaveAsProject();

private:
    bool openProject(const QString &path);
    bool saveProject(const QString &path = "");

    MainWindow *mainWindow_;

    QAction *newProjectAction_;
    QAction *openProjectAction_;
    QAction *saveProjectAction_;
    QAction *saveAsProjectAction_;
};

#endif /* PROJECT_FILE_PLUGIN_HPP */
