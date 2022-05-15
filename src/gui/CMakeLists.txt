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

set(SUB_PROJECT_NAME "3dforestgui")

if (NOT CMAKE_3DFOREST_GUI_FOUND)
  message(WARNING "Gui not found - skipping ${SUB_PROJECT_NAME}")
  return()
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

file(GLOB_RECURSE SOURCES "*.cpp")
file(GLOB_RECURSE SOURCES_IMPORT "../plugins/import/*.cpp")
file(GLOB_RECURSE SOURCES_PROJECT "../plugins/project/*.cpp")
file(GLOB_RECURSE SOURCES_PROJECT_FILE "../plugins/projectfile/*.cpp")
file(GLOB_RECURSE SOURCES_VIEWER "../plugins/viewer/*.cpp")

add_library(
  ${SUB_PROJECT_NAME}
  SHARED
  ${SOURCES}
  ${SOURCES_IMPORT}
  ${SOURCES_PROJECT}
  ${SOURCES_PROJECT_FILE}
  ${SOURCES_VIEWER}
  resources/resources.qrc
  ../plugins/import/resources/resources.qrc
  ../plugins/project/resources.qrc
  ../plugins/projectfile/resources/resources.qrc
  ../plugins/viewer/resources/resources.qrc
)

target_include_directories(
  ${SUB_PROJECT_NAME}
  PUBLIC
  ${CMAKE_CURRENT_LIST_DIR}
  ../plugins/import
  ../plugins/project
  ../plugins/projectfile
  ../plugins/viewer
)

include_directories(${Qt5Widgets_INCLUDE_DIRS})
add_definitions(${Qt5Widgets_DEFINITIONS})

include_directories(${Qt5OpenGL_INCLUDE_DIRS})
add_definitions(${Qt5OpenGL_DEFINITIONS})

target_link_libraries(
  ${SUB_PROJECT_NAME}
  3dforesteditor
  Qt5::Core
  Qt5::Gui
  Qt5::Widgets
  Qt5::OpenGL
  ${OPENGL_gl_LIBRARY}
)

install(TARGETS ${SUB_PROJECT_NAME} DESTINATION bin)