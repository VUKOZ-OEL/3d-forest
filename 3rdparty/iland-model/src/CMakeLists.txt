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

set(SUB_PROJECT_NAME ILandModel)

include(sources.cmake)

add_library(
    ${SUB_PROJECT_NAME}
    SHARED
    ${ILAND_MODEL_SOURCES}
    compat/portable_bindings.cpp
    compat/script.cpp
    compat/process.cpp
    "../../simple-rng/SimpleRNG.cpp")

target_include_directories(
    ${SUB_PROJECT_NAME}
    PRIVATE
    compat/include
    compat/include_modules
    "../../simple-rng"
    "../../mersenne-twister"
    abe
    abe/output
    bite
    core
    output
    plugins
    plugins/barkbeetle
    plugins/fire
    plugins/wind
    tools
)
target_include_directories(
    ${SUB_PROJECT_NAME}
    PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_compile_definitions(
    ${SUB_PROJECT_NAME}
    PRIVATE
    EXPORT_${SUB_PROJECT_NAME}=1
    _USE_MATH_DEFINES
    NOMINMAX
    $<$<CXX_COMPILER_ID:MSVC>:_SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING>
)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
    target_compile_options(
        ${SUB_PROJECT_NAME}
        PRIVATE
        -Wno-sign-conversion
        -Wno-conversion
        -Wno-float-equal
        -Wno-old-style-cast
    )
elseif(MSVC)
    target_compile_options(
        ${SUB_PROJECT_NAME}
        PRIVATE
        /wd4244
    )
endif()

target_link_libraries(
    ${SUB_PROJECT_NAME}
    PRIVATE
    sqlite3
    pugixml::pugixml
    qjs
    Threads::Threads
)

install(TARGETS ${SUB_PROJECT_NAME} DESTINATION bin)
install(FILES runilandmodel.h DESTINATION include)