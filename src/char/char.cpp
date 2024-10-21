// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#pragma warning(disable:4800)
#include "char.hpp"

#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <memory>
#include <unordered_map>

#include <common/cbasetypes.hpp>
#include <common/cli.hpp>
#include <common/core.hpp>
#include <common/db.hpp>
#include <common/malloc.hpp>
#include <common/mapindex.hpp>
#include <common/mmo.hpp>
#include <common/random.hpp>
#include <common/showmsg.hpp>
#include <common/socket.hpp>
#include <common/strlib.hpp>
#include <common/timer.hpp>
#include <common/utilities.hpp>
#include <common/utils.hpp>

#include "char_clif.hpp"
#include "char_cnslif.hpp"
#include "char_logif.hpp"
#include "char_mapif.hpp"
#include "inter.hpp"
#include "int_elemental.hpp"
#include "int_guild.hpp"
#include "int_homun.hpp"
#include "int_mail.hpp"
#include "int_mercenary.hpp"
#include "int_party.hpp"
#include "int_storage.hpp"
#include "packets.hpp"

using namespace rathena;
using namespace rathena::server_character;

//definition of exported var declared in header
int login_fd=-1; //login file descriptor
int char_fd=-1; //char file descriptor
struct Schema_Config schema_config;
struct CharServ_Config charserv_config;
struct mmo_map_server map_server[MAX_MAP_SERVERS];
//Custom limits for the fame lists. [Skotlex]
int fame_list_size_chemist = MAX_FAME_LIST;
int fame_list_size_smith = MAX_FAME_LIST;
int fame_list_size_taekwon = MAX_FAME_LIST;
// Char-server-side stored fame lists [DracoRPG]
struct fame_list smith_fame_list[MAX_FAME_LIST];
struct fame_list chemist_fame_list[MAX_FAME_LIST];
struct fame_list taekwon_fame_list[MAX_FAME_LIST];

#define CHAR_MAX_MSG 300	//max number of msg_conf
static char* msg_table[CHAR_MAX_MSG]; // Login Server messages_conf

// check for exit signal
// 0 is saving complete
// other is char_id
unsigned int save_flag = 0;

// Advanced subnet check [LuzZza]
struct s_subnet {
	uint32 mask;
	uint32 char_ip;
	uint32 map_ip;
} subnet[16];
int subnet_count = 0;

// uint32 account_id -> struct auth_node*
std::unordered_map<uint32, std::shared_ptr<struct auth_node>> auth_db;
// uint32 account_id -> struct online_char_data*
std::unordered_map<uint32, std::shared_ptr<struct online_char_data>> online_char_db;
// uint32 char_id -> struct mmo_charstatus*
std::unordered_map<uint32, std::shared_ptr<struct mmo_charstatus>> char_db;
std::unordered_map<uint32, std::shared_ptr<struct auth_node>>& char_get_authdb() { return auth_db; }
std::unordered_map<uint32, std::shared_ptr<struct online_char_data>>& char_get_onlinedb() { return online_char_db; }
std::unordered_map<uint32, std::shared_ptr<struct mmo_charstatus>>& char_get_chardb() { return char_db; }

online_char_data::online_char_data( uint32 account_id ){
	this->account_id = account_id;
	this->char_id = -1;
	this->server = -1;
	this->fd = -1;
	this->waiting_disconnect = INVALID_TIMER;
	this->pincode_success = false;
}

void char_set_charselect(uint32 account_id) {
	std::shared_ptr<struct online_char_data> character = util::umap_find( char_get_onlinedb(), account_id );

	if( character == nullptr ){
		character = std::make_shared<struct online_char_data>( account_id );
		char_get_onlinedb()[account_id] = character;
	}

	if( character->server > -1 )
		if( map_server[character->server].users > 0 ) // Prevent this value from going negative.
			map_server[character->server].users--;

	character->char_id = -1;
	character->server = -1;

	if(character->waiting_disconnect != INVALID_TIMER) {
		delete_timer(character->waiting_disconnect, char_chardb_waiting_disconnect);
		character->waiting_disconnect = INVALID_TIMER;
	}

	chlogif_send_setacconline(account_id);

}

void char_set_char_online(int map_id, uint32 char_id, uint32 account_id) {
	//Update DB
	if( SQL_ERROR == Sql_Query(sql_handle, "UPDATE `%s` SET `online`='1', `last_login`=NOW() WHERE `char_id`='%d' LIMIT 1", schema_config.char_db, char_id) )
		Sql_ShowDebug(sql_handle);

	//Check to see for online conflicts
	std::shared_ptr<struct online_char_data> character = util::umap_find( char_get_onlinedb(), account_id );

	if( character != nullptr ){
		if( character->char_id != -1 && character->server > -1 && character->server != map_id ){
			ShowNotice("set_char_online: Character %d:%d marked in map server %d, but map server %d claims to have (%d:%d) online!\n",
				character->account_id, character->char_id, character->server, map_id, account_id, char_id);
			mapif_disconnectplayer(map_server[character->server].fd, character->account_id, character->char_id, 2);
		}

		// Get rid of disconnect timer
		if( character->waiting_disconnect != INVALID_TIMER ){
			delete_timer( character->waiting_disconnect, char_chardb_waiting_disconnect );
			character->waiting_disconnect = INVALID_TIMER;
		}
	}else{
		character = std::make_shared<struct online_char_data>( account_id );
		char_get_onlinedb()[account_id] = character;
	}

	//Update state data
	character->char_id = char_id;
	character->server = map_id;

	if( character->server > -1 )
		map_server[character->server].users++;

	//Set char online in guild cache. If char is in memory, use the guild id on it, otherwise seek it.
	std::shared_ptr<struct mmo_charstatus> cp = util::umap_find( char_get_chardb(), char_id );

	inter_guild_CharOnline(char_id, cp?cp->guild_id:-1);

	//Notify login server
	chlogif_send_setacconline(account_id);
}

void char_set_char_offline(uint32 char_id, uint32 account_id){
	if ( char_id == -1 )
	{
		if( SQL_ERROR == Sql_Query(sql_handle, "UPDATE `%s` SET `online`='0' WHERE `account_id`='%d'", schema_config.char_db, account_id) )
			Sql_ShowDebug(sql_handle);
	}
	else
	{
		std::shared_ptr<struct mmo_charstatus> cp = util::umap_find( char_get_chardb(), char_id );

		inter_guild_CharOffline(char_id, cp?cp->guild_id:-1);
		if (cp)
			char_get_chardb().erase( char_id );

		if( SQL_ERROR == Sql_Query(sql_handle, "UPDATE `%s` SET `online`='0' WHERE `char_id`='%d' LIMIT 1", schema_config.char_db, char_id) )
			Sql_ShowDebug(sql_handle);
	}

	std::shared_ptr<struct online_char_data> character = util::umap_find( char_get_onlinedb(), account_id );

	// We don't free yet to avoid aCalloc/aFree spamming during char change. [Skotlex]
	if( character != nullptr ){	
		if( character->server > -1 )
			if( map_server[character->server].users > 0 ) // Prevent this value from going negative.
				map_server[character->server].users--;

		if(character->waiting_disconnect != INVALID_TIMER){
			delete_timer(character->waiting_disconnect, char_chardb_waiting_disconnect);
			character->waiting_disconnect = INVALID_TIMER;
		}

		if(character->char_id == char_id)
		{
			character->char_id = -1;
			character->server = -1;
			// needed if player disconnects completely since Skotlex did not want to free the session
			character->pincode_success = false;
		}

		//FIXME? Why Kevin free'd the online information when the char was effectively in the map-server?
	}

	//Remove char if 1- Set all offline, or 2- character is no longer connected to char-server.
	if (char_id == -1 || character == nullptr || character->fd == -1){
		chlogif_send_setaccoffline(login_fd,account_id);
	}
}

void char_db_setoffline( std::shared_ptr<struct online_char_data> character, int server ){
	if (server == -1) {
		character->char_id = -1;
		character->server = -1;
		if(character->waiting_disconnect != INVALID_TIMER){
			delete_timer(character->waiting_disconnect, char_chardb_waiting_disconnect);
			character->waiting_disconnect = INVALID_TIMER;
		}
	} else if (character->server == server)
		character->server = -2; //In some map server that we aren't connected to.
}

void char_db_kickoffline( std::shared_ptr<struct online_char_data> character, int server_id ){
	if (server_id > -1 && character->server != server_id)
		return;

	//Kick out any connected characters, and set them offline as appropriate.
	if (character->server > -1)
		mapif_disconnectplayer(map_server[character->server].fd, character->account_id, character->char_id, 1);
	else if (character->waiting_disconnect == INVALID_TIMER)
		char_set_char_offline(character->char_id, character->account_id);
}

void char_set_all_offline(int id){
	if (id < 0)
		ShowNotice("Sending all users offline.\n");
	else
		ShowNotice("Sending users of map-server %d offline.\n",id);

	for( const auto& pair : char_get_onlinedb() ){
		char_db_kickoffline( pair.second, id );
	}

	if (id >= 0 || !chlogif_isconnected())
		return;
	//Tell login-server to also mark all our characters as offline.
	chlogif_send_setallaccoffline(login_fd);
}

void char_set_all_offline_sql(void){
	//Set all players to 'OFFLINE'
	if( SQL_ERROR == Sql_Query(sql_handle, "UPDATE `%s` SET `online` = '0'", schema_config.char_db) )
		Sql_ShowDebug(sql_handle);
	if( SQL_ERROR == Sql_Query(sql_handle, "UPDATE `%s` SET `connect_member` = '0'", schema_config.guild_db) )
		Sql_ShowDebug(sql_handle);
}

int char_mmo_char_tosql(uint32 char_id, struct mmo_charstatus* p){
	int i = 0;
	int count = 0;
	int diff = 0;
	char save_status[128]; //For displaying save information. [Skotlex]
	int errors = 0; //If there are any errors while saving, "cp" will not be updated at the end.
	StringBuf buf;

	if (char_id!=p->char_id) return 0;

	std::shared_ptr<struct mmo_charstatus> cp = util::umap_find( char_get_chardb(), char_id );

	if( cp == nullptr ){
		cp = std::make_shared<struct mmo_charstatus>();
		cp->char_id = char_id;
		char_get_chardb()[cp->char_id] = cp;
	}

	StringBuf_Init(&buf);
	memset(save_status, 0, sizeof(save_status));

	if (
		(p->base_exp != cp->base_exp) || (p->base_level != cp->base_level) ||
		(p->job_level != cp->job_level) || (p->job_exp != cp->job_exp) ||
		(p->zeny != cp->zeny) ||
		( strncmp( p->last_point.map, cp->last_point.map, sizeof( p->last_point.map ) ) != 0 ) ||
		(p->last_point.x != cp->last_point.x) || (p->last_point.y != cp->last_point.y) || (p->last_point_instanceid != cp->last_point_instanceid) || 
		( strncmp( p->save_point.map, cp->save_point.map, sizeof( p->save_point.map ) ) != 0 ) ||
		( p->save_point.x != cp->save_point.x ) || ( p->save_point.y != cp->save_point.y ) ||
		(p->max_hp != cp->max_hp) || (p->hp != cp->hp) ||
		(p->max_sp != cp->max_sp) || (p->sp != cp->sp) ||
		(p->status_point != cp->status_point) || (p->skill_point != cp->skill_point) ||
		(p->str != cp->str) || (p->agi != cp->agi) || (p->vit != cp->vit) ||
		(p->int_ != cp->int_) || (p->dex != cp->dex) || (p->luk != cp->luk) ||
		(p->option != cp->option) ||
		(p->party_id != cp->party_id) || (p->guild_id != cp->guild_id) ||
		(p->pet_id != cp->pet_id) || (p->weapon != cp->weapon) || (p->hom_id != cp->hom_id) ||
		(p->ele_id != cp->ele_id) || (p->shield != cp->shield) || (p->head_top != cp->head_top) ||
		(p->head_mid != cp->head_mid) || (p->head_bottom != cp->head_bottom) || (p->delete_date != cp->delete_date) ||
		(p->rename != cp->rename) || (p->robe != cp->robe) || (p->character_moves != cp->character_moves) ||
		(p->unban_time != cp->unban_time) || (p->font != cp->font) || (p->uniqueitem_counter != cp->uniqueitem_counter) ||
		(p->hotkey_rowshift != cp->hotkey_rowshift) || (p->clan_id != cp->clan_id ) || (p->title_id != cp->title_id) ||
		(p->show_equip != cp->show_equip) || (p->hotkey_rowshift2 != cp->hotkey_rowshift2) ||
		(p->max_ap != cp->max_ap) || (p->ap != cp->ap) || (p->trait_point != cp->trait_point) ||
		(p->pow != cp->pow) || (p->sta != cp->sta) || (p->wis != cp->wis) ||
		(p->spl != cp->spl) || (p->con != cp->con) || (p->crt != cp->crt)
	)
	{	//Save status
		if( SQL_ERROR == Sql_Query(sql_handle, "UPDATE `%s` SET `base_level`='%d', `job_level`='%d',"
			"`base_exp`='%" PRIu64 "', `job_exp`='%" PRIu64 "', `zeny`='%d',"
			"`max_hp`='%u',`hp`='%u',`max_sp`='%u',`sp`='%u',`status_point`='%d',`skill_point`='%d',"
			"`str`='%d',`agi`='%d',`vit`='%d',`int`='%d',`dex`='%d',`luk`='%d',"
			"`option`='%d',`party_id`='%d',`guild_id`='%d',`pet_id`='%d',`homun_id`='%d',`elemental_id`='%d',"
			"`weapon`='%d',`shield`='%d',`head_top`='%d',`head_mid`='%d',`head_bottom`='%d',"
			"`last_map`='%s',`last_x`='%d',`last_y`='%d',`last_instanceid`='%d',"
			"`save_map`='%s',`save_x`='%d',`save_y`='%d', `rename`='%d',"
			"`delete_date`='%lu',`robe`='%d',`moves`='%d',`font`='%u',`uniqueitem_counter`='%u',"
			"`hotkey_rowshift`='%d', `clan_id`='%d', `title_id`='%lu', `show_equip`='%d', `hotkey_rowshift2`='%d',"
			"`max_ap`='%u',`ap`='%u',`trait_point`='%d',"
			"`pow`='%d',`sta`='%d',`wis`='%d',`spl`='%d',`con`='%d',`crt`='%d'"
			" WHERE `account_id`='%d' AND `char_id` = '%d'",
			schema_config.char_db, p->base_level, p->job_level,
			p->base_exp, p->job_exp, p->zeny,
			p->max_hp, p->hp, p->max_sp, p->sp, p->status_point, p->skill_point,
			p->str, p->agi, p->vit, p->int_, p->dex, p->luk,
			p->option, p->party_id, p->guild_id, p->pet_id, p->hom_id, p->ele_id,
			p->weapon, p->shield, p->head_top, p->head_mid, p->head_bottom,
			p->last_point.map, p->last_point.x, p->last_point.y, p->last_point_instanceid,
			p->save_point.map, p->save_point.x, p->save_point.y, p->rename,
			(unsigned long)p->delete_date, // FIXME: platform-dependent size
			p->robe, p->character_moves, p->font, p->uniqueitem_counter,
			p->hotkey_rowshift, p->clan_id, p->title_id, p->show_equip, p->hotkey_rowshift2,
			p->max_ap, p->ap, p->trait_point,
			p->pow, p->sta, p->wis, p->spl, p->con, p->crt,
			p->account_id, p->char_id) )
		{
			Sql_ShowDebug(sql_handle);
			errors++;
		} else
			strcat(save_status, " status");
	}

	//Values that will seldom change (to speed up saving)
	if (
		(p->hair != cp->hair) || (p->hair_color != cp->hair_color) || (p->clothes_color != cp->clothes_color) ||
		(p->body != cp->body) || (p->class_ != cp->class_) ||
		(p->partner_id != cp->partner_id) || (p->father != cp->father) ||
		(p->mother != cp->mother) || (p->child != cp->child) ||
 		(p->karma != cp->karma) || (p->manner != cp->manner) ||
		(p->fame != cp->fame) || (p->inventory_slots != cp->inventory_slots) ||
		(p->body_direction != cp->body_direction) || (p->disable_call != cp->disable_call)
	)
	{
		if( SQL_ERROR == Sql_Query(sql_handle, "UPDATE `%s` SET `class`='%d',"
			"`hair`='%d', `hair_color`='%d', `clothes_color`='%d', `body`='%d',"
			"`partner_id`='%u', `father`='%u', `mother`='%u', `child`='%u',"
			"`karma`='%d',`manner`='%d', `fame`='%d', `inventory_slots`='%hu',"
			"`body_direction`='%d',`disable_call`='%d'"
			" WHERE  `account_id`='%d' AND `char_id` = '%d'",
			schema_config.char_db, p->class_,
			p->hair, p->hair_color, p->clothes_color, p->body,
			p->partner_id, p->father, p->mother, p->child,
			p->karma, p->manner, p->fame, p->inventory_slots,
			p->body_direction, p->disable_call,
			p->account_id, p->char_id) )
		{
			Sql_ShowDebug(sql_handle);
			errors++;
		} else
			strcat(save_status, " status2");
	}

	/* Mercenary Owner */
	if( (p->mer_id != cp->mer_id) ||
		(p->arch_calls != cp->arch_calls) || (p->arch_faith != cp->arch_faith) ||
		(p->spear_calls != cp->spear_calls) || (p->spear_faith != cp->spear_faith) ||
		(p->sword_calls != cp->sword_calls) || (p->sword_faith != cp->sword_faith) )
	{
		if (mercenary_owner_tosql(char_id, p))
			strcat(save_status, " mercenary");
		else
			errors++;
	}

	//memo points
	if( memcmp(p->memo_point, cp->memo_point, sizeof(p->memo_point)) )
	{
		char esc_mapname[NAME_LENGTH*2+1];

		//`memo` (`memo_id`,`char_id`,`map`,`x`,`y`)
		if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `char_id`='%d'", schema_config.memo_db, p->char_id) )
		{
			Sql_ShowDebug(sql_handle);
			errors++;
		}

		//insert here.
		StringBuf_Clear(&buf);
		StringBuf_Printf(&buf, "INSERT INTO `%s`(`char_id`,`map`,`x`,`y`) VALUES ", schema_config.memo_db);
		for( i = 0, count = 0; i < MAX_MEMOPOINTS; ++i )
		{
			if( strcmp( "", p->memo_point[i].map ) != 0 ){
				if( count )
					StringBuf_AppendStr(&buf, ",");
				Sql_EscapeString( sql_handle, esc_mapname, p->memo_point[i].map );
				StringBuf_Printf(&buf, "('%d', '%s', '%d', '%d')", char_id, esc_mapname, p->memo_point[i].x, p->memo_point[i].y);
				++count;
			}
		}
		if( count )
		{
			if( SQL_ERROR == Sql_QueryStr(sql_handle, StringBuf_Value(&buf)) )
			{
				Sql_ShowDebug(sql_handle);
				errors++;
			}
		}
		strcat(save_status, " memo");
	}

	//skills
	if( memcmp(p->skill, cp->skill, sizeof(p->skill)) )
	{
		//`skill` (`char_id`, `id`, `lv`)
		if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `char_id`='%d'", schema_config.skill_db, p->char_id) )
		{
			Sql_ShowDebug(sql_handle);
			errors++;
		}

		StringBuf_Clear(&buf);
		StringBuf_Printf(&buf, "INSERT INTO `%s`(`char_id`,`id`,`lv`,`flag`) VALUES ", schema_config.skill_db);
		//insert here.
		for( i = 0, count = 0; i < MAX_SKILL; ++i ) {
			if( p->skill[i].id != 0 && p->skill[i].flag != SKILL_FLAG_TEMPORARY ) {

				if( p->skill[i].lv == 0 && ( p->skill[i].flag == SKILL_FLAG_PERM_GRANTED || p->skill[i].flag == SKILL_FLAG_PERMANENT ) )
					continue;
				if( p->skill[i].flag != SKILL_FLAG_PERMANENT && p->skill[i].flag != SKILL_FLAG_PERM_GRANTED && (p->skill[i].flag - SKILL_FLAG_REPLACED_LV_0) == 0 )
					continue;
				if( count )
					StringBuf_AppendStr(&buf, ",");
				StringBuf_Printf(&buf, "('%d','%d','%d','%d')", char_id, p->skill[i].id,
								 ( (p->skill[i].flag == SKILL_FLAG_PERMANENT || p->skill[i].flag == SKILL_FLAG_PERM_GRANTED) ? p->skill[i].lv : p->skill[i].flag - SKILL_FLAG_REPLACED_LV_0),
								 p->skill[i].flag == SKILL_FLAG_PERM_GRANTED ? p->skill[i].flag : 0);/* other flags do not need to be saved */
				++count;
			}
		}
		if( count )
		{
			if( SQL_ERROR == Sql_QueryStr(sql_handle, StringBuf_Value(&buf)) )
			{
				Sql_ShowDebug(sql_handle);
				errors++;
			}
		}

		strcat(save_status, " skills");
	}

	diff = 0;
	for(i = 0; i < MAX_FRIENDS; i++){
		if(p->friends[i].char_id != cp->friends[i].char_id ||
			p->friends[i].account_id != cp->friends[i].account_id){
			diff = 1;
			break;
		}
	}

	if(diff == 1)
	{	//Save friends
		if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `char_id`='%d'", schema_config.friend_db, char_id) )
		{
			Sql_ShowDebug(sql_handle);
			errors++;
		}

		StringBuf_Clear(&buf);
		StringBuf_Printf(&buf, "INSERT INTO `%s` (`char_id`, `friend_id`) VALUES ", schema_config.friend_db);
		for( i = 0, count = 0; i < MAX_FRIENDS; ++i )
		{
			if( p->friends[i].char_id > 0 )
			{
				if( count )
					StringBuf_AppendStr(&buf, ",");
				StringBuf_Printf(&buf, "('%d','%d')", char_id, p->friends[i].char_id);
				count++;
			}
		}
		if( count )
		{
			if( SQL_ERROR == Sql_QueryStr(sql_handle, StringBuf_Value(&buf)) )
			{
				Sql_ShowDebug(sql_handle);
				errors++;
			}
		}
		strcat(save_status, " friends");
	}

#ifdef HOTKEY_SAVING
	// hotkeys
	StringBuf_Clear(&buf);
	StringBuf_Printf(&buf, "REPLACE INTO `%s` (`char_id`, `hotkey`, `type`, `itemskill_id`, `skill_lvl`) VALUES ", schema_config.hotkey_db);
	diff = 0;
	for(i = 0; i < ARRAYLENGTH(p->hotkeys); i++){
		if(memcmp(&p->hotkeys[i], &cp->hotkeys[i], sizeof(struct hotkey)))
		{
			if( diff )
				StringBuf_AppendStr(&buf, ",");// not the first hotkey
			StringBuf_Printf(&buf, "('%d','%u','%u','%u','%u')", char_id, (unsigned int)i, (unsigned int)p->hotkeys[i].type, p->hotkeys[i].id , (unsigned int)p->hotkeys[i].lv);
			diff = 1;
		}
	}
	if(diff) {
		if( SQL_ERROR == Sql_QueryStr(sql_handle, StringBuf_Value(&buf)) )
		{
			Sql_ShowDebug(sql_handle);
			errors++;
		} else
			strcat(save_status, " hotkeys");
	}
#endif
	StringBuf_Destroy(&buf);
	if (save_status[0]!='\0' && charserv_config.save_log)
		ShowInfo("Saved char %d - %s:%s.\n", char_id, p->name, save_status);

	if( !errors ){
		memcpy( cp.get(), p, sizeof( struct mmo_charstatus ) );
	}

	return 0;
}

