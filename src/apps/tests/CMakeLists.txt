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

if(NOT BUILD_TESTS)
    message(STATUS "BUILD_TESTS not set - skipping tests")
    return()
endif()

set(SUB_PROJECT_NAME "3DForestTest")

file(GLOB_RECURSE SOURCES "*.cpp")
file(GLOB_RECURSE SOURCES_CORE "../../core/tests/*.cpp")
file(GLOB_RECURSE SOURCES_EDITOR "../../editor/tests/*.cpp")
file(GLOB_RECURSE SOURCES_PLUGIN_DESCRIPTOR "../../plugins/descriptor/tests/*.cpp")

add_executable(
    ${SUB_PROJECT_NAME}
    ${SOURCES}
    ${SOURCES_CORE}
    ${SOURCES_EDITOR}
    ${SOURCES_PLUGIN_DESCRIPTOR}
    ../../plugins/descriptor/DescriptorAction.cpp
    ../../plugins/descriptor/DescriptorPca.cpp
)

target_include_directories(
    ${SUB_PROJECT_NAME}
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
    ../../plugins/descriptor
    ../../../3rdparty/eigen
    ../../../3rdparty/unibnoctree
)

target_link_libraries(
    ${SUB_PROJECT_NAME}
    PUBLIC
    3DForestEditor
)

install(TARGETS ${SUB_PROJECT_NAME} DESTINATION bin)