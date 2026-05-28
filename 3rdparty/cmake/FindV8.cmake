# FindV8.cmake
#
# Locate a system V8 install (headers + libraries) and the v8pp
# header-only helper library. Sets:
#
#   V8_FOUND            - TRUE if V8 + v8pp were located
#   V8_INCLUDE_DIRS     - include paths (V8 + v8-platform + v8pp)
#   V8_LIBRARIES        - link list (libv8 + libv8_libplatform + ...)
#
# Search hints:
#   - V8_ROOT            (env or CMake var) — root of a V8 install
#   - V8PP_INCLUDE_DIR   (CMake var) — explicit v8pp header path
#
# On macOS the Homebrew `v8` formula is the default target.
# On Linux we look at /usr/include/v8 and /usr/local/include.
#
# This module deliberately does not bundle V8 — V8 is huge and its
# build is non-trivial. The expectation is that the operator installs
# it once via the platform package manager.

include( FindPackageHandleStandardArgs )

# ---- locate libv8 + libv8_libplatform --------------------------------------

set( _V8_HINTS )
if( DEFINED ENV{V8_ROOT} )
    list( APPEND _V8_HINTS "$ENV{V8_ROOT}" )
endif()
if( DEFINED V8_ROOT )
    list( APPEND _V8_HINTS "${V8_ROOT}" )
endif()

# Homebrew on Apple Silicon / Intel
if( APPLE )
    list( APPEND _V8_HINTS
        "/opt/homebrew/opt/v8"
        "/usr/local/opt/v8" )
endif()

find_path( V8_INCLUDE_DIR
    NAMES v8.h
    HINTS ${_V8_HINTS}
    PATH_SUFFIXES include libexec/include
)

find_library( V8_LIBRARY
    NAMES v8 v8_monolith
    HINTS ${_V8_HINTS}
    PATH_SUFFIXES lib libexec libexec/lib
)

find_library( V8_LIBPLATFORM_LIBRARY
    NAMES v8_libplatform
    HINTS ${_V8_HINTS}
    PATH_SUFFIXES lib libexec libexec/lib
)

find_library( V8_LIBBASE_LIBRARY
    NAMES v8_libbase
    HINTS ${_V8_HINTS}
    PATH_SUFFIXES lib libexec libexec/lib
)

# ---- locate v8pp -----------------------------------------------------------

set( _V8PP_HINTS
    "${CMAKE_SOURCE_DIR}/3rdparty/v8pp/src"     # submodule install
    "${CMAKE_SOURCE_DIR}/3rdparty/v8pp"
)
if( DEFINED V8PP_INCLUDE_DIR )
    list( PREPEND _V8PP_HINTS "${V8PP_INCLUDE_DIR}" )
endif()

find_path( V8PP_INCLUDE_DIR_RESOLVED
    NAMES v8pp/class.hpp
    HINTS ${_V8PP_HINTS}
    PATH_SUFFIXES include
    NO_DEFAULT_PATH
)
if( NOT V8PP_INCLUDE_DIR_RESOLVED )
    find_path( V8PP_INCLUDE_DIR_RESOLVED
        NAMES v8pp/class.hpp
    )
endif()

# ---- assemble result -------------------------------------------------------

set( V8_INCLUDE_DIRS ${V8_INCLUDE_DIR} ${V8PP_INCLUDE_DIR_RESOLVED} )
set( V8_LIBRARIES
    ${V8_LIBRARY}
    ${V8_LIBPLATFORM_LIBRARY}
    ${V8_LIBBASE_LIBRARY}
)

find_package_handle_standard_args( V8
    REQUIRED_VARS
        V8_INCLUDE_DIR
        V8_LIBRARY
        V8_LIBPLATFORM_LIBRARY
        V8PP_INCLUDE_DIR_RESOLVED
)

mark_as_advanced(
    V8_INCLUDE_DIR
    V8_LIBRARY
    V8_LIBPLATFORM_LIBRARY
    V8_LIBBASE_LIBRARY
    V8PP_INCLUDE_DIR_RESOLVED
)