/// Saves an array of 'item' entries into the specified table.
int char_memitemdata_to_sql(const struct item items[], int max, int id, enum storage_type tableswitch, uint8 stor_id) {
	StringBuf buf;
	SqlStmt* stmt;
	int i, j, offset = 0, errors = 0;
	const char *tablename, *selectoption, *printname;
	struct item item; // temp storage variable
	bool* flag; // bit array for inventory matching
	bool found;

	switch (tableswitch) {
		case TABLE_INVENTORY:
			printname = "Inventory";
			tablename = schema_config.inventory_db;
			selectoption = "char_id";
			break;
		case TABLE_CART:
			printname = "Cart";
			tablename = schema_config.cart_db;
			selectoption = "char_id";
			break;
		case TABLE_STORAGE:
			printname = inter_premiumStorage_getPrintableName(stor_id);
			tablename = inter_premiumStorage_getTableName(stor_id);
			selectoption = "account_id";
			break;
		case TABLE_GUILD_STORAGE:
			printname = "Guild Storage";
			tablename = schema_config.guild_storage_db;
			selectoption = "guild_id";
			break;
		default:
			ShowError("Invalid table name!\n");
			return 1;
	}

	// The following code compares inventory with current database values
	// and performs modification/deletion/insertion only on relevant rows.
	// This approach is more complicated than a trivial delete&insert, but
	// it significantly reduces cpu load on the database server.

	StringBuf_Init(&buf);
	StringBuf_AppendStr(&buf, "SELECT `id`, `nameid`, `amount`, `equip`, `identify`, `refine`, `attribute`, `expire_time`, `bound`, `unique_id`, `enchantgrade`");
	if (tableswitch == TABLE_INVENTORY) {
		StringBuf_Printf(&buf, ", `favorite`, `equip_switch`");
		offset = 2;
	}

	for( i = 0; i < MAX_SLOTS; ++i )
		StringBuf_Printf(&buf, ", `card%d`", i);
	for( i = 0; i < MAX_ITEM_RDM_OPT; ++i ) {
		StringBuf_Printf(&buf, ", `option_id%d`", i);
		StringBuf_Printf(&buf, ", `option_val%d`", i);
		StringBuf_Printf(&buf, ", `option_parm%d`", i);
	}
	StringBuf_Printf(&buf, " FROM `%s` WHERE `%s`='%d'", tablename, selectoption, id);

	stmt = SqlStmt_Malloc(sql_handle);
	if( SQL_ERROR == SqlStmt_PrepareStr(stmt, StringBuf_Value(&buf))
	||  SQL_ERROR == SqlStmt_Execute(stmt) )
	{
		SqlStmt_ShowDebug(stmt);
		SqlStmt_Free(stmt);
		StringBuf_Destroy(&buf);
		return 1;
	}

	SqlStmt_BindColumn(stmt, 0, SQLDT_INT,       &item.id,          0, nullptr, nullptr);
	SqlStmt_BindColumn(stmt, 1, SQLDT_UINT,      &item.nameid,      0, nullptr, nullptr);
	SqlStmt_BindColumn(stmt, 2, SQLDT_SHORT,     &item.amount,      0, nullptr, nullptr);
	SqlStmt_BindColumn(stmt, 3, SQLDT_UINT,      &item.equip,       0, nullptr, nullptr);
	SqlStmt_BindColumn(stmt, 4, SQLDT_CHAR,      &item.identify,    0, nullptr, nullptr);
	SqlStmt_BindColumn(stmt, 5, SQLDT_CHAR,      &item.refine,      0, nullptr, nullptr);
	SqlStmt_BindColumn(stmt, 6, SQLDT_CHAR,      &item.attribute,   0, nullptr, nullptr);
	SqlStmt_BindColumn(stmt, 7, SQLDT_UINT,      &item.expire_time, 0, nullptr, nullptr);
	SqlStmt_BindColumn(stmt, 8, SQLDT_UINT,      &item.bound,       0, nullptr, nullptr);
	SqlStmt_BindColumn(stmt, 9, SQLDT_UINT64,    &item.unique_id,   0, nullptr, nullptr);
	SqlStmt_BindColumn(stmt,10, SQLDT_INT8,      &item.enchantgrade,0, nullptr, nullptr);
	if (tableswitch == TABLE_INVENTORY){
		SqlStmt_BindColumn(stmt, 11, SQLDT_CHAR, &item.favorite,    0, nullptr, nullptr);
		SqlStmt_BindColumn(stmt, 12, SQLDT_UINT, &item.equipSwitch, 0, nullptr, nullptr);
	}
	for( i = 0; i < MAX_SLOTS; ++i )
		SqlStmt_BindColumn(stmt, 11+offset+i, SQLDT_UINT, &item.card[i], 0, nullptr, nullptr);
	for( i = 0; i < MAX_ITEM_RDM_OPT; ++i ) {
		SqlStmt_BindColumn(stmt, 11+offset+MAX_SLOTS+i*3, SQLDT_SHORT, &item.option[i].id, 0, nullptr, nullptr);
		SqlStmt_BindColumn(stmt, 12+offset+MAX_SLOTS+i*3, SQLDT_SHORT, &item.option[i].value, 0, nullptr, nullptr);
		SqlStmt_BindColumn(stmt, 13+offset+MAX_SLOTS+i*3, SQLDT_CHAR, &item.option[i].param, 0, nullptr, nullptr);
	}
	// bit array indicating which inventory items have already been matched
	flag = (bool*) aCalloc(max, sizeof(bool));

	while( SQL_SUCCESS == SqlStmt_NextRow(stmt) )
	{
		found = false;
		// search for the presence of the item in the char's inventory
		for( i = 0; i < max; ++i )
		{
			// skip empty and already matched entries
			if( items[i].nameid == 0 || flag[i] )
				continue;

			if( items[i].nameid == item.nameid
			&&  items[i].card[0] == item.card[0]
			&&  items[i].card[2] == item.card[2]
			&&  items[i].card[3] == item.card[3]
			&&  items[i].unique_id == item.unique_id
			) {	//They are the same item.
				int k;
				
				ARR_FIND( 0, MAX_SLOTS, j, items[i].card[j] != item.card[j] );
				ARR_FIND( 0, MAX_ITEM_RDM_OPT, k, items[i].option[k].id != item.option[k].id || items[i].option[k].value != item.option[k].value || items[i].option[k].param != item.option[k].param );
				
				if( j == MAX_SLOTS &&
					k == MAX_ITEM_RDM_OPT &&
					items[i].amount == item.amount &&
					items[i].equip == item.equip &&
					items[i].identify == item.identify &&
					items[i].refine == item.refine &&
					items[i].attribute == item.attribute &&
					items[i].expire_time == item.expire_time &&
					items[i].bound == item.bound &&
					items[i].enchantgrade == item.enchantgrade &&
					(tableswitch != TABLE_INVENTORY || (items[i].favorite == item.favorite && items[i].equipSwitch == item.equipSwitch)) )
				;	//Do nothing.
				else
				{
					// update all fields.
					StringBuf_Clear(&buf);
					StringBuf_Printf(&buf, "UPDATE `%s` SET `amount`='%d', `equip`='%u', `identify`='%d', `refine`='%d',`attribute`='%d', `expire_time`='%u', `bound`='%d', `unique_id`='%" PRIu64 "', `enchantgrade`='%d'",
						tablename, items[i].amount, items[i].equip, items[i].identify, items[i].refine, items[i].attribute, items[i].expire_time, items[i].bound, items[i].unique_id, items[i].enchantgrade);
					if (tableswitch == TABLE_INVENTORY)
						StringBuf_Printf(&buf, ", `favorite`='%d', `equip_switch`='%u'", items[i].favorite, items[i].equipSwitch);
					for( j = 0; j < MAX_SLOTS; ++j )
						StringBuf_Printf(&buf, ", `card%d`=%u", j, items[i].card[j]);
					for( j = 0; j < MAX_ITEM_RDM_OPT; ++j ) {
						StringBuf_Printf(&buf, ", `option_id%d`=%d", j, items[i].option[j].id);
						StringBuf_Printf(&buf, ", `option_val%d`=%d", j, items[i].option[j].value);
						StringBuf_Printf(&buf, ", `option_parm%d`=%d", j, items[i].option[j].param);
					}
					StringBuf_Printf(&buf, " WHERE `id`='%d' LIMIT 1", item.id);

					if( SQL_ERROR == Sql_QueryStr(sql_handle, StringBuf_Value(&buf)) )
					{
						Sql_ShowDebug(sql_handle);
						errors++;
					}
				}

				found = flag[i] = true; //Item dealt with,
				break; //skip to next item in the db.
			}
		}
		if( !found )
		{// Item not present in inventory, remove it.
			if( SQL_ERROR == Sql_Query(sql_handle, "DELETE from `%s` where `id`='%d' LIMIT 1", tablename, item.id) )
			{
				Sql_ShowDebug(sql_handle);
				errors++;
			}
		}
	}
	SqlStmt_Free(stmt);

	StringBuf_Clear(&buf);
	StringBuf_Printf(&buf, "INSERT INTO `%s`(`%s`, `nameid`, `amount`, `equip`, `identify`, `refine`, `attribute`, `expire_time`, `bound`, `unique_id`, `enchantgrade`", tablename, selectoption);
	if (tableswitch == TABLE_INVENTORY)
		StringBuf_Printf(&buf, ", `favorite`, `equip_switch`");
	for( j = 0; j < MAX_SLOTS; ++j )
		StringBuf_Printf(&buf, ", `card%d`", j);
	for( j = 0; j < MAX_ITEM_RDM_OPT; ++j ) {
		StringBuf_Printf(&buf, ", `option_id%d`", j);
		StringBuf_Printf(&buf, ", `option_val%d`", j);
		StringBuf_Printf(&buf, ", `option_parm%d`", j);
	}
	StringBuf_AppendStr(&buf, ") VALUES ");

	found = false;
	// insert non-matched items into the db as new items
	for( i = 0; i < max; ++i )
	{
		// skip empty and already matched entries
		if( items[i].nameid == 0 || flag[i] )
			continue;

		if( found )
			StringBuf_AppendStr(&buf, ",");
		else
			found = true;

		StringBuf_Printf(&buf, "('%d', '%u', '%d', '%u', '%d', '%d', '%d', '%u', '%d', '%" PRIu64 "', '%d'",
			id, items[i].nameid, items[i].amount, items[i].equip, items[i].identify, items[i].refine, items[i].attribute, items[i].expire_time, items[i].bound, items[i].unique_id, items[i].enchantgrade);
		if (tableswitch == TABLE_INVENTORY)
			StringBuf_Printf(&buf, ", '%d', '%u'", items[i].favorite, items[i].equipSwitch);
		for( j = 0; j < MAX_SLOTS; ++j )
			StringBuf_Printf(&buf, ", '%u'", items[i].card[j]);
		for( j = 0; j < MAX_ITEM_RDM_OPT; ++j ) {
			StringBuf_Printf(&buf, ", '%d'", items[i].option[j].id);
			StringBuf_Printf(&buf, ", '%d'", items[i].option[j].value);
			StringBuf_Printf(&buf, ", '%d'", items[i].option[j].param);
		}
		StringBuf_AppendStr(&buf, ")");
	}

	if( found && SQL_ERROR == Sql_QueryStr(sql_handle, StringBuf_Value(&buf)) )
	{
		Sql_ShowDebug(sql_handle);
		errors++;
	}

	ShowInfo("Saved %s (%d) data to table %s for %s: %d\n", printname, stor_id, tablename, selectoption, id);
	StringBuf_Destroy(&buf);
	aFree(flag);

	return errors;
}

bool char_memitemdata_from_sql(struct s_storage* p, int max, int id, enum storage_type tableswitch, uint8 stor_id) {
	StringBuf buf;
	SqlStmt* stmt;
	int i,j, offset = 0, max2;
	struct item item, *storage;
	const char *tablename, *selectoption, *printname;

	switch (tableswitch) {
		case TABLE_INVENTORY:
			printname = "Inventory";
			tablename = schema_config.inventory_db;
			selectoption = "char_id";
			storage = p->u.items_inventory;
			max2 = MAX_INVENTORY;
			break;
		case TABLE_CART:
			printname = "Cart";
			tablename = schema_config.cart_db;
			selectoption = "char_id";
			storage = p->u.items_cart;
			max2 = MAX_CART;
			break;
		case TABLE_STORAGE:
			printname = "Storage";
			tablename = inter_premiumStorage_getTableName(stor_id);
			selectoption = "account_id";
			storage = p->u.items_storage;
			max2 = inter_premiumStorage_getMax(p->stor_id);
			break;
		case TABLE_GUILD_STORAGE:
			printname = "Guild Storage";
			tablename = schema_config.guild_storage_db;
			selectoption = "guild_id";
			storage = p->u.items_guild;
			max2 = inter_guild_storagemax(id);
			break;
		default:
			ShowError("Invalid table name!\n");
			return false;
	}

	memset(p, 0, sizeof(struct s_storage)); //clean up memory
	p->id = id;
	p->type = tableswitch;
	p->stor_id = stor_id;
	p->max_amount = max2;

	stmt = SqlStmt_Malloc(sql_handle);
	if (stmt == nullptr) {
		SqlStmt_ShowDebug(stmt);
		return false;
	}

	StringBuf_Init(&buf);
	StringBuf_AppendStr(&buf, "SELECT `id`,`nameid`,`amount`,`equip`,`identify`,`refine`,`attribute`,`expire_time`,`bound`,`unique_id`,`enchantgrade`");
	if (tableswitch == TABLE_INVENTORY) {
		StringBuf_Printf(&buf, ", `favorite`, `equip_switch`");
		offset = 2;
	}
	for( j = 0; j < MAX_SLOTS; ++j )
		StringBuf_Printf(&buf, ",`card%d`", j);
	for( j = 0; j < MAX_ITEM_RDM_OPT; ++j ) {
		StringBuf_Printf(&buf, ", `option_id%d`", j);
		StringBuf_Printf(&buf, ", `option_val%d`", j);
		StringBuf_Printf(&buf, ", `option_parm%d`", j);
	}
	StringBuf_Printf(&buf, " FROM `%s` WHERE `%s`=? ORDER BY `nameid`", tablename, selectoption );

	if( SQL_ERROR == SqlStmt_PrepareStr(stmt, StringBuf_Value(&buf))
		||	SQL_ERROR == SqlStmt_BindParam(stmt, 0, SQLDT_INT, &id, 0)
		||	SQL_ERROR == SqlStmt_Execute(stmt) )
	{
		SqlStmt_ShowDebug(stmt);
		SqlStmt_Free(stmt);
		StringBuf_Destroy(&buf);
		return false;
	}

	SqlStmt_BindColumn(stmt, 0, SQLDT_INT,          &item.id,        0, nullptr, nullptr);
	SqlStmt_BindColumn(stmt, 1, SQLDT_UINT,         &item.nameid,    0, nullptr, nullptr);
	SqlStmt_BindColumn(stmt, 2, SQLDT_SHORT,        &item.amount,    0, nullptr, nullptr);
	SqlStmt_BindColumn(stmt, 3, SQLDT_UINT,         &item.equip,     0, nullptr, nullptr);
	SqlStmt_BindColumn(stmt, 4, SQLDT_CHAR,         &item.identify,  0, nullptr, nullptr);
	SqlStmt_BindColumn(stmt, 5, SQLDT_CHAR,         &item.refine,    0, nullptr, nullptr);
	SqlStmt_BindColumn(stmt, 6, SQLDT_CHAR,         &item.attribute, 0, nullptr, nullptr);
	SqlStmt_BindColumn(stmt, 7, SQLDT_UINT,         &item.expire_time, 0, nullptr, nullptr);
	SqlStmt_BindColumn(stmt, 8, SQLDT_CHAR,         &item.bound,     0, nullptr, nullptr);
	SqlStmt_BindColumn(stmt, 9, SQLDT_ULONGLONG,    &item.unique_id, 0, nullptr, nullptr);
	SqlStmt_BindColumn(stmt,10, SQLDT_INT8,         &item.enchantgrade, 0, nullptr, nullptr);
	if (tableswitch == TABLE_INVENTORY){
		SqlStmt_BindColumn(stmt, 11, SQLDT_CHAR, &item.favorite,    0, nullptr, nullptr);
		SqlStmt_BindColumn(stmt, 12, SQLDT_UINT, &item.equipSwitch, 0, nullptr, nullptr);
	}
	for( i = 0; i < MAX_SLOTS; ++i )
		SqlStmt_BindColumn(stmt, 11+offset+i, SQLDT_UINT, &item.card[i],   0, nullptr, nullptr);
 	for( i = 0; i < MAX_ITEM_RDM_OPT; ++i ) {
		SqlStmt_BindColumn(stmt, 11+offset+MAX_SLOTS+i*3, SQLDT_SHORT, &item.option[i].id, 0, nullptr, nullptr);
		SqlStmt_BindColumn(stmt, 12+offset+MAX_SLOTS+i*3, SQLDT_SHORT, &item.option[i].value, 0, nullptr, nullptr);
		SqlStmt_BindColumn(stmt, 13+offset+MAX_SLOTS+i*3, SQLDT_CHAR, &item.option[i].param, 0, nullptr, nullptr);
 	}

	for( i = 0; i < max && SQL_SUCCESS == SqlStmt_NextRow(stmt); ++i )
		memcpy(&storage[i], &item, sizeof(item));

	p->amount = i;
	ShowInfo("Loaded %s data from table %s for %s: %d (total: %d)\n", printname, tablename, selectoption, id, p->amount);

	SqlStmt_FreeResult(stmt);
	SqlStmt_Free(stmt);
	StringBuf_Destroy(&buf);

	return true;
}

