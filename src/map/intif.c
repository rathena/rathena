// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/showmsg.h"
#include "../common/socket.h"
#include "../common/timer.h"
#include "../common/nullpo.h"
#include "../common/malloc.h"
#include "../common/strlib.h"
#include "map.h"
#include "battle.h"
#include "chrif.h"
#include "clif.h"
#include "pc.h"
#include "intif.h"
#include "storage.h"
#include "party.h"
#include "guild.h"
#include "pet.h"
#include "atcommand.h"
#include "mercenary.h" //albator
#include "mail.h"

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>
#include <string.h>


static const int packet_len_table[]={
	-1,-1,27,-1, -1, 0,37, 0,  0, 0, 0, 0,  0, 0,  0, 0, //0x3800-0x380f
	-1, 7, 0, 0,  0, 0, 0, 0, -1,11, 0, 0,  0, 0,  0, 0, //0x3810
	39,-1,15,15, 14,19, 7,-1,  0, 0, 0, 0,  0, 0,  0, 0, //0x3820
	10,-1,15, 0, 79,19, 7,-1,  0,-1,-1,-1, 14,67,186,-1, //0x3830
	 9, 9,-1,14,  0, 0, 0, 0, -1,74,-1,11, 11,-1,  0, 0, //0x3840
	-1,-1, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0,  0, 0, //0x3850  Auctions [Zephyrus]
	 0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0,  0, 0,
	 0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0,  0, 0,
	11,-1, 7, 3,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0,  0, 0, //0x3880
	-1,-1, 7, 3,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0,  0, 0, //0x3890  Homunculus [albator]
};

extern int char_fd;		// inter serverのfdはchar_fdを使う
#define inter_fd char_fd	// エイリアス

//-----------------------------------------------------------------
// inter serverへの送信

int CheckForCharServer(void)
{
	return ((char_fd <= 0) || session[char_fd] == NULL || session[char_fd]->wdata == NULL);
}

// pet
int intif_create_pet(int account_id,int char_id,short pet_class,short pet_lv,short pet_egg_id,
	short pet_equip,short intimate,short hungry,char rename_flag,char incuvate,char *pet_name)
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
	WFIFOB(inter_fd,23) = incuvate;
	memcpy(WFIFOP(inter_fd,24),pet_name,NAME_LENGTH);
	WFIFOSET(inter_fd,24+NAME_LENGTH);

	return 0;
}

int intif_request_petdata(int account_id,int char_id,int pet_id)
{
	if (CheckForCharServer())
		return 0;
	WFIFOHEAD(inter_fd, 14);
	WFIFOW(inter_fd,0) = 0x3081;
	WFIFOL(inter_fd,2) = account_id;
	WFIFOL(inter_fd,6) = char_id;
	WFIFOL(inter_fd,10) = pet_id;
	WFIFOSET(inter_fd,14);

	return 0;
}

int intif_save_petdata(int account_id,struct s_pet *p)
{
	if (CheckForCharServer())
		return 0;
	WFIFOHEAD(inter_fd, sizeof(struct s_pet) + 8);
	WFIFOW(inter_fd,0) = 0x3082;
	WFIFOW(inter_fd,2) = sizeof(struct s_pet) + 8;
	WFIFOL(inter_fd,4) = account_id;
	memcpy(WFIFOP(inter_fd,8),p,sizeof(struct s_pet));
	WFIFOSET(inter_fd,WFIFOW(inter_fd,2));

	return 0;
}

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

int intif_rename(struct map_session_data *sd, int type, char *name)
{
	if (CheckForCharServer())
		return 1;

	WFIFOHEAD(inter_fd,NAME_LENGTH+12);
	WFIFOW(inter_fd,0) = 0x3006;
	WFIFOL(inter_fd,2) = sd->status.account_id;
	WFIFOL(inter_fd,6) = sd->status.char_id;
	WFIFOB(inter_fd,10) = type;  //Type: 0 - PC, 1 - PET, 2 - HOM
	memcpy(WFIFOP(inter_fd,11),name, NAME_LENGTH);
	WFIFOSET(inter_fd,NAME_LENGTH+12);
	return 0;
}

// GMメッセージを送信
int intif_GMmessage(const char* mes,int len,int flag)
{
	int lp = (flag&0x10) ? 8 : 4;

	// Send to the local players
	clif_GMmessage(NULL, mes, len, flag);

	if (CheckForCharServer())
		return 0;

	if (other_mapserver_count < 1)
		return 0; //No need to send.

	WFIFOHEAD(inter_fd,lp + len + 4);
	WFIFOW(inter_fd,0) = 0x3000;
	WFIFOW(inter_fd,2) = lp + len + 4;
	WFIFOL(inter_fd,4) = 0xFF000000; //"invalid" color signals standard broadcast.
	WFIFOL(inter_fd,8) = 0x65756c62;
	memcpy(WFIFOP(inter_fd,4+lp), mes, len);
	WFIFOSET(inter_fd, WFIFOW(inter_fd,2));
	return 0;
}

int intif_announce(const char* mes,int len, unsigned long color, int flag)
{
	// Send to the local players
	if(color == 0xFE000000) // This is main chat message [LuzZza]
		clif_MainChatMessage(mes);
	else
		clif_announce(NULL, mes, len, color, flag);

	if (CheckForCharServer())
		return 0;

	if (other_mapserver_count < 1)
		return 0; //No need to send.

	WFIFOHEAD(inter_fd, 8 + len);
	WFIFOW(inter_fd,0) = 0x3000;
	WFIFOW(inter_fd,2) = 8 + len;
	WFIFOL(inter_fd,4) = color;
	memcpy(WFIFOP(inter_fd,8), mes, len);
	WFIFOSET(inter_fd, WFIFOW(inter_fd,2));
	return 0;
}

// The transmission of Wisp/Page to inter-server (player not found on this server)
int intif_wis_message(struct map_session_data *sd, char *nick, char *mes, int mes_len)
{
	nullpo_retr(0, sd);
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

	return 0;
}

// The reply of Wisp/page
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

	return 0;
}

// The transmission of GM only Wisp/Page from server to inter-server
int intif_wis_message_to_gm(char *Wisp_name, int min_gm_level, char *mes)
{
	int mes_len;
	if (CheckForCharServer())
		return 0;
	mes_len = strlen(mes) + 1; // + null
	WFIFOHEAD(inter_fd, mes_len + 30);
	WFIFOW(inter_fd,0) = 0x3003;
	WFIFOW(inter_fd,2) = mes_len + 30;
	memcpy(WFIFOP(inter_fd,4), Wisp_name, NAME_LENGTH);
	WFIFOW(inter_fd,4+NAME_LENGTH) = (short)min_gm_level;
	memcpy(WFIFOP(inter_fd,6+NAME_LENGTH), mes, mes_len);
	WFIFOSET(inter_fd, WFIFOW(inter_fd,2));

	if (battle_config.etc_log)
		ShowNotice("intif_wis_message_to_gm: from: '%s', min level: %d, message: '%s'.\n", Wisp_name, min_gm_level, mes);

	return 0;
}

