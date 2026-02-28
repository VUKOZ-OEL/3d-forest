# Copyright 2020 VUKOZ
#
# This file is part of 3D Forest.
#
# 3D Forest is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# 3D Forest is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with 3D Forest.  If not, see <https://www.gnu.org/licenses/>.

cmake_minimum_required(VERSION 3.10)

# Declare the CMake project name and version (packaging, install rules, etc.)
project(3DForest VERSION 1.0)

# Default to Debug builds if the user didn't specify a build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# Project-specific compiler/language flags
include(cmake/cxxflags.cmake)

# Always require threading support (e.g., pthread on Linux)
find_package(Threads REQUIRED)

# Try to locate system OpenGL
find_package(OpenGL QUIET)

# If set, prefer Qt5 even if Qt6 is available on the system
if(FORCE_QT5)
    # Explicitly search for Qt5 modules used by the GUI
    find_package(Qt5 COMPONENTS Core Gui Widgets OpenGL)

    if (Qt5_FOUND AND Qt5OpenGL_FOUND AND OpenGL_FOUND)
        message(STATUS "Gui with Qt5 and OpenGL found")
        set(QT_VERSION_MAJOR 5)
    endif()
else()
    # Default behavior: try Qt6 first, then fall back to Qt5
    find_package(Qt6 QUIET COMPONENTS Core Gui Widgets)

    if(Qt6_FOUND)
        find_package(Qt6 QUIET
            COMPONENTS Core Gui Widgets
            OPTIONAL_COMPONENTS
                Network
                WebEngineWidgets
                OpenGL
                OpenGLWidgets
                Concurrent
                Qml
                Sql
                Xml
        )
    endif()

    if(Qt6_FOUND AND Qt6OpenGL_FOUND AND OpenGL_FOUND)
        message(STATUS "Gui with Qt6 and OpenGL found")
        set(QT_VERSION_MAJOR 6)
    else()
        find_package(Qt5 COMPONENTS Core Gui Widgets OpenGL QUIET)
        if (Qt5_FOUND AND Qt5OpenGL_FOUND AND OpenGL_FOUND)
            message(STATUS "Gui with Qt5 and OpenGL found")
            set(QT_VERSION_MAJOR 5)
        else()
            message(STATUS "Qt library for Gui not found")
        endif()
    endif()
endif()

# Helper to:
#  1) install an app target, and
#  2) generate/install a Qt deployment script that copies required Qt runtime deps
function(install_and_deploy_qt_runtime app_target bin_dest)
  install(TARGETS ${app_target}
    RUNTIME DESTINATION ${bin_dest}
    BUNDLE  DESTINATION ${bin_dest}
  )

  qt_generate_deploy_app_script(
    TARGET ${app_target}
    OUTPUT_SCRIPT ${app_target}_deploy.cmake
    NO_UNSUPPORTED_PLATFORM_ERROR
  )
  install(SCRIPT ${${app_target}_deploy.cmake})
endfunction()

# Build third-party dependencies first (libraries, externals)
add_subdirectory(3rdparty)

# Build the project sources (main targets)
add_subdirectory(src)

# Developer tooling integrations: formatting, documentation generation
include(cmake/clangformat.cmake)
include(cmake/doxygen.cmake)
