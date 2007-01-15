// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include <sys/types.h>

#ifdef LCCWIN32
#include <winsock2.h>
#else
#ifdef __WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#else
#include <sys/socket.h>
#include <netinet/in.h> 
#include <arpa/inet.h>
#include <netdb.h>
#endif
#endif

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h> // for stat/lstat/fstat
#include <signal.h>
#include <fcntl.h>
#include <string.h>

//add include for DBMS(mysql)
#include <mysql.h>

#include "../common/core.h"
#include "../common/socket.h"
#include "../common/malloc.h"
#include "../common/db.h"
#include "../common/timer.h"
#include "../common/strlib.h"
#include "../common/mmo.h"
#include "../common/showmsg.h"
#include "../common/version.h"
#include "../common/cbasetypes.h"
#include "login.h"

#ifdef PASSWORDENC
#include "../common/md5calc.h"
#endif

#define J_MAX_MALLOC_SIZE 65535

//-----------------------------------------------------
// global variable
//-----------------------------------------------------
int use_dnsbl=0; // [Zido]
char dnsbl_servs[1024];
int server_num;
int new_account_flag = 0; //Set from config too XD [Sirius]
in_addr_t bind_ip= 0;
char bind_ip_str[128];
int login_port = 6900;

// Advanced subnet check [LuzZza]
struct _subnet {
	long subnet;
	long mask;
	long char_ip;
	long map_ip;
} subnet[16];

int subnet_count = 0;

struct mmo_char_server server[MAX_SERVERS];
int server_fd[MAX_SERVERS];

int login_fd;

//Account flood protection [Kevin]
unsigned int new_reg_tick=0;
int allowed_regs=1;
int num_regs=0;
int time_allowed=10; //Init this to 10 secs, not 10K secs [Skotlex]

char date_format[32] = "%Y-%m-%d %H:%M:%S";
unsigned int auth_num = 0, auth_max = 0;

int min_level_to_connect = 0; // minimum level of player/GM (0: player, 1-99: gm) to connect on the server
int check_ip_flag = 1; // It's to check IP of a player between login-server and char-server (part of anti-hacking system)
int check_client_version = 0; //Client version check ON/OFF .. (sirius)
int client_version_to_connect = 20; //Client version needed to connect ..(sirius)
static int online_check=1; //When set to 1, login server rejects incoming players that are already registered as online. [Skotlex]
static int ip_sync_interval = 0;

MYSQL mysql_handle;

int ipban = 1;
int dynamic_account_ban = 1;
int dynamic_account_ban_class = 0;
int dynamic_pass_failure_ban = 1;
int dynamic_pass_failure_ban_time = 5;
int dynamic_pass_failure_ban_how_many = 3;
int dynamic_pass_failure_ban_how_long = 60;

int login_server_port = 3306;
char login_server_ip[32] = "127.0.0.1";
char login_server_id[32] = "ragnarok";
char login_server_pw[32] = "ragnarok";
char login_server_db[32] = "ragnarok";
char default_codepage[32] = ""; //Feature by irmin.
int use_md5_passwds = 0;
char login_db[256] = "login";
int log_login=1; //Whether to log the logins or not. [Skotlex]
char loginlog_db[256] = "loginlog";
bool login_gm_read = true;
int connection_ping_interval = 0;

// added to help out custom login tables, without having to recompile
// source so options are kept in the login_athena.conf or the inter_athena.conf
char login_db_account_id[256] = "account_id";
char login_db_userid[256] = "userid";
char login_db_user_pass[256] = "user_pass";
char login_db_level[256] = "level";

char gm_db[256] = "gm_accounts";

char reg_db[256] = "global_reg_value";

struct gm_account *gm_account_db;
int GM_num;
char tmpsql[65535], tmp_sql[65535];

int console = 0;

int case_sensitive = 1;

//-----------------------------------------------------

#define AUTH_FIFO_SIZE 256
struct {
	int account_id,login_id1,login_id2;
	int ip,sex,delflag;
} auth_fifo[AUTH_FIFO_SIZE];

int auth_fifo_pos = 0;

struct online_login_data {
	int account_id;
	short char_server;
	short waiting_disconnect;
};

//-----------------------------------------------------

static char md5key[20], md5keylen = 16;

struct dbt *online_db;

static void* create_online_user(DBKey key, va_list args) {
	struct online_login_data *p;
	p = aCalloc(1, sizeof(struct online_login_data));
	p->account_id = key.i;
	p->char_server = -1;
	return p;	
}

int charif_sendallwos(int sfd, unsigned char *buf, unsigned int len);

//-----------------------------------------------------
// Online User Database [Wizputer]
//-----------------------------------------------------

void add_online_user(int char_server, int account_id) {
	struct online_login_data *p;
	if (!online_check)
		return;
	p = idb_ensure(online_db, account_id, create_online_user);
	p->char_server = char_server;
	p->waiting_disconnect = 0;
}

int is_user_online(int account_id) {
	return (idb_get(online_db, account_id) != NULL);
}

void remove_online_user(int account_id) {
	if(!online_check)
		return;
	if (account_id == 99) {	// reset all to offline
		online_db->clear(online_db, NULL);
		return;
	}
	idb_remove(online_db,account_id);
}

int waiting_disconnect_timer(int tid, unsigned int tick, int id, int data)
{
	struct online_login_data *p;
	if ((p= idb_get(online_db, id)) != NULL && p->waiting_disconnect)
		remove_online_user(id);
	return 0;
}

static int sync_ip_addresses(int tid, unsigned int tick, int id, int data){
	unsigned char buf[2];
	ShowInfo("IP Sync in progress...\n");
	WBUFW(buf,0) = 0x2735;
	charif_sendallwos(-1, buf, 2);
	return 0;
}

//-----------------------------------------------------
// Read GM accounts
//-----------------------------------------------------
void read_gm_account(void) {
	MYSQL_RES* sql_res ;
	MYSQL_ROW sql_row;

	if(!login_gm_read)
		return;
	sprintf(tmp_sql, "SELECT `%s`,`%s` FROM `%s` WHERE `%s`> '0'",login_db_account_id,login_db_level,login_db,login_db_level);
	if (mysql_query(&mysql_handle, tmp_sql)) {
		ShowSQL("DB error - %s\n",mysql_error(&mysql_handle));
		ShowDebug("at %s:%d - %s\n", __FILE__,__LINE__,tmp_sql);
		return; //Failed to read GM list!
	}

	if (gm_account_db != NULL)
	{
		aFree(gm_account_db);
		gm_account_db = NULL;
	}
	GM_num = 0;

	sql_res = mysql_store_result(&mysql_handle);
	if (sql_res) {
		gm_account_db = (struct gm_account*)aCalloc((size_t)mysql_num_rows(sql_res), sizeof(struct gm_account));
		while ((sql_row = mysql_fetch_row(sql_res))) {
			gm_account_db[GM_num].account_id = atoi(sql_row[0]);
			gm_account_db[GM_num].level = atoi(sql_row[1]);
			GM_num++;
		}
		mysql_free_result(sql_res);
	}
}

//-----------------------------------------------------
// Send GM accounts to all char-server
//-----------------------------------------------------
void send_GM_accounts(int fd) {
	int i;
	unsigned char buf[32767];
	int len;

	if(!login_gm_read)
		return;
	len = 4;
	WBUFW(buf,0) = 0x2732;
	for(i = 0; i < GM_num; i++)
		// send only existing accounts. We can not create a GM account when server is online.
		if (gm_account_db[i].level > 0) {
			WBUFL(buf,len) = gm_account_db[i].account_id;
			WBUFB(buf,len+4) = (unsigned char)gm_account_db[i].level;
			len += 5;
			if (len >= 32000) {
				ShowWarning("send_GM_accounts: Too many accounts! Only %d out of %d were sent.\n", i, GM_num);
				break;
			}
		}
		WBUFW(buf,2) = len;
		if (fd == -1)
			charif_sendallwos(-1, buf, len);
		else
		{
			WFIFOHEAD(fd, len);
			memcpy(WFIFOP(fd,0), buf, len);
			WFIFOSET(fd, len);
		}
		return;
}

//-----------------------------------------------------
// check user level
//-----------------------------------------------------
/*
int isGM(int account_id) {
	int level;

	MYSQL_RES* 	sql_res;
	MYSQL_ROW	sql_row;
	level = 0;
	sprintf(tmpsql,"SELECT `%s` FROM `%s` WHERE `%s`='%d'", login_db_level, login_db, login_db_account_id, account_id);
	if (mysql_query(&mysql_handle, tmpsql)) {
		ShowSQL("DB error - %s\n",mysql_error(&mysql_handle));
		ShowDebug("at %s:%d - %s\n", __FILE__,__LINE__,tmpsql);
	}
	sql_res = mysql_store_result(&mysql_handle);
	if (sql_res) {
		sql_row = mysql_fetch_row(sql_res);
		level = atoi(sql_row[0]);
		if (level > 99)
			level = 99;
	}

	if (level == 0) {
		return 0;
		//not GM
	}

	mysql_free_result(sql_res);

	return level;
}
*/

//---------------------------------------------------
// E-mail check: return 0 (not correct) or 1 (valid).
//---------------------------------------------------
int e_mail_check(char *email) {
	char ch;
	char* last_arobas;

	// athena limits
	if (strlen(email) < 3 || strlen(email) > 39)
		return 0;

	// part of RFC limits (official reference of e-mail description)
	if (strchr(email, '@') == NULL || email[strlen(email)-1] == '@')
		return 0;

	if (email[strlen(email)-1] == '.')
		return 0;

	last_arobas = strrchr(email, '@');

	if (strstr(last_arobas, "@.") != NULL ||
		strstr(last_arobas, "..") != NULL)
		return 0;

	for(ch = 1; ch < 32; ch++) {
		if (strchr(last_arobas, ch) != NULL) {
			return 0;
			break;
		}
	}

	if (strchr(last_arobas, ' ') != NULL ||
		strchr(last_arobas, ';') != NULL)
		return 0;

	// all correct
	return 1;
}

/*======================================================
 * Does a mysql_ping to all connection handles. [Skotlex]
 *------------------------------------------------------
 */
int login_sql_ping(int tid, unsigned int tick, int id, int data) 
{
	ShowInfo("Pinging SQL server to keep connection alive...\n");
	mysql_ping(&mysql_handle);
	return 0;
}

//-----------------------------------------------------
// Read Account database - mysql db
//-----------------------------------------------------
int mmo_auth_sqldb_init(void) {

	ShowStatus("Login server init....\n");

	// memory initialize
	ShowStatus("memory initialize....\n");

	mysql_init(&mysql_handle);

	// DB connection start
	ShowStatus("Connect Login Database Server....\n");
	if (!mysql_real_connect(&mysql_handle, login_server_ip, login_server_id, login_server_pw,
	    login_server_db, login_server_port, (char *)NULL, 0)) {
		// pointer check
		ShowFatalError("%s\n", mysql_error(&mysql_handle));
		exit(1);
	} else {
		ShowStatus("Connect success!\n");
	}
	if( strlen(default_codepage) > 0 ) {
		sprintf( tmpsql, "SET NAMES %s", default_codepage );
		if (mysql_query(&mysql_handle, tmpsql)) {
			ShowSQL("DB error - %s\n",mysql_error(&mysql_handle));
			ShowDebug("at %s:%d - %s\n", __FILE__,__LINE__,tmpsql);
		}
	}

	if (log_login)
	{
		sprintf(tmpsql, "INSERT DELAYED INTO `%s`(`time`,`ip`,`user`,`rcode`,`log`) VALUES (NOW(), '0', 'lserver','100','login server started')", loginlog_db);

		//query
		if (mysql_query(&mysql_handle, tmpsql)) {
			ShowSQL("DB error - %s\n",mysql_error(&mysql_handle));
			ShowDebug("at %s:%d - %s\n", __FILE__,__LINE__,tmpsql);
		}
	}

	if (connection_ping_interval) {
		add_timer_func_list(login_sql_ping, "login_sql_ping");
		add_timer_interval(gettick()+connection_ping_interval*60*60*1000,
				login_sql_ping, 0, 0, connection_ping_interval*60*60*1000);
	}
	return 0;
}

//-----------------------------------------------------
// DB server connect check
//-----------------------------------------------------
void mmo_auth_sqldb_sync(void) {
	// db connect check? or close?
	// ping pong DB server -if losted? then connect try. else crash.
}

