// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "char.h"
#include "../common/malloc.h"
#include "../common/strlib.h"
#include "../common/showmsg.h"
#include "inter.h"
#include "int_party.h"
#include "int_guild.h"
#include "int_storage.h"
#include "int_pet.h"
#include "int_homun.h" //albator

#define WISDATA_TTL (60*1000)	// Wisデータの生存時間(60秒)
#define WISDELLIST_MAX 256			// Wisデータ削除リストの要素数


MYSQL mysql_handle;
MYSQL_RES* 	sql_res ;
MYSQL_ROW	sql_row ;
int sql_fields, sql_cnt;
char tmp_sql[65535];

MYSQL lmysql_handle;
MYSQL_RES* 	lsql_res ;
MYSQL_ROW	lsql_row ;

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

// sending packet list
// NOTE: This variable ain't used at all! And it's confusing.. where do I add that the length of packet 0x2b07 is 10? x.x [Skotlex]
int inter_send_packet_length[]={
	-1,-1,27,-1, -1, 0, 0, 0,  0, 0, 0, 0,  0, 0,  0, 0,
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
	 5, 9, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0,  0, 0,
	 0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0,  0, 0,
	 0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0,  0, 0,
	 0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0,  0, 0,
	48,14,-1, 6,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0,  0, 0, //0x3080-0x308f
	-1,10,-1, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0x3090 - 0x309f  Homunculus packets [albator]
};

struct WisData {
	int id, fd, count, len;
	unsigned long tick;
	unsigned char src[24], dst[24], msg[512];
};
static struct dbt * wis_db = NULL;
static int wis_dellist[WISDELLIST_MAX], wis_delnum;

int inter_sql_test (void);

#endif //TXT_SQL_CONVERT
//--------------------------------------------------------
// Save registry to sql
int inter_accreg_tosql(int account_id, int char_id, struct accreg *reg, int type){

	int j;
	char temp_str[64]; //Needs be twice the source to ensure it fits [Skotlex]
	char temp_str2[512];
	if (account_id<=0) return 0;
	reg->account_id=account_id;
	reg->char_id = char_id;

	switch (type) {
		case 3: //Char Reg
		//`global_reg_value` (`type`, `account_id`, `char_id`, `str`, `value`)
			sprintf(tmp_sql,"DELETE FROM `%s` WHERE `type`=3 AND `char_id`='%d'",reg_db, char_id);
			break;
		case 2: //Account Reg
		//`global_reg_value` (`type`, `account_id`, `char_id`, `str`, `value`)
			sprintf(tmp_sql,"DELETE FROM `%s` WHERE `type`=2 AND `account_id`='%d'",reg_db, account_id);
			break;
		case 1: //Account2 Reg
			ShowError("inter_accreg_tosql: Char server shouldn't handle type 1 registry values (##). That is the login server's work!\n");
			return 0;
		default:
			ShowError("inter_accreg_tosql: Invalid type %d\n", type);
			return 0;
			
	}	
	if(mysql_query(&mysql_handle, tmp_sql) ) {
		ShowSQL("DB error - %s\n",mysql_error(&mysql_handle));
		ShowDebug("at %s:%d - %s\n", __FILE__,__LINE__,tmp_sql);
	}

	if (reg->reg_num<=0) return 0;

	for(j=0;j<reg->reg_num;j++){
		if(reg->reg[j].str != NULL){
			sprintf(tmp_sql,"INSERT INTO `%s` (`type`, `account_id`, `char_id`, `str`, `value`) VALUES ('%d','%d','%d','%s','%s')",
				reg_db, type, type!=3?reg->account_id:0, type==3?reg->char_id:0,
				jstrescapecpy(temp_str,reg->reg[j].str), jstrescapecpy(temp_str2,reg->reg[j].value));
			if(mysql_query(&mysql_handle, tmp_sql) ) {
				ShowSQL("DB error - %s\n",mysql_error(&mysql_handle));
				ShowDebug("at %s:%d - %s\n", __FILE__,__LINE__,tmp_sql);
			}
		}
	}
	return 1;
}
#ifndef TXT_SQL_CONVERT

