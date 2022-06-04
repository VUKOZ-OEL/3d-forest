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

if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

include(cmake/cxxflags.cmake)

if (MSVC)
  set(CMAKE_PREFIX_PATH "C:\\Qt\\6.3.0\\msvc2019_64")
endif()

find_package(OpenGL QUIET)
find_package(Qt6 COMPONENTS Core Gui Widgets Help OpenGL OpenGLWidgets)
if (Qt6_FOUND AND Qt6OpenGL_FOUND AND OpenGL_FOUND)
  message(STATUS "Gui with Qt6 and OpenGL found")
  set(QT_VERSION_MAJOR 6)
else()
  find_package(Qt5 COMPONENTS Core Gui Widgets Help OpenGL)
  if (Qt5_FOUND AND Qt5OpenGL_FOUND AND OpenGL_FOUND)
    message(STATUS "Gui with Qt5 and OpenGL found")
    set(QT_VERSION_MAJOR 5)
  endif()
endif()

add_subdirectory(src)

include(cmake/clangformat.cmake)
include(cmake/doxygen.cmake)