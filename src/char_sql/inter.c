// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/mmo.h"
#include "../common/db.h"
#include "../common/malloc.h"
#include "../common/strlib.h"
#include "../common/showmsg.h"
#include "../common/socket.h"
#include "../common/timer.h"
#include "char.h"
#include "inter.h"
#include "int_party.h"
#include "int_guild.h"
#include "int_storage.h"
#include "int_pet.h"
#include "int_homun.h"
#include "int_mail.h"
#include "int_auction.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define WISDATA_TTL (60*1000)	// Wisデータの生存時間(60秒)
#define WISDELLIST_MAX 256			// Wisデータ削除リストの要素数


Sql* sql_handle = NULL;
Sql* lsql_handle = NULL;

int char_server_port = 3306;
char char_server_ip[32] = "127.0.0.1";
char char_server_id[32] = "ragnarok";
char char_server_pw[32] = "ragnarok";
char char_server_db[32] = "ragnarok";
char default_codepage[32] = ""; //Feature by irmin.

int login_server_port = 3306;
char login_server_ip[32] = "127.0.0.1";
char login_server_id[32] = "ragnarok";
char login_server_pw[32] = "ragnarok";
char login_server_db[32] = "ragnarok";

#ifndef TXT_SQL_CONVERT

static struct accreg *accreg_pt;
unsigned int party_share_level = 10;
char main_chat_nick[16] = "Main";

// recv. packet list
int inter_recv_packet_length[] = {
	-1,-1, 7,-1, -1,13,36, 0,  0, 0, 0, 0,  0, 0,  0, 0,	// 3000-
	 6,-1, 0, 0,  0, 0, 0, 0, 10,-1, 0, 0,  0, 0,  0, 0,	// 3010-
	-1, 6,-1,14, 14,19, 6,-1, 14,14, 0, 0,  0, 0,  0, 0,	// 3020-
	-1, 6,-1,-1, 55,19, 6,-1, 14,-1,-1,-1, 14,19,186,-1,	// 3030-
	 5, 9, 0, 0,  0, 0, 0, 0,  7, 6,10,10, 10,-1,  0, 0,	// 3040-
	-1,-1,10,10,  0,-1, 0, 0,  0, 0, 0, 0,  0, 0,  0, 0,	// 3050-  Auction System [Zephyrus]
	 0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0,  0, 0,	// 3060-
	 0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0,  0, 0,	// 3070-
	48,14,-1, 6,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0,  0, 0,	// 3080-
	-1,10,-1, 6,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0,  0, 0,	// 3090-  Homunculus packets [albator]
};

struct WisData {
	int id, fd, count, len;
	unsigned long tick;
	unsigned char src[24], dst[24], msg[512];
};
static DBMap* wis_db = NULL; // int wis_id -> struct WisData*
static int wis_dellist[WISDELLIST_MAX], wis_delnum;

int inter_sql_test (void);

#endif //TXT_SQL_CONVERT
//--------------------------------------------------------
// Save registry to sql
int inter_accreg_tosql(int account_id, int char_id, struct accreg* reg, int type)
{
	struct global_reg* r;
	SqlStmt* stmt;
	int i;

	if( account_id <= 0 )
		return 0;
	reg->account_id = account_id;
	reg->char_id = char_id;

	//`global_reg_value` (`type`, `account_id`, `char_id`, `str`, `value`)
	switch( type )
	{
	case 3: //Char Reg
		if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `type`=3 AND `char_id`='%d'", reg_db, char_id) )
			Sql_ShowDebug(sql_handle);
		account_id = 0;
		break;
	case 2: //Account Reg
		if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `type`=2 AND `account_id`='%d'", reg_db, account_id) )
			Sql_ShowDebug(sql_handle);
		char_id = 0;
		break;
	case 1: //Account2 Reg
		ShowError("inter_accreg_tosql: Char server shouldn't handle type 1 registry values (##). That is the login server's work!\n");
		return 0;
	default:
		ShowError("inter_accreg_tosql: Invalid type %d\n", type);
		return 0;
	}

	if( reg->reg_num <= 0 )
		return 0;

	stmt = SqlStmt_Malloc(sql_handle);
	if( SQL_ERROR == SqlStmt_Prepare(stmt, "INSERT INTO `%s` (`type`, `account_id`, `char_id`, `str`, `value`) VALUES ('%d','%d','%d',?,?)", reg_db, type, account_id, char_id) )
		SqlStmt_ShowDebug(stmt);
	for( i = 0; i < reg->reg_num; ++i )
	{
		r = &reg->reg[i];
		if( r->str[0] != '\0' && r->value != '\0' )
		{
			// str
			SqlStmt_BindParam(stmt, 0, SQLDT_STRING, r->str, strnlen(r->str, sizeof(r->str)));
			// value
			SqlStmt_BindParam(stmt, 1, SQLDT_STRING, r->value, strnlen(r->value, sizeof(r->value)));

			if( SQL_ERROR == SqlStmt_Execute(stmt) )
				SqlStmt_ShowDebug(stmt);
		}
	}
	SqlStmt_Free(stmt);
	return 1;
}
#ifndef TXT_SQL_CONVERT

