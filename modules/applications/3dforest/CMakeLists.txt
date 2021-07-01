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

find_package(Qt5 COMPONENTS Core Gui Widgets Help OpenGL)
if (NOT Qt5_FOUND OR NOT Qt5OpenGL_FOUND)
  message(WARNING "Qt5 not found - skipping 3dforest")
  return()
endif()

find_package(OpenGL QUIET)
if (NOT OpenGL_FOUND)
  message(WARNING "OpenGL not found - skipping 3dforest")
  return()
endif()

add_subdirectory(application)
add_subdirectory(plugins)