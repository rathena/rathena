include(CheckCSourceRuns)
include(CheckFunctionExists)

#
# Test monotonic clock
#
# CLOCK_MONOTONIC clock for clock_gettime
# Normally defines _POSIX_TIMERS > 0 and _POSIX_MONOTONIC_CLOCK (for posix
# compliant systems) and __FreeBSD_cc_version >= 500005 (for FreeBSD
# >= 5.1.0, which does not have the posix defines (ref. r11983)) would be
# checked but some systems define them even when they do not support it
# (ref. bugreport:1003).
#
message(STATUS "Check for monotonic clock")
find_library(RT_LIBRARY rt)# (optional, rt on Debian)
mark_as_advanced(RT_LIBRARY)
set(CMAKE_REQUIRED_LIBRARIES ${GLOBAL_LIBRARIES} ${RT_LIBRARY})
file(READ "${CMAKE_SOURCE_DIR}/3rdparty/cmake/tests/HAVE_MONOTONIC_CLOCK.c" _SOURCE)
CHECK_C_SOURCE_RUNS("${_SOURCE}" HAVE_MONOTONIC_CLOCK)
if(HAVE_MONOTONIC_CLOCK)
	message(STATUS "Check for monotonic clock - yes")
	set(MONOTONIC_CLOCK_LIBRARIES ${RT_LIBRARY})
	set(MONOTONIC_CLOCK_DEFINITIONS "-DHAVE_MONOTONIC_CLOCK")
else()
	message(STATUS "Check for monotonic clock - no")
endif()

# Check ENABLE_MEMORY option
if(ENABLE_MEMORY STREQUAL "system")
	# use the normal system functions
else()
	message(STATUS "Feature not implemented, defaulting to system")
endif()


add_library(minicore EXCLUDE_FROM_ALL)
add_library(common)
# add_library(core-tools)
add_library(common-win INTERFACE)

if(WIN32)
	target_link_libraries(common-win INTERFACE "ws2_32.lib")

	if(MSVC)
		target_compile_definitions(common-win INTERFACE
			_WINSOCK_DEPRECATED_NO_WARNINGS _CRT_SECURE_NO_DEPRECATE _CRT_NONSTDC_NO_DEPRECATE)
	endif()

	target_sources(common-win INTERFACE
		"${CMAKE_CURRENT_SOURCE_DIR}/winapi.cpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/winapi.hpp"
	)

	target_link_libraries(minicore PUBLIC common-win)
	target_link_libraries(common PUBLIC common-win)
endif()

target_sources(minicore PRIVATE
	"core.cpp"
	"database.cpp"
	"des.cpp"
	"grfio.cpp"
	"malloc.cpp"
	"nullpo.cpp"
	"showmsg.cpp"
	"strlib.cpp"
	"utils.cpp"
)

if(WIN32)
	target_sources(minicore PRIVATE
		"cbasetypes.hpp"
		"core.hpp"
		"database.hpp"
		"des.hpp"
		"grfio.hpp"
		"malloc.hpp"
		"nullpo.hpp"
		"showmsg.hpp"
		"strlib.hpp"
		"utils.hpp"
	)
	set_target_properties(minicore PROPERTIES FOLDER "Core")
endif()

target_compile_definitions(minicore PRIVATE
	"-DMINICORE"
	${MONOTONIC_CLOCK_DEFINITIONS}
)

target_include_directories(minicore PUBLIC
	"${RA_INCLUDE_DIRS}"
	"${ZLIB_INCLUDE_DIRS}"
)

target_link_libraries(minicore PUBLIC
	${MONOTONIC_CLOCK_LIBRARIES}
	libconfig
	ryml
	${ZLIB_LIBRARIES}
	yaml-cpp
)

target_sources(common PRIVATE
	"cli.cpp"
	"conf.cpp"
	"core.cpp"
	"database.cpp"
	"db.cpp"
	"des.cpp"
	"ers.cpp"
	"grfio.cpp"
	"malloc.cpp"
	"mapindex.cpp"
	"md5calc.cpp"
	"msg_conf.cpp"
	"nullpo.cpp"
	"random.cpp"
	"showmsg.cpp"
	"socket.cpp"
	"sql.cpp"
	"strlib.cpp"
	"timer.cpp"
	"utils.cpp"
	"utilities.cpp"
)

if(WIN32)
	target_sources(common PRIVATE
		"cbasetypes.hpp"
		"cli.hpp"
		"conf.hpp"
		"core.hpp"
		"database.hpp"
		"db.hpp"
		"des.hpp"
		"ers.hpp"
		"grfio.hpp"
		"malloc.hpp"
		"mapindex.hpp"
		"md5calc.hpp"
		"mmo.hpp"
		"msg_conf.hpp"
		"nullpo.hpp"
		"random.hpp"
		"showmsg.hpp"
		"socket.hpp"
		"sql.hpp"
		"strlib.hpp"
		"timer.hpp"
		"utilities.hpp"
		"utils.hpp"
	)
	set_target_properties(common PROPERTIES FOLDER "Core")
endif()

# defines just for common
target_compile_definitions(common PRIVATE ${MONOTONIC_CLOCK_DEFINITIONS})

if(ENABLE_RDTSC)
	target_compile_definitions(common PRIVATE "-DENABLE_RDTSC")
endif()


if(ENABLE_MEMMGR)
	target_compile_definitions(common PRIVATE "-DUSE_MEMMGR")
else()
	target_compile_definitions(common PRIVATE "-DNO_MEMMGR")
endif()

# Propagated defines
target_compile_definitions(common PUBLIC "-DPACKETVER=${PACKETVER}")

if(ENABLE_PRERENEWAL)
	target_compile_definitions(common PUBLIC "-DPRERE")
endif()

if(ENABLE_EXTRA_DEBUG_CODE)
	target_compile_definitions(common PUBLIC "-DDEBUG")
endif()

if(ENABLE_EXTRA_BUILDBOT_CODE)
	target_compile_definitions(common PUBLIC "-DBUILDBOT")
endif()

if(ENABLE_EPOLL)
	target_compile_definitions(common PRIVATE "-DSOCKET_EPOLL")
endif()

if(ENABLE_VIP)
	target_compile_definitions(common INTERFACE "-DVIP_ENABLE")
endif()

if(MAXCONN)
    target_compile_definitions(common PUBLIC "-DMAXCONN=${MAXCONN}")
endif()

CHECK_FUNCTION_EXISTS(setrlimit HAVE_SETRLIMIT)
if (HAVE_SETRLIMIT)
	target_compile_definitions(common PUBLIC "-DHAVE_SETRLIMIT")
endif()

CHECK_FUNCTION_EXISTS(strnlen HAVE_STRNLEN)
if (HAVE_STRNLEN)
	target_compile_definitions(common PUBLIC "-DHAVE_STRNLEN")
endif()

target_include_directories(common PUBLIC
	"${MYSQL_INCLUDE_DIRS}"
	"${ZLIB_INCLUDE_DIRS}"
	"${RA_INCLUDE_DIRS}"
)

target_link_libraries(common PUBLIC
	${MONOTONIC_CLOCK_LIBRARIES}
	libconfig
	ryml
	${MYSQL_LIBRARIES}
	${ZLIB_LIBRARIES}
)


if(ENABLE_PROFILER STREQUAL "gprof")
	target_compile_options(common PUBLIC "-pg")
	target_link_libraries(common PUBLIC "-pg")
endif()
