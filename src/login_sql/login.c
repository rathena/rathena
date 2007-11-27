// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/cbasetypes.h"
#include "../common/mmo.h"
#include "../common/core.h"
#include "../common/socket.h"
#include "../common/db.h"
#include "../common/timer.h"
#include "../common/malloc.h"
#include "../common/strlib.h"
#include "../common/showmsg.h"
#include "../common/version.h"
#include "../common/md5calc.h"
#include "../common/sql.h"
#include "login.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h> // for stat/lstat/fstat

struct Login_Config login_config;

int login_fd; // login server socket
#define MAX_SERVERS 30
struct mmo_char_server server[MAX_SERVERS]; // char server data

// Advanced subnet check [LuzZza]
struct s_subnet {
	uint32 mask;
	uint32 char_ip;
	uint32 map_ip;
} subnet[16];

int subnet_count = 0;

struct gm_account* gm_account_db = NULL;
unsigned int GM_num = 0; // number of gm accounts

//Account registration flood protection [Kevin]
int allowed_regs = 1;
int time_allowed = 10; //in seconds
unsigned int new_reg_tick = 0;
int num_regs = 0;

Sql* sql_handle;

// database parameters
uint16 login_server_port = 3306;
char login_server_ip[32] = "127.0.0.1";
char login_server_id[32] = "ragnarok";
char login_server_pw[32] = "ragnarok";
char login_server_db[32] = "ragnarok";
char default_codepage[32] = "";

char login_db[256] = "login";
char loginlog_db[256] = "loginlog";
char reg_db[256] = "global_reg_value";

// added to help out custom login tables, without having to recompile
// source so options are kept in the login_athena.conf or the inter_athena.conf
char login_db_account_id[256] = "account_id";
char login_db_userid[256] = "userid";
char login_db_user_pass[256] = "user_pass";
char login_db_level[256] = "level";

//-----------------------------------------------------

struct login_session_data {
	uint16 md5keylen;
	char md5key[20];
};

// auth information of incoming clients
struct _auth_fifo auth_fifo[AUTH_FIFO_SIZE];
int auth_fifo_pos = 0;

//-----------------------------------------------------
// Online User Database [Wizputer]
//-----------------------------------------------------

struct online_login_data {
	int account_id;
	int waiting_disconnect;
	int char_server;
};

static DBMap* online_db; // int account_id -> struct online_login_data*
static int waiting_disconnect_timer(int tid, unsigned int tick, int id, int data);

static void* create_online_user(DBKey key, va_list args)
{
	struct online_login_data* p;
	CREATE(p, struct online_login_data, 1);
	p->account_id = key.i;
	p->char_server = -1;
	p->waiting_disconnect = -1;
	return p;
}

void add_online_user(int char_server, int account_id)
{
	struct online_login_data* p;
	if( !login_config.online_check )
		return;
	p = idb_ensure(online_db, account_id, create_online_user);
	p->char_server = char_server;
	if( p->waiting_disconnect != -1 )
	{
		delete_timer(p->waiting_disconnect, waiting_disconnect_timer);
		p->waiting_disconnect = -1;
	}
}

void remove_online_user(int account_id)
{
	if( !login_config.online_check )
		return;
	if( account_id == 99 )
	{// reset all to offline
		online_db->clear(online_db, NULL); // purge db
		return;
	}
	idb_remove(online_db, account_id);
}

static int waiting_disconnect_timer(int tid, unsigned int tick, int id, int data)
{
	struct online_login_data* p = idb_get(online_db, id);
	if( p != NULL && p->waiting_disconnect == id )
	{
		p->waiting_disconnect = -1;
		remove_online_user(id);
	}
	return 0;
}

//--------------------------------------------------------------------
// Packet send to all char-servers, except one (wos: without our self)
//--------------------------------------------------------------------
int charif_sendallwos(int sfd, uint8* buf, size_t len)
{
	int i, c;

	for( i = 0, c = 0; i < MAX_SERVERS; ++i )
	{
		int fd = server[i].fd;
		if( session_isValid(fd) && fd != sfd )
		{
			WFIFOHEAD(fd,len);
			memcpy(WFIFOP(fd,0), buf, len);
			WFIFOSET(fd,len);
			++c;
		}
	}

	return c;
}

//-----------------------------------------------------
// Read GM accounts
//-----------------------------------------------------
void read_gm_account(void)
{
	if( !login_config.login_gm_read )
		return;// char server's job

	if( SQL_ERROR == Sql_Query(sql_handle, "SELECT `%s`,`%s` FROM `%s` WHERE `%s` > '0'", login_db_account_id, login_db_level, login_db, login_db_level) )
	{
		Sql_ShowDebug(sql_handle);
		return;// Failed to read GM list!
	}

	RECREATE(gm_account_db, struct gm_account, (size_t)Sql_NumRows(sql_handle));

	for( GM_num = 0; SQL_SUCCESS == Sql_NextRow(sql_handle); ++GM_num )
	{
		char* account;
		char* level;

		Sql_GetData(sql_handle, 0, &account, NULL);
		Sql_GetData(sql_handle, 1, &level, NULL);

		gm_account_db[GM_num].account_id = atoi(account);
		gm_account_db[GM_num].level = atoi(level);
	}

	Sql_FreeResult(sql_handle);
}

//-----------------------------------------------------
// Send GM accounts to one or all char-servers
//-----------------------------------------------------
void send_GM_accounts(int fd)
{
	unsigned int i;
	uint8 buf[32767];
	uint16 len;

	if( !login_config.login_gm_read )
		return;

	len = 4;
	WBUFW(buf,0) = 0x2732;
	for( i = 0; i < GM_num; ++i )
	{
		// send only existing accounts. We can not create a GM account when server is online.
		if( gm_account_db[i].level > 0 )
		{
			WBUFL(buf,len) = gm_account_db[i].account_id;
			WBUFB(buf,len+4) = (uint8)gm_account_db[i].level;
			len += 5;
			if( len >= 32000 )
			{
				ShowWarning("send_GM_accounts: Too many accounts! Only %d out of %d were sent.\n", i, GM_num);
				break;
			}
		}
	}

	WBUFW(buf,2) = len;
	if( fd == -1 )// send to all charservers
		charif_sendallwos(-1, buf, len);
	else
	{// send only to target
		WFIFOHEAD(fd,len);
		memcpy(WFIFOP(fd,0), buf, len);
		WFIFOSET(fd,len);
	}

	return;
}

/*=============================================
 * Does a mysql_ping to all connection handles
 *---------------------------------------------*/
int login_sql_ping(int tid, unsigned int tick, int id, int data) 
{
	ShowInfo("Pinging SQL server to keep connection alive...\n");
	Sql_Ping(sql_handle);
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
	add_timer_func_list(login_sql_ping, "login_sql_ping");
	add_timer_interval(gettick() + connection_ping_interval*1000, login_sql_ping, 0, 0, connection_ping_interval*1000);

	return 0;
}

//-----------------------------------------------------
// Read Account database - mysql db
//-----------------------------------------------------
int mmo_auth_sqldb_init(void)
{
	ShowStatus("Login server init....\n");

	sql_handle = Sql_Malloc();

	// DB connection start
	ShowStatus("Connect Login Database Server....\n");
	if( SQL_ERROR == Sql_Connect(sql_handle, login_server_id, login_server_pw, login_server_ip, login_server_port, login_server_db) )
	{
		Sql_ShowDebug(sql_handle);
		Sql_Free(sql_handle);
		exit(EXIT_FAILURE);
	}
	else
	{
		ShowStatus("Connect success!\n");
	}

	if( default_codepage[0] != '\0' && SQL_ERROR == Sql_SetEncoding(sql_handle, default_codepage) )
		Sql_ShowDebug(sql_handle);

	if( login_config.log_login && SQL_ERROR == Sql_Query(sql_handle, "INSERT DELAYED INTO `%s` (`time`,`ip`,`user`,`rcode`,`log`) VALUES (NOW(), '0', 'lserver','100','login server started')", loginlog_db) )
		Sql_ShowDebug(sql_handle);

	sql_ping_init();

	return 0;
}


//-----------------------------------------------------
// close DB
//-----------------------------------------------------
void mmo_db_close(void)
{
	int i, fd;

	//set log.
	if( login_config.log_login && SQL_ERROR == Sql_Query(sql_handle, "INSERT DELAYED INTO `%s`(`time`,`ip`,`user`,`rcode`,`log`) VALUES (NOW(), '0', 'lserver','100', 'login server shutdown')", loginlog_db) )
		Sql_ShowDebug(sql_handle);

	for( i = 0; i < MAX_SERVERS; ++i )
	{
		fd = server[i].fd;
		if( session_isValid(fd) )
		{// Clean only data related to servers we are connected to. [Skotlex]
			if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `sstatus` WHERE `index` = '%d'", i) )
				Sql_ShowDebug(sql_handle);
			do_close(fd);
		}
	}
	Sql_Free(sql_handle);
	sql_handle = NULL;
	ShowStatus("close DB connect....\n");
	if( login_fd > 0 )
		do_close(login_fd);
}

//-----------------------------------------------------
// periodic ip address synchronization
//-----------------------------------------------------
static int sync_ip_addresses(int tid, unsigned int tick, int id, int data)
{
	uint8 buf[2];
	ShowInfo("IP Sync in progress...\n");
	WBUFW(buf,0) = 0x2735;
	charif_sendallwos(-1, buf, 2);
	return 0;
}

