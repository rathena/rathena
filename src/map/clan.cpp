// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "clan.hpp"

#include <cstring> //memset

#include <common/cbasetypes.hpp>
#include <common/malloc.hpp>
#include <common/mmo.hpp>
#include <common/nullpo.hpp>
#include <common/showmsg.hpp>

#include "battle.hpp"
#include "clif.hpp"
#include "instance.hpp"
#include "intif.hpp"
#include "log.hpp"
#include "pc.hpp"
#include "script.hpp"
#include "status.hpp"

static DBMap* clan_db; // int32 clan_id -> struct clan*

void do_init_clan(){
	clan_db = idb_alloc(DB_OPT_RELEASE_DATA);
}

void do_final_clan(){
	db_destroy(clan_db);
}

void clan_load_clandata( int32 count, struct clan* clans ){
	int32 i,j;

	nullpo_retv( clans );

	for( i = 0, j = 0; i < count; i++, clans++ ){
		struct clan* clan = clans;
		struct clan* clanCopy;

		clanCopy = (struct clan*)aMalloc( sizeof( struct clan ) );

		if( clanCopy == nullptr ){
			ShowError("Memory could not be allocated for a clan.\n");
			break;
		}

		memcpy( clanCopy, clan, sizeof( struct clan ) );
		memset( clanCopy->members, 0, sizeof( clanCopy->members ) );

		idb_put( clan_db, clanCopy->id, clanCopy );
		j++;
	}

	ShowStatus( "Received '" CL_WHITE "%d" CL_RESET "' clans from char-server.\n", j );
}

struct clan* clan_search( int32 id ){
	return (struct clan*)idb_get(clan_db,id);
}

struct clan* clan_searchname( const char* name ){
	struct clan* c;

	DBIterator *iter = db_iterator(clan_db);
	for( c = (struct clan*)dbi_first(iter); dbi_exists(iter); c = (struct clan*)dbi_next(iter) ){
		if( strncmpi( c->name, name, NAME_LENGTH ) == 0 ){
			break;
		}
	}
	dbi_destroy(iter);

	return c;
}

map_session_data* clan_getavailablesd( struct clan& clan ){
	int32 i;

	ARR_FIND( 0, clan.max_member, i, clan.members[i] != nullptr );

	return ( i < clan.max_member ) ? clan.members[i] : nullptr;
}

int32 clan_getMemberIndex( struct clan* clan, uint32 account_id ){
	int32 i;

	nullpo_retr(-1,clan);

	ARR_FIND( 0, clan->max_member, i, clan->members[i] != nullptr && clan->members[i]->status.account_id == account_id );

	if( i == clan->max_member ){
		return -1;
	}else{
		return i;
	}
}

int32 clan_getNextFreeMemberIndex( struct clan* clan ){
	int32 i;

	nullpo_retr(-1,clan);

	ARR_FIND( 0, clan->max_member, i, clan->members[i] == nullptr );

	if( i == clan->max_member ){
		return -1;
	}else{
		return i;
	}
}

void clan_member_joined( map_session_data& sd ){
	if( sd.clan != nullptr ){
		clif_clan_basicinfo( sd );
		clif_clan_onlinecount( *sd.clan );
		return;
	}

	struct clan* clan = clan_search( sd.status.clan_id );

	if( clan == nullptr ){
		return;
	}

	int32 index = clan_getNextFreeMemberIndex( clan );

	if( index >= 0 ){
		sd.clan = clan;
		clan->members[index] = &sd;
		clan->connect_member++;

		clif_clan_basicinfo(sd);

		intif_clan_member_joined(clan->id);
		clif_clan_onlinecount( *clan );
	}
}

void clan_member_left( map_session_data& sd ){
	struct clan* clan = sd.clan;

	if( clan == nullptr ){
		return;
	}

	int32 index = clan_getMemberIndex( clan, sd.status.account_id );

	if( index >= 0 ){
		clan->members[index] = nullptr;
		clan->connect_member--;

		intif_clan_member_left(clan->id);
		clif_clan_onlinecount( *clan );
	}
}

bool clan_member_join( map_session_data& sd, int32 clan_id, uint32 account_id, uint32 char_id ){
	struct clan *clan = clan_search( clan_id );

	if( clan == nullptr ){
		return false;
	}

	if( sd.status.account_id != account_id || sd.status.char_id != char_id || sd.status.clan_id != 0 ){
		return false;
	}

	if( clan->instance_id > 0 && battle_config.instance_block_invite ){
		return false;
	}

	sd.status.clan_id = clan->id;

	clan_member_joined(sd);

	return true;
}

bool clan_member_leave( map_session_data& sd, int32 clan_id, uint32 account_id, uint32 char_id ){
	if( sd.status.account_id != account_id || sd.status.char_id != char_id || sd.status.clan_id != clan_id ){
		return false;
	}

	struct clan* clan = sd.clan;

	if( clan == nullptr ){
		return false;
	}

	if( clan->instance_id > 0 && battle_config.instance_block_leave ){
		return false;
	}

	clan_member_left(sd);

	sd.clan = nullptr;
	sd.status.clan_id = 0;

	clif_clan_leave(sd);

	return true;
}

void clan_recv_message( int32 clan_id, uint32 account_id, const char *mes, size_t len ){
	struct clan *clan = clan_search( clan_id );

	if( clan == nullptr ){
		return;
	}

	clif_clan_message( *clan, mes, len );
}

void clan_send_message( map_session_data& sd, const char *mes, size_t len ){
	if( sd.clan == nullptr ){
		return;
	}

	intif_clan_message( sd.status.clan_id, sd.status.account_id, mes, len );
	clan_recv_message( sd.status.clan_id, sd.status.account_id, mes, len );
	log_chat( LOG_CHAT_CLAN, sd.status.clan_id, sd.status.char_id, sd.status.account_id, mapindex_id2name( sd.mapindex ), sd.x, sd.y, nullptr, mes );
}

int32 clan_get_alliance_count( struct clan& clan, int32 flag ){
	int32 count = 0;

	for( int32 i = 0; i < MAX_CLANALLIANCE; i++ ){
		if(	clan.alliance[i].clan_id > 0 && clan.alliance[i].opposition == flag ){
			count++;
		}
	}

	return count;
}
