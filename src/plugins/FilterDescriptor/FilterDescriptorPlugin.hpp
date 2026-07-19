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

/** @file FilterDescriptorPlugin.hpp */

#ifndef FILTER_DESCRIPTOR_PLUGIN_HPP
#define FILTER_DESCRIPTOR_PLUGIN_HPP

// Include 3D Forest.
#include <Plugin.hpp>
class FilterDescriptorWindow;

#if defined(_MSC_VER)
    #if defined(EXPORT_3DForestFilterDescriptorPlugin)
        #define EXPORT_FILTER_DESCRIPTOR_PLUGIN __declspec(dllexport)
    #else
        #define EXPORT_FILTER_DESCRIPTOR_PLUGIN __declspec(dllimport)
    #endif
#else
    #define EXPORT_FILTER_DESCRIPTOR_PLUGIN
#endif

/** Filter Descriptor Plugin. */
class FilterDescriptorPlugin : public Plugin
{
public:
    FilterDescriptorPlugin();

    const char *name() const override { return "FilterDescriptorPlugin"; }
    void initialize(Application *app) override;
    void release() override { delete this; }

    void slotPlugin();

private:
    Application *app_;
    FilterDescriptorWindow *pluginWindow_;
};

extern "C" EXPORT_FILTER_DESCRIPTOR_PLUGIN Plugin *createPlugin()
{
    return new FilterDescriptorPlugin();
}

#endif /* FILTER_DESCRIPTOR_PLUGIN_HPP */
