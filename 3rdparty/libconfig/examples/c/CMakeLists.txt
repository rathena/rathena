if(CMAKE_HOST_WIN32)
    set(libname "libconfig")
else()
    set(libname "config")
endif()

add_executable(c_example1 example1.c )
add_executable(c_example2 example2.c )
add_executable(c_example3 example3.c )

target_link_libraries(c_example1 ${libname} )
target_link_libraries(c_example2 ${libname} )
target_link_libraries(c_example3 ${libname} )
