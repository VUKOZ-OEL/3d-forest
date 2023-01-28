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

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    message(STATUS "Clang compiler detected, adding compile flags")
    set(CLANG_COMPILE_FLAGS "\
-D_FILE_OFFSET_BITS=64 \
-std=c++17 \
-pedantic -Wall -Wextra"
    )
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CLANG_COMPILE_FLAGS}")
elseif(CMAKE_COMPILER_IS_GNUCXX)
    message(STATUS "GCC compiler detected, adding compile flags")
    set(GCC_COMPILE_FLAGS "\
-D_FILE_OFFSET_BITS=64 \
-std=c++17 \
-pedantic -Wall -Wextra \
-Wshadow -Wredundant-decls \
-Wconversion -Wsign-conversion -Wfloat-equal \
-Wcast-align -Wcast-qual \
-Wswitch-default -Wswitch-enum \
-Wlogical-op -Wwrite-strings -Wold-style-cast -Woverloaded-virtual"
    )
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GCC_COMPILE_FLAGS}")
elseif(MSVC)
    message(STATUS "MSVC compiler detected, adding compile flags")
    set(MSVC_COMPILE_FLAGS "\
/std:c++17 \
/Zc:__cplusplus \
/permissive-"
    )
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${MSVC_COMPILE_FLAGS}")
else()
    message(WARNING "Clang/GCC/MSVC compiler not detected")
endif()