int intif_regtostr(char* str, struct global_reg *reg, int qty)
{
	int len =0, i;

	for (i = 0; i < qty; i++) {
		len+= sprintf(str+len, "%s", reg[i].str)+1; //We add 1 to consider the '\0' in place.
		len+= sprintf(str+len, "%s", reg[i].value)+1;
	}
	return len;
}

//Request for saving registry values.
int intif_saveregistry(struct map_session_data *sd, int type)
{
	struct global_reg *reg;
	int count;
	int i, p;

	if (CheckForCharServer())
		return -1;

	switch (type) {
	case 3: //Character reg
		reg = sd->save_reg.global;
		count = sd->save_reg.global_num;
		sd->state.reg_dirty &= ~0x4;
	break;
	case 2: //Account reg
		reg = sd->save_reg.account;
		count = sd->save_reg.account_num;
		sd->state.reg_dirty &= ~0x2;
	break;
	case 1: //Account2 reg
		reg = sd->save_reg.account2;
		count = sd->save_reg.account2_num;
		sd->state.reg_dirty &= ~0x1;
	break;
	default: //Broken code?
		ShowError("intif_saveregistry: Invalid type %d\n", type);
		return -1;
	}
	WFIFOHEAD(inter_fd, 288 * MAX_REG_NUM+13);
	WFIFOW(inter_fd,0)=0x3004;
	WFIFOL(inter_fd,4)=sd->status.account_id;
	WFIFOL(inter_fd,8)=sd->status.char_id;
	WFIFOB(inter_fd,12)=type;
	for( p = 13, i = 0; i < count; i++ ) {
		if (reg[i].str[0] && reg[i].value != 0) {
			p+= sprintf((char*)WFIFOP(inter_fd,p), "%s", reg[i].str)+1; //We add 1 to consider the '\0' in place.
			p+= sprintf((char*)WFIFOP(inter_fd,p), "%s", reg[i].value)+1;
		}
	}
	WFIFOW(inter_fd,2)=p;
	WFIFOSET(inter_fd,WFIFOW(inter_fd,2));
	return 0;
}

//Request the registries for this player.
int intif_request_registry(struct map_session_data *sd, int flag)
{
	nullpo_retr(0, sd);

	sd->save_reg.account2_num = -1;
	sd->save_reg.account_num = -1;
	sd->save_reg.global_num = -1;

	if (CheckForCharServer())
		return 0;

	WFIFOHEAD(inter_fd,6);
	WFIFOW(inter_fd,0) = 0x3005;
	WFIFOL(inter_fd,2) = sd->status.account_id;
	WFIFOL(inter_fd,6) = sd->status.char_id;
	WFIFOB(inter_fd,10) = (flag&1?1:0); //Request Acc Reg 2
	WFIFOB(inter_fd,11) = (flag&2?1:0); //Request Acc Reg
	WFIFOB(inter_fd,12) = (flag&4?1:0); //Request Char Reg
	WFIFOSET(inter_fd,13);

	return 0;
}

// 倉庫データ要求
int intif_request_storage(int account_id)
{
	if (CheckForCharServer())
		return 0;
	WFIFOHEAD(inter_fd,6);
	WFIFOW(inter_fd,0) = 0x3010;
	WFIFOL(inter_fd,2) = account_id;
	WFIFOSET(inter_fd,6);
	return 0;
}
// 倉庫データ送信
int intif_send_storage(struct storage *stor)
{
	if (CheckForCharServer())
		return 0;
	nullpo_retr(0, stor);
	WFIFOHEAD(inter_fd,sizeof(struct storage)+8);
	WFIFOW(inter_fd,0) = 0x3011;
	WFIFOW(inter_fd,2) = sizeof(struct storage)+8;
	WFIFOL(inter_fd,4) = stor->account_id;
	memcpy( WFIFOP(inter_fd,8),stor, sizeof(struct storage) );
	WFIFOSET(inter_fd,WFIFOW(inter_fd,2));
	return 0;
}

int intif_request_guild_storage(int account_id,int guild_id)
{
	if (CheckForCharServer())
		return 0;
	WFIFOHEAD(inter_fd,10);
	WFIFOW(inter_fd,0) = 0x3018;
	WFIFOL(inter_fd,2) = account_id;
	WFIFOL(inter_fd,6) = guild_id;
	WFIFOSET(inter_fd,10);
	return 0;
}
int intif_send_guild_storage(int account_id,struct guild_storage *gstor)
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
	return 0;
}

// パーティ作成要求
int intif_create_party(struct party_member *member,char *name,int item,int item2)
{
	if (CheckForCharServer())
		return 0;
	nullpo_retr(0, member);

	WFIFOHEAD(inter_fd,64);
	WFIFOW(inter_fd,0) = 0x3020;
	WFIFOW(inter_fd,2) = 30+sizeof(struct party_member);
	memcpy(WFIFOP(inter_fd,4),name, NAME_LENGTH);
	WFIFOB(inter_fd,28)= item;
	WFIFOB(inter_fd,29)= item2;
	memcpy(WFIFOP(inter_fd,30), member, sizeof(struct party_member));
	WFIFOSET(inter_fd,WFIFOW(inter_fd, 2));
	return 0;
}
// パーティ情報要求
int intif_request_partyinfo(int party_id)
{
	if (CheckForCharServer())
		return 0;
	WFIFOHEAD(inter_fd,6);
	WFIFOW(inter_fd,0) = 0x3021;
	WFIFOL(inter_fd,2) = party_id;
	WFIFOSET(inter_fd,6);
	return 0;
}
// パーティ追加要求
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
// パーティ設定変更
int intif_party_changeoption(int party_id,int account_id,int exp,int item)
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
	return 0;
}
// パーティ脱退要求
int intif_party_leave(int party_id,int account_id, int char_id)
{
	if (CheckForCharServer())
		return 0;
	WFIFOHEAD(inter_fd,14);
	WFIFOW(inter_fd,0)=0x3024;
	WFIFOL(inter_fd,2)=party_id;
	WFIFOL(inter_fd,6)=account_id;
	WFIFOL(inter_fd,10)=char_id;
	WFIFOSET(inter_fd,14);
	return 0;
}
// パーティ移動要求
int intif_party_changemap(struct map_session_data *sd,int online)
{
	if (CheckForCharServer())
		return 0;
	if(!sd)
		return 0;

	WFIFOHEAD(inter_fd,19);
	WFIFOW(inter_fd,0)=0x3025;
	WFIFOL(inter_fd,2)=sd->status.party_id;
	WFIFOL(inter_fd,6)=sd->status.account_id;
	WFIFOL(inter_fd,10)=sd->status.char_id;
	WFIFOW(inter_fd,14)=sd->mapindex;
	WFIFOB(inter_fd,16)=online;
	WFIFOW(inter_fd,17)=sd->status.base_level;
	WFIFOSET(inter_fd,19);
	return 1;
}
// パーティー解散要求
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
// パーティ会話送信
int intif_party_message(int party_id,int account_id,const char *mes,int len)
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
	return 0;
}
// パーティ競合チェック要求
int intif_party_checkconflict(int party_id,int account_id,int char_id)
{
	if (CheckForCharServer())
		return 0;
	WFIFOHEAD(inter_fd,10 + NAME_LENGTH);
	WFIFOW(inter_fd,0)=0x3028;
	WFIFOL(inter_fd,2)=party_id;
	WFIFOL(inter_fd,6)=account_id;
	WFIFOL(inter_fd,10)=char_id;
	WFIFOSET(inter_fd,14);
	return 0;
}

