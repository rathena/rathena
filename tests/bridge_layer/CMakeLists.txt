# Bridge Layer Test Suite
# Compile and run C++ tests for rAthena Bridge Layer

cmake_minimum_required(VERSION 3.16)
project(BridgeLayerTests CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Google Test
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

# Include project headers
include_directories(${CMAKE_SOURCE_DIR}/src/aiworld)
include_directories(${CMAKE_SOURCE_DIR}/src/common)

# Test executables
add_executable(test_http_commands test_http_commands.cpp)
target_link_libraries(test_http_commands ${GTEST_LIBRARIES} pthread)

add_executable(test_event_dispatcher test_event_dispatcher.cpp)
target_link_libraries(test_event_dispatcher ${GTEST_LIBRARIES} pthread)

add_executable(test_action_executor test_action_executor.cpp)
target_link_libraries(test_action_executor ${GTEST_LIBRARIES} pthread)

add_executable(test_integration_flow test_integration_flow.cpp)
target_link_libraries(test_integration_flow ${GTEST_LIBRARIES} pthread)

# Enable testing
enable_testing()

add_test(NAME HTTPCommands COMMAND test_http_commands)
add_test(NAME EventDispatcher COMMAND test_event_dispatcher)
add_test(NAME ActionExecutor COMMAND test_action_executor)
add_test(NAME IntegrationFlow COMMAND test_integration_flow)

# Custom test target
add_custom_target(run_bridge_tests
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    DEPENDS test_http_commands test_event_dispatcher test_action_executor test_integration_flow
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Running Bridge Layer test suite..."
)

# Coverage target (if gcov available)
if(CMAKE_BUILD_TYPE MATCHES Coverage)
    add_custom_target(coverage
        COMMAND lcov --capture --directory . --output-file coverage.info
        COMMAND genhtml coverage.info --output-directory coverage_html
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Generating coverage report..."
    )
endif()