// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/db.h"
#include "../common/mmo.h"
#include "../common/socket.h"
#include "../common/timer.h"
#include "../common/malloc.h"
#include "../common/lock.h"
#include "../common/showmsg.h"
#include "../common/strlib.h"
#include "char.h"
#include "inter.h"
#include "int_party.h"
#include "int_guild.h"
#include "int_status.h"
#include "int_storage.h"
#include "int_pet.h"
#include "int_homun.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define WISDATA_TTL (60*1000)	// Existence time of Wisp/page data (60 seconds)
                             	// that is the waiting time of answers of all map-servers
#define WISDELLIST_MAX 256   	// Number of elements of Wisp/page data deletion list

char accreg_txt[1024] = "save/accreg.txt";
#ifndef TXT_SQL_CONVERT
char inter_log_filename[1024] = "log/inter.log";
char main_chat_nick[16] = "Main";

static DBMap* accreg_db = NULL; // int account_id -> struct accreg*

unsigned int party_share_level = 10;

// sending packet list
// NOTE: This variable ain't used at all! And it's confusing.. where do I add that the length of packet 0x2b07 is 10? x.x [Skotlex]
int inter_send_packet_length[]={
	-1,-1,27,-1, -1, 0, 0, 0,  0, 0, 0, 0,  0, 0,  0, 0, //0x3000-0x300f
	-1, 7, 0, 0,  0, 0, 0, 0, -1,11, 0, 0,  0, 0,  0, 0,
	35,-1,11,15, 34,29, 7,-1,  0, 0, 0, 0,  0, 0,  0, 0,
	10,-1,15, 0, 79,19, 7,-1,  0,-1,-1,-1, 14,67,186,-1,
	 9, 9,-1, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0,  0, 0,
	 0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0,  0, 0,
	 0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0,  0, 0,
	 0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0,  0, 0,
	11,-1, 7, 3, 36, 0, 0, 0,  0, 0, 0, 0,  0, 0,  0, 0,
};
// recv. packet list
int inter_recv_packet_length[]={
	-1,-1, 7,-1, -1,13,36, 0,  0, 0, 0, 0,  0, 0,  0, 0, //0x3000-0x300f
	 6,-1, 0, 0,  0, 0, 0, 0, 10,-1, 0, 0,  0, 0,  0, 0, //0x3010-0x301f
	-1, 6,-1,14, 14,19, 6,-1, 14,14, 0, 0,  0, 0,  0, 0, //0x3020-0x302f
	-1, 6,-1,-1, 55,19, 6,-1, 14,-1,-1,-1, 14,19,186,-1, //0x3030-0x303f
	 5, 9, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0,  0, 0, //0x3040-0x304f
	 0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0,  0, 0, //0x3050-0x305f
	 0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0,  0, 0, //0x3060-0x306f
	 0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0,  0, 0, //0x3070-0x307f
	48,14,-1, 6,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0,  0, 0, //0x3080-0x308f
	-1,10,-1, 6,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0,  0, 0, //0x3090-0x309f  Homunculus packets [albator]
};

struct WisData {
	int id, fd, count, len;
	unsigned long tick;
	unsigned char src[24], dst[24], msg[1024];
};
static DBMap* wis_db = NULL; // int wis_id -> struct WisData*
static int wis_dellist[WISDELLIST_MAX], wis_delnum;


//--------------------------------------------------------