/**
 * Returns the correct gender ID for the given character and enum value.
 *
 * If the per-character sex is defined but not supported by the current packetver, the account gender is returned.
 *
 * @param sd Character data, if available.
 * @param p  Character status.
 * @param sex Character sex (database enum)
 *
 * @retval SEX_MALE if the per-character sex is male
 * @retval SEX_FEMALE if the per-character sex is female
 */
int char_mmo_gender( const struct char_session_data *sd, const struct mmo_charstatus *p, char sex ){
#if PACKETVER >= 20141016
	switch( sex ){
		case 'M':
			return SEX_MALE;
		case 'F':
			return SEX_FEMALE;
		default:
#else
	// No matter what the database says, always return the account gender
	// Per character gender is not supported before 2014-10-16
#endif
			// There are calls to this function that do not contain the session
			if( sd == nullptr ){
				int i;

				// Find player session
				ARR_FIND( 0, fd_max, i, session[i] && ( sd = (struct char_session_data*)session[i]->session_data ) && sd->account_id == p->account_id );

				if( i >= fd_max ){
					ShowWarning( "Session was not found for character '%s' (CID: %d, AID: %d), defaulting gender to male...\n", p->name, p->char_id, p->account_id );
					return SEX_MALE;
				}
			}

#if PACKETVER >= 20141016
			ShowWarning( "Found unknown gender '%c' for character '%s' (CID: %d, AID: %d), returning account gender...\n", sex, p->name, p->char_id, p->account_id );
#endif

			return sd->sex;
#if PACKETVER >= 20141016
	}
#endif
}

int char_mmo_char_tobuf(uint8* buf, struct mmo_charstatus* p);

//=====================================================================================================
// Loads the basic character rooster for the given account. Returns total buffer used.
int char_mmo_chars_fromsql(struct char_session_data* sd, uint8* buf, uint8* count ) {
	SqlStmt* stmt;
	struct mmo_charstatus p;
	int j = 0, i;
	char sex[2];

	stmt = SqlStmt_Malloc(sql_handle);
	if( stmt == nullptr ) {
		SqlStmt_ShowDebug(stmt);
		return 0;
	}
	memset(&p, 0, sizeof(p));

	for( i = 0; i < MAX_CHARS; i++ ) {
		sd->found_char[i] = -1;
		sd->unban_time[i] = 0;
	}

	// read char data
	if( SQL_ERROR == SqlStmt_Prepare(stmt, "SELECT "
		"`char_id`,`char_num`,`name`,`class`,`base_level`,`job_level`,`base_exp`,`job_exp`,`zeny`,"
		"`str`,`agi`,`vit`,`int`,`dex`,`luk`,`max_hp`,`hp`,`max_sp`,`sp`,"
		"`status_point`,`skill_point`,`option`,`karma`,`manner`,`hair`,`hair_color`,"
		"`clothes_color`,`body`,`weapon`,`shield`,`head_top`,`head_mid`,`head_bottom`,`last_map`,`rename`,`delete_date`,"
		"`robe`,`moves`,`unban_time`,`font`,`uniqueitem_counter`,`sex`,`hotkey_rowshift`,`title_id`,`show_equip`,"
		"`hotkey_rowshift2`,"
		"`max_ap`,`ap`,`trait_point`,`pow`,`sta`,`wis`,`spl`,`con`,`crt`,"
		"`inventory_slots`,`body_direction`,`disable_call`"
		" FROM `%s` WHERE `account_id`='%d' AND `char_num` < '%d'", schema_config.char_db, sd->account_id, MAX_CHARS)
	||	SQL_ERROR == SqlStmt_Execute(stmt)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 0,  SQLDT_INT,    &p.char_id, 0, nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 1,  SQLDT_UCHAR,  &p.slot, 0, nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 2,  SQLDT_STRING, &p.name, sizeof(p.name), nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 3,  SQLDT_SHORT,  &p.class_, 0, nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 4,  SQLDT_UINT,   &p.base_level, 0, nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 5,  SQLDT_UINT,   &p.job_level, 0, nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 6,  SQLDT_UINT64, &p.base_exp, 0, nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 7,  SQLDT_UINT64, &p.job_exp, 0, nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 8,  SQLDT_INT,    &p.zeny, 0, nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 9,  SQLDT_SHORT,  &p.str, 0, nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 10, SQLDT_SHORT,  &p.agi, 0, nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 11, SQLDT_SHORT,  &p.vit, 0, nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 12, SQLDT_SHORT,  &p.int_, 0, nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 13, SQLDT_SHORT,  &p.dex, 0, nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 14, SQLDT_SHORT,  &p.luk, 0, nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 15, SQLDT_UINT,   &p.max_hp, 0, nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 16, SQLDT_UINT,   &p.hp, 0, nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 17, SQLDT_UINT,   &p.max_sp, 0, nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 18, SQLDT_UINT,   &p.sp, 0, nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 19, SQLDT_UINT,   &p.status_point, 0, nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 20, SQLDT_UINT,   &p.skill_point, 0, nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 21, SQLDT_UINT,   &p.option, 0, nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 22, SQLDT_UCHAR,  &p.karma, 0, nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 23, SQLDT_SHORT,  &p.manner, 0, nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 24, SQLDT_SHORT,  &p.hair, 0, nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 25, SQLDT_SHORT,  &p.hair_color, 0, nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 26, SQLDT_SHORT,  &p.clothes_color, 0, nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 27, SQLDT_SHORT,  &p.body, 0, nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 28, SQLDT_SHORT,  &p.weapon, 0, nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 29, SQLDT_SHORT,  &p.shield, 0, nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 30, SQLDT_SHORT,  &p.head_top, 0, nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 31, SQLDT_SHORT,  &p.head_mid, 0, nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 32, SQLDT_SHORT,  &p.head_bottom, 0, nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 33, SQLDT_STRING, &p.last_point.map, sizeof(p.last_point.map), nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 34, SQLDT_SHORT,	&p.rename, 0, nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 35, SQLDT_UINT32, &p.delete_date, 0, nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 36, SQLDT_SHORT,  &p.robe, 0, nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 37, SQLDT_UINT,   &p.character_moves, 0, nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 38, SQLDT_LONG,   &p.unban_time, 0, nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 39, SQLDT_UCHAR,  &p.font, 0, nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 40, SQLDT_UINT,   &p.uniqueitem_counter, 0, nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 41, SQLDT_ENUM,   &sex, sizeof(sex), nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 42, SQLDT_UCHAR,  &p.hotkey_rowshift, 0, nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 43, SQLDT_ULONG,  &p.title_id, 0, nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 44, SQLDT_UINT16, &p.show_equip, 0, nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 45, SQLDT_UCHAR,  &p.hotkey_rowshift2, 0, nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 46, SQLDT_UINT,   &p.max_ap, 0, nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 47, SQLDT_UINT,   &p.ap, 0, nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 48, SQLDT_UINT,   &p.trait_point, 0, nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 49, SQLDT_SHORT,  &p.pow, 0, nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 50, SQLDT_SHORT,  &p.sta, 0, nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 51, SQLDT_SHORT,  &p.wis, 0, nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 52, SQLDT_SHORT,  &p.spl, 0, nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 53, SQLDT_SHORT,  &p.con, 0, nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 54, SQLDT_SHORT,  &p.crt, 0, nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 55, SQLDT_UINT16, &p.inventory_slots, 0, nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 56, SQLDT_UINT8,  &p.body_direction, 0, nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 57, SQLDT_UINT16, &p.disable_call, 0, nullptr, nullptr)
	)
	{
		SqlStmt_ShowDebug(stmt);
		SqlStmt_Free(stmt);
		return 0;
	}

	for( i = 0; i < MAX_CHARS && SQL_SUCCESS == SqlStmt_NextRow(stmt); i++ )
	{
		sd->found_char[p.slot] = p.char_id;
		sd->unban_time[p.slot] = p.unban_time;
		p.sex = char_mmo_gender(sd, &p, sex[0]);
		j += char_mmo_char_tobuf(WBUFP(buf, j), &p);

		// Addon System
		// store the required info into the session
		sd->char_moves[p.slot] = p.character_moves;
	}

	if( count != nullptr ){
		*count = i;
	}

	memset(sd->new_name,0,sizeof(sd->new_name));

	SqlStmt_Free(stmt);
	return j;
}

//=====================================================================================================
int char_mmo_char_fromsql(uint32 char_id, struct mmo_charstatus* p, bool load_everything) {
	int i;
	SqlStmt* stmt;
	struct s_point_str tmp_point;
	struct s_skill tmp_skill;
	uint16 skill_count = 0;
	struct s_friend tmp_friend;
#ifdef HOTKEY_SAVING
	struct hotkey tmp_hotkey;
	int hotkey_num;
#endif
	StringBuf msg_buf;
	char sex[2];

	memset(p, 0, sizeof(struct mmo_charstatus));

	if (charserv_config.save_log) ShowInfo("Char load request (%d)\n", char_id);

	stmt = SqlStmt_Malloc(sql_handle);
	if( stmt == nullptr )
	{
		SqlStmt_ShowDebug(stmt);
		return 0;
	}

	// read char data
	if( SQL_ERROR == SqlStmt_Prepare(stmt, "SELECT "
		"`char_id`,`account_id`,`char_num`,`name`,`class`,`base_level`,`job_level`,`base_exp`,`job_exp`,`zeny`,"
		"`str`,`agi`,`vit`,`int`,`dex`,`luk`,`max_hp`,`hp`,`max_sp`,`sp`,"
		"`status_point`,`skill_point`,`option`,`karma`,`manner`,`party_id`,`guild_id`,`pet_id`,`homun_id`,`elemental_id`,`hair`,"
		"`hair_color`,`clothes_color`,`body`,`weapon`,`shield`,`head_top`,`head_mid`,`head_bottom`,`last_map`,`last_x`,`last_y`,"
		"`save_map`,`save_x`,`save_y`,`partner_id`,`father`,`mother`,`child`,`fame`,`rename`,`delete_date`,`robe`, `moves`,"
		"`unban_time`,`font`,`uniqueitem_counter`,`sex`,`hotkey_rowshift`,`clan_id`,`title_id`,`show_equip`,`hotkey_rowshift2`,"
		"`max_ap`,`ap`,`trait_point`,`pow`,`sta`,`wis`,`spl`,`con`,`crt`,"
		"`inventory_slots`,`body_direction`,`disable_call`,`last_instanceid`"
		" FROM `%s` WHERE `char_id`=? LIMIT 1", schema_config.char_db)
	||	SQL_ERROR == SqlStmt_BindParam(stmt, 0, SQLDT_INT, &char_id, 0)
	||	SQL_ERROR == SqlStmt_Execute(stmt)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 0,  SQLDT_INT,    &p->char_id, 0, nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 1,  SQLDT_INT,    &p->account_id, 0, nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 2,  SQLDT_UCHAR,  &p->slot, 0, nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 3,  SQLDT_STRING, &p->name, sizeof(p->name), nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 4,  SQLDT_SHORT,  &p->class_, 0, nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 5,  SQLDT_UINT,   &p->base_level, 0, nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 6,  SQLDT_UINT,   &p->job_level, 0, nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 7,  SQLDT_UINT64, &p->base_exp, 0, nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 8,  SQLDT_UINT64, &p->job_exp, 0, nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 9,  SQLDT_INT,    &p->zeny, 0, nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 10, SQLDT_SHORT,  &p->str, 0, nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 11, SQLDT_SHORT,  &p->agi, 0, nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 12, SQLDT_SHORT,  &p->vit, 0, nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 13, SQLDT_SHORT,  &p->int_, 0, nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 14, SQLDT_SHORT,  &p->dex, 0, nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 15, SQLDT_SHORT,  &p->luk, 0, nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 16, SQLDT_UINT,   &p->max_hp, 0, nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 17, SQLDT_UINT,   &p->hp, 0, nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 18, SQLDT_UINT,   &p->max_sp, 0, nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 19, SQLDT_UINT,   &p->sp, 0, nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 20, SQLDT_UINT,   &p->status_point, 0, nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 21, SQLDT_UINT,   &p->skill_point, 0, nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 22, SQLDT_UINT,   &p->option, 0, nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 23, SQLDT_UCHAR,  &p->karma, 0, nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 24, SQLDT_SHORT,  &p->manner, 0, nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 25, SQLDT_INT,    &p->party_id, 0, nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 26, SQLDT_INT,    &p->guild_id, 0, nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 27, SQLDT_INT,    &p->pet_id, 0, nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 28, SQLDT_INT,    &p->hom_id, 0, nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 29, SQLDT_INT,    &p->ele_id, 0, nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 30, SQLDT_SHORT,  &p->hair, 0, nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 31, SQLDT_SHORT,  &p->hair_color, 0, nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 32, SQLDT_SHORT,  &p->clothes_color, 0, nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 33, SQLDT_SHORT,  &p->body, 0, nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 34, SQLDT_SHORT,  &p->weapon, 0, nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 35, SQLDT_SHORT,  &p->shield, 0, nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 36, SQLDT_SHORT,  &p->head_top, 0, nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 37, SQLDT_SHORT,  &p->head_mid, 0, nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 38, SQLDT_SHORT,  &p->head_bottom, 0, nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 39, SQLDT_STRING, &p->last_point.map, sizeof(p->last_point.map), nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 40, SQLDT_SHORT,  &p->last_point.x, 0, nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 41, SQLDT_SHORT,  &p->last_point.y, 0, nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 42, SQLDT_STRING, &p->save_point.map, sizeof(p->save_point.map), nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 43, SQLDT_SHORT,  &p->save_point.x, 0, nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 44, SQLDT_SHORT,  &p->save_point.y, 0, nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 45, SQLDT_UINT32,    &p->partner_id, 0, nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 46, SQLDT_UINT32,    &p->father, 0, nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 47, SQLDT_UINT32,    &p->mother, 0, nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 48, SQLDT_UINT32,    &p->child, 0, nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 49, SQLDT_INT,    &p->fame, 0, nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 50, SQLDT_SHORT,  &p->rename, 0, nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 51, SQLDT_UINT32, &p->delete_date, 0, nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 52, SQLDT_SHORT,  &p->robe, 0, nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 53, SQLDT_UINT32, &p->character_moves, 0, nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 54, SQLDT_LONG,   &p->unban_time, 0, nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 55, SQLDT_UCHAR,  &p->font, 0, nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 56, SQLDT_UINT,   &p->uniqueitem_counter, 0, nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 57, SQLDT_ENUM,   &sex, sizeof(sex), nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 58, SQLDT_UCHAR,  &p->hotkey_rowshift, 0, nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 59, SQLDT_INT,    &p->clan_id, 0, nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 60, SQLDT_ULONG,  &p->title_id, 0, nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 61, SQLDT_UINT16, &p->show_equip, 0, nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 62, SQLDT_UCHAR,  &p->hotkey_rowshift2, 0, nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 63, SQLDT_UINT,   &p->max_ap, 0, nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 64, SQLDT_UINT,   &p->ap, 0, nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 65, SQLDT_UINT,   &p->trait_point, 0, nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 66, SQLDT_SHORT,  &p->pow, 0, nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 67, SQLDT_SHORT,  &p->sta, 0, nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 68, SQLDT_SHORT,  &p->wis, 0, nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 69, SQLDT_SHORT,  &p->spl, 0, nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 70, SQLDT_SHORT,  &p->con, 0, nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 71, SQLDT_SHORT,  &p->crt, 0, nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 72, SQLDT_UINT16, &p->inventory_slots, 0, nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 73, SQLDT_UINT8,  &p->body_direction, 0, nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 74, SQLDT_UINT8,	&p->disable_call, 0, nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 75, SQLDT_INT,    &p->last_point_instanceid, 0, nullptr, nullptr)
	)
	{
		SqlStmt_ShowDebug(stmt);
		SqlStmt_Free(stmt);
		return 0;
	}
	if( SQL_ERROR == SqlStmt_NextRow(stmt) )
	{
		ShowError("Requested non-existant character id: %d!\n", char_id);
		SqlStmt_Free(stmt);
		return 0;
	}
	p->sex = char_mmo_gender(nullptr, p, sex[0]);

	StringBuf_Init(&msg_buf);
	StringBuf_AppendStr(&msg_buf, " status");

	if (!load_everything) // For quick selection of data when displaying the char menu
	{
		SqlStmt_Free(stmt);
		StringBuf_Destroy(&msg_buf);
		return 1;
	}

	//read memo data
	//`memo` (`memo_id`,`char_id`,`map`,`x`,`y`)
	if( SQL_ERROR == SqlStmt_Prepare(stmt, "SELECT `map`,`x`,`y` FROM `%s` WHERE `char_id`=? ORDER by `memo_id` LIMIT %d", schema_config.memo_db, MAX_MEMOPOINTS)
	||	SQL_ERROR == SqlStmt_BindParam(stmt, 0, SQLDT_INT, &char_id, 0)
	||	SQL_ERROR == SqlStmt_Execute(stmt)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 0, SQLDT_STRING, &tmp_point.map, sizeof(tmp_point.map), nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 1, SQLDT_SHORT,  &tmp_point.x, 0, nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 2, SQLDT_SHORT,  &tmp_point.y, 0, nullptr, nullptr) )
		SqlStmt_ShowDebug(stmt);

	for( i = 0; i < MAX_MEMOPOINTS && SQL_SUCCESS == SqlStmt_NextRow(stmt); ++i )
	{
		memcpy(&p->memo_point[i], &tmp_point, sizeof(tmp_point));
	}
	StringBuf_AppendStr(&msg_buf, " memo");

	//read skill
	//`skill` (`char_id`, `id`, `lv`)
	if( SQL_ERROR == SqlStmt_Prepare(stmt, "SELECT `id`, `lv`,`flag` FROM `%s` WHERE `char_id`=? LIMIT %d", schema_config.skill_db, MAX_SKILL)
		||	SQL_ERROR == SqlStmt_BindParam(stmt, 0, SQLDT_INT, &char_id, 0)
		||	SQL_ERROR == SqlStmt_Execute(stmt)
		||	SQL_ERROR == SqlStmt_BindColumn(stmt, 0, SQLDT_UINT16, &tmp_skill.id  , 0, nullptr, nullptr)
		||	SQL_ERROR == SqlStmt_BindColumn(stmt, 1, SQLDT_UINT8 , &tmp_skill.lv  , 0, nullptr, nullptr)
		||	SQL_ERROR == SqlStmt_BindColumn(stmt, 2, SQLDT_UINT8 , &tmp_skill.flag, 0, nullptr, nullptr) )
		SqlStmt_ShowDebug(stmt);

	if( tmp_skill.flag != SKILL_FLAG_PERM_GRANTED )
		tmp_skill.flag = SKILL_FLAG_PERMANENT;

	for( i = 0; skill_count < MAX_SKILL && SQL_SUCCESS == SqlStmt_NextRow(stmt); i++ ) {
		if( tmp_skill.id > 0 && tmp_skill.id < MAX_SKILL_ID ) {
			memcpy(&p->skill[i], &tmp_skill, sizeof(tmp_skill));
			skill_count++;
		}
		else
			ShowWarning("mmo_char_fromsql: ignoring invalid skill (id=%u,lv=%u) of character %s (AID=%d,CID=%d)\n", tmp_skill.id, tmp_skill.lv, p->name, p->account_id, p->char_id);
	}
	StringBuf_Printf(&msg_buf, " %d skills", skill_count);

	//read friends
	//`friends` (`char_id`, `friend_id`)
	if( SQL_ERROR == SqlStmt_Prepare(stmt, "SELECT c.`account_id`, c.`char_id`, c.`name` FROM `%s` c LEFT JOIN `%s` f ON f.`friend_id` = c.`char_id` WHERE f.`char_id`=? LIMIT %d", schema_config.char_db, schema_config.friend_db, MAX_FRIENDS)
	||	SQL_ERROR == SqlStmt_BindParam(stmt, 0, SQLDT_INT, &char_id, 0)
	||	SQL_ERROR == SqlStmt_Execute(stmt)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 0, SQLDT_INT,    &tmp_friend.account_id, 0, nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 1, SQLDT_INT,    &tmp_friend.char_id, 0, nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 2, SQLDT_STRING, &tmp_friend.name, sizeof(tmp_friend.name), nullptr, nullptr) )
		SqlStmt_ShowDebug(stmt);

	for( i = 0; i < MAX_FRIENDS && SQL_SUCCESS == SqlStmt_NextRow(stmt); ++i )
		memcpy(&p->friends[i], &tmp_friend, sizeof(tmp_friend));
	StringBuf_AppendStr(&msg_buf, " friends");