//-----------------------------------------------------
// close DB
//-----------------------------------------------------
void mmo_db_close(void) {
	int i, fd;

	//set log.
	if (log_login)
	{
		sprintf(tmpsql,"INSERT DELAYED INTO `%s`(`time`,`ip`,`user`,`rcode`,`log`) VALUES (NOW(), '0', 'lserver','100', 'login server shutdown')", loginlog_db);

		//query
		if (mysql_query(&mysql_handle, tmpsql)) {
			ShowSQL("DB error - %s\n",mysql_error(&mysql_handle));
			ShowDebug("at %s:%d - %s\n", __FILE__,__LINE__,tmpsql);
		}
	}
/*
	//delete all server status
	sprintf(tmpsql,"DELETE FROM `sstatus`");
	//query
	if (mysql_query(&mysql_handle, tmpsql)) {
		ShowSQL("DB error - %s\n",mysql_error(&mysql_handle));
		ShowDebug("at %s:%d - %s\n", __FILE__,__LINE__,tmpsql);
	}
	mysql_close(&mysql_handle);
	ShowStatus("close DB connect....\n");
*/

	for (i = 0; i < MAX_SERVERS; i++) {
		if ((fd = server_fd[i]) >= 0)
		{	//Clean only data related to servers we are connected to. [Skotlex]
			sprintf(tmpsql,"DELETE FROM `sstatus` WHERE `index` = '%d'", i);
			if (mysql_query(&mysql_handle, tmpsql))
			{
				ShowSQL("DB error - %s\n",mysql_error(&mysql_handle));
				ShowDebug("at %s:%d - %s\n", __FILE__,__LINE__,tmpsql);
			}
			delete_session(fd);
		}
	}
	mysql_close(&mysql_handle);
	ShowStatus("close DB connect....\n");
	delete_session(login_fd);
}

//-----------------------------------------------------
// Make new account
//-----------------------------------------------------
int mmo_auth_new(struct mmo_account* account, char sex)
{
	MYSQL_RES* 	sql_res;
	unsigned int tick = gettick();
	char user_password[256];
	//Account Registration Flood Protection by [Kevin]
	if(DIFF_TICK(tick, new_reg_tick) < 0 && num_regs >= allowed_regs) {
		ShowNotice("Account registration denied (registration limit exceeded)\n");
		return 3;
	}

	//Check for preexisting account
	sprintf(tmp_sql, "SELECT `%s` FROM `%s` WHERE `userid` = '%s'", login_db_userid, login_db, account->userid);
	if(mysql_query(&mysql_handle, tmp_sql)){
		ShowSQL("DB error - %s\n",mysql_error(&mysql_handle));
		ShowDebug("at %s:%d - %s\n", __FILE__,__LINE__,tmp_sql);
		return 1; //Return Incorrect user/pass?
	}

	sql_res = mysql_store_result(&mysql_handle);
	if(mysql_num_rows(sql_res) > 0){
		mysql_free_result(sql_res);
		return 1; //Already exists, return incorrect user/pass.
	}
	mysql_free_result(sql_res); //Only needed for the already-exists check...

	mysql_real_escape_string(&mysql_handle, account->userid, account->userid, strlen(account->userid));
	mysql_real_escape_string(&mysql_handle, account->passwd, account->passwd, strlen(account->passwd));

	if (sex == 'f') sex = 'F';
	else if (sex == 'm') sex = 'M';
	if (use_md5_passwds)
		MD5_String(account->passwd,user_password);
	else
		jstrescapecpy(user_password, account->passwd);

	ShowInfo("New account: user: %s with passwd: %s sex: %c\n", account->userid, user_password, sex);

	sprintf(tmp_sql, "INSERT INTO `%s` (`%s`, `%s`, `sex`, `email`) VALUES ('%s', '%s', '%c', '%s')", login_db, login_db_userid, login_db_user_pass, account->userid, user_password, sex, "a@a.com");

	if(mysql_query(&mysql_handle, tmp_sql)){
		//Failed to insert new acc :/
		ShowSQL("DB error - %s\n",mysql_error(&mysql_handle));
		ShowDebug("at %s:%d - %s\n", __FILE__,__LINE__,tmp_sql);
		return 1;
	}

	if(mysql_field_count(&mysql_handle) == 0 &&
		mysql_insert_id(&mysql_handle) < START_ACCOUNT_NUM) {
		//Invalid Account ID! Must update it.
		int id = (int)mysql_insert_id(&mysql_handle);
		sprintf(tmp_sql, "UPDATE `%s` SET `%s`='%d' WHERE `%s`='%d'", login_db, login_db_account_id, START_ACCOUNT_NUM, login_db_account_id, id);
		if(mysql_query(&mysql_handle, tmp_sql)){
			ShowError("New account %s has an invalid account ID [%d] which could not be updated (account_id must be %d or higher).", account->userid, id, START_ACCOUNT_NUM);
			ShowSQL("DB error - %s\n",mysql_error(&mysql_handle));
			ShowDebug("at %s:%d - %s\n", __FILE__,__LINE__,tmp_sql);
			//Just delete it and fail.
			sprintf(tmp_sql, "DELETE FROM `%s` WHERE `%s`='%d'", login_db, login_db_account_id, id);
			if(mysql_query(&mysql_handle, tmp_sql)){
				ShowSQL("DB error - %s\n",mysql_error(&mysql_handle));
				ShowDebug("at %s:%d - %s\n", __FILE__,__LINE__,tmp_sql);
			}
			return 1;
		}
		ShowNotice("Updated New account %s's ID %d->%d (account_id must be %d or higher).", account->userid, id, START_ACCOUNT_NUM, START_ACCOUNT_NUM);
	}
	if(DIFF_TICK(tick, new_reg_tick) > 0)
	{	//Update the registration check.
		num_regs=0;
		new_reg_tick=tick+time_allowed*1000;
	}
	num_regs++;

	return 0;
}

// Send to char
int charif_sendallwos(int sfd, unsigned char *buf, unsigned int len) {
	int i, c;
	int fd;

	c = 0;
	for(i = 0; i < MAX_SERVERS; i++) {
		if ((fd = server_fd[i]) > 0 && fd != sfd) {
			WFIFOHEAD(fd,len);
			if (WFIFOSPACE(fd) < len) //Increase buffer size.
				realloc_writefifo(fd, len);
			memcpy(WFIFOP(fd,0), buf, len);
			WFIFOSET(fd,len);
			c++;
		}
	}

	return c;
}