//-----------------------------------------------------
// encrypted/unencrypted password check
//-----------------------------------------------------
bool check_encrypted(const char* str1, const char* str2, const char* passwd)
{
	char md5str[64], md5bin[32];

	snprintf(md5str, sizeof(md5str), "%s%s", str1, str2);
	md5str[sizeof(md5str)-1] = '\0';
	MD5_String2binary(md5str, md5bin);

	return (0==memcmp(passwd, md5bin, 16));
}

bool check_password(struct login_session_data* ld, int passwdenc, const char* passwd, const char* refpass)
{	
	if(passwdenc == 0)
	{
		return (0==strcmp(passwd, refpass));
	}
	else if (ld)
	{
		// password mode set to 1 -> (md5key, refpass) enable with <passwordencrypt></passwordencrypt>
		// password mode set to 2 -> (refpass, md5key) enable with <passwordencrypt2></passwordencrypt2>
		
		return ((passwdenc&0x01) && check_encrypted(ld->md5key, refpass, passwd)) ||
		       ((passwdenc&0x02) && check_encrypted(refpass, ld->md5key, passwd));
	}
	return false;
}


//-----------------------------------------------------
// Make new account
//-----------------------------------------------------
int mmo_auth_new(struct mmo_account* account, char sex)
{
	unsigned int tick = gettick();
	char md5buf[32+1];
	SqlStmt* stmt;
	int result = 0;

	//Account Registration Flood Protection by [Kevin]
	if( DIFF_TICK(tick, new_reg_tick) < 0 && num_regs >= allowed_regs )
	{
		ShowNotice("Account registration denied (registration limit exceeded)\n");
		return 3;
	}

	// check if the account doesn't exist already
	stmt = SqlStmt_Malloc(sql_handle);
	if( SQL_SUCCESS != SqlStmt_Prepare(stmt, "SELECT `%s` FROM `%s` WHERE `userid` = ?", login_db_userid, login_db)
	||  SQL_SUCCESS != SqlStmt_BindParam(stmt, 0, SQLDT_STRING, account->userid, strnlen(account->userid, NAME_LENGTH))
	||  SQL_SUCCESS != SqlStmt_Execute(stmt) )
	{
		SqlStmt_ShowDebug(stmt);
		result = 1;// error
	}
	else if( SqlStmt_NumRows(stmt) > 0 )
		result = 1;// incorrect user/pass
	SqlStmt_Free(stmt);
	if( result )
		return result;// error or incorrect user/pass

	// insert new entry into db
	//TODO: error checking
	stmt = SqlStmt_Malloc(sql_handle);
	SqlStmt_Prepare(stmt, "INSERT INTO `%s` (`%s`, `%s`, `sex`, `email`) VALUES (?, ?, '%c', 'a@a.com')", login_db, login_db_userid, login_db_user_pass, TOUPPER(sex));
	SqlStmt_BindParam(stmt, 0, SQLDT_STRING, account->userid, strnlen(account->userid, NAME_LENGTH));
	if( login_config.use_md5_passwds )
	{
		MD5_String(account->passwd, md5buf);
		SqlStmt_BindParam(stmt, 1, SQLDT_STRING, md5buf, 32);
	}
	else
		SqlStmt_BindParam(stmt, 1, SQLDT_STRING, account->passwd, strnlen(account->passwd, NAME_LENGTH));
	SqlStmt_Execute(stmt);
	SqlStmt_Free(stmt);

	ShowInfo("New account: userid='%s' passwd='%s' sex='%c'\n", account->userid, account->passwd, TOUPPER(sex));

	if( DIFF_TICK(tick, new_reg_tick) > 0 )
	{// Update the registration check.
		num_regs = 0;
		new_reg_tick = tick + time_allowed*1000;
	}
	++num_regs;
	return 0;
}


//-----------------------------------------------------
// Check/authentication of a connection
//-----------------------------------------------------
int mmo_auth(struct mmo_account* account, int fd)
{
	time_t ban_until_time;
	char esc_userid[NAME_LENGTH*2+1];// escaped username
	char user_password[256], password[256];
	long connect_until;
	int state;
	size_t len;
	char* data;

	char ip[16];
	uint8* sin_addr = (uint8*)&session[fd]->client_addr;
	sprintf(ip, "%u.%u.%u.%u", sin_addr[3], sin_addr[2], sin_addr[1], sin_addr[0]);

	// DNS Blacklist check
	if( login_config.use_dnsbl )
	{
		char r_ip[16];
		char ip_dnsbl[256];
		char* dnsbl_serv;
		bool matched = false;

		sprintf(r_ip, "%u.%u.%u.%u", sin_addr[0], sin_addr[1], sin_addr[2], sin_addr[3]);

		for( dnsbl_serv = strtok(login_config.dnsbl_servs,","); !matched && dnsbl_serv != NULL; dnsbl_serv = strtok(NULL,",") )
		{
			sprintf(ip_dnsbl, "%s.%s", r_ip, dnsbl_serv);
			if( host2ip(ip_dnsbl) )
				matched = true;
		}

		if( matched )
		{
			ShowInfo("DNSBL: (%s) Blacklisted. User Kicked.\n", r_ip);
			return 3;
		}
	}

	//Client Version check
	if( login_config.check_client_version && account->version != 0 &&
		account->version != login_config.client_version_to_connect )
		return 5;

	len = strnlen(account->userid, NAME_LENGTH);

	// Account creation with _M/_F
	if( login_config.new_account_flag )
	{
		if( len > 2 && strnlen(account->passwd, NAME_LENGTH) > 0 && // valid user and password lengths
			account->passwdenc == 0 && // unencoded password
			account->userid[len-2] == '_' && memchr("FfMm", (unsigned char)account->userid[len-1], 4) ) // _M/_F suffix
		{
			int result;
			char sex;

			len -= 2;
			account->userid[len] = '\0';// nul-terminate the name.
			sex = account->userid[len+1];
			result = mmo_auth_new(account, sex);
			if( result )
				return result;// Failed to make account. [Skotlex].
		}
	}

	// escape username
	Sql_EscapeStringLen(sql_handle, esc_userid, account->userid, len);

	// retrieve login entry for the specified username
	if( SQL_ERROR == Sql_Query(sql_handle,
		"SELECT `%s`,`%s`,`lastlogin`,`sex`,`connect_until`,`ban_until`,`state`,`%s` FROM `%s` WHERE `%s`= %s '%s'",
		login_db_account_id, login_db_user_pass, login_db_level,
		login_db, login_db_userid, (login_config.case_sensitive ? "BINARY" : ""), esc_userid) )
		Sql_ShowDebug(sql_handle);

	if( Sql_NumRows(sql_handle) == 0 ) // no such entry
	{
		ShowNotice("auth failed: no such account '%s'\n", esc_userid);
		Sql_FreeResult(sql_handle);
		return 0;
	}

	Sql_NextRow(sql_handle); //TODO: error checking?

	Sql_GetData(sql_handle, 0, &data, NULL); account->account_id = atoi(data);
	Sql_GetData(sql_handle, 1, &data, &len); safestrncpy(password, data, sizeof(password));
	Sql_GetData(sql_handle, 2, &data, NULL); safestrncpy(account->lastlogin, data, sizeof(account->lastlogin));
	Sql_GetData(sql_handle, 3, &data, NULL); account->sex = (*data == 'S' ? 2 : *data == 'M' ? 1 : 0);
	Sql_GetData(sql_handle, 4, &data, NULL); connect_until = atol(data);
	Sql_GetData(sql_handle, 5, &data, NULL); ban_until_time = atol(data);
	Sql_GetData(sql_handle, 6, &data, NULL); state = atoi(data);
	Sql_GetData(sql_handle, 7, &data, NULL); account->level = atoi(data);
	if( len > sizeof(password) - 1 )
		ShowDebug("mmo_auth: password buffer is too small (len=%u,buflen=%u)\n", len, sizeof(password));
	if( account->level > 99 )
		account->level = 99;

	Sql_FreeResult(sql_handle);

	if( login_config.use_md5_passwds )
		MD5_String(account->passwd, user_password);
	else
		safestrncpy(user_password, account->passwd, NAME_LENGTH);

	if( !check_password(session[fd]->session_data, account->passwdenc, user_password, password) )
	{
		ShowInfo("Invalid password (account: '%s', pass: '%s', received pass: '%s', ip: %s)\n",
			esc_userid, password, (account->passwdenc) ? "[MD5]" : user_password, ip);
		return 1; // 1 = Incorrect Password
	}

	if( connect_until != 0 && connect_until < time(NULL) )
		return 2; // 2 = This ID is expired

	if( ban_until_time != 0 && ban_until_time > time(NULL) )
		return 6; // 6 = Your are Prohibited to log in until %s

	if( state )
	{
		ShowInfo("Connection refused (account: %s, pass: %s, state: %d, ip: %s)\n", account->userid, account->passwd, state, ip);
		return state - 1;
	}

	if( login_config.online_check )
	{
		struct online_login_data* data = idb_get(online_db, account->account_id);
		if( data && data->char_server > -1 )
		{
			//Request char servers to kick this account out. [Skotlex]
			uint8 buf[8];
			ShowNotice("User '%s' is already online - Rejected.\n", account->userid);
			WBUFW(buf,0) = 0x2734;
			WBUFL(buf,2) = account->account_id;
			charif_sendallwos(-1, buf, 6);
			if( data->waiting_disconnect == -1 )
				data->waiting_disconnect = add_timer(gettick()+30000, waiting_disconnect_timer, account->account_id, 0);
			return 3; // Rejected
		}
	}

	account->login_id1 = rand();
	account->login_id2 = rand();

	if( account->sex != 2 && account->account_id < START_ACCOUNT_NUM )
		ShowWarning("Account %s has account id %d! Account IDs must be over %d to work properly!\n", account->userid, account->account_id, START_ACCOUNT_NUM);

	if( SQL_ERROR == Sql_Query(sql_handle, "UPDATE `%s` SET `lastlogin` = NOW(), `logincount`=`logincount`+1, `last_ip`='%s', `ban_until`='0', `state`='0' WHERE `%s` = '%d'",
		login_db, ip, login_db_account_id, account->account_id) )
		Sql_ShowDebug(sql_handle);

	return -1; // account OK
}