// Load account_reg from sql (type=2)
int inter_accreg_fromsql(int account_id,int char_id, struct accreg *reg, int type)
{
	struct global_reg* r;
	char* data;
	size_t len;
	int i;

	if( reg == NULL)
		return 0;

	memset(reg, 0, sizeof(struct accreg));
	reg->account_id = account_id;
	reg->char_id = char_id;

	//`global_reg_value` (`type`, `account_id`, `char_id`, `str`, `value`)
	switch( type )
	{
	case 3: //char reg
		if( SQL_ERROR == Sql_Query(sql_handle, "SELECT `str`, `value` FROM `%s` WHERE `type`=3 AND `char_id`='%d'", reg_db, char_id) )
			Sql_ShowDebug(sql_handle);
		break;
	case 2: //account reg
		if( SQL_ERROR == Sql_Query(sql_handle, "SELECT `str`, `value` FROM `%s` WHERE `type`=2 AND `account_id`='%d'", reg_db, account_id) )
			Sql_ShowDebug(sql_handle);
		break;
	case 1: //account2 reg
		ShowError("inter_accreg_fromsql: Char server shouldn't handle type 1 registry values (##). That is the login server's work!\n");
		return 0;
	default:
		ShowError("inter_accreg_fromsql: Invalid type %d\n", type);
		return 0;
	}
	for( i = 0; i < MAX_REG_NUM && SQL_SUCCESS == Sql_NextRow(sql_handle); ++i )
	{
		r = &reg->reg[i];
		// str
		Sql_GetData(sql_handle, 0, &data, &len);
		memcpy(r->str, data, min(len, sizeof(r->str)));
		// value
		Sql_GetData(sql_handle, 1, &data, &len);
		memcpy(r->value, data, min(len, sizeof(r->value)));
	}
	reg->reg_num = i;
	Sql_FreeResult(sql_handle);
	return 1;
}

// Initialize
int inter_accreg_sql_init(void)
{
	CREATE(accreg_pt, struct accreg, 1);
	return 0;

}
#endif //TXT_SQL_CONVERT

/*==========================================
 * read config file
 *------------------------------------------*/
