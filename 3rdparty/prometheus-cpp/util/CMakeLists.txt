add_library(util INTERFACE)

add_library(${PROJECT_NAME}::util ALIAS util)

target_compile_features(util
  INTERFACE
    cxx_std_11
)

target_include_directories(util
  INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

install(
  TARGETS util
  EXPORT ${PROJECT_NAME}-targets
  RUNTIME DESTINATION  ${CMAKE_INSTALL_BINDIR}
  LIBRARY DESTINATION  ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION  ${CMAKE_INSTALL_LIBDIR}
)

if(ENABLE_TESTING)
  add_subdirectory(tests)
endif()
