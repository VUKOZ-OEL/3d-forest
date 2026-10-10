add_library(jsoncpp_static STATIC
    src/lib_json/json_reader.cpp
    src/lib_json/json_value.cpp
    src/lib_json/json_writer.cpp
)

target_include_directories(jsoncpp_static PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_compile_features(jsoncpp_static PUBLIC
    cxx_std_11
)

set_target_properties(jsoncpp_static PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)

if(MSVC)
    target_compile_definitions(jsoncpp_static PRIVATE
        _CRT_SECURE_NO_WARNINGS
    )
endif()