// アカウント変数を文字列へ変換
int inter_accreg_tostr(char *str, struct accreg *reg) {
	int j;
	char *p = str;

	p += sprintf(p, "%d\t", reg->account_id);
	for(j = 0; j < reg->reg_num; j++) {
		p += sprintf(p,"%s,%s ", reg->reg[j].str, reg->reg[j].value);
	}

	return 0;
}
#endif //TXT_SQL_CONVERT
// アカウント変数を文字列から変換
int inter_accreg_fromstr(const char *str, struct accreg *reg) {
	int j, n;
	const char *p = str;

	if (sscanf(p, "%d\t%n", &reg->account_id, &n ) != 1 || reg->account_id <= 0)
		return 1;

	for(j = 0, p += n; j < ACCOUNT_REG_NUM; j++, p += n) {
		if (sscanf(p, "%[^,],%[^ ] %n", reg->reg[j].str, reg->reg[j].value, &n) != 2) 
			break;
	}
	reg->reg_num = j;

	return 0;
}
#ifndef TXT_SQL_CONVERT
// アカウント変数の読み込み
int inter_accreg_init(void) {
	char line[8192];
	FILE *fp;
	int c = 0;
	struct accreg *reg;

	accreg_db = idb_alloc(DB_OPT_RELEASE_DATA);

	if( (fp = fopen(accreg_txt, "r")) == NULL)
		return 1;
	while(fgets(line, sizeof(line), fp))
	{
		reg = (struct accreg*)aCalloc(sizeof(struct accreg), 1);
		if (reg == NULL) {
			ShowFatalError("inter: accreg: out of memory!\n");
			exit(EXIT_FAILURE);
		}
		if (inter_accreg_fromstr(line, reg) == 0 && reg->account_id > 0) {
			idb_put(accreg_db, reg->account_id, reg);
		} else {
			ShowError("inter: accreg: broken data [%s] line %d\n", accreg_txt, c);
			aFree(reg);
		}
		c++;
	}
	fclose(fp);

	return 0;
}

// アカウント変数のセーブ用
int inter_accreg_save_sub(DBKey key, void *data, va_list ap) {
	char line[8192];
	FILE *fp;
	struct accreg *reg = (struct accreg *)data;

	if (reg->reg_num > 0) {
		inter_accreg_tostr(line,reg);
		fp = va_arg(ap, FILE *);
		fprintf(fp, "%s\n", line);
	}

	return 0;
}

// アカウント変数のセーブ
int inter_accreg_save(void) {
	FILE *fp;
	int lock;

	if ((fp = lock_fopen(accreg_txt,&lock)) == NULL) {
		ShowError("int_accreg: can't write [%s] !!! data is lost !!!\n", accreg_txt);
		return 1;
	}
	accreg_db->foreach(accreg_db, inter_accreg_save_sub,fp);
	lock_fclose(fp, accreg_txt, &lock);

	return 0;
}

//--------------------------------------------------------
#endif //TXT_SQL_CONVERT
/*==========================================
 * 設定ファイルを読み込む
 *------------------------------------------*/
