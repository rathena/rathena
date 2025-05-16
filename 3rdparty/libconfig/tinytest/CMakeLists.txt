add_library(libtinytest STATIC
    tinytest.c tinytest.h)

target_include_directories(libtinytest
    PUBLIC
        $<INSTALL_INTERFACE:.>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

if(CMAKE_HOST_WIN32)
    if(MSVC)
        set_target_properties(libtinytest
            PROPERTIES
                COMPILE_FLAGS "/wd4996"
            )
    endif()
endif()
