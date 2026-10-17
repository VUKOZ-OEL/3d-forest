cmake_minimum_required(VERSION 3.10)

project(sqlite LANGUAGES C)

option(BUILD_SHARED_LIBS "Build shared libraries" OFF)
add_library(sqlite3 sqlite3.c)
target_include_directories(sqlite3 PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
set_target_properties(sqlite3 PROPERTIES
    POSITION_INDEPENDENT_CODE ON
    WINDOWS_EXPORT_ALL_SYMBOLS ON)
if(NOT BUILD_SHARED_LIBS)
    set_target_properties(sqlite3 PROPERTIES C_VISIBILITY_PRESET hidden)
endif()