static int inter_config_read(const char *cfgName) {
	char line[1024], w1[1024], w2[1024];
	FILE *fp;

	fp = fopen(cfgName, "r");
	if (fp == NULL) {
		ShowError("file not found: %s\n", cfgName);
		return 1;
	}
	while(fgets(line, sizeof(line), fp))
	{
		if (line[0] == '/' && line[1] == '/')
			continue;

		if (sscanf(line,"%[^:]: %[^\r\n]", w1, w2) != 2)
			continue;

		if (strcmpi(w1, "storage_txt") == 0) {
			strncpy(storage_txt, w2, sizeof(storage_txt));
		} else if (strcmpi(w1, "party_txt") == 0) {
			strncpy(party_txt, w2, sizeof(party_txt));
		} else if (strcmpi(w1, "pet_txt") == 0) {
			strncpy(pet_txt, w2, sizeof(pet_txt));
		} else if (strcmpi(w1, "accreg_txt") == 0) {
			strncpy(accreg_txt, w2, sizeof(accreg_txt));
		} else if (strcmpi(w1, "guild_txt") == 0) {
			strncpy(guild_txt, w2, sizeof(guild_txt));
		} else if (strcmpi(w1, "castle_txt") == 0) {
			strncpy(castle_txt, w2, sizeof(castle_txt));
		} else if (strcmpi(w1, "guild_storage_txt") == 0) {
			strncpy(guild_storage_txt, w2, sizeof(guild_storage_txt));
#ifndef TXT_SQL_CONVERT
		} else if (strcmpi(w1, "homun_txt") == 0) {
			strncpy(homun_txt, w2, sizeof(homun_txt));
		} else if (strcmpi(w1, "party_share_level") == 0) {
			party_share_level = (unsigned int)atof(w2);
		} else if (strcmpi(w1, "inter_log_filename") == 0) {
			strncpy(inter_log_filename, w2, sizeof(inter_log_filename));
		} else if(strcmpi(w1,"log_inter")==0) {
			log_inter = atoi(w2);
		} else if(strcmpi(w1, "main_chat_nick")==0){	// Main chat nick [LuzZza]
			strcpy(main_chat_nick, w2);
#endif //TXT_SQL_CONVERT
		} else if (strcmpi(w1, "import") == 0) {
			inter_config_read(w2);
		}
	}
	fclose(fp);

	return 0;
}
#ifndef TXT_SQL_CONVERT
// ログ書き出し
int inter_log(char *fmt,...) {
	FILE *logfp;
	va_list ap;

	va_start(ap,fmt);
	logfp = fopen(inter_log_filename, "a");
	if (logfp) {
		vfprintf(logfp, fmt, ap);
		fclose(logfp);
	}
	va_end(ap);

	return 0;
}

// セーブ
int inter_save(void) {
#ifdef ENABLE_SC_SAVING
	inter_status_save();
#endif
	inter_party_save();
	inter_guild_save();
	inter_storage_save();
	inter_guild_storage_save();
	inter_pet_save();
	inter_homun_save();
	inter_accreg_save();

	return 0;
}
#endif //TXT_SQL_CONVERT
// 初期化
int inter_init_txt(const char *file) {
	inter_config_read(file);

#ifndef TXT_SQL_CONVERT
	wis_db = idb_alloc(DB_OPT_RELEASE_DATA);

	inter_party_init();
	inter_guild_init();
	inter_storage_init();
	inter_pet_init();
	inter_homun_init();
	inter_accreg_init();
#endif //TXT_SQL_CONVERT
	return 0;
}
#ifndef TXT_SQL_CONVERT
// finalize
void inter_final(void) {
	accreg_db->destroy(accreg_db, NULL);
	wis_db->destroy(wis_db, NULL);

	inter_party_final();
	inter_guild_final();
	inter_storage_final();
	inter_pet_final();
	inter_homun_final();

	return;
}

// マップサーバー接続
int inter_mapif_init(int fd) {
	inter_guild_mapif_init(fd);

	return 0;
}

//--------------------------------------------------------
// sended packets to map-server

//Sends the current max account/char id to map server [Skotlex]
void mapif_send_maxid(int account_id, int char_id)
{
	unsigned char buf[12];

	WBUFW(buf,0) = 0x2b07;
	WBUFL(buf,2) = account_id;
	WBUFL(buf,6) = char_id;
	mapif_sendall(buf, 10);
}

// GMメッセージ送信
int mapif_GMmessage(unsigned char *mes, int len, unsigned long color, int sfd) {
	unsigned char buf[2048];

	if (len > 2048) len = 2047; //Make it fit to avoid crashes. [Skotlex]
	WBUFW(buf,0) = 0x3800;
	WBUFW(buf,2) = len;
	WBUFL(buf,4) = color;
	memcpy(WBUFP(buf,8), mes, len - 8);
	mapif_sendallwos(sfd, buf, len);

	return 0;
}

// Wisp/page transmission to one map-server
int mapif_wis_message2(struct WisData *wd, int fd) {
	WFIFOHEAD(fd, 56+wd->len);
	WFIFOW(fd, 0) = 0x3801;
	WFIFOW(fd, 2) = 56 + wd->len;
	WFIFOL(fd, 4) = wd->id;
	memcpy(WFIFOP(fd, 8), wd->src, NAME_LENGTH);
	memcpy(WFIFOP(fd,32), wd->dst, NAME_LENGTH);
	memcpy(WFIFOP(fd,56), wd->msg, wd->len);
	wd->count = 1;
	WFIFOSET(fd,WFIFOW(fd,2));
	return 1;
}

