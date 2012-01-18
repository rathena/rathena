// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/cbasetypes.h"
#include "../common/malloc.h"
#include "../common/nullpo.h"
#include "../common/showmsg.h"
#include "../common/strlib.h"
#include "../common/mmo.h"
#include "atcommand.h" // msg_txt()
#include "battle.h" // struct battle_config
#include "clif.h"
#include "map.h"
#include "npc.h" // npc_event_do()
#include "pc.h"
#include "chat.h"

#include <stdio.h>
#include <string.h>


int chat_triggerevent(struct chat_data *cd); // forward declaration

/// Initializes a chatroom object (common functionality for both pc and npc chatrooms).
/// Returns a chatroom object on success, or NULL on failure.
static struct chat_data* chat_createchat(struct block_list* bl, const char* title, const char* pass, int limit, bool pub, int trigger, const char* ev, int zeny, int minLvl, int maxLvl)
{
	struct chat_data* cd;
	nullpo_retr(NULL, bl);

	cd = (struct chat_data *) aMalloc(sizeof(struct chat_data));

	safestrncpy(cd->title, title, sizeof(cd->title));
	safestrncpy(cd->pass, pass, sizeof(cd->pass));
	cd->pub = pub;
	cd->users = 0;
	cd->limit = min(limit, ARRAYLENGTH(cd->usersd));
	cd->trigger = trigger;
	cd->zeny = zeny;
	cd->minLvl = minLvl;
	cd->maxLvl = maxLvl;
	memset(cd->usersd, 0, sizeof(cd->usersd));
	cd->owner = bl;
	safestrncpy(cd->npc_event, ev, sizeof(cd->npc_event));

	cd->bl.id   = map_get_new_object_id();
	cd->bl.m    = bl->m;
	cd->bl.x    = bl->x;
	cd->bl.y    = bl->y;
	cd->bl.type = BL_CHAT;
	cd->bl.next = cd->bl.prev = NULL;

	if( cd->bl.id == 0 )
	{
		aFree(cd);
		cd = NULL;
	}

	map_addiddb(&cd->bl);

	if( bl->type != BL_NPC )
		cd->kick_list = idb_alloc(DB_OPT_BASE);
	
	return cd;
}

/*==========================================
 * player chatroom creation
 *------------------------------------------*/
int chat_createpcchat(struct map_session_data* sd, const char* title, const char* pass, int limit, bool pub)
{
	struct chat_data* cd;
	nullpo_ret(sd);

	if( sd->chatID )
		return 0; //Prevent people abusing the chat system by creating multiple chats, as pointed out by End of Exam. [Skotlex]

	if( sd->state.vending || sd->state.buyingstore )
	{// not chat, when you already have a store open
		return 0;
	}

	if( map[sd->bl.m].flag.nochat )
	{
		clif_displaymessage(sd->fd, msg_txt(281));
		return 0; //Can't create chatrooms on this map.
	}

	if( map_getcell(sd->bl.m,sd->bl.x,sd->bl.y,CELL_CHKNOCHAT) )
	{
		clif_displaymessage (sd->fd, "Can't create chat rooms in this Area.");
		return 0;
	}

	pc_stop_walking(sd,1);

	cd = chat_createchat(&sd->bl, title, pass, limit, pub, 0, "", 0, 1, MAX_LEVEL);
	if( cd )
	{
		cd->users = 1;
		cd->usersd[0] = sd;
		pc_setchatid(sd,cd->bl.id);
		clif_createchat(sd,0);
		clif_dispchat(cd,0);
	}
	else
		clif_createchat(sd,1);

	return 0;
}

/*==========================================
 * join an existing chatroom
 *------------------------------------------*/