//-----------------------------------------------------
// Auth
//-----------------------------------------------------
int mmo_auth( struct mmo_account* account , int fd){
	time_t ban_until_time, raw_time;
	char tmpstr[256];
	char t_uid[256], t_pass[256];
	char user_password[256];
	char *dnsbl_serv;

	//added for account creation _M _F
	int len;

	MYSQL_RES* 	sql_res;
	MYSQL_ROW	sql_row;
	//int sql_fields, sql_cnt;
	char md5str[64], md5bin[32];

	char ip[16];

	unsigned char * sin_addr = (unsigned char *)&session[fd]->client_addr.sin_addr.s_addr;

	char r_ip[16]; // [Zido]
	char ip_dnsbl[256]; // [Zido]

	// Start DNS Blacklist check [Zido]
	if(use_dnsbl) {
		sprintf(r_ip, "%d.%d.%d.%d", sin_addr[3], sin_addr[2], sin_addr[1], sin_addr[0]);

		dnsbl_serv=strtok(dnsbl_servs,",");
		sprintf(ip_dnsbl,"%s.%s",r_ip,dnsbl_serv);
// Using directly gethostbyname should be quicker. [Skotlex]
//		if(resolve_hostbyname(ip_dnsbl, NULL, NULL)) {
		if(gethostbyname(ip_dnsbl)) {
			ShowInfo("DNSBL: (%s) Blacklisted. User Kicked.\n",ip);
			return 3;
		}

		while((dnsbl_serv=strtok(dnsbl_servs,","))!=NULL) {
			sprintf(ip_dnsbl,"%s.%s",r_ip,dnsbl_serv);
// Using directly gethostbyname should be quicker. [Skotlex]
//			if(resolve_hostbyname(ip_dnsbl, NULL, NULL)) {
			if(gethostbyname(ip_dnsbl)) {
				ShowInfo("DNSBL: (%s) Blacklisted. User Kicked.\n",ip);
				return 3;
			}
		}

	}
	// End DNS Blacklist check [Zido]

	sprintf(ip, "%d.%d.%d.%d", sin_addr[0], sin_addr[1], sin_addr[2], sin_addr[3]);
	//ShowInfo("auth start for %s...\n", ip);

	//accountreg with _M/_F .. [Sirius]
	len = strlen(account->userid) -2;

	if (account->passwdenc == 0 && account->userid[len] == '_' &&
		(account->userid[len+1] == 'F' || account->userid[len+1] == 'M' ||
		account->userid[len+1] == 'f' || account->userid[len+1] == 'm') &&
		new_account_flag == 1 && 
		len >= 4 && strlen(account->passwd) >= 4)
	{
		int result;
		account->userid[len] = '\0'; //Terminating the name.
		if ((result = mmo_auth_new(account, account->userid[len+1])))
			return result; //Failed to make account. [Skotlex].
	}

	// auth start : time seed
	// Platform/Compiler dependant clock() for time check is removed. [Lance]
	// clock() is originally used to track processing ticks on program execution.
	time(&raw_time);
	strftime(tmpstr, 24, "%Y-%m-%d %H:%M:%S",localtime(&raw_time));

	jstrescapecpy(t_uid,account->userid);

	if (account->passwdenc==PASSWORDENC) {
		memcpy(t_pass, account->passwd, NAME_LENGTH);
		t_pass[NAME_LENGTH] = '\0';
	} else
		jstrescapecpy(t_pass, account->passwd);


	// make query
	sprintf(tmpsql, "SELECT `%s`,`%s`,`%s`,`lastlogin`,`logincount`,`sex`,`connect_until`,`last_ip`,`ban_until`,`state`,`%s`"
		" FROM `%s` WHERE `%s`= %s '%s'", login_db_account_id, login_db_userid, login_db_user_pass, login_db_level, login_db, login_db_userid, case_sensitive ? "BINARY" : "", t_uid);
	//login {0-account_id/1-userid/2-user_pass/3-lastlogin/4-logincount/5-sex/6-connect_untl/7-last_ip/8-ban_until/9-state/10-level}

	// query
	if (mysql_query(&mysql_handle, tmpsql)) {
		ShowSQL("DB error - %s\n",mysql_error(&mysql_handle));
		ShowDebug("at %s:%d - %s\n", __FILE__,__LINE__,tmpsql);
	}
	sql_res = mysql_store_result(&mysql_handle) ;
	if (sql_res) {
		sql_row = mysql_fetch_row(sql_res);	//row fetching
		if (!sql_row) {
			//there's no id.
			ShowNotice("auth failed: no such account %s %s %s\n", tmpstr, account->userid, account->passwd);
			mysql_free_result(sql_res);
			return 0;
		}
	} else {
		ShowError("mmo_auth DB result error ! \n");
		return 0;
	}

	//Client Version check[Sirius]
	if(check_client_version == 1 && account->version != 0){
		if(account->version != client_version_to_connect){
			mysql_free_result(sql_res);
			return 5;
		}
	}           

	// Documented by CLOWNISIUS || LLRO || Gunstar lead this one with me
	// IF changed to diferent returns~ you get diferent responses from your msgstringtable.txt
	//Ireturn 2  == line 9
	//Ireturn 5  == line 311
	//Ireturn 6  == line 450
	//Ireturn 7  == line 440
	//Ireturn 8  == line 682
	//Ireturn 9  == line 704
	//Ireturn 10 == line 705
	//Ireturn 11 == line 706
	//Ireturn 12 == line 707
	//Ireturn 13 == line 708
	//Ireturn 14 == line 709
	//Ireturn 15 == line 710
	//Ireturn -1 == line 010
	// Check status
	{
		int encpasswdok = 0;

		if (atoi(sql_row[9]) == -3) {
			//id is banned
			mysql_free_result(sql_res);
			return -3;
		} else if (atoi(sql_row[9]) == -2) { //dynamic ban
			//id is banned
			mysql_free_result(sql_res);
			//add IP list.
			return -2;
		}

		if (use_md5_passwds) {
			MD5_String(account->passwd,user_password);
		} else {
			jstrescapecpy(user_password, account->passwd);
		}
		//ShowInfo("account id ok encval:%d\n",account->passwdenc);
#ifdef PASSWORDENC
		if (account->passwdenc > 0) {
			int j = account->passwdenc;
			//ShowInfo("start md5calc..\n");
			if (j > 2)
				j = 1;
			do {
				if (j == 1) {
					sprintf(md5str, "%s%s", md5key,sql_row[2]);
				} else if (j == 2) {
					sprintf(md5str, "%s%s", sql_row[2], md5key);
				} else
					md5str[0] = 0;
				//ShowDebug("j:%d mdstr:%s\n", j, md5str);
				MD5_String2binary(md5str, md5bin);
				encpasswdok = (memcmp(user_password, md5bin, 16) == 0);
			} while (j < 2 && !encpasswdok && (j++) != account->passwdenc);
			//printf("key[%s] md5 [%s] ", md5key, md5);
			//ShowInfo("client [%s] accountpass [%s]\n", user_password, sql_row[2]);
			//ShowInfo("end md5calc..\n");
		}
#endif
		if ((strcmp(user_password, sql_row[2]) && !encpasswdok)) {
			if (account->passwdenc == 0) {
				ShowNotice("auth failed pass error %s %s %s" RETCODE, tmpstr, account->userid, user_password);
#ifdef PASSWORDENC
			} else {
				char logbuf[1024], *p = logbuf;
				int j;
				p += sprintf(p, "auth failed pass error %s %s recv-md5[", tmpstr, account->userid);
				for(j = 0; j < 16; j++)
					p += sprintf(p, "%02x", ((unsigned char *)user_password)[j]);
				p += sprintf(p, "] calc-md5[");
				for(j = 0; j < 16; j++)
					p += sprintf(p, "%02x", ((unsigned char *)md5bin)[j]);
				p += sprintf(p, "] md5key[");
				for(j = 0; j < md5keylen; j++)
					p += sprintf(p, "%02x", ((unsigned char *)md5key)[j]);
				p += sprintf(p, "]" RETCODE);
				ShowNotice("%s\n", p);
#endif
			}
			return 1;
		}
		//ShowInfo("auth ok %s %s" RETCODE, tmpstr, account->userid);
	}

/*
// do not remove this section. this is meant for future, and current forums usage
// as a login manager and CP for login server. [CLOWNISIUS]
	if (atoi(sql_row[10]) == 1) {
		return 4;
	}

	if (atoi(sql_row[10]) >= 5) {
		switch(atoi(sql_row[10])) {
		case 5:
			return 5;
			break;
		case 6:
			return 7;
			break;
		case 7:
			return 9;
			break;
		case 8:
			return 10;
			break;
		case 9:
			return 11;
			break;
		default:
			return 10;
			break;
		}
	}
*/
	ban_until_time = atol(sql_row[8]);

	//login {0-account_id/1-userid/2-user_pass/3-lastlogin/4-logincount/5-sex/6-connect_untl/7-last_ip/8-ban_until/9-state}
	if (ban_until_time != 0) { // if account is banned
		if (ban_until_time > time(NULL)) // always banned
			return 6; // 6 = Your are Prohibited to log in until %s

		sprintf(tmpsql, "UPDATE `%s` SET `ban_until`='0' WHERE `%s`= %s '%s'", login_db, login_db_userid, case_sensitive ? "BINARY" : "", t_uid);
		if (mysql_query(&mysql_handle, tmpsql)) {
			ShowSQL("DB error - %s\n",mysql_error(&mysql_handle));
			ShowDebug("at %s:%d - %s\n", __FILE__,__LINE__,tmpsql);
		}
	}

	if (atoi(sql_row[9])) {
		switch(atoi(sql_row[9])) { // packet 0x006a value + 1
		case 1: // 0 = Unregistered ID
		case 2: // 1 = Incorrect Password
		case 3: // 2 = This ID is expired
		case 4: // 3 = Rejected from Server
		case 5: // 4 = You have been blocked by the GM Team
		case 6: // 5 = Your Game's EXE file is not the latest version
		case 7: // 6 = Your are Prohibited to log in until %s
		case 8: // 7 = Server is jammed due to over populated
		case 9: // 8 = No more accounts may be connected from this company
		case 10: // 9 = MSI_REFUSE_BAN_BY_DBA
		case 11: // 10 = MSI_REFUSE_EMAIL_NOT_CONFIRMED
		case 12: // 11 = MSI_REFUSE_BAN_BY_GM
		case 13: // 12 = MSI_REFUSE_TEMP_BAN_FOR_DBWORK
		case 14: // 13 = MSI_REFUSE_SELF_LOCK
		case 15: // 14 = MSI_REFUSE_NOT_PERMITTED_GROUP
		case 16: // 15 = MSI_REFUSE_NOT_PERMITTED_GROUP
		case 100: // 99 = This ID has been totally erased
		case 101: // 100 = Login information remains at %s.
		case 102: // 101 = Account has been locked for a hacking investigation. Please contact the GM Team for more information
		case 103: // 102 = This account has been temporarily prohibited from login due to a bug-related investigation
		case 104: // 103 = This character is being deleted. Login is temporarily unavailable for the time being
		case 105: // 104 = Your spouse character is being deleted. Login is temporarily unavailable for the time being
			ShowNotice("Auth Error #%d\n", atoi(sql_row[9]));
			return atoi(sql_row[9]) - 1;
			break;
		default:
			return 99; // 99 = ID has been totally erased
			break;
		}
	}

	if (atol(sql_row[6]) != 0 && atol(sql_row[6]) < time(NULL)) {
		return 2; // 2 = This ID is expired
	}

	if (online_check) {
		struct online_login_data* data = idb_get(online_db,atoi(sql_row[0]));
		unsigned char buf[8];
		if (data && data->char_server > -1) {
			//Request char servers to kick this account out. [Skotlex]
			ShowWarning("User [%s] is already online - Rejected.\n",sql_row[1]);
			WBUFW(buf,0) = 0x2734;
			WBUFL(buf,2) = atol(sql_row[0]);
			charif_sendallwos(-1, buf, 6);
			if (!data->waiting_disconnect)
				add_timer(gettick()+30000, waiting_disconnect_timer, atol(sql_row[0]), 0);
			data->waiting_disconnect = 1;
			return 3; // Rejected
		}
	}

	account->account_id = atoi(sql_row[0]);
	account->login_id1 = rand();
	account->login_id2 = rand();
	memcpy(tmpstr, sql_row[3], 19);
	memcpy(account->lastlogin, tmpstr, 24);
	account->sex = sql_row[5][0] == 'S' ? 2 : sql_row[5][0]=='M';
	account->level = atoi(sql_row[10]) > 99 ? 99 : atoi(sql_row[10]); // as was in isGM() [zzo]

	if (account->sex != 2 && account->account_id < START_ACCOUNT_NUM)
		ShowWarning("Account %s has account id %d! Account IDs must be over %d to work properly!\n", account->userid, account->account_id, START_ACCOUNT_NUM);
	sprintf(tmpsql, "UPDATE `%s` SET `lastlogin` = NOW(), `logincount`=`logincount` +1, `last_ip`='%s'  WHERE `%s` = %s '%s'",
		login_db, ip, login_db_userid, case_sensitive ? "BINARY" : "", sql_row[1]);
	mysql_free_result(sql_res) ; //resource free
	if (mysql_query(&mysql_handle, tmpsql)) {
		ShowSQL("DB error - %s\n",mysql_error(&mysql_handle));
		ShowDebug("at %s:%d - %s\n", __FILE__,__LINE__,tmpsql);
	}

	return -1;
}

static int online_db_setoffline(DBKey key, void* data, va_list ap) {
	struct online_login_data *p = (struct online_login_data *)data;
	int server = va_arg(ap, int);
	if (server == -1) {
		p->char_server = -1;
		p->waiting_disconnect = 0;
	} else if (p->char_server == server)
		p->char_server = -2; //Char server disconnected.
	return 0;
}

