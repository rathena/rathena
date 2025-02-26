# Find required packages
find_package(ZLIB REQUIRED)
find_package(MySQL REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(Threads REQUIRED)

# Add network monitoring sources
set(COMMON_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/network_monitor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/network_security.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/network_optimization.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/p2p_config_parser.cpp
)

# Add header files
set(COMMON_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/network_monitor.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/network_security.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/network_optimization.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/thread_guard.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/sync.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/p2p_config_parser.hpp
)

# Library configuration
add_library(common STATIC ${COMMON_SOURCES} ${COMMON_HEADERS})

target_include_directories(common PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/3rdparty
    ${ZLIB_INCLUDE_DIRS}
    ${MYSQL_INCLUDE_DIR}
    ${LIBCONFIG_INCLUDE_DIR}
)

target_link_libraries(common PUBLIC
    ${ZLIB_LIBRARIES}
    ${MYSQL_LIBRARIES}
    nlohmann_json::nlohmann_json
    Threads::Threads
    libconfig
)

# Compiler flags
if(MSVC)
    target_compile_options(common PRIVATE 
        /W4 
        /wd4100  # Unreferenced formal parameter
        /wd4245  # Conversion from 'type1' to 'type2', signed/unsigned mismatch
        /wd4458  # Declaration of 'identifier' hides class member
    )
else()
    target_compile_options(common PRIVATE 
        -Wall 
        -Wextra 
        -Werror
        -Wno-unused-parameter
        -Wno-sign-compare
    )
endif()

# Enable C++17 features
target_compile_features(common PUBLIC cxx_std_17)

# Define platform-specific macros
if(WIN32)
    target_compile_definitions(common PRIVATE
        WIN32_LEAN_AND_MEAN
        NOMINMAX
        _CRT_SECURE_NO_WARNINGS
    )
endif()

# Installation
install(TARGETS common
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(FILES ${COMMON_HEADERS}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/common
)

# Add package config file
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/CommonConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/CommonConfig.cmake
    @ONLY
)

# Install package config file
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/CommonConfig.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Common
)

# Generate and install version file
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/CommonConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/CommonConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Common
)

# Add test subdirectory if testing is enabled
if(BUILD_TESTING)
    add_subdirectory(test)
endif()
