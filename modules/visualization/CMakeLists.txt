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
  message(WARNING "Qt5 not found - skipping visualization")
  return()
endif()

find_package(OpenGL QUIET)
if (NOT OpenGL_FOUND)
  message(WARNING "OpenGL not found - skipping visualization")
  return()
endif()

set(SUB_PROJECT_NAME "visualization")
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

file(GLOB_RECURSE SOURCES_VISUALIZATION "src/*.cpp")

add_library(${SUB_PROJECT_NAME} SHARED ${SOURCES_VISUALIZATION})

target_include_directories(${SUB_PROJECT_NAME} PUBLIC src/editor)
target_include_directories(${SUB_PROJECT_NAME} PUBLIC src/opengl)
target_include_directories(${SUB_PROJECT_NAME} PUBLIC src/widgets)

include_directories(${Qt5Widgets_INCLUDE_DIRS})
add_definitions(${Qt5Widgets_DEFINITIONS})

include_directories(${Qt5OpenGL_INCLUDE_DIRS})
add_definitions(${Qt5OpenGL_DEFINITIONS})

target_link_libraries(
  ${SUB_PROJECT_NAME}
  Qt5::Core
  Qt5::Gui
  Qt5::Widgets
  Qt5::OpenGL
  ${OPENGL_gl_LIBRARY}
  project
)

install(TARGETS ${SUB_PROJECT_NAME} DESTINATION bin)