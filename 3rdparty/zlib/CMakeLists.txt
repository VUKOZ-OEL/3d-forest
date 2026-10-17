add_library(zlibstatic STATIC
    adler32.c
    compress.c
    crc32.c
    deflate.c
    gzclose.c
    gzlib.c
    gzread.c
    gzwrite.c
    infback.c
    inffast.c
    inflate.c
    inftrees.c
    trees.c
    uncompr.c
    zutil.c
)

target_include_directories(zlibstatic PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}"
)

set_target_properties(zlibstatic PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)

if(MSVC)
    target_compile_definitions(zlibstatic PRIVATE
        _CRT_SECURE_NO_WARNINGS
        _CRT_NONSTDC_NO_WARNINGS
    )
endif()

add_library(ZLIB::ZLIB ALIAS zlibstatic)