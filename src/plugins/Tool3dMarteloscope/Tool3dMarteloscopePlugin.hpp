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

/** @file Tool3dMarteloscopePlugin.hpp */

#ifndef TOOL_3D_MARTELOSCOPE_PLUGIN_HPP
#define TOOL_3D_MARTELOSCOPE_PLUGIN_HPP

// Include 3D Forest.
#include <Plugin.hpp>

class Tool3dMarteloscopeRunner;

// Include std.
#include <memory>

#if defined(_MSC_VER)
    #if defined(EXPORT_3DForestTool3dMarteloscopePlugin)
        #define EXPORT_TOOL_3D_MARTELOSCOPE_PLUGIN __declspec(dllexport)
    #else
        #define EXPORT_TOOL_3D_MARTELOSCOPE_PLUGIN __declspec(dllimport)
    #endif
#else
    #define EXPORT_TOOL_3D_MARTELOSCOPE_PLUGIN
#endif

/** Tool 3d-Marteloscope Plugin. */
class Tool3dMarteloscopePlugin : public Plugin
{
public:
    const char *name() const override { return "Tool3dMarteloscopePlugin"; }
    void initialize(Application *app) override;
    void release() override { delete this; }

    void slotPlugin();

private:
    Application *app_{nullptr};
    std::unique_ptr<Tool3dMarteloscopeRunner> runner_;
};

extern "C" EXPORT_TOOL_3D_MARTELOSCOPE_PLUGIN Plugin *createPlugin()
{
    return new Tool3dMarteloscopePlugin();
}

#endif /* TOOL_3D_MARTELOSCOPE_PLUGIN_HPP */
