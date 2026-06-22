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

find_program(CLANG_FORMAT_EXE NAMES clang-format clang-format.exe)

if(NOT CLANG_FORMAT_EXE)
    message(WARNING "clang-format not found - skipping format target")
    return()
endif()

message(STATUS "clang-format: ${CLANG_FORMAT_EXE}")

file(GLOB_RECURSE ALL_SOURCE_FILES
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/*.hpp"
    "${CMAKE_SOURCE_DIR}/src/*.c"
    "${CMAKE_SOURCE_DIR}/src/*.h"
)

set(FORMAT_SCRIPT "${CMAKE_BINARY_DIR}/format.cmake")

file(WRITE "${FORMAT_SCRIPT}" "")

foreach(src IN LISTS ALL_SOURCE_FILES)
    file(APPEND "${FORMAT_SCRIPT}" "
execute_process(
    COMMAND \"${CLANG_FORMAT_EXE}\"
            -style=file
            -i
            -fallback-style=none
            \"${src}\"
    COMMAND_ERROR_IS_FATAL ANY
)
")
endforeach()

add_custom_target(format
    COMMAND "${CMAKE_COMMAND}" -P "${FORMAT_SCRIPT}"
    COMMENT "Running clang-format"
)