static int inter_config_read(const char* cfgName)
{
	int i;
	char line[1024], w1[1024], w2[1024];
	FILE* fp;

	fp = fopen(cfgName, "r");
	if(fp == NULL) {
		ShowError("file not found: %s\n", cfgName);
		return 1;
	}

	ShowInfo("reading file %s...\n", cfgName);

	while(fgets(line, sizeof(line), fp))
	{
		i = sscanf(line, "%[^:]: %[^\r\n]", w1, w2);
		if(i != 2)
			continue;

		if(!strcmpi(w1,"char_server_ip")) {
			strcpy(char_server_ip,w2);
			ShowStatus ("set char_server_ip : %s\n", w2);
		} else
		if(!strcmpi(w1,"char_server_port")) {
			char_server_port = atoi(w2);
			ShowStatus ("set char_server_port : %s\n", w2);
		} else
		if(!strcmpi(w1,"char_server_id")) {
			strcpy(char_server_id,w2);
			ShowStatus ("set char_server_id : %s\n", w2);
		} else
		if(!strcmpi(w1,"char_server_pw")) {
			strcpy(char_server_pw,w2);
			ShowStatus ("set char_server_pw : %s\n", w2);
		} else
		if(!strcmpi(w1,"char_server_db")) {
			strcpy(char_server_db,w2);
			ShowStatus ("set char_server_db : %s\n", w2);
		} else
		if(!strcmpi(w1,"default_codepage")) {
			strcpy(default_codepage,w2);
			ShowStatus ("set default_codepage : %s\n", w2);
		}
		//Logins information to be read from the inter_athena.conf
		//for character deletion (checks email in the loginDB)
		else
		if(!strcmpi(w1,"login_server_ip")) {
			strcpy(login_server_ip, w2);
			ShowStatus ("set login_server_ip : %s\n", w2);
		} else
		if(!strcmpi(w1,"login_server_port")) {
			login_server_port = atoi(w2);
			ShowStatus ("set login_server_port : %s\n", w2);
		} else
		if(!strcmpi(w1,"login_server_id")) {
			strcpy(login_server_id, w2);
			ShowStatus ("set login_server_id : %s\n", w2);
		} else
		if(!strcmpi(w1,"login_server_pw")) {
			strcpy(login_server_pw, w2);
			ShowStatus ("set login_server_pw : %s\n", w2);
		} else
		if(!strcmpi(w1,"login_server_db")) {
			strcpy(login_server_db, w2);
			ShowStatus ("set login_server_db : %s\n", w2);
		}
#ifndef TXT_SQL_CONVERT
		else if(!strcmpi(w1,"party_share_level"))
			party_share_level = atoi(w2);
		else if(!strcmpi(w1,"log_inter"))
			log_inter = atoi(w2);
		else if(!strcmpi(w1,"main_chat_nick"))
			strcpy(main_chat_nick, w2);
#endif //TXT_SQL_CONVERT
		else if(!strcmpi(w1,"import"))
			inter_config_read(w2);
	}
	fclose(fp);

	ShowInfo ("done reading %s.\n", cfgName);

	return 0;
}
#ifndef TXT_SQL_CONVERT

// Save interlog into sql
int inter_log(char* fmt, ...)
{
	char str[255];
	char esc_str[sizeof(str)*2+1];// escaped str
	va_list ap;

	va_start(ap,fmt);
	vsnprintf(str, sizeof(str), fmt, ap);
	va_end(ap);

	Sql_EscapeStringLen(sql_handle, esc_str, str, strnlen(str, sizeof(str)));
	if( SQL_ERROR == Sql_Query(sql_handle, "INSERT INTO `%s` (`time`, `log`) VALUES (NOW(),  '%s')", interlog_db, esc_str) )
		Sql_ShowDebug(sql_handle);

	return 0;
}

/*=============================================
 * Does a mysql_ping to all connection handles
 *---------------------------------------------*/
int inter_sql_ping(int tid, unsigned int tick, int id, int data) 
{
	ShowInfo("Pinging SQL server to keep connection alive...\n");
	Sql_Ping(sql_handle);
	if( char_gm_read )
		Sql_Ping(lsql_handle);
	return 0;
}


int sql_ping_init(void)
{
	uint32 connection_timeout, connection_ping_interval;

	// set a default value first
	connection_timeout = 28800; // 8 hours

	// ask the mysql server for the timeout value
	if( SQL_SUCCESS == Sql_GetTimeout(sql_handle, &connection_timeout) && connection_timeout < 60 )
		connection_timeout = 60;

	// establish keepalive
	connection_ping_interval = connection_timeout - 30; // 30-second reserve
	add_timer_func_list(inter_sql_ping, "inter_sql_ping");
	add_timer_interval(gettick() + connection_ping_interval*1000, inter_sql_ping, 0, 0, connection_ping_interval*1000);

	return 0;
}

#endif //TXT_SQL_CONVERT

