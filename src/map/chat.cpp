// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "chat.hpp"

#include <cstring>

#include <common/cbasetypes.hpp>
#include <common/malloc.hpp>
#include <common/mmo.hpp>
#include <common/nullpo.hpp>
#include <common/showmsg.hpp>
#include <common/strlib.hpp>

#include "achievement.hpp"
#include "atcommand.hpp" // msg_txt()
#include "battle.hpp" // struct battle_config
#include "clif.hpp"
#include "map.hpp"
#include "npc.hpp" // npc_event_do()
#include "pc.hpp"
#include "pc_groups.hpp"

namespace chats{

/// Initializes a chatroom object (common functionality for both pc and npc chatrooms).
/// Returns a chatroom object on success, or nullptr on failure.
ChatData* ChatData::CreateChat(
		block_list* bl,
		const char* title,
		const char* pass,
		int limit,
		bool pub,
		int trigger,
		const char* ev,
		int zeny,
		int min_level,
		int max_level) {

	nullpo_retr(nullptr, bl);

	ChatData* cd = (ChatData *) aMalloc(sizeof(ChatData));

	safestrncpy(cd->title, title, sizeof(cd->title));
	safestrncpy(cd->pass, pass, sizeof(cd->pass));
	cd->pub = pub;
	cd->users = 0;
	cd->limit = min(limit, cd->usersd.size());
	cd->trigger = trigger;
	cd->zeny = zeny;
	cd->min_level = min_level;
	cd->max_level = max_level;
	cd->usersd.fill(nullptr);
	cd->owner = bl;
	safestrncpy(cd->npc_event, ev, sizeof(cd->npc_event));

	cd->bl.id   = map_get_new_object_id();
	cd->bl.m    = bl->m;
	cd->bl.x    = bl->x;
	cd->bl.y    = bl->y;
	cd->bl.type = BL_CHAT;
	cd->bl.next = cd->bl.prev = nullptr;

	if( cd->bl.id == 0 ) {
		aFree(cd);
		cd = nullptr;
	}

	map_addiddb(&cd->bl);

	if( bl->type != BL_NPC )
		cd->kick_list = idb_alloc(DB_OPT_BASE);

	return cd;
}

/**
 * Player chat room creation.
 * @param sd : player requesting
 * @param title : title of chat room
 * @param pass : password for chat room
 * @param limit : amount allowed to enter
 * @param pub : public or private
 */
void CreatePcChat(map_session_data* sd, const char* title, const char* pass, int limit, bool pub){

	nullpo_retv(sd);

	if( sd->chatID )
		return; //Prevent people abusing the chat system by creating multiple chats, as pointed out by End of Exam. [Skotlex]

	if( sd->state.vending || sd->state.buyingstore ) // not chat, when you already have a store open
		return;

	if( map_getmapflag(sd->bl.m, MF_NOCHAT) ) {
		clif_displaymessage(sd->fd, msg_txt(sd,281));
		return; //Can't create chatrooms on this map.
	}

	if( map_getcell(sd->bl.m,sd->bl.x,sd->bl.y,CELL_CHKNOCHAT) ) {
		clif_displaymessage (sd->fd, msg_txt(sd,665));
		return;
	}
	pc_stop_walking(sd,1);
	ChatData* cd = ChatData::CreateChat(&sd->bl, title, pass, limit, pub, 0, "", 0, 1, MAX_LEVEL);

	if( cd ) {
		cd->users = 1;
		cd->usersd[0] = sd;
		pc_setchatid(sd,cd->bl.id);
		pc_stop_attack(sd);
		clif_createchat(sd,0);
		clif_dispchat(cd,0);

		if (status_isdead(&sd->bl))
			achievement_update_objective(sd, AG_CHATTING_DYING, 1, 1);
		else
			achievement_update_objective(sd, AG_CHATTING_CREATE, 1, 1);
	} else
		clif_createchat(sd,1);

	return;
}

