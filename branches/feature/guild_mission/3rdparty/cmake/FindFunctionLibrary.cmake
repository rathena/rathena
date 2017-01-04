# - Check which library is needed to link a C function
# find_function_library( <function> <variable> [<library> ...] )
#
# Check which library provides the <function>.
# Sets <variable> to 0 if found in the global libraries.
# Sets <variable> to the library path if found in the provided libraries.
# Raises a FATAL_ERROR if not found.
#
# The following variables may be set before calling this macro to
# modify the way the check is run:
#
#  CMAKE_REQUIRED_FLAGS = string of compile command line flags
#  CMAKE_REQUIRED_DEFINITIONS = list of macros to define (-DFOO=bar)
#  CMAKE_REQUIRED_INCLUDES = list of include directories
#  CMAKE_REQUIRED_LIBRARIES = list of libraries to link
include( CheckFunctionExists )

macro( find_function_library FUNC VAR )
	if( "${VAR}" MATCHES "^${VAR}$" )
		CHECK_FUNCTION_EXISTS( ${FUNC} ${VAR} )
		if( ${VAR} )
			message( STATUS "Found ${FUNC} in global libraries" )
			set( ${VAR} 0 CACHE INTERNAL "Found ${FUNC} in global libraries" )# global
		else()
			foreach( LIB IN ITEMS ${ARGN} )
				message( STATUS "Looking for ${FUNC} in ${LIB}" )
				find_library( ${LIB}_LIBRARY ${LIB} )
				mark_as_advanced( ${LIB}_LIBRARY )
				if( ${LIB}_LIBRARY )
					unset( ${VAR} CACHE )
					set( CMAKE_REQUIRED_LIBRARIES ${${LIB}_LIBRARY} )
					CHECK_FUNCTION_EXISTS( ${FUNC} ${VAR} )
					set( CMAKE_REQUIRED_LIBRARIES )
					if( ${VAR} )
						message( STATUS "Found ${FUNC} in ${LIB}: ${${LIB}_LIBRARY}" )
						set( ${VAR} ${${LIB}_LIBRARY} CACHE INTERNAL "Found ${FUNC} in ${LIB}" )# lib
						break()
					endif()
				endif()
			endforeach()
			if( NOT ${VAR} )
				message( FATAL_ERROR "Function ${FUNC} not found" )
			endif()
		endif()
	endif()
endmacro( find_function_library )