// initialize
int inter_init_sql(const char *file)
{
	//int i;

	ShowInfo ("interserver initialize...\n");
	inter_config_read(file);

	//DB connection initialized
	sql_handle = Sql_Malloc();
	ShowInfo("Connect Character DB server.... (Character Server)\n");
	if( SQL_ERROR == Sql_Connect(sql_handle, char_server_id, char_server_pw, char_server_ip, (uint16)char_server_port, char_server_db) )
	{
		Sql_ShowDebug(sql_handle);
		Sql_Free(sql_handle);
		exit(EXIT_FAILURE);
	}
#ifndef TXT_SQL_CONVERT
	else if (inter_sql_test()) {
		ShowStatus("Connect Success! (Character Server)\n");
	}

	if(char_gm_read) {
		lsql_handle = Sql_Malloc();
		ShowInfo("Connect Character DB server.... (login server)\n");
		if( SQL_ERROR == Sql_Connect(lsql_handle, login_server_id, login_server_pw, login_server_ip, (uint16)login_server_port, login_server_db) )
		{
			Sql_ShowDebug(lsql_handle);
			Sql_Free(lsql_handle);
			Sql_Free(sql_handle);
			exit(EXIT_FAILURE);
		}
		else
		{
			ShowStatus ("Connect Success! (Login Server)\n");
		}
	}
#endif //TXT_SQL_CONVERT
	if( *default_codepage ) {
		if( SQL_ERROR == Sql_SetEncoding(sql_handle, default_codepage) )
			Sql_ShowDebug(sql_handle);
#ifndef TXT_SQL_CONVERT
		if( char_gm_read && SQL_ERROR == Sql_SetEncoding(lsql_handle, default_codepage) )
			Sql_ShowDebug(lsql_handle);
#endif //TXT_SQL_CONVERT
	}

#ifndef TXT_SQL_CONVERT
	wis_db = idb_alloc(DB_OPT_RELEASE_DATA);
	inter_guild_sql_init();
	inter_storage_sql_init();
	inter_party_sql_init();
	inter_pet_sql_init();
	inter_homunculus_sql_init(); // albator
	inter_accreg_sql_init();
	inter_mail_sql_init();
	inter_auction_sql_init();

	sql_ping_init();
#endif //TXT_SQL_CONVERT
	return 0;
}
#ifndef TXT_SQL_CONVERT

int inter_sql_test (void)
{
	const char fields[][24] = {
		"father",	// version 1363
		"fame",		// version 1491
	};	
	char buf[1024] = "";
	char* p;
	size_t len;
	int i;

	if( SQL_ERROR == Sql_GetColumnNames(sql_handle, char_db, buf, sizeof(buf), '\n') )
		Sql_ShowDebug(sql_handle);

	// check DB strings
	for( i = 0; i < ARRAYLENGTH(fields); ++i )
	{
		len = strlen(fields[i]);
		p = strstr(buf, fields[i]);
		while( p != NULL && p[len] != '\n' )
			p = strstr(p, fields[i]);
		if( p == NULL )
		{
			ShowSQL ("Field `%s` not be found in `%s`. Consider updating your database!\n", fields[i], char_db);
			if( lsql_handle )
				Sql_Free(lsql_handle);
			Sql_Free(sql_handle);
			exit(EXIT_FAILURE);
		}
	}

	return 1;
}

// finalize
void inter_final(void)
{
	wis_db->destroy(wis_db, NULL);

	inter_guild_sql_final();
	inter_storage_sql_final();
	inter_party_sql_final();
	inter_pet_sql_final();
	inter_homunculus_sql_final();	//[orn]
	inter_mail_sql_final();
	inter_auction_sql_final();
	
	if (accreg_pt) aFree(accreg_pt);
	return;
}

int inter_mapif_init(int fd)
{
	inter_guild_mapif_init(fd);

	return 0;
}


//--------------------------------------------------------

// GM message sending
int mapif_GMmessage(unsigned char *mes, int len, unsigned long color, int sfd)
{
	unsigned char buf[2048];

	if (len > 2048) len = 2047; //Make it fit to avoid crashes. [Skotlex]
	WBUFW(buf,0) = 0x3800;
	WBUFW(buf,2) = len;
	WBUFL(buf,4) = color;
	memcpy(WBUFP(buf,8), mes, len - 8);
	mapif_sendallwos(sfd, buf, len);
	return 0;
}

// Wis sending
int mapif_wis_message(struct WisData *wd)
{
	unsigned char buf[2048];
	if (wd->len > 2047-56) wd->len = 2047-56; //Force it to fit to avoid crashes. [Skotlex]

	WBUFW(buf, 0) = 0x3801;
	WBUFW(buf, 2) = 56 +wd->len;
	WBUFL(buf, 4) = wd->id;
	memcpy(WBUFP(buf, 8), wd->src, NAME_LENGTH);
	memcpy(WBUFP(buf,32), wd->dst, NAME_LENGTH);
	memcpy(WBUFP(buf,56), wd->msg, wd->len);
	wd->count = mapif_sendall(buf,WBUFW(buf,2));

	return 0;
}