int chat_joinchat(struct map_session_data* sd, int chatid, const char* pass)
{
	struct chat_data* cd;

	nullpo_ret(sd);
	cd = (struct chat_data*)map_id2bl(chatid);

	if( cd == NULL || cd->bl.type != BL_CHAT || cd->bl.m != sd->bl.m || sd->state.vending || sd->state.buyingstore || sd->chatID || cd->users >= cd->limit )
	{
		clif_joinchatfail(sd,0);
		return 0;
	}

	if( !cd->pub && strncmp(pass, cd->pass, sizeof(cd->pass)) != 0 && !(battle_config.gm_join_chat && pc_isGM(sd) >= battle_config.gm_join_chat) )
	{
		clif_joinchatfail(sd,1);
		return 0;
	}

	if( sd->status.base_level < cd->minLvl || sd->status.base_level > cd->maxLvl ) {
		if(sd->status.base_level < cd->minLvl)
			clif_joinchatfail(sd,5);
		else
			clif_joinchatfail(sd,6);

		return 0;
	}

	if( sd->status.zeny < cd->zeny ) {
		clif_joinchatfail(sd,4);
		return 0;
	}

	if( cd->owner->type != BL_NPC && idb_exists(cd->kick_list,sd->status.char_id) ) {
		clif_joinchatfail(sd,2);//You have been kicked out of the room.
		return 0;
	}

	pc_stop_walking(sd,1);
	cd->usersd[cd->users] = sd;
	cd->users++;

	pc_setchatid(sd,cd->bl.id);

	clif_joinchatok(sd,cd);	// 新たに参加した人には全員のリスト
	clif_addchat(cd,sd);	// 既に中に居た人には追加した人の報告
	clif_dispchat(cd,0);	// 周囲の人には人数変化報告

	chat_triggerevent(cd); // イベント
	
	return 0;
}


/*==========================================
 * leave a chatroom
 *------------------------------------------*/
int chat_leavechat(struct map_session_data* sd, bool kicked)
{
	struct chat_data* cd;
	int i;
	int leavechar;

	nullpo_retr(1, sd);

	cd = (struct chat_data*)map_id2bl(sd->chatID);
	if( cd == NULL )
	{
		pc_setchatid(sd, 0);
		return 1;
	}

	ARR_FIND( 0, cd->users, i, cd->usersd[i] == sd );
	if ( i == cd->users )
	{	// Not found in the chatroom?
		pc_setchatid(sd, 0);
		return -1;
	}

	clif_leavechat(cd, sd, kicked);
	pc_setchatid(sd, 0);
	cd->users--;

	leavechar = i;

	for( i = leavechar; i < cd->users; i++ )
		cd->usersd[i] = cd->usersd[i+1];


	if( cd->users == 0 && cd->owner->type == BL_PC ) { // Delete empty chatroom
		clif_clearchat(cd, 0);
		db_destroy(cd->kick_list);
		map_deliddb(&cd->bl);
		map_delblock(&cd->bl);
		map_freeblock(&cd->bl);
		return 1;
	}

	if( leavechar == 0 && cd->owner->type == BL_PC )
	{	// Set and announce new owner
		cd->owner = (struct block_list*) cd->usersd[0];
		clif_changechatowner(cd, cd->usersd[0]);
		clif_clearchat(cd, 0);

		//Adjust Chat location after owner has been changed.
		map_delblock( &cd->bl );
		cd->bl.x=cd->usersd[0]->bl.x;
		cd->bl.y=cd->usersd[0]->bl.y;
		map_addblock( &cd->bl );

		clif_dispchat(cd,0);
	}
	else
		clif_dispchat(cd,0); // refresh chatroom

	return 0;
}

/*==========================================
 * change a chatroom's owner
 *------------------------------------------*/
int chat_changechatowner(struct map_session_data* sd, const char* nextownername)
{
	struct chat_data* cd;
	struct map_session_data* tmpsd;
	int i;

	nullpo_retr(1, sd);

	cd = (struct chat_data*)map_id2bl(sd->chatID);
	if( cd == NULL || (struct block_list*) sd != cd->owner )
		return 1;

	ARR_FIND( 1, cd->users, i, strncmp(cd->usersd[i]->status.name, nextownername, NAME_LENGTH) == 0 );
	if( i == cd->users )
		return -1;  // name not found

	// erase temporarily
	clif_clearchat(cd,0);

	// set new owner
	cd->owner = (struct block_list*) cd->usersd[i];
	clif_changechatowner(cd,cd->usersd[i]);

	// swap the old and new owners' positions
	tmpsd = cd->usersd[i];
	cd->usersd[i] = cd->usersd[0];
	cd->usersd[0] = tmpsd;

	// set the new chatroom position
	map_delblock( &cd->bl );
	cd->bl.x = cd->owner->x;
	cd->bl.y = cd->owner->y;
	map_addblock( &cd->bl );

	// and display again
	clif_dispchat(cd,0);

	return 0;
}