//-----------------------------------------------------
// char-server packet parse
//-----------------------------------------------------
int parse_fromchar(int fd){
	int i, id;
	MYSQL_RES* sql_res;
	MYSQL_ROW  sql_row = NULL;

	unsigned char *p = (unsigned char *) &session[fd]->client_addr.sin_addr.s_addr;
	unsigned long ipl = session[fd]->client_addr.sin_addr.s_addr;
	char ip[16];
	RFIFOHEAD(fd);

	sprintf(ip, "%d.%d.%d.%d", p[0], p[1], p[2], p[3]);

	for(id = 0; id < MAX_SERVERS; id++)
		if (server_fd[id] == fd)
			break;

	if (id == MAX_SERVERS)
		session[fd]->eof = 1;
	if(session[fd]->eof) {
		if (id < MAX_SERVERS) {
			ShowStatus("Char-server '%s' has disconnected.\n", server[id].name);
			server_fd[id] = -1;
			memset(&server[id], 0, sizeof(struct mmo_char_server));
			online_db->foreach(online_db,online_db_setoffline,id); //Set all chars from this char server to offline.
			// server delete
			sprintf(tmpsql, "DELETE FROM `sstatus` WHERE `index`='%d'", id);
			// query
			if (mysql_query(&mysql_handle, tmpsql)) {
				ShowSQL("DB error - %s\n",mysql_error(&mysql_handle));
				ShowDebug("at %s:%d - %s\n", __FILE__,__LINE__,tmpsql);
			}
		}
		do_close(fd);
		return 0;
	}

	while(RFIFOREST(fd) >= 2) {
//		printf("char_parse: %d %d packet case=%x\n", fd, RFIFOREST(fd), RFIFOW(fd, 0));

		switch (RFIFOW(fd,0)) {
		case 0x2709:
			if (log_login)
			{
				sprintf(tmpsql,"INSERT DELAYED INTO `%s`(`time`,`ip`,`user`,`log`) VALUES (NOW(), '%u', '%s', 'GM reload request')", loginlog_db, (unsigned int)ntohl(ipl),server[id].name);
				if (mysql_query(&mysql_handle, tmpsql)) {
					ShowSQL("DB error - %s\n",mysql_error(&mysql_handle));
					ShowDebug("at %s:%d - %s\n", __FILE__,__LINE__,tmpsql);
				}
			}
			read_gm_account();
			// send GM accounts to all char-servers
			send_GM_accounts(-1);
			RFIFOSKIP(fd,2);
			break;

		case 0x2712:
			if (RFIFOREST(fd) < 19)
				return 0;
		{
			int account_id;
			WFIFOHEAD(fd,51);
			account_id = RFIFOL(fd,2); // speed up
			for(i=0;i<AUTH_FIFO_SIZE;i++){
				if (auth_fifo[i].account_id == account_id &&
					auth_fifo[i].login_id1 == RFIFOL(fd,6) &&
#if CMP_AUTHFIFO_LOGIN2 != 0
					auth_fifo[i].login_id2 == RFIFOL(fd,10) && // relate to the versions higher than 18
#endif
					auth_fifo[i].sex == RFIFOB(fd,14) &&
#if CMP_AUTHFIFO_IP != 0
					auth_fifo[i].ip == RFIFOL(fd,15) &&
#endif
					!auth_fifo[i].delflag)
				{
					auth_fifo[i].delflag = 1;
					ShowDebug("auth -> %d\n", i);
					break;
				}
			}

			if (i != AUTH_FIFO_SIZE && account_id > 0) { // send ack 
				time_t connect_until_time = 0;
				char email[40] = "";
				account_id=RFIFOL(fd,2);
				sprintf(tmpsql, "SELECT `email`,`connect_until` FROM `%s` WHERE `%s`='%d'", login_db, login_db_account_id, account_id);
				if (mysql_query(&mysql_handle, tmpsql)) {
					ShowSQL("DB error - %s\n",mysql_error(&mysql_handle));
					ShowDebug("at %s:%d - %s\n", __FILE__,__LINE__,tmpsql);
				}
				sql_res = mysql_store_result(&mysql_handle) ;
				if (sql_res) {
					sql_row = mysql_fetch_row(sql_res);
					connect_until_time = atol(sql_row[1]);
					strcpy(email, sql_row[0]);
					mysql_free_result(sql_res);
				}
				WFIFOW(fd,0) = 0x2713;
				WFIFOL(fd,2) = account_id;
				WFIFOB(fd,6) = 0;
				memcpy(WFIFOP(fd, 7), email, 40);
				WFIFOL(fd,47) = (unsigned long) connect_until_time;
				WFIFOSET(fd,51);
			} else {
				WFIFOW(fd,0) = 0x2713;
				WFIFOL(fd,2) = account_id;
				WFIFOB(fd,6) = 1;
				WFIFOSET(fd,51);
			}
			RFIFOSKIP(fd,19);
			break;
		}

		case 0x2714:
			if (RFIFOREST(fd) < 6)
				return 0;
			// how many users on world? (update)
			if (server[id].users != RFIFOL(fd,2))
			{
				ShowStatus("set users %s : %d\n", server[id].name, RFIFOL(fd,2));

				server[id].users = RFIFOL(fd,2);
				sprintf(tmpsql,"UPDATE `sstatus` SET `user` = '%d' WHERE `index` = '%d'", server[id].users, id);
				// query
				if (mysql_query(&mysql_handle, tmpsql)) {
					ShowSQL("DB error - %s\n",mysql_error(&mysql_handle));
					ShowDebug("at %s:%d - %s\n", __FILE__,__LINE__,tmpsql);
				}
			}
		{	// send some answer
			WFIFOHEAD(fd,6);
			WFIFOW(fd,0) = 0x2718;
			WFIFOSET(fd,2);
		}
			RFIFOSKIP(fd,6);
			break;

		// We receive an e-mail/limited time request, because a player comes back from a map-server to the char-server
		case 0x2716:
			if (RFIFOREST(fd) < 6)
				return 0;
		{
			int account_id;
			time_t connect_until_time = 0;
			char email[40] = "";
			WFIFOHEAD(fd,50);
			account_id=RFIFOL(fd,2);
			sprintf(tmpsql,"SELECT `email`,`connect_until` FROM `%s` WHERE `%s`='%d'",login_db, login_db_account_id, account_id);
			if(mysql_query(&mysql_handle, tmpsql)) {
				ShowSQL("DB error - %s\n",mysql_error(&mysql_handle));
				ShowDebug("at %s:%d - %s\n", __FILE__,__LINE__,tmpsql);
			}
			sql_res = mysql_store_result(&mysql_handle) ;
			if (sql_res) {
				sql_row = mysql_fetch_row(sql_res);
				connect_until_time = atol(sql_row[1]);
				strcpy(email, sql_row[0]);
			}
			mysql_free_result(sql_res);
			//printf("parse_fromchar: E-mail/limited time request from '%s' server (concerned account: %d)\n", server[id].name, RFIFOL(fd,2));
			WFIFOW(fd,0) = 0x2717;
			WFIFOL(fd,2) = RFIFOL(fd,2);
			memcpy(WFIFOP(fd, 6), email, 40);
			WFIFOL(fd,46) = (unsigned long) connect_until_time;
			WFIFOSET(fd,50);
		}
			RFIFOSKIP(fd,6);
			break;

		case 0x2720:	// GM
			if (RFIFOREST(fd) < 4)
				return 0;
			if (RFIFOREST(fd) < RFIFOW(fd,2))
				return 0;
			//oldacc = RFIFOL(fd,4);
			ShowWarning("change GM isn't supported in this login server version.\n");
			ShowError("change GM error 0 %s\n", RFIFOP(fd, 8));

			RFIFOSKIP(fd, RFIFOW(fd, 2));
		{
			WFIFOHEAD(fd, 10);
			WFIFOW(fd, 0) = 0x2721;
			WFIFOL(fd, 2) = RFIFOL(fd,4); // oldacc;
			WFIFOL(fd, 6) = 0; // newacc;
			WFIFOSET(fd, 10);
		}
			return 0;

		// Map server send information to change an email of an account via char-server
		case 0x2722:	// 0x2722 <account_id>.L <actual_e-mail>.40B <new_e-mail>.40B
			if (RFIFOREST(fd) < 86)
				return 0;
		{
			int acc;
			char actual_email[40], new_email[40];
			acc = RFIFOL(fd,2);
			memcpy(actual_email, RFIFOP(fd,6), 40);
			memcpy(new_email, RFIFOP(fd,46), 40);
			if (e_mail_check(actual_email) == 0)
				ShowWarning("Char-server '%s': Attempt to modify an e-mail on an account (@email GM command), but actual email is invalid (account: %d, ip: %s)" RETCODE,
				server[id].name, acc, ip);
			else if (e_mail_check(new_email) == 0)
				ShowWarning("Char-server '%s': Attempt to modify an e-mail on an account (@email GM command) with a invalid new e-mail (account: %d, ip: %s)" RETCODE,
				server[id].name, acc, ip);
			else if (strcmpi(new_email, "a@a.com") == 0)
				ShowWarning("Char-server '%s': Attempt to modify an e-mail on an account (@email GM command) with a default e-mail (account: %d, ip: %s)" RETCODE,
				server[id].name, acc, ip);
			else {
				sprintf(tmpsql, "SELECT `%s`,`email` FROM `%s` WHERE `%s` = '%d'", login_db_userid, login_db, login_db_account_id, acc);
				if (mysql_query(&mysql_handle, tmpsql))
				{
					ShowSQL("DB error - %s\n",mysql_error(&mysql_handle));
					ShowDebug("at %s:%d - %s\n", __FILE__,__LINE__,tmpsql);
				}
				sql_res = mysql_store_result(&mysql_handle);
				if (sql_res) {
					sql_row = mysql_fetch_row(sql_res);	//row fetching

					if (strcmpi(sql_row[1], actual_email) == 0) {
						sprintf(tmpsql, "UPDATE `%s` SET `email` = '%s' WHERE `%s` = '%d'", login_db, new_email, login_db_account_id, acc);
						// query
						if (mysql_query(&mysql_handle, tmpsql)) {
							ShowSQL("DB error - %s\n",mysql_error(&mysql_handle));
							ShowDebug("at %s:%d - %s\n", __FILE__,__LINE__,tmpsql);
						}
						ShowInfo("Char-server '%s': Modify an e-mail on an account (@email GM command) (account: %d (%s), new e-mail: %s, ip: %s)." RETCODE,
							server[id].name, acc, sql_row[0], actual_email, ip);
					}
				}

			}
			RFIFOSKIP(fd, 86);
			break;
		}

		case 0x2724:	// Receiving of map-server via char-server a status change resquest (by Yor)
			if (RFIFOREST(fd) < 10)
				return 0;
		{
			int acc, statut;
			acc = RFIFOL(fd,2);
			statut = RFIFOL(fd,6);
			sprintf(tmpsql, "SELECT `state` FROM `%s` WHERE `%s` = '%d'", login_db, login_db_account_id, acc);
			if (mysql_query(&mysql_handle, tmpsql)) {
				ShowSQL("DB error - %s\n",mysql_error(&mysql_handle));
				ShowDebug("at %s:%d - %s\n", __FILE__,__LINE__,tmpsql);
			}
			sql_res = mysql_store_result(&mysql_handle);
			if (sql_res) {
				sql_row = mysql_fetch_row(sql_res); // row fetching
			}
			if (atoi(sql_row[0]) != statut && statut != 0) {
				unsigned char buf[16];
				WBUFW(buf,0) = 0x2731;
				WBUFL(buf,2) = acc;
				WBUFB(buf,6) = 0; // 0: change of statut, 1: ban
				WBUFL(buf,7) = statut; // status or final date of a banishment
				charif_sendallwos(-1, buf, 11);
			}
			sprintf(tmpsql,"UPDATE `%s` SET `state` = '%d' WHERE `%s` = '%d'", login_db, statut,login_db_account_id,acc);
			//query
			if(mysql_query(&mysql_handle, tmpsql)) {
				ShowSQL("DB error - %s\n",mysql_error(&mysql_handle));
				ShowDebug("at %s:%d - %s\n", __FILE__,__LINE__,tmpsql);
			}
			RFIFOSKIP(fd,10);
			break;
		}

		case 0x2725: // Receiving of map-server via char-server a ban resquest (by Yor)
			if (RFIFOREST(fd) < 18)
				return 0;
		{
			int acc;
			struct tm *tmtime;
			time_t timestamp, tmptime;
			acc = RFIFOL(fd,2);
			sprintf(tmpsql, "SELECT `ban_until` FROM `%s` WHERE `%s` = '%d'",login_db,login_db_account_id,acc);
			if (mysql_query(&mysql_handle, tmpsql)) {
				ShowSQL("DB error - %s\n",mysql_error(&mysql_handle));
				ShowDebug("at %s:%d - %s\n", __FILE__,__LINE__,tmpsql);
			}
			sql_res = mysql_store_result(&mysql_handle);
			if (sql_res) {
				sql_row = mysql_fetch_row(sql_res); // row fetching
			}
			tmptime = atol(sql_row[0]);
			if (tmptime == 0 || tmptime < time(NULL))
				timestamp = time(NULL);
			else
				timestamp = tmptime;
			tmtime = localtime(&timestamp);
			tmtime->tm_year = tmtime->tm_year + (short)RFIFOW(fd,6);
			tmtime->tm_mon = tmtime->tm_mon + (short)RFIFOW(fd,8);
			tmtime->tm_mday = tmtime->tm_mday + (short)RFIFOW(fd,10);
			tmtime->tm_hour = tmtime->tm_hour + (short)RFIFOW(fd,12);
			tmtime->tm_min = tmtime->tm_min + (short)RFIFOW(fd,14);
			tmtime->tm_sec = tmtime->tm_sec + (short)RFIFOW(fd,16);
			timestamp = mktime(tmtime);
			if (timestamp != -1) {
				if (timestamp <= time(NULL))
					timestamp = 0;
				if (tmptime != timestamp) {
					if (timestamp != 0) {
						unsigned char buf[16];
						WBUFW(buf,0) = 0x2731;
						WBUFL(buf,2) = acc;
						WBUFB(buf,6) = 1; // 0: change of statut, 1: ban
						WBUFL(buf,7) = (unsigned int)timestamp; // status or final date of a banishment
						charif_sendallwos(-1, buf, 11);
					}
					ShowNotice("Account: %d Banned until: %ld\n", acc, timestamp);
					sprintf(tmpsql, "UPDATE `%s` SET `ban_until` = '%ld' WHERE `%s` = '%d'", login_db, (unsigned long)timestamp, login_db_account_id, acc);
					// query
					if (mysql_query(&mysql_handle, tmpsql)) {
						ShowSQL("DB error - %s\n",mysql_error(&mysql_handle));
						ShowDebug("at %s:%d - %s\n", __FILE__,__LINE__,tmpsql);
					}
				}
			}
			RFIFOSKIP(fd,18);
			break;
		}

		case 0x2727:
			if (RFIFOREST(fd) < 6)
				return 0;
		{
			int acc,sex;
			unsigned char buf[16];
			acc=RFIFOL(fd,2);
			sprintf(tmpsql,"SELECT `sex` FROM `%s` WHERE `%s` = '%d'",login_db,login_db_account_id,acc);

			if(mysql_query(&mysql_handle, tmpsql)) {
				ShowSQL("DB error - %s\n",mysql_error(&mysql_handle));
				ShowDebug("at %s:%d - %s\n", __FILE__,__LINE__,tmpsql);
				return 0;
			}

			sql_res = mysql_store_result(&mysql_handle) ;

			if (sql_res)	{
				if (mysql_num_rows(sql_res) == 0) {
					mysql_free_result(sql_res);
					return 0;
				}
				sql_row = mysql_fetch_row(sql_res);	//row fetching
			}

			if (strcmpi(sql_row[0], "M") == 0)
				sex = 0; //Change to female
			else
				sex = 1; //Change to make
			sprintf(tmpsql,"UPDATE `%s` SET `sex` = '%c' WHERE `%s` = '%d'", login_db, (sex?'M':'F'), login_db_account_id, acc);
			//query
			if(mysql_query(&mysql_handle, tmpsql)) {
				ShowSQL("DB error - %s\n",mysql_error(&mysql_handle));
				ShowDebug("at %s:%d - %s\n", __FILE__,__LINE__,tmpsql);
			}
			WBUFW(buf,0) = 0x2723;
			WBUFL(buf,2) = acc;
			WBUFB(buf,6) = sex;
			charif_sendallwos(-1, buf, 7);
			RFIFOSKIP(fd,6);
			break;
		}

		case 0x2728:	// save account_reg2
			if (RFIFOREST(fd) < 4 || RFIFOREST(fd) < RFIFOW(fd,2))
				return 0;
			if (RFIFOL(fd,4) > 0) {
				int acc,p,j,len;
				char str[32];
				char temp_str[64]; //Needs twice as much space as the original string.
				char temp_str2[512];
				char value[256];
				unsigned char *buf;
				acc=RFIFOL(fd,4);
				buf = (unsigned char*)aCalloc(RFIFOW(fd,2)+1, sizeof(unsigned char));
				//Delete all global account variables....
				sprintf(tmpsql,"DELETE FROM `%s` WHERE `type`='1' AND `account_id`='%d';",reg_db,acc);
				if(mysql_query(&mysql_handle, tmpsql)) {
					ShowSQL("DB error - %s\n",mysql_error(&mysql_handle));
					ShowDebug("at %s:%d - %s\n", __FILE__,__LINE__,tmpsql);
				}
				//Proceed to insert them....
				for(j=0,p=13;j<ACCOUNT_REG2_NUM && p<RFIFOW(fd,2);j++){
					sscanf(RFIFOP(fd,p), "%31c%n",str,&len);
					str[len]='\0';
					p +=len+1; //+1 to skip the '\0' between strings.
					sscanf(RFIFOP(fd,p), "%255c%n",value,&len);
					value[len]='\0';
					p +=len+1;

					sprintf(tmpsql,"INSERT INTO `%s` (`type`, `account_id`, `str`, `value`) VALUES ( 1 , '%d' , '%s' , '%s');",  reg_db, acc, jstrescapecpy(temp_str,str), jstrescapecpy(temp_str2,value));
					if(mysql_query(&mysql_handle, tmpsql)) {
						ShowSQL("DB error - %s\n",mysql_error(&mysql_handle));
						ShowDebug("at %s:%d - %s\n", __FILE__,__LINE__,tmpsql);
					}
				}

				// Send to char
				memcpy(WBUFP(buf,0),RFIFOP(fd,0),RFIFOW(fd,2));
				WBUFW(buf,0)=0x2729;
				charif_sendallwos(fd,buf,WBUFW(buf,2));
				if (buf) aFree(buf);
			}
			RFIFOSKIP(fd,RFIFOW(fd,2));
			//printf("login: save account_reg (from char)\n");
			break;

		case 0x272a:	// Receiving of map-server via char-server a unban resquest (by Yor)
			if (RFIFOREST(fd) < 6)
				return 0;
			{
				int acc;
				acc = RFIFOL(fd,2);
				sprintf(tmpsql,"SELECT `ban_until` FROM `%s` WHERE `%s` = '%d'",login_db,login_db_account_id,acc);
				if(mysql_query(&mysql_handle, tmpsql)) {
					ShowSQL("DB error - %s\n",mysql_error(&mysql_handle));
					ShowDebug("at %s:%d - %s\n", __FILE__,__LINE__,tmpsql);
				}
				sql_res = mysql_store_result(&mysql_handle) ;
				if (sql_res && mysql_num_rows(sql_res) > 0) { //Found a match
					sprintf(tmpsql,"UPDATE `%s` SET `ban_until` = '0' WHERE `%s` = '%d'", login_db,login_db_account_id,acc);
					//query
					if(mysql_query(&mysql_handle, tmpsql)) {
						ShowSQL("DB error - %s\n",mysql_error(&mysql_handle));
						ShowDebug("at %s:%d - %s\n", __FILE__,__LINE__,tmpsql);
					}
				}
				if (sql_res) mysql_free_result(sql_res);
				RFIFOSKIP(fd,6);
			}
			return 0;

		case 0x272b:    // Set account_id to online [Wizputer]
			if (RFIFOREST(fd) < 6)
				return 0;
			add_online_user(id, RFIFOL(fd,2));
			RFIFOSKIP(fd,6);
			break;

		case 0x272c:   // Set account_id to offline [Wizputer]
			if (RFIFOREST(fd) < 6)
				return 0;
			remove_online_user(RFIFOL(fd,2));
			RFIFOSKIP(fd,6);
			break;
		case 0x272d:	// Receive list of all online accounts. [Skotlex]
			if (RFIFOREST(fd) < 4 || RFIFOREST(fd) < RFIFOW(fd,2))
				return 0;
			if (!online_check) {
				RFIFOSKIP(fd,RFIFOW(fd,2));
				break;	
			}
			{
				struct online_login_data *p;
				int aid, users;
				online_db->foreach(online_db,online_db_setoffline,id); //Set all chars from this char-server offline first
				users = RFIFOW(fd,4);
				for (i = 0; i < users; i++) {
					aid = RFIFOL(fd,6+i*4);
					p = idb_ensure(online_db, aid, create_online_user);
					p->char_server = id;
				}
				RFIFOSKIP(fd,RFIFOW(fd,2));
				break;
			}
		case 0x272e: //Request account_reg2 for a character.
			if (RFIFOREST(fd) < 10)
				return 0;
			{
				int account_id = RFIFOL(fd, 2);
				int char_id = RFIFOL(fd, 6);
				int p;
				WFIFOHEAD(fd,10000);
				RFIFOSKIP(fd,10);
				sprintf(tmpsql, "SELECT `str`,`value` FROM `%s` WHERE `type`='1' AND `account_id`='%d'",reg_db, account_id);
				if (mysql_query(&mysql_handle, tmpsql)) {
					ShowSQL("DB error - %s\n",mysql_error(&mysql_handle));
					ShowDebug("at %s:%d - %s\n", __FILE__,__LINE__,tmpsql);
					break;
				}
				sql_res = mysql_store_result(&mysql_handle) ;
				if (!sql_res) {
					break;
				}
				WFIFOW(fd,0) = 0x2729;
				WFIFOL(fd,4) = account_id;
				WFIFOL(fd,8) = char_id;
				WFIFOB(fd,12) = 1; //Type 1 for Account2 registry
				for(p = 13; (sql_row = mysql_fetch_row(sql_res)) && p < 9000;){
					if (sql_row[0][0]) {
						p+= sprintf(WFIFOP(fd,p), "%s", sql_row[0])+1; //We add 1 to consider the '\0' in place.
						p+= sprintf(WFIFOP(fd,p), "%s", sql_row[1])+1;
					}
				}
				if (p >= 9000)
					ShowWarning("Too many account2 registries for AID %d. Some registries were not sent.\n", account_id);
				WFIFOW(fd,2) = p;
				WFIFOSET(fd,WFIFOW(fd,2));
				mysql_free_result(sql_res);
			}
			break;

		case 0x2736: // WAN IP update from char-server
			if (RFIFOREST(fd) < 6)
				return 0;
			ShowInfo("Updated IP of Server #%d to %d.%d.%d.%d.\n",id,
			(int)RFIFOB(fd,2),(int)RFIFOB(fd,3),
			(int)RFIFOB(fd,4),(int)RFIFOB(fd,5));
			server[id].ip = RFIFOL(fd,2);
			RFIFOSKIP(fd,6);
			break;

		case 0x2737: //Request to set all offline.
			ShowInfo("Setting accounts from char-server %d offline.\n", id);
			online_db->foreach(online_db,online_db_setoffline,id);
			RFIFOSKIP(fd,2);
			break;

		default:
			ShowError("login: unknown packet %x! (from char).\n", RFIFOW(fd,0));
			session[fd]->eof = 1;
			return 0;
		}
	}

	RFIFOSKIP(fd,RFIFOREST(fd));
	return 0;
}

