// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/showmsg.h"
#include "../common/socket.h"
#include "../common/nullpo.h"
#include "../common/malloc.h"
#include "../common/strlib.h"
#include "../common/mmo.h"
#include "map.h"
#include "battle.h"
#include "chrif.h"
#include "clif.h"
#include "pc.h"
#include "intif.h"
#include "storage.h"
#include "party.h"
#include "pet.h"
#include "mercenary.h"
#include "homunculus.h"
#include "elemental.h"
#include "mail.h"
#include "quest.h"

#include <stdlib.h>

static const int packet_len_table[]={
	-1,-1,27,-1, -1, 0,37,-1, 10+NAME_LENGTH,-1, 0, 0,  0, 0,  0, 0, //0x3800-0x380f
	 0, 0, 0, 0,  0, 0, 0, 0, -1,11, 0, 0,  0, 0,  0, 0, //0x3810
	39,-1,15,15, 14,19, 7,-1,  0, 0, 0, 0,  0, 0,  0, 0, //0x3820
	10,-1,15, 0, 79,19, 7,-1,  0,-1,-1,-1, 14,67,186,-1, //0x3830
	-1, 0, 0,14,  0, 0, 0, 0, -1,74,-1,11, 11,-1,  0, 0, //0x3840
	-1,-1, 7, 7,  7,11, 8,-1,  0, 0, 0, 0,  0, 0,  0, 0, //0x3850  Auctions [Zephyrus] itembound[Akinari]
	-1, 7, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0,  0, 0, //0x3860  Quests [Kevin] [Inkfish]
	-1, 3, 3, 0,  0, 0, 0, 0,  0, 0, 0, 0, -1, 3,  3, 0, //0x3870  Mercenaries [Zephyrus] / Elemental [pakpil]
	12,-1, 7, 3,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0,  0, 0, //0x3880
	-1,-1, 7, 3,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0,  0, 0, //0x3890  Homunculus [albator]
};

extern int char_fd; // inter server Fd used for char_fd
#define inter_fd char_fd	// alias

//-----------------------------------------------------------------
// Send to inter server

/**
 * Verify the char-serv is up and running
 * @return 0=no, 1=ok
 */
int CheckForCharServer(void)
{
	return ((char_fd <= 0) || session[char_fd] == NULL || session[char_fd]->wdata == NULL);
}

/**
 * Request the char-serv to create a pet (to register it actually)
 * @param account_id
 * @param char_id
 * @param pet_class
 * @param pet_lv
 * @param pet_egg_id
 * @param pet_equip
 * @param intimate
 * @param hungry
 * @param rename_flag
 * @param incubate
 * @param pet_name
 * @return 
 */
int intif_create_pet(uint32 account_id,uint32 char_id,short pet_class,short pet_lv,short pet_egg_id,
	short pet_equip,short intimate,short hungry,char rename_flag,char incubate,char *pet_name)
{
	if (CheckForCharServer())
		return 0;
	WFIFOHEAD(inter_fd, 24 + NAME_LENGTH);
	WFIFOW(inter_fd,0) = 0x3080;
	WFIFOL(inter_fd,2) = account_id;
	WFIFOL(inter_fd,6) = char_id;
	WFIFOW(inter_fd,10) = pet_class;
	WFIFOW(inter_fd,12) = pet_lv;
	WFIFOW(inter_fd,14) = pet_egg_id;
	WFIFOW(inter_fd,16) = pet_equip;
	WFIFOW(inter_fd,18) = intimate;
	WFIFOW(inter_fd,20) = hungry;
	WFIFOB(inter_fd,22) = rename_flag;
	WFIFOB(inter_fd,23) = incubate;
	memcpy(WFIFOP(inter_fd,24),pet_name,NAME_LENGTH);
	WFIFOSET(inter_fd,24+NAME_LENGTH);

	return 1;
}

/**
 * Request char-serv to load a pet from persistence (SQL)
 * @param account_id
 * @param char_id
 * @param pet_id
 * @return 
 */
int intif_request_petdata(uint32 account_id,uint32 char_id,int pet_id)
{
	if (CheckForCharServer())
		return 0;
	WFIFOHEAD(inter_fd, 14);
	WFIFOW(inter_fd,0) = 0x3081;
	WFIFOL(inter_fd,2) = account_id;
	WFIFOL(inter_fd,6) = char_id;
	WFIFOL(inter_fd,10) = pet_id;
	WFIFOSET(inter_fd,14);

	return 1;
}

/**
 * Request char-serv to save a pet in persistence (SQL)
 * @param account_id
 * @param p
 * @return 
 */
int intif_save_petdata(uint32 account_id,struct s_pet *p)
{
	if (CheckForCharServer())
		return 0;
	WFIFOHEAD(inter_fd, sizeof(struct s_pet) + 8);
	WFIFOW(inter_fd,0) = 0x3082;
	WFIFOW(inter_fd,2) = sizeof(struct s_pet) + 8;
	WFIFOL(inter_fd,4) = account_id;
	memcpy(WFIFOP(inter_fd,8),p,sizeof(struct s_pet));
	WFIFOSET(inter_fd,WFIFOW(inter_fd,2));

	return 1;
}

/**
 * Request char-serv to delete the entry for this pet-char association
 * @param pet_id
 * @return 
 */
int intif_delete_petdata(int pet_id)
{
	if (CheckForCharServer())
		return 0;
	WFIFOHEAD(inter_fd,6);
	WFIFOW(inter_fd,0) = 0x3083;
	WFIFOL(inter_fd,2) = pet_id;
	WFIFOSET(inter_fd,6);

	return 1;
}

/**
 * Ask char-serv to rename a CHAR (PC|PET|HOM)
 * @param sd
 * @param type
 * @param name
 * @return 
 */
int intif_rename(struct map_session_data *sd, int type, char *name)
{
	if (CheckForCharServer())
		return 0;

	WFIFOHEAD(inter_fd,NAME_LENGTH+12);
	WFIFOW(inter_fd,0) = 0x3006;
	WFIFOL(inter_fd,2) = sd->status.account_id;
	WFIFOL(inter_fd,6) = sd->status.char_id;
	WFIFOB(inter_fd,10) = type;  //Type: 0 - PC, 1 - PET, 2 - HOM
	memcpy(WFIFOP(inter_fd,11),name, NAME_LENGTH);
	WFIFOSET(inter_fd,NAME_LENGTH+12);
	return 1;
}

/**
 * Request to broadcast a message via char-serv to reach all map-serv connected
 * @param mes : Message to send
 * @param len ; Size of the message
 * @param type : Color of msg
 * @return 0=error occured, 1=msg sent
 */
int intif_broadcast(const char* mes, int len, int type)
{
	int lp = (type|BC_COLOR_MASK) ? 4 : 0;

	// Send to the local players
	clif_broadcast(NULL, mes, len, type, ALL_CLIENT);

	if (CheckForCharServer())
		return 0;

	if (other_mapserver_count < 1)
		return 0; //No need to send.

	WFIFOHEAD(inter_fd, 16 + lp + len);
	WFIFOW(inter_fd,0)  = 0x3000;
	WFIFOW(inter_fd,2)  = 16 + lp + len;
	WFIFOL(inter_fd,4)  = 0xFF000000; // 0xFF000000 color signals standard broadcast
	WFIFOW(inter_fd,8)  = 0; // fontType not used with standard broadcast
	WFIFOW(inter_fd,10) = 0; // fontSize not used with standard broadcast
	WFIFOW(inter_fd,12) = 0; // fontAlign not used with standard broadcast
	WFIFOW(inter_fd,14) = 0; // fontY not used with standard broadcast
	if (type|BC_BLUE)
		WFIFOL(inter_fd,16) = 0x65756c62; //If there's "blue" at the beginning of the message, game client will display it in blue instead of yellow.
	else if (type|BC_WOE)
		WFIFOL(inter_fd,16) = 0x73737373; //If there's "ssss", game client will recognize message as 'WoE broadcast'.
	memcpy(WFIFOP(inter_fd,16 + lp), mes, len);
	WFIFOSET(inter_fd, WFIFOW(inter_fd,2));
	return 1;
}

/**
 * Request to char-serv to broadcast a message to all map-serv
 * @param mes : Message to brodcast
 * @param len : Size of message
 * @param fontColor : color to display message
 * @param fontType : 
 * @param fontSize :
 * @param fontAlign :
 * @param fontY :
 * @return 0=not send to char-serv, 1=send to char-serv
 */
int intif_broadcast2(const char* mes, int len, unsigned long fontColor, short fontType, short fontSize, short fontAlign, short fontY)
{
	// Send to the local players
	clif_broadcast2(NULL, mes, len, fontColor, fontType, fontSize, fontAlign, fontY, ALL_CLIENT);

	if (CheckForCharServer())
		return 0;

	if (other_mapserver_count < 1)
		return 0; //No need to send.

	WFIFOHEAD(inter_fd, 16 + len);
	WFIFOW(inter_fd,0)  = 0x3000;
	WFIFOW(inter_fd,2)  = 16 + len;
	WFIFOL(inter_fd,4)  = fontColor;
	WFIFOW(inter_fd,8)  = fontType;
	WFIFOW(inter_fd,10) = fontSize;
	WFIFOW(inter_fd,12) = fontAlign;
	WFIFOW(inter_fd,14) = fontY;
	memcpy(WFIFOP(inter_fd,16), mes, len);
	WFIFOSET(inter_fd, WFIFOW(inter_fd,2));
	return 1;
}

/**
 * send a message using the main chat system
 * @param sd : Player source of message
 * @param message : the message to sent
 * @return 
 */
int intif_main_message(struct map_session_data* sd, const char* message)
{
	char output[256];

	nullpo_ret(sd);

	// format the message for main broadcasting
	snprintf( output, sizeof(output), msg_txt(sd,386), sd->status.name, message );

	// send the message using the inter-server broadcast service
	intif_broadcast2( output, strlen(output) + 1, 0xFE000000, 0, 0, 0, 0 );

	// log the chat message
	log_chat( LOG_CHAT_MAINCHAT, 0, sd->status.char_id, sd->status.account_id, mapindex_id2name(sd->mapindex), sd->bl.x, sd->bl.y, NULL, message );

	return 1;
}

/**
 * Request char-serv to transmit a whisper message. (Private message from one player to another)
 * (player not found on this server)
 * @param sd : Player ending message
 * @param nick : Name of receiver
 * @param mes : Message to send
 * @param mes_len : Size of message
 * @return 0=Message not send, 1=Message send
 */
int intif_wis_message(struct map_session_data *sd, char *nick, char *mes, int mes_len)
{
	nullpo_ret(sd);
	if (CheckForCharServer())
		return 0;

	if (other_mapserver_count < 1)
	{	//Character not found.
		clif_wis_end(sd->fd, 1);
		return 0;
	}

	WFIFOHEAD(inter_fd,mes_len + 52);
	WFIFOW(inter_fd,0) = 0x3001;
	WFIFOW(inter_fd,2) = mes_len + 52;
	memcpy(WFIFOP(inter_fd,4), sd->status.name, NAME_LENGTH);
	memcpy(WFIFOP(inter_fd,4+NAME_LENGTH), nick, NAME_LENGTH);
	memcpy(WFIFOP(inter_fd,4+2*NAME_LENGTH), mes, mes_len);
	WFIFOSET(inter_fd, WFIFOW(inter_fd,2));

	if (battle_config.etc_log)
		ShowInfo("intif_wis_message from %s to %s (message: '%s')\n", sd->status.name, nick, mes);

	return 1;
}

/**
 * Inform the char-serv of the result of the whisper
 * @param id : Character ID
 * @param flag : 0: success to send wisper, 1: target character is not loged in?, 2: ignored by target
 * @return 0=no char-serv connected, 1=msg sent
 */
int intif_wis_replay(int id, int flag)
{
	if (CheckForCharServer())
		return 0;
	WFIFOHEAD(inter_fd,7);
	WFIFOW(inter_fd,0) = 0x3002;
	WFIFOL(inter_fd,2) = id;
	WFIFOB(inter_fd,6) = flag; // flag: 0: success to send wisper, 1: target character is not loged in?, 2: ignored by target
	WFIFOSET(inter_fd,7);

	if (battle_config.etc_log)
		ShowInfo("intif_wis_replay: id: %d, flag:%d\n", id, flag);

	return 1;
}

/**
 * Whisper message from player to all GM from map-server to inter-server (@request)
 * @param wisp_name
 * @param permission
 * @param mes
 * @return 0:no char-serv connected, 1:transfered
 */
int intif_wis_message_to_gm(char *wisp_name, int permission, char *mes)
{
	int mes_len;
	if (CheckForCharServer())
		return 0;
	mes_len = strlen(mes) + 1; // + null
	WFIFOHEAD(inter_fd, mes_len + 8 + NAME_LENGTH);
	WFIFOW(inter_fd,0) = 0x3003;
	WFIFOW(inter_fd,2) = mes_len + 32;
	memcpy(WFIFOP(inter_fd,4), wisp_name, NAME_LENGTH);
	WFIFOL(inter_fd,4+NAME_LENGTH) = permission;
	memcpy(WFIFOP(inter_fd,8+NAME_LENGTH), mes, mes_len);
	WFIFOSET(inter_fd, WFIFOW(inter_fd,2));

	if (battle_config.etc_log)
		ShowNotice("intif_wis_message_to_gm: from: '%s', required permission: %d, message: '%s'.\n", wisp_name, permission, mes);

	return 1;
}

/**
 * Request for saving registry values.
 * @param sd : Player to save registry
 * @return 1=msg sent, -1=error
 */
