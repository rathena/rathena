// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "int_clan.hpp"

#include <stdlib.h>
#include <string.h> //memset

#include "../common/cbasetypes.hpp"
#include "../common/malloc.hpp"
#include "../common/mmo.hpp"
#include "../common/showmsg.hpp"
#include "../common/socket.hpp"
#include "../common/strlib.hpp"

#include "char.hpp"
#include "char_mapif.hpp"
#include "inter.hpp"

//clan cache
static DBMap* clan_db; // int clan_id -> struct clan*

int inter_clan_removemember_tosql(uint32 account_id, uint32 char_id){
	if( SQL_ERROR == Sql_Query( sql_handle, "UPDATE `%s` SET `clan_id` = '0' WHERE `char_id` = '%d'", schema_config.char_db, char_id ) ){
		Sql_ShowDebug( sql_handle );
		return 1;
	}else{
		return 0;
	}
}

struct clan* inter_clan_fromsql(int clan_id){
	struct clan* clan;
	char* data;
	size_t len;
	int i;

	if( clan_id <= 0 )
		return NULL;

	clan = (struct clan*)idb_get(clan_db, clan_id);

	if( clan ){
		return clan;
	}

	if( SQL_ERROR == Sql_Query(sql_handle, "SELECT `name`, `master`, `mapname`, `max_member` FROM `%s` WHERE `clan_id`='%d'", schema_config.clan_table, clan_id) ){
		Sql_ShowDebug(sql_handle);
		return NULL;
	}

	if( SQL_SUCCESS != Sql_NextRow(sql_handle) ){
		return NULL;// Clan does not exists.
	}

	CREATE(clan, struct clan, 1);
	memset(clan, 0, sizeof(struct clan));

	clan->id = clan_id;
	Sql_GetData(sql_handle,  0, &data, &len); memcpy(clan->name, data, min(len, NAME_LENGTH));
	Sql_GetData(sql_handle,  1, &data, &len); memcpy(clan->master, data, min(len, NAME_LENGTH));
	Sql_GetData(sql_handle,  2, &data, &len); memcpy(clan->map, data, min(len, MAP_NAME_LENGTH_EXT));
	Sql_GetData(sql_handle,  3, &data, NULL); clan->max_member = atoi(data);

	clan->connect_member = 0;

	Sql_FreeResult( sql_handle );

	if( clan->max_member > MAX_CLAN ){
		ShowWarning("Clan %d:%s specifies higher capacity (%d) than MAX_CLAN (%d)\n", clan_id, clan->name, clan->max_member, MAX_CLAN);
		clan->max_member = MAX_CLAN;
	}

	if( SQL_ERROR == Sql_Query(sql_handle, "SELECT `opposition`,`alliance_id`,`name` FROM `%s` WHERE `clan_id`='%d'", schema_config.clan_alliance_table, clan_id) ){
		Sql_ShowDebug(sql_handle);
		aFree(clan);
		return NULL;
	}

	for( i = 0; i < MAX_CLANALLIANCE && SQL_SUCCESS == Sql_NextRow(sql_handle); i++ ){
		struct clan_alliance* a = &clan->alliance[i];

		Sql_GetData(sql_handle, 0, &data, NULL); a->opposition = atoi(data);
		Sql_GetData(sql_handle, 1, &data, NULL); a->clan_id = atoi(data);
		Sql_GetData(sql_handle, 2, &data, &len); memcpy(a->name, data, zmin(len, NAME_LENGTH));
	}

	idb_put( clan_db, clan_id, clan);

	if (charserv_config.save_log)
		ShowInfo("Clan loaded (%d - %s)\n", clan_id, clan->name);

	return clan;
}

