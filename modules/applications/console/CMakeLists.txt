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

add_executable(las src/las.cpp)
target_link_libraries(las PUBLIC core)
install(TARGETS las DESTINATION bin)

add_executable(sandbox src/sandbox.cpp)
target_link_libraries(sandbox PUBLIC core editor)
install(TARGETS sandbox DESTINATION bin)

add_executable(classify src/classify.cpp)
target_link_libraries(classify PUBLIC editor)
install(TARGETS classify DESTINATION bin)
