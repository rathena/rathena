# Find required packages
find_package(GTest REQUIRED)
find_package(Threads REQUIRED)
find_package(MySQL REQUIRED)
find_package(nlohmann_json REQUIRED)

# Create test executables
add_executable(network_tests
    network_monitor_test.cpp
    sync_test.cpp
    p2p_config_parser_test.cpp
)

# Link dependencies
target_link_libraries(network_tests
    PRIVATE
    common
    GTest::GTest
    GTest::Main
    ${MYSQL_LIBRARIES}
    nlohmann_json::nlohmann_json
    Threads::Threads
)

# Include directories
target_include_directories(network_tests
    PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${GTEST_INCLUDE_DIRS}
    ${MYSQL_INCLUDE_DIR}
)

# Enable testing
enable_testing()

# Add tests to CTest
add_test(NAME network_monitor_test 
         COMMAND network_tests --gtest_filter=NetworkMonitorTest.*)
add_test(NAME sync_test 
         COMMAND network_tests --gtest_filter=SyncTest.*)
add_test(NAME config_parser_test 
         COMMAND network_tests --gtest_filter=P2PConfigParserTest.*)

# Set environment variables for tests
set_tests_properties(network_monitor_test PROPERTIES
    ENVIRONMENT "TEST_DB_HOST=localhost;TEST_DB_USER=test_user;TEST_DB_PASS=test_pass;TEST_DB_NAME=test_rathena"
)

set_tests_properties(config_parser_test PROPERTIES
    ENVIRONMENT "TEST_CONFIG_PATH=${CMAKE_CURRENT_BINARY_DIR}/test_data"
)

# Create test database setup script
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/setup_test_db.sql.in
    ${CMAKE_CURRENT_BINARY_DIR}/setup_test_db.sql
    @ONLY
)

# Copy test configuration files
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/test_p2p_security.conf.in
    ${CMAKE_CURRENT_BINARY_DIR}/test_data/test_p2p_security.conf
    @ONLY
)

# Database setup target
add_custom_target(setup_test_db
    COMMAND ${CMAKE_COMMAND} -E echo "Setting up test database..."
    COMMAND mysql -u${TEST_DB_USER} -p${TEST_DB_PASS} < ${CMAKE_CURRENT_BINARY_DIR}/setup_test_db.sql
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/setup_test_db.sql
)

# Make tests depend on database setup
add_dependencies(network_tests setup_test_db)

# Compiler flags
if(MSVC)
    target_compile_options(network_tests PRIVATE 
        /W4
        /WX
        /wd4100  # Unreferenced formal parameter
    )
else()
    target_compile_options(network_tests PRIVATE 
        -Wall 
        -Wextra 
        -Werror
        -Wno-unused-parameter
    )
endif()

# Enable C++17 features
target_compile_features(network_tests PRIVATE cxx_std_17)

# Create directories for test data
add_custom_command(
    TARGET network_tests PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/test_data
)

# Test support macros
target_compile_definitions(network_tests PRIVATE
    TEST_DATA_DIR="${CMAKE_CURRENT_BINARY_DIR}/test_data"
)