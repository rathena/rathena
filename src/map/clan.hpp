// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef CLAN_HPP
#define CLAN_HPP

#include <common/mmo.hpp>

struct clan;
class map_session_data;

void do_init_clan();
void do_final_clan();
struct clan* clan_search( int id );
struct clan* clan_searchname( const char* name );
void clan_load_clandata( int count, struct clan* clans );
void clan_member_joined( map_session_data* sd );
void clan_member_left( map_session_data* sd );
bool clan_member_join( map_session_data *sd, int clan_id, uint32 account_id, uint32 char_id );
bool clan_member_leave( map_session_data* sd, int clan_id, uint32 account_id, uint32 char_id );
void clan_send_message( map_session_data *sd, const char *mes, int len );
void clan_recv_message(int clan_id,uint32 account_id,const char *mes,int len);
map_session_data* clan_getavailablesd( struct clan* clan );
int clan_get_alliance_count( struct clan *clan, int flag );

#endif /* CLAN_HPP */
