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

/** @file FilterAreaPlugin.hpp */

#ifndef FILTER_AREA_PLUGIN_HPP
#define FILTER_AREA_PLUGIN_HPP

// Include 3D Forest.
#include <Plugin.hpp>
class FilterAreaWindow;

#if defined(_MSC_VER)
    #if defined(EXPORT_3DForestFilterAreaPlugin)
        #define EXPORT_FILTER_AREA_PLUGIN __declspec(dllexport)
    #else
        #define EXPORT_FILTER_AREA_PLUGIN __declspec(dllimport)
    #endif
#else
    #define EXPORT_FILTER_AREA_PLUGIN
#endif

/** Filter Area Plugin. */
class FilterAreaPlugin : 
                                                   public Plugin
{
public:
    FilterAreaPlugin();

    const char *name() const override { return "FilterAreaPlugin"; }
    void initialize(Application *app) override;
    void release() override { delete this; }

    void slotPlugin();

private:
    Application *app_;
    FilterAreaWindow *pluginWindow_;
};

extern "C" EXPORT_FILTER_AREA_PLUGIN Plugin *createPlugin()
{
    return new FilterAreaPlugin();
}

#endif /* FILTER_AREA_PLUGIN_HPP */