static int online_db_setoffline(DBKey key, void* data, va_list ap)
{
	struct online_login_data* p = (struct online_login_data*)data;
	int server = va_arg(ap, int);
	if( server == -1 )
	{
		p->char_server = -1;
		if( p->waiting_disconnect != -1 )
		{
			delete_timer(p->waiting_disconnect, waiting_disconnect_timer);
			p->waiting_disconnect = -1;
		}
	}
	else if( p->char_server == server )
		p->char_server = -2; //Char server disconnected.
	return 0;
}

//--------------------------------
// Packet parsing for char-servers
//--------------------------------
int parse_fromchar(int fd)
{
	int i, id;

	uint32 ipl = session[fd]->client_addr;
	char ip[16];
	ip2str(ipl, ip);

	ARR_FIND( 0, MAX_SERVERS, id, server[id].fd == fd );
	if( id == MAX_SERVERS )
	{// not a char server
		set_eof(fd);
		do_close(fd);
		return 0;
	}

	if( session[fd]->eof )
	{
		ShowStatus("Char-server '%s' has disconnected.\n", server[id].name);
		server[id].fd = -1;
		memset(&server[id], 0, sizeof(struct mmo_char_server));
		online_db->foreach(online_db, online_db_setoffline, id); //Set all chars from this char server to offline.
		if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `sstatus` WHERE `index`='%d'", id) )
			Sql_ShowDebug(sql_handle);
		do_close(fd);
		return 0;
	}

	while( RFIFOREST(fd) >= 2 )
	{
		uint16 command = RFIFOW(fd,0);

		switch( command )
		{

		case 0x2709: // request from map-server via char-server to reload GM accounts
			ShowStatus("Char-server '%s': Request to re-load GM configuration file (ip: %s).\n", server[id].name, ip);
			if( login_config.log_login )
			{
				if( SQL_ERROR == Sql_Query(sql_handle, "INSERT DELAYED INTO `%s`(`time`,`ip`,`user`,`log`) VALUES (NOW(), '%u', '%s', 'GM reload request')", loginlog_db, ipl, server[id].name) )
					Sql_ShowDebug(sql_handle);
			}
			read_gm_account();
			// send GM accounts to all char-servers
			send_GM_accounts(-1);
			RFIFOSKIP(fd,2);
		break;

		case 0x2712: // request from char-server to authenticate an account
			if( RFIFOREST(fd) < 19 )
				return 0;
		{
			int account_id = RFIFOL(fd,2);
			ARR_FIND( 0, AUTH_FIFO_SIZE, i, 
				auth_fifo[i].account_id == RFIFOL(fd,2) &&
				auth_fifo[i].login_id1  == RFIFOL(fd,6) &&
				auth_fifo[i].login_id2  == RFIFOL(fd,10) &&
				auth_fifo[i].sex        == RFIFOB(fd,14) &&
				auth_fifo[i].ip         == ntohl(RFIFOL(fd,15)) &&
				!auth_fifo[i].delflag );

			if( i < AUTH_FIFO_SIZE )
				auth_fifo[i].delflag = 1;

			if( i < AUTH_FIFO_SIZE && account_id > 0 )
			{// send ack 
				uint32 connect_until_time;
				char email[40];

				if( SQL_ERROR == Sql_Query(sql_handle, "SELECT `email`,`connect_until` FROM `%s` WHERE `%s`='%d'", login_db, login_db_account_id, account_id) )
					Sql_ShowDebug(sql_handle);
				if( SQL_SUCCESS == Sql_NextRow(sql_handle) )
				{
					char* data = NULL;
					size_t len = 0;

					Sql_GetData(sql_handle, 0, &data, &len); safestrncpy(email, data, sizeof(email));
					Sql_GetData(sql_handle, 1, &data, NULL); connect_until_time = (uint32)strtoul(data, NULL, 10);
					if( len > sizeof(email) )
						ShowDebug("parse_fromchar:0x2712: email is too long (len=%u,maxlen=%u)\n", len, sizeof(email));

					Sql_FreeResult(sql_handle);
				}
				else
				{
					memset(email, 0, sizeof(email));
					connect_until_time = 0;
				}

				WFIFOHEAD(fd,51);
				WFIFOW(fd,0) = 0x2713;
				WFIFOL(fd,2) = account_id;
				WFIFOB(fd,6) = 0;
				memcpy(WFIFOP(fd, 7), email, 40);
				WFIFOL(fd,47) = connect_until_time;
				WFIFOSET(fd,51);
			}
			else
			{// authentication not found
				ShowStatus("Char-server '%s': authentication of the account %d REFUSED (ip: %s).\n", server[id].name, account_id, ip);
				WFIFOHEAD(fd,51);
				WFIFOW(fd,0) = 0x2713;
				WFIFOL(fd,2) = account_id;
				WFIFOB(fd,6) = 1;
				// It is unnecessary to send email
				// It is unnecessary to send validity date of the account
				WFIFOSET(fd,51);
			}

			RFIFOSKIP(fd,19);
		}
		break;

		case 0x2714:
			if( RFIFOREST(fd) < 6 )
				return 0;

			// how many users on world? (update)
			if( server[id].users != RFIFOL(fd,2) )
			{
				ShowStatus("set users %s : %d\n", server[id].name, RFIFOL(fd,2));

				server[id].users = RFIFOL(fd,2);
				if( SQL_ERROR == Sql_Query(sql_handle, "UPDATE `sstatus` SET `user` = '%d' WHERE `index` = '%d'", server[id].users, id) )
					Sql_ShowDebug(sql_handle);
			}
			RFIFOSKIP(fd,6);
		break;

		case 0x2716: // received an e-mail/limited time request, because a player comes back from a map-server to the char-server
			if( RFIFOREST(fd) < 6 )
				return 0;
		{
			int account_id;
			uint32 connect_until_time = 0;
			char email[40];

			memset(email, 0, sizeof(email));
			account_id = RFIFOL(fd,2);

			if( SQL_ERROR == Sql_Query(sql_handle, "SELECT `email`,`connect_until` FROM `%s` WHERE `%s`='%d'", login_db, login_db_account_id, account_id) )
				Sql_ShowDebug(sql_handle);
			else if( SQL_SUCCESS == Sql_NextRow(sql_handle) )
			{
				char* data;
				size_t len;

				Sql_GetData(sql_handle, 0, &data, &len);
				if( len > sizeof(email) )
				{
#if defined(DEBUG)
					ShowDebug("parse_fromchar:0x2716: email is too long (len=%u,maxlen=%u)\n", len, sizeof(email));
#endif
					len = sizeof(email);
				}
				memcpy(email, data, len);

				Sql_GetData(sql_handle, 1, &data, NULL);
				connect_until_time = (uint32)strtoul(data, NULL, 10);

				Sql_FreeResult(sql_handle);
			}
			//printf("parse_fromchar: E-mail/limited time request from '%s' server (concerned account: %d)\n", server[id].name, RFIFOL(fd,2));
			WFIFOHEAD(fd,50);
			WFIFOW(fd,0) = 0x2717;
			WFIFOL(fd,2) = RFIFOL(fd,2);
			memcpy(WFIFOP(fd, 6), email, 40);
			WFIFOL(fd,46) = connect_until_time;
			WFIFOSET(fd,50);

			RFIFOSKIP(fd,6);
		}
		break;

		case 0x2719: // ping request from charserver
			if( RFIFOREST(fd) < 2 )
				return 0;

			WFIFOHEAD(fd,2);
			WFIFOW(fd,0) = 0x2718;
			WFIFOSET(fd,2);

			RFIFOSKIP(fd,2);
		break;

		case 0x2720: // Request to become a GM (TXT only!)
			if (RFIFOREST(fd) < 4 || RFIFOREST(fd) < RFIFOW(fd,2))
				return 0;
			//oldacc = RFIFOL(fd,4);
			ShowWarning("change GM isn't supported in this login server version.\n");
			ShowError("change GM error 0 %s\n", RFIFOP(fd, 8));

			WFIFOHEAD(fd,10);
			WFIFOW(fd,0) = 0x2721;
			WFIFOL(fd,2) = RFIFOL(fd,4); // oldacc;
			WFIFOL(fd,6) = 0; // newacc;
			WFIFOSET(fd,10);

			RFIFOSKIP(fd, RFIFOW(fd, 2));
			return 0;

		// Map server send information to change an email of an account via char-server
		case 0x2722:	// 0x2722 <account_id>.L <actual_e-mail>.40B <new_e-mail>.40B
			if (RFIFOREST(fd) < 86)
				return 0;
		{
			int account_id;
			char actual_email[40], new_email[40];

			account_id = RFIFOL(fd,2);
			memcpy(actual_email, RFIFOP(fd,6), 40);
			memcpy(new_email, RFIFOP(fd,46), 40);
			if( e_mail_check(actual_email) == 0 )
				ShowWarning("Char-server '%s': Attempt to modify an e-mail on an account (@email GM command), but actual email is invalid (account: %d, ip: %s)\n",
				server[id].name, account_id, ip);
			else if( e_mail_check(new_email) == 0 )
				ShowWarning("Char-server '%s': Attempt to modify an e-mail on an account (@email GM command) with a invalid new e-mail (account: %d, ip: %s)\n",
				server[id].name, account_id, ip);
			else if( strcmpi(new_email, "a@a.com") == 0 )
				ShowWarning("Char-server '%s': Attempt to modify an e-mail on an account (@email GM command) with a default e-mail (account: %d, ip: %s)\n",
				server[id].name, account_id, ip);
			else if( SQL_ERROR == Sql_Query(sql_handle, "SELECT `%s`,`email` FROM `%s` WHERE `%s` = '%d'", login_db_userid, login_db, login_db_account_id, account_id) )
				Sql_ShowDebug(sql_handle);
			else if( SQL_SUCCESS == Sql_NextRow(sql_handle) )
			{
				char* data;
				size_t len;

				Sql_GetData(sql_handle, 1, &data, &len);
				if( len > sizeof(actual_email) )
				{
#if defined(DEBUG)
					ShowDebug("parse_fromchar:0x2722: email is too long (len=%u,maxlen=%u)\n", len, sizeof(actual_email));
#endif
					len = sizeof(actual_email);
				}
				if( strncasecmp(data, actual_email, sizeof(actual_email)) == 0 )
				{
					char esc_user_id[NAME_LENGTH*2+1];
					char esc_new_email[sizeof(new_email)*2+1];

					Sql_GetData(sql_handle, 0, &data, &len);
					if( len > NAME_LENGTH )
					{
#if defined(DEBUG)
						ShowDebug("parse_fromchar:0x2722: userid is too long (len=%u,maxlen=%u)\n", len, NAME_LENGTH);
#endif
						len = NAME_LENGTH;
					}
					Sql_EscapeStringLen(sql_handle, esc_user_id, data, len);
					Sql_EscapeStringLen(sql_handle, esc_new_email, new_email, strnlen(new_email, sizeof(new_email)));

					if( SQL_ERROR == Sql_Query(sql_handle, "UPDATE `%s` SET `email` = '%s' WHERE `%s` = '%d'", login_db, esc_new_email, login_db_account_id, account_id) )
						Sql_ShowDebug(sql_handle);
					ShowInfo("Char-server '%s': Modify an e-mail on an account (@email GM command) (account: %d ('%s'), new e-mail: '%s', ip: %s).\n",
						server[id].name, account_id, esc_user_id, esc_new_email, ip);
				}
				Sql_FreeResult(sql_handle);
			}

			RFIFOSKIP(fd, 86);
		}
		break;

		case 0x2724: // Receiving an account state update request from a map-server (relayed via char-server)
			if (RFIFOREST(fd) < 10)
				return 0;
		{
			int account_id = RFIFOL(fd,2);
			int state = RFIFOL(fd,6);
			if( SQL_ERROR == Sql_Query(sql_handle, "SELECT `state` FROM `%s` WHERE `%s` = '%d'", login_db, login_db_account_id, account_id) )
				Sql_ShowDebug(sql_handle);
			else if( SQL_SUCCESS == Sql_NextRow(sql_handle) )
			{
				char* data;

				Sql_GetData(sql_handle, 0, &data, NULL);
				if( atoi(data) != state && state != 0 )
				{
					uint8 buf[11];
					WBUFW(buf,0) = 0x2731;
					WBUFL(buf,2) = account_id;
					WBUFB(buf,6) = 0; // 0: change of state, 1: ban
					WBUFL(buf,7) = state; // status or final date of a banishment
					charif_sendallwos(-1, buf, 11);
				}
				Sql_FreeResult(sql_handle);
			}

			if( SQL_ERROR == Sql_Query(sql_handle, "UPDATE `%s` SET `state` = '%d' WHERE `%s` = '%d'", login_db, state, login_db_account_id, account_id) )
				Sql_ShowDebug(sql_handle);

			RFIFOSKIP(fd,10);
		}
		break;

		case 0x2725: // Receiving of map-server via char-server a ban request
			if (RFIFOREST(fd) < 18)
				return 0;
		{
			int account_id;
			struct tm *tmtime;
			time_t tmptime = 0;
			time_t timestamp = time(NULL);

			account_id = RFIFOL(fd,2);
			if( SQL_ERROR == Sql_Query(sql_handle, "SELECT `ban_until` FROM `%s` WHERE `%s` = '%d'", login_db, login_db_account_id, account_id) )
				Sql_ShowDebug(sql_handle);
			else if( SQL_SUCCESS == Sql_NextRow(sql_handle) )
			{
				char* data;

				Sql_GetData(sql_handle, 0, &data, NULL);
				tmptime = (time_t)strtoul(data, NULL, 10);
				if( tmptime > time(NULL) )
					timestamp = tmptime;
			}
			tmtime = localtime(&timestamp);
			tmtime->tm_year = tmtime->tm_year + (int16)RFIFOW(fd,6);
			tmtime->tm_mon  = tmtime->tm_mon  + (int16)RFIFOW(fd,8);
			tmtime->tm_mday = tmtime->tm_mday + (int16)RFIFOW(fd,10);
			tmtime->tm_hour = tmtime->tm_hour + (int16)RFIFOW(fd,12);
			tmtime->tm_min  = tmtime->tm_min  + (int16)RFIFOW(fd,14);
			tmtime->tm_sec  = tmtime->tm_sec  + (int16)RFIFOW(fd,16);
			timestamp = mktime(tmtime);
			if( timestamp != (time_t)-1 )
			{
				if( timestamp <= time(NULL) )
					timestamp = 0;
				if( tmptime != timestamp )
				{
					if( timestamp != 0 )
					{
						uint8 buf[11];
						WBUFW(buf,0) = 0x2731;
						WBUFL(buf,2) = account_id;
						WBUFB(buf,6) = 1; // 0: change of statut, 1: ban
						WBUFL(buf,7) = (uint32)timestamp; // status or final date of a banishment
						charif_sendallwos(-1, buf, 11);
					}
					ShowNotice("Account: %d Banned until: %lu\n", account_id, (unsigned long)timestamp);
					if( SQL_ERROR == Sql_Query(sql_handle, "UPDATE `%s` SET `ban_until` = '%lu' WHERE `%s` = '%d'", login_db, (unsigned long)timestamp, login_db_account_id, account_id) )
						Sql_ShowDebug(sql_handle);
				}
			}

			RFIFOSKIP(fd,18);
		}
		break;

		case 0x2727: // Change of sex (sex is reversed)
			if( RFIFOREST(fd) < 6 )
				return 0;
		{
			int account_id;
			int sex;
			uint8 buf[16];

			account_id = RFIFOL(fd,2);
			if( SQL_ERROR == Sql_Query(sql_handle, "SELECT `sex` FROM `%s` WHERE `%s` = '%d'", login_db, login_db_account_id, account_id) )
				Sql_ShowDebug(sql_handle);
			else if( SQL_SUCCESS == Sql_NextRow(sql_handle) )
			{
				char* data;

				Sql_GetData(sql_handle, 0, &data, NULL);
				if( *data == 'M' || *data == 'm' )
					sex = 0; //Change to female
				else
					sex = 1; //Change to male

				if( SQL_ERROR == Sql_Query(sql_handle, "UPDATE `%s` SET `sex` = '%c' WHERE `%s` = '%d'", login_db, (sex ? 'M' : 'F'), login_db_account_id, account_id) )
					Sql_ShowDebug(sql_handle);

				WBUFW(buf,0) = 0x2723;
				WBUFL(buf,2) = account_id;
				WBUFB(buf,6) = sex;
				charif_sendallwos(-1, buf, 7);
			}

			RFIFOSKIP(fd,6);
		}
		break;

		case 0x2728:	// save account_reg2
			if( RFIFOREST(fd) < 4 || RFIFOREST(fd) < RFIFOW(fd,2) )
				return 0;
			if( RFIFOL(fd,4) > 0 )
			{
				SqlStmt* stmt;
				int account_id;
				size_t off;

				account_id = RFIFOL(fd,4);

				//Delete all global account variables....
				if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `type`='1' AND `account_id`='%d';", reg_db, account_id) )
					Sql_ShowDebug(sql_handle);

				//Proceed to insert them....
				stmt = SqlStmt_Malloc(sql_handle);
				if( SQL_ERROR == SqlStmt_Prepare(stmt, "INSERT INTO `%s` (`type`, `account_id`, `str`, `value`) VALUES ( 1 , '%d' , ? , ?);",  reg_db, account_id) )
					SqlStmt_ShowDebug(stmt);
				for( i = 0, off = 13; i < ACCOUNT_REG2_NUM && off < RFIFOW(fd,2); ++i )
				{
					uint8* p;
					size_t len;

					// str
					p = (uint8*)RFIFOP(fd,off);
					len = strlen(p);
					SqlStmt_BindParam(stmt, 0, SQLDT_STRING, p, len);
					off += len + 1;

					// value
					p = (uint8*)RFIFOP(fd,off);
					len = strlen(p);
					SqlStmt_BindParam(stmt, 1, SQLDT_STRING, p, len);
					off += len + 1;

					if( SQL_ERROR == SqlStmt_Execute(stmt) )
						SqlStmt_ShowDebug(stmt);
				}
				SqlStmt_Free(stmt);

				{// Send to char
					//uint8* buf;
					//CREATE(buf, uint8, RFIFOW(fd,2));
					//memcpy(WBUFP(buf,0), RFIFOP(fd,0), RFIFOW(fd,2));
					//WBUFW(buf,0)=0x2729;
					//charif_sendallwos(fd, buf, WBUFW(buf,2));
					//aFree(buf);

					RFIFOW(fd,0) = 0x2729;// reusing read buffer
					charif_sendallwos(fd, RFIFOP(fd,0), RFIFOW(fd,2));
				}
			}
			RFIFOSKIP(fd,RFIFOW(fd,2));
		break;

		case 0x272a:	// Receiving of map-server via char-server an unban request
			if( RFIFOREST(fd) < 6 )
				return 0;
		{
			int account_id;

			account_id = RFIFOL(fd,2);
			if( SQL_ERROR == Sql_Query(sql_handle, "SELECT `ban_until` FROM `%s` WHERE `%s` = '%d'", login_db, login_db_account_id, account_id) )
				Sql_ShowDebug(sql_handle);
			else if( Sql_NumRows(sql_handle) > 0 )
			{// Found a match
				if( SQL_ERROR == Sql_Query(sql_handle, "UPDATE `%s` SET `ban_until` = '0' WHERE `%s` = '%d'", login_db, login_db_account_id, account_id) )
					Sql_ShowDebug(sql_handle);
			}

			RFIFOSKIP(fd,6);
			return 0;
		}

		case 0x272b:    // Set account_id to online [Wizputer]
			if( RFIFOREST(fd) < 6 )
				return 0;
			add_online_user(id, RFIFOL(fd,2));
			RFIFOSKIP(fd,6);
		break;

		case 0x272c:   // Set account_id to offline [Wizputer]
			if( RFIFOREST(fd) < 6 )
				return 0;
			remove_online_user(RFIFOL(fd,2));
			RFIFOSKIP(fd,6);
		break;

		case 0x272d:	// Receive list of all online accounts. [Skotlex]
			if (RFIFOREST(fd) < 4 || RFIFOREST(fd) < RFIFOW(fd,2))
				return 0;
			if( login_config.online_check )
			{
				struct online_login_data *p;
				int aid;
				uint32 i, users;
				online_db->foreach(online_db, online_db_setoffline, id); //Set all chars from this char-server offline first
				users = RFIFOW(fd,4);
				for (i = 0; i < users; i++) {
					aid = RFIFOL(fd,6+i*4);
					p = idb_ensure(online_db, aid, create_online_user);
					p->char_server = id;
					if (p->waiting_disconnect != -1)
					{
						delete_timer(p->waiting_disconnect, waiting_disconnect_timer);
						p->waiting_disconnect = -1;
					}
				}
			}
			RFIFOSKIP(fd,RFIFOW(fd,2));
		break;

		case 0x272e: //Request account_reg2 for a character.
			if (RFIFOREST(fd) < 10)
				return 0;
		{
			int account_id = RFIFOL(fd, 2);
			int char_id = RFIFOL(fd, 6);
			size_t off;

			if( SQL_ERROR == Sql_Query(sql_handle, "SELECT `str`,`value` FROM `%s` WHERE `type`='1' AND `account_id`='%d'", reg_db, account_id) )
				Sql_ShowDebug(sql_handle);

			WFIFOHEAD(fd,10000);
			WFIFOW(fd,0) = 0x2729;
			WFIFOL(fd,4) = account_id;
			WFIFOL(fd,8) = char_id;
			WFIFOB(fd,12) = 1; //Type 1 for Account2 registry
			off = 13;
			while( SQL_SUCCESS == Sql_NextRow(sql_handle) && off < 9000 )
			{
				char* data;
				
				// str
				Sql_GetData(sql_handle, 0, &data, NULL);
				if( *data != '\0' )
				{
					off += sprintf((char*)WFIFOP(fd,off), "%s", data)+1; //We add 1 to consider the '\0' in place.
					
					// value
					Sql_GetData(sql_handle, 1, &data, NULL);
					off += sprintf((char*)WFIFOP(fd,off), "%s", data)+1;
				}
			}
			Sql_FreeResult(sql_handle);
			if( off >= 9000 )
				ShowWarning("Too many account2 registries for AID %d. Some registries were not sent.\n", account_id);
			WFIFOW(fd,2) = (uint16)off;
			WFIFOSET(fd,WFIFOW(fd,2));

			RFIFOSKIP(fd,10);
		}
		break;

		case 0x2736: // WAN IP update from char-server
			if( RFIFOREST(fd) < 6 )
				return 0;
			server[id].ip = ntohl(RFIFOL(fd,2));
			ShowInfo("Updated IP of Server #%d to %d.%d.%d.%d.\n",id, CONVIP(server[id].ip));
			RFIFOSKIP(fd,6);
		break;

		case 0x2737: //Request to set all offline.
			ShowInfo("Setting accounts from char-server %d offline.\n", id);
			online_db->foreach(online_db, online_db_setoffline, id);
			RFIFOSKIP(fd,2);
		break;

		default:
			ShowError("parse_fromchar: Unknown packet 0x%x from a char-server! Disconnecting!\n", command);
			set_eof(fd);
			return 0;
		} // switch
	} // while

	RFIFOSKIP(fd,RFIFOREST(fd));
	return 0;
}

