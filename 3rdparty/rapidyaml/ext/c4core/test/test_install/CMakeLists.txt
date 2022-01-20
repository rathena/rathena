cmake_minimum_required(VERSION 3.12 FATAL_ERROR)
project(c4core
    DESCRIPTION "Test project for an installed c4core"
    HOMEPAGE_URL "https://github.com/biojppm/c4core"
    LANGUAGES CXX)
include(../../cmake/c4Project.cmake)
c4_project(VERSION 0.1.8
    AUTHOR "Joao Paulo Magalhaes <dev@jpmag.me>")

if(C4CORE_TEST_INSTALL_PACKAGE_MODE)
    find_package(c4core REQUIRED)
    add_library(c4core ALIAS c4core::c4core)
else()
    find_path(SUBSTR_HPP substr.hpp PATH_SUFFIXES c4 REQUIRED)
    find_library(C4CORE_LIBRARY c4core REQUIRED)
    get_filename_component(C4CORE_INCLUDE_DIR "${SUBSTR_HPP}" DIRECTORY)
    add_library(c4core UNKNOWN IMPORTED)
    set_target_properties(c4core PROPERTIES IMPORTED_LOCATION "${C4CORE_LIBRARY}")
    target_include_directories(c4core INTERFACE "${C4CORE_INCLUDE_DIR}")
endif()

enable_testing()
set(C4CORE_INSTALL_TEST ON)
add_subdirectory(../../test test)