int intif_saveregistry(struct map_session_data *sd)
{
	DBIterator *iter;
	DBKey key;
	DBData *data;
	int plen = 0;
	size_t len;

	if (CheckForCharServer() || !sd->regs.vars)
		return -1;

	WFIFOHEAD(inter_fd, 60000 + 300);
	WFIFOW(inter_fd,0)  = 0x3004;
	// 0x2 = length (set later)
	WFIFOL(inter_fd,4)  = sd->status.account_id;
	WFIFOL(inter_fd,8)  = sd->status.char_id;
	WFIFOW(inter_fd,12) = 0; // count

	plen = 14;

	iter = db_iterator(sd->regs.vars);
	for( data = iter->first(iter,&key); iter->exists(iter); data = iter->next(iter,&key) ) {
		const char *varname = NULL;
		struct script_reg_state *src = NULL;

		if( data->type != DB_DATA_PTR ) // it's a @number
			continue;

		varname = get_str(script_getvarid(key.i64));

		if( varname[0] == '@' ) // @string$ can get here, so we skip
			continue;

		src = (struct script_reg_state *)db_data2ptr(data);

		if( !src->update )
			continue;

		src->update = false;

		len = strlen(varname)+1;

		WFIFOB(inter_fd, plen) = (unsigned char)len; // won't be higher; the column size is 32
		plen += 1;

		safestrncpy((char*)WFIFOP(inter_fd,plen), varname, len);
		plen += len;

		WFIFOL(inter_fd, plen) = script_getvaridx(key.i64);
		plen += 4;

		if( src->type ) {
			struct script_reg_str *p = (struct script_reg_str *)src;

			WFIFOB(inter_fd, plen) = p->value ? 2 : 3;
			plen += 1;

			if( p->value ) {
				len = strlen(p->value)+1;

				WFIFOB(inter_fd, plen) = (unsigned char)len; // won't be higher; the column size is 254
				plen += 1;

				safestrncpy((char*)WFIFOP(inter_fd,plen), p->value, len);
				plen += len;
			} else {
				script_reg_destroy_single(sd,key.i64,&p->flag);
			}

		} else {
			struct script_reg_num *p = (struct script_reg_num *)src;

			WFIFOB(inter_fd, plen) =  p->value ? 0 : 1;
			plen += 1;

			if( p->value ) {
				WFIFOL(inter_fd, plen) = p->value;
				plen += 4;
			} else {
				script_reg_destroy_single(sd,key.i64,&p->flag);
			}

		}

		WFIFOW(inter_fd,12) += 1;

		if( plen > 60000 ) {
			WFIFOW(inter_fd, 2) = plen;
			WFIFOSET(inter_fd, plen);

			// prepare follow up
			WFIFOHEAD(inter_fd, 60000 + 300);
			WFIFOW(inter_fd,0)  = 0x3004;
			// 0x2 = length (set later)
			WFIFOL(inter_fd,4)  = sd->status.account_id;
			WFIFOL(inter_fd,8)  = sd->status.char_id;
			WFIFOW(inter_fd,12) = 0; // count

			plen = 14;
		}
	}
	dbi_destroy(iter);

	WFIFOW(inter_fd, 2) = plen;
	WFIFOSET(inter_fd, plen);

	sd->vars_dirty = false;

	return 0;
}

/**
 * Request the registries for this player.
 * @param sd : Player to load registry
 * @param flag : Type of registry to load, &1=acc (login-serv), &2=acc (char-serv), &4=char
 * @return 
 */
int intif_request_registry(struct map_session_data *sd, int flag)
{
	nullpo_ret(sd);

	if (CheckForCharServer())
		return 0;

	WFIFOHEAD(inter_fd,6);
	WFIFOW(inter_fd,0) = 0x3005;
	WFIFOL(inter_fd,2) = sd->status.account_id;
	WFIFOL(inter_fd,6) = sd->status.char_id;
	WFIFOB(inter_fd,10) = (flag&1?1:0); //Request Acc Reg 2 (from login-serv))
	WFIFOB(inter_fd,11) = (flag&2?1:0); //Request Acc Reg (from char-serv)
	WFIFOB(inter_fd,12) = (flag&4?1:0); //Request Char Reg
	WFIFOSET(inter_fd,13);

	return 1;
}

/**
 * Request to load guild storage from char-serv
 * @param account_id : Player account identification
 * @param guild_id : Guild of player
 * @return 0:error, 1=msg sent
 */
int intif_request_guild_storage(uint32 account_id,int guild_id)
{
	if (CheckForCharServer())
		return 0;
	WFIFOHEAD(inter_fd,10);
	WFIFOW(inter_fd,0) = 0x3018;
	WFIFOL(inter_fd,2) = account_id;
	WFIFOL(inter_fd,6) = guild_id;
	WFIFOSET(inter_fd,10);
	return 1;
}

/**
 * Request to save guild storage
 * @param account_id : account requesting the save
 * @param gstor : Guild storage struct to save
 * @return 
 */
int intif_send_guild_storage(uint32 account_id,struct guild_storage *gstor)
{
	if (CheckForCharServer())
		return 0;
	WFIFOHEAD(inter_fd,sizeof(struct guild_storage)+12);
	WFIFOW(inter_fd,0) = 0x3019;
	WFIFOW(inter_fd,2) = (unsigned short)sizeof(struct guild_storage)+12;
	WFIFOL(inter_fd,4) = account_id;
	WFIFOL(inter_fd,8) = gstor->guild_id;
	memcpy( WFIFOP(inter_fd,12),gstor, sizeof(struct guild_storage) );
	WFIFOSET(inter_fd,WFIFOW(inter_fd,2));
	return 1;
}

/**
 * Party creation request
 * @param member : Struct of 1 party member
 * @param name : Party name
 * @param item : item pickup rule 
 * @param item2 : item share rule
 * @return 0=error, 1=msg sent
 */
int intif_create_party(struct party_member *member,char *name,int item,int item2)
{
	if (CheckForCharServer())
		return 0;
	nullpo_ret(member);

	WFIFOHEAD(inter_fd,64);
	WFIFOW(inter_fd,0) = 0x3020;
	WFIFOW(inter_fd,2) = 30+sizeof(struct party_member);
	memcpy(WFIFOP(inter_fd,4),name, NAME_LENGTH);
	WFIFOB(inter_fd,28)= item;
	WFIFOB(inter_fd,29)= item2;
	memcpy(WFIFOP(inter_fd,30), member, sizeof(struct party_member));
	WFIFOSET(inter_fd,WFIFOW(inter_fd, 2));
	return 1;
}

/**
 * Party information request
 * @param party_id : Party id to request info
 * @param char_id : Player id requesting
 * @return 0=error, 1=msg sent
 */
int intif_request_partyinfo(int party_id, uint32 char_id)
{
	if (CheckForCharServer())
		return 0;
	WFIFOHEAD(inter_fd,10);
	WFIFOW(inter_fd,0) = 0x3021;
	WFIFOL(inter_fd,2) = party_id;
	WFIFOL(inter_fd,6) = char_id;
	WFIFOSET(inter_fd,10);
	return 1;
}

/**
 * Request to add a member to party
 * @param party_id : Party to add member to
 * @param member : member to add to party
 * @return 
 */
int intif_party_addmember(int party_id,struct party_member *member)
{
	if (CheckForCharServer())
		return 0;
	WFIFOHEAD(inter_fd,42);
	WFIFOW(inter_fd,0)=0x3022;
	WFIFOW(inter_fd,2)=8+sizeof(struct party_member);
	WFIFOL(inter_fd,4)=party_id;
	memcpy(WFIFOP(inter_fd,8),member,sizeof(struct party_member));
	WFIFOSET(inter_fd,WFIFOW(inter_fd, 2));
	return 1;
}

/**
 * Request to change party configuration (exp,item share)
 * @param party_id : Party to alter
 * @param account_id : account requesting change
 * @param exp : sharing exp option
 * @param item :  sharing item option
 * @return 0=error, 1=msg sent
 */
int intif_party_changeoption(int party_id,uint32 account_id,int exp,int item)
{
	if (CheckForCharServer())
		return 0;
	WFIFOHEAD(inter_fd,14);
	WFIFOW(inter_fd,0)=0x3023;
	WFIFOL(inter_fd,2)=party_id;
	WFIFOL(inter_fd,6)=account_id;
	WFIFOW(inter_fd,10)=exp;
	WFIFOW(inter_fd,12)=item;
	WFIFOSET(inter_fd,14);
	return 1;
}

/**
 * Ask the char-serv to make aid,cid quit party
 * @param party_id : Party to leave
 * @param account_id : aid of player to leave
 * @param char_id : cid of player to leave
 * @return 0:char-serv disconected, 1=msg sent
 */
int intif_party_leave(int party_id,uint32 account_id, uint32 char_id)
{
	if (CheckForCharServer())
		return 0;
	WFIFOHEAD(inter_fd,14);
	WFIFOW(inter_fd,0)=0x3024;
	WFIFOL(inter_fd,2)=party_id;
	WFIFOL(inter_fd,6)=account_id;
	WFIFOL(inter_fd,10)=char_id;
	WFIFOSET(inter_fd,14);
	return 1;
}

/**
 * Inform char-serv that a member as quit/lvlup change map, therefore changing party state
 * @param sd : Player that moved
 * @param online : If the player will stay online or no
 * @return 0=error, 1=msg sent
 */
int intif_party_changemap(struct map_session_data *sd,int online)
{
	int16 m, mapindex;

	if (CheckForCharServer())
		return 0;
	if(!sd)
		return 0;

	if( (m=map_mapindex2mapid(sd->mapindex)) >= 0 && map[m].instance_id )
		mapindex = map[map[m].instance_src_map].index;
	else
		mapindex = sd->mapindex;

	WFIFOHEAD(inter_fd,19);
	WFIFOW(inter_fd,0)=0x3025;
	WFIFOL(inter_fd,2)=sd->status.party_id;
	WFIFOL(inter_fd,6)=sd->status.account_id;
	WFIFOL(inter_fd,10)=sd->status.char_id;
	WFIFOW(inter_fd,14)=mapindex;
	WFIFOB(inter_fd,16)=online;
	WFIFOW(inter_fd,17)=sd->status.base_level;
	WFIFOSET(inter_fd,19);
	return 1;
}

/**
 * Request breaking party
 * @param party_id : Party to delete
 * @return 0=error, 1=msg sent
 */
int intif_break_party(int party_id)
{
	if (CheckForCharServer())
		return 0;
	WFIFOHEAD(inter_fd,6);
	WFIFOW(inter_fd,0)=0x3026;
	WFIFOL(inter_fd,2)=party_id;
	WFIFOSET(inter_fd,6);
	return 0;
}

// 
/**
 * Request sending party chat
 * (we using this in case we have multiple map-serv attached 
 * to be sure all party get the message)
 * @param party_id : Party identification
 * @param account_id : Player sending the message
 * @param mes : Message to send
 * @param len : Size of the message
 * @return 0=error, 1=msg sent
 */
int intif_party_message(int party_id,uint32 account_id,const char *mes,int len)
{
	if (CheckForCharServer())
		return 0;

	if (other_mapserver_count < 1)
		return 0; //No need to send.

	WFIFOHEAD(inter_fd,len + 12);
	WFIFOW(inter_fd,0)=0x3027;
	WFIFOW(inter_fd,2)=len+12;
	WFIFOL(inter_fd,4)=party_id;
	WFIFOL(inter_fd,8)=account_id;
	memcpy(WFIFOP(inter_fd,12),mes,len);
	WFIFOSET(inter_fd,len+12);
	return 1;
}

/**
 * Request a new leader for party
 * @param party_id : Party to alter
 * @param account_id : player to set as new leader
 * @param char_id : player to set as new leader
 * @return  0=error, 1=msg sent
 */
int intif_party_leaderchange(int party_id,uint32 account_id,uint32 char_id)
{
	if (CheckForCharServer())
		return 0;
	WFIFOHEAD(inter_fd,14);
	WFIFOW(inter_fd,0)=0x3029;
	WFIFOL(inter_fd,2)=party_id;
	WFIFOL(inter_fd,6)=account_id;
	WFIFOL(inter_fd,10)=char_id;
	WFIFOSET(inter_fd,14);
	return 1;
}

/**
 * Request to update party share level
 * @param share_lvl : Max level number of difference to share exp
 * @return  0=error, 1=msg sent
 */
int intif_party_sharelvlupdate(unsigned int share_lvl)
{
	if (CheckForCharServer())
		return 0;
	WFIFOHEAD(inter_fd,6);
	WFIFOW(inter_fd,0)=0x302A;
	WFIFOL(inter_fd,2)=share_lvl;
	WFIFOSET(inter_fd,6);
	return 1;
}

/**
 * Request a Guild creation
 * @param name : Name of the guild
 * @param master : Guild_member info of master
 * @return 0=error, 1=msg_sent
 */
int intif_guild_create(const char *name,const struct guild_member *master)
{
	if (CheckForCharServer())
		return 0;
	nullpo_ret(master);

	WFIFOHEAD(inter_fd,sizeof(struct guild_member)+(8+NAME_LENGTH));
	WFIFOW(inter_fd,0)=0x3030;
	WFIFOW(inter_fd,2)=sizeof(struct guild_member)+(8+NAME_LENGTH);
	WFIFOL(inter_fd,4)=master->account_id;
	memcpy(WFIFOP(inter_fd,8),name,NAME_LENGTH);
	memcpy(WFIFOP(inter_fd,8+NAME_LENGTH),master,sizeof(struct guild_member));
	WFIFOSET(inter_fd,WFIFOW(inter_fd,2));
	return 1;
}

/**
 * Request Guild information
 * @param guild_id : guild to get info from
 * @return  0=error, 1=msg_sent
 */
int intif_guild_request_info(int guild_id)
{
	if (CheckForCharServer())
		return 0;
	WFIFOHEAD(inter_fd,6);
	WFIFOW(inter_fd,0) = 0x3031;
	WFIFOL(inter_fd,2) = guild_id;
	WFIFOSET(inter_fd,6);
	return 1;
}