// Wisp/page transmission to all map-server
int mapif_wis_message(struct WisData *wd) {
	unsigned char buf[2048];
	if (wd->len > 2047-56) wd->len = 2047-56; //Force it to fit to avoid crashes. [Skotlex]

	WBUFW(buf, 0) = 0x3801;
	WBUFW(buf, 2) = 56 + wd->len;
	WBUFL(buf, 4) = wd->id;
	memcpy(WBUFP(buf, 8), wd->src, NAME_LENGTH);
	memcpy(WBUFP(buf,32), wd->dst, NAME_LENGTH);
	memcpy(WBUFP(buf,56), wd->msg, wd->len);
	wd->count = mapif_sendall(buf, WBUFW(buf,2));

	return 0;
}

int mapif_wis_fail(int fd, char *src) {
	unsigned char buf[27];
	WBUFW(buf, 0) = 0x3802;
	memcpy(WBUFP(buf, 2), src, NAME_LENGTH);
	WBUFB(buf,26) = 1; // flag: 0: success to send wisper, 1: target character is not loged in?, 2: ignored by target
	mapif_send(fd, buf, 27);
	return 0;

}

// Wisp/page transmission result to map-server
int mapif_wis_end(struct WisData *wd, int flag)
{
	unsigned char buf[27];

	WBUFW(buf, 0) = 0x3802;
	memcpy(WBUFP(buf, 2), wd->src, 24);
	WBUFB(buf,26) = flag; // flag: 0: success to send wisper, 1: target character is not loged in?, 2: ignored by target
	mapif_send(wd->fd, buf, 27);

	return 0;
}

// Account registry transfer to map-server
static void mapif_account_reg(int fd, unsigned char *src)
{
	WBUFW(src,0)=0x3804; //NOTE: writing to RFIFO
	mapif_sendallwos(fd, src, WBUFW(src,2));
}

// アカウント変数要求返信
int mapif_account_reg_reply(int fd,int account_id, int char_id)
{
	struct accreg *reg = (struct accreg*)idb_get(accreg_db,account_id);

	WFIFOHEAD(fd, ACCOUNT_REG_NUM * 288+ 13);
	WFIFOW(fd,0) = 0x3804;
	WFIFOL(fd,4) = account_id;
	WFIFOL(fd,8) = char_id;
	WFIFOB(fd,12) = 2; //Acc Reg
	if (reg == NULL) {
		WFIFOW(fd,2) = 13;
	} else {
		int i, p;
		for (p=13,i = 0; i < reg->reg_num; i++) {
			p+= sprintf((char*)WFIFOP(fd,p), "%s", reg->reg[i].str)+1; //We add 1 to consider the '\0' in place.
			p+= sprintf((char*)WFIFOP(fd,p), "%s", reg->reg[i].value)+1;
		}
		WFIFOW(fd,2)=p;
	}
	WFIFOSET(fd,WFIFOW(fd,2));
	return 0;
}

//Request to kick char from a certain map server. [Skotlex]
int mapif_disconnectplayer(int fd, int account_id, int char_id, int reason)
{
	if (fd < 0)
		return -1;
	
	WFIFOHEAD(fd, 7);
	WFIFOW(fd,0) = 0x2b1f;
	WFIFOL(fd,2) = account_id;
	WFIFOB(fd,6) = reason;
	WFIFOSET(fd,7);
	
	return 0;
}

//--------------------------------------------------------

