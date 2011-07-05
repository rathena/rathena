
#
# setup
#
set( MT19937AR_SOURCES
	"${MT19937AR_SOURCE_DIR}/mt19937ar.c"
	)
set( COMMON_SOURCES
	"${COMMON_SOURCE_DIR}/core.c"
	"${COMMON_SOURCE_DIR}/db.c"
	"${COMMON_SOURCE_DIR}/ers.c"
#	"${COMMON_SOURCE_DIR}/grfio.c"
	"${COMMON_SOURCE_DIR}/lock.c"
	"${COMMON_SOURCE_DIR}/malloc.c"
#	"${COMMON_SOURCE_DIR}/mapindex.c"
	"${COMMON_SOURCE_DIR}/md5calc.c"
#	"${COMMON_SOURCE_DIR}/nullpo.c"
	"${COMMON_SOURCE_DIR}/plugins.c"
	"${COMMON_SOURCE_DIR}/random.c"
	"${COMMON_SOURCE_DIR}/showmsg.c"
	"${COMMON_SOURCE_DIR}/socket.c"
	"${COMMON_SOURCE_DIR}/strlib.c"
	"${COMMON_SOURCE_DIR}/timer.c"
	"${COMMON_SOURCE_DIR}/utils.c"
	)
set( COMMON_SQL_SOURCES
	"${COMMON_SOURCE_DIR}/sql.c"
	)
set( LOGIN_SOURCES
	"${CMAKE_CURRENT_SOURCE_DIR}/login.c"
	)
set( LOGIN_TXT_SOURCES
	"${CMAKE_CURRENT_SOURCE_DIR}/account_txt.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/ipban_txt.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/loginlog_txt.c"
	)
set( LOGIN_SQL_SOURCES
	"${CMAKE_CURRENT_SOURCE_DIR}/account_sql.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/ipban_sql.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/loginlog_sql.c"
	)


#
# targets
#
add_subdirectory( txt )
add_subdirectory( sql )
