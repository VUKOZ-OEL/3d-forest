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
set(PLUGIN_EXAMPLE "plugintoolexample")
add_library(
  ${PLUGIN_EXAMPLE}
  SHARED
  plugins/toolExample/Forest3dPluginToolExample.cpp
  plugins/toolExample/Forest3dPluginToolExampleWindow.cpp
)
target_include_directories(${PLUGIN_EXAMPLE} PUBLIC plugins/toolExample)
target_include_directories(${PLUGIN_EXAMPLE} PUBLIC src)
target_link_libraries(${PLUGIN_EXAMPLE} PRIVATE Qt5::Widgets visualization)

# Application
set(SUB_PROJECT_NAME "3dforest")
file(GLOB_RECURSE SOURCES_3DFOREST "src/*.cpp")
list(APPEND SOURCES_3DFOREST resources/3dforest.rc)

add_executable(
  ${SUB_PROJECT_NAME}
#  WIN32
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
install(TARGETS ${PLUGIN_EXAMPLE} DESTINATION bin)