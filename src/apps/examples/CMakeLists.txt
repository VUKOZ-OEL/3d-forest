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

if(NOT BUILD_EXAMPLES)
    message(STATUS "BUILD_EXAMPLES not set - skipping examples")
    return()
endif()

add_subdirectory(colorpalette)
add_subdirectory(editor)
add_subdirectory(eigen)
add_subdirectory(file)
add_subdirectory(filesize)
add_subdirectory(image)
add_subdirectory(lascreate)
#add_subdirectory(median)
add_subdirectory(meshdistance)
#add_subdirectory(pca)
add_subdirectory(query)
add_subdirectory(queryfilter)
add_subdirectory(queryvoxels)
add_subdirectory(thread)
add_subdirectory(vector3)
#add_subdirectory(voxels)
