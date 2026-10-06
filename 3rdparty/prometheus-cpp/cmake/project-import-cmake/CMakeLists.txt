cmake_minimum_required(VERSION 3.12 FATAL_ERROR)

project(prometheus-cpp-import)

find_package(prometheus-cpp CONFIG REQUIRED)

if(NOT DEFINED prometheus-cpp_VERSION)
  message(FATAL_ERROR "prometheus-cpp_VERSION is not defined")
endif()

if(PROMETHEUS_CPP_ENABLE_PUSH)
  add_executable(sample-client sample_client.cc)
  target_link_libraries(sample-client PRIVATE prometheus-cpp::push $<$<BOOL:${WIN32}>:Ws2_32>)
endif()

if(PROMETHEUS_CPP_ENABLE_PULL)
  add_executable(sample-server sample_server.cc)
  target_link_libraries(sample-server PRIVATE prometheus-cpp::pull)
endif()