/*==========================================
 * change a chatroom's status (title, etc)
 *------------------------------------------*/
int chat_changechatstatus(struct map_session_data* sd, const char* title, const char* pass, int limit, bool pub)
{
	struct chat_data* cd;

	nullpo_retr(1, sd);

	cd = (struct chat_data*)map_id2bl(sd->chatID);
	if( cd==NULL || (struct block_list *)sd != cd->owner )
		return 1;

	safestrncpy(cd->title, title, CHATROOM_TITLE_SIZE);
	safestrncpy(cd->pass, pass, CHATROOM_PASS_SIZE);
	cd->limit = min(limit, ARRAYLENGTH(cd->usersd));
	cd->pub = pub;

	clif_changechatstatus(cd);
	clif_dispchat(cd,0);

	return 0;
}

/*==========================================
 * kick an user from a chatroom
 *------------------------------------------*/
int chat_kickchat(struct map_session_data* sd, const char* kickusername)
{
	struct chat_data* cd;
	int i;

	nullpo_retr(1, sd);

	cd = (struct chat_data *)map_id2bl(sd->chatID);
	
	if( cd==NULL || (struct block_list *)sd != cd->owner )
		return -1;

	ARR_FIND( 0, cd->users, i, strncmp(cd->usersd[i]->status.name, kickusername, NAME_LENGTH) == 0 );
	if( i == cd->users )
		return -1;

	if( battle_config.gm_kick_chat && pc_isGM(cd->usersd[i]) >= battle_config.gm_kick_chat )
		return 0; //gm kick protection [Valaris]
	
	idb_put(cd->kick_list,cd->usersd[i]->status.char_id,(void*)1);

	chat_leavechat(cd->usersd[i],1);
	return 0;
}

/// Creates a chat room for the npc.
int chat_createnpcchat(struct npc_data* nd, const char* title, int limit, bool pub, int trigger, const char* ev, int zeny, int minLvl, int maxLvl)
{
	struct chat_data* cd;
	nullpo_ret(nd);

	if( nd->chat_id ) {
		ShowError("chat_createnpcchat: npc '%s' already has a chatroom, cannot create new one!\n", nd->exname);
		return 0;
	}

	if( zeny > MAX_ZENY || maxLvl > MAX_LEVEL ) {
		ShowError("chat_createnpcchat: npc '%s' has a required lvl or amount of zeny over the max limit!\n", nd->exname);
		return 0;
	}

	cd = chat_createchat(&nd->bl, title, "", limit, pub, trigger, ev, zeny, minLvl, maxLvl);

	if( cd ) {
		nd->chat_id = cd->bl.id;
		clif_dispchat(cd,0);
	}

	return 0;
}

/// Removes the chatroom from the npc.
int chat_deletenpcchat(struct npc_data* nd)
{
	struct chat_data *cd;
	nullpo_ret(nd);

	cd = (struct chat_data*)map_id2bl(nd->chat_id);
	if( cd == NULL )
		return 0;
	
	chat_npckickall(cd);
	clif_clearchat(cd, 0);
	map_deliddb(&cd->bl);
	map_delblock(&cd->bl);
	map_freeblock(&cd->bl);
	nd->chat_id = 0;
	
	return 0;
}

/*==========================================
 * 規定人数以上でイベントが定義されてるなら実行
 *------------------------------------------*/
int chat_triggerevent(struct chat_data *cd)
{
	nullpo_ret(cd);

	if( cd->users >= cd->trigger && cd->npc_event[0] )
		npc_event_do(cd->npc_event);
	return 0;
}

/// Enables the event of the chat room.
/// At most, 127 users are needed to trigger the event.
int chat_enableevent(struct chat_data* cd)
{
	nullpo_ret(cd);

	cd->trigger &= 0x7f;
	chat_triggerevent(cd);
	return 0;
}

/// Disables the event of the chat room
int chat_disableevent(struct chat_data* cd)
{
	nullpo_ret(cd);

	cd->trigger |= 0x80;
	return 0;
}

/// Kicks all the users from the chat room.
int chat_npckickall(struct chat_data* cd)
{
	nullpo_ret(cd);

	while( cd->users > 0 )
		chat_leavechat(cd->usersd[cd->users-1],0);

	return 0;
}