//--------------------------------------------
// Test to know if an IP come from LAN or WAN.
//--------------------------------------------
int lan_subnetcheck(uint32 ip)
{
	int i;
	ARR_FIND( 0, subnet_count, i, (subnet[i].char_ip & subnet[i].mask) == (ip & subnet[i].mask) );
	return ( i < subnet_count ) ? subnet[i].char_ip : 0;
}

int login_ip_ban_check(uint32 ip)
{
	uint8* p = (uint8*)&ip;
	char* data = NULL;
	int matches;

	if( SQL_ERROR == Sql_Query(sql_handle, "SELECT count(*) FROM `ipbanlist` WHERE `list` = '%u.*.*.*' OR `list` = '%u.%u.*.*' OR `list` = '%u.%u.%u.*' OR `list` = '%u.%u.%u.%u'",
		p[3], p[3], p[2], p[3], p[2], p[1], p[3], p[2], p[1], p[0]) )
	{
		Sql_ShowDebug(sql_handle);
		// close connection because we can't verify their connectivity.
		return 1;
	}

	if( SQL_ERROR == Sql_NextRow(sql_handle) )
		return 1;// Shouldn't happen, but just in case...

	Sql_GetData(sql_handle, 0, &data, NULL);
	matches = atoi(data);
	Sql_FreeResult(sql_handle);

	if( matches == 0 )
		return 0;// No ban

	// ip ban ok.
	ShowInfo("Packet from banned ip : %u.%u.%u.%u\n", CONVIP(ip));

	if( login_config.log_login && SQL_ERROR == Sql_Query(sql_handle, "INSERT DELAYED INTO `%s`(`time`,`ip`,`user`,`rcode`,`log`) VALUES (NOW(), '%u', 'unknown','-3', 'ip banned')", loginlog_db, ip) )
		Sql_ShowDebug(sql_handle);
	return 1;
}