int mapif_clan_info( int fd ){
	DBIterator *iter;
	int offset;
	struct clan* clan;
	int length;

	length = 4 + db_size(clan_db) * sizeof( struct clan );

	WFIFOHEAD( fd, length );
	WFIFOW( fd, 0 ) = 0x38A0;
	WFIFOW( fd, 2 ) = length;

	offset = 4;
	iter = db_iterator(clan_db);
	for( clan = (struct clan*)dbi_first(iter); dbi_exists(iter); clan = (struct clan*)dbi_next(iter) ){
		memcpy( WFIFOP( fd, offset ), clan, sizeof( struct clan ) );
		offset += sizeof( struct clan );
	}
	dbi_destroy(iter);

	WFIFOSET( fd, length );

	return 0;
}

static int mapif_parse_clan_request( int fd ){
	mapif_clan_info( fd );

	return 0;
}

static int mapif_parse_clan_message( int fd ){
	unsigned char buf[500];
	uint16 len;

	len = RFIFOW(fd,2);

	WBUFW(buf,0) = 0x38A1;
	memcpy( WBUFP(buf,2), RFIFOP(fd,2), len-2 );

	chmapif_sendallwos( fd, buf, len );

	return 0;
}

static void mapif_clan_refresh_onlinecount( int fd, struct clan* clan ){
	unsigned char buf[8];

	WBUFW(buf,0) = 0x38A2;
	WBUFL(buf,2) = clan->id;
	WBUFW(buf,6) = clan->connect_member;

	chmapif_sendallwos( fd, buf, 8 );
}

static void mapif_parse_clan_member_left( int fd ){
	struct clan* clan = (struct clan*)idb_get(clan_db, RFIFOL(fd,2) );

	if( clan == NULL ){ // Unknown clan
		return;
	}

	if( clan->connect_member > 0 ){
		clan->connect_member--;

		mapif_clan_refresh_onlinecount( fd, clan );
	}
}

static void mapif_parse_clan_member_joined( int fd ){
	struct clan* clan = (struct clan*)idb_get(clan_db, RFIFOL(fd,2) );

	if( clan == NULL ){ // Unknown clan
		return;
	}

	clan->connect_member++;

	mapif_clan_refresh_onlinecount( fd, clan );
}

// Communication from the map server
// - Can analyzed only one by one packet
// Data packet length that you set to inter.cpp
//- Shouldn't do checking and packet length, RFIFOSKIP is done by the caller
// Must Return
//	1 : ok
//  0 : error
int inter_clan_parse_frommap( int fd ){
	RFIFOHEAD(fd);
	switch(RFIFOW(fd,0)) {
		case 0x30A0:
			mapif_parse_clan_request( fd );
			return 1;
		case 0x30A1:
			mapif_parse_clan_message( fd );
			return 1;
		case 0x30A2:
			mapif_parse_clan_member_left( fd );
			return 1;
		case 0x30A3:
			mapif_parse_clan_member_joined( fd );
			return 1;
		default:
			return 0;
	}
}

// Initialize clan sql
int inter_clan_init(void){
	char* data;
	int* clan_ids;
	int amount;

	clan_db = idb_alloc(DB_OPT_RELEASE_DATA);

	if( SQL_ERROR == Sql_Query( sql_handle, "SELECT `clan_id` FROM `%s`", schema_config.clan_table ) ){
		Sql_ShowDebug(sql_handle);
		return 1;
	}

	amount = (int)Sql_NumRows( sql_handle );

	if( amount > 0 ){
		int i;

		CREATE( clan_ids, int, amount );
		i = 0;

		while( SQL_SUCCESS == Sql_NextRow(sql_handle) ){
			Sql_GetData(sql_handle,  0, &data, NULL);
			clan_ids[i++] = atoi(data);
		}

		Sql_FreeResult( sql_handle );

		// If we didnt load a row as expected
		amount = i;

		for( i = 0; i < amount; i++ ){
			inter_clan_fromsql( clan_ids[i] );
		}

		aFree(clan_ids);
	}

	return 0;
}

void inter_clan_final(){
	db_destroy(clan_db);
}
