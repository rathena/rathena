cmake_minimum_required(VERSION 3.13)
project(rathena_tests)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

file(GLOB TEST_SOURCES "*.cpp")

# Add core implementation files needed for unit tests
set(TEST_IMPL_SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/git_svn_stubs.cpp
  ${CMAKE_SOURCE_DIR}/src/common/utils.cpp
  ${CMAKE_SOURCE_DIR}/src/common/random.cpp
  ${CMAKE_SOURCE_DIR}/src/common/showmsg.cpp
  ${CMAKE_SOURCE_DIR}/src/common/strlib.cpp
  ${CMAKE_SOURCE_DIR}/src/common/malloc.cpp
  ${CMAKE_SOURCE_DIR}/src/common/p2p_coordinator.cpp
  ${CMAKE_SOURCE_DIR}/src/common/quic.cpp
)

add_executable(rathena_unit_tests
  ${CMAKE_CURRENT_SOURCE_DIR}/git_svn_stubs.cpp
  ${TEST_SOURCES}
  ${TEST_IMPL_SOURCES}
  ${CMAKE_CURRENT_SOURCE_DIR}/mem_stubs.cpp
)

target_link_libraries(rathena_unit_tests
  gtest_main
  curl
)
target_compile_definitions(rathena_unit_tests PRIVATE UNIT_TEST)

target_include_directories(rathena_unit_tests PRIVATE
  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_SOURCE_DIR}/src/common
  ${CMAKE_SOURCE_DIR}/3rdparty/libconfig
  ${CMAKE_SOURCE_DIR}/3rdparty/rapidyaml/src
  ${CMAKE_SOURCE_DIR}/3rdparty/rapidyaml/ext/c4core/src
)

include(GoogleTest)
gtest_discover_tests(rathena_unit_tests)