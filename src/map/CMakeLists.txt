
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
	"${COMMON_SOURCE_DIR}/grfio.c"
	"${COMMON_SOURCE_DIR}/lock.c"
	"${COMMON_SOURCE_DIR}/malloc.c"
	"${COMMON_SOURCE_DIR}/mapindex.c"
	"${COMMON_SOURCE_DIR}/md5calc.c"
	"${COMMON_SOURCE_DIR}/nullpo.c"
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
set( MAP_SOURCES
	"${CMAKE_CURRENT_SOURCE_DIR}/atcommand.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/battle.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/battleground.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/buyingstore.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/chat.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/chrif.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/clif.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/date.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/duel.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/guild.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/homunculus.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/instance.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/intif.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/itemdb.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/log.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/mail.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/map.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/mercenary.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/mob.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/npc.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/npc_chat.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/party.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/path.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/pc.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/pet.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/quest.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/script.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/searchstore.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/skill.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/status.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/storage.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/trade.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/unit.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/vending.c"
	)
set( MAP_TXT_SOURCES
	"${CMAKE_CURRENT_SOURCE_DIR}/mapreg_txt.c"
	)
set( MAP_SQL_SOURCES
	"${CMAKE_CURRENT_SOURCE_DIR}/mapreg_sql.c"
	)


#
# targets
#
add_subdirectory( txt )
add_subdirectory( sql )