int intif_party_leaderchange(int party_id,int account_id,int char_id)
{
	if (CheckForCharServer())
		return 0;
	WFIFOHEAD(inter_fd,14);
	WFIFOW(inter_fd,0)=0x3029;
	WFIFOL(inter_fd,2)=party_id;
	WFIFOL(inter_fd,6)=account_id;
	WFIFOL(inter_fd,10)=char_id;
	WFIFOSET(inter_fd,14);
	return 0;
}


// ギルド作成要求
int intif_guild_create(const char *name,const struct guild_member *master)
{
	if (CheckForCharServer())
		return 0;
	nullpo_retr(0, master);

	WFIFOHEAD(inter_fd,sizeof(struct guild_member)+(8+NAME_LENGTH));
	WFIFOW(inter_fd,0)=0x3030;
	WFIFOW(inter_fd,2)=sizeof(struct guild_member)+(8+NAME_LENGTH);
	WFIFOL(inter_fd,4)=master->account_id;
	memcpy(WFIFOP(inter_fd,8),name,NAME_LENGTH);
	memcpy(WFIFOP(inter_fd,8+NAME_LENGTH),master,sizeof(struct guild_member));
	WFIFOSET(inter_fd,WFIFOW(inter_fd,2));
	return 0;
}
// ギルド情報要求
int intif_guild_request_info(int guild_id)
{
	if (CheckForCharServer())
		return 0;
	WFIFOHEAD(inter_fd,6);
	WFIFOW(inter_fd,0) = 0x3031;
	WFIFOL(inter_fd,2) = guild_id;
	WFIFOSET(inter_fd,6);
	return 0;
}
// ギルドメンバ追加要求
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
	return 0;
}

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
	return 0;
}

// ギルドメンバ脱退/追放要求
int intif_guild_leave(int guild_id,int account_id,int char_id,int flag,const char *mes)
{
	if (CheckForCharServer())
		return 0;
	WFIFOHEAD(inter_fd, 55);
	WFIFOW(inter_fd, 0) = 0x3034;
	WFIFOL(inter_fd, 2) = guild_id;
	WFIFOL(inter_fd, 6) = account_id;
	WFIFOL(inter_fd,10) = char_id;
	WFIFOB(inter_fd,14) = flag;
	safestrncpy(WFIFOP(inter_fd,15),mes,40);
	WFIFOSET(inter_fd,55);
	return 0;
}
// ギルドメンバのオンライン状況/Lv更新要求
int intif_guild_memberinfoshort(int guild_id,int account_id,int char_id,int online,int lv,int class_)
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
	return 0;
}
// ギルド解散通知
int intif_guild_break(int guild_id)
{
	if (CheckForCharServer())
		return 0;
	WFIFOHEAD(inter_fd, 6);
	WFIFOW(inter_fd, 0) = 0x3036;
	WFIFOL(inter_fd, 2) = guild_id;
	WFIFOSET(inter_fd,6);
	return 0;
}
// ギルド会話送信
int intif_guild_message(int guild_id,int account_id,const char *mes,int len)
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

	return 0;
}
// ギルド基本情報変更要求
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
	return 0;
}
// ギルドメンバ情報変更要求
int intif_guild_change_memberinfo(int guild_id,int account_id,int char_id,
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
	return 0;
}
// ギルド役職変更要求
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
	return 0;
}
// ギルドスキルアップ要求
int intif_guild_skillup(int guild_id, int skill_num, int account_id)
{
	if( CheckForCharServer() )
		return 0;
	WFIFOHEAD(inter_fd, 14);
	WFIFOW(inter_fd, 0)  = 0x303c;
	WFIFOL(inter_fd, 2)  = guild_id;
	WFIFOL(inter_fd, 6)  = skill_num;
	WFIFOL(inter_fd, 10) = account_id;
	WFIFOSET(inter_fd, 14);
	return 0;
}
// ギルド同盟/敵対要求
int intif_guild_alliance(int guild_id1,int guild_id2,int account_id1,int account_id2,int flag)
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
	return 0;
}
// ギルド告知変更要求
int intif_guild_notice(int guild_id,const char *mes1,const char *mes2)
{
	if (CheckForCharServer())
		return 0;
	WFIFOHEAD(inter_fd,186);
	WFIFOW(inter_fd,0)=0x303e;
	WFIFOL(inter_fd,2)=guild_id;
	memcpy(WFIFOP(inter_fd,6),mes1,60);
	memcpy(WFIFOP(inter_fd,66),mes2,120);
	WFIFOSET(inter_fd,186);
	return 0;
}
// ギルドエンブレム変更要求
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
	return 0;
}
//現在のギルド城占領ギルドを調べる
int intif_guild_castle_dataload(int castle_id,int index)
{
	if (CheckForCharServer())
		return 0;
	WFIFOHEAD(inter_fd,5);
	WFIFOW(inter_fd,0)=0x3040;
	WFIFOW(inter_fd,2)=castle_id;
	WFIFOB(inter_fd,4)=index;
	WFIFOSET(inter_fd,5);
	return 0;
}

//ギルド城占領ギルド変更要求
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
	return 0;
}

//-----------------------------------------------------------------
// Homunculus Packets send to Inter server [albator]
//-----------------------------------------------------------------

