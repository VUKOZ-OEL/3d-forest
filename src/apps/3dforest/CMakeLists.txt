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

set(SUB_PROJECT_NAME "3DForest")

if(NOT QT_VERSION_MAJOR)
    message(WARNING "Gui not found - skipping ${SUB_PROJECT_NAME}")
    return()
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

file(GLOB_RECURSE SOURCES "*.cpp")
list(APPEND SOURCES resources/resources.rc)

include(../../../cmake/githash.cmake)

add_executable(
    ${SUB_PROJECT_NAME}
    WIN32 MACOSX_BUNDLE
    ${SOURCES}
    resources/resources.qrc
)

target_include_directories(
    ${SUB_PROJECT_NAME}
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(
    ${SUB_PROJECT_NAME}
    3DForestGui
#    Qt${QT_VERSION_MAJOR}::Help
)

install_and_deploy_qt_runtime(${SUB_PROJECT_NAME} "bin")