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

/** @file ImportFilePlugin.hpp */

#ifndef IMPORT_FILE_PLUGIN_HPP
#define IMPORT_FILE_PLUGIN_HPP

// Include 3D Forest.
#include <ImportFileInterface.hpp>
#include <Plugin.hpp>

#if defined(_MSC_VER)
    #if defined(EXPORT_3DForestImportFilePlugin)
        #define EXPORT_IMPORT_FILE_PLUGIN __declspec(dllexport)
    #else
        #define EXPORT_IMPORT_FILE_PLUGIN __declspec(dllimport)
    #endif
#else
    #define EXPORT_IMPORT_FILE_PLUGIN
#endif

/** Import File Plugin. */
class ImportFilePlugin : public Plugin, public ImportFileInterface
{
public:
    ImportFilePlugin();

    const char *name() const override { return "ImportFilePlugin"; }
    void initialize(Application *app) override;
    void release() override { delete this; }

    virtual void importFile();

    void slotImportFile();

private:
    Application *app_;
    Action *importFileAction_;
};

extern "C" EXPORT_IMPORT_FILE_PLUGIN Plugin *createPlugin()
{
    return new ImportFilePlugin();
}

#endif /* IMPORT_FILE_PLUGIN_HPP */
