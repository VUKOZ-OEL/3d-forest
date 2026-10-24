cmake_minimum_required(VERSION 3.24)
project(quickjs VERSION 0.11.0 LANGUAGES C)
find_package(Threads REQUIRED)
include(GNUInstallDirs)

# Runtime library only. Upstream tools and test files live in slop/iland-model/vendor/quickjs.
add_library(qjs cutils.c dtoa.c libregexp.c libunicode.c quickjs.c)
set_target_properties(qjs PROPERTIES
    C_STANDARD 11
    C_STANDARD_REQUIRED ON
    C_EXTENSIONS ON
    C_VISIBILITY_PRESET hidden
    POSITION_INDEPENDENT_CODE ON
    WINDOWS_EXPORT_ALL_SYMBOLS ON
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR})
target_compile_definitions(qjs PRIVATE _GNU_SOURCE)

if(WIN32)
    target_compile_definitions(qjs PRIVATE WIN32_LEAN_AND_MEAN _WIN32_WINNT=0x0601)
endif()

if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
    target_compile_options(qjs PRIVATE
        -Wno-sign-conversion
        -Wno-conversion
        -Wno-float-equal
        $<$<COMPILE_LANGUAGE:CXX>:-Wno-old-style-cast>
    )
elseif(MSVC)
    target_compile_options(qjs PRIVATE
        /experimental:c11atomics
        /wd4018
        /wd4146
        /wd4244
        /wd4267
        /wd4334
    )
endif()

target_include_directories(qjs PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>")
target_link_libraries(qjs PUBLIC Threads::Threads ${CMAKE_DL_LIBS})
if(NOT WIN32)
    find_library(QJS_MATH_LIBRARY m)
    if(QJS_MATH_LIBRARY)
        target_link_libraries(qjs PUBLIC "${QJS_MATH_LIBRARY}")
    endif()
endif()
install(TARGETS qjs EXPORT qjsConfig
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(EXPORT qjsConfig DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/quickjs)
install(FILES quickjs.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(FILES LICENSE DESTINATION ${CMAKE_INSTALL_DOCDIR})