/**
 * Request to add member to the guild
 * @param guild_id : Guild to alter
 * @param m : Member to add to the guild
 * @return 0=error, 1=msg_sent
 */
int intif_guild_addmember(int guild_id,struct guild_member *m)
{
	if (CheckForCharServer())
		return 0;
	WFIFOHEAD(inter_fd,sizeof(struct guild_member)+8);
	WFIFOW(inter_fd,0) = 0x3032;
	WFIFOW(inter_fd,2) = sizeof(struct guild_member)+8;
	WFIFOL(inter_fd,4) = guild_id;
	memcpy(WFIFOP(inter_fd,8),m,sizeof(struct guild_member));
	WFIFOSET(inter_fd,WFIFOW(inter_fd,2));
	return 1;
}

/**
 * Request a new leader for guild
 * @param guild_id : guild to alter
 * @param name : name of the new master
 * @param len : size of the name
 * @return 0=error, 1=msg_sent
 */
int intif_guild_change_gm(int guild_id, const char* name, int len)
{
	if (CheckForCharServer())
		return 0;
	WFIFOHEAD(inter_fd, len + 8);
	WFIFOW(inter_fd, 0)=0x3033;
	WFIFOW(inter_fd, 2)=len+8;
	WFIFOL(inter_fd, 4)=guild_id;
	memcpy(WFIFOP(inter_fd,8),name,len);
	WFIFOSET(inter_fd,len+8);
	return 1;
}

/**
 * Request to make a player leave a guild
 * @param guild_id : guild to alter
 * @param account_id : player aid to kick
 * @param char_id : player cid to kick
 * @param flag : 0:normal quit, 1=expulsion
 * @param mes : quitting message (max 40)
 * @return 0=error, 1=msg_sent
 */
int intif_guild_leave(int guild_id,uint32 account_id,uint32 char_id,int flag,const char *mes)
{
	if (CheckForCharServer())
		return 0;
	WFIFOHEAD(inter_fd, 55);
	WFIFOW(inter_fd, 0) = 0x3034;
	WFIFOL(inter_fd, 2) = guild_id;
	WFIFOL(inter_fd, 6) = account_id;
	WFIFOL(inter_fd,10) = char_id;
	WFIFOB(inter_fd,14) = flag;
	safestrncpy((char*)WFIFOP(inter_fd,15),mes,40);
	WFIFOSET(inter_fd,55);
	return 1;
}

/**
 * Update request / Lv online status of the guild members
 * @param guild_id : guild to alter
 * @param account_id : player aid to alter
 * @param char_id : player cid to alter
 * @param online : does the player is online (or will stay online)
 * @param lv : player lv
 * @param class_ : player class
 * @return 0=error, 1=msg_sent
 */
int intif_guild_memberinfoshort(int guild_id,uint32 account_id,uint32 char_id,int online,int lv,int class_)
{
	if (CheckForCharServer())
		return 0;
	WFIFOHEAD(inter_fd, 19);
	WFIFOW(inter_fd, 0) = 0x3035;
	WFIFOL(inter_fd, 2) = guild_id;
	WFIFOL(inter_fd, 6) = account_id;
	WFIFOL(inter_fd,10) = char_id;
	WFIFOB(inter_fd,14) = online;
	WFIFOW(inter_fd,15) = lv;
	WFIFOW(inter_fd,17) = class_;
	WFIFOSET(inter_fd,19);
	return 1;
}

/**
 * Guild disbanded notification 
 * @param guild_id : guild to disband
 * @return 0=error, 1=msg_sent
 */
int intif_guild_break(int guild_id)
{
	if (CheckForCharServer())
		return 0;
	WFIFOHEAD(inter_fd, 6);
	WFIFOW(inter_fd, 0) = 0x3036;
	WFIFOL(inter_fd, 2) = guild_id;
	WFIFOSET(inter_fd,6);
	return 1;
}

/**
 * Send a guild message
 * (This is goign throught char-serv in case of multi-map setup)
 * @param guild_id : Guild id to send the message to
 * @param account_id : Player sending the msg
 * @param mes : Message to send
 * @param len : Size of the message
 * @return 0=error, 1=msg_sent
 */
int intif_guild_message(int guild_id,uint32 account_id,const char *mes,int len)
{
	if (CheckForCharServer())
		return 0;

	if (other_mapserver_count < 1)
		return 0; //No need to send.

	WFIFOHEAD(inter_fd, len + 12);
	WFIFOW(inter_fd,0)=0x3037;
	WFIFOW(inter_fd,2)=len+12;
	WFIFOL(inter_fd,4)=guild_id;
	WFIFOL(inter_fd,8)=account_id;
	memcpy(WFIFOP(inter_fd,12),mes,len);
	WFIFOSET(inter_fd,len+12);

	return 1;
}

/**
 * Request a change of Guild basic information
 * @param guild_id : Guild to alter
 * @param type : type of data to change (currently only guildlv)
 * @param data : new value for type
 * @param len : size of data
 * @return 0=error, 1=msg_sent
 */
int intif_guild_change_basicinfo(int guild_id,int type,const void *data,int len)
{
	if (CheckForCharServer())
		return 0;
	WFIFOHEAD(inter_fd, len + 10);
	WFIFOW(inter_fd,0)=0x3039;
	WFIFOW(inter_fd,2)=len+10;
	WFIFOL(inter_fd,4)=guild_id;
	WFIFOW(inter_fd,8)=type;
	memcpy(WFIFOP(inter_fd,10),data,len);
	WFIFOSET(inter_fd,len+10);
	return 1;
}

/**
 * Request a change of Guild member information
 * @param guild_id : Guild to alter
 * @param account_id : Player aid to alter 
 * @param char_id : Player cid to alter
 * @param type : Type of modification
 * @param data : Value of modification
 * @param len : Size of value
 * @return 0=error, 1=msg_sent
 */
int intif_guild_change_memberinfo(int guild_id,uint32 account_id,uint32 char_id,
	int type,const void *data,int len)
{
	if (CheckForCharServer())
		return 0;
	WFIFOHEAD(inter_fd, len + 18);
	WFIFOW(inter_fd, 0)=0x303a;
	WFIFOW(inter_fd, 2)=len+18;
	WFIFOL(inter_fd, 4)=guild_id;
	WFIFOL(inter_fd, 8)=account_id;
	WFIFOL(inter_fd,12)=char_id;
	WFIFOW(inter_fd,16)=type;
	memcpy(WFIFOP(inter_fd,18),data,len);
	WFIFOSET(inter_fd,len+18);
	return 1;
}

/**
 * Request a change of Guild title
 * @param guild_id : guild to alter
 * @param idx : Position Index
 * @param p : Position data { <mode>.L <ranking>.L <pay rate>.L <name>.24B }
 * @return 0=error, 1=msg_sent
 */
int intif_guild_position(int guild_id,int idx,struct guild_position *p)
{
	if (CheckForCharServer())
		return 0;
	WFIFOHEAD(inter_fd, sizeof(struct guild_position)+12);
	WFIFOW(inter_fd,0)=0x303b;
	WFIFOW(inter_fd,2)=sizeof(struct guild_position)+12;
	WFIFOL(inter_fd,4)=guild_id;
	WFIFOL(inter_fd,8)=idx;
	memcpy(WFIFOP(inter_fd,12),p,sizeof(struct guild_position));
	WFIFOSET(inter_fd,WFIFOW(inter_fd,2));
	return 1;
}

/**
 * Request an update of Guildskill skill_id
 * @param guild_id : Guild to alter
 * @param skill_id : Skill to lvl up
 * @param account_id : aid requesting update
 * @param max : skill max level
 * @return 0=error, 1=msg_sent
 */
int intif_guild_skillup(int guild_id, uint16 skill_id, uint32 account_id, int max)
{
	if( CheckForCharServer() )
		return 0;
	WFIFOHEAD(inter_fd, 18);
	WFIFOW(inter_fd, 0)  = 0x303c;
	WFIFOL(inter_fd, 2)  = guild_id;
	WFIFOL(inter_fd, 6)  = skill_id;
	WFIFOL(inter_fd, 10) = account_id;
	WFIFOL(inter_fd, 14) = max;
	WFIFOSET(inter_fd, 18);
	return 1;
}

/**
 * Request a new guild relationship
 * @param guild_id1 : Guild to associate 1
 * @param guild_id2 : Guild to associate 2
 * @param account_id1 : aid of player in guild1 
 * @param account_id2 : aid of player in guild2
 * @param flag : (GUILD_ALLIANCE_REMOVE|0|1)
 * @return  0=error, 1=msg_sent
 */
int intif_guild_alliance(int guild_id1,int guild_id2,uint32 account_id1,uint32 account_id2,int flag)
{
	if (CheckForCharServer())
		return 0;
	WFIFOHEAD(inter_fd,19);
	WFIFOW(inter_fd, 0)=0x303d;
	WFIFOL(inter_fd, 2)=guild_id1;
	WFIFOL(inter_fd, 6)=guild_id2;
	WFIFOL(inter_fd,10)=account_id1;
	WFIFOL(inter_fd,14)=account_id2;
	WFIFOB(inter_fd,18)=flag;
	WFIFOSET(inter_fd,19);
	return 1;
}

/**
 * Request to change guild notice
 * @param guild_id : Guild to alter
 * @param mes1 : Notice title (max 60)
 * @param mes2 : Notice body (max 120)
 * @return 0=error, 1=msg_sent
 */
int intif_guild_notice(int guild_id,const char *mes1,const char *mes2)
{
	if (CheckForCharServer())
		return 0;
	WFIFOHEAD(inter_fd,186);
	WFIFOW(inter_fd,0)=0x303e;
	WFIFOL(inter_fd,2)=guild_id;
	memcpy(WFIFOP(inter_fd,6),mes1,MAX_GUILDMES1);
	memcpy(WFIFOP(inter_fd,66),mes2,MAX_GUILDMES2);
	WFIFOSET(inter_fd,186);
	return 1;
}

/**
 * Request to change guild emblem
 * @param guild_id
 * @param len
 * @param data
 * @return 
 */
int intif_guild_emblem(int guild_id,int len,const char *data)
{
	if (CheckForCharServer())
		return 0;
	if(guild_id<=0 || len<0 || len>2000)
		return 0;
	WFIFOHEAD(inter_fd,len + 12);
	WFIFOW(inter_fd,0)=0x303f;
	WFIFOW(inter_fd,2)=len+12;
	WFIFOL(inter_fd,4)=guild_id;
	WFIFOL(inter_fd,8)=0;
	memcpy(WFIFOP(inter_fd,12),data,len);
	WFIFOSET(inter_fd,len+12);
	return 1;
}

/**
 * Requests guild castles data from char-server.
 * @param num Number of castles, size of castle_ids array.
 * @param castle_ids Pointer to array of castle IDs.
 */
int intif_guild_castle_dataload(int num, int *castle_ids)
{
	if (CheckForCharServer())
		return 0;
	WFIFOHEAD(inter_fd, 4 + num * sizeof(int));
	WFIFOW(inter_fd, 0) = 0x3040;
	WFIFOW(inter_fd, 2) = 4 + num * sizeof(int);
	memcpy(WFIFOP(inter_fd, 4), castle_ids, num * sizeof(int));
	WFIFOSET(inter_fd, WFIFOW(inter_fd, 2));
	return 1;
}

/**
 * Request change castle guild owner and save data
 * @param castle_id
 * @param index
 * @param value
 * @return 
 */
int intif_guild_castle_datasave(int castle_id,int index, int value)
{
	if (CheckForCharServer())
		return 0;
	WFIFOHEAD(inter_fd,9);
	WFIFOW(inter_fd,0)=0x3041;
	WFIFOW(inter_fd,2)=castle_id;
	WFIFOB(inter_fd,4)=index;
	WFIFOL(inter_fd,5)=value;
	WFIFOSET(inter_fd,9);
	return 1;
}

//-----------------------------------------------------------------
// Homunculus Packets send to Inter server [albator]
//-----------------------------------------------------------------

/**
 * Request to create/register homonculus
 * @param account_id : player requesting
 * @param sh : TMp homunlus data
 * @return 0=error, 1=msg_sent
 */
int intif_homunculus_create(uint32 account_id, struct s_homunculus *sh)
{
	if (CheckForCharServer())
		return 0;
	WFIFOHEAD(inter_fd, sizeof(struct s_homunculus)+8);
	WFIFOW(inter_fd,0) = 0x3090;
	WFIFOW(inter_fd,2) = sizeof(struct s_homunculus)+8;
	WFIFOL(inter_fd,4) = account_id;
	memcpy(WFIFOP(inter_fd,8),sh,sizeof(struct s_homunculus));
	WFIFOSET(inter_fd, WFIFOW(inter_fd,2));
	return 1;
}

/**
 * Request to load homunculus from char-serv
 * @param account_id
 * @param homun_id
 * @return 0=error, 1=msg sent
 */
int intif_homunculus_requestload(uint32 account_id, int homun_id)
{
	if (CheckForCharServer())
		return 0;
	WFIFOHEAD(inter_fd, 10);
	WFIFOW(inter_fd,0) = 0x3091;
	WFIFOL(inter_fd,2) = account_id;
	WFIFOL(inter_fd,6) = homun_id;
	WFIFOSET(inter_fd, 10);
	return 1;
}

/**
 * Request to save homunculus
 * @param account_id : Player asking save
 * @param sh : homunculus struct
 * @return : 0=error, 1=msg sent
 */
