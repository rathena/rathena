#
# Find svnversion
#
function(get_svn_version)
  message( STATUS "Detecting svnversion" )
  find_program( SVNVERSION_EXECUTABLE svnversion )
  mark_as_advanced( SVNVERSION_EXECUTABLE )
  if( SVNVERSION_EXECUTABLE )
    message( STATUS "Found svnversion: ${SVNVERSION_EXECUTABLE}" )
  endif()
  message( STATUS "Detecting svnversion - done" )
  #
  # Find Subversion
  #
  message( STATUS "Detecting Subversion" )
  find_package( Subversion )
  message( STATUS "Detecting Subversion - done" )
  #
  # SVNVERSION
  #
  if( SVNVERSION_EXECUTABLE )
    message( STATUS "Getting svn version" )
    execute_process( COMMAND ${SVNVERSION_EXECUTABLE} ${PROJECT_SOURCE_DIR}
      OUTPUT_VARIABLE SVNVERSION
      OUTPUT_STRIP_TRAILING_WHITESPACE )
    if( SVNVERSION MATCHES "^Unversioned" )
      set( SVNVERSION )
    endif()
    string( REGEX REPLACE "[^1234567890MSexported]" "_" SVNVERSION "${SVNVERSION}" )
    message( STATUS "Found SVNversion: ${SVNVERSION}" )
    message( STATUS "Getting svn version - done" )
  endif()
  if( Subversion_FOUND AND SVNVERSION )
    message( STATUS "Getting svn branch" )
    Subversion_WC_INFO( ${PROJECT_SOURCE_DIR} rAthena )
    if( rAthena_WC_URL )
      string( REGEX MATCH "[^/]+$" BRANCH ${rAthena_WC_URL} )
      set( SVNVERSION "${BRANCH}-${SVNVERSION}" )
      message( STATUS "Found branch: ${BRANCH}" )
    endif()
    message( STATUS "Getting svn branch - done" )
  endif()
  set(SVNVERSION ${SVNVERSION} PARENT_SCOPE)
endfunction()
