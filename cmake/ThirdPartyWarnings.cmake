# Copyright 2026 VUKOZ
#
# This file is part of 3D Forest.
#
# 3D Forest is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with 3D Forest.  If not, see <https://www.gnu.org/licenses/>.

function(forest_disable_third_party_warnings target)
    if(CMAKE_C_COMPILER_ID MATCHES "Clang|AppleClang")
        set(c_warning_options
            -Wno-sign-conversion
            -Wno-conversion
            -Wno-float-equal
            -Wno-unused-variable
            -Wno-unused-function
            -Wno-unused-but-set-variable
            -Wno-deprecated-declarations
            -Wno-logical-op-parentheses
            -Wno-format
            -Wno-dangling-else
        )
    elseif(CMAKE_C_COMPILER_ID STREQUAL "GNU")
        set(c_warning_options
            -Wno-sign-conversion
            -Wno-conversion
            -Wno-float-equal
            -Wno-unused-variable
            -Wno-unused-function
            -Wno-unused-but-set-variable
            -Wno-deprecated-declarations
            -Wno-format
            -Wno-dangling-else
        )
    elseif(CMAKE_C_COMPILER_ID STREQUAL "MSVC")
        set(c_warning_options /wd4244)
    endif()

    foreach(option IN LISTS c_warning_options)
        target_compile_options(
            ${target}
            PRIVATE
            "$<$<COMPILE_LANGUAGE:C>:${option}>"
        )
    endforeach()

    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang")
        set(cxx_warning_options
            -Wno-sign-conversion
            -Wno-conversion
            -Wno-float-equal
            -Wno-old-style-cast
            -Wno-unused-variable
            -Wno-unused-function
            -Wno-unused-but-set-variable
            -Wno-range-loop-construct
            -Wno-deprecated-declarations
            -Wno-logical-op-parentheses
            -Wno-format
            -Wno-unused-private-field
            -Wno-unused-local-typedef
            -Wno-dangling-else
            -Wno-inconsistent-missing-override
        )
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        set(cxx_warning_options
            -Wno-sign-conversion
            -Wno-conversion
            -Wno-float-equal
            -Wno-old-style-cast
            -Wno-unused-variable
            -Wno-unused-function
            -Wno-unused-but-set-variable
            -Wno-range-loop-construct
            -Wno-deprecated-declarations
            -Wno-format
            -Wno-dangling-else
        )
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        set(cxx_warning_options /wd4244)
    endif()

    foreach(option IN LISTS cxx_warning_options)
        target_compile_options(
            ${target}
            PRIVATE
            "$<$<COMPILE_LANGUAGE:CXX>:${option}>"
        )
    endforeach()
endfunction()