//----------------------------------------------------------------------------------------
// Default packet parsing (normal players or administation/char-server connection requests)
//----------------------------------------------------------------------------------------
int parse_login(int fd)
{
	char esc_userid[NAME_LENGTH*2+1];// escaped username
	struct mmo_account account;
	int result, i;
	uint32 ipl = session[fd]->client_addr;
	char ip[16];

	if( session[fd]->eof )
	{
		do_close(fd);
		return 0;
	}

	ip2str(ipl, ip);

	while( RFIFOREST(fd) >= 2 )
	{
		uint16 command = RFIFOW(fd,0);

		switch( command )
		{
		case 0x0200:		// New alive packet: structure: 0x200 <account.userid>.24B. used to verify if client is always alive.
			if (RFIFOREST(fd) < 26)
				return 0;
			RFIFOSKIP(fd,26);
		break;

		case 0x0204:		// New alive packet: structure: 0x204 <encrypted.account.userid>.16B. (new ragexe from 22 june 2004)
			if (RFIFOREST(fd) < 18)
				return 0;
			RFIFOSKIP(fd,18);
		break;

		case 0x0064:		// request client login
		case 0x01dd:		// request client login (encryption mode)
		case 0x0277:		// New login packet (kRO 2006-04-24aSakexe langtype 0)
		case 0x02b0:		// New login packet (kRO 2007-05-14aSakexe langtype 0)
		{
			size_t packet_len = RFIFOREST(fd); // assume no other packet was sent

			// Perform ip-ban check
			if( login_config.ipban && login_ip_ban_check(ipl) )
			{
				ShowStatus("Connection refused: IP isn't authorised (deny/allow, ip: %s).\n", ip);
				WFIFOHEAD(fd,23);
				WFIFOW(fd,0) = 0x6a;
				WFIFOB(fd,2) = 3; // 3 = Rejected from Server
				WFIFOSET(fd,23);
				RFIFOSKIP(fd,packet_len);
				set_eof(fd);
				break;
			}

			if( (command == 0x0064 && packet_len < 55)
			||  (command == 0x01dd && packet_len < 47)
			||  (command == 0x0277 && packet_len < 84)
			||  (command == 0x02b0 && packet_len < 85) )
				return 0;

			// S 0064 <version>.l <account name>.24B <password>.24B <version2>.B
			// S 01dd <version>.l <account name>.24B <md5 binary>.16B <version2>.B
			// S 0277 <version>.l <account name>.24B <password>.24B <junk?>.29B <version2>.B
			// S 02b0 <version>.l <account name>.24B <password>.24B <junk?>.30B <version2>.B

			memset(&account, 0, sizeof(account));
			account.version = RFIFOL(fd,2);
			if( !account.version )
				account.version = 1; //Force some version...
			safestrncpy(account.userid, (char*)RFIFOP(fd,6), NAME_LENGTH);//## does it have to be nul-terminated?
			safestrncpy(account.passwd, (char*)RFIFOP(fd,30), NAME_LENGTH);//## does it have to be nul-terminated?
			account.passwdenc = (command == 0x01dd) ? PASSWORDENC : 0;
			Sql_EscapeStringLen(sql_handle, esc_userid, account.userid, strlen(account.userid));

			result = mmo_auth(&account, fd);
			if( result == -1 )
			{	// auth success
				if( login_config.min_level_to_connect > account.level )
				{
					ShowStatus("Connection refused: the minimum GM level for connection is %d (account: %s, GM level: %d, ip: %s).\n", login_config.min_level_to_connect, account.userid, account.level, ip);
					WFIFOHEAD(fd,3);
					WFIFOW(fd,0) = 0x81;
					WFIFOB(fd,2) = 1; // 01 = Server closed
					WFIFOSET(fd,3);
				}
				else
				{
					uint8 server_num, n;
					uint32 subnet_char_ip;

					server_num = 0;
					for( i = 0; i < MAX_SERVERS; ++i )
						if( session_isValid(server[i].fd) )
							server_num++;

					if( server_num > 0 )
					{// if at least 1 char-server
						if( login_config.log_login && SQL_ERROR == Sql_Query(sql_handle, "INSERT DELAYED INTO `%s`(`time`,`ip`,`user`,`rcode`,`log`) VALUES (NOW(), '%u', '%s','100', 'login ok')", loginlog_db, ipl, esc_userid) )
							Sql_ShowDebug(sql_handle);
						if( account.level )
							ShowStatus("Connection of the GM (level:%d) account '%s' accepted.\n", account.level, account.userid);
						else
							ShowStatus("Connection of the account '%s' accepted.\n", account.userid);

						WFIFOHEAD(fd,47+32*server_num);
						WFIFOW(fd,0) = 0x69;
						WFIFOW(fd,2) = 47+32*server_num;
						WFIFOL(fd,4) = account.login_id1;
						WFIFOL(fd,8) = account.account_id;
						WFIFOL(fd,12) = account.login_id2;
						WFIFOL(fd,16) = 0; // in old version, that was for ip (not more used)
						//memcpy(WFIFOP(fd,20), account.lastlogin, 24); // in old version, that was for name (not more used)
						WFIFOW(fd,44) = 0; // unknown
						WFIFOB(fd,46) = account.sex;
						for( i = 0, n = 0; i < MAX_SERVERS; ++i )
						{
							if( !session_isValid(server[i].fd) )
								continue;

							subnet_char_ip = lan_subnetcheck(ipl); // Advanced subnet check [LuzZza]
							WFIFOL(fd,47+n*32) = htonl((subnet_char_ip) ? subnet_char_ip : server[i].ip);
							WFIFOW(fd,47+n*32+4) = ntows(htons(server[i].port)); // [!] LE byte order here [!]
							memcpy(WFIFOP(fd,47+n*32+6), server[i].name, 20);
							WFIFOW(fd,47+n*32+26) = server[i].users;
							WFIFOW(fd,47+n*32+28) = server[i].maintenance;
							WFIFOW(fd,47+n*32+30) = server[i].new_;
							n++;
						}
						WFIFOSET(fd,47+32*server_num);

						if (auth_fifo_pos >= AUTH_FIFO_SIZE)
							auth_fifo_pos = 0;
						auth_fifo[auth_fifo_pos].account_id = account.account_id;
						auth_fifo[auth_fifo_pos].login_id1 = account.login_id1;
						auth_fifo[auth_fifo_pos].login_id2 = account.login_id2;
						auth_fifo[auth_fifo_pos].sex = account.sex;
						auth_fifo[auth_fifo_pos].delflag = 0;
						auth_fifo[auth_fifo_pos].ip = session[fd]->client_addr;
						auth_fifo_pos++;
					}
					else
					{// if no char-server, don't send void list of servers, just disconnect the player with proper message
						ShowStatus("Connection refused: there is no char-server online (account: %s, ip: %s).\n", account.userid, ip);
						WFIFOHEAD(fd,3);
						WFIFOW(fd,0) = 0x81;
						WFIFOB(fd,2) = 1; // 01 = Server closed
						WFIFOSET(fd,3);
					}
				}
			}
			else
			{	// auth failed
				if (login_config.log_login)
				{
					const char* error;
					switch( result ) {
					case   0: error = "Unregistered ID."; break; // 0 = Unregistered ID
					case   1: error = "Incorrect Password."; break; // 1 = Incorrect Password
					case   2: error = "Account Expired."; break; // 2 = This ID is expired
					case   3: error = "Rejected from server."; break; // 3 = Rejected from Server
					case   4: error = "Blocked by GM."; break; // 4 = You have been blocked by the GM Team
					case   5: error = "Not latest game EXE."; break; // 5 = Your Game's EXE file is not the latest version
					case   6: error = "Banned."; break; // 6 = Your are Prohibited to log in until %s
					case   7: error = "Server Over-population."; break; // 7 = Server is jammed due to over populated
					case   8: error = "Account limit from company"; break; // 8 = No more accounts may be connected from this company
					case   9: error = "Ban by DBA"; break; // 9 = MSI_REFUSE_BAN_BY_DBA
					case  10: error = "Email not confirmed"; break; // 10 = MSI_REFUSE_EMAIL_NOT_CONFIRMED
					case  11: error = "Ban by GM"; break; // 11 = MSI_REFUSE_BAN_BY_GM
					case  12: error = "Working in DB"; break; // 12 = MSI_REFUSE_TEMP_BAN_FOR_DBWORK
					case  13: error = "Self Lock"; break; // 13 = MSI_REFUSE_SELF_LOCK
					case  14: error = "Not Permitted Group"; break; // 14 = MSI_REFUSE_NOT_PERMITTED_GROUP
					case  15: error = "Not Permitted Group"; break; // 15 = MSI_REFUSE_NOT_PERMITTED_GROUP
					case  99: error = "Account gone."; break; // 99 = This ID has been totally erased
					case 100: error = "Login info remains."; break; // 100 = Login information remains at %s
					case 101: error = "Hacking investigation."; break; // 101 = Account has been locked for a hacking investigation. Please contact the GM Team for more information
					case 102: error = "Bug investigation."; break; // 102 = This account has been temporarily prohibited from login due to a bug-related investigation
					case 103: error = "Deleting char."; break; // 103 = This character is being deleted. Login is temporarily unavailable for the time being
					case 104: error = "Deleting spouse char."; break; // 104 = This character is being deleted. Login is temporarily unavailable for the time being
					default : error = "Unknown Error."; break;
					}

					if( SQL_ERROR == Sql_Query(sql_handle, "INSERT DELAYED INTO `%s`(`time`,`ip`,`user`,`rcode`,`log`) VALUES (NOW(), '%u', '%s', '%d','login failed : %s')", loginlog_db, ipl, esc_userid, result, error) )
						Sql_ShowDebug(sql_handle);
				}

				if( result == 1 && login_config.dynamic_pass_failure_ban && login_config.log_login ) // failed password
				{
					unsigned long failures = 0;
					if( SQL_ERROR == Sql_Query(sql_handle, "SELECT count(*) FROM `%s` WHERE `ip` = '%u' AND `rcode` = '1' AND `time` > NOW() - INTERVAL %d MINUTE",
						loginlog_db, ipl, login_config.dynamic_pass_failure_ban_interval) )// how many times failed account? in one ip.
						Sql_ShowDebug(sql_handle);

					//check query result
					if( SQL_SUCCESS == Sql_NextRow(sql_handle) )
					{
						char* data;
						Sql_GetData(sql_handle, 0, &data, NULL);
						failures = strtoul(data, NULL, 10);
						Sql_FreeResult(sql_handle);
					}
					if( failures >= login_config.dynamic_pass_failure_ban_limit )
					{
						uint8* p = (uint8*)&ipl;
						if( SQL_ERROR == Sql_Query(sql_handle, "INSERT INTO `ipbanlist`(`list`,`btime`,`rtime`,`reason`) VALUES ('%u.%u.%u.*', NOW() , NOW() +  INTERVAL %d MINUTE ,'Password error ban: %s')", p[3], p[2], p[1], login_config.dynamic_pass_failure_ban_duration, esc_userid) )
							Sql_ShowDebug(sql_handle);
					}
				}

				WFIFOHEAD(fd,23);
				WFIFOW(fd,0) = 0x6a;
				WFIFOB(fd,2) = (uint8)result;
				if( result != 6 )
					memset(WFIFOP(fd,3), '\0', 20);
				else
				{// 6 = Your are Prohibited to log in until %s
					if( SQL_ERROR == Sql_Query(sql_handle, "SELECT `ban_until` FROM `%s` WHERE `%s` = %s '%s'", login_db, login_db_userid, (login_config.case_sensitive ? "BINARY" : ""), esc_userid) )
						Sql_ShowDebug(sql_handle);
					else if( SQL_SUCCESS == Sql_NextRow(sql_handle) )
					{
						char* data;
						time_t ban_until_time;

						Sql_GetData(sql_handle, 0, &data, NULL);
						ban_until_time = (time_t)strtoul(data, NULL, 10);
						Sql_FreeResult(sql_handle);

						strftime((char*)WFIFOP(fd,3), 20, login_config.date_format, localtime(&ban_until_time));
					}
				}
				WFIFOSET(fd,23);
			}

			RFIFOSKIP(fd,packet_len);
		}
		break;

		case 0x01db:	// Sending request of the coding key
		{
			struct login_session_data* ld;
			if( session[fd]->session_data )
			{
				ShowWarning("login: abnormal request of MD5 key (already opened session).\n");
				set_eof(fd);
				return 0;
			}

			CREATE(ld, struct login_session_data, 1);
			session[fd]->session_data = ld;

			// Creation of the coding key
			memset(ld->md5key, '\0', sizeof(ld->md5key));
			ld->md5keylen = (uint16)(12 + rand() % 4);
			for( i = 0; i < ld->md5keylen; ++i )
				ld->md5key[i] = (char)(1 + rand() % 255);

			WFIFOHEAD(fd,4 + ld->md5keylen);
			WFIFOW(fd,0) = 0x01dc;
			WFIFOW(fd,2) = 4 + ld->md5keylen;
			memcpy(WFIFOP(fd,4), ld->md5key, ld->md5keylen);
			WFIFOSET(fd,WFIFOW(fd,2));

			RFIFOSKIP(fd,2);
		}
		break;

		case 0x2710:	// Connection request of a char-server
			if (RFIFOREST(fd) < 86)
				return 0;
		{
			char esc_server_name[20*2+1];
			char* server_name;
			uint32 server_ip;
			uint16 server_port;

			memset(&account, 0, sizeof(account));
			safestrncpy(account.userid, (char*)RFIFOP(fd,2), NAME_LENGTH);
			safestrncpy(account.passwd, (char*)RFIFOP(fd,26), NAME_LENGTH);
			account.passwdenc = 0;
			server_ip = ntohl(RFIFOL(fd,54));
			server_port = ntohs(RFIFOW(fd,58));
			server_name = (char*)RFIFOP(fd,60);

			Sql_EscapeStringLen(sql_handle, esc_server_name, server_name, strnlen(server_name, 20));
			Sql_EscapeStringLen(sql_handle, esc_userid, account.userid, strnlen(account.userid, NAME_LENGTH));

			ShowInfo("Connection request of the char-server '%s' @ %d.%d.%d.%d:%d (account: '%s', pass: '%s', ip: '%s')\n", esc_server_name, CONVIP(server_ip), server_port, account.userid, account.passwd, ip);

			if( login_config.log_login && SQL_ERROR == Sql_Query(sql_handle, "INSERT DELAYED INTO `%s`(`time`,`ip`,`user`,`rcode`,`log`) VALUES (NOW(), '%u', '%s@%s','100', 'charserver - %s@%u.%u.%u.%u:%d')",
				loginlog_db, ipl, esc_userid, esc_server_name, esc_server_name, CONVIP(server_ip), server_port) )
				Sql_ShowDebug(sql_handle);

			result = mmo_auth(&account, fd);
			if( result == -1 && account.sex == 2 && account.account_id < MAX_SERVERS && server[account.account_id].fd == -1 )
			{
				ShowStatus("Connection of the char-server '%s' accepted.\n", esc_server_name);
				memset(&server[account.account_id], 0, sizeof(struct mmo_char_server));
				server[account.account_id].ip = ntohl(RFIFOL(fd,54));
				server[account.account_id].port = ntohs(RFIFOW(fd,58));
				safestrncpy(server[account.account_id].name, server_name, sizeof(server[account.account_id].name));
				server[account.account_id].users = 0;
				server[account.account_id].maintenance = RFIFOW(fd,82);
				server[account.account_id].new_ = RFIFOW(fd,84);
				server[account.account_id].fd = fd;

				WFIFOHEAD(fd,3);
				WFIFOW(fd,0) = 0x2711;
				WFIFOB(fd,2) = 0;
				WFIFOSET(fd,3);

				session[fd]->func_parse = parse_fromchar;
				session[fd]->client_addr = 0;
				realloc_fifo(fd, FIFOSIZE_SERVERLINK, FIFOSIZE_SERVERLINK);

				send_GM_accounts(fd); // send GM account to char-server

				if( SQL_ERROR == Sql_Query(sql_handle, "REPLACE INTO `sstatus`(`index`,`name`,`user`) VALUES ( '%d', '%s', '%d')", account.account_id, esc_server_name, 0) )
					Sql_ShowDebug(sql_handle);
			}
			else
			{
				ShowNotice("Connection of the char-server '%s' REFUSED.\n", esc_server_name);
				WFIFOHEAD(fd,3);
				WFIFOW(fd,0) = 0x2711;
				WFIFOB(fd,2) = 3;
				WFIFOSET(fd,3);
			}
		}
			RFIFOSKIP(fd,86);
			return 0;

		case 0x7530:	// Server version information request
			ShowStatus("Sending server version information to ip: %s\n", ip);
			WFIFOHEAD(fd,10);
			WFIFOW(fd,0) = 0x7531;
			WFIFOB(fd,2) = ATHENA_MAJOR_VERSION;
			WFIFOB(fd,3) = ATHENA_MINOR_VERSION;
			WFIFOB(fd,4) = ATHENA_REVISION;
			WFIFOB(fd,5) = ATHENA_RELEASE_FLAG;
			WFIFOB(fd,6) = ATHENA_OFFICIAL_FLAG;
			WFIFOB(fd,7) = ATHENA_SERVER_LOGIN;
			WFIFOW(fd,8) = ATHENA_MOD_VERSION;
			WFIFOSET(fd,10);

			RFIFOSKIP(fd,2);
		break;

		case 0x7532:	// Request to end connection
			ShowStatus("End of connection (ip: %s)\n", ip);
			set_eof(fd);
		break;

		default:
			ShowNotice("Abnormal end of connection (ip: %s): Unknown packet 0x%x\n", ip, command);
			set_eof(fd);
			return 0;
		}
	}

	RFIFOSKIP(fd,RFIFOREST(fd));
	return 0;
}