//--------------------------------------------
// Test to know if an IP come from LAN or WAN.
// Rewrote: Adnvanced subnet check [LuzZza]
//--------------------------------------------
int lan_subnetcheck(long p) {

	int i;
	unsigned char *sbn, *msk, *src = (unsigned char *)&p;

	for(i=0; i<subnet_count; i++) {

		if(subnet[i].subnet == (p & subnet[i].mask)) {

			sbn = (unsigned char *)&subnet[i].subnet;
			msk = (unsigned char *)&subnet[i].mask;

			ShowInfo("Subnet check [%u.%u.%u.%u]: Matches "CL_CYAN"%u.%u.%u.%u/%u.%u.%u.%u"CL_RESET"\n",
				src[0], src[1], src[2], src[3], sbn[0], sbn[1], sbn[2], sbn[3], msk[0], msk[1], msk[2], msk[3]);

			return subnet[i].char_ip;
		}
	}

	ShowInfo("Subnet check [%u.%u.%u.%u]: "CL_CYAN"WAN"CL_RESET"\n", src[0], src[1], src[2], src[3]);
	return 0;
}

int login_ip_ban_check(unsigned char *p, unsigned long ipl)
{
	MYSQL_RES* sql_res;
	MYSQL_ROW  sql_row;
	//ip ban
	//p[0], p[1], p[2], p[3]
	//request DB connection
	//check
	sprintf(tmpsql, "SELECT count(*) FROM `ipbanlist` WHERE `list` = '%d.*.*.*' OR `list` = '%d.%d.*.*' OR `list` = '%d.%d.%d.*' OR `list` = '%d.%d.%d.%d'",
		p[0], p[0], p[1], p[0], p[1], p[2], p[0], p[1], p[2], p[3]);
	if (mysql_query(&mysql_handle, tmpsql)) {
		ShowSQL("DB error - %s\n",mysql_error(&mysql_handle));
		ShowDebug("at %s:%d - %s\n", __FILE__,__LINE__,tmpsql);
		// close connection because we can't verify their connectivity.
		return 1;
	}
	sql_res = mysql_store_result(&mysql_handle) ;
	sql_row = sql_res?mysql_fetch_row(sql_res):NULL;	//row fetching

	if(!sql_row) return 1; //Shouldn't happen, but just in case...

	if (atoi(sql_row[0]) == 0) { //No ban
		mysql_free_result(sql_res);
		return 0;
	}
		
	// ip ban ok.
	ShowWarning("packet from banned ip : %d.%d.%d.%d\n" RETCODE, p[0], p[1], p[2], p[3]);

	if (log_login)
	{
		sprintf(tmpsql,"INSERT DELAYED INTO `%s`(`time`,`ip`,`user`,`rcode`,`log`) VALUES (NOW(), '%u', 'unknown','-3', 'ip banned')", loginlog_db, (unsigned int)ntohl(ipl));
		// query
		if(mysql_query(&mysql_handle, tmpsql)) {
			ShowSQL("DB error - %s\n",mysql_error(&mysql_handle));
			ShowDebug("at %s:%d - %s\n", __FILE__,__LINE__,tmpsql);
		}
	}
	mysql_free_result(sql_res);
	return 1;
}
//----------------------------------------------------------------------------------------
// Default packet parsing (normal players or administation/char-server connection requests)
//----------------------------------------------------------------------------------------
int parse_login(int fd) {
	//int len;

	MYSQL_RES* sql_res ;
	MYSQL_ROW  sql_row = NULL;

	char t_uid[100];
	struct mmo_account account;
	long subnet_char_ip;
	int packet_len;

	int result, i;
	unsigned char *p = (unsigned char *) &session[fd]->client_addr.sin_addr.s_addr;
	unsigned long ipl = session[fd]->client_addr.sin_addr.s_addr;
	char ip[16];
	RFIFOHEAD(fd);

	sprintf(ip, "%d.%d.%d.%d", p[0], p[1], p[2], p[3]);

	memset(&account, 0, sizeof(account));

	if (session[fd]->eof) {
		for(i = 0; i < MAX_SERVERS; i++)
			if (server_fd[i] == fd)
				server_fd[i] = -1;
		do_close(fd);
		return 0;
	}

	while(RFIFOREST(fd)>=2 && !session[fd]->eof){
//		ShowDebug("parse_login : %d %d packet case=%x\n", fd, RFIFOREST(fd), RFIFOW(fd,0));

		switch(RFIFOW(fd,0)){
		case 0x200:		// New alive packet: structure: 0x200 <account.userid>.24B. used to verify if client is always alive.
			if (RFIFOREST(fd) < 26)
				return 0;
			RFIFOSKIP(fd,26);
			break;

		case 0x204:		// New alive packet: structure: 0x204 <encrypted.account.userid>.16B. (new ragexe from 22 june 2004)
			if (RFIFOREST(fd) < 18)
				return 0;
			RFIFOSKIP(fd,18);
			break;

		case 0x277: // New login packet
		case 0x64:		// request client login
		case 0x01dd:	// request client login with encrypt

			packet_len = RFIFOREST(fd);

			//Perform ip-ban check ONLY on login packets
			if (ipban > 0 && login_ip_ban_check(p,ipl))
			{
				RFIFOSKIP(fd,packet_len);
				session[fd]->eof = 1;
				break;
			}

			switch(RFIFOW(fd,0)){
				case 0x64:
					if(packet_len < 55)
						return 0;
					break;
				case 0x01dd:
					if(packet_len < 47)
						return 0;
					break;
				case 0x277:
					if(packet_len < 84)
						return 0;
					break;
			}

			account.version = RFIFOL(fd, 2);
			if (!account.version) account.version = 1; //Force some version...
			memcpy(account.userid,RFIFOP(fd, 6),NAME_LENGTH);
			account.userid[23] = '\0';
			memcpy(account.passwd,RFIFOP(fd, 30),NAME_LENGTH);
			account.passwd[23] = '\0';

			ShowInfo("client connection request %s from %d.%d.%d.%d\n", RFIFOP(fd, 6), p[0], p[1], p[2], p[3]);
#ifdef PASSWORDENC
			account.passwdenc= (RFIFOW(fd,0)!=0x01dd)?0:PASSWORDENC;
#else
			account.passwdenc=0;
#endif
			result=mmo_auth(&account, fd);


			jstrescapecpy(t_uid,account.userid);
			if(result==-1){
				// as we have queried account level earlier in mmo_auth anyway, no need to do this again [zzo]
				//		    int gm_level = isGM(account.account_id); // removed by [zzo]

				if (min_level_to_connect > account.level) {
					WFIFOHEAD(fd,3);
					WFIFOW(fd,0) = 0x81;
					WFIFOB(fd,2) = 1; // 01 = Server closed
					WFIFOSET(fd,3);
				} else {
					WFIFOHEAD(fd,47+32*MAX_SERVERS);
					if (p[0] != 127 && log_login) {
						sprintf(tmpsql,"INSERT DELAYED INTO `%s`(`time`,`ip`,`user`,`rcode`,`log`) VALUES (NOW(), '%u', '%s','100', 'login ok')", loginlog_db, (unsigned int)ntohl(ipl), t_uid);
						//query
						if(mysql_query(&mysql_handle, tmpsql)) {
							ShowSQL("DB error - %s\n",mysql_error(&mysql_handle));
							ShowDebug("at %s:%d - %s\n", __FILE__,__LINE__,tmpsql);
						}
					}
					if (account.level)
						ShowStatus("Connection of the GM (level:%d) account '%s' accepted.\n", account.level, account.userid);
					else
						ShowStatus("Connection of the account '%s' accepted.\n", account.userid);
					server_num=0;
					for(i = 0; i < MAX_SERVERS; i++) {
						if (server_fd[i] >= 0) {
							// Advanced subnet check [LuzZza]
							if((subnet_char_ip = lan_subnetcheck(ipl)))
								WFIFOL(fd,47+server_num*32) = subnet_char_ip;
							else
								WFIFOL(fd,47+server_num*32) = server[i].ip;
							WFIFOW(fd,47+server_num*32+4) = server[i].port;
							memcpy(WFIFOP(fd,47+server_num*32+6), server[i].name, 20);
							WFIFOW(fd,47+server_num*32+26) = server[i].users;
							WFIFOW(fd,47+server_num*32+28) = server[i].maintenance;
							WFIFOW(fd,47+server_num*32+30) = server[i].new_;
							server_num++;
						}
					}
					// if at least 1 char-server
					if (server_num > 0) {
						WFIFOW(fd,0)=0x69;
						WFIFOW(fd,2)=47+32*server_num;
						WFIFOL(fd,4)=account.login_id1;
						WFIFOL(fd,8)=account.account_id;
						WFIFOL(fd,12)=account.login_id2;
						WFIFOL(fd,16)=0;
						memcpy(WFIFOP(fd,20),account.lastlogin,24);
						WFIFOB(fd,46)=account.sex;
						WFIFOSET(fd,47+32*server_num);
						if(auth_fifo_pos>=AUTH_FIFO_SIZE)
							auth_fifo_pos=0;
						auth_fifo[auth_fifo_pos].account_id=account.account_id;
						auth_fifo[auth_fifo_pos].login_id1=account.login_id1;
						auth_fifo[auth_fifo_pos].login_id2=account.login_id2;
						auth_fifo[auth_fifo_pos].sex=account.sex;
						auth_fifo[auth_fifo_pos].delflag=0;
						auth_fifo[auth_fifo_pos].ip = session[fd]->client_addr.sin_addr.s_addr;
						auth_fifo_pos++;
					} else {
						WFIFOW(fd,0) = 0x81;
						WFIFOB(fd,2) = 1; // 01 = Server closed
						WFIFOSET(fd,3);
					}
				}
			} else {
				char tmp_sql[512];
				char error[64];
				WFIFOHEAD(fd,23);
				if (log_login)
				{
					sprintf(tmp_sql,"INSERT DELAYED INTO `%s`(`time`,`ip`,`user`,`rcode`,`log`) VALUES (NOW(), '%u', '%s', '%d','login failed : %%s')", loginlog_db, (unsigned int)ntohl(ipl), t_uid, result);
					switch((result + 1)) {
					case -2:	//-3 = Account Banned
						sprintf(tmpsql,tmp_sql,"Account banned.");
						sprintf(error,"Account banned.");
						break;
					case -1:	//-2 = Dynamic Ban
						sprintf(tmpsql,tmp_sql,"dynamic ban (ip and account).");
						sprintf(error,"dynamic ban (ip and account).");
						break;
					case 1:	// 0 = Unregistered ID
						sprintf(tmpsql,tmp_sql,"Unregisterd ID.");
						sprintf(error,"Unregisterd ID.");
						break;
					case 2:	// 1 = Incorrect Password
						sprintf(tmpsql,tmp_sql,"Incorrect Password.");
						sprintf(error,"Incorrect Password.");
						break;
					case 3:	// 2 = This ID is expired
						sprintf(tmpsql,tmp_sql,"Account Expired.");
						sprintf(error,"Account Expired.");
						break;
					case 4:	// 3 = Rejected from Server
						sprintf(tmpsql,tmp_sql,"Rejected from server.");
						sprintf(error,"Rejected from server.");
						break;
					case 5:	// 4 = You have been blocked by the GM Team
						sprintf(tmpsql,tmp_sql,"Blocked by GM.");
						sprintf(error,"Blocked by GM.");
						break;
					case 6:	// 5 = Your Game's EXE file is not the latest version
						sprintf(tmpsql,tmp_sql,"Not latest game EXE.");
						sprintf(error,"Not latest game EXE.");
						break;
					case 7:	// 6 = Your are Prohibited to log in until %s
						sprintf(tmpsql,tmp_sql,"Banned.");
						sprintf(error,"Banned.");
						break;
					case 8:	// 7 = Server is jammed due to over populated
						sprintf(tmpsql,tmp_sql,"Server Over-population.");
						sprintf(error,"Server Over-population.");
						break;
					case 9:	// 8 = No more accounts may be connected from this company
						sprintf(tmpsql,tmp_sql,"Account limit from company");
						sprintf(error,"Account limit from company");
						break;
					case 10:  // 9 = MSI_REFUSE_BAN_BY_DBA
						sprintf(tmpsql,tmp_sql,"Ban by DBA");
						sprintf(error,"Ban by DBA");
						break;
					case 11:  // 10 = MSI_REFUSE_EMAIL_NOT_CONFIRMED
						sprintf(tmpsql,tmp_sql,"Email not confirmed");
						sprintf(error,"Email not confirmed");
						break;
					case 12:  // 11 = MSI_REFUSE_BAN_BY_GM
						sprintf(tmpsql,tmp_sql,"Ban by GM");
						sprintf(error,"Ban by GM");
						break;
					case 13:  // 12 = MSI_REFUSE_TEMP_BAN_FOR_DBWORK
						sprintf(tmpsql,tmp_sql,"Working in DB");
						sprintf(error,"Working in DB");
						break;
					case 14:  // 13 = MSI_REFUSE_SELF_LOCK
						sprintf(tmpsql,tmp_sql,"Self Lock");
						sprintf(error,"Self Lock");
						break;
					case 15:  // 14 = MSI_REFUSE_NOT_PERMITTED_GROUP
						sprintf(tmpsql,tmp_sql,"Not Permitted Group");
						sprintf(error,"Not Permitted Group");
						break;
					case 16:  // 15 = MSI_REFUSE_NOT_PERMITTED_GROUP
						sprintf(tmpsql,tmp_sql,"Not Permitted Group");
						sprintf(error,"Not Permitted Group");
						break;
					case 100: // 99 = This ID has been totally erased
						sprintf(tmpsql,tmp_sql,"Account gone.");
						sprintf(error,"Account gone.");
						break;
					case 101: // 100 = Login information remains at %s
						sprintf(tmpsql,tmp_sql,"Login info remains.");
						sprintf(error,"Login info remains.");
						break;
					case 102: // 101 = Account has been locked for a hacking investigation. Please contact the GM Team for more information
						sprintf(tmpsql,tmp_sql,"Hacking investigation.");
						sprintf(error,"Hacking investigation.");
						break;
					case 103: // 102 = This account has been temporarily prohibited from login due to a bug-related investigation
						sprintf(tmpsql,tmp_sql,"Bug investigation.");
						sprintf(error,"Bug investigation.");
						break;
					case 104: // 103 = This character is being deleted. Login is temporarily unavailable for the time being
						sprintf(tmpsql,tmp_sql,"Deleting char.");
						sprintf(error,"Deleting char.");
						break;
					case 105: // 104 = This character is being deleted. Login is temporarily unavailable for the time being
						sprintf(tmpsql,tmp_sql,"Deleting spouse char.");
						sprintf(error,"Deleting spouse char.");
						break;
					default:
						sprintf(tmpsql,tmp_sql,"Unknown Error.");
						sprintf(error,"Unknown Error.");
						break;
					}
					//query
					if(mysql_query(&mysql_handle, tmpsql)) {
						ShowSQL("DB error - %s\n",mysql_error(&mysql_handle));
						ShowDebug("at %s:%d - %s\n", __FILE__,__LINE__,tmpsql);
					}
				} //End login log of error.
				if ((result == 1) && (dynamic_pass_failure_ban != 0) && log_login){	// failed password
					sprintf(tmpsql,"SELECT count(*) FROM `%s` WHERE `ip` = '%u' AND `rcode` = '1' AND `time` > NOW() - INTERVAL %d MINUTE",
						loginlog_db,(unsigned int)ntohl(ipl), dynamic_pass_failure_ban_time);	//how many times filed account? in one ip.
					if(mysql_query(&mysql_handle, tmpsql)) {
						ShowSQL("DB error - %s\n",mysql_error(&mysql_handle));
						ShowDebug("at %s:%d - %s\n", __FILE__,__LINE__,tmpsql);
					}
					//check query result
					sql_res = mysql_store_result(&mysql_handle) ;
					sql_row = sql_res?mysql_fetch_row(sql_res):NULL;	//row fetching

					if (sql_row && atoi(sql_row[0]) >= dynamic_pass_failure_ban_how_many ) {
						sprintf(tmpsql,"INSERT INTO `ipbanlist`(`list`,`btime`,`rtime`,`reason`) VALUES ('%d.%d.%d.*', NOW() , NOW() +  INTERVAL %d MINUTE ,'Password error ban: %s')", p[0], p[1], p[2], dynamic_pass_failure_ban_how_long, t_uid);
						if(mysql_query(&mysql_handle, tmpsql)) {
							ShowSQL("DB error - %s\n",mysql_error(&mysql_handle));
							ShowDebug("at %s:%d - %s\n", __FILE__,__LINE__,tmpsql);
						}
					}
					if(sql_res) mysql_free_result(sql_res);
				}
				else if (result == -2){	//dynamic banned - add ip to ban list.
					sprintf(tmpsql,"INSERT INTO `ipbanlist`(`list`,`btime`,`rtime`,`reason`) VALUES ('%d.%d.%d.*', NOW() , NOW() +  INTERVAL 1 MONTH ,'Dynamic banned user id : %s')", p[0], p[1], p[2], t_uid);
					if(mysql_query(&mysql_handle, tmpsql)) {
						ShowSQL("DB error - %s\n",mysql_error(&mysql_handle));
						ShowDebug("at %s:%d - %s\n", __FILE__,__LINE__,tmpsql);
					}
					result = -3;
				}else if(result == 6){ //not lastet version ..
					//result = 5;
				}

				sprintf(tmpsql,"SELECT `ban_until` FROM `%s` WHERE `%s` = %s '%s'",login_db, login_db_userid, case_sensitive ? "BINARY" : "", t_uid);
				if(mysql_query(&mysql_handle, tmpsql)) {
					ShowSQL("DB error - %s\n",mysql_error(&mysql_handle));
					ShowDebug("at %s:%d - %s\n", __FILE__,__LINE__,tmpsql);
				}
				sql_res = mysql_store_result(&mysql_handle) ;
				sql_row = sql_res?mysql_fetch_row(sql_res):NULL;	//row fetching
				//cannot connect login failed
				memset(WFIFOP(fd,0),'\0',23);
				WFIFOW(fd,0)=0x6a;
				WFIFOB(fd,2)=result;
				if (result == 6) { // 6 = Your are Prohibited to log in until %s
					if (sql_row && atol(sql_row[0]) != 0) { // if account is banned, we send ban timestamp
						char tmpstr[256];
						time_t ban_until_time;
						ban_until_time = atol(sql_row[0]);
						strftime(tmpstr, 20, date_format, localtime(&ban_until_time));
						tmpstr[19] = '\0';
						memcpy(WFIFOP(fd,3), tmpstr, 20);
					} else { // we send error message
						memcpy(WFIFOP(fd,3), error, 20);
					}
				}
				WFIFOSET(fd,23);
			}
			RFIFOSKIP(fd,packet_len);
			break;

		case 0x01db:	// request password key
			if (session[fd]->session_data) {
				ShowWarning("login: abnormal request of MD5 key (already opened session).\n");
				session[fd]->eof = 1;
				return 0;
			}
		{
			WFIFOHEAD(fd,4+md5keylen);
			WFIFOW(fd,0)=0x01dc;
			WFIFOW(fd,2)=4+md5keylen;
			memcpy(WFIFOP(fd,4),md5key,md5keylen);
			WFIFOSET(fd,WFIFOW(fd,2));

			ShowDebug("Request Password key -%s\n",md5key);
			RFIFOSKIP(fd,2);
		}
			break;

		case 0x2710:	// request Char-server connection
			if(RFIFOREST(fd)<86)
				return 0;
			{
				unsigned char* server_name;
				WFIFOHEAD(fd, 3);
				memcpy(account.userid,RFIFOP(fd, 2),NAME_LENGTH);
				account.userid[23] = '\0';
				memcpy(account.passwd,RFIFOP(fd, 26),NAME_LENGTH);
				account.passwd[23] = '\0';
				account.passwdenc = 0;
				server_name = RFIFOP(fd,60);
				server_name[20] = '\0';
				ShowInfo("server connection request %s @ %d.%d.%d.%d:%d (%d.%d.%d.%d)\n",
					server_name, RFIFOB(fd, 54), RFIFOB(fd, 55), RFIFOB(fd, 56), RFIFOB(fd, 57), RFIFOW(fd, 58),
					p[0], p[1], p[2], p[3]);
				jstrescapecpy(t_uid,server_name);
				if (log_login)
				{
					char t_login[50];
					jstrescapecpy(t_login,account.userid);
					sprintf(tmpsql,"INSERT DELAYED INTO `%s`(`time`,`ip`,`user`,`rcode`,`log`) VALUES (NOW(), '%u', '%s@%s','100', 'charserver - %s@%d.%d.%d.%d:%d')",
						loginlog_db, (unsigned int)ntohl(ipl),
						t_login, t_uid, t_uid,
						RFIFOB(fd, 54), RFIFOB(fd, 55), RFIFOB(fd, 56), RFIFOB(fd, 57),
						RFIFOW(fd, 58));

					//query
					if(mysql_query(&mysql_handle, tmpsql)) {
						ShowSQL("DB error - %s\n",mysql_error(&mysql_handle));
						ShowDebug("at %s:%d - %s\n", __FILE__,__LINE__,tmpsql);
					}
				}
				result = mmo_auth(&account, fd);
				//printf("Result: %d - Sex: %d - Account ID: %d\n",result,account.sex,(int) account.account_id);

				if(result == -1 && account.sex==2 && account.account_id<MAX_SERVERS && server_fd[account.account_id]==-1){
					ShowStatus("Connection of the char-server '%s' accepted.\n", server_name);
					memset(&server[account.account_id], 0, sizeof(struct mmo_char_server));
					server[account.account_id].ip=RFIFOL(fd,54);
					server[account.account_id].port=RFIFOW(fd,58);
					memcpy(server[account.account_id].name,server_name,20);
					server[account.account_id].users=0;
					server[account.account_id].maintenance=RFIFOW(fd,82);
					server[account.account_id].new_=RFIFOW(fd,84);
					server_fd[account.account_id]=fd;
					sprintf(tmpsql,"DELETE FROM `sstatus` WHERE `index`='%ld'", account.account_id);
					//query
					if(mysql_query(&mysql_handle, tmpsql)) {
						ShowSQL("DB error - %s\n",mysql_error(&mysql_handle));
						ShowDebug("at %s:%d - %s\n", __FILE__,__LINE__,tmpsql);
					}

					sprintf(tmpsql,"INSERT INTO `sstatus`(`index`,`name`,`user`) VALUES ( '%ld', '%s', '%d')",
						account.account_id, t_uid,0);
					//query
					if(mysql_query(&mysql_handle, tmpsql)) {
						ShowSQL("DB error - %s\n",mysql_error(&mysql_handle));
						ShowDebug("at %s:%d - %s\n", __FILE__,__LINE__,tmpsql);
					}
					WFIFOW(fd,0)=0x2711;
					WFIFOB(fd,2)=0;
					WFIFOSET(fd,3);
					session[fd]->func_parse=parse_fromchar;
					realloc_fifo(fd,FIFOSIZE_SERVERLINK,FIFOSIZE_SERVERLINK);
					// send GM account to char-server
					send_GM_accounts(fd);
				} else {
					WFIFOW(fd, 0) =0x2711;
					WFIFOB(fd, 2)=3;
					WFIFOSET(fd, 3);
				}
			}
			RFIFOSKIP(fd, 86);
			return 0;

		case 0x7530:	// request Athena information
		{
			WFIFOHEAD(fd,10);
			WFIFOW(fd,0)=0x7531;
			WFIFOB(fd,2)=ATHENA_MAJOR_VERSION;
			WFIFOB(fd,3)=ATHENA_MINOR_VERSION;
			WFIFOB(fd,4)=ATHENA_REVISION;
			WFIFOB(fd,5)=ATHENA_RELEASE_FLAG;
			WFIFOB(fd,6)=ATHENA_OFFICIAL_FLAG;
			WFIFOB(fd,7)=ATHENA_SERVER_LOGIN;
			WFIFOW(fd,8)=ATHENA_MOD_VERSION;
			WFIFOSET(fd,10);
			RFIFOSKIP(fd,2);
			ShowInfo ("Athena version check...\n");
			break;
		}
		case 0x7532:
			ShowStatus ("End of connection (ip: %s)" RETCODE, ip);
			session[fd]->eof = 1;
			break;
		default:
			ShowStatus ("Abnormal end of connection (ip: %s): Unknown packet 0x%x " RETCODE, ip, RFIFOW(fd,0));
			session[fd]->eof = 1;
			return 0;
		}
	}

	RFIFOSKIP(fd,RFIFOREST(fd));
	return 0;
}

