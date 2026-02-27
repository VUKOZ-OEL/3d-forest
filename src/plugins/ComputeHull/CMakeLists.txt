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

set(SUB_PROJECT_NAME "3DForestComputeHullPlugin")

if(NOT QT_VERSION_MAJOR)
    message(WARNING "Gui not found - skipping ${SUB_PROJECT_NAME}")
    return()
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

file(GLOB_RECURSE SOURCES "*.cpp")

add_library(
    ${SUB_PROJECT_NAME}
    SHARED
    ${SOURCES}
    ../../../3rdparty/cdt/CDT.cpp
    resources/resources.qrc
)

target_include_directories(
    ${SUB_PROJECT_NAME}
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
    ../../../3rdparty/cdt
    ../../../3rdparty/convhull_3d
    ../../../3rdparty/boost
    ../../../3rdparty/cgal/Algebraic_foundations/include
    ../../../3rdparty/cgal/Alpha_shapes_2/include
    ../../../3rdparty/cgal/Alpha_shapes_3/include
    ../../../3rdparty/cgal/Arithmetic_kernel/include
    ../../../3rdparty/cgal/Cartesian_kernel/include
    ../../../3rdparty/cgal/CGAL_Core/include
    ../../../3rdparty/cgal/Circulator/include
    ../../../3rdparty/cgal/Distance_2/include
    ../../../3rdparty/cgal/Distance_3/include
    ../../../3rdparty/cgal/Filtered_kernel/include
    ../../../3rdparty/cgal/Hash_map/include
    ../../../3rdparty/cgal/Homogeneous_kernel/include
    ../../../3rdparty/cgal/Installation/include
    ../../../3rdparty/cgal/Intersections_2/include
    ../../../3rdparty/cgal/Intersections_3/include
    ../../../3rdparty/cgal/Interval_support/include
    ../../../3rdparty/cgal/Kernel_23/include
    ../../../3rdparty/cgal/Kernel_d/include
    ../../../3rdparty/cgal/Modular_arithmetic/include
    ../../../3rdparty/cgal/Number_types/include
    ../../../3rdparty/cgal/Polygon/include
    ../../../3rdparty/cgal/Profiling_tools/include
    ../../../3rdparty/cgal/Property_map/include
    ../../../3rdparty/cgal/Spatial_sorting/include
    ../../../3rdparty/cgal/STL_Extension/include
    ../../../3rdparty/cgal/Stream_support/include
    ../../../3rdparty/cgal/TDS_2/include
    ../../../3rdparty/cgal/TDS_3/include
    ../../../3rdparty/cgal/Triangulation_2/include
    ../../../3rdparty/cgal/Triangulation_3/include
)

target_compile_definitions(
    ${SUB_PROJECT_NAME}
    PRIVATE
    EXPORT_${SUB_PROJECT_NAME}=1
)

target_link_libraries(
    ${SUB_PROJECT_NAME}
    PRIVATE
    3DForestGui
)

install(TARGETS ${SUB_PROJECT_NAME} DESTINATION bin/plugins)