int intif_homunculus_create(int account_id, struct s_homunculus *sh)
{
	if (CheckForCharServer())
		return 0;
	WFIFOHEAD(inter_fd, sizeof(struct s_homunculus)+8);
	WFIFOW(inter_fd,0) = 0x3090;
	WFIFOW(inter_fd,2) = sizeof(struct s_homunculus)+8;
	WFIFOL(inter_fd,4) = account_id;
	memcpy(WFIFOP(inter_fd,8),sh,sizeof(struct s_homunculus));
	WFIFOSET(inter_fd, WFIFOW(inter_fd,2));
	return 0;
}

int intif_homunculus_requestload(int account_id, int homun_id)
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

int intif_homunculus_requestsave(int account_id, struct s_homunculus* sh)
{
	if (CheckForCharServer())
		return 0;
	WFIFOHEAD(inter_fd, sizeof(struct s_homunculus)+8);
	WFIFOW(inter_fd,0) = 0x3092;
	WFIFOW(inter_fd,2) = sizeof(struct s_homunculus)+8;
	WFIFOL(inter_fd,4) = account_id;
	memcpy(WFIFOP(inter_fd,8),sh,sizeof(struct s_homunculus));
	WFIFOSET(inter_fd, WFIFOW(inter_fd,2));
	return 0;

}

int intif_homunculus_requestdelete(int homun_id)
{
	if (CheckForCharServer())
		return 0;
	WFIFOHEAD(inter_fd, 6);
	WFIFOW(inter_fd, 0) = 0x3093;
	WFIFOL(inter_fd,2) = homun_id;
	WFIFOSET(inter_fd,6);
	return 0;

}


//-----------------------------------------------------------------
// Packets receive from inter server

// Wisp/Page reception // rewritten by [Yor]
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
	intif_wis_replay(id,0);   // 送信成功
	return 0;
}

// Wisp/page transmission result reception
int intif_parse_WisEnd(int fd)
{
	struct map_session_data* sd;

	if (battle_config.etc_log)
		ShowInfo("intif_parse_wisend: player: %s, flag: %d\n", RFIFOP(fd,2), RFIFOB(fd,26)); // flag: 0: success to send wisper, 1: target character is not loged in?, 2: ignored by target
	sd = (struct map_session_data *)map_nick2sd((char *) RFIFOP(fd,2));
	if (sd != NULL)
		clif_wis_end(sd->fd, RFIFOB(fd,26));

	return 0;
}

static int mapif_parse_WisToGM_sub(struct map_session_data* sd,va_list va)
{
	int min_gm_level = va_arg(va, int);
	char *wisp_name;
	char *message;
	int len;
	if (pc_isGM(sd) < min_gm_level) return 0;
	wisp_name = va_arg(va, char*);
	message = va_arg(va, char*);
	len = va_arg(va, int);
	clif_wis_message(sd->fd, wisp_name, message, len);
	return 1;
}

// Received wisp message from map-server via char-server for ALL gm
// 0x3003/0x3803 <packet_len>.w <wispname>.24B <min_gm_level>.w <message>.?B
int mapif_parse_WisToGM(int fd)
{
	int min_gm_level, mes_len;
	char Wisp_name[NAME_LENGTH];
	char mbuf[255];
	char *message;

	mes_len =  RFIFOW(fd,2) - 30;
	message = (char *) (mes_len >= 255 ? (char *) aMallocA(mes_len) : mbuf);

	min_gm_level = (int)RFIFOW(fd,28);
	safestrncpy(Wisp_name, (char*)RFIFOP(fd,4), NAME_LENGTH);
	safestrncpy(message, (char*)RFIFOP(fd,30), mes_len);
	// information is sended to all online GM
	clif_foreachclient(mapif_parse_WisToGM_sub, min_gm_level, Wisp_name, message, mes_len);

	if (message != mbuf)
		aFree(message);
	return 0;
}

// アカウント変数通知
int intif_parse_Registers(int fd)
{
	int j,p,len,max, flag;
	struct map_session_data *sd;
	struct global_reg *reg;
	int *qty;
	int account_id = RFIFOL(fd,4), char_id = RFIFOL(fd,8);
	struct auth_node *node = chrif_auth_check(account_id, char_id, ST_LOGIN);
	if (node)
		sd = node->sd;
	else { //Normally registries should arrive for in log-in chars.
		sd = map_id2sd(account_id);
		if (sd && RFIFOB(fd,12) == 3 && sd->status.char_id != char_id)
			sd = NULL; //Character registry from another character.
	}
	if (!sd) return 1;

	flag = (sd->save_reg.global_num == -1 || sd->save_reg.account_num == -1 || sd->save_reg.account2_num == -1);

	switch (RFIFOB(fd,12)) {
		case 3: //Character Registry
			reg = sd->save_reg.global;
			qty = &sd->save_reg.global_num;
			max = GLOBAL_REG_NUM;
		break;
		case 2: //Account Registry
			reg = sd->save_reg.account;
			qty = &sd->save_reg.account_num;
			max = ACCOUNT_REG_NUM;
		break;
		case 1: //Account2 Registry
			reg = sd->save_reg.account2;
			qty = &sd->save_reg.account2_num;
			max = ACCOUNT_REG2_NUM;
		break;
		default:
			ShowError("intif_parse_Registers: Unrecognized type %d\n",RFIFOB(fd,12));
			return 0;
	}
	for(j=0,p=13;j<max && p<RFIFOW(fd,2);j++){
		sscanf((char*)RFIFOP(fd,p), "%31c%n", reg[j].str,&len);
		reg[j].str[len]='\0';
		p += len+1; //+1 to skip the '\0' between strings.
		sscanf((char*)RFIFOP(fd,p), "%255c%n", reg[j].value,&len);
		reg[j].value[len]='\0';
		p += len+1;
	}
	*qty = j;

	if (flag && sd->save_reg.global_num > -1 && sd->save_reg.account_num > -1 && sd->save_reg.account2_num > -1)
		pc_reg_received(sd); //Received all registry values, execute init scripts and what-not. [Skotlex]
	return 1;
}