// Console Command Parser [Wizputer]
int parse_console(char* buf)
{
	char command[256];

	memset(command, 0, sizeof(command));

	sscanf(buf, "%[^\n]", command);

	//login_log("Console command :%s" RETCODE, command);

	if( strcmpi("shutdown", command) == 0 ||
		strcmpi("exit", command) == 0 ||
		strcmpi("quit", command) == 0 ||
		strcmpi("end", command) == 0 )
		runflag = 0;
	else if( strcmpi("alive", command) == 0 ||
			strcmpi("status", command) == 0 )
		ShowInfo(CL_CYAN"Console: "CL_BOLD"I'm Alive."CL_RESET"\n");
	else if( strcmpi("help", command) == 0 ){
		printf(CL_BOLD"Help of commands:"CL_RESET"\n");
		printf("  To shutdown the server:\n");
		printf("  'shutdown|exit|qui|end'\n");
		printf("  To know if server is alive:\n");
		printf("  'alive|status'\n");
	}

	return 0;
}

static int online_data_cleanup_sub(DBKey key, void *data, va_list ap)
{
	struct online_login_data *character= (struct online_login_data*)data;
	if (character->char_server == -2) //Unknown server.. set them offline
		remove_online_user(character->account_id);
	else if (character->char_server < 0)
		//Free data from players that have not been online for a while.
		db_remove(online_db, key);
	return 0;
}

