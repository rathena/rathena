# - Returns a version string from Git tags
#
# This function inspects the annotated git tags for the project and returns a string
# into a CMake variable
# higly adapted from https://raw.githubusercontent.com/google/benchmark/master/cmake/GetGitVersion.cmake
# lighta
#
#  get_git_version()
#
# - Example
#
# include(GetGitVersion)
# get_git_version(GIT_VERSION)
# return 
#     - GIT_VERSION : remote tracking sha of master
#     - GIT_HEAD_VERSION : current sha of current branch
#
# Requires CMake 2.8.11+
find_package(Git)

if(__get_git_version)
  return()
endif()
set(__get_git_version INCLUDED)

function(get_git_version)
  if(GIT_EXECUTABLE)
      #determine remote tracking master sha
      execute_process(COMMAND ${GIT_EXECUTABLE} rev-parse origin/master
          RESULT_VARIABLE status
          OUTPUT_VARIABLE GIT_VERSION
          ERROR_QUIET)
      if(${status})
        set(GIT_VERSION "unknow")
      else()
        string(STRIP ${GIT_VERSION} GIT_VERSION)
      endif()
    
      #determine current head sha
      execute_process(COMMAND ${GIT_EXECUTABLE} rev-parse --verify HEAD
          RESULT_VARIABLE status
          OUTPUT_VARIABLE GIT_HEAD_VERSION
          ERROR_QUIET)
      if(${status})
        set(GIT_HEAD_VERSION "unknow")
      else()
        string(STRIP ${GIT_HEAD_VERSION} GIT_HEAD_VERSION)
      endif()   

  endif()
  message("-- git Version: ${GIT_VERSION}, ${GIT_HEAD_VERSION}")
  set(GIT_VERSION ${GIT_VERSION} PARENT_SCOPE)
  set(GIT_HEAD_VERSION ${GIT_HEAD_VERSION} PARENT_SCOPE)
endfunction()