// Existence check of WISP data
int check_ttl_wisdata_sub(DBKey key, void *data, va_list ap) {
	unsigned long tick;
	struct WisData *wd = (struct WisData *)data;
	tick = va_arg(ap, unsigned long);

	if (DIFF_TICK(tick, wd->tick) > WISDATA_TTL && wis_delnum < WISDELLIST_MAX)
		wis_dellist[wis_delnum++] = wd->id;

	return 0;
}

int check_ttl_wisdata(void) {
	unsigned long tick = gettick();
	int i;

	do {
		wis_delnum = 0;
		wis_db->foreach(wis_db, check_ttl_wisdata_sub, tick);
		for(i = 0; i < wis_delnum; i++) {
			struct WisData *wd = (struct WisData*)idb_get(wis_db, wis_dellist[i]);
			ShowWarning("inter: wis data id=%d time out : from %s to %s\n", wd->id, wd->src, wd->dst);
			// removed. not send information after a timeout. Just no answer for the player
			//mapif_wis_end(wd, 1); // flag: 0: success to send wisper, 1: target character is not loged in?, 2: ignored by target
			idb_remove(wis_db, wd->id);
		}
	} while(wis_delnum >= WISDELLIST_MAX);

	return 0;
}

//--------------------------------------------------------
// received packets from map-server

// GMメッセージ送信
int mapif_parse_GMmessage(int fd) {
	RFIFOHEAD(fd);
	mapif_GMmessage(RFIFOP(fd,8), RFIFOW(fd,2), RFIFOL(fd,4), fd);

	return 0;
}

static struct WisData* mapif_create_whisper(int fd, char* src, char* dst, char* mes, int meslen)
{
	static int wisid = 0;
	struct WisData* wd = (struct WisData *)aCalloc(sizeof(struct WisData), 1);
	if (wd == NULL){
		ShowFatalError("inter: WisRequest: out of memory !\n");
		return NULL;
	}
	wd->id = ++wisid;
	wd->fd = fd;
	wd->len= meslen;
	memcpy(wd->src, src, NAME_LENGTH);
	memcpy(wd->dst, dst, NAME_LENGTH);
	memcpy(wd->msg, mes, meslen);
	wd->tick = gettick();
	return wd;
}

// Wisp/page request to send
int mapif_parse_WisRequest(int fd)
{
	struct mmo_charstatus* char_status;
	struct WisData* wd;
	char name[NAME_LENGTH];
	int fd2;

	if (RFIFOW(fd,2)-52 >= sizeof(wd->msg)) {
		ShowWarning("inter: Wis message size too long.\n");
		return 0;
	} else if (RFIFOW(fd,2)-52 <= 0) { // normaly, impossible, but who knows...
		ShowError("inter: Wis message doesn't exist.\n");
		return 0;
	}

	safestrncpy(name, (char*)RFIFOP(fd,28), NAME_LENGTH); //Received name may be too large and not contain \0! [Skotlex]

	// search if character exists before to ask all map-servers
	char_status = search_character_byname(name);
	if (char_status == NULL)
		return mapif_wis_fail(fd, (char*)RFIFOP(fd, 4));

	// Character exists.
	// to be sure of the correct name, rewrite it
	memset(name, 0, NAME_LENGTH);
	strncpy(name, char_status->name, NAME_LENGTH);
	// if source is destination, don't ask other servers.
	if (strcmp((char*)RFIFOP(fd,4),name) == 0)
		return mapif_wis_fail(fd, (char*)RFIFOP(fd, 4));

	//Look for online character.
	fd2 = search_character_online(char_status->account_id, char_status->char_id);
	if (fd2 >= 0) {	//Character online, send whisper.
		wd = mapif_create_whisper(fd, (char*)RFIFOP(fd, 4), (char*)RFIFOP(fd,28), (char*)RFIFOP(fd,52), RFIFOW(fd,2)-52);
		if (!wd) return 1;
		idb_put(wis_db, wd->id, wd);
		mapif_wis_message2(wd, fd2);
		return 0;
	}
	//Not found.
	return mapif_wis_fail(fd, (char*)RFIFOP(fd,4));
}