static int online_data_cleanup(int tid, unsigned int tick, int id, int data)
{
	online_db->foreach(online_db, online_data_cleanup_sub);
	return 0;
} 

//-------------------------------------------------
// Return numerical value of a switch configuration
// on/off, english, français, deutsch, español
//-------------------------------------------------
int config_switch(const char *str) {
	if (strcmpi(str, "on") == 0 || strcmpi(str, "yes") == 0 || strcmpi(str, "oui") == 0 || strcmpi(str, "ja") == 0 || strcmpi(str, "si") == 0)
		return 1;
	if (strcmpi(str, "off") == 0 || strcmpi(str, "no") == 0 || strcmpi(str, "non") == 0 || strcmpi(str, "nein") == 0)
		return 0;

	return atoi(str);
}


//----------------------------------
// Reading Lan Support configuration
// Rewrote: Anvanced subnet check [LuzZza]
//----------------------------------
int login_lan_config_read(const char *lancfgName) {

	FILE *fp;
	int line_num = 0;
	char line[1024], w1[64], w2[64], w3[64], w4[64];

	if((fp = fopen(lancfgName, "r")) == NULL) {
		ShowWarning("LAN Support configuration file is not found: %s\n", lancfgName);
		return 1;
	}

	ShowInfo("Reading the configuration file %s...\n", lancfgName);

	while(fgets(line, sizeof(line)-1, fp)) {

		line_num++;		
		if ((line[0] == '/' && line[1] == '/') || line[0] == '\n' || line[1] == '\n')
			continue;

		line[sizeof(line)-1] = '\0';
		if(sscanf(line,"%[^:]: %[^:]:%[^:]:%[^\r\n]", w1, w2, w3, w4) != 4) {

			ShowWarning("Error syntax of configuration file %s in line %d.\n", lancfgName, line_num);	
			continue;
		}

		remove_control_chars((unsigned char *)w1);
		remove_control_chars((unsigned char *)w2);
		remove_control_chars((unsigned char *)w3);
		remove_control_chars((unsigned char *)w4);

		if(strcmpi(w1, "subnet") == 0) {

			subnet[subnet_count].mask = inet_addr(w2);
			subnet[subnet_count].char_ip = inet_addr(w3);
			subnet[subnet_count].map_ip = inet_addr(w4);
			subnet[subnet_count].subnet = subnet[subnet_count].char_ip&subnet[subnet_count].mask;
			if (subnet[subnet_count].subnet != (subnet[subnet_count].map_ip&subnet[subnet_count].mask)) {
				ShowError("%s: Configuration Error: The char server (%s) and map server (%s) belong to different subnetworks!\n", lancfgName, w3, w4);
				continue;
			}

			subnet_count++;
		}

		ShowStatus("Read information about %d subnetworks.\n", subnet_count);
	}

	fclose(fp);
	return 0;
}

//-----------------------------------------------------
//BANNED IP CHECK.
//-----------------------------------------------------
int ip_ban_check(int tid, unsigned int tick, int id, int data){

	//query
	if(mysql_query(&mysql_handle, "DELETE FROM `ipbanlist` WHERE `rtime` <= NOW()")) {
		ShowSQL("DB error - %s\n",mysql_error(&mysql_handle));
		ShowDebug("at %s:%d - DELETE FROM `ipbanlist` WHERE `rtime` <= NOW()\n", __FILE__,__LINE__);
	}

	return 0;
}

