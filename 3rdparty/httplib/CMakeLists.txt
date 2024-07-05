add_library(httplib STATIC)

target_sources(httplib PRIVATE "httplib.cc")

target_include_directories(httplib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

if(NOT WIN32)
    target_compile_definitions(httplib PUBLIC "CPPHTTPLIB_SEND_FLAGS=MSG_NOSIGNAL")
endif()

if(WIN32)
    target_sources(httplib PRIVATE "httplib.h")
    set_target_properties(httplib PROPERTIES FOLDER "3rdparty")
endif()

if(NOT ENABLE_WEB_SERVER)
    set_target_properties(httplib PROPERTIES EXCLUDE_FROM_ALL TRUE)
endif()