// Wisp/page transmission result
int mapif_parse_WisReply(int fd) {
	int id, flag;
	struct WisData *wd;
	RFIFOHEAD(fd);
	id = RFIFOL(fd,2);
	flag = RFIFOB(fd,6);
	wd = (struct WisData*)idb_get(wis_db, id);

	if (wd == NULL)
		return 0;	// This wisp was probably suppress before, because it was timeout or because of target was found on another map-server

	if ((--wd->count) <= 0 || flag != 1) {
		mapif_wis_end(wd, flag); // flag: 0: success to send wisper, 1: target character is not loged in?, 2: ignored by target
		idb_remove(wis_db, id);
	}

	return 0;
}

// Received wisp message from map-server for ALL gm (just copy the message and resends it to ALL map-servers)
int mapif_parse_WisToGM(int fd) {
	unsigned char buf[2048]; // 0x3003/0x3803 <packet_len>.w <wispname>.24B <min_gm_level>.w <message>.?B

	ShowDebug("Sent packet back!\n");
	memcpy(WBUFP(buf,0), RFIFOP(fd,0), RFIFOW(fd,2));
	WBUFW(buf, 0) = 0x3803;
	mapif_sendall(buf, RFIFOW(fd,2));

	return 0;
}

static void* create_accreg(DBKey key, va_list args) {
	struct accreg *reg;
	reg = (struct accreg*)aCalloc(sizeof(struct accreg), 1);
	reg->account_id = key.i;
	return reg;
}

// アカウント変数保存要求
int mapif_parse_Registry(int fd) {
	int j, p, len;
	struct accreg *reg;
	RFIFOHEAD(fd);

	switch (RFIFOB(fd,12)) {
		case 3: //Character registry
			return char_parse_Registry(RFIFOL(fd,4), RFIFOL(fd,8), RFIFOP(fd,13), RFIFOW(fd,2)-13);
		case 2: //Acc Reg
			break;
		case 1: //Acc Reg2, forward to login
			return save_accreg2(RFIFOP(fd,4), RFIFOW(fd,2)-4);
		default: //Error?
			return 1; 
	}
	reg = (struct accreg*)idb_ensure(accreg_db, RFIFOL(fd,4), create_accreg);

	for(j=0,p=13;j<ACCOUNT_REG_NUM && p<RFIFOW(fd,2);j++){
		sscanf((char*)RFIFOP(fd,p), "%31c%n",reg->reg[j].str,&len);
		reg->reg[j].str[len]='\0';
		p +=len+1; //+1 to skip the '\0' between strings.
		sscanf((char*)RFIFOP(fd,p), "%255c%n",reg->reg[j].value,&len);
		reg->reg[j].value[len]='\0';
		p +=len+1;
	}
	reg->reg_num=j;
	mapif_account_reg(fd, RFIFOP(fd,0));	// 他のMAPサーバーに送信

	return 0;
}

// Request the value of all registries.
int mapif_parse_RegistryRequest(int fd)
{	
	RFIFOHEAD(fd);
	//Load Char Registry
	if (RFIFOB(fd,12))
		char_account_reg_reply(fd,RFIFOL(fd,2),RFIFOL(fd,6));
	//Load Account Registry
	if (RFIFOB(fd,11))
		mapif_account_reg_reply(fd,RFIFOL(fd,2),RFIFOL(fd,6));
	//Ask Login Server for Account2 values.
	if (RFIFOB(fd,10))
		request_accreg2(RFIFOL(fd,2),RFIFOL(fd,6));
	return 1;
}

static void mapif_namechange_ack(int fd, int account_id, int char_id, int type, int flag, char *name){
	WFIFOHEAD(fd, NAME_LENGTH+13);
	WFIFOW(fd, 0) =0x3806;
	WFIFOL(fd, 2) =account_id;
	WFIFOL(fd, 6) =char_id;
	WFIFOB(fd,10) =type;
	WFIFOB(fd,11) =flag;
	memcpy(WFIFOP(fd, 12), name, NAME_LENGTH);
	WFIFOSET(fd, NAME_LENGTH+13);
}