#ifdef HOTKEY_SAVING
	//read hotkeys
	//`hotkey` (`char_id`, `hotkey`, `type`, `itemskill_id`, `skill_lvl`
	if( SQL_ERROR == SqlStmt_Prepare(stmt, "SELECT `hotkey`, `type`, `itemskill_id`, `skill_lvl` FROM `%s` WHERE `char_id`=?", schema_config.hotkey_db)
	||	SQL_ERROR == SqlStmt_BindParam(stmt, 0, SQLDT_INT, &char_id, 0)
	||	SQL_ERROR == SqlStmt_Execute(stmt)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 0, SQLDT_INT,    &hotkey_num, 0, nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 1, SQLDT_UCHAR,  &tmp_hotkey.type, 0, nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 2, SQLDT_UINT,   &tmp_hotkey.id, 0, nullptr, nullptr)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 3, SQLDT_USHORT, &tmp_hotkey.lv, 0, nullptr, nullptr) )
		SqlStmt_ShowDebug(stmt);

	while( SQL_SUCCESS == SqlStmt_NextRow(stmt) )
	{
		if( hotkey_num >= 0 && hotkey_num < MAX_HOTKEYS_DB )
			memcpy(&p->hotkeys[hotkey_num], &tmp_hotkey, sizeof(tmp_hotkey));
		else
			ShowWarning("mmo_char_fromsql: ignoring invalid hotkey (hotkey=%d,type=%u,id=%u,lv=%u) of character %s (AID=%d,CID=%d)\n", hotkey_num, tmp_hotkey.type, tmp_hotkey.id, tmp_hotkey.lv, p->name, p->account_id, p->char_id);
	}
	StringBuf_AppendStr(&msg_buf, " hotkeys");
#endif

	/* Mercenary Owner DataBase */
	mercenary_owner_fromsql(char_id, p);
	StringBuf_AppendStr(&msg_buf, " mercenary");


	if (charserv_config.save_log)
		ShowInfo("Loaded char (%d - %s): %s\n", char_id, p->name, StringBuf_Value(&msg_buf)); //ok. all data load successfully!
	SqlStmt_Free(stmt);

	std::shared_ptr<struct mmo_charstatus> cp = util::umap_find( char_get_chardb(), char_id );

	if( cp == nullptr ){
		cp = std::make_shared<struct mmo_charstatus>();
		cp->char_id = char_id;
		char_get_chardb()[cp->char_id] = cp;
	}

	memcpy( cp.get(), p, sizeof( struct mmo_charstatus ) );

	StringBuf_Destroy(&msg_buf);
	return 1;
}

//==========================================================================================================
int char_mmo_sql_init(void) {
	ShowStatus("Characters per Account: '%d'.\n", charserv_config.char_config.char_per_account);

	//the 'set offline' part is now in check_login_conn ...
	//if the server connects to loginserver
	//it will dc all off players
	//and send the loginserver the new state....

	// Force all users offline in sql when starting char-server
	// (useful when servers crashs and don't clean the database)
	char_set_all_offline_sql();

	return 0;
}

//-----------------------------------
// Function to change chararcter's names
//-----------------------------------
int char_rename_char_sql(struct char_session_data *sd, uint32 char_id)
{
	struct mmo_charstatus char_dat;
	char esc_name[NAME_LENGTH*2+1];

	if( sd->new_name[0] == 0 ) // Not ready for rename
		return 2;

	if( !char_mmo_char_fromsql(char_id, &char_dat, false) ) // Only the short data is needed.
		return 2;

	// If the new name is exactly the same as the old one
	if( !strcmp( sd->new_name, char_dat.name ) )
		return 0;

	if( char_dat.rename == 0 )
		return 1;

	if( !charserv_config.char_config.char_rename_party && char_dat.party_id ){
		return 6;
	}

	if( !charserv_config.char_config.char_rename_guild && char_dat.guild_id ){
		return 5;
	}

	Sql_EscapeStringLen(sql_handle, esc_name, sd->new_name, strnlen(sd->new_name, NAME_LENGTH));

	switch( char_check_char_name( sd->new_name, esc_name ) ){
		case 0:
			break;
		case -1:
			// character already exists
			return 4;
		default:
			return 8;
	}

	if( SQL_ERROR == Sql_Query(sql_handle, "UPDATE `%s` SET `name` = '%s', `rename` = '%d' WHERE `char_id` = '%d'", schema_config.char_db, esc_name, --char_dat.rename, char_id) )
	{
		Sql_ShowDebug(sql_handle);
		return 3;
	}
	
	// Update party and party members with the new player name
	if( char_dat.party_id )
		inter_party_charname_changed(char_dat.party_id, char_id, sd->new_name);

	// Change character's name into guild_db.
	if( char_dat.guild_id )
		inter_guild_charname_changed(char_dat.guild_id, sd->account_id, char_id, sd->new_name);

	safestrncpy(char_dat.name, sd->new_name, NAME_LENGTH);
	memset(sd->new_name,0,sizeof(sd->new_name));

	// log change
	if( charserv_config.log_char )
	{
		if( SQL_ERROR == Sql_Query(sql_handle, "INSERT INTO `%s` (`time`, `char_msg`,`account_id`,`char_num`,`name`,`str`,`agi`,`vit`,`int`,`dex`,`luk`,`hair`,`hair_color`)"
			"VALUES (NOW(), '%s', '%d', '%d', '%s', '0', '0', '0', '0', '0', '0', '0', '0')",
			schema_config.charlog_db, "change char name", sd->account_id, char_dat.slot, esc_name) )
			Sql_ShowDebug(sql_handle);
	}

	return 0;
}

int char_check_char_name(char * name, char * esc_name)
{
	int i;

	// check length of character name
	if( name[0] == '\0' )
		return -2; // empty character name
	/**
	 * By default the client does not allow you to create names with less than 4 characters,
	 * however the use of WPE can bypass this, and this fixes the exploit.
	 * It can also be changed in the configuration file in conjunction with the
	 * 'Remove 4/6 letter Character Name Limit' client diff patch.
	 **/
	if( strlen( name ) < charserv_config.char_config.char_name_min_length )
		return -2;
	// check content of character name
	if( remove_control_chars(name) )
		return -2; // control chars in name

	// check for reserved names
	if( strcmpi(name, charserv_config.wisp_server_name) == 0 )
		return -1; // nick reserved for internal server messages

	// check for the channel symbol
	if( name[0] == '#' )
		return -2;

	// Check Authorised letters/symbols in the name of the character
	if( charserv_config.char_config.char_name_option == 1 )
	{ // only letters/symbols in char_name_letters are authorised
		for( i = 0; i < NAME_LENGTH && name[i]; i++ )
			if( strchr(charserv_config.char_config.char_name_letters, name[i]) == nullptr )
				return -2;
	}
	else if( charserv_config.char_config.char_name_option == 2 )
	{ // letters/symbols in char_name_letters are forbidden
		for( i = 0; i < NAME_LENGTH && name[i]; i++ )
			if( strchr(charserv_config.char_config.char_name_letters, name[i]) != nullptr )
				return -2;
	}
	if( charserv_config.char_config.name_ignoring_case ) {
		if( SQL_ERROR == Sql_Query(sql_handle, "SELECT 1 FROM `%s` WHERE BINARY `name` = '%s' LIMIT 1", schema_config.char_db, esc_name) ) {
			Sql_ShowDebug(sql_handle);
			return -2;
		}
	} else {
		if( SQL_ERROR == Sql_Query(sql_handle, "SELECT 1 FROM `%s` WHERE `name` = '%s' LIMIT 1", schema_config.char_db, esc_name) ) {
			Sql_ShowDebug(sql_handle);
			return -2;
		}
	}
	if( Sql_NumRows(sql_handle) > 0 )
		return -1; // name already exists

	return 0;
}

//-----------------------------------
// Function to create a new character
//-----------------------------------
int char_make_new_char( struct char_session_data* sd, char* name_, int str, int agi, int vit, int int_, int dex, int luk, int slot, int hair_color, int hair_style, short start_job, int sex ){
	char name[NAME_LENGTH];
	char esc_name[NAME_LENGTH*2+1];
	struct s_point_str tmp_start_point[MAX_STARTPOINT];
	struct startitem tmp_start_items[MAX_STARTITEM];
	uint32 char_id;
	int flag, k, start_point_idx = rnd() % charserv_config.start_point_count;
	int status_points;

	safestrncpy(name, name_, NAME_LENGTH);
	normalize_name(name,TRIM_CHARS);
	Sql_EscapeStringLen(sql_handle, esc_name, name, strnlen(name, NAME_LENGTH));

	memset( tmp_start_point, 0, sizeof( tmp_start_point ) );
	memset(tmp_start_items, 0, MAX_STARTITEM * sizeof(struct startitem));
	memcpy( tmp_start_point, charserv_config.start_point, sizeof( tmp_start_point ) );
	memcpy(tmp_start_items, charserv_config.start_items, MAX_STARTITEM * sizeof(struct startitem));

	flag = char_check_char_name(name,esc_name);
	if( flag < 0 )
		return flag;

	// Check inputs from the client - never trust it!

	// Check the slot
	if( slot < 0 || slot >= sd->char_slots ){
		return -4; // invalid slot
	}

	// Check gender
	switch( sex ){
		case SEX_FEMALE:
			sex = 'F';
			break;
		case SEX_MALE:
			sex = 'M';
			break;
		default:
			ShowWarning( "Received unsupported gender '%d'...\n", sex );
			return -2; // invalid input
	}

	// Check status values
#if PACKETVER < 20120307
	// All stats together always have to add up to a total of 30 points
	if( ( str + agi + vit + int_ + dex + luk ) != 30 ){
		return -2; // invalid input
	}

	// No status can be below 1
	if( str < 1 || agi < 1 || vit < 1 || int_ < 1 || dex < 1 || luk < 1 ){
		return -2; // invalid input
	}

	// No status can be higher than 9
	if( str > 9 || agi > 9 || vit > 9 || int_ > 9 || dex > 9 || luk > 9 ){
		return -2; // invalid input
	}

	// The status pairs always have to add up to a total of 10 points
	if( ( str + int_ ) != 10 || ( agi + luk ) != 10 || ( vit + dex ) != 10 ){
		return -2; // invalid input
	}

	status_points = 0;
#else
	status_points = charserv_config.start_status_points;
#endif

	// check the number of already existing chars in this account
	if( SQL_ERROR == Sql_Query(sql_handle, "SELECT 1 FROM `%s` WHERE `account_id` = '%d'", schema_config.char_db, sd->account_id) )
		Sql_ShowDebug(sql_handle);
#ifdef VIP_ENABLE
	if( Sql_NumRows(sql_handle) >= MAX_CHARS )
		return -2; // character account limit exceeded
#else
	if( Sql_NumRows(sql_handle) >= sd->char_slots )
		return -2; // character account limit exceeded
#endif

	// check char slot
	if( SQL_ERROR == Sql_Query(sql_handle, "SELECT 1 FROM `%s` WHERE `account_id` = '%d' AND `char_num` = '%d' LIMIT 1", schema_config.char_db, sd->account_id, slot) )
		Sql_ShowDebug(sql_handle);
	if( Sql_NumRows(sql_handle) > 0 )
		return -2; // slot already in use

	// validation success, log result
	if (charserv_config.log_char) {
		if( SQL_ERROR == Sql_Query(sql_handle, "INSERT INTO `%s` (`time`, `char_msg`,`account_id`,`char_num`,`name`,`str`,`agi`,`vit`,`int`,`dex`,`luk`,`hair`,`hair_color`)"
			"VALUES (NOW(), '%s', '%d', '%d', '%s', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d')",
			schema_config.charlog_db, "make new char", sd->account_id, slot, esc_name, str, agi, vit, int_, dex, luk, hair_style, hair_color) )
			Sql_ShowDebug(sql_handle);
	}

#if PACKETVER >= 20151001
	if(!(start_job == JOB_NOVICE && (charserv_config.allowed_job_flag&1)) && 
		!(start_job == JOB_SUMMONER && (charserv_config.allowed_job_flag&2)))
		return -2; // Invalid job

	// Check for Doram based information.
	if (start_job == JOB_SUMMONER) { // Check for just this job for now.
		memset( tmp_start_point, 0, sizeof( tmp_start_point ) );
		memset(tmp_start_items, 0, MAX_STARTITEM * sizeof(struct startitem));
		memcpy( tmp_start_point, charserv_config.start_point_doram, sizeof( tmp_start_point ) );
		memcpy(tmp_start_items, charserv_config.start_items_doram, MAX_STARTITEM * sizeof(struct startitem));
		start_point_idx = rnd() % charserv_config.start_point_count_doram;
	}
#endif

	//Insert the new char entry to the database
	if( SQL_ERROR == Sql_Query(sql_handle, "INSERT INTO `%s` (`account_id`, `char_num`, `name`, `class`, `zeny`, `status_point`, `str`, `agi`, `vit`, `int`, `dex`, `luk`, `max_hp`, `hp`,"
		"`max_sp`, `sp`, `hair`, `hair_color`, `last_map`, `last_x`, `last_y`, `save_map`, `save_x`, `save_y`, `sex`, `last_instanceid`) VALUES ("
		"'%d', '%d', '%s', '%d', '%d',  '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%u', '%u', '%u', '%u', '%d', '%d', '%s', '%d', '%d', '%s', '%d', '%d', '%c', '0')",
		schema_config.char_db, sd->account_id , slot, esc_name, start_job, charserv_config.start_zeny, status_points, str, agi, vit, int_, dex, luk,
		(40 * (100 + vit)/100) , (40 * (100 + vit)/100 ),  (11 * (100 + int_)/100), (11 * (100 + int_)/100), hair_style, hair_color,
		tmp_start_point[start_point_idx].map, tmp_start_point[start_point_idx].x, tmp_start_point[start_point_idx].y, tmp_start_point[start_point_idx].map, tmp_start_point[start_point_idx].x, tmp_start_point[start_point_idx].y, sex ) )
	{
		Sql_ShowDebug(sql_handle);
		return -2; //No, stop the procedure!
	}

	//Retrieve the newly auto-generated char id
	char_id = (int)Sql_LastInsertId(sql_handle);
	//Give the char the default items
	for (k = 0; k <= MAX_STARTITEM && tmp_start_items[k].nameid != 0; k++) {
		if( SQL_ERROR == Sql_Query(sql_handle, "INSERT INTO `%s` (`char_id`,`nameid`, `amount`, `equip`, `identify`) VALUES ('%d', '%u', '%hu', '%u', '%d')", schema_config.inventory_db, char_id, tmp_start_items[k].nameid, tmp_start_items[k].amount, tmp_start_items[k].pos, 1) )
			Sql_ShowDebug(sql_handle);
	}

	ShowInfo("Created char: account: %d, char: %d, slot: %d, name: %s\n", sd->account_id, char_id, slot, name);
	return char_id;
}

/*----------------------------------------------------------------------------------------------------------*/
/* Divorce Players */
/*----------------------------------------------------------------------------------------------------------*/
int char_divorce_char_sql(int partner_id1, int partner_id2){
	if( SQL_ERROR == Sql_Query(sql_handle, "UPDATE `%s` SET `partner_id`='0' WHERE `char_id`='%d' OR `char_id`='%d' LIMIT 2", schema_config.char_db, partner_id1, partner_id2) )
		Sql_ShowDebug(sql_handle);
	if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE (`nameid`='%u' OR `nameid`='%u') AND (`char_id`='%d' OR `char_id`='%d') LIMIT 2", schema_config.inventory_db, WEDDING_RING_M, WEDDING_RING_F, partner_id1, partner_id2) )
		Sql_ShowDebug(sql_handle);
	chmapif_send_ackdivorce(partner_id1, partner_id2);
	return 0;
}

/*----------------------------------------------------------------------------------------------------------*/
/* Delete char - davidsiaw */
/*----------------------------------------------------------------------------------------------------------*/
/* Returns 0 if successful
 * Returns < 0 for error
 */
enum e_char_del_response char_delete(struct char_session_data* sd, uint32 char_id){
	char name[NAME_LENGTH];
	char esc_name[NAME_LENGTH*2+1]; //Name needs be escaped.
	uint32 account_id;
	int party_id, guild_id, hom_id, base_level, partner_id, father_id, mother_id, elemental_id;
	time_t delete_date;
	char *data;
	size_t len;
	int i;

	ARR_FIND(0, MAX_CHARS, i, sd->found_char[i] == char_id);

	// Such a character does not exist in the account
	if (i == MAX_CHARS) {
		ShowInfo("Char deletion aborted: %s, Account ID: %u, Character ID: %u\n", name, sd->account_id, char_id);
		return CHAR_DELETE_NOTFOUND;
	}

	if (SQL_ERROR == Sql_Query(sql_handle, "SELECT `name`,`account_id`,`party_id`,`guild_id`,`base_level`,`homun_id`,`partner_id`,`father`,`mother`,`elemental_id`,`delete_date` FROM `%s` WHERE `account_id`='%u' AND `char_id`='%u'", schema_config.char_db, sd->account_id, char_id)){
		Sql_ShowDebug(sql_handle);
		return CHAR_DELETE_DATABASE;
	}

	if( SQL_SUCCESS != Sql_NextRow(sql_handle) )
	{
		ShowInfo("Char deletion aborted: %s, Account ID: %u, Character ID: %u\n", name, sd->account_id, char_id);
		Sql_FreeResult(sql_handle);
		return CHAR_DELETE_NOTFOUND;
	}

	Sql_GetData(sql_handle, 0, &data, &len); safestrncpy(name, data, NAME_LENGTH);
	Sql_GetData(sql_handle, 1, &data, nullptr); account_id = atoi(data);
	Sql_GetData(sql_handle, 2, &data, nullptr); party_id = atoi(data);
	Sql_GetData(sql_handle, 3, &data, nullptr); guild_id = atoi(data);
	Sql_GetData(sql_handle, 4, &data, nullptr); base_level = atoi(data);
	Sql_GetData(sql_handle, 5, &data, nullptr); hom_id = atoi(data);
	Sql_GetData(sql_handle, 6, &data, nullptr); partner_id = atoi(data);
	Sql_GetData(sql_handle, 7, &data, nullptr); father_id = atoi(data);
	Sql_GetData(sql_handle, 8, &data, nullptr); mother_id = atoi(data);
	Sql_GetData(sql_handle, 9, &data, nullptr); elemental_id = atoi(data);
	Sql_GetData(sql_handle,10, &data, nullptr); delete_date = strtoul(data, nullptr, 10);

	Sql_EscapeStringLen(sql_handle, esc_name, name, zmin(len, NAME_LENGTH));
	Sql_FreeResult(sql_handle);

	//check for config char del condition [Lupus]
	if( ( charserv_config.char_config.char_del_level > 0 && base_level >= charserv_config.char_config.char_del_level )
	 || ( charserv_config.char_config.char_del_level < 0 && base_level <= -charserv_config.char_config.char_del_level )
	) {
			ShowInfo("Char deletion aborted: %s, BaseLevel: %i\n", name, base_level);
			return CHAR_DELETE_BASELEVEL;
	}

	if (charserv_config.char_config.char_del_restriction&CHAR_DEL_RESTRICT_GUILD && guild_id) // character is in guild
	{
		ShowInfo("Char deletion aborted: %s, Guild ID: %i\n", name, guild_id);
		return CHAR_DELETE_GUILD;
	}

	if (charserv_config.char_config.char_del_restriction&CHAR_DEL_RESTRICT_PARTY && party_id) // character is in party
	{
		ShowInfo("Char deletion aborted: %s, Party ID: %i\n", name, party_id);
		return CHAR_DELETE_PARTY;
	}

	if( charserv_config.char_config.char_del_delay > 0 && ( !delete_date || delete_date > time(nullptr) ) ){ // not queued or delay not yet passed
		ShowInfo("Char deletion aborted: %s, Time was not set or has not been reached ye\n", name );
		return CHAR_DELETE_TIME;
	}

	/* Divorce [Wizputer] */
	if( partner_id )
		char_divorce_char_sql(char_id, partner_id);

	/* De-addopt [Zephyrus] */
	if( father_id || mother_id )
	{ // Char is Baby
		unsigned char buf[64];

		if( SQL_ERROR == Sql_Query(sql_handle, "UPDATE `%s` SET `child`='0' WHERE `char_id`='%d' OR `char_id`='%d'", schema_config.char_db, father_id, mother_id) )
			Sql_ShowDebug(sql_handle);
		if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `id` = '410' AND (`char_id`='%d' OR `char_id`='%d')", schema_config.skill_db, father_id, mother_id) )
			Sql_ShowDebug(sql_handle);

		WBUFW(buf,0) = 0x2b25;
		WBUFL(buf,2) = father_id;
		WBUFL(buf,6) = mother_id;
		WBUFL(buf,10) = char_id; // Baby
		chmapif_sendall(buf,14);
	}

	//Make the character leave the party [Skotlex]
	if (party_id)
		inter_party_leave(party_id, account_id, char_id, name);

	/* delete char's pet */
	//Delete the hatched pet if you have one...
	if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `char_id`='%d' AND `incubate` = '0'", schema_config.pet_db, char_id) )
		Sql_ShowDebug(sql_handle);

