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

set(SUB_PROJECT_NAME "3DForestExternal3dMarteloscopePlugin")

if(NOT QT_VERSION_MAJOR)
    message(WARNING "Gui not found - skipping ${SUB_PROJECT_NAME}")
    return()
endif()

if(NOT TARGET Qt${QT_VERSION_MAJOR}::Network)
    message(WARNING "Skipping ${SUB_PROJECT_NAME} (Qt Network not available)")
    return()
endif()

if(NOT TARGET Qt${QT_VERSION_MAJOR}::WebEngineWidgets)
    message(WARNING "Skipping ${SUB_PROJECT_NAME} (Qt WebEngine not available)")
    return()
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

file(GLOB_RECURSE SOURCES "*.cpp")

add_library(
    ${SUB_PROJECT_NAME}
    SHARED
    ${SOURCES}
    resources/resources.qrc
)

target_include_directories(
    ${SUB_PROJECT_NAME}
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
)

target_compile_definitions(
    ${SUB_PROJECT_NAME}
    PRIVATE
    EXPORT_${SUB_PROJECT_NAME}=1
)

# if(TARGET Qt${QT_VERSION_MAJOR}::WebEngineWidgets)
#     target_compile_definitions(
#         ${SUB_PROJECT_NAME}
#         PRIVATE
#         HAS_QT_WEB_ENGINE_WIDGETS=1
#     )
# endif()

target_link_libraries(
    ${SUB_PROJECT_NAME}
    PRIVATE
    3DForestGui
    Qt${QT_VERSION_MAJOR}::Network
    Qt${QT_VERSION_MAJOR}::WebEngineWidgets
)

# if(TARGET Qt${QT_VERSION_MAJOR}::WebEngineWidgets)
#     target_link_libraries(
#         ${SUB_PROJECT_NAME}
#         PRIVATE
#         Qt${QT_VERSION_MAJOR}::WebEngineWidgets
#     )
# endif()

# Install plugin
# install(TARGETS ${SUB_PROJECT_NAME} DESTINATION bin/plugins)
install_and_deploy_qt_runtime(${SUB_PROJECT_NAME} "bin/plugins")

# Install python app
install(
    DIRECTORY python
    DESTINATION bin/plugins/${SUB_PROJECT_NAME}
)