 /**
 * Join an existing chat room.
 * @param sd : player requesting
 * @param chatid : ID of the chat room
 * @param pass : password of chat room
 */
void JoinChat(map_session_data* sd, int chatid, const char* pass){
	nullpo_retv(sd);

	ChatData* cd = (ChatData*)map_id2bl(chatid);

	if( cd == nullptr || cd->bl.type != BL_CHAT || cd->bl.m != sd->bl.m || sd->state.vending || sd->state.buyingstore || sd->chatID || ((cd->owner->type == BL_NPC) ? cd->users+1 : cd->users) >= cd->limit ) {
		clif_joinchatfail(sd,0);
		return;
	}

	if( !cd->pub && strncmp(pass, cd->pass, sizeof(cd->pass)) != 0 && !pc_has_permission(sd, PC_PERM_JOIN_ALL_CHAT) ) {
		clif_joinchatfail(sd,1);
		return;
	}

	if( sd->status.base_level < cd->min_level || sd->status.base_level > cd->max_level ) {
		if(sd->status.base_level < cd->min_level)
			clif_joinchatfail(sd,5);
		else
			clif_joinchatfail(sd,6);

		return;
	}

	if( sd->status.zeny < cd->zeny ) {
		clif_joinchatfail(sd,4);
		return;
	}

	if( cd->owner->type != BL_NPC && idb_exists(cd->kick_list,sd->status.char_id) ) {
		clif_joinchatfail(sd,2);//You have been kicked out of the room.
		return;
	}

	pc_stop_walking(sd,1);
	cd->usersd[cd->users] = sd;
	cd->users++;

	pc_setchatid(sd,cd->bl.id);

	clif_joinchatok(sd, cd); //To the person who newly joined the list of all
	clif_addchat(cd, sd); //Reports To the person who already in the chat
	clif_dispchat(cd, 0); //Reported number of changes to the people around

	if (cd->owner->type == BL_PC)
		achievement_update_objective(map_id2sd(cd->owner->id), AG_CHATTING_COUNT, 1, cd->users);

	cd->TriggerEvent(); //Event

	return;
}

/**
 * Make player (sd) leave a chat room.
 * @param sd : player requesting
 * @param kicked : for clif notification, kicked=1 or regular leave
 */
void LeaveChat(map_session_data* sd, bool kicked){
	nullpo_retv(sd);
	ChatData* cd = (ChatData*)map_id2bl(sd->chatID);
	if( cd == nullptr ) {
		pc_setchatid(sd, 0);
		return;
	}

	auto i = std::find(cd->usersd.begin(), cd->usersd.end(), sd) - cd->usersd.begin();
	if (i == cd->users){ // Not found in the chatroom?
		pc_setchatid(sd, 0);
		return;
	}

	clif_leavechat(cd, sd, kicked);
	pc_setchatid(sd, 0);
	cd->users--;

	int leavechar = i;

	for( i = leavechar; i <= cd->users && i < cd->usersd.size() - 1; i++ )
		cd->usersd[i] = cd->usersd[i+1];

	if (cd->users == 0 && cd->owner->type == BL_PC) { // Delete empty chatroom
		clif_clearchat(cd, 0);
		db_destroy(cd->kick_list);
		map_deliddb(&cd->bl);
		map_delblock(&cd->bl);
		map_freeblock(&cd->bl);

		skill_unit* unit = map_find_skill_unit_oncell(&sd->bl, sd->bl.x, sd->bl.y, AL_WARP, nullptr, 0);

		if (unit != nullptr && unit->group != nullptr)
			ext_skill_unit_onplace(unit, &sd->bl, unit->group->tick);
		return;
	}

	if ( leavechar == 0 && cd->owner->type == BL_PC) { // Set and announce new owner
		cd->owner = (struct block_list*)cd->usersd[0];
		clif_changechatowner(cd, cd->usersd[0]);
		clif_clearchat(cd, 0);

		//Adjust Chat location after owner has been changed.
		map_delblock(&cd->bl);
		cd->bl.x = cd->usersd[0]->bl.x;
		cd->bl.y = cd->usersd[0]->bl.y;
		if (map_addblock( &cd->bl ))
			return;
	}
	clif_dispchat(cd, 0); // refresh chatroom
}

/**
 * Change a chat room's owner.
 * @param sd : player requesting
 * @param nextownername : string of new owner (name should be in chatroom)
 */
void ChangeChatOwner(const map_session_data* sd, const char* nextownername){
	nullpo_retv(sd);

	ChatData* cd = (ChatData*)map_id2bl(sd->chatID);

	if( cd == nullptr || (block_list*) sd != cd->owner )
		return;

	auto has_name = [&nextownername](auto* usd){return strncmp(usd->status.name, nextownername, NAME_LENGTH) == 0;};
	auto i = std::find_if(cd->usersd.begin() + 1, cd->usersd.end(), has_name) - cd->usersd.begin();
	if( i == cd->users )
		return;  // name not found

	// erase temporarily
	clif_clearchat(cd,0);

	// set new owner
	cd->owner = (struct block_list*) cd->usersd[i];
	clif_changechatowner(cd,cd->usersd[i]);

	// swap the old and new owners' positions
	map_session_data *tmpsd = cd->usersd[i];
	cd->usersd[i] = cd->usersd[0];
	cd->usersd[0] = tmpsd;

	// set the new chatroom position
	map_delblock( &cd->bl );
	cd->bl.x = cd->owner->x;
	cd->bl.y = cd->owner->y;

	if(map_addblock( &cd->bl ))
		return;

	// and display again
	clif_dispchat(cd,0);

	return;
}

/**
 * Change a chat room's status (title, etc).
 * @param sd : player requesting
 * @param title : new title
 * @param pass : new password
 * @param limit : new limit
 * @param pub : public or private
 */
void ChangeChatStatus(const map_session_data* sd, const char* title, const char* pass, int limit, bool pub)
{
	nullpo_retv(sd);

	ChatData* cd = (ChatData*)map_id2bl(sd->chatID);

	if( cd == nullptr || (block_list*)sd != cd->owner )
		return;

	safestrncpy(cd->title, title, CHATROOM_TITLE_SIZE);
	safestrncpy(cd->pass, pass, CHATROOM_PASS_SIZE);
	cd->limit = min(limit, ARRAYLENGTH(cd->usersd));
	cd->pub = pub;

	clif_changechatstatus(cd);
	clif_dispchat(cd,0);

}

/**
 * Kicks a user from the chat room.
 * @param cd : chat to be kicked from
 * @param kick_user_name : player name to be kicked
 */
void ChatData::NpcKickChat(const char* kick_user_name){
	auto has_name = [&](map_session_data *sd){return strncmp(sd->status.name, kick_user_name, NAME_LENGTH) == 0;};
	if(auto it = std::find_if(usersd.cbegin(),usersd.cbegin() + users, has_name); it != usersd.cbegin() + users)
		LeaveChat(*it, 1);
}

/**
 * Kick a member from a chat room.
 * @param sd : player requesting
 * @param kick_user_name : player name to be kicked
 */
void KickPcFromChat(map_session_data* sd, const char* kickusername){
	nullpo_retv(sd);

	ChatData* cd = (ChatData *)map_id2bl(sd->chatID);

	if( cd == nullptr || (block_list*)sd != cd->owner)
		return;

	auto has_name = [&kickusername](auto* usd){return strncmp(usd->status.name, kickusername, NAME_LENGTH) == 0;};
	auto i = std::find_if(cd->usersd.begin(), cd->usersd.end(), has_name) - cd->usersd.begin();
	if( i == cd->users )
		return;

	if (pc_has_permission(cd->usersd[i], PC_PERM_NO_CHAT_KICK))
		return; //gm kick protection [Valaris]

	idb_put(cd->kick_list,cd->usersd[i]->status.char_id,(void*)1);

	LeaveChat(cd->usersd[i],1);

}

/**
 * Creates a chat room for a NPC.
 * @param nd : NPC requesting
 * @param title : title of chat room
 * @param limit : limit of users in chat room
 * @param pub : public or private
 * @param trigger : event trigger
 * @param ev : event name
 * @param zeny : zeny cost
 * @param min_level : minimum level to enter
 * @param max_level : maximum level to enter
 */
void CreateNpcChat(
		npc_data* nd,
		const char* title,
		int limit,
		bool pub,
		int trigger,
		const char* ev,
		int zeny,
		int minLvl,
		int maxLvl)
{
	nullpo_retv(nd);

	if( nd->chat_id ) {
		ShowError("chats::CreateNpcChat: npc '%s' already has a chatroom, cannot create new one!\n", nd->exname);
	}

	if( zeny > MAX_ZENY || maxLvl > MAX_LEVEL ) {
		ShowError("chats::CreateNpcChat: npc '%s' has a required lvl or amount of zeny over the max limit!\n", nd->exname);
	}


	if(ChatData* cd = ChatData::CreateChat(&nd->bl, title, "", limit, pub, trigger, ev, zeny, minLvl, maxLvl); cd != nullptr) {
		nd->chat_id = cd->bl.id;
		clif_dispchat(cd,0);
	}
}

/**
 * Removes a chat room for a NPC.
 * @param nd : NPC requesting
 */
void DeleteNpcChat(npc_data* nd){
	nullpo_retv(nd);

	ChatData *cd = (ChatData*)map_id2bl(nd->chat_id);

	if(cd == nullptr)
		return;

	cd->NpcKickAll();
	clif_clearchat(cd, 0);
	map_deliddb(&cd->bl);
	map_delblock(&cd->bl);
	map_freeblock(&cd->bl);
	nd->chat_id = 0;
}

 /**
 * Trigger NPC event when entering the chat room.
 * @param cd : chat room to trigger event
 * @return 0
 */
void ChatData::TriggerEvent()
{
	if( this->users >= this->trigger && this->npc_event[0] )
		npc_event_do(this->npc_event);
}

 /**
 * Enables the event of the chat room.
 * At most, 127 users are needed to trigger the event.
 * @param cd : chat room to trigger event
 */
void ChatData::EnableEvent(){
	this->trigger &= 0x7f;
	this->TriggerEvent();
}

/**
 * Disables the event of the chat room.
 * @param cd : chat room to trigger event
 */
void ChatData::DisableEvent(){

	this->trigger |= 0x80;
}

/**
 * Kicks all the users from the chat room.
 * @param cd : chat room to trigger event
 */
void ChatData::NpcKickAll(){
	while( this->users > 0 )
		LeaveChat(this->usersd[this->users-1],0);
}

}
