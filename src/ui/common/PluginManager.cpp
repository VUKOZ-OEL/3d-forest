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

#include <Platform.hpp>

// Include std.
#if defined(PLATFORM_WINDOWS)
    #define NOMINMAX
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

static void *pluginManagerOpenLibrary(const std::string &fileName)
{
#if defined(PLATFORM_WINDOWS)
    return reinterpret_cast<void *>(LoadLibraryA(fileName.c_str()));
#else
    return dlopen(fileName.c_str(), RTLD_NOW);
#endif
}

static void pluginManagerCloseLibrary(void *handle)
{
#if defined(PLATFORM_WINDOWS)
    FreeLibrary(reinterpret_cast<HMODULE>(handle));
#else
    dlclose(handle);
#endif
}

static void *pluginManagerLoadSymbol(void *handle, const char *name)
{
#if defined(PLATFORM_WINDOWS)
    return reinterpret_cast<void *>(
        GetProcAddress(reinterpret_cast<HMODULE>(handle), name));
#else
    return dlsym(handle, name);
#endif
}

static void pluginManagerUnLoad(PluginHandle &pluginHandle)
{
    if (pluginHandle.plugin)
    {
        pluginHandle.plugin->release();
        pluginHandle.plugin = nullptr;
    }

    if (pluginHandle.handle)
    {
        pluginManagerCloseLibrary(pluginHandle.handle);
        pluginHandle.handle = nullptr;
    }
}

static PluginHandle pluginManagerLoad(const std::string &fileName)
{
    PluginHandle result;

    using CreatePluginFn = Plugin *(*)();

    result.handle = pluginManagerOpenLibrary(fileName);
    if (!result.handle)
    {
        LOG_WARNING(<< "Load library failed.");
        return result;
    }

    auto create = reinterpret_cast<CreatePluginFn>(
        pluginManagerLoadSymbol(result.handle, "createPlugin"));
    if (!create)
    {
        LOG_WARNING(<< "Load library failed: missing createPlugin.");
        pluginManagerUnLoad(result);
        return result;
    }

    result.plugin = create();

    return result;
}

PluginManager::PluginManager()
{
}

PluginManager::~PluginManager()
{
    unload();
}

void PluginManager::load(Application *app)
{
    std::string dirPath = File::currentPath() + "plugins/";

#if defined(PLATFORM_WINDOWS)
    std::string pattern = "*.dll";
#elif defined(PLATFORM_MACOS)
    std::string pattern = "*.dylib";
#else
    std::string pattern = "*.so";
#endif

    LOG_DEBUG(<< "Load plugins from directory <" << dirPath << ">.");

    std::vector<std::string> fileNames = File::listFiles(dirPath, pattern);

    LOG_DEBUG(<< "Found number of files <" << fileNames.size() << ">.");

    for (const auto &fileName : fileNames)
    {
        std::string path = dirPath + fileName;
        load(app, path);
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

void PluginManager::load(Application *app, const std::string &fileName)
{
    LOG_DEBUG(<< "Load file path <" << fileName << ">.");

    PluginHandle handle = pluginManagerLoad(fileName);
    if (!handle.plugin)
    {
        return;
    }

    Plugin *pluginInterface;
    pluginInterface = dynamic_cast<Plugin *>(handle.plugin);
    if (!pluginInterface)
    {
        LOG_DEBUG(<< "Plugin interface not recognized.");
        return;
    }

    handle.plugin->initialize(app);
    plugins_.push_back(handle);

#if 0
    // Modifier.
    ModifierInterface *modifierInterface;
    modifierInterface = dynamic_cast<ModifierInterface *>(pluginInterface);
    if (modifierInterface)
    {
        LOG_DEBUG(<< "Add modifier plugin.");
        editor_.addModifier(modifierInterface);
    }
#endif
    // Project file.
    ProjectFileInterface *projectFileInterface;
    projectFileInterface =
        dynamic_cast<ProjectFileInterface *>(pluginInterface);
    if (projectFileInterface)
    {
        LOG_DEBUG(<< "Set project file plugin.");
        projectFilePlugin_ = projectFileInterface;
    }
#if 0
    // Import file.
    ImportFileInterface *importFileInterface;
    importFileInterface = dynamic_cast<ImportFileInterface *>(pluginInterface);
    if (importFileInterface)
    {
        LOG_DEBUG(<< "Set import file plugin.");
        importFilePlugin_ = importFileInterface;
    }

    // Viewer.
    ViewerInterface *viewerInterface;
    viewerInterface = dynamic_cast<ViewerInterface *>(pluginInterface);
    if (viewerInterface)
    {
        LOG_DEBUG(<< "Set viewer plugin.");
        viewerPlugin_ = viewerInterface;
    }
#endif
}
