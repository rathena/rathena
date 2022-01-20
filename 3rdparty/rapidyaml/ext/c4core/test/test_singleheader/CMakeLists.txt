cmake_minimum_required(VERSION 3.15 FATAL_ERROR)
project(c4core
    DESCRIPTION "Single header version of c4core library for testing"
    HOMEPAGE_URL "https://github.com/biojppm/c4core"
    LANGUAGES CXX)
include(../../cmake/c4Project.cmake)
c4_project(VERSION 0.1.8
    AUTHOR "Joao Paulo Magalhaes <dev@jpmag.me>")

# amalgamate c4core to get the single header
function(amalgamate_c4core header_dir header_file)
    set(c4coredir "${CMAKE_CURRENT_LIST_DIR}/../..")
    set(singleheaderdir "${c4coredir}/src_singleheader")
    set(singleheader "${singleheaderdir}/c4/c4core_all.hpp")
    set(amscript "${c4coredir}/tools/amalgamate.py")
    file(GLOB_RECURSE srcfiles
        LIST_DIRECTORIES FALSE
        CONFIGURE_DEPENDS
        "${c4coredir}/src")
    add_custom_command(OUTPUT "${singleheader}"
        COMMAND python "${amscript}" "${singleheader}"
        COMMENT "python ${amscript} ${singleheader}"
        DEPENDS ${srcfiles} "${amscript}" "${c4coredir}/cmake/amalgamate_utils.py")
    set(${header_dir} "${singleheaderdir}" PARENT_SCOPE)
    set(${header_file} "${singleheader}" PARENT_SCOPE)
endfunction()
amalgamate_c4core(HEADER_DIR HEADER_FILE)

c4_add_library(c4core
    INC_DIRS
        $<BUILD_INTERFACE:${HEADER_DIR}> $<INSTALL_INTERFACE:include>
    SOURCE_ROOT ${HEADER_DIR}
    SOURCES
        ${HEADER_FILE}
        ${CMAKE_CURRENT_LIST_DIR}/libc4core_singleheader.cpp)
target_compile_definitions(c4core PUBLIC -DC4CORE_SINGLE_HEADER)

enable_testing()
set(C4CORE_DEFINED_FROM_SINGLEHEADER ON)
add_subdirectory(../../test test)