	//Delete all pets that are stored in eggs (inventory + cart)
	if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` USING `%s` JOIN `%s` ON `pet_id` = `card1`|`card2`<<16 WHERE `%s`.char_id = '%d' AND card0 = 256", schema_config.pet_db, schema_config.pet_db, schema_config.inventory_db, schema_config.inventory_db, char_id) )
		Sql_ShowDebug(sql_handle);
	if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` USING `%s` JOIN `%s` ON `pet_id` = `card1`|`card2`<<16 WHERE `%s`.char_id = '%d' AND card0 = 256", schema_config.pet_db, schema_config.pet_db, schema_config.cart_db, schema_config.cart_db, char_id) )
		Sql_ShowDebug(sql_handle);

	/* remove homunculus */
	if( hom_id )
		mapif_homunculus_delete(hom_id);

	/* remove elemental */
	if (elemental_id)
		mapif_elemental_delete(elemental_id);

	/* remove mercenary data */
	mercenary_owner_delete(char_id);

	/* delete char's friends list */
	if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `char_id` = '%d'", schema_config.friend_db, char_id) )
		Sql_ShowDebug(sql_handle);

	/* delete char from other's friend list */
	//NOTE: Won't this cause problems for people who are already online? [Skotlex]
	if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `friend_id` = '%d'", schema_config.friend_db, char_id) )
		Sql_ShowDebug(sql_handle);

#ifdef HOTKEY_SAVING
	/* delete hotkeys */
	if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `char_id`='%d'", schema_config.hotkey_db, char_id) )
		Sql_ShowDebug(sql_handle);
#endif

	/* delete inventory */
	if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `char_id`='%d'", schema_config.inventory_db, char_id) )
		Sql_ShowDebug(sql_handle);

	/* delete cart inventory */
	if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `char_id`='%d'", schema_config.cart_db, char_id) )
		Sql_ShowDebug(sql_handle);

	/* delete memo areas */
	if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `char_id`='%d'", schema_config.memo_db, char_id) )
		Sql_ShowDebug(sql_handle);

	/* delete character registry */
	if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `char_id`='%d'", schema_config.char_reg_str_table, char_id) )
		Sql_ShowDebug(sql_handle);
	if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `char_id`='%d'", schema_config.char_reg_num_table, char_id) )
		Sql_ShowDebug(sql_handle);

	/* delete skills */
	if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `char_id`='%d'", schema_config.skill_db, char_id) )
		Sql_ShowDebug(sql_handle);

