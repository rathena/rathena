# CMakeLists.txt for rAthena AIWorld Plugin/Module (ZeroMQ IPC)

cmake_minimum_required(VERSION 3.20)
project(aiworld_plugin LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find ZeroMQ
find_package(PkgConfig REQUIRED)
pkg_check_modules(ZeroMQ REQUIRED libzmq)

# Plugin sources
set(AIWORLD_SRC
    aiworld_plugin.cpp
    aiworld_ipc.cpp
    aiworld_ipc.hpp
    aiworld_messages.hpp
    aiworld_plugin.hpp
    aiworld_utils.cpp
    aiworld_utils.hpp
    aiworld_http_client.hpp
    aiworld_http_client.cpp
    aiworld_script_commands.cpp
    aiworld_native_commands.hpp
    aiworld_native_commands.cpp
    aiworld_native_api.hpp
    aiworld_native_api.cpp
)

add_library(aiworld_plugin SHARED ${AIWORLD_SRC})

target_include_directories(aiworld_plugin
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${ZeroMQ_INCLUDE_DIRS}
        ${CMAKE_SOURCE_DIR}/3rdparty/httplib
        ${CMAKE_SOURCE_DIR}/3rdparty/json/single_include
        ../common
        ../map
        ../char
        ../config
)

target_link_libraries(aiworld_plugin
    ${ZeroMQ_LIBRARIES}
)

target_compile_definitions(aiworld_plugin PRIVATE AIWORLD_PLUGIN_BUILD)