int intif_homunculus_requestsave(uint32 account_id, struct s_homunculus* sh)
{
	if (CheckForCharServer())
		return 0;
	WFIFOHEAD(inter_fd, sizeof(struct s_homunculus)+8);
	WFIFOW(inter_fd,0) = 0x3092;
	WFIFOW(inter_fd,2) = sizeof(struct s_homunculus)+8;
	WFIFOL(inter_fd,4) = account_id;
	memcpy(WFIFOP(inter_fd,8),sh,sizeof(struct s_homunculus));
	WFIFOSET(inter_fd, WFIFOW(inter_fd,2));
	return 1;

}

/**
 * request to delete homunculus
 * @param homun_id
 * @return 0=error, 1=msg sent
 */
int intif_homunculus_requestdelete(int homun_id)
{
	if (CheckForCharServer())
		return 0;
	WFIFOHEAD(inter_fd, 6);
	WFIFOW(inter_fd, 0) = 0x3093;
	WFIFOL(inter_fd,2) = homun_id;
	WFIFOSET(inter_fd,6);
	return 1;

}


//-----------------------------------------------------------------
// Packets receive from inter server

/**
 * Receive a whisper request from char-serv transmit it to player
 * @author : rewritten by [Yor]
 * @param fd : char-serv link
 * @return 0=not found or ignored, 1=transmited
 */
int intif_parse_WisMessage(int fd)
{
	struct map_session_data* sd;
	char *wisp_source;
	char name[NAME_LENGTH];
	int id, i;

	id=RFIFOL(fd,4);

	safestrncpy(name, (char*)RFIFOP(fd,32), NAME_LENGTH);
	sd = map_nick2sd(name);
	if(sd == NULL || strcmp(sd->status.name, name) != 0)
	{	//Not found
		intif_wis_replay(id,1);
		return 0;
	}
	if(sd->state.ignoreAll) {
		intif_wis_replay(id, 2);
		return 0;
	}
	wisp_source = (char *) RFIFOP(fd,8); // speed up [Yor]
	for(i=0; i < MAX_IGNORE_LIST &&
		sd->ignore[i].name[0] != '\0' &&
		strcmp(sd->ignore[i].name, wisp_source) != 0
		; i++);

	if (i < MAX_IGNORE_LIST && sd->ignore[i].name[0] != '\0')
	{	//Ignored
		intif_wis_replay(id, 2);
		return 0;
	}
	//Success to send whisper.
	clif_wis_message(sd->fd, wisp_source, (char*)RFIFOP(fd,56),RFIFOW(fd,2)-56);
	intif_wis_replay(id,0);   // success
	return 1;
}

/**
 * Wisp/page transmission result reception
 * @param fd : char-serv link
 * @return 1
 */
int intif_parse_WisEnd(int fd)
{
	struct map_session_data* sd;

	if (battle_config.etc_log)
		ShowInfo("intif_parse_wisend: player: %s, flag: %d\n", RFIFOP(fd,2), RFIFOB(fd,26)); // flag: 0: success to send wisper, 1: target character is not loged in?, 2: ignored by target
	sd = (struct map_session_data *)map_nick2sd((char *) RFIFOP(fd,2));
	if (sd != NULL)
		clif_wis_end(sd->fd, RFIFOB(fd,26));

	return 1;
}

/**
 * Transmit a whisper message to sd
 * @param sd : Player to transmit the message to
 * @param va : list of arguments ( wisp_name, message, len)
 * @return 0=error, 1=msg sent
 */
static int mapif_parse_WisToGM_sub(struct map_session_data* sd,va_list va)
{
	int permission = va_arg(va, int);
	char *wisp_name;
	char *message;
	int len;

	if (!pc_has_permission(sd, permission))
		return 0;
	wisp_name = va_arg(va, char*);
	message = va_arg(va, char*);
	len = va_arg(va, int);
	clif_wis_message(sd->fd, wisp_name, message, len);
	return 1;
}


/**
 * Received wisp message from map-server via char-server for ALL gm
 * 0x3003/0x3803 <packet_len>.w <wispname>.24B <permission>.l <message>.?B
 * @see mapif_parse_WisToGM_sub, for 1 transmission
 * @param fd : char-serv link
 * @return 1
 */
int mapif_parse_WisToGM(int fd)
{
	int permission, mes_len;
	char Wisp_name[NAME_LENGTH];
	char *message;

	mes_len =  RFIFOW(fd,2) - 8+NAME_LENGTH;
	message = (char *) aMalloc(mes_len);

	permission = RFIFOL(fd,4+NAME_LENGTH);
	safestrncpy(Wisp_name, (char*)RFIFOP(fd,4), NAME_LENGTH);
	safestrncpy(message, (char*)RFIFOP(fd,8+NAME_LENGTH), mes_len);
	// information is sent to all online GM
	map_foreachpc(mapif_parse_WisToGM_sub, permission, Wisp_name, message, mes_len);
	aFree(message);
	return 1;
}

/**
 * Request player registry
 * @param fd : char-serv link
 * @return 0=error, 1=sucess
 */
void intif_parse_Registers(int fd)
{
	int flag;
	struct map_session_data *sd;
	uint32 account_id = RFIFOL(fd,4), char_id = RFIFOL(fd,8);
	struct auth_node *node = chrif_auth_check(account_id, char_id, ST_LOGIN);
	char type = RFIFOB(fd, 13);

	if (node)
		sd = node->sd;
	else { //Normally registries should arrive for in log-in chars.
		sd = map_id2sd(account_id);
	}

	if (!sd || sd->status.char_id != char_id) {
		return; //Character registry from another character.
	}

	flag = ( sd->vars_received&PRL_ACCG && sd->vars_received&PRL_ACCL && sd->vars_received&PRL_CHAR ) ? 0 : 1;

	switch (RFIFOB(fd,12)) {
		case 3: //Character Registry
			sd->vars_received |= PRL_CHAR;
			break;
		case 2: //Account Registry
			sd->vars_received |= PRL_ACCL;
			break;
		case 1: //Account2 Registry
			sd->vars_received |= PRL_ACCG;
			break;
		case 0:
			break;
		default:
			ShowError("intif_parse_Registers: Unrecognized type %d\n",RFIFOB(fd,12));
			return;
	}

	// have it not complain about insertion of vars before loading, and not set those vars as new or modified
	reg_load = true;
	
	if( RFIFOW(fd, 14) ) {
		char key[32];
		unsigned int index;
		int max = RFIFOW(fd, 14), cursor = 16, i;

		/**
		 * Vessel!char_reg_num_db
		 *
		 * str type
		 * { keyLength(B), key(<keyLength>), index(L), valLength(B), val(<valLength>) }
		 **/
		if (type) {
			for(i = 0; i < max; i++) {
				char sval[254];
				safestrncpy(key, (char*)RFIFOP(fd, cursor + 1), RFIFOB(fd, cursor));
				cursor += RFIFOB(fd, cursor) + 1;

				index = RFIFOL(fd, cursor);
				cursor += 4;

				safestrncpy(sval, (char*)RFIFOP(fd, cursor + 1), RFIFOB(fd, cursor));
				cursor += RFIFOB(fd, cursor) + 1;

				set_reg(NULL,sd,reference_uid(add_str(key), index), key, (void*)sval, NULL);
			}
		/**
		 * Vessel!
		 *
		 * int type
		 * { keyLength(B), key(<keyLength>), index(L), value(L) }
		 **/
		} else {
			for(i = 0; i < max; i++) {
				int ival;
				safestrncpy(key, (char*)RFIFOP(fd, cursor + 1), RFIFOB(fd, cursor));
				cursor += RFIFOB(fd, cursor) + 1;

				index = RFIFOL(fd, cursor);
				cursor += 4;

				ival = RFIFOL(fd, cursor);
				cursor += 4;

				set_reg(NULL,sd,reference_uid(add_str(key), index), key, (void*)__64BPRTSIZE(ival), NULL);
			}
		}
	}

	reg_load = false;

	if (flag && sd->vars_received&PRL_ACCG && sd->vars_received&PRL_ACCL && sd->vars_received&PRL_CHAR)
		pc_reg_received(sd); //Received all registry values, execute init scripts and what-not. [Skotlex]
}

/**
 * Received a guild storage
 * @param fd : char-serv link
 * @return 0=error, 1=sucess
 */
int intif_parse_LoadGuildStorage(int fd)
{
	struct guild_storage *gstor;
	struct map_session_data *sd;
	int guild_id, flag;

	guild_id = RFIFOL(fd,8);
	flag = RFIFOL(fd,12);
	if (guild_id <= 0)
		return 0;

	sd = map_id2sd( RFIFOL(fd,4) );
	if (flag){ //If flag != 0, we attach a player and open the storage
		if(sd == NULL){
			ShowError("intif_parse_LoadGuildStorage: user not found (AID: %d)\n",RFIFOL(fd,4));
			return 0;
		}
	}
	gstor = gstorage_guild2storage(guild_id);
	if (!gstor) {
		ShowWarning("intif_parse_LoadGuildStorage: error guild_id %d not exist\n",guild_id);
		return 0;
	}
	if (gstor->opened) { // Already open.. lets ignore this update
		ShowWarning("intif_parse_LoadGuildStorage: storage received for a client already open (User %d:%d)\n", flag?sd->status.account_id:1, flag?sd->status.char_id:1);
		return 0;
	}
	if (gstor->dirty) { // Already have storage, and it has been modified and not saved yet! Exploit! [Skotlex]
		ShowWarning("intif_parse_LoadGuildStorage: received storage for an already modified non-saved storage! (User %d:%d)\n", flag?sd->status.account_id:1, flag?sd->status.char_id:1);
		return 0;
	}
	if( RFIFOW(fd,2)-13 != sizeof(struct guild_storage) ){
		ShowError("intif_parse_LoadGuildStorage: data size error %d %d\n",RFIFOW(fd,2)-13 , sizeof(struct guild_storage));
		gstor->opened = 0;
		return 0;
	}

	memcpy(gstor,RFIFOP(fd,13),sizeof(struct guild_storage));
	if( flag )
		gstorage_storageopen(sd);

	return 1;
}

/**
 * ACK guild_storage saved
 * @param fd : char-serv link
 * @return 1
 */
int intif_parse_SaveGuildStorage(int fd)
{
	gstorage_storagesaved(/*RFIFOL(fd,2), */RFIFOL(fd,6));
	return 1;
}

/**
 * ACK party creation
 * @param fd : char-serv link
 * @return 1
 */
int intif_parse_PartyCreated(int fd)
{
	if(battle_config.etc_log)
		ShowInfo("intif: party created by account %d\n\n", RFIFOL(fd,2));
	party_created(RFIFOL(fd,2), RFIFOL(fd,6),RFIFOB(fd,10),RFIFOL(fd,11), (char *)RFIFOP(fd,15));
	return 1;
}

/**
 * Receive party info
 * @param fd : char-serv link
 * @return 0=error, 1=sucess
 */
int intif_parse_PartyInfo(int fd)
{
	if( RFIFOW(fd,2) == 12 ){
		ShowWarning("intif: party noinfo (char_id=%d party_id=%d)\n", RFIFOL(fd,4), RFIFOL(fd,8));
		party_recv_noinfo(RFIFOL(fd,8), RFIFOL(fd,4));
		return 0;
	}

	if( RFIFOW(fd,2) != 8+sizeof(struct party) )
		ShowError("intif: party info : data size error (char_id=%d party_id=%d packet_len=%d expected_len=%d)\n", RFIFOL(fd,4), RFIFOL(fd,8), RFIFOW(fd,2), 8+sizeof(struct party));
	party_recv_info((struct party *)RFIFOP(fd,8), RFIFOL(fd,4));
	return 1;
}

/**
 * ACK adding party member
 * @param fd : char-serv link
 * @return 1
 */
int intif_parse_PartyMemberAdded(int fd)
{
	if(battle_config.etc_log)
		ShowInfo("intif: party member added Party (%d), Account(%d), Char(%d)\n",RFIFOL(fd,2),RFIFOL(fd,6),RFIFOL(fd,10));
	party_member_added(RFIFOL(fd,2),RFIFOL(fd,6),RFIFOL(fd,10), RFIFOB(fd, 14));
	return 1;
}

/**
 * ACK changing party option
 * @param fd : char-serv link
 * @return 1
 */
int intif_parse_PartyOptionChanged(int fd)
{
	party_optionchanged(RFIFOL(fd,2),RFIFOL(fd,6),RFIFOW(fd,10),RFIFOW(fd,12),RFIFOB(fd,14));
	return 1;
}

/**
 * ACK member leaving party
 * @param fd : char-serv link
 * @return 1
 */
int intif_parse_PartyMemberWithdraw(int fd)
{
	if(battle_config.etc_log)
		ShowInfo("intif: party member withdraw: Party(%d), Account(%d), Char(%d)\n",RFIFOL(fd,2),RFIFOL(fd,6),RFIFOL(fd,10));
	party_member_withdraw(RFIFOL(fd,2),RFIFOL(fd,6),RFIFOL(fd,10));
	return 1;
}

/**
 * ACK party break
 * @param fd : char-serv link
 * @return 1
 */
int intif_parse_PartyBroken(int fd)
{
	party_broken(RFIFOL(fd,2));
	return 1;
}

/**
 * ACK party on new map
 * @param fd : char-serv link
 * @return 1
 */
int intif_parse_PartyMove(int fd)
{
	party_recv_movemap(RFIFOL(fd,2),RFIFOL(fd,6),RFIFOL(fd,10),RFIFOW(fd,14),RFIFOB(fd,16),RFIFOW(fd,17));
	return 1;
}

/**
 * ACK party messages
 * @param fd : char-serv link
 * @return 1
 */
int intif_parse_PartyMessage(int fd)
{
	party_recv_message(RFIFOL(fd,4),RFIFOL(fd,8),(char *) RFIFOP(fd,12),RFIFOW(fd,2)-12);
	return 1;
}

/**
 * ACK guild creation
 * @param fd : char-serv link
 * @return 1
 */
