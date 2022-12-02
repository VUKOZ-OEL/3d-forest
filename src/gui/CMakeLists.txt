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

set(SUB_PROJECT_NAME "3DForestGui")

if(NOT QT_VERSION_MAJOR)
    message(WARNING "Gui not found - skipping ${SUB_PROJECT_NAME}")
    return()
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

file(GLOB_RECURSE SOURCES "*.cpp")
file(GLOB_RECURSE SOURCES_IMPORT "../plugins/import/*.cpp")
file(GLOB_RECURSE SOURCES_EXPORT "../plugins/export/*.cpp")
file(GLOB_RECURSE SOURCES_PROJECT "../plugins/projectnavigator/*.cpp")
file(GLOB_RECURSE SOURCES_PROJECT_FILE "../plugins/projectfile/*.cpp")
file(GLOB_RECURSE SOURCES_SETTINGS "../plugins/settings/*.cpp")
file(GLOB_RECURSE SOURCES_VIEWER "../plugins/viewer/*.cpp")
file(GLOB_RECURSE SOURCES_HELP "../plugins/help/*.cpp")

add_library(
    ${SUB_PROJECT_NAME}
    SHARED
    ${SOURCES}
    ${SOURCES_IMPORT}
    ${SOURCES_EXPORT}
    ${SOURCES_PROJECT}
    ${SOURCES_PROJECT_FILE}
    ${SOURCES_SETTINGS}
    ${SOURCES_VIEWER}
    ${SOURCES_HELP}
    ../../3rdparty/widgets/ctkrangeslider/ctkrangeslider.cpp
    ../../3rdparty/widgets/flowlayout/flowlayout.cpp
    resources/resources.qrc
    ../plugins/import/resources/resources.qrc
    ../plugins/export/resources/resources.qrc
    ../plugins/projectnavigator/resources/resources.qrc
    ../plugins/projectfile/resources/resources.qrc
    ../plugins/settings/resources/resources.qrc
    ../plugins/viewer/resources/resources.qrc
    ../plugins/help/resources/resources.qrc
)

target_include_directories(
    ${SUB_PROJECT_NAME}
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
    ../../3rdparty/widgets/ctkrangeslider
    ../../3rdparty/widgets/flowlayout
    ../plugins/import
    ../plugins/export
    ../plugins/projectnavigator
    ../plugins/projectfile
    ../plugins/settings
    ../plugins/viewer
    ../plugins/help
)

target_compile_definitions(
    ${SUB_PROJECT_NAME}
    PRIVATE
    EXPORT_${SUB_PROJECT_NAME}=1
)

if(QT_VERSION_MAJOR EQUAL 5)
    include_directories(${Qt5Widgets_INCLUDE_DIRS})
    include_directories(${Qt5OpenGL_INCLUDE_DIRS})

    add_definitions(${Qt5Widgets_DEFINITIONS})
    add_definitions(${Qt5OpenGL_DEFINITIONS})

    target_link_libraries(
        ${SUB_PROJECT_NAME}
        3DForestEditor
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::OpenGL
        ${OPENGL_gl_LIBRARY}
    )
else()
    include_directories(${Qt6OpenGLWidgets_INCLUDE_DIRS})
    add_definitions(${Qt6OpenGLWidgets_DEFINITIONS})

    target_link_libraries(
        ${SUB_PROJECT_NAME}
        3DForestEditor
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::OpenGL
        Qt${QT_VERSION_MAJOR}::OpenGLWidgets
        ${OPENGL_gl_LIBRARY}
    )
endif()

install(TARGETS ${SUB_PROJECT_NAME} DESTINATION bin)