int mapif_parse_NameChangeRequest(int fd)
{
	int account_id, char_id, type;
	char* name;
	int i;

	RFIFOHEAD(fd);
	account_id = RFIFOL(fd, 2);
	char_id = RFIFOL(fd, 6);
	type = RFIFOB(fd, 10);
	name = (char*)RFIFOP(fd, 11);

	// Check Authorised letters/symbols in the name
	if (char_name_option == 1) { // only letters/symbols in char_name_letters are authorised
		for (i = 0; i < NAME_LENGTH && name[i]; i++)
		if (strchr(char_name_letters, name[i]) == NULL) {
			mapif_namechange_ack(fd, account_id, char_id, type, 0, name);
			return 0;
		}
	} else if (char_name_option == 2) { // letters/symbols in char_name_letters are forbidden
		for (i = 0; i < NAME_LENGTH && name[i]; i++)
		if (strchr(char_name_letters, name[i]) != NULL) {
			mapif_namechange_ack(fd, account_id, char_id, type, 0, name);
			return 0;
		}
	}
	//TODO: type holds the type of object to rename.
	//If it were a player, it needs to have the guild information and db information
	//updated here, because changing it on the map won't make it be saved [Skotlex]

	//name allowed.
	mapif_namechange_ack(fd, account_id, char_id, type, 1, name);
	return 0;
}

//--------------------------------------------------------

// map server からの通信（１パケットのみ解析すること）
// エラーなら0(false)、処理できたなら1、
// パケット長が足りなければ2をかえさなければならない
int inter_parse_frommap(int fd) {
	int cmd, len;
	RFIFOHEAD(fd);
	cmd = RFIFOW(fd,0);
	len = 0;

	// inter鯖管轄かを調べる
	if (cmd < 0x3000 || cmd >= 0x3000 + ARRAYLENGTH(inter_recv_packet_length))
		return 0;
	
	if (inter_recv_packet_length[cmd-0x3000] == 0) //This is necessary, because otherwise we return 2 and the char server will just hang waiting for packets! [Skotlex]
		return 0;

	// パケット長を調べる
	if ((len = inter_check_length(fd, inter_recv_packet_length[cmd - 0x3000])) == 0)
		return 2;

	switch(cmd) {
	case 0x3000: mapif_parse_GMmessage(fd); break;
	case 0x3001: mapif_parse_WisRequest(fd); break;
	case 0x3002: mapif_parse_WisReply(fd); break;
	case 0x3003: mapif_parse_WisToGM(fd); break;
	case 0x3004: mapif_parse_Registry(fd); break;
	case 0x3005: mapif_parse_RegistryRequest(fd); break;
	case 0x3006: mapif_parse_NameChangeRequest(fd); break;
	default:
		if (inter_party_parse_frommap(fd))
			break;
		if (inter_guild_parse_frommap(fd))
			break;
		if (inter_storage_parse_frommap(fd))
			break;
		if (inter_pet_parse_frommap(fd))
			break;
		if (inter_homun_parse_frommap(fd))
			break;
		return 0;
	}
	RFIFOSKIP(fd, len);
	return 1;
}

// RFIFOのパケット長確認
// 必要パケット長があればパケット長、まだ足りなければ0
int inter_check_length(int fd, int length) {
	if (length == -1) {	// 可変パケット長
		RFIFOHEAD(fd);
		if (RFIFOREST(fd) < 4)	// パケット長が未着
			return 0;
		length = RFIFOW(fd,2);
	}

	if ((int)RFIFOREST(fd) < length)	// パケットが未着
		return 0;

	return length;
}
#endif //TXT_SQL_CONVERT
