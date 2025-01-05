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

project(3DForest VERSION 1.0)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

include(cmake/cxxflags.cmake)

find_package(Threads REQUIRED)

if(MSVC)
    # Lazy fix for Windows issue with dual mingw + msvc environment
    # Microsoft Visual Studio insists on using GNU MinGW Qt
    # Should be set when running cmake or by ordering env variables
    set(CMAKE_PREFIX_PATH "C:\\Qt\\6.8.1\\msvc2022_64")
endif()

find_package(OpenGL QUIET)

if(FORCE_QT5)
    # When CMake would otherwise autodetect Qt6
    find_package(Qt5 COMPONENTS Core Gui Widgets OpenGL)
    if (Qt5_FOUND AND Qt5OpenGL_FOUND AND OpenGL_FOUND)
        message(STATUS "Gui with Qt5 and OpenGL found")
        set(QT_VERSION_MAJOR 5)
    endif()
else()
    # Default autodetect Qt version
    find_package(Qt6 COMPONENTS Core Gui Widgets OpenGL OpenGLWidgets QUIET)
    if(Qt6_FOUND AND Qt6OpenGL_FOUND AND OpenGL_FOUND)
        message(STATUS "Gui with Qt6 and OpenGL found")
        set(QT_VERSION_MAJOR 6)
    else()
        find_package(Qt5 COMPONENTS Core Gui Widgets OpenGL QUIET)
        if (Qt5_FOUND AND Qt5OpenGL_FOUND AND OpenGL_FOUND)
            message(STATUS "Gui with Qt5 and OpenGL found")
            set(QT_VERSION_MAJOR 5)
        endif()
    endif()
endif()

add_subdirectory(src)

include(cmake/clangformat.cmake)
include(cmake/doxygen.cmake)
