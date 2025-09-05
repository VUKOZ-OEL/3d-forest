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

set(SUB_PROJECT_NAME "Pcl")

add_library(
    ${SUB_PROJECT_NAME}
    SHARED
    common/src/fft/kiss_fft.c
    common/src/fft/kiss_fftr.c
    common/src/bearing_angle_image.cpp
    common/src/colors.cpp
    common/src/common.cpp
    common/src/correspondence.cpp
    common/src/distances.cpp
    common/src/feature_histogram.cpp
    common/src/gaussian.cpp
    common/src/io.cpp
    common/src/parse.cpp
    common/src/pcl_base.cpp
    common/src/PCLPointCloud2.cpp
    common/src/point_types.cpp
    common/src/poses_from_matches.cpp
    common/src/print.cpp
    common/src/projection_matrix.cpp
    common/src/range_image.cpp
    common/src/range_image_planar.cpp
    common/src/time_trigger.cpp
    kdtree/src/kdtree_flann.cpp
    octree/src/octree_inst.cpp
    search/src/brute_force.cpp
    search/src/kdtree.cpp
    search/src/octree.cpp
    search/src/organized.cpp
    search/src/search.cpp
)

if (WIN32)
    set_target_properties(
        ${SUB_PROJECT_NAME}
        PROPERTIES
        WINDOWS_EXPORT_ALL_SYMBOLS ON
    )
endif()

target_include_directories(
    ${SUB_PROJECT_NAME}
    PUBLIC
    ../../3rdparty/boost
    ../../3rdparty/eigen
    ../../3rdparty/lz4/lib
    ../../3rdparty/flann/src/cpp
    common/include
    kdtree/include
    octree/include
    search/include
)

target_compile_definitions(
    ${SUB_PROJECT_NAME}
    PRIVATE
    EXPORT_${SUB_PROJECT_NAME}=1
)

target_link_libraries(
    ${SUB_PROJECT_NAME}
    PRIVATE
    LZ4
)

install(TARGETS ${SUB_PROJECT_NAME} DESTINATION bin)