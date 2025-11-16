cmake_minimum_required(VERSION 3.16)
project(p2p_coordinator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)

# Add dependencies: websocket, redis, rest, json, etc.
# Example: find_package(nlohmann_json REQUIRED)
# Example: find_package(cpprestsdk REQUIRED)
# Example: find_package(hiredis REQUIRED)

include_directories(include)

add_executable(p2p_coordinator
    src/main.cpp
    src/HostRegistry.cpp
    src/SessionManager.cpp
    src/ZoneManager.cpp
    src/WebRTCSignaling.cpp
    src/RestApiServer.cpp
    src/RedisClient.cpp
    src/AIServiceClient.cpp
    src/HealthMonitor.cpp
    src/Logger.cpp
    src/ErrorHandling.cpp
)

target_link_libraries(p2p_coordinator
    Threads::Threads
    # nlohmann_json::nlohmann_json
    # cpprestsdk::cpprest
    # hiredis::hiredis
)