// Load account_reg from sql (type=2)
int inter_accreg_fromsql(int account_id,int char_id, struct accreg *reg, int type)
{
	int j=0;
	if (reg==NULL) return 0;
	memset(reg, 0, sizeof(struct accreg));
	reg->account_id=account_id;
	reg->char_id=char_id;

	//`global_reg_value` (`type`, `account_id`, `char_id`, `str`, `value`)
	switch (type) {
	case 3: //char reg
		sprintf (tmp_sql, "SELECT `str`, `value` FROM `%s` WHERE `type`=3 AND `char_id`='%d'",reg_db, reg->char_id);
	break;
	case 2: //account reg
		sprintf (tmp_sql, "SELECT `str`, `value` FROM `%s` WHERE `type`=2 AND `account_id`='%d'",reg_db, reg->account_id);
	break;
	case 1: //account2 reg
		ShowError("inter_accreg_fromsql: Char server shouldn't handle type 1 registry values (##). That is the login server's work!\n");
		return 0;
	}
	if(mysql_query(&mysql_handle, tmp_sql) ) {
		ShowSQL("DB error - %s\n",mysql_error(&mysql_handle));
		ShowDebug("at %s:%d - %s\n", __FILE__,__LINE__,tmp_sql);
	}
	sql_res = mysql_store_result(&mysql_handle);

	if (sql_res) {
		for(j=0;(sql_row = mysql_fetch_row(sql_res));j++){
			strcpy(reg->reg[j].str, sql_row[0]);
			strcpy(reg->reg[j].value, sql_row[1]);
		}
		mysql_free_result(sql_res);
	}
	reg->reg_num=j;
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
static int inter_config_read(const char *cfgName)
{
	int i;
	char line[1024], w1[1024], w2[1024];
	FILE *fp;

	fp=fopen(cfgName,"r");
	if(fp==NULL){
		ShowError("file not found: %s\n", cfgName);
		return 1;
	}

	ShowInfo("reading file %s...\n",cfgName);

	while(fgets(line, sizeof(line), fp))
	{
		i=sscanf(line,"%[^:]: %[^\r\n]",w1,w2);
		if(i!=2)
			continue;

		if(strcmpi(w1,"char_server_ip")==0){
			strcpy(char_server_ip, w2);
			ShowStatus ("set char_server_ip : %s\n",w2);
		}
		else if(strcmpi(w1,"char_server_port")==0){
			char_server_port=atoi(w2);
			ShowStatus ("set char_server_port : %s\n",w2);
		}
		else if(strcmpi(w1,"char_server_id")==0){
			strcpy(char_server_id, w2);
			ShowStatus ("set char_server_id : %s\n",w2);
		}
		else if(strcmpi(w1,"char_server_pw")==0){
			strcpy(char_server_pw, w2);
			ShowStatus ("set char_server_pw : %s\n",w2);
		}
		else if(strcmpi(w1,"char_server_db")==0){
			strcpy(char_server_db, w2);
			ShowStatus ("set char_server_db : %s\n",w2);
		}
		else if(strcmpi(w1,"default_codepage")==0){
			strcpy(default_codepage, w2);
			ShowStatus ("set default_codepage : %s\n",w2);
		}
		//Logins information to be read from the inter_athena.conf
		//for character deletion (checks email in the loginDB)
		else if(strcmpi(w1,"login_server_ip")==0){
			strcpy(login_server_ip, w2);
			ShowStatus ("set login_server_ip : %s\n",w2);
		}
		else if(strcmpi(w1,"login_server_port")==0){
			login_server_port=atoi(w2);
			ShowStatus ("set login_server_port : %s\n",w2);
		}
		else if(strcmpi(w1,"login_server_id")==0){
			strcpy(login_server_id, w2);
			ShowStatus ("set login_server_id : %s\n",w2);
		}
		else if(strcmpi(w1,"login_server_pw")==0){
			strcpy(login_server_pw, w2);
			ShowStatus ("set login_server_pw : %s\n",w2);
		}
		else if(strcmpi(w1,"login_server_db")==0){
			strcpy(login_server_db, w2);
			ShowStatus ("set login_server_db : %s\n",w2);
		}
#ifndef TXT_SQL_CONVERT
		else if(strcmpi(w1,"party_share_level")==0){
			party_share_level=(unsigned int)atof(w2);
		}
		else if(strcmpi(w1,"log_inter")==0){
			log_inter = atoi(w2);
		}
		else if(strcmpi(w1, "main_chat_nick")==0){	// Main chat nick [LuzZza]
			strcpy(main_chat_nick, w2);				// 			
		}
#endif //TXT_SQL_CONVERT
		else if(strcmpi(w1,"import")==0){
			inter_config_read(w2);
		}
	}
	fclose(fp);

	ShowInfo ("done reading %s.\n", cfgName);

	return 0;
}
#ifndef TXT_SQL_CONVERT

// Save interlog into sql
int inter_log(char *fmt,...)
{
	char str[255];
	char temp_str[510]; //Needs be twice as long as str[] //Skotlex
	va_list ap;
	va_start(ap,fmt);

	vsprintf(str,fmt,ap);
	sprintf(tmp_sql,"INSERT INTO `%s` (`time`, `log`) VALUES (NOW(),  '%s')",interlog_db, jstrescapecpy(temp_str,str));
	if(mysql_query(&mysql_handle, tmp_sql) ) {
		ShowSQL("DB error - %s\n",mysql_error(&mysql_handle));
		ShowDebug("at %s:%d - %s\n", __FILE__,__LINE__,tmp_sql);
	}

	va_end(ap);
	return 0;
}

/*=============================================
 * Does a mysql_ping to all connection handles
 *---------------------------------------------*/
int inter_sql_ping(int tid, unsigned int tick, int id, int data) 
{
	ShowInfo("Pinging SQL server to keep connection alive...\n");
	mysql_ping(&mysql_handle);
	if(char_gm_read)
		mysql_ping(&lmysql_handle);
	return 0;
}


int sql_ping_init(void)
{
	int connection_timeout, connection_ping_interval;

	// set a default value first
	connection_timeout = 28800; // 8 hours

	// ask the mysql server for the timeout value
	if (!mysql_query(&mysql_handle, "SHOW VARIABLES LIKE 'wait_timeout'")
	&& (sql_res = mysql_store_result(&mysql_handle)) != NULL) {
		sql_row = mysql_fetch_row(sql_res);
		if (sql_row)
			connection_timeout = atoi(sql_row[1]);
		if (connection_timeout < 60)
			connection_timeout = 60;
		mysql_free_result(sql_res);
	}

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
	mysql_init(&mysql_handle);
	ShowInfo("Connect Character DB server.... (Character Server)\n");
	if(!mysql_real_connect(&mysql_handle, char_server_ip, char_server_id, char_server_pw,
		char_server_db ,char_server_port, (char *)NULL, 0)) {
			//pointer check
			ShowFatalError("%s\n",mysql_error(&mysql_handle));
			exit(1);
	}
#ifndef TXT_SQL_CONVERT
	else if (inter_sql_test()) {
		ShowStatus("Connect Success! (Character Server)\n");
	}

	if(char_gm_read) {
		mysql_init(&lmysql_handle);
		ShowInfo("Connect Character DB server.... (login server)\n");
		if(!mysql_real_connect(&lmysql_handle, login_server_ip, login_server_id, login_server_pw,
			login_server_db ,login_server_port, (char *)NULL, 0)) {
				//pointer check
				ShowFatalError("%s\n",mysql_error(&lmysql_handle));
				exit(1);
		}else {
			ShowStatus ("Connect Success! (Login Server)\n");
		}
	}
#endif //TXT_SQL_CONVERT
	if(strlen(default_codepage) > 0 ) {
		sprintf( tmp_sql, "SET NAMES %s", default_codepage );
		if (mysql_query(&mysql_handle, tmp_sql)) {
			ShowSQL("DB error - %s\n",mysql_error(&mysql_handle));
			ShowDebug("at %s:%d - %s\n", __FILE__,__LINE__,tmp_sql);
		}
#ifndef TXT_SQL_CONVERT
		if(char_gm_read)
			if (mysql_query(&lmysql_handle, tmp_sql)) {
				ShowSQL("DB error - %s\n",mysql_error(&lmysql_handle));
				ShowDebug("at %s:%d - %s\n", __FILE__,__LINE__,tmp_sql);
			}
#endif //TXT_SQL_CONVERT
	}

#ifndef TXT_SQL_CONVERT
	wis_db = db_alloc(__FILE__,__LINE__,DB_INT,DB_OPT_RELEASE_DATA,sizeof(int));
	inter_guild_sql_init();
	inter_storage_sql_init();
	inter_party_sql_init();
	inter_pet_sql_init();
	inter_homunculus_sql_init(); // albator
	inter_accreg_sql_init();

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
	int i;

	sprintf(tmp_sql, "EXPLAIN `%s`",char_db);
	if (mysql_query(&mysql_handle, tmp_sql)) {
		ShowSQL("DB error - %s\n",mysql_error(&mysql_handle));
		ShowDebug("at %s:%d - %s\n", __FILE__,__LINE__,tmp_sql);
	}
	sql_res = mysql_store_result(&mysql_handle);
	// store DB fields
	if (sql_res) {
		while((sql_row = mysql_fetch_row(sql_res))) {
			strcat (buf, sql_row[0]);
			strcat (buf, " ");
		}
	}

	// check DB strings
	for (i = 0; i < (int)(sizeof(fields) / sizeof(fields[0])); i++) {
		if(!strstr(buf, fields[i])) {
			ShowSQL ("Field `%s` not be found in `%s`. Consider updating your database!\n", fields[i], char_db);
			exit(1);
		}
	}

	mysql_free_result(sql_res);

	return 1;
}

// finalize
void inter_final(void) {
	wis_db->destroy(wis_db, NULL);

	inter_guild_sql_final();
	inter_storage_sql_final();
	inter_party_sql_final();
	inter_pet_sql_final();
	inter_homunculus_sql_final();	//[orn]
	
	if (accreg_pt) aFree(accreg_pt);
	return;
}

int inter_mapif_init(int fd) {
	inter_guild_mapif_init(fd);

	return 0;
}


//--------------------------------------------------------

// GM message sending
int mapif_GMmessage(unsigned char *mes, int len, unsigned long color, int sfd) {
	unsigned char buf[2048];

	if (len > 2048) len = 2047; //Make it fit to avoid crashes. [Skotlex]
	WBUFW(buf, 0) = 0x3800;
	WBUFW(buf, 2) = len;
	WBUFL(buf, 4) = color;
	memcpy(WBUFP(buf, 8), mes, len-8);
	mapif_sendallwos(sfd, buf, len);
	return 0;
}

// Wis sending
int mapif_wis_message(struct WisData *wd) {
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
int mapif_wis_end(struct WisData *wd,int flag)
{
	unsigned char buf[27];

	WBUFW(buf, 0)=0x3802;
	memcpy(WBUFP(buf, 2),wd->src,24);
	WBUFB(buf,26)=flag;
	mapif_send(wd->fd,buf,27);
//	printf("inter server wis_end %d\n",flag);
	return 0;
}

int mapif_account_reg(int fd,unsigned char *src)
{
//	unsigned char buf[WBUFW(src,2)]; <- Hey, can this really be done? [Skotlex]
	unsigned char *buf = aCalloc(1,WBUFW(src,2)); // [Lance] - Skot... Dynamic allocation is better :D
	memcpy(WBUFP(buf,0),src,WBUFW(src,2));
	WBUFW(buf, 0)=0x3804;
	mapif_sendallwos(fd, buf, WBUFW(buf,2));
	aFree(buf);
	return 0;
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
			p+= sprintf(WFIFOP(fd,p), "%s", reg->reg[i].str)+1; //We add 1 to consider the '\0' in place.
			p+= sprintf(WFIFOP(fd,p), "%s", reg->reg[i].value)+1;
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
		WBUFL(buf, len) = gm_account[i].account_id;
		WBUFB(buf, len+4) = (unsigned char)gm_account[i].level;
		len += 5;
	}
	WBUFW(buf, 2) = len;
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
			struct WisData *wd = idb_get(wis_db, wis_dellist[i]);
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
	RFIFOHEAD(fd);
	mapif_GMmessage(RFIFOP(fd, 8), RFIFOW(fd, 2), RFIFOL(fd, 4), fd);
	return 0;
}


// Wisp/page request to send
int mapif_parse_WisRequest(int fd) {
	struct WisData* wd;
	static int wisid = 0;
	char name[NAME_LENGTH], t_name[NAME_LENGTH*2]; //Needs space to allocate names with escaped chars [Skotlex]
	RFIFOHEAD(fd);
	if ( fd <= 0 ) {return 0;} // check if we have a valid fd

	if (RFIFOW(fd,2)-52 >= sizeof(wd->msg)) {
		ShowWarning("inter: Wis message size too long.\n");
		return 0;
	} else if (RFIFOW(fd,2)-52 <= 0) { // normaly, impossible, but who knows...
		ShowError("inter: Wis message doesn't exist.\n");
		return 0;
	}
	memcpy(name, RFIFOP(fd,28), NAME_LENGTH); //Received name may be too large and not contain \0! [Skotlex]
	name[NAME_LENGTH-1]= '\0';
	
	sprintf (tmp_sql, "SELECT `name` FROM `%s` WHERE `name`='%s'",
		char_db, jstrescapecpy(t_name, name));
	if(mysql_query(&mysql_handle, tmp_sql) ) {
		ShowSQL("DB error - %s\n",mysql_error(&mysql_handle));
		ShowDebug("at %s:%d - %s\n", __FILE__,__LINE__,tmp_sql);
	}
	sql_res = mysql_store_result(&mysql_handle);

	// search if character exists before to ask all map-servers
	if (!(sql_row = mysql_fetch_row(sql_res))) {
		unsigned char buf[27];
		WBUFW(buf, 0) = 0x3802;
		memcpy(WBUFP(buf, 2), RFIFOP(fd, 4), NAME_LENGTH);
		WBUFB(buf,26) = 1; // flag: 0: success to send wisper, 1: target character is not loged in?, 2: ignored by target
		mapif_send(fd, buf, 27);
	// Character exists. So, ask all map-servers
	} else {
		// to be sure of the correct name, rewrite it
		memset(name, 0, NAME_LENGTH);
		strncpy(name, sql_row[0], NAME_LENGTH);
		// if source is destination, don't ask other servers.
		if (strcmp((char*)RFIFOP(fd,4),name) == 0) {
			unsigned char buf[27];
			WBUFW(buf, 0) = 0x3802;
			memcpy(WBUFP(buf, 2), RFIFOP(fd, 4), NAME_LENGTH);
			WBUFB(buf,26) = 1; // flag: 0: success to send wisper, 1: target character is not loged in?, 2: ignored by target
			mapif_send(fd, buf, 27);
		} else {

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
	
	//Freeing ... O.o 
	if(sql_res){
		mysql_free_result(sql_res);
	}
	
	return 0;
}


// Wisp/page transmission result
int mapif_parse_WisReply(int fd) {
	int id, flag;
	struct WisData *wd;
	RFIFOHEAD(fd);
	id = RFIFOL(fd,2);
	flag = RFIFOB(fd,6);
	wd = idb_get(wis_db, id);
	if (wd == NULL)
		return 0;	// This wisp was probably suppress before, because it was timeout of because of target was found on another map-server

	if ((--wd->count) <= 0 || flag != 1) {
		mapif_wis_end(wd, flag); // flag: 0: success to send wisper, 1: target character is not loged in?, 2: ignored by target
		idb_remove(wis_db, id);
	}

	return 0;
}

// Received wisp message from map-server for ALL gm (just copy the message and resends it to ALL map-servers)
int mapif_parse_WisToGM(int fd) {
	unsigned char buf[2048]; // 0x3003/0x3803 <packet_len>.w <wispname>.24B <min_gm_level>.w <message>.?B
	RFIFOHEAD(fd);
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
	RFIFOHEAD(fd);	
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
		sscanf(RFIFOP(fd,p), "%31c%n",reg->reg[j].str,&len);
		reg->reg[j].str[len]='\0';
		p +=len+1; //+1 to skip the '\0' between strings.
		sscanf(RFIFOP(fd,p), "%255c%n",reg->reg[j].value,&len);
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
	RFIFOHEAD(fd);
	//Load Char Registry
	if (RFIFOB(fd,12))
		mapif_account_reg_reply(fd,RFIFOL(fd,2),RFIFOL(fd,6),3);
	//Load Account Registry
	if (RFIFOB(fd,11))
		mapif_account_reg_reply(fd,RFIFOL(fd,2),RFIFOL(fd,6),2);
	//Ask Login Server for Account2 values.
	if (RFIFOB(fd,10))
		request_accreg2(RFIFOL(fd,2),RFIFOL(fd,6)-2);
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
	name = RFIFOP(fd, 11);

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
	int len=0;
	RFIFOHEAD(fd);
	cmd=RFIFOW(fd,0);
	// inter鯖管轄かを調べる
	if(cmd < 0x3000 || cmd >= 0x3000 + (sizeof(inter_recv_packet_length)/
		sizeof(inter_recv_packet_length[0]) ) )
		return 0;

	if (inter_recv_packet_length[cmd-0x3000] == 0) //This is necessary, because otherwise we return 2 and the char server will just hang waiting for packets! [Skotlex]
		return 0;
	
	// パケット長を調べる
	if((len = inter_check_length(fd, inter_recv_packet_length[cmd - 0x3000])) == 0)
		return 2;

	switch(cmd){
	case 0x3000: mapif_parse_GMmessage(fd); break;
	case 0x3001: mapif_parse_WisRequest(fd); break;
	case 0x3002: mapif_parse_WisReply(fd); break;
	case 0x3003: mapif_parse_WisToGM(fd); break;
	case 0x3004: mapif_parse_Registry(fd); break;
	case 0x3005: mapif_parse_RegistryRequest(fd); break;
	case 0x3006: mapif_parse_NameChangeRequest(fd); break;
	default:
		if(inter_party_parse_frommap(fd))
			break;
		if(inter_guild_parse_frommap(fd))
			break;
		if(inter_storage_parse_frommap(fd))
			break;
		if(inter_pet_parse_frommap(fd))
			break;
		if(inter_homunculus_parse_frommap(fd)) //albator
			break;
		return 0;
	}

	RFIFOSKIP(fd, len);
	return 1;
}

// RFIFO check
int inter_check_length(int fd, int length)
{
	RFIFOHEAD(fd);
	if(length==-1){	// v-len packet
		if(RFIFOREST(fd)<4)	// packet not yet
			return 0;
		length = RFIFOW(fd, 2);
	}

	if((int)RFIFOREST(fd) < length)	// packet not yet
		return 0;

	return length;
}
#endif //TXT_SQL_CONVERT
