# Fuzz process is dependent upon a few environment variables provided by OSSFuzz during the build process
# For more information, see google.github.io/oss-fuzz/getting-started/new-project-guide/#buildsh-script-environment

include(ExternalProject)
add_definitions(-DNDEBUG)  # Do not want assertions for fuzz-testing

if (DEFINED ENV{CFLAGS})
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} $ENV{CFLAGS}")
endif()

if (DEFINED ENV{CXXFLAGS})
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} $ENV{CXXFLAGS}")
endif()

if (DEFINED ENV{CC})
    set(CMAKE_C_COMPILER "$ENV{CC}" CACHE STRING "C compiler" FORCE)
endif()

if (DEFINED ENV{CXX})
    set(CMAKE_CXX_COMPILER "$ENV{CXX}" CACHE STRING "CXX compiler" FORCE)
endif()

if(CMAKE_HOST_WIN32)
    set(libname "libconfig")
else()
    set(libname "config")
endif()

set(CMAKE_EXE_LINKER_FLAGS "$ENV{LIB_FUZZING_ENGINE}")

add_executable(config_read_fuzzer fuzz_config_read.c fuzz_data.c)

target_link_libraries(config_read_fuzzer PRIVATE
        ${libname}
        $ENV{LIB_FUZZING_ENGINE}
)

if (DEFINED ENV{OUT})
    install(TARGETS config_read_fuzzer DESTINATION $ENV{OUT})
else()
    message(WARNING "Cannot install if $OUT is not defined!")
endif()