//-----------------------------------------------------
// reading configuration
//-----------------------------------------------------
int login_config_read(const char *cfgName){
	int i;
	char line[1024], w1[1024], w2[1024];
	FILE *fp;

	fp=fopen(cfgName,"r");

	if(fp==NULL){
		ShowError("Configuration file (%s) not found.\n", cfgName);
		return 1;
	}
	ShowInfo("reading configuration file %s...\n", cfgName);
	while(fgets(line, sizeof(line)-1, fp)){
		if(line[0] == '/' && line[1] == '/')
			continue;

		i=sscanf(line,"%[^:]: %[^\r\n]",w1,w2);
		if(i!=2)
			continue;

		remove_control_chars((unsigned char *) w1);
		remove_control_chars((unsigned char *) w2);
		if(strcmpi(w1,"timestamp_format") == 0) {
			strncpy(timestamp_format, w2, 20);
		} else if(strcmpi(w1,"stdout_with_ansisequence")==0){
			stdout_with_ansisequence = config_switch(w2);
		} else if(strcmpi(w1,"console_silent")==0){
			msg_silent = 0; //To always allow the next line to show up.
			ShowInfo("Console Silent Setting: %d\n", atoi(w2));
			msg_silent = atoi(w2);
		} else if (strcmpi(w1, "bind_ip") == 0) {
			bind_ip = resolve_hostbyname(w2, NULL, bind_ip_str);
			if (bind_ip)
				ShowStatus("Login server binding IP address : %s -> %s\n", w2, bind_ip_str);
		} else if(strcmpi(w1,"login_port")==0){
			login_port=atoi(w2);
			ShowStatus("set login_port : %s\n",w2);
		}
		else if(strcmpi(w1,"ipban")==0){
			ipban=atoi(w2);
			ShowStatus("set ipban : %d\n",ipban);
		}
		//account ban -> ip ban
		else if(strcmpi(w1,"dynamic_account_ban")==0){
			dynamic_account_ban=atoi(w2);
			ShowStatus("set dynamic_account_ban : %d\n",dynamic_account_ban);
		}
		else if(strcmpi(w1,"dynamic_account_ban_class")==0){
			dynamic_account_ban_class=atoi(w2);
			ShowStatus("set dynamic_account_ban_class : %d\n",dynamic_account_ban_class);
		}
		//dynamic password error ban
		else if(strcmpi(w1,"dynamic_pass_failure_ban")==0){
			dynamic_pass_failure_ban=atoi(w2);
			ShowStatus("set dynamic_pass_failure_ban : %d\n",dynamic_pass_failure_ban);
		}
		else if(strcmpi(w1,"dynamic_pass_failure_ban_time")==0){
			dynamic_pass_failure_ban_time=atoi(w2);
			ShowStatus("set dynamic_pass_failure_ban_time : %d\n",dynamic_pass_failure_ban_time);
		}
		else if(strcmpi(w1,"dynamic_pass_failure_ban_how_many")==0){
			dynamic_pass_failure_ban_how_many=atoi(w2);
			ShowStatus("set dynamic_pass_failure_ban_how_many : %d\n",dynamic_pass_failure_ban_how_many);
		}
		else if(strcmpi(w1,"dynamic_pass_failure_ban_how_long")==0){
			dynamic_pass_failure_ban_how_long=atoi(w2);
			ShowStatus("set dynamic_pass_failure_ban_how_long : %d\n",dynamic_pass_failure_ban_how_long);
		} else if(strcmpi(w1, "new_account") == 0){ 	//Added by Sirius for new account _M/_F
			new_account_flag = atoi(w2);		//Added by Sirius for new account _M/_F		
		} else if(strcmpi(w1, "check_client_version") == 0){ 		//Added by Sirius for client version check
			//check_client_version = config_switch(w2); 		//Added by Sirius for client version check
			if(strcmpi(w2,"on") == 0 || strcmpi(w2,"yes") == 0 ){
				check_client_version = 1;
			} else if(strcmpi(w2,"off") == 0 || strcmpi(w2,"no") == 0 ){
				check_client_version = 0;
			}                                                                                          
		} else if(strcmpi(w1, "client_version_to_connect") == 0){	//Added by Sirius for client version check
			client_version_to_connect = atoi(w2);			//Added by SIrius for client version check
		} else if(strcmpi(w1,"use_MD5_passwords")==0){
			if (!strcmpi(w2,"yes")) {
				use_md5_passwds=1;
			} else if (!strcmpi(w2,"no")){
				use_md5_passwds=0;
			}
			ShowStatus("Using MD5 Passwords: %s \n",w2);
		}
		else if (strcmpi(w1, "date_format") == 0) { // note: never have more than 19 char for the date!
			switch (atoi(w2)) {
				case 0:
					strcpy(date_format, "%d-%m-%Y %H:%M:%S"); // 31-12-2004 23:59:59
					break;
				case 1:
					strcpy(date_format, "%m-%d-%Y %H:%M:%S"); // 12-31-2004 23:59:59
					break;
				case 2:
					strcpy(date_format, "%Y-%d-%m %H:%M:%S"); // 2004-31-12 23:59:59
					break;
				case 3:
					strcpy(date_format, "%Y-%m-%d %H:%M:%S"); // 2004-12-31 23:59:59
					break;
			}
		}
		else if (strcmpi(w1, "min_level_to_connect") == 0) {
			min_level_to_connect = atoi(w2);
		}
		else if (strcmpi(w1, "check_ip_flag") == 0) {
			check_ip_flag = config_switch(w2);
		}
		else if (strcmpi(w1, "console") == 0) {
			if(strcmpi(w2,"on") == 0 || strcmpi(w2,"yes") == 0 )
				console = 1;
		}
		else if (strcmpi(w1, "case_sensitive") == 0) {
			if(strcmpi(w2,"on") == 0 || strcmpi(w2,"yes") == 0 )
				case_sensitive = 1;
			if(strcmpi(w2,"off") == 0 || strcmpi(w2,"no") == 0 )
				case_sensitive = 0;
			else
				case_sensitive = atoi(w2);
		} else if (strcmpi(w1, "allowed_regs") == 0) { //account flood protection system [Kevin]
			allowed_regs = atoi(w2);
		} else if (strcmpi(w1, "time_allowed") == 0) {
			time_allowed = atoi(w2);
		} else if (strcmpi(w1, "online_check") == 0) {
			if(strcmpi(w2,"on") == 0 || strcmpi(w2,"yes") == 0 )
				online_check = 1;
			else if(strcmpi(w2,"off") == 0 || strcmpi(w2,"no") == 0 )
				online_check = 0;
			else
				online_check = atoi(w2);
		} else if (strcmpi(w1, "log_login") == 0) {
			if(strcmpi(w2,"on") == 0 || strcmpi(w2,"yes") == 0 )
				log_login = 1;
			else if(strcmpi(w2,"off") == 0 || strcmpi(w2,"no") == 0 )
				log_login = 0;
			else
				log_login = atoi(w2);
		} else if (strcmpi(w1, "import") == 0) {
			login_config_read(w2);
		} else if(strcmpi(w1,"use_dnsbl")==0) { // [Zido]
			use_dnsbl=atoi(w2);
		} else if(strcmpi(w1,"dnsbl_servers")==0) { // [Zido]
			strcpy(dnsbl_servs,w2);
		} else if(strcmpi(w1,"ip_sync_interval")==0) {
			ip_sync_interval = 1000*60*atoi(w2); //w2 comes in minutes.
		}
	}
	fclose(fp);
	ShowInfo("done reading %s.\n", cfgName);
	return 0;
}

void sql_config_read(const char *cfgName){ /* Kalaspuff, to get login_db */
	int i;
	char line[1024], w1[1024], w2[1024];
	FILE *fp=fopen(cfgName,"r");
	if(fp==NULL){
		ShowFatalError("file not found: %s\n",cfgName);
		exit(1);
	}
	ShowInfo("reading configuration file %s...\n", cfgName);
	while(fgets(line, sizeof(line)-1, fp)){
		if(line[0] == '/' && line[1] == '/')
			continue;
		i=sscanf(line,"%[^:]: %[^\r\n]",w1,w2);
		if(i!=2)
			continue;
		if(strcmpi(w1, "gm_read_method") == 0) {
			if(atoi(w2) == 0)
				login_gm_read = true;
			else
				login_gm_read = false;
		} else if(strcmpi(w1, "gm_db") == 0) {
			strcpy(gm_db, w2);
		} else if (strcmpi(w1, "login_db") == 0) {
			strcpy(login_db, w2);
		}
		//add for DB connection
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
		else if(strcmpi(w1,"connection_ping_interval")==0) {
			connection_ping_interval = atoi(w2);
		}
		else if(strcmpi(w1,"default_codepage")==0){
			strcpy(default_codepage, w2);
			ShowStatus ("set default_codepage : %s\n",w2);
		}
		//added for custom column names for custom login table
		else if(strcmpi(w1,"login_db_account_id")==0){
			strcpy(login_db_account_id, w2);
		}
		else if(strcmpi(w1,"login_db_userid")==0){
			strcpy(login_db_userid, w2);
		}
		else if(strcmpi(w1,"login_db_user_pass")==0){
			strcpy(login_db_user_pass, w2);
		}
		else if(strcmpi(w1,"login_db_level")==0){
			strcpy(login_db_level, w2);
		}
		else if (strcmpi(w1, "loginlog_db") == 0) {
			strcpy(loginlog_db, w2);
		}
		else if (strcmpi(w1, "reg_db") == 0) {
			strcpy(reg_db, w2);
		}
		//support the import command, just like any other config
		else if(strcmpi(w1,"import")==0){
			sql_config_read(w2);
		}
	}
	fclose(fp);
	ShowInfo("done reading %s.\n", cfgName);
}

//--------------------------------------
// Function called at exit of the server
//--------------------------------------
void do_final(void) {
	//sync account when terminating.
	//but no need when you using DBMS (mysql)
	ShowStatus("Terminating...\n");
	mmo_db_close();
	online_db->destroy(online_db, NULL);
	if (gm_account_db)
		aFree(gm_account_db);
}

void set_server_type(void)
{
	SERVER_TYPE = ATHENA_SERVER_LOGIN;
}
int do_init(int argc,char **argv){
	//initialize login server
	int i;

	//read login configue
	login_config_read( (argc>1)?argv[1]:LOGIN_CONF_NAME );
	sql_config_read(SQL_CONF_NAME);
	login_lan_config_read((argc > 2) ? argv[2] : LAN_CONF_NAME);
	//Generate Passworded Key.
	ShowInfo("Initializing md5key...\n");
	memset(md5key, 0, sizeof(md5key));
	md5keylen=rand()%4+12;
	for(i=0;i<md5keylen;i++)
		md5key[i]=rand()%255+1;
	ShowInfo("md5key setup complete\n");


	ShowInfo("set FIFO Size\n");
	for(i=0;i<AUTH_FIFO_SIZE;i++)
		auth_fifo[i].delflag=1;
	ShowInfo("set FIFO Size complete\n");

	ShowInfo("set max servers\n");
	for(i=0;i<MAX_SERVERS;i++)
		server_fd[i]=-1;
	ShowInfo("set max servers complete\n");
	//server port open & binding

	// Online user database init
	online_db = db_alloc(__FILE__,__LINE__,DB_INT,DB_OPT_RELEASE_DATA,sizeof(int));	// reinitialise
	add_timer_func_list(waiting_disconnect_timer, "waiting_disconnect_timer");

	login_fd = make_listen_bind(bind_ip?bind_ip:INADDR_ANY,login_port);

	//Auth start
	ShowInfo("Running mmo_auth_sqldb_init()\n");
	mmo_auth_sqldb_init();
	ShowInfo("finished mmo_auth_sqldb_init()\n");

	if(login_gm_read)
		//Read account information.
		read_gm_account();

	//set default parser as parse_login function
	set_defaultparse(parse_login);

	// ban deleter timer - 1 minute term
	ShowStatus("add interval tic (ip_ban_check)....\n");
	add_timer_func_list(ip_ban_check,"ip_ban_check");
	add_timer_interval(gettick()+10, ip_ban_check,0,0,60*1000);

	add_timer_func_list(online_data_cleanup, "online_data_cleanup");
	add_timer_interval(gettick() + 600*1000, online_data_cleanup, 0, 0, 600*1000); // every 10 minutes cleanup online account db.

	if (ip_sync_interval) {
		add_timer_func_list(sync_ip_addresses, "sync_ip_addresses");
		add_timer_interval(gettick() + ip_sync_interval, sync_ip_addresses, 0, 0, ip_sync_interval);
	}

	if (console) {
		set_defaultconsoleparse(parse_console);
		start_console();
	}

	new_reg_tick=gettick();

	ShowStatus("The login-server is "CL_GREEN"ready"CL_RESET" (Server is listening on the port %d).\n\n", login_port);

	return 0;
}
