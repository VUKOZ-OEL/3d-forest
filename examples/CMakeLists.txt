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

add_executable(examplelascreate core/examplelascreate.cpp)
target_link_libraries(examplelascreate PUBLIC core)
install(TARGETS examplelascreate DESTINATION bin)

add_executable(examplevector3 core/examplevector3.cpp)
target_link_libraries(examplevector3 PUBLIC core)
set_target_properties(
  examplevector3
  PROPERTIES
  COMPILE_FLAGS
  "-save-temps -fverbose-asm"
)
install(TARGETS examplevector3 DESTINATION bin)

add_executable(examplecolorpalette core/examplecolorpalette.cpp)
target_link_libraries(examplecolorpalette PUBLIC core)
install(TARGETS examplecolorpalette DESTINATION bin)

add_executable(exampleeditor editor/exampleeditor.cpp)
target_link_libraries(exampleeditor PUBLIC editor)
install(TARGETS exampleeditor DESTINATION bin)