	/* delete mail attachments (only received) */
	if (SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `id` IN ( SELECT `id` FROM `%s` WHERE `dest_id`='%d' )", schema_config.mail_attachment_db, schema_config.mail_db, char_id))
		Sql_ShowDebug(sql_handle);
	
	/* delete mails (only received) */
	if (SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `dest_id`='%d'", schema_config.mail_db, char_id))
		Sql_ShowDebug(sql_handle);

	/* mark mails as sent from server, if a character gets deleted */
	if (SQL_ERROR == Sql_Query(sql_handle, "UPDATE `%s` SET `send_id`='0' WHERE `send_id`='%d'", schema_config.mail_db, char_id))
		Sql_ShowDebug(sql_handle);

#ifdef ENABLE_SC_SAVING
	/* status changes */
	if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `account_id` = '%d' AND `char_id`='%d'", schema_config.scdata_db, account_id, char_id) )
		Sql_ShowDebug(sql_handle);
#endif

	/* bonus_scripts */
	if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `char_id` = '%d'", schema_config.bonus_script_db, char_id) )
		Sql_ShowDebug(sql_handle);

	/* Quest Data */
	if (SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `char_id` = '%d'", schema_config.quest_db, char_id))
		Sql_ShowDebug(sql_handle);

	/* Achievement Data */
	if (SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `char_id` = '%d'", schema_config.achievement_table, char_id))
		Sql_ShowDebug(sql_handle);

	if (charserv_config.log_char) {
		if( SQL_ERROR == Sql_Query(sql_handle, "INSERT INTO `%s`(`time`, `account_id`,`char_num`,`char_msg`,`name`) VALUES (NOW(), '%d', '%d', 'Deleted char (CID %d)', '%s')",
			schema_config.charlog_db, account_id, 0, char_id, esc_name) )
			Sql_ShowDebug(sql_handle);
	}

	/* delete character */
	if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `char_id`='%d'", schema_config.char_db, char_id) )
		Sql_ShowDebug(sql_handle);

	/* No need as we used inter_guild_leave [Skotlex]
	// Also delete info from guildtables.
	if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `char_id`='%d'", guild_member_db, char_id) )
		Sql_ShowDebug(sql_handle);
	*/

	if( SQL_ERROR == Sql_Query(sql_handle, "SELECT `guild_id` FROM `%s` WHERE `char_id` = '%d'", schema_config.guild_db, char_id) )
		Sql_ShowDebug(sql_handle);
	else if( Sql_NumRows(sql_handle) > 0 )
		mapif_parse_BreakGuild(0,guild_id);
	else if( guild_id )
		inter_guild_leave(guild_id, account_id, char_id);// Leave your guild.

	// refresh character list cache
	sd->found_char[i] = -1;

	return CHAR_DELETE_OK;
}

/**
 * This function parse all map-serv attached to this char-serv and increase user count
 * @return numbers of total users
 */
int char_count_users(void)
{
	int i, users;

	users = 0;
	for(i = 0; i < ARRAYLENGTH(map_server); i++) {
		if (session_isValid(map_server[i].fd)) {
			users += map_server[i].users;
		}
	}
	return users;
}

// Writes char data to the buffer in the format used by the client.
// Used in packets 0x6b (chars info) and 0x6d (new char info)
// Returns the size
int char_mmo_char_tobuf(uint8* buffer, struct mmo_charstatus* p){
	if( buffer == nullptr || p == nullptr )
		return 0;

	struct CHARACTER_INFO* info = (struct CHARACTER_INFO*)buffer;

	info->GID = p->char_id;
#if PACKETVER >= 20170830
	info->exp = u64min( p->base_exp, MAX_EXP );
#else
	info->exp = (int32)u64min( p->base_exp, MAX_EXP );
#endif
	info->money = p->zeny;
#if PACKETVER >= 20170830
	info->jobexp = u64min( p->job_exp, MAX_EXP );
#else
	info->jobexp = (int32)u64min( p->job_exp, MAX_EXP );
#endif
	info->joblevel = p->job_level;
	info->bodystate = 0; // probably opt1
	info->healthstate = 0; // probably opt2
	info->effectstate = p->option;
	info->virtue = p->karma;
	info->honor = p->manner;
	info->jobpoint = umin( p->status_point, INT16_MAX );
	info->hp = p->hp;
	info->maxhp = p->max_hp;
	info->sp = min( p->sp, INT16_MAX );
	info->maxsp = min( p->max_sp, INT16_MAX );
	info->speed = DEFAULT_WALK_SPEED; // p->speed;
	info->job = p->class_;
	info->head = p->hair;
#if PACKETVER >= 20141022
	info->body = p->body;
#endif
	//When the weapon is sent and your option is riding, the client crashes on login!?
	info->weapon = p->option&(0x20|0x80000|0x100000|0x200000|0x400000|0x800000|0x1000000|0x2000000|0x4000000|0x8000000) ? 0 : p->weapon;
	info->level = p->base_level;
	info->sppoint = umin( p->skill_point, INT16_MAX );
	info->accessory = p->head_bottom;
	info->shield = p->shield;
	info->accessory2 = p->head_top;
	info->accessory3 = p->head_mid;
	info->headpalette = p->hair_color;
	info->bodypalette = p->clothes_color;
	safestrncpy( info->name, p->name, NAME_LENGTH );
	info->Str = (uint8)u16min( p->str, UINT8_MAX );
	info->Agi = (uint8)u16min( p->agi, UINT8_MAX );
	info->Vit = (uint8)u16min( p->vit, UINT8_MAX );
	info->Int = (uint8)u16min( p->int_, UINT8_MAX );
	info->Dex = (uint8)u16min( p->dex, UINT8_MAX );
	info->Luk = (uint8)u16min( p->luk, UINT8_MAX );
	info->CharNum = p->slot;
	info->hairColor = (uint8)u16min( p->hair_color, UINT8_MAX );
	info->bIsChangedCharName = ( p->rename > 0 ) ? 0 : 1;
#if (PACKETVER >= 20100720 && PACKETVER <= 20100727) || PACKETVER >= 20100803
	mapindex_getmapname_ext( p->last_point.map, info->mapName );
#endif
#if PACKETVER >= 20100803
#if PACKETVER_CHAR_DELETEDATE
	info->DelRevDate = ( p->delete_date ? TOL( p->delete_date - time( nullptr ) ) : 0 );
#else
	info->DelRevDate = TOL( p->delete_date );
#endif
#endif
#if PACKETVER >= 20110111
	info->robePalette = p->robe;
#endif
#if PACKETVER >= 20110928
	// change slot feature (0 = disabled, otherwise enabled)
	if( charserv_config.charmove_config.char_move_enabled == 0 )
		info->chr_slot_changeCnt = 0;
	else if( charserv_config.charmove_config.char_moves_unlimited )
		info->chr_slot_changeCnt = 1;
	else
		info->chr_slot_changeCnt = max( 0, (int)p->character_moves );
#endif
#if PACKETVER >= 20111025
	info->chr_name_changeCnt = ( p->rename > 0 ) ? 1 : 0; // (0 = disabled, otherwise displays "Add-Ons" sidebar)
#endif
#if PACKETVER >= 20141016
	info->sex = p->sex; // sex - (0 = female, 1 = male, 99 = logindefined)
#endif

	return sizeof( struct CHARACTER_INFO );
}


int char_married(int pl1, int pl2)
{
	if( SQL_ERROR == Sql_Query(sql_handle, "SELECT `partner_id` FROM `%s` WHERE `char_id` = '%d'", schema_config.char_db, pl1) )
		Sql_ShowDebug(sql_handle);
	else if( SQL_SUCCESS == Sql_NextRow(sql_handle) )
	{
		char* data;

		Sql_GetData(sql_handle, 0, &data, nullptr);
		if( pl2 == atoi(data) )
		{
			Sql_FreeResult(sql_handle);
			return 1;
		}
	}
	Sql_FreeResult(sql_handle);
	return 0;
}

int char_child(int parent_id, int child_id)
{
	if( parent_id == 0 || child_id == 0) //Failsafe, avoild querys and fix EXP bug dividing with lower level chars
		return 0;
	if( SQL_ERROR == Sql_Query(sql_handle, "SELECT `child` FROM `%s` WHERE `char_id` = '%d'", schema_config.char_db, parent_id) )
		Sql_ShowDebug(sql_handle);
	else if( SQL_SUCCESS == Sql_NextRow(sql_handle) )
	{
		char* data;

		Sql_GetData(sql_handle, 0, &data, nullptr);
		if( child_id == atoi(data) )
		{
			Sql_FreeResult(sql_handle);
			return 1;
		}
	}
	Sql_FreeResult(sql_handle);
	return 0;
}

int char_family(int cid1, int cid2, int cid3)
{
	if ( cid1 == 0 || cid2 == 0 || cid3 == 0 ) //Failsafe, and avoid querys where there is no sense to keep executing if any of the inputs are 0
		return 0;
	if( SQL_ERROR == Sql_Query(sql_handle, "SELECT `char_id`,`partner_id`,`child` FROM `%s` WHERE `char_id` IN ('%d','%d','%d')", schema_config.char_db, cid1, cid2, cid3) )
		Sql_ShowDebug(sql_handle);
	else while( SQL_SUCCESS == Sql_NextRow(sql_handle) )
	{
		int charid;
		int partnerid;
		int childid;
		char* data;

		Sql_GetData(sql_handle, 0, &data, nullptr); charid = atoi(data);
		Sql_GetData(sql_handle, 1, &data, nullptr); partnerid = atoi(data);
		Sql_GetData(sql_handle, 2, &data, nullptr); childid = atoi(data);

		if( (cid1 == charid    && ((cid2 == partnerid && cid3 == childid  ) || (cid2 == childid   && cid3 == partnerid))) ||
			(cid1 == partnerid && ((cid2 == charid    && cid3 == childid  ) || (cid2 == childid   && cid3 == charid   ))) ||
			(cid1 == childid   && ((cid2 == charid    && cid3 == partnerid) || (cid2 == partnerid && cid3 == charid   ))) )
		{
			Sql_FreeResult(sql_handle);
			return childid;
		}
	}
	Sql_FreeResult(sql_handle);
	return 0;
}

//----------------------------------------------------------------------
// Force disconnection of an online player (with account value) by [Yor]
//----------------------------------------------------------------------
void char_disconnect_player(uint32 account_id)
{
	int i;
	struct char_session_data* sd;

	// disconnect player if online on char-server
	ARR_FIND( 0, fd_max, i, session[i] && (sd = (struct char_session_data*)session[i]->session_data) && sd->account_id == account_id );
	if( i < fd_max )
		set_eof(i);
}

/**
* Set 'flag' value of char_session_data
* @param account_id
* @param value
* @param set True: set the value by using '|= val', False: unset the value by using '&= ~val'
**/
void char_set_session_flag_(int account_id, int val, bool set) {
	int i;
	struct char_session_data* sd;

	ARR_FIND(0, fd_max, i, session[i] && (sd = (struct char_session_data*)session[i]->session_data) && sd->account_id == account_id);
	if (i < fd_max) {
		if (set)
			sd->flag |= val;
		else
			sd->flag &= ~val;
	}
}

void char_auth_ok(int fd, struct char_session_data *sd) {
	std::shared_ptr<struct online_char_data> character = util::umap_find( char_get_onlinedb(), sd->account_id );

	// Check if character is not online already. [Skotlex]
	if( character != nullptr ){
		if (character->server > -1)
		{	//Character already online. KICK KICK KICK
			mapif_disconnectplayer(map_server[character->server].fd, character->account_id, character->char_id, 2);
			if (character->waiting_disconnect == INVALID_TIMER)
				character->waiting_disconnect = add_timer(gettick()+AUTH_TIMEOUT, char_chardb_waiting_disconnect, character->account_id, 0);
			chclif_send_auth_result(fd,8);
			return;
		}
		if (session_isValid(character->fd) && character->fd != fd)
		{	//There's already a connection from this account that hasn't picked a char yet.
			chclif_send_auth_result(fd,8);
			return;
		}
		character->fd = fd;
	}

	chlogif_send_reqaccdata(login_fd,sd); // request account data

	// mark session as 'authed'
	sd->auth = true;

	// set char online on charserver
	char_set_charselect(sd->account_id);

	// continues when account data is received...
}

void char_read_fame_list(void)
{
	int i;
	char* data;
	size_t len;

	// Empty ranking lists
	memset(smith_fame_list, 0, sizeof(smith_fame_list));
	memset(chemist_fame_list, 0, sizeof(chemist_fame_list));
	memset(taekwon_fame_list, 0, sizeof(taekwon_fame_list));
	// Build Blacksmith ranking list
	if( SQL_ERROR == Sql_Query(sql_handle, "SELECT `char_id`,`fame`,`name` FROM `%s` WHERE `fame`>0 AND (`class`='%d' OR `class`='%d' OR `class`='%d' OR `class`='%d' OR `class`='%d' OR `class`='%d' OR `class`='%d') ORDER BY `fame` DESC LIMIT 0,%d", schema_config.char_db, JOB_BLACKSMITH, JOB_WHITESMITH, JOB_BABY_BLACKSMITH, JOB_MECHANIC, JOB_MECHANIC_T, JOB_BABY_MECHANIC, JOB_MEISTER, fame_list_size_smith) )
		Sql_ShowDebug(sql_handle);
	for( i = 0; i < fame_list_size_smith && SQL_SUCCESS == Sql_NextRow(sql_handle); ++i )
	{
		// char_id
		Sql_GetData(sql_handle, 0, &data, nullptr);
		smith_fame_list[i].id = atoi(data);
		// fame
		Sql_GetData(sql_handle, 1, &data, &len);
		smith_fame_list[i].fame = atoi(data);
		// name
		Sql_GetData(sql_handle, 2, &data, &len);
		memcpy(smith_fame_list[i].name, data, zmin(len, NAME_LENGTH));
	}
	// Build Alchemist ranking list
	if( SQL_ERROR == Sql_Query(sql_handle, "SELECT `char_id`,`fame`,`name` FROM `%s` WHERE `fame`>0 AND (`class`='%d' OR `class`='%d' OR `class`='%d' OR `class`='%d' OR `class`='%d' OR `class`='%d' OR `class`='%d') ORDER BY `fame` DESC LIMIT 0,%d", schema_config.char_db, JOB_ALCHEMIST, JOB_CREATOR, JOB_BABY_ALCHEMIST, JOB_GENETIC, JOB_GENETIC_T, JOB_BABY_GENETIC, JOB_BIOLO, fame_list_size_chemist) )
		Sql_ShowDebug(sql_handle);
	for( i = 0; i < fame_list_size_chemist && SQL_SUCCESS == Sql_NextRow(sql_handle); ++i )
	{
		// char_id
		Sql_GetData(sql_handle, 0, &data, nullptr);
		chemist_fame_list[i].id = atoi(data);
		// fame
		Sql_GetData(sql_handle, 1, &data, &len);
		chemist_fame_list[i].fame = atoi(data);
		// name
		Sql_GetData(sql_handle, 2, &data, &len);
		memcpy(chemist_fame_list[i].name, data, zmin(len, NAME_LENGTH));
	}
	// Build Taekwon ranking list
	if( SQL_ERROR == Sql_Query(sql_handle, "SELECT `char_id`,`fame`,`name` FROM `%s` WHERE `fame`>0 AND (`class`='%d' OR `class`='%d') ORDER BY `fame` DESC LIMIT 0,%d", schema_config.char_db, JOB_TAEKWON, JOB_BABY_TAEKWON, fame_list_size_taekwon) )
		Sql_ShowDebug(sql_handle);
	for( i = 0; i < fame_list_size_taekwon && SQL_SUCCESS == Sql_NextRow(sql_handle); ++i )
	{
		// char_id
		Sql_GetData(sql_handle, 0, &data, nullptr);
		taekwon_fame_list[i].id = atoi(data);
		// fame
		Sql_GetData(sql_handle, 1, &data, &len);
		taekwon_fame_list[i].fame = atoi(data);
		// name
		Sql_GetData(sql_handle, 2, &data, &len);
		memcpy(taekwon_fame_list[i].name, data, zmin(len, NAME_LENGTH));
	}
	Sql_FreeResult(sql_handle);
}

//Loads a character's name and stores it in the buffer given (must be NAME_LENGTH in size)
//Returns 1 on found, 0 on not found (buffer is filled with Unknown char name)
int char_loadName(uint32 char_id, char* name){
	char* data;
	size_t len;

	if( SQL_ERROR == Sql_Query(sql_handle, "SELECT `name` FROM `%s` WHERE `char_id`='%d'", schema_config.char_db, char_id) )
		Sql_ShowDebug(sql_handle);
	else if( SQL_SUCCESS == Sql_NextRow(sql_handle) )
	{
		Sql_GetData(sql_handle, 0, &data, &len);
		safestrncpy(name, data, NAME_LENGTH);
		return 1;
	}
#if PACKETVER < 20180221
	else
	{
		safestrncpy(name, charserv_config.char_config.unknown_char_name, NAME_LENGTH);
	}
#else
	name[0] = '\0';
#endif
	return 0;
}

// Searches for the mapserver that has a given map (and optionally ip/port, if not -1).
// If found, returns the server's index in the 'server' array (otherwise returns -1).
int char_search_mapserver( const std::string& map, uint32 ip, uint16 port ){
	for(int i = 0; i < ARRAYLENGTH(map_server); i++)
	{
		if (session_isValid(map_server[i].fd)
		&& (ip == (uint32)-1 || map_server[i].ip == ip)
		&& (port == (uint16)-1 || map_server[i].port == port))
		{
			for( std::string& m : map_server[i].maps ){
				if( m == map ){
					return i;
				}
			}
		}
	}

	return -1;
}

/**
 * Test to know if an IP come from LAN or WAN.
 * @param ip: ip to check if in auth network
 * @return 0 if from wan, or subnet_map_ip if lan
 **/
int char_lan_subnetcheck(uint32 ip){
	int i;
	ARR_FIND( 0, subnet_count, i, (subnet[i].char_ip & subnet[i].mask) == (ip & subnet[i].mask) );
	if( i < subnet_count ) {
		ShowInfo("Subnet check [%u.%u.%u.%u]: Matches " CL_CYAN "%u.%u.%u.%u/%u.%u.%u.%u" CL_RESET"\n", CONVIP(ip), CONVIP(subnet[i].char_ip & subnet[i].mask), CONVIP(subnet[i].mask));
		return subnet[i].map_ip;
	} else {
		ShowInfo("Subnet check [%u.%u.%u.%u]: " CL_CYAN "WAN" CL_RESET "\n", CONVIP(ip));
		return 0;
	}
}

// Console Command Parser [Wizputer]
//FIXME to be remove (moved to cnslif / will be done once map/char/login, all have their cnslif interface ready)
int parse_console(const char* buf){
	return cnslif_parse(buf);
}

#if PACKETVER_SUPPORTS_PINCODE
//------------------------------------------------
//Pincode system
//------------------------------------------------
int char_pincode_compare( int fd, struct char_session_data* sd, char* pin ){
	if( strcmp( sd->pincode, pin ) == 0 ){
		sd->pincode_try = 0;
		return 1;
	}else{
		chclif_pincode_sendstate( fd, sd, PINCODE_WRONG );

		if( charserv_config.pincode_config.pincode_maxtry && ++sd->pincode_try >= charserv_config.pincode_config.pincode_maxtry ){
			chlogif_pincode_notifyLoginPinError( sd->account_id );
		}

		return 0;
	}
}

bool char_pincode_decrypt( uint32 userSeed, char* pin ){
	int i;
	char tab[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
	char *buf;

	if (safestrnlen(pin, 4) != PINCODE_LENGTH)
		return false;

	for (i = 0; i < PINCODE_LENGTH; ++i) {
		if (!ISDIGIT(pin[i]))
			return false;
	}

	for( i = 1; i < 10; i++ ){
		int pos;
		uint32 multiplier = 0x3498, baseSeed = 0x881234;

		userSeed = baseSeed + userSeed * multiplier;
		pos = userSeed % ( i + 1 );
		if( i != pos ){
			tab[i] ^= tab[pos];
			tab[pos] ^= tab[i];
			tab[i] ^= tab[pos];
		}
	}

	buf = (char *)aMalloc( sizeof(char) * ( PINCODE_LENGTH + 1 ) );
	memset( buf, 0, PINCODE_LENGTH + 1 );
	for( i = 0; i < PINCODE_LENGTH; i++ ){
		sprintf( buf + i, "%d", tab[pin[i] - '0'] );
	}
	strcpy( pin, buf );
	aFree( buf );

	return true;
}
#endif

//------------------------------------------------
//Invoked 15 seconds after mapif_disconnectplayer in case the map server doesn't
//replies/disconnect the player we tried to kick. [Skotlex]
//------------------------------------------------
TIMER_FUNC(char_chardb_waiting_disconnect){
	std::shared_ptr<struct online_char_data> character = util::umap_find( char_get_onlinedb(), static_cast<uint32>( id ) );

	// Mark it offline due to timeout.
	if( character != nullptr && character->waiting_disconnect == tid ){	
		character->waiting_disconnect = INVALID_TIMER;
		char_set_char_offline(character->char_id, character->account_id);
	}
	return 0;
}

TIMER_FUNC(char_online_data_cleanup){
	for( auto it = char_get_onlinedb().begin(); it != char_get_onlinedb().end(); ){
		std::shared_ptr<struct online_char_data> character = it->second;

		// Character still connected
		if( character->fd != -1 ){
			return 0;
		}

		// Unknown server - set them offline
		if( character->server == -2 ){
			char_set_char_offline( character->char_id, character->account_id );
		}

		// Free data from players that have not been online for a while.
		if( character->server < 0 ){
			it = char_get_onlinedb().erase( it );
		}else{
			it++;
		}
	}

	return 0;
}

TIMER_FUNC(char_clan_member_cleanup){
	// Auto removal is disabled
	if( charserv_config.clan_remove_inactive_days <= 0 ){
		return 0;
	}

	if( SQL_ERROR == Sql_Query( sql_handle, "UPDATE `%s` SET `clan_id`='0' WHERE `online`='0' AND `clan_id`<>'0' AND `last_login` IS NOT NULL AND `last_login` <= NOW() - INTERVAL %d DAY", schema_config.char_db, charserv_config.clan_remove_inactive_days ) ){
		Sql_ShowDebug(sql_handle);
	}

	return 0;
}

//----------------------------------
// Reading Lan Support configuration
// Rewrote: Anvanced subnet check [LuzZza]
//----------------------------------
int char_lan_config_read(const char *lancfgName) {
	FILE *fp;
	int line_num = 0, s_subnet=ARRAYLENGTH(subnet);
	char line[1024], w1[64], w2[64], w3[64], w4[64];

	if((fp = fopen(lancfgName, "r")) == nullptr) {
		ShowWarning("LAN Support configuration file is not found: %s\n", lancfgName);
		return 1;
	}

	while(fgets(line, sizeof(line), fp)) {
		line_num++;
		if ((line[0] == '/' && line[1] == '/') || line[0] == '\n' || line[1] == '\n')
			continue;

		if(sscanf(line,"%63[^:]: %63[^:]:%63[^:]:%63[^\r\n]", w1, w2, w3, w4) != 4) {

			ShowWarning("Error syntax of configuration file %s in line %d.\n", lancfgName, line_num);
			continue;
		}

		remove_control_chars(w1);
		remove_control_chars(w2);
		remove_control_chars(w3);
		remove_control_chars(w4);

		if( strcmpi(w1, "subnet") == 0 ){
			if(subnet_count>=s_subnet) { //We skip instead of break in case we want to add other conf in that file.
				ShowError("%s: Too many subnets defined, skipping line %d...\n", lancfgName, line_num);
				continue;
			}
			subnet[subnet_count].mask = str2ip(w2);
			subnet[subnet_count].char_ip = str2ip(w3);
			subnet[subnet_count].map_ip = str2ip(w4);
			if( (subnet[subnet_count].char_ip & subnet[subnet_count].mask) != (subnet[subnet_count].map_ip & subnet[subnet_count].mask) )
			{
				ShowError("%s: Configuration Error: The char server (%s) and map server (%s) belong to different subnetworks!\n", lancfgName, w3, w4);
				continue;
			}
			subnet_count++;
		}
	}

	if( subnet_count > 1 ) /* only useful if there is more than 1 */
		ShowStatus("Read information about %d subnetworks.\n", subnet_count);

	fclose(fp);
	return 0;
}

/**
 * Check if our table are all ok in sqlserver
 * Char tables to check
 * @return 0:fail, 1:success
 */
bool char_checkdb(void){
	int i;
	const char* sqltable[] = {
		schema_config.char_db, schema_config.hotkey_db, schema_config.scdata_db, schema_config.cart_db, 
                schema_config.inventory_db, schema_config.charlog_db,
                schema_config.char_reg_str_table, schema_config.char_reg_num_table, schema_config.acc_reg_str_table,
                schema_config.acc_reg_num_table, schema_config.skill_db, schema_config.interlog_db, schema_config.memo_db,
		schema_config.guild_db, schema_config.guild_alliance_db, schema_config.guild_castle_db, 
                schema_config.guild_expulsion_db, schema_config.guild_member_db, 
                schema_config.guild_skill_db, schema_config.guild_position_db, schema_config.guild_storage_db,
		schema_config.party_db, schema_config.pet_db, schema_config.friend_db, schema_config.mail_db, 
                schema_config.auction_db, schema_config.quest_db, schema_config.homunculus_db, schema_config.skill_homunculus_db,
                schema_config.mercenary_db, schema_config.mercenary_owner_db,
		schema_config.elemental_db, schema_config.skillcooldown_db, schema_config.bonus_script_db,
		schema_config.clan_table, schema_config.clan_alliance_table, schema_config.mail_attachment_db, schema_config.achievement_table
	};
	ShowInfo("Start checking DB integrity\n");
	for (i=0; i<ARRAYLENGTH(sqltable); i++){ //check if they all exist and we can acces them in sql-server
		if( SQL_ERROR == Sql_Query(sql_handle, "SELECT 1 FROM `%s` LIMIT 1;", sqltable[i]) ){
			Sql_ShowDebug(sql_handle);
			return false;
		}
	}
	//checking char_db
	if( SQL_ERROR == Sql_Query(sql_handle, "SELECT `char_id`,`account_id`,`char_num`,`name`,`class`,"
		"`base_level`,`job_level`,`base_exp`,`job_exp`,`zeny`,`str`,`agi`,`vit`,`int`,`dex`,`luk`,"
		"`max_hp`,`hp`,`max_sp`,`sp`,`status_point`,`skill_point`,`option`,`karma`,`manner`,`party_id`,"
		"`guild_id`,`pet_id`,`homun_id`,`elemental_id`,`hair`,`hair_color`,`clothes_color`,`weapon`,"
		"`shield`,`head_top`,`head_mid`,`head_bottom`,`robe`,`last_map`,`last_x`,`last_y`,`save_map`,"
		"`save_x`,`save_y`,`partner_id`,`online`,`father`,`mother`,`child`,`fame`,`rename`,`delete_date`,"
		"`moves`,`unban_time`,`font`,`sex`,`hotkey_rowshift`,`clan_id`,`last_login`,`title_id`,`show_equip`,"
		"`hotkey_rowshift2`,"
		"`max_ap`,`ap`,`trait_point`,`pow`,`sta`,`wis`,`spl`,`con`,`crt`,"
		"`inventory_slots`,`body_direction`,`disable_call`,`last_instanceid`"
		" FROM `%s` LIMIT 1;", schema_config.char_db) ){
		Sql_ShowDebug(sql_handle);
		return false;
	}
	//checking charlog_db
	if( SQL_ERROR == Sql_Query(sql_handle, "SELECT `time`,`char_msg`,`account_id`,`char_num`,`name`,"
		"`str`,`agi`,`vit`,`int`,`dex`,`luk`,`hair`,`hair_color`"
		" FROM `%s` LIMIT 1;", schema_config.charlog_db) ){
		Sql_ShowDebug(sql_handle);
		return false;
	}
	//checking char_reg_str_table
	if( SQL_ERROR == Sql_Query(sql_handle, "SELECT `char_id`,`key`,`index`,`value` from `%s` LIMIT 1;", schema_config.char_reg_str_table) ) {
		Sql_ShowDebug(sql_handle);
		return false;
	}
	//checking char_reg_num_table
	if( SQL_ERROR == Sql_Query(sql_handle, "SELECT `char_id`,`key`,`index`,`value` from `%s` LIMIT 1;", schema_config.char_reg_num_table) ) {
		Sql_ShowDebug(sql_handle);
		return false;
	}
	//checking acc_reg_str_table
	if( SQL_ERROR == Sql_Query(sql_handle, "SELECT `account_id`,`key`,`index`,`value` from `%s` LIMIT 1;", schema_config.acc_reg_str_table) ) {
		Sql_ShowDebug(sql_handle);
		return false;
	}
	//checking acc_reg_num_table
	if( SQL_ERROR == Sql_Query(sql_handle, "SELECT `account_id`,`key`,`index`,`value` from `%s` LIMIT 1;", schema_config.acc_reg_num_table) ) {
		Sql_ShowDebug(sql_handle);
		return false;
	}
	//checking hotkey_db
	if( SQL_ERROR == Sql_Query(sql_handle, "SELECT  `char_id`,`hotkey`,`type`,`itemskill_id`,`skill_lvl`"
		" FROM `%s` LIMIT 1;", schema_config.hotkey_db) ){
		Sql_ShowDebug(sql_handle);
		return false;
	}
	//checking scdata_db
	if( SQL_ERROR == Sql_Query(sql_handle, "SELECT  `account_id`,`char_id`,`type`,`tick`,`val1`,`val2`,`val3`,`val4`"
		" FROM `%s` LIMIT 1;", schema_config.scdata_db) ){
		Sql_ShowDebug(sql_handle);
		return false;
	}
	//checking skill_db
	if( SQL_ERROR == Sql_Query(sql_handle, "SELECT  `char_id`,`id`,`lv`,`flag` FROM `%s` LIMIT 1;", schema_config.skill_db) ){
		Sql_ShowDebug(sql_handle);
		return false;
	}
	//checking interlog_db
	if( SQL_ERROR == Sql_Query(sql_handle, "SELECT  `time`,`log` FROM `%s` LIMIT 1;", schema_config.interlog_db) ){
		Sql_ShowDebug(sql_handle);
		return false;
	}
	//checking memo_db
	if( SQL_ERROR == Sql_Query(sql_handle, "SELECT  `memo_id`,`char_id`,`map`,`x`,`y` FROM `%s` LIMIT 1;", schema_config.memo_db) ){
		Sql_ShowDebug(sql_handle);
		return false;
	}
	//checking guild_db
	if( SQL_ERROR == Sql_Query(sql_handle, "SELECT  `guild_id`,`name`,`char_id`,`master`,`guild_lv`,"
			"`connect_member`,`max_member`,`average_lv`,`exp`,`next_exp`,`skill_point`,`mes1`,`mes2`,"
			"`emblem_len`,`emblem_id`,`emblem_data`,`last_master_change`"
			" FROM `%s` LIMIT 1;", schema_config.guild_db) ){
		Sql_ShowDebug(sql_handle);
		return false;
	}
	//checking guild_alliance_db
	if( SQL_ERROR == Sql_Query(sql_handle, "SELECT  `guild_id`,`opposition`,`alliance_id`,`name` FROM `%s` LIMIT 1;", schema_config.guild_alliance_db) ){
		Sql_ShowDebug(sql_handle);
		return false;
	}
	//checking guild_castle_db
	if( SQL_ERROR == Sql_Query(sql_handle, "SELECT  `castle_id`,`guild_id`,`economy`,`defense`,`triggerE`,"
			"`triggerD`,`nextTime`,`payTime`,`createTime`,`visibleC`,`visibleG0`,`visibleG1`,`visibleG2`,"
			"`visibleG3`,`visibleG4`,`visibleG5`,`visibleG6`,`visibleG7` "
			" FROM `%s` LIMIT 1;", schema_config.guild_castle_db) ){
		Sql_ShowDebug(sql_handle);
		return false;
	}
	//checking guild_expulsion_db
	if( SQL_ERROR == Sql_Query(sql_handle, "SELECT  `guild_id`,`account_id`,`name`,`mes`,`char_id` FROM `%s` LIMIT 1;", schema_config.guild_expulsion_db) ){
		Sql_ShowDebug(sql_handle);
		return false;
	}
	//checking guild_member_db
	if( SQL_ERROR == Sql_Query(sql_handle, "SELECT  `guild_id`,`char_id`,`exp`,`position` FROM `%s` LIMIT 1;", schema_config.guild_member_db) ){
		Sql_ShowDebug(sql_handle);
		return false;
	}
	//checking guild_skill_db
	if( SQL_ERROR == Sql_Query(sql_handle, "SELECT  `guild_id`,`id`,`lv` FROM `%s` LIMIT 1;", schema_config.guild_skill_db) ){
		Sql_ShowDebug(sql_handle);
		return false;
	}
	//checking guild_position_db
	if( SQL_ERROR == Sql_Query(sql_handle, "SELECT  `guild_id`,`position`,`name`,`mode`,`exp_mode` FROM `%s` LIMIT 1;", schema_config.guild_position_db) ){
		Sql_ShowDebug(sql_handle);
		return false;
	}
	//checking party_db
	if( SQL_ERROR == Sql_Query(sql_handle, "SELECT  `party_id`,`name`,`exp`,`item`,`leader_id`,`leader_char` FROM `%s` LIMIT 1;", schema_config.party_db) ){
		Sql_ShowDebug(sql_handle);
		return false;
	}
	//checking pet_db
	if( SQL_ERROR == Sql_Query(sql_handle, "SELECT  `pet_id`,`class`,`name`,`account_id`,`char_id`,`level`,"
			"`egg_id`,`equip`,`intimate`,`hungry`,`rename_flag`,`incubate`"
			" FROM `%s` LIMIT 1;", schema_config.pet_db) ){
		Sql_ShowDebug(sql_handle);
		return false;
	}
	//checking friend_db
	if( SQL_ERROR == Sql_Query(sql_handle, "SELECT  `char_id`,`friend_id` FROM `%s` LIMIT 1;", schema_config.friend_db) ){
		Sql_ShowDebug(sql_handle);
		return false;
	}
	//checking mail_db
	if( SQL_ERROR == Sql_Query(sql_handle, "SELECT  `id`,`send_name`,`send_id`,`dest_name`,`dest_id`,"
			"`title`,`message`,`time`,`status`,`zeny`,`type`"
			" FROM `%s` LIMIT 1;", schema_config.mail_db) ){
		Sql_ShowDebug(sql_handle);
		return false;
	}
	//checking mail_attachment_db
	if( SQL_ERROR == Sql_Query(sql_handle, "SELECT  `id`,`index`,`nameid`,`amount`,`refine`,`attribute`,`identify`,"
			"`card0`,`card1`,`card2`,`card3`,`option_id0`,`option_val0`,`option_parm0`,`option_id1`,`option_val1`,`option_parm1`,`option_id2`,`option_val2`,`option_parm2`,`option_id3`,`option_val3`,`option_parm3`,`option_id4`,`option_val4`,`option_parm4`,`unique_id`, `bound`, `enchantgrade`"
			" FROM `%s` LIMIT 1;", schema_config.mail_attachment_db) ){
		Sql_ShowDebug(sql_handle);
		return false;
	}

	//checking auction_db
	if( SQL_ERROR == Sql_Query(sql_handle, "SELECT  `auction_id`,`seller_id`,`seller_name`,`buyer_id`,`buyer_name`,"
			"`price`,`buynow`,`hours`,`timestamp`,`nameid`,`item_name`,`type`,`refine`,`attribute`,`card0`,`card1`,"
			"`card2`,`card3`,`option_id0`,`option_val0`,`option_parm0`,`option_id1`,`option_val1`,`option_parm1`,`option_id2`,`option_val2`,`option_parm2`,`option_id3`,`option_val3`,`option_parm3`,`option_id4`,`option_val4`,`option_parm4`,`unique_id`, `enchantgrade` "
			"FROM `%s` LIMIT 1;", schema_config.auction_db) ){
		Sql_ShowDebug(sql_handle);
		return false;
	}
	//checking quest_db
	if( SQL_ERROR == Sql_Query(sql_handle, "SELECT  `char_id`,`quest_id`,`state`,`time`,`count1`,`count2`,`count3` from `%s`;", schema_config.quest_db) ){
		Sql_ShowDebug(sql_handle);
		return false;
	}
	//checking homunculus_db
	if( SQL_ERROR == Sql_Query(sql_handle, "SELECT  `homun_id`,`char_id`,`class`,`prev_class`,`name`,`level`,`exp`,`intimacy`,`hunger`,"
		"`str`,`agi`,`vit`,`int`,`dex`,`luk`,`hp`,`max_hp`,`sp`,`max_sp`,`skill_point`,`alive`,`rename_flag`,`vaporize`,`autofeed` "
		" FROM `%s` LIMIT 1;", schema_config.homunculus_db) ){
		Sql_ShowDebug(sql_handle);
		return false;
	}
	//checking skill_homunculus_db
	if( SQL_ERROR == Sql_Query(sql_handle, "SELECT  `homun_id`,`id`,`lv` FROM `%s` LIMIT 1;", schema_config.skill_homunculus_db) ){
		Sql_ShowDebug(sql_handle);
		return false;
	}
	//checking mercenary_db
	if( SQL_ERROR == Sql_Query(sql_handle, "SELECT  `mer_id`,`char_id`,`class`,`hp`,`sp`,`kill_counter`,`life_time` FROM `%s` LIMIT 1;", schema_config.mercenary_db) ){
		Sql_ShowDebug(sql_handle);
		return false;
	}
	//checking mercenary_owner_db
	if( SQL_ERROR == Sql_Query(sql_handle, "SELECT  `char_id`,`merc_id`,`arch_calls`,`arch_faith`,"
		"`spear_calls`,`spear_faith`,`sword_calls`,`sword_faith`"
		" FROM `%s` LIMIT 1;", schema_config.mercenary_owner_db) ){
		Sql_ShowDebug(sql_handle);
		return false;
	}
	//checking elemental_db
	if( SQL_ERROR == Sql_Query(sql_handle, "SELECT  `ele_id`,`char_id`,`class`,`mode`,`hp`,`sp`,`max_hp`,`max_sp`,"
		"`atk1`,`atk2`,`matk`,`aspd`,`def`,`mdef`,`flee`,`hit`,`life_time` "
		" FROM `%s` LIMIT 1;", schema_config.elemental_db) ){
		Sql_ShowDebug(sql_handle);
		return false;
	}
	//checking skillcooldown_db
	if( SQL_ERROR == Sql_Query(sql_handle, "SELECT  `account_id`,`char_id`,`skill`,`tick` FROM `%s` LIMIT 1;", schema_config.skillcooldown_db) ){
		Sql_ShowDebug(sql_handle);
		return false;
	}
	//checking bonus_script_db
	if( SQL_ERROR == Sql_Query(sql_handle, "SELECT  `char_id`,`script`,`tick`,`flag`,`type`,`icon` FROM `%s` LIMIT 1;", schema_config.bonus_script_db) ){
		Sql_ShowDebug(sql_handle);
		return false;
	}
	//checking cart_db
	if( SQL_ERROR == Sql_Query(sql_handle, "SELECT  `id`,`char_id`,`nameid`,`amount`,`equip`,`identify`,`refine`,"
		"`attribute`,`card0`,`card1`,`card2`,`card3`,`option_id0`,`option_val0`,`option_parm0`,`option_id1`,`option_val1`,`option_parm1`,`option_id2`,`option_val2`,`option_parm2`,`option_id3`,`option_val3`,`option_parm3`,`option_id4`,`option_val4`,`option_parm4`,`expire_time`,`bound`,`unique_id`,`enchantgrade`"
		" FROM `%s` LIMIT 1;", schema_config.cart_db) ){
		Sql_ShowDebug(sql_handle);
		return false;
	}
	//checking inventory_db
	if( SQL_ERROR == Sql_Query(sql_handle, "SELECT  `id`,`char_id`,`nameid`,`amount`,`equip`,`identify`,`refine`,"
		"`attribute`,`card0`,`card1`,`card2`,`card3`,`option_id0`,`option_val0`,`option_parm0`,`option_id1`,`option_val1`,`option_parm1`,`option_id2`,`option_val2`,`option_parm2`,`option_id3`,`option_val3`,`option_parm3`,`option_id4`,`option_val4`,`option_parm4`,`expire_time`,`bound`,`unique_id`,`enchantgrade`"
		",`favorite`,`equip_switch`"
		" FROM `%s` LIMIT 1;", schema_config.inventory_db) ){
		Sql_ShowDebug(sql_handle);
		return false;
	}
	//checking guild_storage_db
	if( SQL_ERROR == Sql_Query(sql_handle, "SELECT  `id`,`guild_id`,`nameid`,`amount`,`equip`,`identify`,`refine`,"
		"`attribute`,`card0`,`card1`,`card2`,`card3`,`option_id0`,`option_val0`,`option_parm0`,`option_id1`,`option_val1`,`option_parm1`,`option_id2`,`option_val2`,`option_parm2`,`option_id3`,`option_val3`,`option_parm3`,`option_id4`,`option_val4`,`option_parm4`,`expire_time`,`bound`,`unique_id`,`enchantgrade`"
		" FROM `%s` LIMIT 1;", schema_config.guild_storage_db) ){
		Sql_ShowDebug(sql_handle);
		return false;
	}
	//checking clan table
	if( SQL_ERROR == Sql_Query(sql_handle, "SELECT  `clan_id`,`name`,`master`,`mapname`,`max_member`"
		" FROM `%s` LIMIT 1;", schema_config.clan_table) ){
		Sql_ShowDebug(sql_handle);
		return false;
	}
	//checking clan alliance table
	if( SQL_ERROR == Sql_Query(sql_handle, "SELECT  `clan_id`,`opposition`,`alliance_id`,`name`"
		" FROM `%s` LIMIT 1;", schema_config.clan_alliance_table) ){
		Sql_ShowDebug(sql_handle);
		return false;
	}
	//checking achievement_table
	if (SQL_ERROR == Sql_Query(sql_handle, "SELECT `char_id`,`id`,`count1`,`count2`,`count3`,`count4`,`count5`,`count6`,`count7`,`count8`,`count9`,`count10`,`completed`,`rewarded`"
		" FROM `%s` LIMIT 1;", schema_config.achievement_table)) {
		Sql_ShowDebug(sql_handle);
		return false;
	}
	Sql_FreeResult(sql_handle);
	ShowInfo("DB integrity check finished with success\n");
	return true;
}

void char_sql_config_read(const char* cfgName) {
	char line[1024], w1[1024], w2[1024];
	FILE* fp;

	if ((fp = fopen(cfgName, "r")) == nullptr) {
		ShowError("File not found: %s\n", cfgName);
		return;
	}

	while(fgets(line, sizeof(line), fp)) {
		if(line[0] == '/' && line[1] == '/')
			continue;

		if (sscanf(line, "%1023[^:]: %1023[^\r\n]", w1, w2) != 2)
			continue;

		if(!strcmpi(w1,"char_db"))
			safestrncpy(schema_config.char_db, w2, sizeof(schema_config.char_db));
		else if(!strcmpi(w1,"scdata_db"))
			safestrncpy(schema_config.scdata_db, w2, sizeof(schema_config.scdata_db));
		else if(!strcmpi(w1,"cart_db"))
			safestrncpy(schema_config.cart_db, w2, sizeof(schema_config.cart_db));
		else if(!strcmpi(w1,"inventory_db"))
			safestrncpy(schema_config.inventory_db, w2, sizeof(schema_config.inventory_db));
		else if(!strcmpi(w1,"charlog_db"))
			safestrncpy(schema_config.charlog_db, w2, sizeof(schema_config.charlog_db));
		else if(!strcmpi(w1,"skill_db"))
			safestrncpy(schema_config.skill_db, w2, sizeof(schema_config.skill_db));
		else if(!strcmpi(w1,"interlog_db"))
			safestrncpy(schema_config.interlog_db, w2, sizeof(schema_config.interlog_db));
		else if(!strcmpi(w1,"memo_db"))
			safestrncpy(schema_config.memo_db, w2, sizeof(schema_config.memo_db));
		else if(!strcmpi(w1,"guild_db"))
			safestrncpy(schema_config.guild_db, w2, sizeof(schema_config.guild_db));
		else if(!strcmpi(w1,"guild_alliance_db"))
			safestrncpy(schema_config.guild_alliance_db, w2, sizeof(schema_config.guild_alliance_db));
		else if(!strcmpi(w1,"guild_castle_db"))
			safestrncpy(schema_config.guild_castle_db, w2, sizeof(schema_config.guild_castle_db));
		else if(!strcmpi(w1,"guild_expulsion_db"))
			safestrncpy(schema_config.guild_expulsion_db, w2, sizeof(schema_config.guild_expulsion_db));
		else if(!strcmpi(w1,"guild_member_db"))
			safestrncpy(schema_config.guild_member_db, w2, sizeof(schema_config.guild_member_db));
		else if(!strcmpi(w1,"guild_skill_db"))
			safestrncpy(schema_config.guild_skill_db, w2, sizeof(schema_config.guild_skill_db));
		else if(!strcmpi(w1,"guild_position_db"))
			safestrncpy(schema_config.guild_position_db, w2, sizeof(schema_config.guild_position_db));
		else if(!strcmpi(w1,"guild_storage_db"))
			safestrncpy(schema_config.guild_storage_db, w2, sizeof(schema_config.guild_storage_db));
		else if(!strcmpi(w1,"party_db"))
			safestrncpy(schema_config.party_db, w2, sizeof(schema_config.party_db));
		else if(!strcmpi(w1,"pet_db"))
			safestrncpy(schema_config.pet_db, w2, sizeof(schema_config.pet_db));
		else if(!strcmpi(w1,"mail_db"))
			safestrncpy(schema_config.mail_db, w2, sizeof(schema_config.mail_db));
		else if (!strcmpi(w1, "mail_attachment_db"))
			safestrncpy(schema_config.mail_attachment_db, w2, sizeof(schema_config.mail_attachment_db));
		else if(!strcmpi(w1,"auction_db"))
			safestrncpy(schema_config.auction_db, w2, sizeof(schema_config.auction_db));
		else if(!strcmpi(w1,"friend_db"))
			safestrncpy(schema_config.friend_db, w2, sizeof(schema_config.friend_db));
		else if(!strcmpi(w1,"hotkey_db"))
			safestrncpy(schema_config.hotkey_db, w2, sizeof(schema_config.hotkey_db));
		else if(!strcmpi(w1,"quest_db"))
			safestrncpy(schema_config.quest_db,w2,sizeof(schema_config.quest_db));
		else if(!strcmpi(w1,"homunculus_db"))
			safestrncpy(schema_config.homunculus_db,w2,sizeof(schema_config.homunculus_db));
		else if(!strcmpi(w1,"skill_homunculus_db"))
			safestrncpy(schema_config.skill_homunculus_db,w2,sizeof(schema_config.skill_homunculus_db));
		else if(!strcmpi(w1,"mercenary_db"))
			safestrncpy(schema_config.mercenary_db,w2,sizeof(schema_config.mercenary_db));
		else if(!strcmpi(w1,"mercenary_owner_db"))
			safestrncpy(schema_config.mercenary_owner_db,w2,sizeof(schema_config.mercenary_owner_db));
		else if(!strcmpi(w1,"elemental_db"))
			safestrncpy(schema_config.elemental_db,w2,sizeof(schema_config.elemental_db));
		else if(!strcmpi(w1,"skillcooldown_db"))
			safestrncpy(schema_config.skillcooldown_db, w2, sizeof(schema_config.skillcooldown_db));
		else if(!strcmpi(w1,"bonus_script_db"))
			safestrncpy(schema_config.bonus_script_db, w2, sizeof(schema_config.bonus_script_db));
		else if(!strcmpi(w1,"char_reg_num_table"))
			safestrncpy(schema_config.char_reg_num_table, w2, sizeof(schema_config.char_reg_num_table));
		else if(!strcmpi(w1,"char_reg_str_table"))
			safestrncpy(schema_config.char_reg_str_table, w2, sizeof(schema_config.char_reg_str_table));
		else if(!strcmpi(w1,"acc_reg_str_table"))
			safestrncpy(schema_config.acc_reg_str_table, w2, sizeof(schema_config.acc_reg_str_table));
		else if(!strcmpi(w1,"acc_reg_num_table"))
			safestrncpy(schema_config.acc_reg_num_table, w2, sizeof(schema_config.acc_reg_num_table));
		else if(!strcmpi(w1,"clan_table"))
			safestrncpy(schema_config.clan_table, w2, sizeof(schema_config.clan_table));
		else if(!strcmpi(w1,"clan_alliance_table"))
			safestrncpy(schema_config.clan_alliance_table, w2, sizeof(schema_config.clan_alliance_table));
		else if(!strcmpi(w1,"achievement_table"))
			safestrncpy(schema_config.achievement_table, w2, sizeof(schema_config.achievement_table));
		else if(!strcmpi(w1, "start_status_points"))
			charserv_config.start_status_points = atoi(w2);
		//support the import command, just like any other config
		else if(!strcmpi(w1,"import"))
			char_sql_config_read(w2);
	}
	fclose(fp);
	ShowInfo("Done reading %s.\n", cfgName);
}


void char_set_default_sql(){
//	schema_config.db_use_sqldbs;
	safestrncpy(schema_config.db_path,"db",sizeof(schema_config.db_path));
	safestrncpy(schema_config.char_db,"char",sizeof(schema_config.char_db));
	safestrncpy(schema_config.scdata_db,"sc_data",sizeof(schema_config.scdata_db));
	safestrncpy(schema_config.cart_db,"cart_inventory",sizeof(schema_config.cart_db));
	safestrncpy(schema_config.inventory_db,"inventory",sizeof(schema_config.inventory_db));
	safestrncpy(schema_config.charlog_db,"charlog",sizeof(schema_config.charlog_db));
	safestrncpy(schema_config.storage_db,"storage",sizeof(schema_config.storage_db));
	safestrncpy(schema_config.interlog_db,"interlog",sizeof(schema_config.interlog_db));
	safestrncpy(schema_config.skill_db,"skill",sizeof(schema_config.skill_db));
	safestrncpy(schema_config.memo_db,"memo",sizeof(schema_config.memo_db));
	safestrncpy(schema_config.guild_db,"guild",sizeof(schema_config.guild_db));
	safestrncpy(schema_config.guild_alliance_db,"guild_alliance",sizeof(schema_config.guild_alliance_db));
	safestrncpy(schema_config.guild_castle_db,"guild_castle",sizeof(schema_config.guild_castle_db));
	safestrncpy(schema_config.guild_expulsion_db,"guild_expulsion",sizeof(schema_config.guild_expulsion_db));
	safestrncpy(schema_config.guild_member_db,"guild_member",sizeof(schema_config.guild_member_db));
	safestrncpy(schema_config.guild_position_db,"guild_position",sizeof(schema_config.guild_position_db));
	safestrncpy(schema_config.guild_skill_db,"guild_skill",sizeof(schema_config.guild_skill_db));
	safestrncpy(schema_config.guild_storage_db,"guild_storage",sizeof(schema_config.guild_storage_db));
	safestrncpy(schema_config.party_db,"party",sizeof(schema_config.party_db));
	safestrncpy(schema_config.pet_db,"pet",sizeof(schema_config.pet_db));
	safestrncpy(schema_config.mail_db,"mail",sizeof(schema_config.mail_db)); // MAIL SYSTEM
	safestrncpy(schema_config.auction_db,"auction",sizeof(schema_config.auction_db)); // Auctions System
	safestrncpy(schema_config.friend_db,"friends",sizeof(schema_config.friend_db));
	safestrncpy(schema_config.hotkey_db,"hotkey",sizeof(schema_config.hotkey_db));
	safestrncpy(schema_config.quest_db,"quest",sizeof(schema_config.quest_db));
	safestrncpy(schema_config.homunculus_db,"homunculus",sizeof(schema_config.homunculus_db));
	safestrncpy(schema_config.skill_homunculus_db,"skill_homunculus",sizeof(schema_config.skill_homunculus_db));
	safestrncpy(schema_config.mercenary_db,"mercenary",sizeof(schema_config.mercenary_db));
	safestrncpy(schema_config.mercenary_owner_db,"mercenary_owner",sizeof(schema_config.mercenary_owner_db));
	safestrncpy(schema_config.skillcooldown_db,"skillcooldown",sizeof(schema_config.skillcooldown_db));
	safestrncpy(schema_config.bonus_script_db,"bonus_script",sizeof(schema_config.bonus_script_db));
	safestrncpy(schema_config.char_reg_num_table,"char_reg_num",sizeof(schema_config.char_reg_num_table));
	safestrncpy(schema_config.char_reg_str_table,"char_reg_str",sizeof(schema_config.char_reg_str_table));
	safestrncpy(schema_config.acc_reg_str_table,"acc_reg_str",sizeof(schema_config.acc_reg_str_table));
	safestrncpy(schema_config.acc_reg_num_table,"acc_reg_num",sizeof(schema_config.acc_reg_num_table));
	safestrncpy(schema_config.clan_table,"clan",sizeof(schema_config.clan_table));
	safestrncpy(schema_config.clan_table,"clan_alliance",sizeof(schema_config.clan_alliance_table));
	safestrncpy(schema_config.achievement_table,"achievement",sizeof(schema_config.achievement_table));
}

//set default config
void char_set_defaults(){
#if PACKETVER_SUPPORTS_PINCODE
	charserv_config.pincode_config.pincode_enabled = true;
	charserv_config.pincode_config.pincode_changetime = 0;
	charserv_config.pincode_config.pincode_maxtry = 3;
	charserv_config.pincode_config.pincode_force = true;
	charserv_config.pincode_config.pincode_allow_repeated = false;
	charserv_config.pincode_config.pincode_allow_sequential = false;
#endif

	charserv_config.charmove_config.char_move_enabled = true;
	charserv_config.charmove_config.char_movetoused = true;
	charserv_config.charmove_config.char_moves_unlimited = false;

	charserv_config.char_config.char_per_account = 0; //Maximum chars per account (default unlimited) [Sirius]
	charserv_config.char_config.char_del_level = 0; //From which level u can delete character [Lupus]
	charserv_config.char_config.char_del_delay = 86400;
#if PACKETVER >= 20100803
	charserv_config.char_config.char_del_option = CHAR_DEL_BIRTHDATE;
#else
	charserv_config.char_config.char_del_option = CHAR_DEL_EMAIL;
#endif
	charserv_config.char_config.char_del_restriction = CHAR_DEL_RESTRICT_ALL;

//	charserv_config.userid[24];
//	charserv_config.passwd[24];
//	charserv_config.server_name[20];
	safestrncpy(charserv_config.wisp_server_name,"Server",sizeof(charserv_config.wisp_server_name));
//	charserv_config.login_ip_str[128];
	charserv_config.login_ip = 0;
	charserv_config.login_port = 6900;
//	charserv_config.char_ip_str[128];
	charserv_config.char_ip = 0;
//	charserv_config.bind_ip_str[128];
	charserv_config.bind_ip = INADDR_ANY;
	charserv_config.char_port = 6121;
	charserv_config.char_maintenance = 0;
	charserv_config.char_new = true;
	charserv_config.char_new_display = 0;

	charserv_config.char_config.name_ignoring_case = false; // Allow or not identical name for characters but with a different case by [Yor]
	charserv_config.char_config.char_name_option = 0; // Option to know which letters/symbols are authorised in the name of a character (0: all, 1: only those in char_name_letters, 2: all EXCEPT those in char_name_letters) by [Yor]
	safestrncpy(charserv_config.char_config.unknown_char_name,"Unknown",sizeof(charserv_config.char_config.unknown_char_name)); // Name to use when the requested name cannot be determined
	safestrncpy(charserv_config.char_config.char_name_letters,"",sizeof(charserv_config.char_config.char_name_letters)); // list of letters/symbols allowed (or not) in a character name. by [Yor]
	charserv_config.char_config.char_name_min_length = 4; // Minimum character name length

	charserv_config.save_log = 1; // show loading/saving messages
	charserv_config.log_char = 1;	// loggin char or not [devil]
	charserv_config.log_inter = 1;	// loggin inter or not [devil]
	charserv_config.char_check_db =1;

	// See const.hpp to change the default values
	safestrncpy( charserv_config.start_point[0].map, MAP_DEFAULT_NAME, sizeof( charserv_config.start_point[0].map ) ); 
	charserv_config.start_point[0].x = MAP_DEFAULT_X;
	charserv_config.start_point[0].y = MAP_DEFAULT_Y;
	charserv_config.start_point_count = 1;

#if PACKETVER >= 20151001
	safestrncpy( charserv_config.start_point_doram[0].map, MAP_DEFAULT_NAME, sizeof( charserv_config.start_point_doram[0].map ) );
	charserv_config.start_point_doram[0].x = MAP_DEFAULT_X;
	charserv_config.start_point_doram[0].y = MAP_DEFAULT_Y;
	charserv_config.start_point_count_doram = 1;
#endif

	charserv_config.start_items[0].nameid = 1201;
	charserv_config.start_items[0].amount = 1;
	charserv_config.start_items[0].pos = EQP_HAND_R;
	charserv_config.start_items[1].nameid = 2301;
	charserv_config.start_items[1].amount = 1;
	charserv_config.start_items[1].pos = EQP_ARMOR;

#if PACKETVER >= 20150101
	charserv_config.start_items_doram[0].nameid = 1681;
	charserv_config.start_items_doram[0].amount = 1;
	charserv_config.start_items_doram[0].pos = EQP_HAND_R;
	charserv_config.start_items_doram[1].nameid = 2301;
	charserv_config.start_items_doram[1].amount = 1;
	charserv_config.start_items_doram[1].pos = EQP_ARMOR;
#endif

	charserv_config.start_status_points = 48;

	charserv_config.console = 0;
	charserv_config.max_connect_user = -1;
	charserv_config.gm_allow_group = -1;
	charserv_config.autosave_interval = DEFAULT_AUTOSAVE_INTERVAL;
	charserv_config.start_zeny = 0;
	charserv_config.guild_exp_rate = 100;

	charserv_config.clan_remove_inactive_days = 14;
	charserv_config.mail_return_days = 14;
	charserv_config.mail_delete_days = 14;
	charserv_config.mail_retrieve = 1;

#if defined(RENEWAL) && PACKETVER >= 20151001
	charserv_config.allowed_job_flag = 3;
#else
	charserv_config.allowed_job_flag = 1;
#endif

	charserv_config.clear_parties = 0;
}

/**
 * Split start_point configuration values.
 * @param w1_value: Value from w1
 * @param w2_value: Value from w2
 * @param start: Start point reference
 * @param count: Start point count reference
 */
void char_config_split_startpoint( char* w1_value, char* w2_value, struct s_point_str start_point[MAX_STARTPOINT], short* count ){
	char *lineitem, **fields;
	int i = 0;
	size_t fields_length = 3 + 1;

	(*count) = 0; // Reset to begin reading
	memset(start_point, 0, sizeof(struct s_point_str) * MAX_STARTPOINT);

	fields = (char **)aMalloc(fields_length * sizeof(char *));
	if (fields == nullptr)
		return; // Failed to allocate memory.
	lineitem = strtok(w2_value, ":");

	while (lineitem != nullptr && (*count) < MAX_STARTPOINT) {
		bool error;
		size_t n = sv_split( lineitem, strlen( lineitem ), 0, ',', fields, fields_length, SV_NOESCAPE_NOTERMINATE, error );

		if( error || ( n + 1 ) < fields_length ){
			ShowDebug("%s: not enough arguments for %s! Skipping...\n", w1_value, lineitem);
			lineitem = strtok(nullptr, ":"); //next lineitem
			continue;
		}

		safestrncpy( start_point[i].map, fields[1], sizeof( start_point[i].map ) );
		start_point[i].x = max( 0, atoi( fields[2] ) );
		start_point[i].y = max( 0, atoi( fields[3] ) );
		(*count)++;

		lineitem = strtok(nullptr, ":"); //next lineitem
		i++;
	}
	aFree(fields);
}

/**
 * Split start_items configuration values.
 * @param w1_value: Value from w1
 * @param w2_value: Value from w2
 * @param start: Start item reference
 */
void char_config_split_startitem(char *w1_value, char *w2_value, struct startitem start_items[MAX_STARTITEM])
{
	char *lineitem, **fields;
	int i = 0;
	size_t fields_length = 3 + 1;

	memset(start_items, 0, sizeof(struct startitem) * MAX_STARTITEM);

	fields = (char **)aMalloc(fields_length * sizeof(char *));
	if (fields == nullptr)
		return; // Failed to allocate memory.
	lineitem = strtok(w2_value, ":");

	while (lineitem != nullptr && i < MAX_STARTITEM) {
		bool error;
		size_t n = sv_split( lineitem, strlen( lineitem ), 0, ',', fields, fields_length, SV_NOESCAPE_NOTERMINATE, error );

		if( error || ( n + 1 ) < fields_length ){
			ShowDebug("%s: not enough arguments for %s! Skipping...\n", w1_value, lineitem);
			lineitem = strtok(nullptr, ":"); //next lineitem
			continue;
		}

		// TODO: Item ID verification
		start_items[i].nameid = strtoul( fields[1], nullptr, 10 );
		// TODO: Stack verification
		start_items[i].amount = min( (uint16)strtoul( fields[2], nullptr, 10 ), MAX_AMOUNT );
		start_items[i].pos = strtoul( fields[3], nullptr, 10 );

		lineitem = strtok(nullptr, ":"); //next lineitem
		i++;
	}
	aFree(fields);
}

bool char_config_read(const char* cfgName, bool normal){
	char line[1024], w1[1024], w2[1024];
	FILE* fp = fopen(cfgName, "r");

	if (fp == nullptr) {
		ShowError("Configuration file not found: %s.\n", cfgName);
		return false;
	}

	while(fgets(line, sizeof(line), fp)) {
		if (line[0] == '/' && line[1] == '/')
			continue;

		if (sscanf(line, "%1023[^:]: %1023[^\r\n]", w1, w2) != 2)
			continue;

		remove_control_chars(w1);
		remove_control_chars(w2);

		// Config that loaded only when server started, not by reloading config file
		if (normal) {
			if (strcmpi(w1, "userid") == 0) {
				safestrncpy(charserv_config.userid, w2, sizeof(charserv_config.userid));
			} else if (strcmpi(w1, "passwd") == 0) {
				safestrncpy(charserv_config.passwd, w2, sizeof(charserv_config.passwd));
			} else if (strcmpi(w1, "server_name") == 0) {
				safestrncpy(charserv_config.server_name, w2, sizeof(charserv_config.server_name));
			} else if (strcmpi(w1, "wisp_server_name") == 0) {
				if (strlen(w2) >= 4) {
					safestrncpy(charserv_config.wisp_server_name, w2, sizeof(charserv_config.wisp_server_name));
				}
			} else if (strcmpi(w1, "login_ip") == 0) {
				charserv_config.login_ip = host2ip(w2);
				if (charserv_config.login_ip) {
					char ip_str[16];
					safestrncpy(charserv_config.login_ip_str, w2, sizeof(charserv_config.login_ip_str));
					ShowStatus("Login server IP address : %s -> %s\n", w2, ip2str(charserv_config.login_ip, ip_str));
				}
			} else if (strcmpi(w1, "login_port") == 0) {
				charserv_config.login_port = atoi(w2);
			} else if (strcmpi(w1, "char_ip") == 0) {
				charserv_config.char_ip = host2ip(w2);
				if (charserv_config.char_ip) {
					char ip_str[16];
					safestrncpy(charserv_config.char_ip_str, w2, sizeof(charserv_config.char_ip_str));
					ShowStatus("Character server IP address : %s -> %s\n", w2, ip2str(charserv_config.char_ip, ip_str));
				}
			} else if (strcmpi(w1, "bind_ip") == 0) {
				charserv_config.bind_ip = host2ip(w2);
				if (charserv_config.bind_ip) {
					char ip_str[16];
					safestrncpy(charserv_config.bind_ip_str, w2, sizeof(charserv_config.bind_ip_str));
					ShowStatus("Character server binding IP address : %s -> %s\n", w2, ip2str(charserv_config.bind_ip, ip_str));
				}
			} else if (strcmpi(w1, "char_port") == 0) {
				charserv_config.char_port = atoi(w2);
			} else if (strcmpi(w1, "console") == 0) {
				charserv_config.console = config_switch(w2);
			}
		}

		if(strcmpi(w1,"timestamp_format") == 0) {
			safestrncpy(timestamp_format, w2, sizeof(timestamp_format));
		} else if(strcmpi(w1,"console_silent")==0){
			msg_silent = atoi(w2);
			if( msg_silent ) /* only bother if its actually enabled */
				ShowInfo("Console Silent Setting: %d\n", atoi(w2));
		} else if (strcmpi(w1, "console_msg_log") == 0) {
			console_msg_log = atoi(w2);
		} else if  (strcmpi(w1, "console_log_filepath") == 0) {
			safestrncpy(console_log_filepath, w2, sizeof(console_log_filepath));
		} else if(strcmpi(w1,"stdout_with_ansisequence")==0){
			stdout_with_ansisequence = config_switch(w2);
		} else if (strcmpi(w1, "char_maintenance") == 0) {
			charserv_config.char_maintenance = atoi(w2);
		} else if (strcmpi(w1, "char_new") == 0) {
			charserv_config.char_new = (bool)config_switch(w2);
		} else if (strcmpi(w1, "char_new_display") == 0) {
			charserv_config.char_new_display = atoi(w2);
		} else if (strcmpi(w1, "max_connect_user") == 0) {
			charserv_config.max_connect_user = atoi(w2);
			if (charserv_config.max_connect_user < -1)
				charserv_config.max_connect_user = -1;
		} else if(strcmpi(w1, "gm_allow_group") == 0) {
			charserv_config.gm_allow_group = atoi(w2);
		} else if (strcmpi(w1, "autosave_time") == 0) {
			charserv_config.autosave_interval = atoi(w2)*1000;
			if (charserv_config.autosave_interval <= 0)
				charserv_config.autosave_interval = DEFAULT_AUTOSAVE_INTERVAL;
		} else if (strcmpi(w1, "save_log") == 0) {
			charserv_config.save_log = config_switch(w2);
#ifdef RENEWAL
		} else if (strcmpi(w1, "start_point") == 0) {
#else
		} else if (strcmpi(w1, "start_point_pre") == 0) {
#endif
			char_config_split_startpoint(w1, w2, charserv_config.start_point, &charserv_config.start_point_count);
#if PACKETVER >= 20151001
		} else if (strcmpi(w1, "start_point_doram") == 0) {
			char_config_split_startpoint(w1, w2, charserv_config.start_point_doram, &charserv_config.start_point_count_doram);
#endif
		} else if (strcmpi(w1, "start_zeny") == 0) {
			charserv_config.start_zeny = atoi(w2);
			if (charserv_config.start_zeny < 0)
				charserv_config.start_zeny = 0;
#ifdef RENEWAL
		} else if (strcmpi(w1, "start_items") == 0) {
#else
		} else if (strcmpi(w1, "start_items_pre") == 0) {
#endif
			char_config_split_startitem(w1, w2, charserv_config.start_items);
#if PACKETVER >= 20151001
		} else if (strcmpi(w1, "start_items_doram") == 0) {
			char_config_split_startitem(w1, w2, charserv_config.start_items_doram);
#endif
		} else if(strcmpi(w1,"log_char")==0) {		//log char or not [devil]
			charserv_config.log_char = config_switch(w2);
		} else if (strcmpi(w1, "unknown_char_name") == 0) {
			safestrncpy(charserv_config.char_config.unknown_char_name, w2, sizeof(charserv_config.char_config.unknown_char_name));
			charserv_config.char_config.unknown_char_name[NAME_LENGTH-1] = '\0';
		} else if (strcmpi(w1, "name_ignoring_case") == 0) {
			charserv_config.char_config.name_ignoring_case = (bool)config_switch(w2);
		} else if (strcmpi(w1, "char_name_option") == 0) {
			charserv_config.char_config.char_name_option = atoi(w2);
		} else if (strcmpi(w1, "char_name_letters") == 0) {
			safestrncpy(charserv_config.char_config.char_name_letters, w2, sizeof(charserv_config.char_config.char_name_letters));
		} else if (strcmpi(w1, "char_name_min_length") == 0) {
			charserv_config.char_config.char_name_min_length = cap_value(atoi(w2), 0, NAME_LENGTH - 1);
		} else if (strcmpi(w1, "char_del_level") == 0) { //disable/enable char deletion by its level condition [Lupus]
			charserv_config.char_config.char_del_level = atoi(w2);
		} else if (strcmpi(w1, "char_del_delay") == 0) {
			charserv_config.char_config.char_del_delay = atoi(w2);
		} else if (strcmpi(w1, "char_del_option") == 0) {
			charserv_config.char_config.char_del_option = atoi(w2);
		} else if (strcmpi(w1, "char_del_restriction") == 0) {
			charserv_config.char_config.char_del_restriction = atoi(w2);
		} else if (strcmpi(w1, "char_rename_party") == 0) {
			charserv_config.char_config.char_rename_party = (bool)config_switch(w2);
		} else if (strcmpi(w1, "char_rename_guild") == 0) {
			charserv_config.char_config.char_rename_guild = (bool)config_switch(w2);
		} else if(strcmpi(w1,"db_path")==0) {
			safestrncpy(schema_config.db_path, w2, sizeof(schema_config.db_path));
		} else if (strcmpi(w1, "fame_list_alchemist") == 0) {
			fame_list_size_chemist = atoi(w2);
			if (fame_list_size_chemist > MAX_FAME_LIST) {
				ShowWarning("Max fame list size is %d (fame_list_alchemist)\n", MAX_FAME_LIST);
				fame_list_size_chemist = MAX_FAME_LIST;
			}
		} else if (strcmpi(w1, "fame_list_blacksmith") == 0) {
			fame_list_size_smith = atoi(w2);
			if (fame_list_size_smith > MAX_FAME_LIST) {
				ShowWarning("Max fame list size is %d (fame_list_blacksmith)\n", MAX_FAME_LIST);
				fame_list_size_smith = MAX_FAME_LIST;
			}
		} else if (strcmpi(w1, "fame_list_taekwon") == 0) {
			fame_list_size_taekwon = atoi(w2);
			if (fame_list_size_taekwon > MAX_FAME_LIST) {
				ShowWarning("Max fame list size is %d (fame_list_taekwon)\n", MAX_FAME_LIST);
				fame_list_size_taekwon = MAX_FAME_LIST;
			}
		} else if (strcmpi(w1, "guild_exp_rate") == 0) {
			charserv_config.guild_exp_rate = atoi(w2);
		} else if (strcmpi(w1, "pincode_enabled") == 0) {
#if PACKETVER_SUPPORTS_PINCODE
			charserv_config.pincode_config.pincode_enabled = config_switch(w2);
		} else if (strcmpi(w1, "pincode_changetime") == 0) {
			charserv_config.pincode_config.pincode_changetime = atoi(w2)*60*60*24;
		} else if (strcmpi(w1, "pincode_maxtry") == 0) {
			charserv_config.pincode_config.pincode_maxtry = atoi(w2);
		} else if (strcmpi(w1, "pincode_force") == 0) {
			charserv_config.pincode_config.pincode_force = config_switch(w2);
		}  else if (strcmpi(w1, "pincode_allow_repeated") == 0) {
			charserv_config.pincode_config.pincode_allow_repeated = config_switch(w2);
		}  else if (strcmpi(w1, "pincode_allow_sequential") == 0) {
			charserv_config.pincode_config.pincode_allow_sequential = config_switch(w2);
#else
			if( config_switch(w2) ) {
				ShowWarning("pincode_enabled requires PACKETVER 20110309 or higher.\n");
			}
#endif
		} else if (strcmpi(w1, "char_move_enabled") == 0) {
			charserv_config.charmove_config.char_move_enabled = config_switch(w2);
		} else if (strcmpi(w1, "char_movetoused") == 0) {
			charserv_config.charmove_config.char_movetoused = config_switch(w2);
		} else if (strcmpi(w1, "char_moves_unlimited") == 0) {
			charserv_config.charmove_config.char_moves_unlimited = config_switch(w2);
		} else if (strcmpi(w1, "char_checkdb") == 0) {
			charserv_config.char_check_db = config_switch(w2);
		} else if (strcmpi(w1, "clan_remove_inactive_days") == 0) {
			charserv_config.clan_remove_inactive_days = atoi(w2);
		} else if (strcmpi(w1, "mail_return_days") == 0) {
			charserv_config.mail_return_days = atoi(w2);
		} else if (strcmpi(w1, "mail_delete_days") == 0) {
			charserv_config.mail_delete_days = atoi(w2);
		} else if (strcmpi(w1, "mail_return_empty") == 0) {
			charserv_config.mail_return_empty = config_switch(w2);
		} else if (strcmpi(w1, "allowed_job_flag") == 0) {
			charserv_config.allowed_job_flag = atoi(w2);
		} else if (strcmpi(w1, "clear_parties") == 0) {
			charserv_config.clear_parties = config_switch(w2);
		} else if (strcmpi(w1, "import") == 0) {
			char_config_read(w2, normal);
		}
	}
	fclose(fp);

	ShowInfo("Done reading %s.\n", cfgName);
	return true;
}

/**
 * Checks for values out of range.
 */
void char_config_adjust() {
#if PACKETVER < 20100803
	if (charserv_config.char_config.char_del_option&CHAR_DEL_BIRTHDATE) {
		ShowWarning("conf/char_athena.conf:char_del_option birthdate is enabled but it requires PACKETVER 2010-08-03 or newer, defaulting to email...\n");
		charserv_config.char_config.char_del_option &= ~CHAR_DEL_BIRTHDATE;
	}
#endif
}

/*
 * Message conf function
 */
int char_msg_config_read(const char *cfgName){
	return _msg_config_read(cfgName,CHAR_MAX_MSG,msg_table);
}
const char* char_msg_txt(int msg_number){
	return _msg_txt(msg_number,CHAR_MAX_MSG,msg_table);
}
void char_do_final_msg(void){
	_do_final_msg(CHAR_MAX_MSG,msg_table);
}

void CharacterServer::finalize(){
	ShowStatus("Terminating...\n");

	char_set_all_offline(-1);
	char_set_all_offline_sql();

	inter_final();

	flush_fifos();

	do_final_msg();
	do_final_chmapif();
	do_final_chlogif();

	char_get_chardb().clear();
	char_get_onlinedb().clear();
	char_get_authdb().clear();

	if( char_fd != -1 )
	{
		do_close(char_fd);
		char_fd = -1;
	}

	Sql_Free(sql_handle);

	ShowStatus("Finished.\n");
}

/// Called when a terminate signal is received.
void CharacterServer::handle_shutdown(){
	ShowStatus("Shutting down...\n");
	// TODO proper shutdown procedure; wait for acks?, kick all characters, ... [FlavoJS]
	for( int id = 0; id < ARRAYLENGTH(map_server); ++id )
		chmapif_server_reset(id);
	flush_fifos();
}

bool CharacterServer::initialize( int argc, char *argv[] ){
	// Init default value
	safestrncpy(console_log_filepath, "./log/char-msg_log.log", sizeof(console_log_filepath));

	cli_get_options(argc,argv);

	char_set_defaults();
	char_config_read(CHAR_CONF_NAME, true);
	char_config_adjust();
	char_lan_config_read(LAN_CONF_NAME);
	char_set_default_sql();
	char_sql_config_read(INTER_CONF_NAME);
	msg_config_read(MSG_CONF_NAME_EN);

#if !defined(BUILDBOT)
	if (strcmp(charserv_config.userid, "s1")==0 && strcmp(charserv_config.passwd, "p1")==0) {
		ShowWarning("Using the default user/password s1/p1 is NOT RECOMMENDED.\n");
		ShowNotice("Please edit your 'login' table to create a proper inter-server user/password (gender 'S')\n");
		ShowNotice("And then change the user/password to use in conf/char_athena.conf (or conf/import/char_conf.txt)\n");
	}
#endif

	inter_init_sql(INTER_CONF_NAME); // inter server configuration

	char_mmo_sql_init();
	char_read_fame_list(); //Read fame lists.

	if ((naddr_ != 0) && (!(charserv_config.login_ip) || !(charserv_config.char_ip) ))
	{
		char ip_str[16];
		ip2str(addr_[0], ip_str);

		if (naddr_ > 1)
			ShowStatus("Multiple interfaces detected..  using %s as our IP address\n", ip_str);
		else
			ShowStatus("Defaulting to %s as our IP address\n", ip_str);
		if (!(charserv_config.login_ip) ) {
			safestrncpy(charserv_config.login_ip_str, ip_str, sizeof(charserv_config.login_ip_str));
			charserv_config.login_ip = str2ip(charserv_config.login_ip_str);
		}
		if (!(charserv_config.char_ip)) {
			safestrncpy(charserv_config.char_ip_str, ip_str, sizeof(charserv_config.char_ip_str));
			charserv_config.char_ip = str2ip(charserv_config.char_ip_str);
		}
	}

	do_init_chlogif();
	do_init_chmapif();

	// periodically update the overall user count on all mapservers + login server
	add_timer_func_list(chlogif_broadcast_user_count, "broadcast_user_count");
	add_timer_interval(gettick() + 1000, chlogif_broadcast_user_count, 0, 0, 5 * 1000);

	// Timer to clear (online_char_db)
	add_timer_func_list(char_chardb_waiting_disconnect, "chardb_waiting_disconnect");

	// Online Data timers (checking if char still connected)
	add_timer_func_list(char_online_data_cleanup, "online_data_cleanup");
	add_timer_interval(gettick() + 1000, char_online_data_cleanup, 0, 0, 600 * 1000);

	// periodically remove players that have not logged in for a long time from clans
	add_timer_func_list(char_clan_member_cleanup, "clan_member_cleanup");
	add_timer_interval(gettick() + 1000, char_clan_member_cleanup, 0, 0, 60 * 60 * 1000); // every 60 minutes

	// periodically check if mails need to be returned to their sender
	add_timer_func_list(mail_return_timer, "mail_return_timer");
	add_timer_interval(gettick() + 1000, mail_return_timer, 0, 0, 1 * 60 * 1000); // every minute

	// periodically check if mails need to be deleted completely
	add_timer_func_list(mail_delete_timer, "mail_delete_timer");
	add_timer_interval(gettick() + 1000, mail_delete_timer, 0, 0, 1 * 60 * 1000); // every minute

	//check db tables
	if(charserv_config.char_check_db && char_checkdb() == 0){
		ShowFatalError("char : A tables is missing in sql-server, please fix it, see (sql-files main.sql for structure) \n");
		return false;
	}
	//Cleaning the tables for nullptr entrys @ startup [Sirius]
	//Chardb clean
	if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `account_id` = '0'", schema_config.char_db) )
		Sql_ShowDebug(sql_handle);

	//guilddb clean
	if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `guild_lv` = '0' AND `max_member` = '0' AND `exp` = '0' AND `next_exp` = '0' AND `average_lv` = '0'", schema_config.guild_db) )
		Sql_ShowDebug(sql_handle);

	//guildmemberdb clean
	if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `guild_id` = '0' AND `char_id` = '0'", schema_config.guild_member_db) )
		Sql_ShowDebug(sql_handle);

	set_defaultparse(chclif_parse);

	if( (char_fd = make_listen_bind(charserv_config.bind_ip,charserv_config.char_port)) == -1 ) {
		ShowFatalError("Failed to bind to port '" CL_WHITE "%d" CL_RESET "'\n",charserv_config.char_port);
		return false;
	}

	do_init_chcnslif();

	ShowStatus("The char-server is " CL_GREEN "ready" CL_RESET " (Server is listening on the port %d).\n\n", charserv_config.char_port);

	return true;
}

int main( int argc, char *argv[] ){
	return main_core<CharacterServer>( argc, argv );
}
