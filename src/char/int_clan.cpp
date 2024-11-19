// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "int_clan.hpp"

#include <cstdlib>
#include <cstring> //memset
#include <memory>
#include <unordered_map>
#include <vector>

#include <common/cbasetypes.hpp>
#include <common/malloc.hpp>
#include <common/mmo.hpp>
#include <common/showmsg.hpp>
#include <common/socket.hpp>
#include <common/strlib.hpp>

#include "char.hpp"
#include "char_mapif.hpp"
#include "inter.hpp"

// int32 clan_id -> struct clan*
static std::unordered_map<int32, std::shared_ptr<struct clan>> clan_db;

using namespace rathena;

int32 inter_clan_removemember_tosql(uint32 account_id, uint32 char_id){
	if( SQL_ERROR == Sql_Query( sql_handle, "UPDATE `%s` SET `clan_id` = '0' WHERE `char_id` = '%d'", schema_config.char_db, char_id ) ){
		Sql_ShowDebug( sql_handle );
		return 1;
	}else{
		return 0;
	}
}

std::shared_ptr<struct clan> inter_clan_fromsql(int32 clan_id){
	char* data;
	size_t len;
	int32 i;

	if( clan_id <= 0 )
		return nullptr;

	std::shared_ptr<struct clan> clan = util::umap_find( clan_db, clan_id );

	if( clan ){
		return clan;
	}

	if( SQL_ERROR == Sql_Query(sql_handle, "SELECT `name`, `master`, `mapname`, `max_member` FROM `%s` WHERE `clan_id`='%d'", schema_config.clan_table, clan_id) ){
		Sql_ShowDebug(sql_handle);
		return nullptr;
	}

	if( SQL_SUCCESS != Sql_NextRow(sql_handle) ){
		return nullptr; // Clan does not exists.
	}

	clan = std::make_shared<struct clan>();

	clan->id = clan_id;
	Sql_GetData( sql_handle,  0, &data, &len );
	memcpy( clan->name, data, std::min( len, static_cast<decltype(len)>( NAME_LENGTH ) ) );
	Sql_GetData( sql_handle,  1, &data, &len );
	memcpy( clan->master, data, std::min( len, static_cast<decltype(len)>( NAME_LENGTH ) ) );
	Sql_GetData( sql_handle,  2, &data, &len );
	memcpy( clan->map, data, std::min( len, static_cast<decltype(len)>( MAP_NAME_LENGTH_EXT ) ) );
	Sql_GetData(sql_handle,  3, &data, nullptr); clan->max_member = atoi(data);

	clan->connect_member = 0;

	Sql_FreeResult( sql_handle );

	if( clan->max_member > MAX_CLAN ){
		ShowWarning("Clan %d:%s specifies higher capacity (%d) than MAX_CLAN (%d)\n", clan_id, clan->name, clan->max_member, MAX_CLAN);
		clan->max_member = MAX_CLAN;
	}

	if( SQL_ERROR == Sql_Query(sql_handle, "SELECT `opposition`,`alliance_id`,`name` FROM `%s` WHERE `clan_id`='%d'", schema_config.clan_alliance_table, clan_id) ){
		Sql_ShowDebug(sql_handle);
		return nullptr;
	}

	for( i = 0; i < MAX_CLANALLIANCE && SQL_SUCCESS == Sql_NextRow(sql_handle); i++ ){
		struct clan_alliance* a = &clan->alliance[i];

		Sql_GetData(sql_handle, 0, &data, nullptr); a->opposition = atoi(data);
		Sql_GetData(sql_handle, 1, &data, nullptr); a->clan_id = atoi(data);
		Sql_GetData(sql_handle, 2, &data, &len); memcpy(a->name, data, zmin(len, NAME_LENGTH));
	}

	clan_db[clan->id] = clan;

	if (charserv_config.save_log)
		ShowInfo("Clan loaded (%d - %s)\n", clan_id, clan->name);

	return clan;
}

int32 mapif_clan_info( int32 fd ){
	size_t offset = 4;
	size_t length = offset + clan_db.size() * sizeof( struct clan );

	WFIFOHEAD( fd, length );
	WFIFOW( fd, 0 ) = 0x38A0;
	WFIFOW( fd, 2 ) = static_cast<int16>( length );

	for( const auto& pair : clan_db ){
		std::shared_ptr<struct clan> clan = pair.second;

		memcpy( WFIFOP( fd, offset ), clan.get(), sizeof( struct clan ) );
		offset += sizeof( struct clan );
	}

	WFIFOSET( fd, length );

	return 0;
}

static int32 mapif_parse_clan_request( int32 fd ){
	mapif_clan_info( fd );

	return 0;
}

static int32 mapif_parse_clan_message( int32 fd ){
	unsigned char buf[500];
	uint16 len;

	len = RFIFOW(fd,2);

	WBUFW(buf,0) = 0x38A1;
	memcpy( WBUFP(buf,2), RFIFOP(fd,2), len-2 );

	chmapif_sendallwos( fd, buf, len );

	return 0;
}

static void mapif_clan_refresh_onlinecount( int32 fd, std::shared_ptr<struct clan> clan ){
	unsigned char buf[8];

	WBUFW(buf,0) = 0x38A2;
	WBUFL(buf,2) = clan->id;
	WBUFW(buf,6) = clan->connect_member;

	chmapif_sendallwos( fd, buf, 8 );
}

static void mapif_parse_clan_member_left( int32 fd ){
	std::shared_ptr<struct clan> clan = util::umap_find( clan_db, static_cast<int32>( RFIFOL( fd, 2 ) ) );

	// Unknown clan
	if( clan == nullptr ){
		return;
	}

	if( clan->connect_member > 0 ){
		clan->connect_member--;

		mapif_clan_refresh_onlinecount( fd, clan );
	}
}

static void mapif_parse_clan_member_joined( int32 fd ){
	std::shared_ptr<struct clan> clan = util::umap_find( clan_db, static_cast<int32>( RFIFOL( fd, 2 ) ) );

	// Unknown clan
	if( clan == nullptr ){
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
int32 inter_clan_parse_frommap( int32 fd ){
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
int32 inter_clan_init(void){
	if( SQL_ERROR == Sql_Query( sql_handle, "SELECT `clan_id` FROM `%s`", schema_config.clan_table ) ){
		Sql_ShowDebug(sql_handle);
		return 1;
	}

	std::vector<int32> clan_ids;

	while( SQL_SUCCESS == Sql_NextRow( sql_handle ) ){
		char* data;
		Sql_GetData( sql_handle,  0, &data, nullptr );
		clan_ids.push_back( atoi( data ) );
	}

	Sql_FreeResult( sql_handle );

	for( int32 clan_id : clan_ids ){
		inter_clan_fromsql( clan_id );
	}

	return 0;
}

void inter_clan_final(){
	clan_db.clear();
}
