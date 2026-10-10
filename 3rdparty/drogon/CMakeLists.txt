# Source versions: Drogon 1.9.11, Trantor 1.5.24.
# Builds static Drogon + Trantor for HTTP/WebSockets, without TLS or DB drivers.
# Requires the existing jsoncpp_static and zlibstatic targets from the parent.
# Parent order:
#   add_subdirectory(3rdparty/zlib)
#   add_subdirectory(3rdparty/jsoncpp)
#   add_subdirectory(3rdparty/drogon)
# Consumer: target_link_libraries(3DForestUiWeb PUBLIC Drogon::Drogon)
cmake_minimum_required(VERSION 3.20)
project(3DForestDrogon LANGUAGES C CXX)

if(NOT WIN32 AND NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
    message(FATAL_ERROR "This reduced build supports Windows and Linux only")
endif()

foreach(dependency IN ITEMS jsoncpp_static zlibstatic)
    if(NOT TARGET ${dependency})
        message(FATAL_ERROR
            "Add the ${dependency} dependency before add_subdirectory(3rdparty/drogon)")
    endif()
endforeach()

if(TARGET drogon OR TARGET trantor)
    message(FATAL_ERROR "Drogon/Trantor targets have already been added")
endif()

# Ignore Qt's parent-directory automatic code generation for these libraries.
set(CMAKE_AUTOMOC OFF)
set(CMAKE_AUTOUIC OFF)
set(CMAKE_AUTORCC OFF)

if(NOT CMAKE_CXX_STANDARD OR CMAKE_CXX_STANDARD LESS 17)
    set(CMAKE_CXX_STANDARD 17)
endif()
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(GenerateExportHeader)
include(CheckCXXSourceCompiles)
find_package(Threads REQUIRED)

set(forest_drogon_root "${CMAKE_CURRENT_SOURCE_DIR}")
set(forest_trantor_root "${forest_drogon_root}/trantor")
set(forest_generated "${CMAKE_CURRENT_BINARY_DIR}/generated")
file(MAKE_DIRECTORY "${forest_generated}/drogon" "${forest_generated}/trantor")

# ---------------------------------------------------------------------------
# Trantor: event loops, TCP, built-in DNS resolver and built-in hash functions.
# ---------------------------------------------------------------------------
set(forest_trantor_sources
    trantor/utils/AsyncFileLogger.cc
    trantor/utils/ConcurrentTaskQueue.cc
    trantor/utils/Date.cc
    trantor/utils/LogStream.cc
    trantor/utils/Logger.cc
    trantor/utils/MsgBuffer.cc
    trantor/utils/SerialTaskQueue.cc
    trantor/utils/TimingWheel.cc
    trantor/utils/Utilities.cc
    trantor/net/EventLoop.cc
    trantor/net/EventLoopThread.cc
    trantor/net/EventLoopThreadPool.cc
    trantor/net/InetAddress.cc
    trantor/net/TcpClient.cc
    trantor/net/TcpServer.cc
    trantor/net/Channel.cc
    trantor/net/inner/Acceptor.cc
    trantor/net/inner/Connector.cc
    trantor/net/inner/Poller.cc
    trantor/net/inner/Socket.cc
    trantor/net/inner/MemBufferNode.cc
    trantor/net/inner/StreamBufferNode.cc
    trantor/net/inner/AsyncStreamBufferNode.cc
    trantor/net/inner/TcpConnectionImpl.cc
    trantor/net/inner/Timer.cc
    trantor/net/inner/TimerQueue.cc
    trantor/net/inner/poller/EpollPoller.cc
    trantor/net/inner/poller/KQueue.cc
    trantor/net/inner/poller/PollPoller.cc
    trantor/net/inner/NormalResolver.cc
    trantor/utils/crypto/sha3.cc
    trantor/utils/crypto/md5.cc
    trantor/utils/crypto/sha1.cc
    trantor/utils/crypto/sha256.cc
    trantor/utils/crypto/blake2.cc
)

if(WIN32)
    list(APPEND forest_trantor_sources
        third_party/wepoll/Wepoll.c
        trantor/utils/WindowsSupport.cc
        trantor/net/inner/FileBufferNodeWin.cc
    )
else()
    list(APPEND forest_trantor_sources
        trantor/net/inner/FileBufferNodeUnix.cc
    )
endif()

list(TRANSFORM forest_trantor_sources PREPEND "${forest_trantor_root}/")
add_library(trantor STATIC ${forest_trantor_sources})
add_library(Trantor::Trantor ALIAS trantor)

target_compile_features(trantor PUBLIC cxx_std_17)
set_target_properties(trantor PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_include_directories(trantor
    PUBLIC
        "${forest_trantor_root}"
        "${forest_generated}"
    PRIVATE
        "${forest_trantor_root}/trantor/utils"
        "${forest_trantor_root}/trantor/net"
        "${forest_trantor_root}/trantor/net/inner"
)

target_compile_definitions(trantor
    PUBLIC TRANTOR_STATIC_DEFINE
    PRIVATE TRANTOR_TLS_PROVIDER=None
)
target_link_libraries(trantor PUBLIC Threads::Threads)

generate_export_header(trantor
    EXPORT_FILE_NAME "${forest_generated}/trantor/exports.h"
)

if(WIN32)
    target_include_directories(trantor PRIVATE
        "${forest_trantor_root}/third_party/wepoll"
    )
    target_link_libraries(trantor PRIVATE ws2_32 rpcrt4 advapi32)
endif()

# Some architectures need libatomic for 64-bit atomics.
check_cxx_source_compiles("
    #include <atomic>
    #include <cstdint>
    int main() {
        std::atomic<std::int64_t> value{0};
        return static_cast<int>(value.fetch_add(1));
    }
" FOREST_TRANTOR_ATOMICS_LINK)
if(NOT FOREST_TRANTOR_ATOMICS_LINK)
    target_link_libraries(trantor PUBLIC atomic)
endif()

# ---------------------------------------------------------------------------
# Drogon: framework sources and database-disabled implementations.
# Generic ORM classes remain dependencies of the public framework interface.
# ---------------------------------------------------------------------------
add_library(drogon STATIC
    lib/src/AOPAdvice.cc
    lib/src/AccessLogger.cc
    lib/src/CacheFile.cc
    lib/src/ConfigAdapterManager.cc
    lib/src/ConfigLoader.cc
    lib/src/Cookie.cc
    lib/src/DrClassMap.cc
    lib/src/DrTemplateBase.cc
    lib/src/MiddlewaresFunction.cc
    lib/src/FixedWindowRateLimiter.cc
    lib/src/GlobalFilters.cc
    lib/src/Histogram.cc
    lib/src/Hodor.cc
    lib/src/HttpAppFrameworkImpl.cc
    lib/src/HttpBinder.cc
    lib/src/HttpClientImpl.cc
    lib/src/HttpConnectionLimit.cc
    lib/src/HttpControllerBinder.cc
    lib/src/HttpControllersRouter.cc
    lib/src/HttpFileImpl.cc
    lib/src/HttpFileUploadRequest.cc
    lib/src/HttpRequestImpl.cc
    lib/src/HttpRequestParser.cc
    lib/src/RequestStream.cc
    lib/src/HttpResponseImpl.cc
    lib/src/HttpResponseParser.cc
    lib/src/HttpServer.cc
    lib/src/HttpUtils.cc
    lib/src/HttpViewData.cc
    lib/src/IntranetIpFilter.cc
    lib/src/JsonConfigAdapter.cc
    lib/src/ListenerManager.cc
    lib/src/LocalHostFilter.cc
    lib/src/MultiPart.cc
    lib/src/MultipartStreamParser.cc
    lib/src/NotFound.cc
    lib/src/PluginsManager.cc
    lib/src/PromExporter.cc
    lib/src/RangeParser.cc
    lib/src/RateLimiter.cc
    lib/src/RealIpResolver.cc
    lib/src/SecureSSLRedirector.cc
    lib/src/Redirector.cc
    lib/src/SessionManager.cc
    lib/src/SlashRemover.cc
    lib/src/SlidingWindowRateLimiter.cc
    lib/src/StaticFileRouter.cc
    lib/src/TaskTimeoutFlag.cc
    lib/src/TokenBucketRateLimiter.cc
    lib/src/Utilities.cc
    lib/src/WebSocketClientImpl.cc
    lib/src/WebSocketConnectionImpl.cc
    lib/src/YamlConfigAdapter.cc

    lib/src/DbClientManagerSkipped.cc
    lib/src/RedisClientSkipped.cc
    lib/src/RedisResultSkipped.cc
    lib/src/RedisClientManagerSkipped.cc

    orm_lib/src/ArrayParser.cc
    orm_lib/src/Criteria.cc
    orm_lib/src/DbClient.cc
    orm_lib/src/DbClientImpl.cc
    orm_lib/src/DbClientLockFree.cc
    orm_lib/src/DbConnection.cc
    orm_lib/src/DbListener.cc
    orm_lib/src/Exception.cc
    orm_lib/src/Field.cc
    orm_lib/src/Result.cc
    orm_lib/src/Row.cc
    orm_lib/src/SqlBinder.cc
    orm_lib/src/TransactionImpl.cc
    orm_lib/src/RestfulController.cc
)
add_library(Drogon::Drogon ALIAS drogon)

target_compile_features(drogon PUBLIC cxx_std_17)
set_target_properties(drogon PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_include_directories(drogon
    PUBLIC
        "${forest_drogon_root}/lib/inc"
        "${forest_drogon_root}/orm_lib/inc"
        "${forest_drogon_root}/nosql_lib/redis/inc"
        "${forest_generated}"
)

target_compile_definitions(drogon PUBLIC DROGON_STATIC_DEFINE)
target_link_libraries(drogon
    PUBLIC trantor jsoncpp_static
    PRIVATE zlibstatic
)

generate_export_header(drogon
    EXPORT_FILE_NAME "${forest_generated}/drogon/exports.h"
)

# Local variables deliberately override optional dependencies detected elsewhere
# in a larger parent project. Disabled #ifdef-style flags must stay undefined.
set(USE_POSTGRESQL OFF)
set(LIBPQ_SUPPORTS_BATCH_MODE OFF)
set(USE_MYSQL OFF)
set(USE_SQLITE3 OFF)
set(HAS_STD_FILESYSTEM_PATH ON)
set(OpenSSL_FOUND OFF)
set(Boost_FOUND OFF)

set(DROGON_MAJOR_VERSION 1)
set(DROGON_MINOR_VERSION 9)
set(DROGON_PATCH_VERSION 11)
set(DROGON_VERSION_STRING "1.9.11")
set(GIT_SHA1 "vendored-1.9.11")
set(DROGON_CXX_STANDARD "${CMAKE_CXX_STANDARD}")
get_filename_component(COMPILER_COMMAND "${CMAKE_CXX_COMPILER}" NAME)
set(COMPILER_ID "${CMAKE_CXX_COMPILER_ID}")
set(COMPILATION_FLAGS "-std=c++")
set(INCLUDING_DIRS "")
# Metadata used by Drogon's Unix dynamic-view loader, not by the normal build.
foreach(forest_include IN ITEMS
    "${forest_drogon_root}/lib/inc"
    "${forest_drogon_root}/orm_lib/inc"
    "${forest_drogon_root}/nosql_lib/redis/inc"
    "${forest_trantor_root}"
    "${forest_generated}"
)
    string(APPEND INCLUDING_DIRS " -I\\\"${forest_include}\\\"")
endforeach()

configure_file(
    "${forest_drogon_root}/cmake/templates/config.h.in"
    "${forest_generated}/drogon/config.h"
    @ONLY
)
configure_file(
    "${forest_drogon_root}/cmake/templates/version.h.in"
    "${forest_generated}/drogon/version.h"
    @ONLY
)

if(WIN32)
    target_sources(drogon PRIVATE third_party/mman-win32/mman.c)
    target_include_directories(drogon PRIVATE
        "${forest_drogon_root}/third_party/mman-win32"
    )
    target_link_libraries(drogon PRIVATE
        shlwapi ws2_32 iphlpapi rpcrt4 advapi32
    )
    foreach(forest_target IN ITEMS drogon trantor)
        target_compile_definitions(${forest_target} PUBLIC
            NOMINMAX WIN32_LEAN_AND_MEAN
        )
        target_compile_definitions(${forest_target} PRIVATE
            _CRT_SECURE_NO_WARNINGS _CRT_NONSTDC_NO_WARNINGS
        )
        if(MINGW)
            target_compile_definitions(${forest_target} PUBLIC _WIN32_WINNT=0x0601)
        endif()
    endforeach()
else()
    # Linux-only additional dependency: util-linux libuuid development files.
    find_path(FOREST_UUID_INCLUDE_DIR NAMES uuid.h PATH_SUFFIXES uuid REQUIRED)
    find_library(FOREST_UUID_LIBRARY NAMES uuid REQUIRED)
    target_sources(drogon PRIVATE lib/src/SharedLibManager.cc)
    target_include_directories(drogon PRIVATE "${FOREST_UUID_INCLUDE_DIR}")
    target_compile_definitions(drogon PRIVATE USE_OSSP_UUID=0)
    target_link_libraries(drogon PRIVATE "${FOREST_UUID_LIBRARY}" ${CMAKE_DL_LIBS})
endif()

if(MSVC)
    foreach(forest_target IN ITEMS drogon trantor)
        target_compile_options(${forest_target} PRIVATE /utf-8)
        # Headers use __cplusplus to select their API paths; propagate to users.
        target_compile_options(${forest_target} PUBLIC
            "$<$<COMPILE_LANGUAGE:CXX>:/Zc:__cplusplus>"
        )
    endforeach()
endif()

# No upstream add_subdirectory(trantor), find_package(Drogon/Jsoncpp/ZLIB),
# downloads, tools, tests, installation rules, database drivers, TLS providers,
# c-ares, yaml-cpp, Brotli, or spdlog are part of this build.