// Wis sending result
int mapif_wis_end(struct WisData *wd, int flag)
{
	unsigned char buf[27];

	WBUFW(buf, 0)=0x3802;
	memcpy(WBUFP(buf, 2),wd->src,24);
	WBUFB(buf,26)=flag;
	mapif_send(wd->fd,buf,27);
	return 0;
}

// Account registry transfer to map-server
static void mapif_account_reg(int fd, unsigned char *src)
{
	WBUFW(src,0)=0x3804; //NOTE: writing to RFIFO
	mapif_sendallwos(fd, src, WBUFW(src,2));
}

// Send the requested account_reg
int mapif_account_reg_reply(int fd,int account_id,int char_id, int type)
{
	struct accreg *reg=accreg_pt;
	WFIFOHEAD(fd, 13 + 5000);
	inter_accreg_fromsql(account_id,char_id,reg,type);
	
	WFIFOW(fd,0)=0x3804;
	WFIFOL(fd,4)=account_id;
	WFIFOL(fd,8)=char_id;
	WFIFOB(fd,12)=type;
	if(reg->reg_num==0){
		WFIFOW(fd,2)=13;
	}else{
		int i,p;
		for (p=13,i = 0; i < reg->reg_num && p < 5000; i++) {
			p+= sprintf((char*)WFIFOP(fd,p), "%s", reg->reg[i].str)+1; //We add 1 to consider the '\0' in place.
			p+= sprintf((char*)WFIFOP(fd,p), "%s", reg->reg[i].value)+1;
		}
		WFIFOW(fd,2)=p;
		if (p>= 5000)
			ShowWarning("Too many acc regs for %d:%d, not all values were loaded.\n", account_id, char_id);
	}
	WFIFOSET(fd,WFIFOW(fd,2));
	return 0;
}

int mapif_send_gmaccounts()
{
	int i, len = 4;
	unsigned char buf[32000];

	// forward the gm accounts to the map server
	len = 4;
	WBUFW(buf,0) = 0x2b15;
				
	for(i = 0; i < GM_num; i++) {
		WBUFL(buf,len) = gm_account[i].account_id;
		WBUFB(buf,len+4) = (uint8)gm_account[i].level;
		len += 5;
	}
	WBUFW(buf,2) = len;
	mapif_sendall(buf, len);

	return 0;
}

//Sends the current max account/char id to map server [Skotlex]
void mapif_send_maxid(int account_id, int char_id)
{
	unsigned char buf[12];

	WBUFW(buf,0) = 0x2b07;
	WBUFL(buf,2) = account_id;
	WBUFL(buf,6) = char_id;
	mapif_sendall(buf, 10);
}

//Request to kick char from a certain map server. [Skotlex]
int mapif_disconnectplayer(int fd, int account_id, int char_id, int reason)
{
	if (fd >= 0)
	{
		WFIFOHEAD(fd,7);
		WFIFOW(fd,0) = 0x2b1f;
		WFIFOL(fd,2) = account_id;
		WFIFOB(fd,6) = reason;
		WFIFOSET(fd,7);
		return 0;
	}
	return -1;
}

//--------------------------------------------------------

// Existence check of WISP data
int check_ttl_wisdata_sub(DBKey key, void *data, va_list ap)
{
	unsigned long tick;
	struct WisData *wd = (struct WisData *)data;
	tick = va_arg(ap, unsigned long);

	if (DIFF_TICK(tick, wd->tick) > WISDATA_TTL && wis_delnum < WISDELLIST_MAX)
		wis_dellist[wis_delnum++] = wd->id;

	return 0;
}