int intif_parse_GuildCreated(int fd)
{
	guild_created(RFIFOL(fd,2),RFIFOL(fd,6));
	return 1;
}

/**
 * ACK guild infos
 * @param fd : char-serv link
 * @return 0=error, 1=sucess
 */
int intif_parse_GuildInfo(int fd)
{
	if(RFIFOW(fd,2) == 8) {
		ShowWarning("intif: guild noinfo %d\n",RFIFOL(fd,4));
		guild_recv_noinfo(RFIFOL(fd,4));
		return 0;
	}
	if( RFIFOW(fd,2)!=sizeof(struct guild)+4 )
		ShowError("intif: guild info : data size error Gid: %d recv size: %d Expected size: %d\n",RFIFOL(fd,4),RFIFOW(fd,2),sizeof(struct guild)+4);
	guild_recv_info((struct guild *)RFIFOP(fd,4));
	return 1;
}

/**
 * ACK adding guild member
 * @param fd : char-serv link
 * @return 1
 */
int intif_parse_GuildMemberAdded(int fd)
{
	if(battle_config.etc_log)
		ShowInfo("intif: guild member added %d %d %d %d\n",RFIFOL(fd,2),RFIFOL(fd,6),RFIFOL(fd,10),RFIFOB(fd,14));
	guild_member_added(RFIFOL(fd,2),RFIFOL(fd,6),RFIFOL(fd,10),RFIFOB(fd,14));
	return 1;
}

/**
 * ACK member leaving guild
 * @param fd : char-serv link
 * @return 1
 */
int intif_parse_GuildMemberWithdraw(int fd)
{
	guild_member_withdraw(RFIFOL(fd,2),RFIFOL(fd,6),RFIFOL(fd,10),RFIFOB(fd,14),(char *)RFIFOP(fd,55),(char *)RFIFOP(fd,15));
	return 1;
}

/**
 * ACK guild member basic info
 * @param fd : char-serv link
 * @return 1
 */
int intif_parse_GuildMemberInfoShort(int fd)
{
	guild_recv_memberinfoshort(RFIFOL(fd,2),RFIFOL(fd,6),RFIFOL(fd,10),RFIFOB(fd,14),RFIFOW(fd,15),RFIFOW(fd,17));
	return 1;
}

/**
 * ACK guild break
 * @param fd : char-serv link
 * @return 1
 */
int intif_parse_GuildBroken(int fd)
{
	guild_broken(RFIFOL(fd,2),RFIFOB(fd,6));
	return 1;
}

/**
 * basic guild info change notice
 * 0x3839 <packet len>.w <guild id>.l <type>.w <data>.?b
 * @param fd : char-serv link
 * @return 0=error, 1=sucess
 */
int intif_parse_GuildBasicInfoChanged(int fd)
{
	//int len = RFIFOW(fd,2) - 10;
	int guild_id = RFIFOL(fd,4);
	int type = RFIFOW(fd,8);
	//void* data = RFIFOP(fd,10);

	struct guild* g = guild_search(guild_id);
	if( g == NULL )
		return 0;

	switch(type) {
	case GBI_EXP:        g->exp = RFIFOQ(fd,10); break;
	case GBI_GUILDLV:    g->guild_lv = RFIFOW(fd,10); break;
	case GBI_SKILLPOINT: g->skill_point = RFIFOL(fd,10); break;
	}

	return 1;
}

/**
 * guild member info change notice
 * 0x383a <packet len>.w <guild id>.l <account id>.l <char id>.l <type>.w <data>.?b
 * @param fd : char-serv link
 * @return 0=error, 1=sucess
 */
int intif_parse_GuildMemberInfoChanged(int fd)
{
	//int len = RFIFOW(fd,2) - 18;
	int guild_id = RFIFOL(fd,4);
	uint32 account_id = RFIFOL(fd,8);
	uint32 char_id = RFIFOL(fd,12);
	int type = RFIFOW(fd,16);
	//void* data = RFIFOP(fd,18);

	struct guild* g;
	int idx;

	g = guild_search(guild_id);
	if( g == NULL )
		return 0;

	idx = guild_getindex(g,account_id,char_id);
	if( idx == -1 )
		return 0;

	switch( type ) {
	case GMI_POSITION:   g->member[idx].position   = RFIFOW(fd,18); guild_memberposition_changed(g,idx,RFIFOW(fd,18)); break;
	case GMI_EXP:        g->member[idx].exp        = RFIFOQ(fd,18); break;
	case GMI_HAIR:       g->member[idx].hair       = RFIFOW(fd,18); break;
	case GMI_HAIR_COLOR: g->member[idx].hair_color = RFIFOW(fd,18); break;
	case GMI_GENDER:     g->member[idx].gender     = RFIFOW(fd,18); break;
	case GMI_CLASS:      g->member[idx].class_     = RFIFOW(fd,18); break;
	case GMI_LEVEL:      g->member[idx].lv         = RFIFOW(fd,18); break;
	}
	return 1;
}

/**
 * ACK change of guild title
 * @param fd : char-serv link
 * @return 1
 */
int intif_parse_GuildPosition(int fd)
{
	if( RFIFOW(fd,2)!=sizeof(struct guild_position)+12 )
		ShowError("intif: guild info : data size error\n %d %d %d",RFIFOL(fd,4),RFIFOW(fd,2),sizeof(struct guild_position)+12);
	guild_position_changed(RFIFOL(fd,4),RFIFOL(fd,8),(struct guild_position *)RFIFOP(fd,12));
	return 1;
}

/**
 * ACK change of guild skill update
 * @param fd : char-serv link
 * @return 1
 */
int intif_parse_GuildSkillUp(int fd)
{
	guild_skillupack(RFIFOL(fd,2),RFIFOL(fd,6),RFIFOL(fd,10));
	return 1;
}

/**
 * ACK change of guild relationship
 * @param fd : char-serv link
 * @return 1
 */
int intif_parse_GuildAlliance(int fd)
{
	guild_allianceack(RFIFOL(fd,2),RFIFOL(fd,6),RFIFOL(fd,10),RFIFOL(fd,14),RFIFOB(fd,18),(char *) RFIFOP(fd,19),(char *) RFIFOP(fd,43));
	return 1;
}

/**
 * ACK change of guild notice
 * @param fd : char-serv link
 * @return 1
 */
int intif_parse_GuildNotice(int fd)
{
	guild_notice_changed(RFIFOL(fd,2),(char *) RFIFOP(fd,6),(char *) RFIFOP(fd,66));
	return 1;
}

/**
 * ACK change of guild emblem
 * @param fd : char-serv link
 * @return 1
 */
int intif_parse_GuildEmblem(int fd)
{
	guild_emblem_changed(RFIFOW(fd,2)-12,RFIFOL(fd,4),RFIFOL(fd,8), (char *)RFIFOP(fd,12));
	return 1;
}

/**
 * ACK guild message
 * @param fd : char-serv link
 * @return  1
 */
int intif_parse_GuildMessage(int fd)
{
	guild_recv_message(RFIFOL(fd,4),RFIFOL(fd,8),(char *) RFIFOP(fd,12),RFIFOW(fd,2)-12);
	return 1;
}

/**
 * Reply guild castle data request
 * @param fd : char-serv link
 * @return ?
 */
int intif_parse_GuildCastleDataLoad(int fd)
{
	return guild_castledataloadack(RFIFOW(fd,2), (struct guild_castle *)RFIFOP(fd,4));
}

/**
 * ACK change of guildmaster
 * @param fd : char-serv link
 * @return ?
 */
int intif_parse_GuildMasterChanged(int fd)
{
	return guild_gm_changed(RFIFOL(fd,2),RFIFOL(fd,6),RFIFOL(fd,10));
}

/**
 * Request pet creation
 * @param fd : char-serv link
 * @return 1
 */
int intif_parse_CreatePet(int fd)
{
	pet_get_egg(RFIFOL(fd,2),RFIFOW(fd,6),RFIFOL(fd,8));
	return 1;
}

/**
 * ACK pet data
 * @param fd : char-serv link
 * @return 1
 */
int intif_parse_RecvPetData(int fd)
{
	struct s_pet p;
	int len;
	len=RFIFOW(fd,2);
	if(sizeof(struct s_pet)!=len-9) {
		if(battle_config.etc_log)
			ShowError("intif: pet data: data size error %d %d\n",sizeof(struct s_pet),len-9);
	}
	else{
		memcpy(&p,RFIFOP(fd,9),sizeof(struct s_pet));
		pet_recv_petdata(RFIFOL(fd,4),&p,RFIFOB(fd,8));
	}

	return 1;
}

/**
 * ACK pet save data
 * @param fd : char-serv link
 * @return 1
 */
int intif_parse_SavePetOk(int fd)
{
	if(RFIFOB(fd,6) == 1)
		ShowError("pet data save failure\n");

	return 1;
}

/**
 * ACK deleting pet
 * @param fd : char-serv link
 * @return 1
 */
int intif_parse_DeletePetOk(int fd)
{
	if(RFIFOB(fd,2) == 1)
		ShowError("pet data delete failure\n");

	return 1;
}

/**
 * ACK changing name resquest, players,pets,hommon
 * @param fd : char-serv link
 * @return 0=error, 1=sucess
 */
int intif_parse_ChangeNameOk(int fd)
{
	struct map_session_data *sd = NULL;
	if((sd=map_id2sd(RFIFOL(fd,2)))==NULL ||
		sd->status.char_id != RFIFOL(fd,6))
		return 0;

	switch (RFIFOB(fd,10)) {
	case 0: //Players [NOT SUPPORTED YET]
		break;
	case 1: //Pets
		pet_change_name_ack(sd, (char*)RFIFOP(fd,12), RFIFOB(fd,11));
		break;
	case 2: //Hom
		hom_change_name_ack(sd, (char*)RFIFOP(fd,12), RFIFOB(fd,11));
		break;
	}
	return 1;
}

//----------------------------------------------------------------
// Homunculus recv packets [albator]

/**
 * ACK Homunculus creation
 * @param fd : char-serv link
 * @return 0=error, 1=sucess
 */
int intif_parse_CreateHomunculus(int fd)
{
	int len;
	len=RFIFOW(fd,2)-9;
	if(sizeof(struct s_homunculus)!=len) {
		if(battle_config.etc_log)
			ShowError("intif: create homun data: data size error %d != %d\n",sizeof(struct s_homunculus),len);
		return 0;
	}
	hom_recv_data(RFIFOL(fd,4), (struct s_homunculus*)RFIFOP(fd,9), RFIFOB(fd,8)) ;
	return 1;
}

/**
 * ACK homunculus get data (load homun from char)
 * @param fd : char-serv link
 * @return 0=error, 1=sucess
 */
int intif_parse_RecvHomunculusData(int fd)
{
	int len;

	len=RFIFOW(fd,2)-9;

	if(sizeof(struct s_homunculus)!=len) {
		if(battle_config.etc_log)
			ShowError("intif: homun data: data size error %d %d\n",sizeof(struct s_homunculus),len);
		return 0;
	}
	hom_recv_data(RFIFOL(fd,4), (struct s_homunculus*)RFIFOP(fd,9), RFIFOB(fd,8));
	return 1;
}

/**
 * ACK save Homun
 * @param fd : char-serv link
 * @return 1
 */
int intif_parse_SaveHomunculusOk(int fd)
{
	if(RFIFOB(fd,6) != 1)
		ShowError("homunculus data save failure for account %d\n", RFIFOL(fd,2));

	return 1;
}

/**
 * ACK delete Homun
 * @param fd : char-serv link
 * @return 1
 */
int intif_parse_DeleteHomunculusOk(int fd)
{
	if(RFIFOB(fd,2) != 1)
		ShowError("Homunculus data delete failure\n");

	return 1;
}

/**************************************

QUESTLOG SYSTEM FUNCTIONS

***************************************/

/**
 * Requests a character's quest log entries to the inter server.
 * @param sd Character's data
 */
void intif_request_questlog(TBL_PC *sd)
{
	if (CheckForCharServer())
		return;
	
	WFIFOHEAD(inter_fd,6);
	WFIFOW(inter_fd,0) = 0x3060;
	WFIFOL(inter_fd,2) = sd->status.char_id;
	WFIFOSET(inter_fd,6);
}

/**
 * Receive a char quest log
 * @param fd : char-serv link
 */
void intif_parse_questlog(int fd)
{
	uint32 char_id = RFIFOL(fd,4), num_received = (RFIFOW(fd,2) - 8) / sizeof(struct quest);
	TBL_PC *sd = map_charid2sd(char_id);

	if(!sd) // User not online anymore
		return;

	sd->num_quests = sd->avail_quests = 0;

	if(num_received == 0) {
		if(sd->quest_log) {
			aFree(sd->quest_log);
			sd->quest_log = NULL;
		}
	} else {
		struct quest *received = (struct quest *)RFIFOP(fd,8);
		int i, k = num_received;

		if(sd->quest_log)
			RECREATE(sd->quest_log, struct quest, num_received);
		else
			CREATE(sd->quest_log, struct quest, num_received);

		for(i = 0; i < num_received; i++) {
			if(quest_search(received[i].quest_id) == &quest_dummy) {
				ShowError("intif_parse_QuestLog: quest %d not found in DB.\n", received[i].quest_id);
				continue;
			}
			if(received[i].state != Q_COMPLETE) // Insert at the beginning
				memcpy(&sd->quest_log[sd->avail_quests++], &received[i], sizeof(struct quest));
			else // Insert at the end
				memcpy(&sd->quest_log[--k], &received[i], sizeof(struct quest));
			sd->num_quests++;
		}
		if(sd->avail_quests < k) {
			// sd->avail_quests and k didn't meet in the middle: some entries were skipped
			if(k < num_received) // Move the entries at the end to fill the gap
				memmove(&sd->quest_log[k], &sd->quest_log[sd->avail_quests], sizeof(struct quest) * (num_received - k));
			sd->quest_log = (struct quest *)aRealloc(sd->quest_log, sizeof(struct quest) * sd->num_quests);
		}
	}

	quest_pc_login(sd);
}

