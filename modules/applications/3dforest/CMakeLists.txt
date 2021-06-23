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

find_package(Qt5 COMPONENTS Core Gui Widgets OpenGL)
if (NOT Qt5_FOUND OR NOT Qt5OpenGL_FOUND)
  message(WARNING "Qt5 not found - skipping 3dforest")
  return()
endif()

find_package(OpenGL QUIET)
if (NOT OpenGL_FOUND)
  message(WARNING "OpenGL not found - skipping 3dforest")
  return()
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# Plugins
set(PLUGIN_HEIGHT_MAP "pluginheightmap")
add_library(
  ${PLUGIN_HEIGHT_MAP}
  SHARED
  plugins/heightmap/PluginHeightMap.cpp
)
target_include_directories(${PLUGIN_HEIGHT_MAP} PUBLIC plugins/heightmap)
target_include_directories(${PLUGIN_HEIGHT_MAP} PUBLIC src)
target_link_libraries(${PLUGIN_HEIGHT_MAP} PRIVATE Qt5::Widgets visualization)

set(PLUGIN_DATABASE_STATISTICS "plugindatabasestatistics")
add_library(
  ${PLUGIN_DATABASE_STATISTICS}
  SHARED
  plugins/databasestatistics/PluginDatabaseStatistics.cpp
)
target_include_directories(${PLUGIN_DATABASE_STATISTICS} PUBLIC plugins/databasestatistics)
target_include_directories(${PLUGIN_DATABASE_STATISTICS} PUBLIC src)
target_link_libraries(${PLUGIN_DATABASE_STATISTICS} PRIVATE Qt5::Widgets visualization)

# Application
set(SUB_PROJECT_NAME "3dforest")
file(GLOB_RECURSE SOURCES_3DFOREST "src/*.cpp")
list(APPEND SOURCES_3DFOREST resources/3dforest.rc)

add_executable(
  ${SUB_PROJECT_NAME}
  WIN32 MACOSX_BUNDLE
  ${SOURCES_3DFOREST}
  resources/3dforest.qrc
)

target_include_directories(${SUB_PROJECT_NAME} PUBLIC src)

target_link_libraries(
  ${SUB_PROJECT_NAME}
  visualization
)

# Install
install(TARGETS ${SUB_PROJECT_NAME} DESTINATION bin)
install(TARGETS ${PLUGIN_HEIGHT_MAP} DESTINATION bin/plugins)
install(TARGETS ${PLUGIN_DATABASE_STATISTICS} DESTINATION bin/plugins)