// 倉庫データ受信
int intif_parse_LoadStorage(int fd)
{
	struct storage *stor;
	struct map_session_data *sd;

	sd=map_id2sd( RFIFOL(fd,4) );
	if(sd==NULL){
		ShowError("intif_parse_LoadStorage: user not found %d\n",RFIFOL(fd,4));
		return 1;
	}

	stor = account2storage( RFIFOL(fd,4));

	if (stor->storage_status == 1) { // Already open.. lets ignore this update
		ShowWarning("intif_parse_LoadStorage: storage received for a client already open (User %d:%d)\n", sd->status.account_id, sd->status.char_id);
		return 1;
	}
	if (stor->dirty) { // Already have storage, and it has been modified and not saved yet! Exploit! [Skotlex]
		ShowWarning("intif_parse_LoadStorage: received storage for an already modified non-saved storage! (User %d:%d)\n", sd->status.account_id, sd->status.char_id);
		return 1;
	}
	if (RFIFOW(fd,2)-8 != sizeof(struct storage)) {
		ShowError("intif_parse_LoadStorage: data size error %d %d\n", RFIFOW(fd,2)-8, sizeof(struct storage));
		return 1;
	}
	if(battle_config.save_log)
		ShowInfo("intif_openstorage: %d\n",RFIFOL(fd,4) );
	memcpy(stor,RFIFOP(fd,8),sizeof(struct storage));
	stor->dirty=0;
	stor->storage_status=1;
	sd->state.storage_flag = 1;
	clif_storagelist(sd,stor);
	clif_updatestorageamount(sd,stor);

	return 0;
}

// 倉庫データ送信成功
int intif_parse_SaveStorage(int fd)
{
	if(battle_config.save_log)
		ShowInfo("intif_savestorage: done %d %d\n",RFIFOL(fd,2),RFIFOB(fd,6) );
	storage_storage_saved(RFIFOL(fd,2));
	return 0;
}

int intif_parse_LoadGuildStorage(int fd)
{
	struct guild_storage *gstor;
	struct map_session_data *sd;
	int guild_id;
	
	guild_id = RFIFOL(fd,8);
	if(guild_id <= 0)
		return 1;
	sd=map_id2sd( RFIFOL(fd,4) );
	if(sd==NULL){
		ShowError("intif_parse_LoadGuildStorage: user not found %d\n",RFIFOL(fd,4));
		return 1;
	}
	gstor=guild2storage(guild_id);
	if(!gstor) {
		ShowWarning("intif_parse_LoadGuildStorage: error guild_id %d not exist\n",guild_id);
		return 1;
	}
	if (gstor->storage_status == 1) { // Already open.. lets ignore this update
		ShowWarning("intif_parse_LoadGuildStorage: storage received for a client already open (User %d:%d)\n", sd->status.account_id, sd->status.char_id);
		return 1;
	}
	if (gstor->dirty) { // Already have storage, and it has been modified and not saved yet! Exploit! [Skotlex]
		ShowWarning("intif_parse_LoadGuildStorage: received storage for an already modified non-saved storage! (User %d:%d)\n", sd->status.account_id, sd->status.char_id);
		return 1;
	}
	if( RFIFOW(fd,2)-12 != sizeof(struct guild_storage) ){
		ShowError("intif_parse_LoadGuildStorage: data size error %d %d\n",RFIFOW(fd,2)-12 , sizeof(struct guild_storage));
		gstor->storage_status = 0;
		return 1;
	}
	if(battle_config.save_log)
		ShowInfo("intif_open_guild_storage: %d\n",RFIFOL(fd,4) );
	memcpy(gstor,RFIFOP(fd,12),sizeof(struct guild_storage));
	gstor->storage_status = 1;
	sd->state.storage_flag = 2;
	clif_guildstoragelist(sd,gstor);
	clif_updateguildstorageamount(sd,gstor);
	return 0;
}
int intif_parse_SaveGuildStorage(int fd)
{
	if(battle_config.save_log) {
		ShowInfo("intif_save_guild_storage: done %d %d %d\n",RFIFOL(fd,2),RFIFOL(fd,6),RFIFOB(fd,10) );
	}
	storage_guild_storagesaved(/*RFIFOL(fd,2), */RFIFOL(fd,6));
	return 0;
}

// パーティ作成可否
int intif_parse_PartyCreated(int fd)
{
	if(battle_config.etc_log)
		ShowInfo("intif: party created by account %d\n\n", RFIFOL(fd,2));
	party_created(RFIFOL(fd,2), RFIFOL(fd,6),RFIFOB(fd,10),RFIFOL(fd,11), (char *)RFIFOP(fd,15));
	return 0;
}
// パーティ情報
int intif_parse_PartyInfo(int fd)
{
	if( RFIFOW(fd,2)==8){
		ShowWarning("intif: party noinfo %d\n",RFIFOL(fd,4));
		party_recv_noinfo(RFIFOL(fd,4));
		return 0;
	}

	if( RFIFOW(fd,2)!=sizeof(struct party)+4 )
		ShowError("intif: party info : data size error %d %d %d\n",RFIFOL(fd,4),RFIFOW(fd,2),sizeof(struct party)+4);
	party_recv_info((struct party *)RFIFOP(fd,4));
	return 0;
}
// パーティ追加通知
int intif_parse_PartyMemberAdded(int fd)
{
	if(battle_config.etc_log)
		ShowInfo("intif: party member added Party (%d), Account(%d), Char(%d)\n",RFIFOL(fd,2),RFIFOL(fd,6),RFIFOL(fd,10));
	party_member_added(RFIFOL(fd,2),RFIFOL(fd,6),RFIFOL(fd,10), RFIFOB(fd, 14));
	return 0;
}
// パーティ設定変更通知
int intif_parse_PartyOptionChanged(int fd)
{
	party_optionchanged(RFIFOL(fd,2),RFIFOL(fd,6),RFIFOW(fd,10),RFIFOW(fd,12),RFIFOB(fd,14));
	return 0;
}
// パーティ脱退通知
int intif_parse_PartyMemberLeaved(int fd)
{
	if(battle_config.etc_log)
		ShowInfo("intif: party member leaved: Party(%d), Account(%d), Char(%d)\n",RFIFOL(fd,2),RFIFOL(fd,6),RFIFOL(fd,10));
	party_member_leaved(RFIFOL(fd,2),RFIFOL(fd,6),RFIFOL(fd,10));
	return 0;
}
// パーティ解散通知
int intif_parse_PartyBroken(int fd)
{
	party_broken(RFIFOL(fd,2));
	return 0;
}
// パーティ移動通知
int intif_parse_PartyMove(int fd)
{
	party_recv_movemap(RFIFOL(fd,2),RFIFOL(fd,6),RFIFOL(fd,10),RFIFOW(fd,14),RFIFOB(fd,16),RFIFOW(fd,17));
	return 0;
}
// パーティメッセージ
int intif_parse_PartyMessage(int fd)
{
	party_recv_message(RFIFOL(fd,4),RFIFOL(fd,8),(char *) RFIFOP(fd,12),RFIFOW(fd,2)-12);
	return 0;
}