/**
 * Parses the quest log save ack for a character from the inter server.
 * Received in reply to the requests made by intif_quest_save.
 * @see intif_parse
 * @param fd : char-serv link
 */
void intif_parse_questsave(int fd)
{
	int cid = RFIFOL(fd, 2);
	TBL_PC *sd = map_id2sd(cid);

	if( !RFIFOB(fd, 6) )
		ShowError("intif_parse_questsave: Failed to save quest(s) for character %d!\n", cid);
	else if( sd )
		sd->save_quest = false;
}

/**
 * Requests to the inter server to save a character's quest log entries.
 * @param sd Character's data
 * @return 0 in case of success, nonzero otherwise
 */
int intif_quest_save(TBL_PC *sd)
{
	int len = sizeof(struct quest) * sd->num_quests + 8;

	if(CheckForCharServer())
		return 0;

	WFIFOHEAD(inter_fd, len);
	WFIFOW(inter_fd,0) = 0x3061;
	WFIFOW(inter_fd,2) = len;
	WFIFOL(inter_fd,4) = sd->status.char_id;
	if( sd->num_quests )
		memcpy(WFIFOP(inter_fd,8), sd->quest_log, sizeof(struct quest)*sd->num_quests);
	WFIFOSET(inter_fd,  len);

	return 1;
}

/*==========================================
 * MAIL SYSTEM
 * By Zephyrus
 *==========================================*/

/**
 * Request to update inbox
 * @param char_id : Player ID linked with box
 * @param flag 0 Update Inbox | 1 OpenMail
 * @return 0=errur, 1=msg_sent
 */
int intif_Mail_requestinbox(uint32 char_id, unsigned char flag)
{
	if (CheckForCharServer())
		return 0;

	WFIFOHEAD(inter_fd,7);
	WFIFOW(inter_fd,0) = 0x3048;
	WFIFOL(inter_fd,2) = char_id;
	WFIFOB(inter_fd,6) = flag;
	WFIFOSET(inter_fd,7);

	return 1;
}

/**
 * Map-serv received a mail from char-serv 
 * (inform user of new mail)
 * @param fd : char-serv link
 * @return 0=msg fail, 1=msg received 
 */
int intif_parse_Mail_inboxreceived(int fd)
{
	struct map_session_data *sd;
	unsigned char flag = RFIFOB(fd,8);

	sd = map_charid2sd(RFIFOL(fd,4));

	if (sd == NULL)
	{
		ShowError("intif_parse_Mail_inboxreceived: char not found %d\n",RFIFOL(fd,4));
		return 0;
	}

	if (RFIFOW(fd,2) - 9 != sizeof(struct mail_data))
	{
		ShowError("intif_parse_Mail_inboxreceived: data size error %d %d\n", RFIFOW(fd,2) - 9, sizeof(struct mail_data));
		return 0;
	}

	//FIXME: this operation is not safe [ultramage]
	memcpy(&sd->mail.inbox, RFIFOP(fd,9), sizeof(struct mail_data));
	sd->mail.changed = false; // cache is now in sync

	if (flag)
		clif_Mail_refreshinbox(sd);
	else if( battle_config.mail_show_status && ( battle_config.mail_show_status == 1 || sd->mail.inbox.unread ) )
	{
		char output[128];
		sprintf(output, msg_txt(sd,510), sd->mail.inbox.unchecked, sd->mail.inbox.unread + sd->mail.inbox.unchecked);
		clif_disp_onlyself(sd, output, strlen(output));
	}
	return 1;
}

/**
 * Notify char-serv that the mail was read
 * @param mail_id : mail reed
 * @return 0=error, 1=msg sent
 */
int intif_Mail_read(int mail_id)
{
	if (CheckForCharServer())
		return 0;

	WFIFOHEAD(inter_fd,6);
	WFIFOW(inter_fd,0) = 0x3049;
	WFIFOL(inter_fd,2) = mail_id;
	WFIFOSET(inter_fd,6);

	return 1;
}

/**
 * Request the mail attachment for mail
 * @param char_id : Player requesting
 * @param mail_id : Mail identification
 * @return 0=error, 1=msg sent
 */
int intif_Mail_getattach(uint32 char_id, int mail_id)
{
	if (CheckForCharServer())
		return 0;

	WFIFOHEAD(inter_fd,10);
	WFIFOW(inter_fd,0) = 0x304a;
	WFIFOL(inter_fd,2) = char_id;
	WFIFOL(inter_fd,6) = mail_id;
	WFIFOSET(inter_fd, 10);

	return 1;
}

/**
 * Receive the attachment from char-serv of a mail
 * @param fd : char-serv link
 * @return 0=error, 1=sucess
 */
int intif_parse_Mail_getattach(int fd)
{
	struct map_session_data *sd;
	struct item item;
	int zeny = RFIFOL(fd,8);

	sd = map_charid2sd( RFIFOL(fd,4) );

	if (sd == NULL)
	{
		ShowError("intif_parse_Mail_getattach: char not found %d\n",RFIFOL(fd,4));
		return 0;
	}

	if (RFIFOW(fd,2) - 12 != sizeof(struct item))
	{
		ShowError("intif_parse_Mail_getattach: data size error %d %d\n", RFIFOW(fd,2) - 16, sizeof(struct item));
		return 0;
	}

	memcpy(&item, RFIFOP(fd,12), sizeof(struct item));

	mail_getattachment(sd, zeny, &item);
	return 1;
}

/**
 * request to delete a mail
 * @param char_id : player requesting
 * @param mail_id : mail to delete
 * @return 0=error, 1=msg sent
 */
int intif_Mail_delete(uint32 char_id, int mail_id)
{
	if (CheckForCharServer())
		return 0;

	WFIFOHEAD(inter_fd,10);
	WFIFOW(inter_fd,0) = 0x304b;
	WFIFOL(inter_fd,2) = char_id;
	WFIFOL(inter_fd,6) = mail_id;
	WFIFOSET(inter_fd,10);

	return 1;
}

/**
 * Ack of a mail deletion
 * @param fd : char-serv link
 * @return 0=error, 1=success
 */
int intif_parse_Mail_delete(int fd)
{
	uint32 char_id = RFIFOL(fd,2);
	int mail_id = RFIFOL(fd,6);
	bool failed = RFIFOB(fd,10);

	struct map_session_data *sd = map_charid2sd(char_id);
	if (sd == NULL)
	{
		ShowError("intif_parse_Mail_delete: char not found %d\n", char_id);
		return 0;
	}

	if (!failed)
	{
		int i;
		ARR_FIND(0, MAIL_MAX_INBOX, i, sd->mail.inbox.msg[i].id == mail_id);
		if( i < MAIL_MAX_INBOX )
		{
			memset(&sd->mail.inbox.msg[i], 0, sizeof(struct mail_message));
			sd->mail.inbox.amount--;
		}

		if( sd->mail.inbox.full )
			intif_Mail_requestinbox(sd->status.char_id, 1); // Free space is available for new mails
	}

	clif_Mail_delete(sd->fd, mail_id, failed);
	return 1;
}

/*------------------------------------------
 * Return Message
 *------------------------------------------*/

/**
 * Request to return a mail to his sender
 * @param char_id : player asking to return
 * @param mail_id : mail to return
 * @return 0=error, 1=msg sent
 */
int intif_Mail_return(uint32 char_id, int mail_id)
{
	if (CheckForCharServer())
		return 0;

	WFIFOHEAD(inter_fd,10);
	WFIFOW(inter_fd,0) = 0x304c;
	WFIFOL(inter_fd,2) = char_id;
	WFIFOL(inter_fd,6) = mail_id;
	WFIFOSET(inter_fd,10);

	return 1;
}

/**
 * Received a returned mail
 * @param fd
 * @return 
 */
int intif_parse_Mail_return(int fd)
{
	struct map_session_data *sd = map_charid2sd(RFIFOL(fd,2));
	int mail_id = RFIFOL(fd,6);
	short fail = RFIFOB(fd,10);

	if( sd == NULL )
	{
		ShowError("intif_parse_Mail_return: char not found %d\n",RFIFOL(fd,2));
		return 1;
	}

	if( !fail )
	{
		int i;
		ARR_FIND(0, MAIL_MAX_INBOX, i, sd->mail.inbox.msg[i].id == mail_id);
		if( i < MAIL_MAX_INBOX )
		{
			memset(&sd->mail.inbox.msg[i], 0, sizeof(struct mail_message));
			sd->mail.inbox.amount--;
		}

		if( sd->mail.inbox.full )
			intif_Mail_requestinbox(sd->status.char_id, 1); // Free space is available for new mails
	}

	clif_Mail_return(sd->fd, mail_id, fail);
	return 1;
}

/*------------------------------------------
 * Send Mail
 *------------------------------------------*/

/**
 * Request to send a mail
 * @param account_id
 * @param msg : mail struct
 * @return 0=error, 1=msg sent
 */
int intif_Mail_send(uint32 account_id, struct mail_message *msg)
{
	int len = sizeof(struct mail_message) + 8;

	if (CheckForCharServer())
		return 0;

	WFIFOHEAD(inter_fd,len);
	WFIFOW(inter_fd,0) = 0x304d;
	WFIFOW(inter_fd,2) = len;
	WFIFOL(inter_fd,4) = account_id;
	memcpy(WFIFOP(inter_fd,8), msg, sizeof(struct mail_message));
	WFIFOSET(inter_fd,len);

	return 1;
}

/**
 * Received the ack of a mail send request
 * @param fd L char-serv link
 */
static void intif_parse_Mail_send(int fd)
{
	struct mail_message msg;
	struct map_session_data *sd;
	bool fail;

	if( RFIFOW(fd,2) - 4 != sizeof(struct mail_message) )
	{
		ShowError("intif_parse_Mail_send: data size error %d %d\n", RFIFOW(fd,2) - 4, sizeof(struct mail_message));
		return;
	}

	memcpy(&msg, RFIFOP(fd,4), sizeof(struct mail_message));
	fail = (msg.id == 0);

	// notify sender
	sd = map_charid2sd(msg.send_id);
	if( sd != NULL )
	{
		if( fail )
			mail_deliveryfail(sd, &msg);
		else
		{
			clif_Mail_send(sd->fd, false);
			if( save_settings&CHARSAVE_MAIL )
				chrif_save(sd, 0);
		}
	}
}

/**
 * Received a new mail notification
 * @param fd : char-link serv
 */
static void intif_parse_Mail_new(int fd)
{
	struct map_session_data *sd = map_charid2sd(RFIFOL(fd,2));
	int mail_id = RFIFOL(fd,6);
	const char* sender_name = (char*)RFIFOP(fd,10);
	const char* title = (char*)RFIFOP(fd,34);

	if( sd == NULL )
		return;

	sd->mail.changed = true;
	clif_Mail_new(sd->fd, mail_id, sender_name, title);
}

/*==========================================
 * AUCTION SYSTEM
 * By Zephyrus
 *==========================================*/

/**
 * Request a list of auction matching criteria
 * @param char_id : player searching auction
 * @param type : see clif_parse_Auction_search type
 * @param price : min price for search
 * @param searchtext : contain item name
 * @param page : in case of huge result list display 5 entry per page, (kinda suck that we redo the request atm)
 * @return 0=error, 1=msg sent
 */
int intif_Auction_requestlist(uint32 char_id, short type, int price, const char* searchtext, short page)
{
	int len = NAME_LENGTH + 16;

	if( CheckForCharServer() )
		return 0;

	WFIFOHEAD(inter_fd,len);
	WFIFOW(inter_fd,0) = 0x3050;
	WFIFOW(inter_fd,2) = len;
	WFIFOL(inter_fd,4) = char_id;
	WFIFOW(inter_fd,8) = type;
	WFIFOL(inter_fd,10) = price; //min price for search
	WFIFOW(inter_fd,14) = page;
	memcpy(WFIFOP(inter_fd,16), searchtext, NAME_LENGTH);
	WFIFOSET(inter_fd,len);

	return 1;
}

/**
 * Received a list of auction, display them
 * @param fd : Char-serv link
 */
static void intif_parse_Auction_results(int fd)
{
	struct map_session_data *sd = map_charid2sd(RFIFOL(fd,4));
	short count = RFIFOW(fd,8);
	short pages = RFIFOW(fd,10);
	uint8* data = RFIFOP(fd,12);

	if( sd == NULL )
		return;

	clif_Auction_results(sd, count, pages, data);
}

/**
 * Register an auction to char-serv
 * @param auction : tmp auction to register
 * @return 0=error, 1=msg sent
 */
int intif_Auction_register(struct auction_data *auction)
{
	int len = sizeof(struct auction_data) + 4;

	if( CheckForCharServer() )
		return 0;

	WFIFOHEAD(inter_fd,len);
	WFIFOW(inter_fd,0) = 0x3051;
	WFIFOW(inter_fd,2) = len;
	memcpy(WFIFOP(inter_fd,4), auction, sizeof(struct auction_data));
	WFIFOSET(inter_fd,len);

	return 1;
}

/**
 * Receive a auction available from char-serv
 * @param fd : char-serv link
 */