int check_ttl_wisdata(void)
{
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

// GM message sending
int mapif_parse_GMmessage(int fd)
{
	mapif_GMmessage(RFIFOP(fd,8), RFIFOW(fd,2), RFIFOL(fd,4), fd);
	return 0;
}


// Wisp/page request to send
int mapif_parse_WisRequest(int fd)
{
	struct WisData* wd;
	static int wisid = 0;
	char name[NAME_LENGTH];
	char esc_name[NAME_LENGTH*2+1];// escaped name
	char* data;
	size_t len;


	if ( fd <= 0 ) {return 0;} // check if we have a valid fd

	if (RFIFOW(fd,2)-52 >= sizeof(wd->msg)) {
		ShowWarning("inter: Wis message size too long.\n");
		return 0;
	} else if (RFIFOW(fd,2)-52 <= 0) { // normaly, impossible, but who knows...
		ShowError("inter: Wis message doesn't exist.\n");
		return 0;
	}
	
	safestrncpy(name, (char*)RFIFOP(fd,28), NAME_LENGTH); //Received name may be too large and not contain \0! [Skotlex]

	Sql_EscapeStringLen(sql_handle, esc_name, name, strnlen(name, NAME_LENGTH));
	if( SQL_ERROR == Sql_Query(sql_handle, "SELECT `name` FROM `%s` WHERE `name`='%s'", char_db, esc_name) )
		Sql_ShowDebug(sql_handle);

	// search if character exists before to ask all map-servers
	if( SQL_SUCCESS != Sql_NextRow(sql_handle) )
	{
		unsigned char buf[27];
		WBUFW(buf, 0) = 0x3802;
		memcpy(WBUFP(buf, 2), RFIFOP(fd, 4), NAME_LENGTH);
		WBUFB(buf,26) = 1; // flag: 0: success to send wisper, 1: target character is not loged in?, 2: ignored by target
		mapif_send(fd, buf, 27);
	}
	else
	{// Character exists. So, ask all map-servers
		// to be sure of the correct name, rewrite it
		Sql_GetData(sql_handle, 0, &data, &len);
		memset(name, 0, NAME_LENGTH);
		memcpy(name, data, min(len, NAME_LENGTH));
		// if source is destination, don't ask other servers.
		if( strncmp((const char*)RFIFOP(fd,4), name, NAME_LENGTH) == 0 )
		{
			uint8 buf[27];
			WBUFW(buf, 0) = 0x3802;
			memcpy(WBUFP(buf, 2), RFIFOP(fd, 4), NAME_LENGTH);
			WBUFB(buf,26) = 1; // flag: 0: success to send wisper, 1: target character is not loged in?, 2: ignored by target
			mapif_send(fd, buf, 27);
		}
		else
		{

			CREATE(wd, struct WisData, 1);

			// Whether the failure of previous wisp/page transmission (timeout)
			check_ttl_wisdata();

			wd->id = ++wisid;
			wd->fd = fd;
			wd->len= RFIFOW(fd,2)-52;
			memcpy(wd->src, RFIFOP(fd, 4), NAME_LENGTH);
			memcpy(wd->dst, RFIFOP(fd,28), NAME_LENGTH);
			memcpy(wd->msg, RFIFOP(fd,52), wd->len);
			wd->tick = gettick();
			idb_put(wis_db, wd->id, wd);
			mapif_wis_message(wd);
		}
	}

	Sql_FreeResult(sql_handle);
	return 0;
}


// Wisp/page transmission result
int mapif_parse_WisReply(int fd)
{
	int id, flag;
	struct WisData *wd;

	id = RFIFOL(fd,2);
	flag = RFIFOB(fd,6);
	wd = (struct WisData*)idb_get(wis_db, id);
	if (wd == NULL)
		return 0;	// This wisp was probably suppress before, because it was timeout of because of target was found on another map-server

	if ((--wd->count) <= 0 || flag != 1) {
		mapif_wis_end(wd, flag); // flag: 0: success to send wisper, 1: target character is not loged in?, 2: ignored by target
		idb_remove(wis_db, id);
	}

	return 0;
}

// Received wisp message from map-server for ALL gm (just copy the message and resends it to ALL map-servers)
int mapif_parse_WisToGM(int fd)
{
	unsigned char buf[2048]; // 0x3003/0x3803 <packet_len>.w <wispname>.24B <min_gm_level>.w <message>.?B
	
	ShowDebug("Sent packet back!\n");
	memcpy(WBUFP(buf,0), RFIFOP(fd,0), RFIFOW(fd,2));
	WBUFW(buf, 0) = 0x3803;
	mapif_sendall(buf, RFIFOW(fd,2));

	return 0;
}

// Save account_reg into sql (type=2)
int mapif_parse_Registry(int fd)
{
	int j,p,len, max;
	struct accreg *reg=accreg_pt;
	
	memset(accreg_pt,0,sizeof(struct accreg));
	switch (RFIFOB(fd, 12)) {
	case 3: //Character registry
		max = GLOBAL_REG_NUM;
	break;
	case 2: //Account Registry
		max = ACCOUNT_REG_NUM;
	break;
	case 1: //Account2 registry, must be sent over to login server.
		return save_accreg2(RFIFOP(fd,4), RFIFOW(fd,2)-4);
	default:
		return 1;
	}
	for(j=0,p=13;j<max && p<RFIFOW(fd,2);j++){
		sscanf((char*)RFIFOP(fd,p), "%31c%n",reg->reg[j].str,&len);
		reg->reg[j].str[len]='\0';
		p +=len+1; //+1 to skip the '\0' between strings.
		sscanf((char*)RFIFOP(fd,p), "%255c%n",reg->reg[j].value,&len);
		reg->reg[j].value[len]='\0';
		p +=len+1;
	}
	reg->reg_num=j;

	inter_accreg_tosql(RFIFOL(fd,4),RFIFOL(fd,8),reg, RFIFOB(fd,12));
	mapif_account_reg(fd,RFIFOP(fd,0));	// Send updated accounts to other map servers.
	return 0;
}

// Request the value of all registries.
int mapif_parse_RegistryRequest(int fd)
{
	//Load Char Registry
	if (RFIFOB(fd,12)) mapif_account_reg_reply(fd,RFIFOL(fd,2),RFIFOL(fd,6),3);
	//Load Account Registry
	if (RFIFOB(fd,11)) mapif_account_reg_reply(fd,RFIFOL(fd,2),RFIFOL(fd,6),2);
	//Ask Login Server for Account2 values.
	if (RFIFOB(fd,10)) request_accreg2(RFIFOL(fd,2),RFIFOL(fd,6));
	return 1;
}

static void mapif_namechange_ack(int fd, int account_id, int char_id, int type, int flag, char *name)
{
	WFIFOHEAD(fd, NAME_LENGTH+13);
	WFIFOW(fd, 0) = 0x3806;
	WFIFOL(fd, 2) = account_id;
	WFIFOL(fd, 6) = char_id;
	WFIFOB(fd,10) = type;
	WFIFOB(fd,11) = flag;
	memcpy(WFIFOP(fd, 12), name, NAME_LENGTH);
	WFIFOSET(fd, NAME_LENGTH+13);
}

int mapif_parse_NameChangeRequest(int fd)
{
	int account_id, char_id, type;
	char* name;
	int i;

	account_id = RFIFOL(fd,2);
	char_id = RFIFOL(fd,6);
	type = RFIFOB(fd,10);
	name = (char*)RFIFOP(fd,11);

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
int inter_parse_frommap(int fd)
{
	int cmd;
	int len = 0;
	cmd = RFIFOW(fd,0);
	// inter鯖管轄かを調べる
	if(cmd < 0x3000 || cmd >= 0x3000 + ARRAYLENGTH(inter_recv_packet_length) || inter_recv_packet_length[cmd - 0x3000] == 0)
		return 0;

	// パケット長を調べる
	if((len = inter_check_length(fd, inter_recv_packet_length[cmd - 0x3000])) == 0)
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
		if(  inter_party_parse_frommap(fd)
		  || inter_guild_parse_frommap(fd)
		  || inter_storage_parse_frommap(fd)
		  || inter_pet_parse_frommap(fd)
		  || inter_homunculus_parse_frommap(fd)
		  || inter_mail_parse_frommap(fd)
		  || inter_auction_parse_frommap(fd)
		   )
			break;
		else
			return 0;
	}

	RFIFOSKIP(fd, len);
	return 1;
}

// RFIFO check
int inter_check_length(int fd, int length)
{
	if(length == -1) {	// v-len packet
		if(RFIFOREST(fd) < 4)	// packet not yet
			return 0;
		length = RFIFOW(fd, 2);
	}

	if((int)RFIFOREST(fd) < length)	// packet not yet
		return 0;

	return length;
}
#endif //TXT_SQL_CONVERT