// ギルド作成可否
int intif_parse_GuildCreated(int fd)
{
	guild_created(RFIFOL(fd,2),RFIFOL(fd,6));
	return 0;
}
// ギルド情報
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
	return 0;
}
// ギルドメンバ追加通知
int intif_parse_GuildMemberAdded(int fd)
{
	if(battle_config.etc_log)
		ShowInfo("intif: guild member added %d %d %d %d\n",RFIFOL(fd,2),RFIFOL(fd,6),RFIFOL(fd,10),RFIFOB(fd,14));
	guild_member_added(RFIFOL(fd,2),RFIFOL(fd,6),RFIFOL(fd,10),RFIFOB(fd,14));
	return 0;
}
// ギルドメンバ脱退/追放通知
int intif_parse_GuildMemberLeaved(int fd)
{
	guild_member_leaved(RFIFOL(fd,2),RFIFOL(fd,6),RFIFOL(fd,10),RFIFOB(fd,14),(char *)RFIFOP(fd,55),(char *)RFIFOP(fd,15));
	return 0;
}

// ギルドメンバオンライン状態/Lv変更通知
int intif_parse_GuildMemberInfoShort(int fd)
{
	guild_recv_memberinfoshort(RFIFOL(fd,2),RFIFOL(fd,6),RFIFOL(fd,10),RFIFOB(fd,14),RFIFOW(fd,15),RFIFOW(fd,17));
	return 0;
}
// ギルド解散通知
int intif_parse_GuildBroken(int fd)
{
	guild_broken(RFIFOL(fd,2),RFIFOB(fd,6));
	return 0;
}

// basic guild info change notice
// 0x3839 <packet len>.w <guild id>.l <type>.w <data>.?b
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
	case GBI_EXP:        g->exp = RFIFOL(fd,10); break;
	case GBI_GUILDLV:    g->guild_lv = RFIFOW(fd,10); break;
	case GBI_SKILLPOINT: g->skill_point = RFIFOL(fd,10); break;
	}

	return 0;
}

// guild member info change notice
// 0x383a <packet len>.w <guild id>.l <account id>.l <char id>.l <type>.w <data>.?b
int intif_parse_GuildMemberInfoChanged(int fd)
{
	//int len = RFIFOW(fd,2) - 18;
	int guild_id = RFIFOL(fd,4);
	int account_id = RFIFOL(fd,8);
	int char_id = RFIFOL(fd,12);
	int type = RFIFOW(fd,16);
	void* data = RFIFOP(fd,18);
	int dd = *((int *)data);

	struct guild* g;
	int idx;

	g = guild_search(guild_id);
	if( g == NULL )
		return 0;

	idx = guild_getindex(g,account_id,char_id);
	if( idx == -1 )
		return 0;

	switch( type ) {
	case GMI_POSITION:   g->member[idx].position = dd; guild_memberposition_changed(g,idx,dd); break;
	case GMI_EXP:        g->member[idx].exp = dd; break;
	case GMI_HAIR:       g->member[idx].hair = dd; break;
	case GMI_HAIR_COLOR: g->member[idx].hair_color = dd; break;
	case GMI_GENDER:     g->member[idx].gender = dd; break;
	case GMI_CLASS:      g->member[idx].class_ = dd; break;
	case GMI_LEVEL:      g->member[idx].lv = dd; break;
	}
	return 0;
}

// ギルド役職変更通知
int intif_parse_GuildPosition(int fd)
{
	if( RFIFOW(fd,2)!=sizeof(struct guild_position)+12 )
		ShowError("intif: guild info : data size error\n %d %d %d",RFIFOL(fd,4),RFIFOW(fd,2),sizeof(struct guild_position)+12);
	guild_position_changed(RFIFOL(fd,4),RFIFOL(fd,8),(struct guild_position *)RFIFOP(fd,12));
	return 0;
}
// ギルドスキル割り振り通知
int intif_parse_GuildSkillUp(int fd)
{
	guild_skillupack(RFIFOL(fd,2),RFIFOL(fd,6),RFIFOL(fd,10));
	return 0;
}
// ギルド同盟/敵対通知
int intif_parse_GuildAlliance(int fd)
{
	guild_allianceack(RFIFOL(fd,2),RFIFOL(fd,6),RFIFOL(fd,10),RFIFOL(fd,14),RFIFOB(fd,18),(char *) RFIFOP(fd,19),(char *) RFIFOP(fd,43));
	return 0;
}
// ギルド告知変更通知
int intif_parse_GuildNotice(int fd)
{
	guild_notice_changed(RFIFOL(fd,2),(char *) RFIFOP(fd,6),(char *) RFIFOP(fd,66));
	return 0;
}
// ギルドエンブレム変更通知
int intif_parse_GuildEmblem(int fd)
{
	guild_emblem_changed(RFIFOW(fd,2)-12,RFIFOL(fd,4),RFIFOL(fd,8), (char *)RFIFOP(fd,12));
	return 0;
}
// ギルド会話受信
int intif_parse_GuildMessage(int fd)
{
	guild_recv_message(RFIFOL(fd,4),RFIFOL(fd,8),(char *) RFIFOP(fd,12),RFIFOW(fd,2)-12);
	return 0;
}
// ギルド城データ要求返信
int intif_parse_GuildCastleDataLoad(int fd)
{
	return guild_castledataloadack(RFIFOW(fd,2),RFIFOB(fd,4),RFIFOL(fd,5));
}
// ギルド城データ変更通知
int intif_parse_GuildCastleDataSave(int fd)
{
	return guild_castledatasaveack(RFIFOW(fd,2),RFIFOB(fd,4),RFIFOL(fd,5));
}

// ギルド城データ一括受信(初期化時)
int intif_parse_GuildCastleAllDataLoad(int fd)
{
	return guild_castlealldataload(RFIFOW(fd,2),(struct guild_castle *)RFIFOP(fd,4));
}

int intif_parse_GuildMasterChanged(int fd)
{
	return guild_gm_changed(RFIFOL(fd,2),RFIFOL(fd,6),RFIFOL(fd,10));
}

// pet
int intif_parse_CreatePet(int fd)
{
	pet_get_egg(RFIFOL(fd,2),RFIFOL(fd,7),RFIFOB(fd,6));
	return 0;
}

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

	return 0;
}
int intif_parse_SavePetOk(int fd)
{
	if(RFIFOB(fd,6) == 1)
		ShowError("pet data save failure\n");

	return 0;
}

int intif_parse_DeletePetOk(int fd)
{
	if(RFIFOB(fd,2) == 1)
		ShowError("pet data delete failure\n");

	return 0;
}

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
		merc_hom_change_name_ack(sd, (char*)RFIFOP(fd,12), RFIFOB(fd,11));
		break;
	}
	return 0;
}

//----------------------------------------------------------------
// Homunculus recv packets [albator]

