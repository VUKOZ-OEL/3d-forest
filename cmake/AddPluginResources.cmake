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

# Requires CMake 3.17+ and host Python 3.8+.
include_guard(GLOBAL)

function(add_plugin_resources target bundle_name manifest)
    if(NOT TARGET "${target}")
        message(FATAL_ERROR "Unknown resource target: ${target}")
    endif()
    find_package(Python3 3.8 REQUIRED COMPONENTS Interpreter)
    set(generator "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/embed_resources.py")
    get_filename_component(manifest_path "${manifest}" ABSOLUTE
        BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

    execute_process(
        COMMAND "${Python3_EXECUTABLE}" "${generator}" --qrc "${manifest_path}" --list
        RESULT_VARIABLE result
        OUTPUT_VARIABLE dependency_lines
        ERROR_VARIABLE error_text
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if(NOT result EQUAL 0)
        message(FATAL_ERROR "Resource manifest error: ${error_text}")
    endif()
    string(REPLACE "\r\n" "\n" dependency_lines "${dependency_lines}")
    string(REPLACE "\n" ";" dependencies "${dependency_lines}")

    # Changes to the manifest can add/remove dependencies, so reconfigure.
    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
        "${manifest_path}" "${generator}")

    set(output_dir "${CMAKE_CURRENT_BINARY_DIR}/embedded/${target}/${bundle_name}")
    set(source "${output_dir}/${bundle_name}.cpp")
    set(header "${output_dir}/${bundle_name}.hpp")

    add_custom_command(
        OUTPUT "${source}" "${header}"
        COMMAND "${Python3_EXECUTABLE}" "${generator}"
            --qrc "${manifest_path}"
            --name "${bundle_name}"
            --out-dir "${output_dir}"
        DEPENDS "${manifest_path}" "${generator}" ${dependencies}
        COMMENT "Embedding resources: ${bundle_name}"
        VERBATIM
    )

    target_sources("${target}" PRIVATE "${source}" "${header}")
    target_include_directories("${target}" PRIVATE "${output_dir}")
endfunction()