//-----------------------
// Console Command Parser [Wizputer]
//-----------------------
int parse_console(char* buf)
{
	char command[256];

	memset(command, 0, sizeof(command));

	sscanf(buf, "%[^\n]", command);

	ShowInfo("Console command :%s", command);

	if( strcmpi("shutdown", command) == 0 ||
		strcmpi("exit", command) == 0 ||
		strcmpi("quit", command) == 0 ||
		strcmpi("end", command) == 0 )
		runflag = 0;
	else
	if( strcmpi("alive", command) == 0 ||
		strcmpi("status", command) == 0 )
		ShowInfo(CL_CYAN"Console: "CL_BOLD"I'm Alive."CL_RESET"\n");
	else
	if( strcmpi("help", command) == 0 ) {
		printf(CL_BOLD"Help of commands:"CL_RESET"\n");
		printf("  To shutdown the server:\n");
		printf("  'shutdown|exit|quit|end'\n");
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

//----------------------------------
// Reading Lan Support configuration
//----------------------------------
int login_lan_config_read(const char *lancfgName)
{
	FILE *fp;
	int line_num = 0;
	char line[1024], w1[64], w2[64], w3[64], w4[64];

	if((fp = fopen(lancfgName, "r")) == NULL) {
		ShowWarning("LAN Support configuration file is not found: %s\n", lancfgName);
		return 1;
	}

	ShowInfo("Reading the configuration file %s...\n", lancfgName);

	while(fgets(line, sizeof(line), fp))
	{
		line_num++;
		if ((line[0] == '/' && line[1] == '/') || line[0] == '\n' || line[1] == '\n')
			continue;

		if(sscanf(line,"%[^:]: %[^:]:%[^:]:%[^\r\n]", w1, w2, w3, w4) != 4)
		{
			ShowWarning("Error syntax of configuration file %s in line %d.\n", lancfgName, line_num);
			continue;
		}

		remove_control_chars(w1);
		remove_control_chars(w2);
		remove_control_chars(w3);
		remove_control_chars(w4);

		if( strcmpi(w1, "subnet") == 0 )
		{
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

	ShowStatus("Read information about %d subnetworks.\n", subnet_count);

	fclose(fp);
	return 0;
}

//-----------------------------------------------------
// clear expired ip bans
//-----------------------------------------------------
int ip_ban_flush(int tid, unsigned int tick, int id, int data)
{
	if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `ipbanlist` WHERE `rtime` <= NOW()") )
		Sql_ShowDebug(sql_handle);

	return 0;
}

//-----------------------------------
// Reading main configuration file
//-----------------------------------
int login_config_read(const char* cfgName)
{
	char line[1024], w1[1024], w2[1024];
	FILE* fp = fopen(cfgName, "r");
	if (fp == NULL) {
		ShowError("Configuration file (%s) not found.\n", cfgName);
		return 1;
	}
	ShowInfo("Reading configuration file %s...\n", cfgName);
	while(fgets(line, sizeof(line), fp))
	{
		if (line[0] == '/' && line[1] == '/')
			continue;

		if (sscanf(line, "%[^:]: %[^\r\n]", w1, w2) < 2)
			continue;

		remove_control_chars(w1);
		remove_control_chars(w2);

		if(!strcmpi(w1,"timestamp_format"))
			strncpy(timestamp_format, w2, 20);
		else if(!strcmpi(w1,"stdout_with_ansisequence"))
			stdout_with_ansisequence = config_switch(w2);
		else if(!strcmpi(w1,"console_silent")) {
			ShowInfo("Console Silent Setting: %d\n", atoi(w2));
			msg_silent = atoi(w2);
		}
		else if( !strcmpi(w1, "bind_ip") ) {
			char ip_str[16];
			login_config.login_ip = host2ip(w2);
			if( login_config.login_ip )
				ShowStatus("Login server binding IP address : %s -> %s\n", w2, ip2str(login_config.login_ip, ip_str));
		}
		else if( !strcmpi(w1, "login_port") ) {
			login_config.login_port = (uint16)atoi(w2);
			ShowStatus("set login_port : %s\n",w2);
		}
		else if(!strcmpi(w1, "log_login"))
			login_config.log_login = (bool)config_switch(w2);

		else if(!strcmpi(w1, "ipban"))
			login_config.ipban = (bool)config_switch(w2);
		else if(!strcmpi(w1, "dynamic_pass_failure_ban"))
			login_config.dynamic_pass_failure_ban = (bool)config_switch(w2);
		else if(!strcmpi(w1, "dynamic_pass_failure_ban_interval"))
			login_config.dynamic_pass_failure_ban_interval = atoi(w2);
		else if(!strcmpi(w1, "dynamic_pass_failure_ban_limit"))
			login_config.dynamic_pass_failure_ban_limit = atoi(w2);
		else if(!strcmpi(w1, "dynamic_pass_failure_ban_duration"))
			login_config.dynamic_pass_failure_ban_duration = atoi(w2);

		else if(!strcmpi(w1, "new_account"))
			login_config.new_account_flag = (bool)config_switch(w2);
		else if(!strcmpi(w1, "check_client_version"))
			login_config.check_client_version = (bool)config_switch(w2);
		else if(!strcmpi(w1, "client_version_to_connect"))
			login_config.client_version_to_connect = atoi(w2);
		else if(!strcmpi(w1, "use_MD5_passwords"))
			login_config.use_md5_passwds = (bool)config_switch(w2);
		else if(!strcmpi(w1, "min_level_to_connect"))
			login_config.min_level_to_connect = atoi(w2);
		else if(!strcmpi(w1, "date_format"))
			safestrncpy(login_config.date_format, w2, sizeof(login_config.date_format));
		else if(!strcmpi(w1, "console"))
			login_config.console = (bool)config_switch(w2);
		else if(!strcmpi(w1, "case_sensitive"))
			login_config.case_sensitive = (bool)config_switch(w2);
		else if(!strcmpi(w1, "allowed_regs")) //account flood protection system
			allowed_regs = atoi(w2);
		else if(!strcmpi(w1, "time_allowed"))
			time_allowed = atoi(w2);
		else if(!strcmpi(w1, "online_check"))
			login_config.online_check = (bool)config_switch(w2);
		else if(!strcmpi(w1, "use_dnsbl"))
			login_config.use_dnsbl = (bool)config_switch(w2);
		else if(!strcmpi(w1, "dnsbl_servers"))
			safestrncpy(login_config.dnsbl_servs, w2, sizeof(login_config.dnsbl_servs));
		else if(!strcmpi(w1, "ip_sync_interval"))
			login_config.ip_sync_interval = (unsigned int)1000*60*atoi(w2); //w2 comes in minutes.
		else if(!strcmpi(w1, "import"))
			login_config_read(w2);
	}
	fclose(fp);
	ShowInfo("Finished reading %s.\n", cfgName);
	return 0;
}

void sql_config_read(const char* cfgName)
{
	char line[1024], w1[1024], w2[1024];
	FILE* fp = fopen(cfgName, "r");
	if(fp == NULL) {
		ShowError("file not found: %s\n", cfgName);
		return;
	}
	ShowInfo("reading configuration file %s...\n", cfgName);
	while(fgets(line, sizeof(line), fp))
	{
		if (line[0] == '/' && line[1] == '/')
			continue;
		if (sscanf(line, "%[^:]: %[^\r\n]", w1, w2) < 2)
			continue;

		if (!strcmpi(w1, "gm_read_method"))
			login_config.login_gm_read = (atoi(w2) == 0);
		else if (!strcmpi(w1, "login_db"))
			strcpy(login_db, w2);
		else if (!strcmpi(w1, "login_server_ip"))
			strcpy(login_server_ip, w2);
		else if (!strcmpi(w1, "login_server_port"))
			login_server_port = (uint16)atoi(w2);
		else if (!strcmpi(w1, "login_server_id"))
			strcpy(login_server_id, w2);
		else if (!strcmpi(w1, "login_server_pw"))
			strcpy(login_server_pw, w2);
		else if (!strcmpi(w1, "login_server_db"))
			strcpy(login_server_db, w2);
		else if (!strcmpi(w1, "default_codepage"))
			strcpy(default_codepage, w2);
		else if (!strcmpi(w1, "login_db_account_id"))
			strcpy(login_db_account_id, w2);
		else if (!strcmpi(w1, "login_db_userid"))
			strcpy(login_db_userid, w2);
		else if (!strcmpi(w1, "login_db_user_pass"))
			strcpy(login_db_user_pass, w2);
		else if (!strcmpi(w1, "login_db_level"))
			strcpy(login_db_level, w2);
		else if (!strcmpi(w1, "loginlog_db"))
			strcpy(loginlog_db, w2);
		else if (!strcmpi(w1, "reg_db"))
			strcpy(reg_db, w2);
		else if (!strcmpi(w1, "import"))
			sql_config_read(w2);
	}
	fclose(fp);
	ShowInfo("Done reading %s.\n", cfgName);
}

void login_set_defaults()
{
	login_config.login_ip = INADDR_ANY;
	login_config.login_port = 6900;
	login_config.ip_sync_interval = 0;
	login_config.log_login = true;
	safestrncpy(login_config.date_format, "%Y-%m-%d %H:%M:%S", sizeof(login_config.date_format));
	login_config.console = false;
	login_config.new_account_flag = true;
	login_config.case_sensitive = true;
	login_config.use_md5_passwds = false;
	login_config.login_gm_read = true;
	login_config.min_level_to_connect = 0;
	login_config.online_check = true;
	login_config.check_client_version = false;
	login_config.client_version_to_connect = 20;

	login_config.ipban = true;
	login_config.dynamic_pass_failure_ban = true;
	login_config.dynamic_pass_failure_ban_interval = 5;
	login_config.dynamic_pass_failure_ban_limit = 7;
	login_config.dynamic_pass_failure_ban_duration = 5;
	login_config.use_dnsbl = false;
	safestrncpy(login_config.dnsbl_servs, "", sizeof(login_config.dnsbl_servs));
}

//--------------------------------------
// Function called at exit of the server
//--------------------------------------
void do_final(void)
{
	//sync account when terminating.
	//but no need when you using DBMS (mysql)
	ShowStatus("Terminating...\n");
	mmo_db_close();
	online_db->destroy(online_db, NULL);
	if (gm_account_db)
		aFree(gm_account_db);
}

//------------------------------
// Function called when the server
// has received a crash signal.
//------------------------------
void do_abort(void)
{
}

void set_server_type(void)
{
	SERVER_TYPE = ATHENA_SERVER_LOGIN;
}

//------------------------------
// Login server initialization
//------------------------------
int do_init(int argc, char** argv)
{
	int i;

	login_set_defaults();

	// read login-server configuration
	login_config_read((argc > 1) ? argv[1] : LOGIN_CONF_NAME);
	sql_config_read(SQL_CONF_NAME);
	login_lan_config_read((argc > 2) ? argv[2] : LAN_CONF_NAME);

	srand((unsigned int)time(NULL));

	for( i = 0; i < AUTH_FIFO_SIZE; i++ )
		auth_fifo[i].delflag = 1;

	for( i = 0; i < MAX_SERVERS; i++ )
		server[i].fd = -1;

	// Online user database init
	online_db = idb_alloc(DB_OPT_RELEASE_DATA);
	add_timer_func_list(waiting_disconnect_timer, "waiting_disconnect_timer");

	// Auth init
	mmo_auth_sqldb_init();

	// Read account information.
	if(login_config.login_gm_read)
		read_gm_account();

	// set default parser as parse_login function
	set_defaultparse(parse_login);

	// ban deleter timer
	add_timer_func_list(ip_ban_flush, "ip_ban_flush");
	add_timer_interval(gettick()+10, ip_ban_flush, 0, 0, 60*1000);

	// every 10 minutes cleanup online account db.
	add_timer_func_list(online_data_cleanup, "online_data_cleanup");
	add_timer_interval(gettick() + 600*1000, online_data_cleanup, 0, 0, 600*1000);

	// add timer to detect ip address change and perform update
	if (login_config.ip_sync_interval) {
		add_timer_func_list(sync_ip_addresses, "sync_ip_addresses");
		add_timer_interval(gettick() + login_config.ip_sync_interval, sync_ip_addresses, 0, 0, login_config.ip_sync_interval);
	}

	if( login_config.console )
	{
		//##TODO invoke a CONSOLE_START plugin event
	}

	new_reg_tick = gettick();

	// server port open & binding
	login_fd = make_listen_bind(login_config.login_ip, login_config.login_port);

	ShowStatus("The login-server is "CL_GREEN"ready"CL_RESET" (Server is listening on the port %u).\n\n", login_config.login_port);

	return 0;
}