int intif_parse_CreateHomunculus(int fd)
{
	int len;
	len=RFIFOW(fd,2)-9;
	if(sizeof(struct s_homunculus)!=len) {
		if(battle_config.etc_log)
			ShowError("intif: create homun data: data size error %d != %d\n",sizeof(struct s_homunculus),len);
		return 0;
	}
	merc_hom_recv_data(RFIFOL(fd,4), (struct s_homunculus*)RFIFOP(fd,9), RFIFOB(fd,8)) ;
	return 0;
}

int intif_parse_RecvHomunculusData(int fd)
{
	int len;

	len=RFIFOW(fd,2)-9;

	if(sizeof(struct s_homunculus)!=len) {
		if(battle_config.etc_log)
			ShowError("intif: homun data: data size error %d %d\n",sizeof(struct s_homunculus),len);
		return 0;
	}
	merc_hom_recv_data(RFIFOL(fd,4), (struct s_homunculus*)RFIFOP(fd,9), RFIFOB(fd,8));
	return 0;
}

int intif_parse_SaveHomunculusOk(int fd)
{
	if(RFIFOB(fd,6) != 1)
		ShowError("homunculus data save failure for account %d\n", RFIFOL(fd,2));

	return 0;
}

int intif_parse_DeleteHomunculusOk(int fd)
{
	if(RFIFOB(fd,2) != 1)
		ShowError("Homunculus data delete failure\n");

	return 0;
}

#ifndef TXT_ONLY
/*==========================================
 * MAIL SYSTEM
 * By Zephyrus
 *==========================================*/

/*------------------------------------------
 * Inbox Request
 * flag: 0 Update Inbox | 1 OpenMail
 *------------------------------------------*/
int intif_Mail_requestinbox(int char_id, unsigned char flag)
{
	if (CheckForCharServer())
		return 0;

	WFIFOHEAD(inter_fd,7);
	WFIFOW(inter_fd,0) = 0x3048;
	WFIFOL(inter_fd,2) = char_id;
	WFIFOB(inter_fd,6) = flag;
	WFIFOSET(inter_fd,7);

	return 0;
}

int intif_parse_Mail_inboxreceived(int fd)
{
	struct map_session_data *sd;
	unsigned char flag = RFIFOB(fd,8);

	sd = map_charid2sd(RFIFOL(fd,4));

	if (sd == NULL)
	{
		ShowError("intif_parse_Mail_inboxreceived: char not found %d\n",RFIFOL(fd,4));
		return 1;
	}

	if (RFIFOW(fd,2) - 9 != sizeof(struct mail_data))
	{
		ShowError("intif_parse_Mail_inboxreceived: data size error %d %d\n", RFIFOW(fd,2) - 9, sizeof(struct mail_data));
		return 1;
	}

	//FIXME: this operation is not safe [ultramage]
	memcpy(&sd->mail.inbox, RFIFOP(fd,9), sizeof(struct mail_data));

	if (flag)
		clif_Mail_refreshinbox(sd);
	else
	{
		char output[128];
		sprintf(output, "You have %d new emails (%d unread)", sd->mail.inbox.unchecked, sd->mail.inbox.unread + sd->mail.inbox.unchecked);
		clif_disp_onlyself(sd, output, strlen(output));
	}
	return 0;
}
/*------------------------------------------
 * Mail Readed
 *------------------------------------------*/
int intif_Mail_read(int mail_id)
{
	if (CheckForCharServer())
		return 0;

	WFIFOHEAD(inter_fd,6);
	WFIFOW(inter_fd,0) = 0x3049;
	WFIFOL(inter_fd,2) = mail_id;
	WFIFOSET(inter_fd,6);

	return 0;
}
/*------------------------------------------
 * Get Attachment
 *------------------------------------------*/
int intif_Mail_getattach(int char_id, int mail_id)
{
	if (CheckForCharServer())
		return 0;

	WFIFOHEAD(inter_fd,10);
	WFIFOW(inter_fd,0) = 0x304a;
	WFIFOL(inter_fd,2) = char_id;
	WFIFOL(inter_fd,6) = mail_id;
	WFIFOSET(inter_fd, 10);

	return 0;
}

int intif_parse_Mail_getattach(int fd)
{
	struct map_session_data *sd;
	struct item item;
	int zeny = RFIFOL(fd,8);

	sd = map_charid2sd( RFIFOL(fd,4) );

	if (sd == NULL)
	{
		ShowError("intif_parse_Mail_getattach: char not found %d\n",RFIFOL(fd,4));
		return 1;
	}

	if (RFIFOW(fd,2) - 12 != sizeof(struct item))
	{
		ShowError("intif_parse_Mail_getattach: data size error %d %d\n", RFIFOW(fd,2) - 16, sizeof(struct item));
		return 1;
	}

	memcpy(&item, RFIFOP(fd,12), sizeof(struct item));

	mail_getattachment(sd, zeny, &item);
	return 0;
}
/*------------------------------------------
 * Delete Message
 *------------------------------------------*/
int intif_Mail_delete(int char_id, int mail_id)
{
	if (CheckForCharServer())
		return 0;

	WFIFOHEAD(inter_fd,10);
	WFIFOW(inter_fd,0) = 0x304b;
	WFIFOL(inter_fd,2) = char_id;
	WFIFOL(inter_fd,6) = mail_id;
	WFIFOSET(inter_fd,10);

	return 0;
}

int intif_parse_Mail_delete(int fd)
{
	int char_id = RFIFOL(fd,2);
	int mail_id = RFIFOL(fd,6);
	bool failed = RFIFOB(fd,10);

	struct map_session_data *sd = map_charid2sd(char_id);
	if (sd == NULL)
	{
		ShowError("intif_parse_Mail_delete: char not found %d\n", char_id);
		return 1;
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
	return 0;
}
/*------------------------------------------
 * Return Message
 *------------------------------------------*/
int intif_Mail_return(int char_id, int mail_id)
{
	if (CheckForCharServer())
		return 0;

	WFIFOHEAD(inter_fd,10);
	WFIFOW(inter_fd,0) = 0x304c;
	WFIFOL(inter_fd,2) = char_id;
	WFIFOL(inter_fd,6) = mail_id;
	WFIFOSET(inter_fd,10);

	return 0;
}

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
	return 0;
}
/*------------------------------------------
 * Send Mail
 *------------------------------------------*/
int intif_Mail_send(int account_id, struct mail_message *msg)
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

	if( (sd = map_charid2sd(msg.send_id)) )
	{
		if( fail )
			mail_deliveryfail(sd, &msg);
		else
			clif_Mail_send(sd->fd, false);
	}

	if( fail )
		return;

	if( (sd = map_charid2sd(msg.dest_id)) )
	{
		sd->mail.inbox.changed = true;
		clif_Mail_new(sd->fd, msg.id, msg.send_name, msg.title);
	}
}