static void intif_parse_Auction_register(int fd)
{
	struct map_session_data *sd;
	struct auction_data auction;

	if( RFIFOW(fd,2) - 4 != sizeof(struct auction_data) )
	{
		ShowError("intif_parse_Auction_register: data size error %d %d\n", RFIFOW(fd,2) - 4, sizeof(struct auction_data));
		return;
	}

	memcpy(&auction, RFIFOP(fd,4), sizeof(struct auction_data));
	if( (sd = map_charid2sd(auction.seller_id)) == NULL )
		return;

	if( auction.auction_id > 0 )
	{
		clif_Auction_message(sd->fd, 1); // Confirmation Packet ??
		if( save_settings&CHARSAVE_AUCTION )
			chrif_save(sd,0);
	}
	else
	{
		int zeny = auction.hours*battle_config.auction_feeperhour;

		clif_Auction_message(sd->fd, 4);
		pc_additem(sd, &auction.item, auction.item.amount, LOG_TYPE_AUCTION);

		pc_getzeny(sd, zeny, LOG_TYPE_AUCTION, NULL);
	}
}

/**
 * Inform char-serv that the auction is cancelled
 * @param char_id : player that has cancel the auction
 * @param auction_id : auction to cancel
 * @return 0=error, 1=msg sent
 */
int intif_Auction_cancel(uint32 char_id, unsigned int auction_id)
{
	if( CheckForCharServer() )
		return 0;

	WFIFOHEAD(inter_fd,10);
	WFIFOW(inter_fd,0) = 0x3052;
	WFIFOL(inter_fd,2) = char_id;
	WFIFOL(inter_fd,6) = auction_id;
	WFIFOSET(inter_fd,10);

	return 1;
}

/**
 * Receive a notification that the auction was cancelled
 * @param fd : char-serv link
 */
static void intif_parse_Auction_cancel(int fd)
{
	struct map_session_data *sd = map_charid2sd(RFIFOL(fd,2));
	int result = RFIFOB(fd,6);

	if( sd == NULL )
		return;

	switch( result )
	{
	case 0: clif_Auction_message(sd->fd, 2); break;
	case 1: clif_Auction_close(sd->fd, 2); break;
	case 2: clif_Auction_close(sd->fd, 1); break;
	case 3: clif_Auction_message(sd->fd, 3); break;
	}
}

/**
 * Inform the char-serv that the auction has ended
 * @param char_id : player that stop the auction
 * @param auction_id : auction to stop
 * @return 0=error, 1=msg sent
 */
int intif_Auction_close(uint32 char_id, unsigned int auction_id)
{
	if( CheckForCharServer() )
		return 0;

	WFIFOHEAD(inter_fd,10);
	WFIFOW(inter_fd,0) = 0x3053;
	WFIFOL(inter_fd,2) = char_id;
	WFIFOL(inter_fd,6) = auction_id;
	WFIFOSET(inter_fd,10);

	return 1;
}

/**
 * Receive a notification that the auction has ended
 * @param fd : char-serv link
 */
static void intif_parse_Auction_close(int fd)
{
	struct map_session_data *sd = map_charid2sd(RFIFOL(fd,2));
	unsigned char result = RFIFOB(fd,6);

	if( sd == NULL )
		return;

	clif_Auction_close(sd->fd, result);
	if( result == 0 )
	{
		// FIXME: Leeching off a parse function
		clif_parse_Auction_cancelreg(fd, sd);
		intif_Auction_requestlist(sd->status.char_id, 6, 0, "", 1);
	}
}

/**
 * Bid for an auction
 * @param char_id
 * @param name
 * @param auction_id
 * @param bid
 * @return 0=error, 1=msg sent
 */
int intif_Auction_bid(uint32 char_id, const char* name, unsigned int auction_id, int bid)
{
	int len = 16 + NAME_LENGTH;

	if( CheckForCharServer() )
		return 0;

	WFIFOHEAD(inter_fd,len);
	WFIFOW(inter_fd,0) = 0x3055;
	WFIFOW(inter_fd,2) = len;
	WFIFOL(inter_fd,4) = char_id;
	WFIFOL(inter_fd,8) = auction_id;
	WFIFOL(inter_fd,12) = bid;
	memcpy(WFIFOP(inter_fd,16), name, NAME_LENGTH);
	WFIFOSET(inter_fd,len);

	return 1;
}

/**
 * Get back the money from biding auction, 
 * (someone else have bid it over)
 * @param fd : char-serv link
 */
static void intif_parse_Auction_bid(int fd)
{
	struct map_session_data *sd = map_charid2sd(RFIFOL(fd,2));
	int bid = RFIFOL(fd,6);
	unsigned char result = RFIFOB(fd,10);

	if( sd == NULL )
		return;

	clif_Auction_message(sd->fd, result);
	if( bid > 0 )
	{
		pc_getzeny(sd, bid, LOG_TYPE_AUCTION,NULL);
	}
	if( result == 1 )
	{ // To update the list, display your buy list
		clif_parse_Auction_cancelreg(fd, sd);
		intif_Auction_requestlist(sd->status.char_id, 7, 0, "", 1);
	}
}

/**
 * Used to send 'You have won the auction' and 'You failed to won the auction' messages
 * @param fd : char-serv link
 */
static void intif_parse_Auction_message(int fd)
{
	struct map_session_data *sd = map_charid2sd(RFIFOL(fd,2));
	unsigned char result = RFIFOB(fd,6);

	if( sd == NULL )
		return;

	clif_Auction_message(sd->fd, result);
}

/*==========================================
 * Mercenary's System
 *------------------------------------------*/

/**
 * Request to create/register a mercenary on char-serv
 * @param merc : Tmp mercenary data
 * @return 0=error, 1=msg sent
 */
int intif_mercenary_create(struct s_mercenary *merc)
{
	int size = sizeof(struct s_mercenary) + 4;

	if( CheckForCharServer() )
		return 0;

	WFIFOHEAD(inter_fd,size);
	WFIFOW(inter_fd,0) = 0x3070;
	WFIFOW(inter_fd,2) = size;
	memcpy(WFIFOP(inter_fd,4), merc, sizeof(struct s_mercenary));
	WFIFOSET(inter_fd,size);
	return 1;
}

/**
 * Ack of a load or create request
 * @param fd : char-serv link
 * @return 0=error, 1=success
 */
int intif_parse_mercenary_received(int fd)
{
	int len = RFIFOW(fd,2) - 5;
	if( sizeof(struct s_mercenary) != len )
	{
		if( battle_config.etc_log )
			ShowError("intif: create mercenary data size error %d != %d\n", sizeof(struct s_mercenary), len);
		return 0;
	}

	mercenary_recv_data((struct s_mercenary*)RFIFOP(fd,5), RFIFOB(fd,4));
	return 1;
}

/**
 * Request mercenary data from char-serv
 * @param merc_id : mercenary id to load
 * @param char_id : player cid requesting data
 * @return 0=error, 1=msg sent
 */
int intif_mercenary_request(int merc_id, uint32 char_id)
{
	if (CheckForCharServer())
		return 0;

	WFIFOHEAD(inter_fd,10);
	WFIFOW(inter_fd,0) = 0x3071;
	WFIFOL(inter_fd,2) = merc_id;
	WFIFOL(inter_fd,6) = char_id;
	WFIFOSET(inter_fd,10);
	return 1;
}

/**
 * Request to delete a mercenary
 * @param merc_id
 * @return 0=error, 1=msg sent
 */
int intif_mercenary_delete(int merc_id)
{
	if (CheckForCharServer())
		return 0;

	WFIFOHEAD(inter_fd,6);
	WFIFOW(inter_fd,0) = 0x3072;
	WFIFOL(inter_fd,2) = merc_id;
	WFIFOSET(inter_fd,6);
	return 1;
}

/**
 * Ack of a mercenary deletion request
 * @param fd : char-serv link
 * @return 1
 */
int intif_parse_mercenary_deleted(int fd)
{
	if( RFIFOB(fd,2) != 1 )
		ShowError("Mercenary data delete failure\n");

	return 1;
}

/**
 * Request to save a mercenary
 * @param merc : Mercenary struct to save
 * @return 0=error, 1=msg sent
 */
int intif_mercenary_save(struct s_mercenary *merc)
{
	int size = sizeof(struct s_mercenary) + 4;

	if( CheckForCharServer() )
		return 0;

	WFIFOHEAD(inter_fd,size);
	WFIFOW(inter_fd,0) = 0x3073;
	WFIFOW(inter_fd,2) = size;
	memcpy(WFIFOP(inter_fd,4), merc, sizeof(struct s_mercenary));
	WFIFOSET(inter_fd,size);
	return 1;
}

/**
 * Ack of a mercenary save request
 * @param fd : char-serv link
 * @return 1
 */
int intif_parse_mercenary_saved(int fd)
{
	if( RFIFOB(fd,2) != 1 )
		ShowError("Mercenary data save failure\n");

	return 1;
}

/*==========================================
 * Elemental's System
 *------------------------------------------*/

/**
 * Request to create elemental, (verify and save on char-serv)
 * @param ele : Tmp Elemental data 
 * @return 0=error, 1=msg sent
 */
int intif_elemental_create(struct s_elemental *ele)
{
	int size = sizeof(struct s_elemental) + 4;

	if( CheckForCharServer() )
		return 0;

	WFIFOHEAD(inter_fd,size);
	WFIFOW(inter_fd,0) = 0x307c;
	WFIFOW(inter_fd,2) = size;
	memcpy(WFIFOP(inter_fd,4), ele, sizeof(struct s_elemental));
	WFIFOSET(inter_fd,size);
	return 1;
}

/**
 * Receive an elemental data from char-serv
 * @param fd : char-serv link
 * @return 0=error, 1=success
 */
int intif_parse_elemental_received(int fd)
{
	int len = RFIFOW(fd,2) - 5;
	if( sizeof(struct s_elemental) != len )
	{
		if( battle_config.etc_log )
			ShowError("intif: create elemental data size error %d != %d\n", sizeof(struct s_elemental), len);
		return 0;
	}

	elemental_data_received((struct s_elemental*)RFIFOP(fd,5), RFIFOB(fd,4));
	return 1;
}

/**
 * Request to load elemental from char-serv
 * @param ele_id : elemental identification
 * @param char_id : player identification
 * @return 0=error, 1=msg sent
 */
int intif_elemental_request(int ele_id, uint32 char_id)
{
	if (CheckForCharServer())
		return 0;

	WFIFOHEAD(inter_fd,10);
	WFIFOW(inter_fd,0) = 0x307d;
	WFIFOL(inter_fd,2) = ele_id;
	WFIFOL(inter_fd,6) = char_id;
	WFIFOSET(inter_fd,10);
	return 1;
}

/**
 * Request to delete an elemental
 * @param ele_id : Elemental to delete
 * @return 0=error, 1=msg sent
 */
int intif_elemental_delete(int ele_id)
{
	if (CheckForCharServer())
		return 0;

	WFIFOHEAD(inter_fd,6);
	WFIFOW(inter_fd,0) = 0x307e;
	WFIFOL(inter_fd,2) = ele_id;
	WFIFOSET(inter_fd,6);
	return 1;
}

/**
 * Ack of a delete elemental request
 * @param fd : char-serv link
 * @return 1
 */
int intif_parse_elemental_deleted(int fd)
{
	if( RFIFOB(fd,2) != 1 )
		ShowError("Elemental data delete failure\n");

	return 1;
}

/**
 * Request to save elemental data
 * @param ele : elemental struct to save
 * @return 0=error, 1=msg sent
 */
int intif_elemental_save(struct s_elemental *ele)
{
	int size = sizeof(struct s_elemental) + 4;

	if( CheckForCharServer() )
		return 0;

	WFIFOHEAD(inter_fd,size);
	WFIFOW(inter_fd,0) = 0x307f;
	WFIFOW(inter_fd,2) = size;
	memcpy(WFIFOP(inter_fd,4), ele, sizeof(struct s_elemental));
	WFIFOSET(inter_fd,size);
	return 1;
}

/**
 * Ack of a save elemental request
 * @param fd : char-serv link
 * @return 1
 */
int intif_parse_elemental_saved(int fd)
{
	if( RFIFOB(fd,2) != 1 )
		ShowError("Elemental data save failure\n");

	return 1;
}

/**
 * Request account information to char-serv
 * @param u_fd : Player fd (for message back)
 * @param aid : requesting player aid
 * @param group_lv : requesting player lv
 * @param query : name or aid of player we want info
 * @param type : 1 - Only return account id & userid, 0 - Full info
 * @return : 0=errur, 1=msg sent
 */
int intif_request_accinfo(int u_fd, int aid, int group_lv, char* query, char type) {

	if( CheckForCharServer() )
		return 0;
	
	WFIFOHEAD(inter_fd,2 + 4 + 4 + 4 + 1 + NAME_LENGTH);

	WFIFOW(inter_fd,0) = 0x3007;
	WFIFOL(inter_fd,2) = u_fd;
	WFIFOL(inter_fd,6) = aid;
	WFIFOL(inter_fd,10) = group_lv;
	WFIFOB(inter_fd,14) = type;
	safestrncpy((char *)WFIFOP(inter_fd,15), query, NAME_LENGTH);

	WFIFOSET(inter_fd,2 + 4 + 4 + 4 + 1 + NAME_LENGTH);
	return 1;
}

/**
 * Receive the reply of a request_accinfo with type 1
 * @param fd : char-serv link
 */
void intif_parse_accinfo_ack( int fd ) {
	char acc_name[NAME_LENGTH];
	int u_fd = RFIFOL(fd,2);
	int acc_id = RFIFOL(fd,6);
	safestrncpy(acc_name, (char*)RFIFOP(fd,10), NAME_LENGTH);
	clif_account_name(u_fd, acc_id, acc_name);
}

/**
 * Display a message from char-serv to a player
 * @param fd : Char-serv link
 */
void intif_parse_MessageToFD(int fd) {
	int u_fd = RFIFOL(fd,4);

	if( session[u_fd] && session[u_fd]->session_data ) { //check if the player still online
		int aid = RFIFOL(fd,8);
		struct map_session_data * sd = (struct map_session_data *)session[u_fd]->session_data;
		/* matching e.g. previous fd owner didn't dc during request or is still the same */
		if( sd->bl.id == aid ) {
			char msg[512];
			safestrncpy(msg, (char*)RFIFOP(fd,12), RFIFOW(fd,2) - 12);
			clif_displaymessage(u_fd,msg);
		}

	}

	return;
}

