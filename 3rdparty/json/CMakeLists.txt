cmake_minimum_required(VERSION 3.13)

set (JSON_INCLUDE_DIRS
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    CACHE INTERNAL "json include dir" )

mark_as_advanced( JSON_INCLUDE_DIRS )
