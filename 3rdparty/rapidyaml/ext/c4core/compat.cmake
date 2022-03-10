
# old gcc-4.8 support
if((CMAKE_CXX_COMPILER_ID STREQUAL "GNU") AND
  (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 4.8) AND
  (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 5.0))

  c4_install_files(
      "${CMAKE_CURRENT_LIST_DIR}/cmake/compat/c4/gcc-4.8.hpp"
      "include"
      "${CMAKE_CURRENT_LIST_DIR}/cmake/compat")

  # c++17 compiler required
  set(C4CORE_BUILD_BENCHMARKS OFF CACHE BOOL "" FORCE)
  # LLVM required
  set(C4CORE_SANITIZE OFF CACHE BOOL "" FORCE)
endif()
