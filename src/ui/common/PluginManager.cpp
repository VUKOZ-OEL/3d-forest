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

/** @file PluginManager.cpp */

// Include std.
#if defined(_MSC_VER)
    #include <windows.h>
#else
    #include <dlfcn.h>
#endif

// Include 3D Forest.
#include <Application.hpp>
#include <File.hpp>
#include <PluginManager.hpp>

// Include local.
#define LOG_MODULE_NAME "PluginManager"
#define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

PluginManager::PluginManager()
{
}

PluginManager::~PluginManager()
{
    unload();
}

static void pluginManagerUnLoad(PluginHandle &pluginHandle)
{
    if (pluginHandle.plugin)
    {
        pluginHandle.plugin->release();
        pluginHandle.plugin = nullptr;
    }

#if defined(_MSC_VER)
    if (pluginHandle.handle)
    {
        FreeLibrary(pluginHandle.handle);
        pluginHandle.handle = nullptr;
    }
#else
    if (pluginHandle.handle)
    {
        dlclose(pluginHandle.handle);
        pluginHandle.handle = nullptr;
    }
#endif
}

static PluginHandle pluginManagerLoad(const std::string &fileName)
{
    PluginHandle result;

    using CreatePluginFn = Plugin *(*)();

#if defined(_MSC_VER)
    result.handle = LoadLibraryA(fileName.c_str());
    if (!result.handle)
    {
        LOG_WARNING(<< "Load library failed.");
        return result;
    }

    auto create = reinterpret_cast<CreatePluginFn>(
        GetProcAddress(result.handle, "createPlugin"));
#else
    result.handle = dlopen(fileName.c_str(), RTLD_NOW);
    if (!result.handle)
    {
        LOG_WARNING(<< "Load library failed.");
        return result;
    }

    auto create =
        reinterpret_cast<CreatePluginFn>(dlsym(result.handle, "createPlugin"));
#endif

    if (!create)
    {
        LOG_WARNING(<< "Load library failed: missing createPlugin.");
        pluginManagerUnLoad(result);
        return result;
    }

    result.plugin = create();

    return result;
}

void PluginManager::load(Application *app)
{
    std::string dirPath = File::currentPath() + "plugins/";

#if defined(_MSC_VER)
    std::string pattern = "*.dll";
#else
    std::string pattern = "*.so";
#endif

    LOG_DEBUG(<< "Load plugins from directory <" << dirPath << ">.");

    std::vector<std::string> fileNames = File::listFiles(dirPath, pattern);

    LOG_DEBUG(<< "Found number of files <" << fileNames.size() << ">.");

    for (const auto &fileName : fileNames)
    {
        std::string path = dirPath + fileName;

        LOG_DEBUG(<< "Load file path <" << path << ">.");

        PluginHandle handle = pluginManagerLoad(path);
        if (handle.plugin)
        {
            handle.plugin->initialize(app);
            plugins_.push_back(handle);
        }
    }
}

void PluginManager::unload()
{
    for (auto &it : plugins_)
    {
        pluginManagerUnLoad(it);
    }

    plugins_.clear();
}