/// BROADCAST OBTAIN SPECIAL ITEM

/**
 * Request to send broadcast item to all servers
 * ZI 3009 <cmd>.W <len>.W <nameid>.W <source>.W <type>.B <name>.?B
 * @param sd Player who obtain the item
 * @param nameid Obtained item
 * @param sourceid Source of item, another item ID or monster ID
 * @param type Obtain type @see enum BROADCASTING_SPECIAL_ITEM_OBTAIN
 * @return
 **/
int intif_broadcast_obtain_special_item(struct map_session_data *sd, unsigned short nameid, unsigned int sourceid, unsigned char type) {
	nullpo_retr(0, sd);

	// Should not be here!
	if (type == ITEMOBTAIN_TYPE_NPC) {
		intif_broadcast_obtain_special_item_npc(sd, nameid, NULL /*wisp_server_name*/);
		return 0;
	}

	// Send local
	clif_broadcast_obtain_special_item(sd->status.name, nameid, sourceid, (enum BROADCASTING_SPECIAL_ITEM_OBTAIN)type, NULL);

	if (CheckForCharServer())
		return 0;

	if (other_mapserver_count < 1)
		return 0;

	WFIFOHEAD(inter_fd, 9 + NAME_LENGTH);
	WFIFOW(inter_fd, 0) = 0x3009;
	WFIFOW(inter_fd, 2) = 9 + NAME_LENGTH;
	WFIFOW(inter_fd, 4) = nameid;
	WFIFOW(inter_fd, 6) = sourceid;
	WFIFOB(inter_fd, 8) = type;
	safestrncpy((char *)WFIFOP(inter_fd, 9), sd->status.name, NAME_LENGTH);
	WFIFOSET(inter_fd, WFIFOW(inter_fd, 2));

	return 1;
}

/**
 * Request to send broadcast item to all servers.
 * TODO: Confirm the usage. Maybe on getitem-like command?
 * ZI 3009 <cmd>.W <len>.W <nameid>.W <source>.W <type>.B <name>.24B <npcname>.24B
 * @param sd Player who obtain the item
 * @param nameid Obtained item
 * @param srcname Source name
 * @return
 **/
int intif_broadcast_obtain_special_item_npc(struct map_session_data *sd, unsigned short nameid, const char *srcname) {
	nullpo_retr(0, sd);

	// Send local
	clif_broadcast_obtain_special_item(sd->status.name, nameid, 0, ITEMOBTAIN_TYPE_NPC, srcname);

	if (CheckForCharServer())
		return 0;

	if (other_mapserver_count < 1)
		return 0;

	WFIFOHEAD(inter_fd, 9 + NAME_LENGTH*2);
	WFIFOW(inter_fd, 0) = 0x3009;
	WFIFOW(inter_fd, 2) = 9 + NAME_LENGTH*2;
	WFIFOW(inter_fd, 4) = nameid;
	WFIFOW(inter_fd, 6) = 0;
	WFIFOB(inter_fd, 8) = ITEMOBTAIN_TYPE_NPC;
	safestrncpy((char *)WFIFOP(inter_fd, 9), sd->status.name, NAME_LENGTH);
	safestrncpy((char *)WFIFOP(inter_fd, 9 + NAME_LENGTH), srcname, NAME_LENGTH);
	WFIFOSET(inter_fd, WFIFOW(inter_fd, 2));

	return 1;
}

/**
 * Received broadcast item and broadcast on local map.
 * IZ 3809 <cmd>.W <len>.W <nameid>.W <source>.W <type>.B <name>.24B <srcname>.24B
 * @param fd
 **/
void intif_parse_broadcast_obtain_special_item(int fd) {
	int type = RFIFOB(fd, 8);
	char name[NAME_LENGTH], srcname[NAME_LENGTH];

	safestrncpy(name, (char *)RFIFOP(fd, 9), NAME_LENGTH);
	if (type == ITEMOBTAIN_TYPE_NPC)
		safestrncpy(name, (char *)RFIFOP(fd, 9 + NAME_LENGTH), NAME_LENGTH);

	clif_broadcast_obtain_special_item(name, RFIFOW(fd, 4), RFIFOW(fd, 6), (enum BROADCASTING_SPECIAL_ITEM_OBTAIN)type, srcname);
}

/*==========================================
 * Item Bound System
 *------------------------------------------*/

#ifdef BOUND_ITEMS

/**
 * ZI 0x3056 <char_id>.L <account_id>.L <guild_id>.W
 * Request inter-serv to delete some bound item, for non connected cid
 * @param char_id : Char to delete item ID
 * @param aid : Account to delete item ID
 * @param guild_id : Guild of char
 */
void intif_itembound_guild_retrieve(uint32 char_id,uint32 account_id,int guild_id) {
	struct guild_storage *gstor = gstorage_get_storage(guild_id);
	
	if( CheckForCharServer() )
		return;
	
	WFIFOHEAD(inter_fd,12);
	WFIFOW(inter_fd,0) = 0x3056;
	WFIFOL(inter_fd,2) = char_id;
	WFIFOL(inter_fd,6) = account_id;
	WFIFOW(inter_fd,10) = guild_id;
	WFIFOSET(inter_fd,12);
	if (gstor)
		gstor->locked = true; //Lock for retrieval process
	ShowInfo("Request guild bound item(s) retrieval for CID = "CL_WHITE"%d"CL_RESET", AID = %d, Guild ID = "CL_WHITE"%d"CL_RESET".\n", char_id, account_id, guild_id);
}

/**
 * Acknowledge the good deletion of the bound item
 * (unlock the guild storage)
 * @struct : 0x3856 <aid>.L <gid>.W
 * @param fd : Char-serv link
 */
void intif_parse_itembound_ack(int fd) {
	int guild_id = RFIFOW(fd,6);
	struct guild_storage *gstor = gstorage_get_storage(guild_id);
	if (gstor)
		gstor->locked = false; //Unlock now that operation is completed
}

/**
* IZ 0x3857 <size>.W <count>.W <guild_id>.W { <item>.?B }.*MAX_INVENTORY
* Received the retrieved guild bound items from inter-server, store them to guild storage.
* @param fd
* @author [Cydh]
*/
void intif_parse_itembound_store2gstorage(int fd) {
	unsigned short i, failed = 0;
	short count = RFIFOW(fd, 4), guild_id = RFIFOW(fd, 6);
	struct guild_storage *gstor = gstorage_guild2storage(guild_id);

	if (!gstor) {
		ShowError("intif_parse_itembound_store2gstorage: Guild '%d' not found.\n", guild_id);
		return;
	}

	//@TODO: Gives some actions for item(s) that cannot be stored because storage is full or reach the limit of stack amount
	for (i = 0; i < count; i++) {
		struct item *item = (struct item*)RFIFOP(fd, 8 + i*sizeof(struct item));
		if (!item)
			continue;
		if (!gstorage_additem2(gstor, item, item->amount))
			failed++;
	}
	ShowInfo("Retrieved '"CL_WHITE"%d"CL_RESET"' (failed: %d) guild bound item(s) for Guild ID = "CL_WHITE"%d"CL_RESET".\n", count, failed, guild_id);
	gstor->locked = false;
	gstor->opened = 0;
}
#endif

//-----------------------------------------------------------------

/**
 * Communication from the inter server, Main entry point interface (inter<=>map) 
 * @param fd : inter-serv link
 * @return
 *  0 (unknow packet).
 *  1 sucess (no error)
 *  2 invalid lenght of packet (not enough data yet)
 */
int intif_parse(int fd)
{
	int packet_len, cmd;
	cmd = RFIFOW(fd,0);
	// Verify ID of the packet
	if(cmd<0x3800 || cmd>=0x3800+ARRAYLENGTH(packet_len_table) ||
		packet_len_table[cmd-0x3800]==0){
		return 0;
	}
	// Check the length of the packet
	packet_len = packet_len_table[cmd-0x3800];
	if(packet_len==-1){
		if(RFIFOREST(fd)<4)
			return 2;
		packet_len = RFIFOW(fd,2);
	}
	if((int)RFIFOREST(fd)<packet_len){
		return 2;
	}
	// Processing branch
	switch(cmd){
	case 0x3800:
		if (RFIFOL(fd,4) == 0xFF000000) //Normal announce.
			clif_broadcast(NULL, (char *) RFIFOP(fd,16), packet_len-16, BC_DEFAULT, ALL_CLIENT);
		else //Color announce.
			clif_broadcast2(NULL, (char *) RFIFOP(fd,16), packet_len-16, RFIFOL(fd,4), RFIFOW(fd,8), RFIFOW(fd,10), RFIFOW(fd,12), RFIFOW(fd,14), ALL_CLIENT);
		break;
	case 0x3801:	intif_parse_WisMessage(fd); break;
	case 0x3802:	intif_parse_WisEnd(fd); break;
	case 0x3803:	mapif_parse_WisToGM(fd); break;
	case 0x3804:	intif_parse_Registers(fd); break;
	case 0x3806:	intif_parse_ChangeNameOk(fd); break;
	case 0x3807:	intif_parse_MessageToFD(fd); break;
	case 0x3808:	intif_parse_accinfo_ack(fd); break;
	case 0x3809:	intif_parse_broadcast_obtain_special_item(fd); break;
	case 0x3818:	intif_parse_LoadGuildStorage(fd); break;
	case 0x3819:	intif_parse_SaveGuildStorage(fd); break;
	case 0x3820:	intif_parse_PartyCreated(fd); break;
	case 0x3821:	intif_parse_PartyInfo(fd); break;
	case 0x3822:	intif_parse_PartyMemberAdded(fd); break;
	case 0x3823:	intif_parse_PartyOptionChanged(fd); break;
	case 0x3824:	intif_parse_PartyMemberWithdraw(fd); break;
	case 0x3825:	intif_parse_PartyMove(fd); break;
	case 0x3826:	intif_parse_PartyBroken(fd); break;
	case 0x3827:	intif_parse_PartyMessage(fd); break;
	case 0x3830:	intif_parse_GuildCreated(fd); break;
	case 0x3831:	intif_parse_GuildInfo(fd); break;
	case 0x3832:	intif_parse_GuildMemberAdded(fd); break;
	case 0x3834:	intif_parse_GuildMemberWithdraw(fd); break;
	case 0x3835:	intif_parse_GuildMemberInfoShort(fd); break;
	case 0x3836:	intif_parse_GuildBroken(fd); break;
	case 0x3837:	intif_parse_GuildMessage(fd); break;
	case 0x3839:	intif_parse_GuildBasicInfoChanged(fd); break;
	case 0x383a:	intif_parse_GuildMemberInfoChanged(fd); break;
	case 0x383b:	intif_parse_GuildPosition(fd); break;
	case 0x383c:	intif_parse_GuildSkillUp(fd); break;
	case 0x383d:	intif_parse_GuildAlliance(fd); break;
	case 0x383e:	intif_parse_GuildNotice(fd); break;
	case 0x383f:	intif_parse_GuildEmblem(fd); break;
	case 0x3840:	intif_parse_GuildCastleDataLoad(fd); break;
	case 0x3843:	intif_parse_GuildMasterChanged(fd); break;

	// Mail System
	case 0x3848:	intif_parse_Mail_inboxreceived(fd); break;
	case 0x3849:	intif_parse_Mail_new(fd); break;
	case 0x384a:	intif_parse_Mail_getattach(fd); break;
	case 0x384b:	intif_parse_Mail_delete(fd); break;
	case 0x384c:	intif_parse_Mail_return(fd); break;
	case 0x384d:	intif_parse_Mail_send(fd); break;

	// Auction System
	case 0x3850:	intif_parse_Auction_results(fd); break;
	case 0x3851:	intif_parse_Auction_register(fd); break;
	case 0x3852:	intif_parse_Auction_cancel(fd); break;
	case 0x3853:	intif_parse_Auction_close(fd); break;
	case 0x3854:	intif_parse_Auction_message(fd); break;
	case 0x3855:	intif_parse_Auction_bid(fd); break;

	//Bound items
#ifdef BOUND_ITEMS
	case 0x3856:	intif_parse_itembound_ack(fd); break;
	case 0x3857:	intif_parse_itembound_store2gstorage(fd); break;
#endif

	//Quest system
	case 0x3860:	intif_parse_questlog(fd); break;
	case 0x3861:	intif_parse_questsave(fd); break;

	// Mercenary System
	case 0x3870:	intif_parse_mercenary_received(fd); break;
	case 0x3871:	intif_parse_mercenary_deleted(fd); break;
	case 0x3872:	intif_parse_mercenary_saved(fd); break;

	// Elemental System
	case 0x387c:	intif_parse_elemental_received(fd); break;
	case 0x387d:	intif_parse_elemental_deleted(fd); break;
	case 0x387e:	intif_parse_elemental_saved(fd); break;

	// Pet System
	case 0x3880:	intif_parse_CreatePet(fd); break;
	case 0x3881:	intif_parse_RecvPetData(fd); break;
	case 0x3882:	intif_parse_SavePetOk(fd); break;
	case 0x3883:	intif_parse_DeletePetOk(fd); break;

	// Homunculus
	case 0x3890:	intif_parse_CreateHomunculus(fd); break;
	case 0x3891:	intif_parse_RecvHomunculusData(fd); break;
	case 0x3892:	intif_parse_SaveHomunculusOk(fd); break;
	case 0x3893:	intif_parse_DeleteHomunculusOk(fd); break;

	default:
		ShowError("intif_parse : unknown packet %d %x\n",fd,RFIFOW(fd,0));
		return 0;
	}
	// Skip packet
	RFIFOSKIP(fd,packet_len);
	return 1;
}