static void intif_parse_Mail_new(int fd)
{
	struct map_session_data *sd = map_charid2sd(RFIFOL(fd,2));
	int mail_id = RFIFOL(fd,6);
	const char* sender_name = (char*)RFIFOP(fd,10);
	const char* title = (char*)RFIFOP(fd,34);

	if( sd == NULL )
		return;

	sd->mail.inbox.changed = true;
	clif_Mail_new(sd->fd, mail_id, sender_name, title);
}

/*==========================================
 * AUCTION SYSTEM
 * By Zephyrus
 *==========================================*/
int intif_Auction_requestlist(int char_id, short type, int price, const char* searchtext)
{
	int len = NAME_LENGTH + 14;

	if( CheckForCharServer() )
		return 0;

	WFIFOHEAD(inter_fd,len);
	WFIFOW(inter_fd,0) = 0x3050;
	WFIFOW(inter_fd,2) = len;
	WFIFOL(inter_fd,4) = char_id;
	WFIFOW(inter_fd,8) = type;
	WFIFOL(inter_fd,10) = price;
	memcpy(WFIFOP(inter_fd,14), searchtext, NAME_LENGTH);
	WFIFOSET(inter_fd,len);

	return 0;
}

static void intif_parse_Auction_results(int fd)
{
	struct map_session_data *sd = map_charid2sd(RFIFOL(fd,4));
	short count = (RFIFOW(fd,2) - 8) / sizeof(struct auction_data);

	if( sd == NULL )
		return;

	clif_Auction_results(sd, count, (char *)RFIFOP(fd,8));
}

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

static void intif_parse_Auction_register(int fd)
{
	struct map_session_data *sd;
	struct auction_data auction;

	if( RFIFOW(fd,2) - 4 != sizeof(struct auction_data) )
	{
		ShowError("intif_parse_Auction_register: data size error %d %d\n", RFIFOW(fd,2) - 4, sizeof(struct auction_data));
		return;
	}

	memcpy(&auction, WFIFOP(fd,4), sizeof(struct auction_data));
	if( (sd = map_charid2sd(auction.seller_id)) == NULL )
		return;

	if( auction.auction_id > 0 )
		clif_Auction_message(sd->fd, 1); // Confirmation Packet ??
	else
	{
		clif_Auction_message(sd->fd, 4);
		pc_additem(sd, &auction.item, auction.item.amount);
		pc_getzeny(sd, auction.hours * battle_config.auction_feeperhour);
	}
}

#endif

//-----------------------------------------------------------------
// inter serverからの通信
// エラーがあれば0(false)を返すこと
// パケットが処理できれば1,パケット長が足りなければ2を返すこと
int intif_parse(int fd)
{
	int packet_len, cmd;
	cmd = RFIFOW(fd,0);
	// パケットのID確認
	if(cmd<0x3800 || cmd>=0x3800+(sizeof(packet_len_table)/sizeof(packet_len_table[0])) ||
	   packet_len_table[cmd-0x3800]==0){
	   	return 0;
	}
	// パケットの長さ確認
	packet_len = packet_len_table[cmd-0x3800];
	if(packet_len==-1){
		if(RFIFOREST(fd)<4)
			return 2;
		packet_len = RFIFOW(fd,2);
	}
	if((int)RFIFOREST(fd)<packet_len){
		return 2;
	}
	// 処理分岐
	switch(cmd){
	case 0x3800:
		if (RFIFOL(fd,4) == 0xFF000000) //Normal announce.
			clif_GMmessage(NULL,(char *) RFIFOP(fd,8),packet_len-8,0);
		else if (RFIFOL(fd,4) == 0xFE000000) //Main chat message [LuzZza]
			clif_MainChatMessage((char *)RFIFOP(fd,8));
		else //Color announce.
			clif_announce(NULL,(char *) RFIFOP(fd,8),packet_len-8,RFIFOL(fd,4),0);
		break;
	case 0x3801:	intif_parse_WisMessage(fd); break;
	case 0x3802:	intif_parse_WisEnd(fd); break;
	case 0x3803:	mapif_parse_WisToGM(fd); break;
	case 0x3804:	intif_parse_Registers(fd); break;
	case 0x3806:	intif_parse_ChangeNameOk(fd); break;
	case 0x3810:	intif_parse_LoadStorage(fd); break;
	case 0x3811:	intif_parse_SaveStorage(fd); break;
	case 0x3818:	intif_parse_LoadGuildStorage(fd); break;
	case 0x3819:	intif_parse_SaveGuildStorage(fd); break;
	case 0x3820:	intif_parse_PartyCreated(fd); break;
	case 0x3821:	intif_parse_PartyInfo(fd); break;
	case 0x3822:	intif_parse_PartyMemberAdded(fd); break;
	case 0x3823:	intif_parse_PartyOptionChanged(fd); break;
	case 0x3824:	intif_parse_PartyMemberLeaved(fd); break;
	case 0x3825:	intif_parse_PartyMove(fd); break;
	case 0x3826:	intif_parse_PartyBroken(fd); break;
	case 0x3827:	intif_parse_PartyMessage(fd); break;
	case 0x3830:	intif_parse_GuildCreated(fd); break;
	case 0x3831:	intif_parse_GuildInfo(fd); break;
	case 0x3832:	intif_parse_GuildMemberAdded(fd); break;
	case 0x3834:	intif_parse_GuildMemberLeaved(fd); break;
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
	case 0x3841:	intif_parse_GuildCastleDataSave(fd); break;
	case 0x3842:	intif_parse_GuildCastleAllDataLoad(fd); break;
	case 0x3843:	intif_parse_GuildMasterChanged(fd); break;
#ifndef TXT_ONLY
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
#endif
	case 0x3880:	intif_parse_CreatePet(fd); break;
	case 0x3881:	intif_parse_RecvPetData(fd); break;
	case 0x3882:	intif_parse_SavePetOk(fd); break;
	case 0x3883:	intif_parse_DeletePetOk(fd); break;
	case 0x3890:	intif_parse_CreateHomunculus(fd); break;
	case 0x3891:	intif_parse_RecvHomunculusData(fd); break;
	case 0x3892:	intif_parse_SaveHomunculusOk(fd); break;
	case 0x3893:	intif_parse_DeleteHomunculusOk(fd); break;
	default:
		ShowError("intif_parse : unknown packet %d %x\n",fd,RFIFOW(fd,0));
		return 0;
	}
	// パケット読み飛ばし
	RFIFOSKIP(fd,packet_len);
	return 1;
}
