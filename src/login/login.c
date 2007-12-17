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
#include "../common/lock.h"
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
unsigned int GM_max = 0;

//Account registration flood protection [Kevin]
int allowed_regs = 1;
int time_allowed = 10; //in seconds
unsigned int new_reg_tick = 0;
int num_regs = 0;

int account_id_count = START_ACCOUNT_NUM;

char account_filename[1024] = "save/account.txt";
char GM_account_filename[1024] = "conf/GM_account.txt";
FILE *log_fp = NULL;
char login_log_unknown_packets_filename[1024] = "log/login_unknown_packets.log";
int save_unknown_packets = 0;
long creation_time_GM_account_file; // tracks the last-changed timestamp of the gm accounts file
int gm_account_filename_check_timer = 15; // Timer to check if GM_account file has been changed and reload GM account automaticaly (in seconds; default: 15)


enum {
	ACO_DENY_ALLOW = 0,
	ACO_ALLOW_DENY,
	ACO_MUTUAL_FAILTURE,
	ACO_STRSIZE = 128,
};

int access_order = ACO_DENY_ALLOW;
int access_allownum = 0;
int access_denynum = 0;
char *access_allow = NULL;
char *access_deny = NULL;

int access_ladmin_allownum = 0;
char *access_ladmin_allow = NULL;

int start_limited_time = -1; // Starting additional sec from now for the limited time at creation of accounts (-1: unlimited time, 0 or more: additional sec from now)

struct login_session_data {
	uint16 md5keylen;
	char md5key[20];
};

// auth information of incoming clients
struct _auth_fifo auth_fifo[AUTH_FIFO_SIZE];
int auth_fifo_pos = 0;

// account database
struct auth_data* auth_dat = NULL;
uint32 auth_num = 0, auth_max = 0;

// define the number of times that some players must authentify them before to save account file.
// it's just about normal authentication. If an account is created or modified, save is immediatly done.
// An authentication just change last connected IP and date. It already save in log file.
// set minimum auth change before save:
#define AUTH_BEFORE_SAVE_FILE 10
// set divider of auth_num to found number of change before save
#define AUTH_SAVE_FILE_DIVIDER 50
int auth_before_save_file = 0; // Counter. First save when 1st char-server do connection.

bool admin_state = false;
char admin_pass[24] = "";
char gm_pass[64] = "";
int level_new_gm = 60;

int parse_admin(int fd);

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

//----------------------------------------------------------------------
// Determine if an account (id) is a GM account
// and returns its level (or 0 if it isn't a GM account or if not found)
//----------------------------------------------------------------------
int isGM(int account_id)
{
	unsigned int i;
	ARR_FIND( 0, GM_num, i, gm_account_db[i].account_id == account_id );
	return ( i < GM_num ) ? gm_account_db[i].level : 0;
}

//----------------------------------------------------------------------
// Adds a new GM using acc id and level
//----------------------------------------------------------------------
void addGM(int account_id, int level)
{
	unsigned int i;

	ARR_FIND( 0, auth_num, i, auth_dat[i].account_id == account_id );
	if( i == auth_num )
		return; // no such account

	ARR_FIND( 0, GM_num, i, gm_account_db[i].account_id == account_id );
	if( i < GM_num )
	{
		if (gm_account_db[i].level == level)
			ShowWarning("addGM: GM account %d defined twice (same level: %d).\n", account_id, level);
		else {
			ShowWarning("addGM: GM account %d defined twice (levels: %d and %d).\n", account_id, gm_account_db[i].level, level);
			gm_account_db[i].level = level;
		}
		return; // entry already present
	}

	// new account
	if (GM_num >= GM_max) {
		GM_max += 256;
		RECREATE(gm_account_db, struct gm_account, GM_max);
	}
	gm_account_db[GM_num].account_id = account_id;
	gm_account_db[GM_num].level = level;
	GM_num++;
	if (GM_num >= 4000)
		ShowWarning("4000 GM accounts found. Next GM accounts are not read.\n");
}

//-------------------------------------------------------
// Reading function of GM accounts file (and their level)
//-------------------------------------------------------
int read_gm_account(void)
{
	char line[512];
	FILE *fp;
	int account_id, level;
	int line_counter;
	struct stat file_stat;
	int start_range = 0, end_range = 0, is_range = 0, current_id = 0;

	if(gm_account_db) aFree(gm_account_db);
	CREATE(gm_account_db, struct gm_account, 1);
	GM_num = 0;

	// get last modify time/date
	if (stat(GM_account_filename, &file_stat))
		creation_time_GM_account_file = 0; // error
	else
		creation_time_GM_account_file = (long)file_stat.st_mtime;

	if ((fp = fopen(GM_account_filename, "r")) == NULL) {
		ShowError("read_gm_account: GM accounts file [%s] not found.\n", GM_account_filename);
		return 1;
	}

	line_counter = 0;
	// limited to 4000, because we send information to char-servers (more than 4000 GM accounts???)
	// int (id) + int (level) = 8 bytes * 4000 = 32k (limit of packets in windows)
	while(fgets(line, sizeof(line), fp) && GM_num < 4000)
	{
		line_counter++;
		if ((line[0] == '/' && line[1] == '/') || line[0] == '\0' || line[0] == '\n' || line[0] == '\r')
			continue;
		is_range = (sscanf(line, "%d%*[-~]%d %d",&start_range,&end_range,&level)==3); // ID Range [MC Cameri]
		if (!is_range && sscanf(line, "%d %d", &account_id, &level) != 2 && sscanf(line, "%d: %d", &account_id, &level) != 2)
			ShowError("read_gm_account: file [%s], invalid 'acount_id|range level' format (line #%d).\n", GM_account_filename, line_counter);
		else if (level <= 0)
			ShowError("read_gm_account: file [%s] %dth account (line #%d) (invalid level [0 or negative]: %d).\n", GM_account_filename, GM_num+1, line_counter, level);
		else {
			if (level > 99) {
				ShowNotice("read_gm_account: file [%s] %dth account (invalid level, but corrected: %d->99).\n", GM_account_filename, GM_num+1, level);
				level = 99;
			}
			if (is_range) {
				if (start_range==end_range)
					ShowError("read_gm_account: file [%s] invalid range, beginning of range is equal to end of range (line #%d).\n", GM_account_filename, line_counter);
				else if (start_range>end_range)
					ShowError("read_gm_account: file [%s] invalid range, beginning of range must be lower than end of range (line #%d).\n", GM_account_filename, line_counter);
				else
					for (current_id = start_range;current_id<=end_range;current_id++)
						addGM(current_id,level);
			} else {
				addGM(account_id,level);
			}
		}
	}
	fclose(fp);

	ShowStatus("read_gm_account: file '%s' read (%d GM accounts found).\n", GM_account_filename, GM_num);

	return 0;
}

//--------------------------------------------------------------
// Test of the IP mask
// (ip: IP to be tested, str: mask x.x.x.x/# or x.x.x.x/y.y.y.y)
//--------------------------------------------------------------
int check_ipmask(uint32 ip, const unsigned char *str)
{
	unsigned int i = 0, m = 0;
	uint32 ip2, mask = 0;
	uint32 a0, a1, a2, a3;
	uint8* p = (uint8 *)&ip2, *p2 = (uint8 *)&mask;


	// scan ip address
	if (sscanf((const char*)str, "%u.%u.%u.%u/%n", &a0, &a1, &a2, &a3, &i) != 4 || i == 0)
		return 0;
	p[0] = (uint8)a3; p[1] = (uint8)a2; p[2] = (uint8)a1; p[3] = (uint8)a0;

	// scan mask
	if (sscanf((const char*)str+i, "%u.%u.%u.%u", &a0, &a1, &a2, &a3) == 4) {
		p2[0] = (uint8)a3; p2[1] = (uint8)a2; p2[2] = (uint8)a1; p2[3] = (uint8)a0;
	} else if (sscanf((const char*)(str+i), "%u", &m) == 1 && m <= 32) {
		for(i = 32 - m; i < 32; i++)
			mask |= (1 << i);
	} else {
		ShowError("check_ipmask: invalid mask [%s].\n", str);
		return 0;
	}

	return ((ip & mask) == (ip2 & mask));
}

//---------------------
// Access control by IP
//---------------------
int check_ip(uint32 ip)
{
	int i;
	char buf[20];
	char * access_ip;
	enum { ACF_DEF, ACF_ALLOW, ACF_DENY } flag = ACF_DEF;

	if (access_allownum == 0 && access_denynum == 0)
		return 1; // When there is no restriction, all IP are authorised.

//	+   012.345.: front match form, or
//	    all: all IP are matched, or
//	    012.345.678.901/24: network form (mask with # of bits), or
//	    012.345.678.901/255.255.255.0: network form (mask with ip mask)
//	+   Note about the DNS resolution (like www.ne.jp, etc.):
//	    There is no guarantee to have an answer.
//	    If we have an answer, there is no guarantee to have a 100% correct value.
//	    And, the waiting time (to check) can be long (over 1 minute to a timeout). That can block the software.
//	    So, DNS notation isn't authorised for ip checking.
	sprintf(buf, "%u.%u.%u.%u.", CONVIP(ip));

	for(i = 0; i < access_allownum; i++) {
		access_ip = access_allow + i * ACO_STRSIZE;
		if (memcmp(access_ip, buf, strlen(access_ip)) == 0 || check_ipmask(ip, (unsigned char*)access_ip)) {
			if(access_order == ACO_ALLOW_DENY)
				return 1; // With 'allow, deny' (deny if not allow), allow has priority
			flag = ACF_ALLOW;
			break;
		}
	}

	for(i = 0; i < access_denynum; i++) {
		access_ip = access_deny + i * ACO_STRSIZE;
		if (memcmp(access_ip, buf, strlen(access_ip)) == 0 || check_ipmask(ip, (unsigned char*)access_ip)) {
			//flag = ACF_DENY; // not necessary to define flag
			return 0; // At this point, if it's 'deny', we refuse connection.
		}
	}

	return (flag == ACF_ALLOW || access_order == ACO_DENY_ALLOW) ? 1:0;
		// With 'mutual-failture', only 'allow' and non 'deny' IP are authorised.
		//   A non 'allow' (even non 'deny') IP is not authorised. It's like: if allowed and not denied, it's authorised.
		//   So, it's disapproval if you have no description at the time of 'mutual-failture'.
		// With 'deny,allow' (allow if not deny), because here it's not deny, we authorise.
}

//--------------------------------
// Access control by IP for ladmin
//--------------------------------
int check_ladminip(uint32 ip)
{
	int i;
	char buf[20];
	char * access_ip;

	if (access_ladmin_allownum == 0)
		return 1; // When there is no restriction, all IP are authorised.

//	+   012.345.: front match form, or
//	    all: all IP are matched, or
//	    012.345.678.901/24: network form (mask with # of bits), or
//	    012.345.678.901/255.255.255.0: network form (mask with ip mask)
//	+   Note about the DNS resolution (like www.ne.jp, etc.):
//	    There is no guarantee to have an answer.
//	    If we have an answer, there is no guarantee to have a 100% correct value.
//	    And, the waiting time (to check) can be long (over 1 minute to a timeout). That can block the software.
//	    So, DNS notation isn't authorised for ip checking.
	sprintf(buf, "%u.%u.%u.%u.", CONVIP(ip));

	for(i = 0; i < access_ladmin_allownum; i++) {
		access_ip = access_ladmin_allow + i * ACO_STRSIZE;
		if (memcmp(access_ip, buf, strlen(access_ip)) == 0 || check_ipmask(ip, (unsigned char*)access_ip)) {
			return 1;
		}
	}

	return 0;
}

//-----------------------------------------------
// Search an account id
//   (return account index or -1 (if not found))
//   If exact account name is not found,
//   the function checks without case sensitive
//   and returns index if only 1 account is found
//   and similar to the searched name.
//-----------------------------------------------
int search_account_index(char* account_name)
{
	unsigned int i, quantity;
	int index;

	quantity = 0;
	index = -1;

	for(i = 0; i < auth_num; i++) {
		// Without case sensitive check (increase the number of similar account names found)
		if (stricmp(auth_dat[i].userid, account_name) == 0) {
			// Strict comparison (if found, we finish the function immediatly with correct value)
			if (strcmp(auth_dat[i].userid, account_name) == 0)
				return i;
			quantity++;
			index = i;
		}
	}
	// Here, the exact account name is not found
	// We return the found index of a similar account ONLY if there is 1 similar account
	if (quantity == 1)
		return index;

	// Exact account name is not found and 0 or more than 1 similar accounts have been found ==> we say not found
	return -1;
}

//--------------------------------------------------------
// Create a string to save the account in the account file
//--------------------------------------------------------
int mmo_auth_tostr(char* str, struct auth_data* p)
{
	int i;
	char *str_p = str;

	str_p += sprintf(str_p, "%d\t%s\t%s\t%s\t%c\t%d\t%u\t%s\t%s\t%ld\t%s\t%s\t%ld\t",
	                 p->account_id, p->userid, p->pass, p->lastlogin,
	                 p->sex == 2 ? 'S' : p->sex == 1 ? 'M' : 'F',
	                 p->logincount, p->state, p->email, p->error_message,
	                 (long)p->connect_until_time, p->last_ip, p->memo, (long)p->ban_until_time);

	for(i = 0; i < p->account_reg2_num; i++)
		if (p->account_reg2[i].str[0])
			str_p += sprintf(str_p, "%s,%s ", p->account_reg2[i].str, p->account_reg2[i].value);

	return 0;
}

//---------------------------------
// Reading of the accounts database
//---------------------------------
int mmo_auth_init(void)
{
	FILE *fp;
	int account_id;
	uint32 state;
	int logincount, n;
	uint32 i, j;
	char line[2048], *p, userid[2048], pass[2048], lastlogin[2048], sex, email[2048], error_message[2048], last_ip[2048], memo[2048];
	long ban_until_time;
	long connect_until_time;
	char str[2048];
	char v[2048];
	int GM_count = 0;
	int server_count = 0;

	auth_max = 256;
	CREATE(auth_dat, struct auth_data, auth_max);

	if ((fp = fopen(account_filename, "r")) == NULL) {
		// no account file -> no account -> no login, including char-server (ERROR)
		ShowError(CL_RED"mmmo_auth_init: Accounts file [%s] not found."CL_RESET"\n", account_filename);
		return 0;
	}

	while(fgets(line, sizeof(line), fp) != NULL)
	{
		if (line[0] == '/' && line[1] == '/')
			continue;

		p = line;

		memset(userid, 0, sizeof(userid));
		memset(pass, 0, sizeof(pass));
		memset(lastlogin, 0, sizeof(lastlogin));
		memset(email, 0, sizeof(email));
		memset(error_message, 0, sizeof(error_message));
		memset(last_ip, 0, sizeof(last_ip));
		memset(memo, 0, sizeof(memo));

		// database version reading (v2)
		if (((i = sscanf(line, "%d\t%[^\t]\t%[^\t]\t%[^\t]\t%c\t%d\t%u\t"
		                 "%[^\t]\t%[^\t]\t%ld\t%[^\t]\t%[^\t]\t%ld%n",
		    &account_id, userid, pass, lastlogin, &sex, &logincount, &state,
		    email, error_message, &connect_until_time, last_ip, memo, &ban_until_time, &n)) == 13 && line[n] == '\t') ||
		    ((i = sscanf(line, "%d\t%[^\t]\t%[^\t]\t%[^\t]\t%c\t%d\t%u\t"
		                 "%[^\t]\t%[^\t]\t%ld\t%[^\t]\t%[^\t]%n",
		    &account_id, userid, pass, lastlogin, &sex, &logincount, &state,
		    email, error_message, &connect_until_time, last_ip, memo, &n)) == 12 && line[n] == '\t')) {
			n = n + 1;

			// Some checks
			if (account_id > END_ACCOUNT_NUM) {
				ShowError(CL_RED"mmmo_auth_init: an account has an id higher than %d\n", END_ACCOUNT_NUM);
				ShowError("               account id #%d -> account not read (data is lost!)."CL_RESET"\n", account_id);
				continue;
			}
			userid[23] = '\0';
			remove_control_chars(userid);
			for(j = 0; j < auth_num; j++) {
				if (auth_dat[j].account_id == account_id) {
					ShowError(CL_RED"mmmo_auth_init: an account has an identical id to another.\n");
					ShowError("               account id #%d -> new account not read (data is lost!)."CL_RED"\n", account_id);
					break;
				} else if (strcmp(auth_dat[j].userid, userid) == 0) {
					ShowError(CL_RED"mmmo_auth_init: account name already exists.\n");
					ShowError("               account name '%s' -> new account not read (data is lost!)."CL_RESET"\n", userid); // 2 lines, account name can be long.
					break;
				}
			}
			if (j != auth_num)
				continue;

			if (auth_num >= auth_max) {
				auth_max += 256;
				auth_dat = (struct auth_data*)aRealloc(auth_dat, sizeof(struct auth_data) * auth_max);
			}

			memset(&auth_dat[auth_num], '\0', sizeof(struct auth_data));

			auth_dat[auth_num].account_id = account_id;

			strncpy(auth_dat[auth_num].userid, userid, 24);

			pass[32] = '\0';
			remove_control_chars(pass);
			strncpy(auth_dat[auth_num].pass, pass, 32);

			lastlogin[23] = '\0';
			remove_control_chars(lastlogin);
			strncpy(auth_dat[auth_num].lastlogin, lastlogin, 24);

			auth_dat[auth_num].sex = (sex == 'S' || sex == 's') ? 2 : (sex == 'M' || sex == 'm') ? 1 : 0;

			if (logincount >= 0)
				auth_dat[auth_num].logincount = logincount;
			else
				auth_dat[auth_num].logincount = 0;

			if (state > 255)
				auth_dat[auth_num].state = 100;
			else
				auth_dat[auth_num].state = state;

			if (e_mail_check(email) == 0) {
				ShowNotice("Account %s (%d): invalid e-mail (replaced par a@a.com).\n", auth_dat[auth_num].userid, auth_dat[auth_num].account_id);
				strncpy(auth_dat[auth_num].email, "a@a.com", 40);
			} else {
				remove_control_chars(email);
				strncpy(auth_dat[auth_num].email, email, 40);
			}

			error_message[19] = '\0';
			remove_control_chars(error_message);
			if (error_message[0] == '\0' || state != 7) { // 7, because state is packet 0x006a value + 1
				strncpy(auth_dat[auth_num].error_message, "-", 20);
			} else {
				strncpy(auth_dat[auth_num].error_message, error_message, 20);
			}

			if (i == 13)
				auth_dat[auth_num].ban_until_time = (time_t)ban_until_time;
			else
				auth_dat[auth_num].ban_until_time = 0;

			auth_dat[auth_num].connect_until_time = (time_t)connect_until_time;

			last_ip[15] = '\0';
			remove_control_chars(last_ip);
			strncpy(auth_dat[auth_num].last_ip, last_ip, 16);

			memo[254] = '\0';
			remove_control_chars(memo);
			strncpy(auth_dat[auth_num].memo, memo, 255);

			for(j = 0; j < ACCOUNT_REG2_NUM; j++) {
				p += n;
				if (sscanf(p, "%[^\t,],%[^\t ] %n", str, v, &n) != 2) { 
					// We must check if a str is void. If it's, we can continue to read other REG2.
					// Account line will have something like: str2,9 ,9 str3,1 (here, ,9 is not good)
					if (p[0] == ',' && sscanf(p, ",%[^\t ] %n", v, &n) == 1) { 
						j--;
						continue;
					} else
						break;
				}
				str[31] = '\0';
				remove_control_chars(str);
				strncpy(auth_dat[auth_num].account_reg2[j].str, str, 32);
				strncpy(auth_dat[auth_num].account_reg2[j].value,v,256);
			}
			auth_dat[auth_num].account_reg2_num = j;

			if (isGM(account_id) > 0)
				GM_count++;
			if (auth_dat[auth_num].sex == 2)
				server_count++;

			auth_num++;
			if (account_id >= account_id_count)
				account_id_count = account_id + 1;

		// Old athena database version reading (v1)
		} else if ((i = sscanf(line, "%d\t%[^\t]\t%[^\t]\t%[^\t]\t%c\t%d\t%u\t%n",
		           &account_id, userid, pass, lastlogin, &sex, &logincount, &state, &n)) >= 5) {
			if (account_id > END_ACCOUNT_NUM) {
				ShowError(CL_RED"mmmo_auth_init: an account has an id higher than %d\n", END_ACCOUNT_NUM);
				ShowError("               account id #%d -> account not read (data is lost!)."CL_RESET"\n", account_id);
				continue;
			}
			userid[23] = '\0';
			remove_control_chars(userid);
			for(j = 0; j < auth_num; j++) {
				if (auth_dat[j].account_id == account_id) {
					ShowError(CL_RED"mmo_auth_init: an account has an identical id to another.\n");
					ShowError("               account id #%d -> new account not read (data is lost!)."CL_RESET"\n", account_id);
					break;
				} else if (strcmp(auth_dat[j].userid, userid) == 0) {
					ShowError(CL_RED"mmo_auth_init: account name already exists.\n");
					ShowError("               account name '%s' -> new account not read (data is lost!)."CL_RESET"\n", userid);
					break;
				}
			}
			if (j != auth_num)
				continue;

			if (auth_num >= auth_max) {
				auth_max += 256;
				RECREATE(auth_dat, struct auth_data, auth_max);
			}

			memset(&auth_dat[auth_num], '\0', sizeof(struct auth_data));

			auth_dat[auth_num].account_id = account_id;

			strncpy(auth_dat[auth_num].userid, userid, 24);

			pass[23] = '\0';
			remove_control_chars(pass);
			strncpy(auth_dat[auth_num].pass, pass, 24);

			lastlogin[23] = '\0';
			remove_control_chars(lastlogin);
			strncpy(auth_dat[auth_num].lastlogin, lastlogin, 24);

			auth_dat[auth_num].sex = (sex == 'S' || sex == 's') ? 2 : (sex == 'M' || sex == 'm') ? 1 : 0;

			if (i >= 6) {
				if (logincount >= 0)
					auth_dat[auth_num].logincount = logincount;
				else
					auth_dat[auth_num].logincount = 0;
			} else
				auth_dat[auth_num].logincount = 0;

			if (i >= 7) {
				if (state > 255)
					auth_dat[auth_num].state = 100;
				else
					auth_dat[auth_num].state = state;
			} else
				auth_dat[auth_num].state = 0;

			// Initialization of new data
			strncpy(auth_dat[auth_num].email, "a@a.com", 40);
			strncpy(auth_dat[auth_num].error_message, "-", 20);
			auth_dat[auth_num].ban_until_time = 0;
			auth_dat[auth_num].connect_until_time = 0;
			strncpy(auth_dat[auth_num].last_ip, "-", 16);
			strncpy(auth_dat[auth_num].memo, "-", 255);

			for(j = 0; j < ACCOUNT_REG2_NUM; j++) {
				p += n;
				if (sscanf(p, "%[^\t,],%[^\t ] %n", str, v, &n) != 2) { 
					// We must check if a str is void. If it's, we can continue to read other REG2.
					// Account line will have something like: str2,9 ,9 str3,1 (here, ,9 is not good)
					if (p[0] == ',' && sscanf(p, ",%[^\t ] %n", v, &n) == 1) { 
						j--;
						continue;
					} else
						break;
				}
				str[31] = '\0';
				remove_control_chars(str);
				strncpy(auth_dat[auth_num].account_reg2[j].str, str, 32);
				strncpy(auth_dat[auth_num].account_reg2[j].value,v,256);
			}
			auth_dat[auth_num].account_reg2_num = j;

			if (isGM(account_id) > 0)
				GM_count++;
			if (auth_dat[auth_num].sex == 2)
				server_count++;

			auth_num++;
			if (account_id >= account_id_count)
				account_id_count = account_id + 1;

		} else {
			int i = 0;
			if (sscanf(line, "%d\t%%newid%%\n%n", &account_id, &i) == 1 &&
			    i > 0 && account_id > account_id_count)
				account_id_count = account_id;
		}
	}
	fclose(fp);

	if( auth_num == 0 )
		ShowNotice("mmo_auth_init: No account found in %s.\n", account_filename);
	else
	if( auth_num == 1 )
		ShowStatus("mmo_auth_init: 1 account read in %s,\n", account_filename);
	else
		ShowStatus("mmo_auth_init: %d accounts read in %s,\n", auth_num, account_filename);

	if( GM_count == 0 )
		ShowStatus("               of which is no GM account, and \n");
	else
	if( GM_count == 1 )
		ShowStatus("               of which is 1 GM account, and \n");
	else
		ShowStatus("               of which is %d GM accounts, and \n", GM_count);

	if( server_count == 0 )
		ShowStatus("               no server account ('S').\n");
	else
	if( server_count == 1 )
		ShowStatus("               1 server account ('S').\n");
	else
		ShowStatus("               %d server accounts ('S').\n", server_count);

	return 0;
}

//------------------------------------------
// Writing of the accounts database file
//   (accounts are sorted by id before save)
//------------------------------------------
void mmo_auth_sync(void)
{
	FILE *fp;
	unsigned int i, j, k;
	int lock;
	int account_id;
	CREATE_BUFFER(id, int, auth_num);
	char line[65536];

	// Sorting before save
	for(i = 0; i < auth_num; i++) {
		id[i] = i;
		account_id = auth_dat[i].account_id;
		for(j = 0; j < i; j++) {
			if (account_id < auth_dat[id[j]].account_id) {
				for(k = i; k > j; k--)
					id[k] = id[k-1];
				id[j] = i; // id[i]
				break;
			}
		}
	}

	// Data save
	if ((fp = lock_fopen(account_filename, &lock)) == NULL) {
		//if (id) aFree(id); // aFree, right?
		DELETE_BUFFER(id);
		return;
	}

	fprintf(fp, "// Accounts file: here are saved all information about the accounts.\n");
	fprintf(fp, "// Structure: ID, account name, password, last login time, sex, # of logins, state, email, error message for state 7, validity time, last (accepted) login ip, memo field, ban timestamp, repeated(register text, register value)\n");
	fprintf(fp, "// Some explanations:\n");
	fprintf(fp, "//   account name    : between 4 to 23 char for a normal account (standard client can't send less than 4 char).\n");
	fprintf(fp, "//   account password: between 4 to 23 char\n");
	fprintf(fp, "//   sex             : M or F for normal accounts, S for server accounts\n");
	fprintf(fp, "//   state           : 0: account is ok, 1 to 256: error code of packet 0x006a + 1\n");
	fprintf(fp, "//   email           : between 3 to 39 char (a@a.com is like no email)\n");
	fprintf(fp, "//   error message   : text for the state 7: 'Your are Prohibited to login until <text>'. Max 19 char\n");
	fprintf(fp, "//   valitidy time   : 0: unlimited account, <other value>: date calculated by addition of 1/1/1970 + value (number of seconds since the 1/1/1970)\n");
	fprintf(fp, "//   memo field      : max 254 char\n");
	fprintf(fp, "//   ban time        : 0: no ban, <other value>: banned until the date: date calculated by addition of 1/1/1970 + value (number of seconds since the 1/1/1970)\n");
	for(i = 0; i < auth_num; i++) {
		k = id[i]; // use of sorted index
		if (auth_dat[k].account_id == -1)
			continue;

		mmo_auth_tostr(line, &auth_dat[k]);
		fprintf(fp, "%s\n", line);
	}
	fprintf(fp, "%d\t%%newid%%\n", account_id_count);

	lock_fclose(fp, account_filename, &lock);

	// set new counter to minimum number of auth before save
	auth_before_save_file = auth_num / AUTH_SAVE_FILE_DIVIDER; // Re-initialise counter. We have save.
	if (auth_before_save_file < AUTH_BEFORE_SAVE_FILE)
		auth_before_save_file = AUTH_BEFORE_SAVE_FILE;

	//if (id) aFree(id);
	DELETE_BUFFER(id);

	return;
}

//-----------------------------------------------------
// Check if we must save accounts file or not
//   every minute, we check if we must save because we
//   have do some authentications without arrive to
//   the minimum of authentications for the save.
// Note: all other modification of accounts (deletion,
//       change of some informations excepted lastip/
//       lastlogintime, creation) are always save
//       immediatly and set  the minimum of
//       authentications to its initialization value.
//-----------------------------------------------------
int check_auth_sync(int tid, unsigned int tick, int id, int data)
{
	// we only save if necessary:
	// we have do some authentications without do saving
	if (auth_before_save_file < AUTH_BEFORE_SAVE_FILE ||
	    auth_before_save_file < (int)(auth_num / AUTH_SAVE_FILE_DIVIDER))
		mmo_auth_sync();

	return 0;
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
// Send GM accounts to one or all char-servers
//-----------------------------------------------------
void send_GM_accounts(int fd)
{
	unsigned int i;
	uint8 buf[32767];
	uint16 len;

	len = 4;
	WBUFW(buf,0) = 0x2732;
	for(i = 0; i < GM_num; i++)
		// send only existing accounts. We can not create a GM account when server is online.
		if (gm_account_db[i].level > 0) {
			WBUFL(buf,len) = gm_account_db[i].account_id;
			WBUFB(buf,len+4) = (uint8)gm_account_db[i].level;
			len += 5;
			if (len >= 32000) {
				ShowWarning("send_GM_accounts: Too many accounts! Only %d out of %d were sent.\n", i, GM_num);
				break;
			}
		}

	WBUFW(buf,2) = len;
	if (fd == -1) // send to all charservers
		charif_sendallwos(-1, buf, len);
	else { // send only to target
		WFIFOHEAD(fd,len);
		memcpy(WFIFOP(fd,0), buf, len);
		WFIFOSET(fd,len);
	}

	return;
}

//-----------------------------------------------------
// Check if GM file account have been changed
//-----------------------------------------------------
int check_GM_file(int tid, unsigned int tick, int id, int data)
{
	struct stat file_stat;
	long new_time;

	// if we would not check
	if (gm_account_filename_check_timer < 1)
		return 0;

	// get last modify time/date
	if (stat(GM_account_filename, &file_stat))
		new_time = 0; // error
	else
		new_time = (long)file_stat.st_mtime;

	if (new_time != creation_time_GM_account_file) {
		read_gm_account();
		send_GM_accounts(-1);
	}

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


//-------------------------------------
// Make new account
//-------------------------------------
int mmo_auth_new(struct mmo_account* account, char sex, char* email)
{
	time_t timestamp, timestamp_temp;
	struct tm *tmtime;
	int i = auth_num;

	if (auth_num >= auth_max) {
		auth_max += 256;
		auth_dat = (struct auth_data*)aRealloc(auth_dat, sizeof(struct auth_data) * auth_max);
	}

	memset(&auth_dat[i], '\0', sizeof(struct auth_data));

	while (isGM(account_id_count) > 0)
		account_id_count++;

	auth_dat[i].account_id = account_id_count++;
	safestrncpy(auth_dat[i].userid, account->userid, NAME_LENGTH);
	if( login_config.use_md5_passwds )
		MD5_String(account->passwd, auth_dat[i].pass);
	else
		safestrncpy(auth_dat[i].pass, account->passwd, NAME_LENGTH);
	safestrncpy(auth_dat[i].lastlogin, "-", sizeof(auth_dat[i].lastlogin));
	auth_dat[i].sex = (sex == 'M' || sex == 'm');
	auth_dat[i].logincount = 0;
	auth_dat[i].state = 0;
	safestrncpy(auth_dat[i].email, e_mail_check(email) ? email : "a@a.com", sizeof(auth_dat[i].email));
	safestrncpy(auth_dat[i].error_message, "-", sizeof(auth_dat[i].error_message));
	auth_dat[i].ban_until_time = 0;
	if (start_limited_time < 0)
		auth_dat[i].connect_until_time = 0; // unlimited
	else { // limited time
		timestamp = time(NULL) + start_limited_time;
		// double conversion to be sure that it is possible
		tmtime = localtime(&timestamp);
		timestamp_temp = mktime(tmtime);
		if (timestamp_temp != -1 && (timestamp_temp + 3600) >= timestamp) // check possible value and overflow (and avoid summer/winter hour)
			auth_dat[i].connect_until_time = timestamp_temp;
		else
			auth_dat[i].connect_until_time = 0; // unlimited
	}

	strncpy(auth_dat[i].last_ip, "-", 16);
	strncpy(auth_dat[i].memo, "-", 255);
	auth_dat[i].account_reg2_num = 0;

	auth_num++;

	return (account_id_count - 1);
}

//-----------------------------------------------------
// Check/authentication of a connection
//-----------------------------------------------------
int mmo_auth(struct mmo_account* account, int fd)
{
	unsigned int i;
	time_t raw_time;
	char tmpstr[256];
	int len;
	int newaccount = 0;
	char user_password[32+1]; // reserve for md5-ed pw

	char ip[16];
	ip2str(session[fd]->client_addr, ip);

	// DNS Blacklist check
	if( login_config.use_dnsbl )
	{
		char r_ip[16];
		char ip_dnsbl[256];
		char* dnsbl_serv;
		bool matched = false;
		uint8* sin_addr = (uint8*)&session[fd]->client_addr;

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
			account->userid[len-2] == '_' && memchr("FfMm", (unsigned char)account->userid[len-1], 4) && // _M/_F suffix
			account_id_count <= END_ACCOUNT_NUM )
		{
			//only continue if amount in this time limit is allowed (account registration flood protection)[Kevin]
			if(DIFF_TICK(gettick(), new_reg_tick) < 0 && num_regs >= allowed_regs) {
				ShowNotice("Account registration denied (registration limit exceeded) to %s!\n", ip);
				return 3;
			}
			newaccount = 1;
			account->userid[len-2] = '\0';
		}
	}
	
	// Strict account search
	ARR_FIND( 0, auth_num, i, strcmp(account->userid, auth_dat[i].userid) == 0 );

	if( newaccount )
	{
		if( i != auth_num )
		{
			ShowNotice("Attempt of creation of an already existant account (account: %s_%c, pass: %s, received pass: %s, ip: %s)\n", account->userid, account->userid[len-1], auth_dat[i].pass, account->passwd, ip);
			return 1; // 1 = Incorrect Password
		}
		else
		{
			int new_id = mmo_auth_new(account, account->userid[len-1], "a@a.com");
			unsigned int tick = gettick();
			ShowNotice("Account creation (account %s, id: %d, pass: %s, sex: %c, connection with _F/_M, ip: %s)\n", account->userid, new_id, account->passwd, account->userid[len-1], ip);
			auth_before_save_file = 0; // Creation of an account -> save accounts file immediatly
			
			if( DIFF_TICK(tick, new_reg_tick) > 0 )
			{	//Update the registration check.
				num_regs = 0;
				new_reg_tick=tick +time_allowed*1000;
			}
			num_regs++;
		}
	}

	// if there is no creation request and strict account search fails, we do a no sensitive case research for index
	if( !newaccount && i == auth_num )
	{
		i = search_account_index(account->userid);
		if( i == -1 )
			i = auth_num;
		else
			memcpy(account->userid, auth_dat[i].userid, NAME_LENGTH); // for the possible tests/checks afterwards (copy correcte sensitive case).
	}

	if( i == auth_num )
	{
		ShowNotice("Unknown account (account: %s, received pass: %s, ip: %s)\n", account->userid, account->passwd, ip);
		return 0; // 0 = Unregistered ID
	}

	if( login_config.use_md5_passwds )
		MD5_String(account->passwd, user_password);
	else
		safestrncpy(user_password, account->passwd, NAME_LENGTH);

	if( !check_password(session[fd]->session_data, account->passwdenc, user_password, auth_dat[i].pass) )
	{
		ShowNotice("Invalid password (account: %s, pass: %s, received pass: %s, ip: %s)\n", account->userid, auth_dat[i].pass, (account->passwdenc) ? "[MD5]" : account->passwd, ip);
		return 1; // 1 = Incorrect Password
	}

	if( auth_dat[i].connect_until_time != 0 && auth_dat[i].connect_until_time < time(NULL) )
	{
		ShowNotice("Connection refused (account: %s, pass: %s, expired ID, ip: %s)\n", account->userid, account->passwd, ip);
		return 2; // 2 = This ID is expired
	}

	if( auth_dat[i].ban_until_time != 0 && auth_dat[i].ban_until_time > time(NULL) )
	{
		strftime(tmpstr, 20, login_config.date_format, localtime(&auth_dat[i].ban_until_time));
		tmpstr[19] = '\0';
		ShowNotice("Connection refused (account: %s, pass: %s, banned until %s, ip: %s)\n", account->userid, account->passwd, tmpstr, ip);
		return 6; // 6 = Your are Prohibited to log in until %s
	}

	if( auth_dat[i].state )
	{
		ShowNotice("Connection refused (account: %s, pass: %s, state: %d, ip: %s)\n", account->userid, account->passwd, auth_dat[i].state, ip);
		return auth_dat[i].state - 1;
	}

	if( login_config.online_check )
	{
		struct online_login_data* data = idb_get(online_db,auth_dat[i].account_id);
		if( data && data->char_server > -1 )
		{
			//Request char servers to kick this account out. [Skotlex]
			uint8 buf[8];
			ShowNotice("User '%s' is already online - Rejected.\n", auth_dat[i].userid);
			WBUFW(buf,0) = 0x2734;
			WBUFL(buf,2) = auth_dat[i].account_id;
			charif_sendallwos(-1, buf, 6);
			if( data->waiting_disconnect == -1 )
				data->waiting_disconnect = add_timer(gettick()+30000, waiting_disconnect_timer, auth_dat[i].account_id, 0);
			return 3; // Rejected
		}
	}

	ShowNotice("Authentication accepted (account: %s, id: %d, ip: %s)\n", account->userid, auth_dat[i].account_id, ip);

	// auth start : time seed
	time(&raw_time);
	strftime(tmpstr, 24, "%Y-%m-%d %H:%M:%S",localtime(&raw_time));

	account->account_id = auth_dat[i].account_id;
	account->login_id1 = rand();
	account->login_id2 = rand();
	safestrncpy(account->lastlogin, auth_dat[i].lastlogin, 24);
	account->sex = auth_dat[i].sex;

	if( account->sex != 2 && account->account_id < START_ACCOUNT_NUM )
		ShowWarning("Account %s has account id %d! Account IDs must be over %d to work properly!\n", account->userid, account->account_id, START_ACCOUNT_NUM);

	safestrncpy(auth_dat[i].lastlogin, tmpstr, sizeof(auth_dat[i].lastlogin));
	safestrncpy(auth_dat[i].last_ip, ip, sizeof(auth_dat[i].last_ip));
	auth_dat[i].ban_until_time = 0;
	auth_dat[i].logincount++;

	// Save until for change ip/time of auth is not very useful => limited save for that
	// Save there informations isnot necessary, because they are saved in log file.
	if (--auth_before_save_file <= 0) // Reduce counter. 0 or less, we save
		mmo_auth_sync();

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
	uint32 i;
	int j, id;
	uint32 ipl;
	char ip[16];

	ARR_FIND( 0, MAX_SERVERS, id, server[id].fd == fd );
	if( id == MAX_SERVERS )
	{// not a char server
		set_eof(fd);
		do_close(fd);
		return 0;
	}

	if( session[fd]->flag.eof )
	{
		ShowStatus("Char-server '%s' has disconnected.\n", server[id].name);
		online_db->foreach(online_db, online_db_setoffline, id); //Set all chars from this char server to offline.
		memset(&server[id], 0, sizeof(struct mmo_char_server));
		server[id].fd = -1;
		do_close(fd);
		return 0;
	}

	ipl = server[id].ip;
	ip2str(ipl, ip);

	while( RFIFOREST(fd) >= 2 )
	{
		uint16 command = RFIFOW(fd,0);

		switch( command )
		{

		case 0x2709: // request from map-server via char-server to reload GM accounts
			ShowStatus("Char-server '%s': Request to re-load GM configuration file (ip: %s).\n", server[id].name, ip);
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
				time_t connect_until_time;
				char email[40];
				unsigned int k;

				//ShowStatus("Char-server '%s': authentication of the account %d accepted (ip: %s).\n", server[id].name, account_id, ip);

				ARR_FIND( 0, auth_num, k, auth_dat[k].account_id == account_id );
				if( k < auth_num )
				{
					strcpy(email, auth_dat[k].email);
					connect_until_time = auth_dat[k].connect_until_time;
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
				WFIFOL(fd,47) = (unsigned long)connect_until_time;
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
			//printf("parse_fromchar: Receiving of the users number of the server '%s': %d\n", server[id].name, RFIFOL(fd,2));
			server[id].users = RFIFOL(fd,2);

			RFIFOSKIP(fd,6);
		break;

		case 0x2715: // request from char server to change e-email from default "a@a.com"
			if (RFIFOREST(fd) < 46)
				return 0;
		{
			char email[40];
			int acc = RFIFOL(fd,2);
			safestrncpy(email, (char*)RFIFOP(fd,6), 40);
			remove_control_chars(email);
			if( e_mail_check(email) == 0 )
				ShowNotice("Char-server '%s': Attempt to create an e-mail on an account with a default e-mail REFUSED - e-mail is invalid (account: %d, ip: %s)\n", server[id].name, acc, ip);
			else
			{
				ARR_FIND( 0, auth_num, i, auth_dat[i].account_id == acc && (strcmp(auth_dat[i].email, "a@a.com") == 0 || auth_dat[i].email[0] == '\0') );
				if( i == auth_num )
					ShowNotice("Char-server '%s': Attempt to create an e-mail on an account with a default e-mail REFUSED - account doesn't exist or e-mail of account isn't default e-mail (account: %d, ip: %s).\n", server[id].name, acc, ip);
				else
				{
					memcpy(auth_dat[i].email, email, 40);
					ShowNotice("Char-server '%s': Create an e-mail on an account with a default e-mail (account: %d, new e-mail: %s, ip: %s).\n", server[id].name, acc, email, ip);
					// Save
					mmo_auth_sync();
				}
			}

			RFIFOSKIP(fd,46);
		}
		break;

		case 0x2716: // received an e-mail/limited time request, because a player comes back from a map-server to the char-server
			if( RFIFOREST(fd) < 6 )
				return 0;

			ARR_FIND( 0, auth_num, i, auth_dat[i].account_id == (int)RFIFOL(fd,2) );
			if( i == auth_num )
				ShowNotice("Char-server '%s': e-mail of the account %d NOT found (ip: %s).\n", server[id].name, RFIFOL(fd,2), ip);
			else
			{
				//ShowNotice("Char-server '%s': e-mail of the account %d found (ip: %s).\n", server[id].name, RFIFOL(fd,2), ip);
				WFIFOW(fd,0) = 0x2717;
				WFIFOL(fd,2) = RFIFOL(fd,2);
				safestrncpy((char*)WFIFOP(fd, 6), auth_dat[i].email, 40);
				WFIFOL(fd,46) = (unsigned long)auth_dat[i].connect_until_time;
				WFIFOSET(fd,50);
			}

			RFIFOSKIP(fd,6);
		break;

		case 0x2719: // ping request from charserver
			if( RFIFOREST(fd) < 2 )
				return 0;

			WFIFOHEAD(fd,2);
			WFIFOW(fd,0) = 0x2718;
			WFIFOSET(fd,2);

			RFIFOSKIP(fd,2);
		break;

		case 0x2720: // Request to become a GM
			if( RFIFOREST(fd) < 4 || RFIFOREST(fd) < RFIFOW(fd,2) )
				return 0;
		{
			unsigned char buf[10];
			FILE *fp;
			int acc = RFIFOL(fd,4);
			WBUFW(buf,0) = 0x2721;
			WBUFL(buf,2) = acc;
			WBUFL(buf,6) = 0;

			if( strcmp((char*)RFIFOP(fd,8), gm_pass) != 0 ) // password check
				ShowError("Char-server '%s': Error of GM change (suggested account: %d, invalid password, ip: %s).\n", server[id].name, acc, ip);
			else
			if( isGM(acc) > 0 ) // only non-GM can become GM
				ShowError("Char-server '%s': Error of GM change (suggested account: %d (already GM), correct password, ip: %s).\n", server[id].name, acc, ip);
			else
			if( level_new_gm == 0 ) // if we autorise creation
				ShowError("Char-server '%s': Error of GM change (suggested account: %d, correct password, but GM creation is disable (level_new_gm = 0), ip: %s).\n", server[id].name, acc, ip);
			else
			if( (fp = fopen(GM_account_filename, "a")) == NULL ) // if we can open the file to add the new GM
				ShowError("Char-server '%s': Error of GM change (suggested account: %d, correct password, unable to add a GM account in GM accounts file, ip: %s).\n", server[id].name, acc, ip);
			else
			{
				char tmpstr[24];
				time_t raw_time;
				time(&raw_time);
				strftime(tmpstr, 23, login_config.date_format, localtime(&raw_time));
				fprintf(fp, "\n// %s: @GM command on account %d\n%d %d\n", tmpstr, acc, acc, level_new_gm);
				fclose(fp);
				WBUFL(buf,6) = level_new_gm;
				read_gm_account();
				send_GM_accounts(-1);

				ShowNotice("Char-server '%s': GM Change of the account %d: level 0 -> %d (ip: %s).\n", server[id].name, acc, level_new_gm, ip);

			}
			// announce gm level update
			charif_sendallwos(-1, buf, 10);

			RFIFOSKIP(fd, RFIFOW(fd,2));
			return 0;
		}

		// Map server send information to change an email of an account via char-server
		case 0x2722:	// 0x2722 <account_id>.L <actual_e-mail>.40B <new_e-mail>.40B
			if (RFIFOREST(fd) < 86)
				return 0;
		{
			char actual_email[40], new_email[40];
			int acc = RFIFOL(fd,2);
			memcpy(actual_email, RFIFOP(fd,6), 40); actual_email[39] = '\0'; remove_control_chars(actual_email);
			memcpy(new_email, RFIFOP(fd,46), 40); new_email[39] = '\0'; remove_control_chars(new_email);
			if (e_mail_check(actual_email) == 0)
				ShowNotice("Char-server '%s': Attempt to modify an e-mail on an account (@email GM command), but actual email is invalid (account: %d, ip: %s)\n", server[id].name, acc, ip);
			else if (e_mail_check(new_email) == 0)
				ShowNotice("Char-server '%s': Attempt to modify an e-mail on an account (@email GM command) with a invalid new e-mail (account: %d, ip: %s)\n", server[id].name, acc, ip);
			else if (strcmpi(new_email, "a@a.com") == 0)
				ShowNotice("Char-server '%s': Attempt to modify an e-mail on an account (@email GM command) with a default e-mail (account: %d, ip: %s)\n", server[id].name, acc, ip);
			else {
				ARR_FIND( 0, auth_num, i, auth_dat[i].account_id == acc );
				if( i == auth_num )
					ShowNotice("Char-server '%s': Attempt to modify an e-mail on an account (@email GM command), but account doesn't exist (account: %d, ip: %s).\n", server[id].name, acc, ip);
				else
				if( strcmpi(auth_dat[i].email, actual_email) != 0 )
					ShowNotice("Char-server '%s': Attempt to modify an e-mail on an account (@email GM command), but actual e-mail is incorrect (account: %d (%s), actual e-mail: %s, proposed e-mail: %s, ip: %s).\n", server[id].name, acc, auth_dat[i].userid, auth_dat[i].email, actual_email, ip);
				else {
					memcpy(auth_dat[i].email, new_email, 40);
					ShowNotice("Char-server '%s': Modify an e-mail on an account (@email GM command) (account: %d (%s), new e-mail: %s, ip: %s).\n", server[id].name, acc, auth_dat[i].userid, new_email, ip);
					// Save
					mmo_auth_sync();
				}
			}

			RFIFOSKIP(fd, 86);
		}
		break;

		case 0x2724: // Receiving an account state update request from a map-server (relayed via char-server)
			if (RFIFOREST(fd) < 10)
				return 0;
		{
			int account_id = RFIFOL(fd,2);
			uint32 state = RFIFOL(fd,6);

			ARR_FIND( 0, auth_num, i, auth_dat[i].account_id == account_id );
			if( i == auth_num )
				ShowNotice("Char-server '%s': Error of Status change (account: %d not found, suggested status %d, ip: %s).\n", server[id].name, account_id, state, ip);
			else
			if( auth_dat[i].state == state )
				ShowNotice("Char-server '%s':  Error of Status change - actual status is already the good status (account: %d, status %d, ip: %s).\n", server[id].name, account_id, state, ip);
			else {
				ShowNotice("Char-server '%s': Status change (account: %d, new status %d, ip: %s).\n", server[id].name, account_id, state, ip);
				if (state != 0) {
					unsigned char buf[16];
					WBUFW(buf,0) = 0x2731;
					WBUFL(buf,2) = account_id;
					WBUFB(buf,6) = 0; // 0: change of state, 1: ban
					WBUFL(buf,7) = state; // status or final date of a banishment
					charif_sendallwos(-1, buf, 11);
					ARR_FIND( 0, AUTH_FIFO_SIZE, j, auth_fifo[j].account_id == account_id );
					if( j < AUTH_FIFO_SIZE )
						auth_fifo[j].login_id1++; // to avoid reconnection error when come back from map-server (char-server will ask again the authentication)
				}
				auth_dat[i].state = state;
				// Save
				mmo_auth_sync();
			}

			RFIFOSKIP(fd,10);
			return 0;
		}

		case 0x2725: // Receiving of map-server via char-server a ban request
			if (RFIFOREST(fd) < 18)
				return 0;
		{
			int acc = RFIFOL(fd,2);
			ARR_FIND( 0, auth_num, i, auth_dat[i].account_id == acc );
			if( i == auth_num )
				ShowNotice("Char-server '%s': Error of ban request (account: %d not found, ip: %s).\n", server[id].name, acc, ip);
			else
			{
				time_t timestamp;
				struct tm *tmtime;
				if (auth_dat[i].ban_until_time == 0 || auth_dat[i].ban_until_time < time(NULL))
					timestamp = time(NULL);
				else
					timestamp = auth_dat[i].ban_until_time;
				tmtime = localtime(&timestamp);
				tmtime->tm_year = tmtime->tm_year + (short)RFIFOW(fd,6);
				tmtime->tm_mon = tmtime->tm_mon + (short)RFIFOW(fd,8);
				tmtime->tm_mday = tmtime->tm_mday + (short)RFIFOW(fd,10);
				tmtime->tm_hour = tmtime->tm_hour + (short)RFIFOW(fd,12);
				tmtime->tm_min = tmtime->tm_min + (short)RFIFOW(fd,14);
				tmtime->tm_sec = tmtime->tm_sec + (short)RFIFOW(fd,16);
				timestamp = mktime(tmtime);
				if (timestamp == -1)
					ShowNotice("Char-server '%s': Error of ban request (account: %d, invalid date, ip: %s).\n", server[id].name, acc, ip);
				else
				{
					if (timestamp <= time(NULL))
						timestamp = 0;
					else
					{
						if (timestamp == 0)
							ShowNotice("Char-server '%s': Error of ban request (account: %d, new date unbans the account, ip: %s).\n", server[id].name, acc, ip);
						else
						{
							unsigned char buf[16];
							char tmpstr[2048];
							strftime(tmpstr, 24, login_config.date_format, localtime(&timestamp));
							ShowNotice("Char-server '%s': Ban request (account: %d, new final date of banishment: %d (%s), ip: %s).\n", server[id].name, acc, timestamp, (timestamp == 0 ? "no banishment" : tmpstr), ip);
							WBUFW(buf,0) = 0x2731;
							WBUFL(buf,2) = auth_dat[i].account_id;
							WBUFB(buf,6) = 1; // 0: change of statut, 1: ban
							WBUFL(buf,7) = (unsigned int)timestamp; // status or final date of a banishment
							charif_sendallwos(-1, buf, 11);
							ARR_FIND( 0, AUTH_FIFO_SIZE, j, auth_fifo[j].account_id == acc );
							if( j < AUTH_FIFO_SIZE )
								auth_fifo[j].login_id1++; // to avoid reconnection error when come back from map-server (char-server will ask again the authentication)
						}
						auth_dat[i].ban_until_time = timestamp;
						// Save
						mmo_auth_sync();
					}
				}
			}

			RFIFOSKIP(fd,18);
			return 0;
		}

		case 0x2727: // Change of sex (sex is reversed)
			if( RFIFOREST(fd) < 6 )
				return 0;
		{
			uint8 sex;
			int acc = RFIFOL(fd,2);
			for(i = 0; i < auth_num; i++) {
				if (auth_dat[i].account_id == acc) {
					if (auth_dat[i].sex == 2)
						ShowNotice("Char-server '%s': Error of sex change - Server account (suggested account: %d, actual sex %d (Server), ip: %s).\n", server[id].name, acc, auth_dat[i].sex, ip);
					else {
						unsigned char buf[16];
						if (auth_dat[i].sex == 0)
							sex = 1;
						else
							sex = 0;
						ShowNotice("Char-server '%s': Sex change (account: %d, new sex %c, ip: %s).\n", server[id].name, acc, (sex == 2) ? 'S' : (sex == 1 ? 'M' : 'F'), ip);
						for(j = 0; j < AUTH_FIFO_SIZE; j++)
							if (auth_fifo[j].account_id == acc)
								auth_fifo[j].login_id1++; // to avoid reconnection error when come back from map-server (char-server will ask again the authentication)
						auth_dat[i].sex = sex;
						WBUFW(buf,0) = 0x2723;
						WBUFL(buf,2) = acc;
						WBUFB(buf,6) = sex;
						charif_sendallwos(-1, buf, 7);
						// Save
						mmo_auth_sync();
					}
					break;
				}
			}
			if (i == auth_num)
				ShowNotice("Char-server '%s': Error of sex change (account: %d not found, sex would be reversed, ip: %s).\n", server[id].name, acc, ip);

			RFIFOSKIP(fd,6);
			return 0;
		}

		case 0x2728:	// We receive account_reg2 from a char-server, and we send them to other map-servers.
			if (RFIFOREST(fd) < 4 || RFIFOREST(fd) < RFIFOW(fd,2))
				return 0;
			{
				int p;
				int acc = RFIFOL(fd,4);
				for(i = 0; i < auth_num; i++) {
					if (auth_dat[i].account_id == acc) {
						//unsigned char buf[rfifow(fd,2)+1];
						uint8* buf;
						int len;
						CREATE(buf, uint8, RFIFOW(fd,2)+1);
						ShowNotice("char-server '%s': receiving (from the char-server) of account_reg2 (account: %d, ip: %s).\n", server[id].name, acc, ip);
						for(j=0,p=13;j<ACCOUNT_REG2_NUM && p<RFIFOW(fd,2);j++){
							sscanf((char*)RFIFOP(fd,p), "%31c%n",auth_dat[i].account_reg2[j].str,&len);
							auth_dat[i].account_reg2[j].str[len]='\0';
							p +=len+1; //+1 to skip the '\0' between strings.
							sscanf((char*)RFIFOP(fd,p), "%255c%n",auth_dat[i].account_reg2[j].value,&len);
							auth_dat[i].account_reg2[j].value[len]='\0';
							p +=len+1;
							remove_control_chars(auth_dat[i].account_reg2[j].str);
							remove_control_chars(auth_dat[i].account_reg2[j].value);
						}
						auth_dat[i].account_reg2_num = j;
						// Sending information towards the other char-servers.
						memcpy(WBUFP(buf,0), RFIFOP(fd,0), RFIFOW(fd,2));
						WBUFW(buf,0) = 0x2729;
						charif_sendallwos(fd, buf, WBUFW(buf,2));
						// Save
						mmo_auth_sync();
//						printf("parse_fromchar: receiving (from the char-server) of account_reg2 (account id: %d).\n", acc);
						if (buf) aFree(buf);
						break;
					}
				}
				if (i == auth_num) {
					ShowStatus("Char-server '%s': receiving (from the char-server) of account_reg2 (account: %d not found, ip: %s).\n", server[id].name, acc, ip);
				}
			}

			RFIFOSKIP(fd,RFIFOW(fd,2));
		break;

		case 0x272a:	// Receiving of map-server via char-server an unban request
			if( RFIFOREST(fd) < 6 )
				return 0;
		{
			int acc = RFIFOL(fd,2);
			for(i = 0; i < auth_num; i++) {
				if (auth_dat[i].account_id == acc) {
					if (auth_dat[i].ban_until_time != 0) {
						auth_dat[i].ban_until_time = 0;
						ShowNotice("Char-server '%s': UnBan request (account: %d, ip: %s).\n", server[id].name, acc, ip);
					} else {
						ShowNotice("Char-server '%s': Error of UnBan request (account: %d, no change for unban date, ip: %s).\n",
						          server[id].name, acc, ip);
					}
					break;
				}
			}
			if (i == auth_num)
				ShowNotice("Char-server '%s': Error of UnBan request (account: %d not found, ip: %s).\n", server[id].name, acc, ip);

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
			int p;
			WFIFOW(fd,0) = 0x2729;
			WFIFOL(fd,4) = account_id;
			WFIFOL(fd,8) = char_id;
			WFIFOB(fd,12) = 1; //Type 1 for Account2 registry
			for(i = 0; i < auth_num && auth_dat[i].account_id != account_id; i++);
			if (i == auth_num) {
				//Account not found? Send at least empty data, map servers need a reply!
				WFIFOW(fd,2) = 13;
				WFIFOSET(fd,WFIFOW(fd,2));
				break;
			}
			for(p = 13, j = 0; j < auth_dat[i].account_reg2_num; j++) {
				if (auth_dat[i].account_reg2[j].str[0]) {
					p+= sprintf((char*)WFIFOP(fd,p), "%s", auth_dat[i].account_reg2[j].str)+1; //We add 1 to consider the '\0' in place.
					p+= sprintf((char*)WFIFOP(fd,p), "%s", auth_dat[i].account_reg2[j].value)+1;
				}
			}
			WFIFOW(fd,2) = (uint16) p;
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
		{
			FILE* logfp;
			char tmpstr[24];
			time_t raw_time;
			logfp = fopen(login_log_unknown_packets_filename, "a");
			if (logfp) {
				time(&raw_time);
				strftime(tmpstr, 23, login_config.date_format, localtime(&raw_time));
				fprintf(logfp, "%s: receiving of an unknown packet -> disconnection\n", tmpstr);
				fprintf(logfp, "parse_fromchar: connection #%d (ip: %s), packet: 0x%x (with being read: %lu).\n", fd, ip, command, (unsigned long)RFIFOREST(fd));
				fprintf(logfp, "Detail (in hex):\n");
				fprintf(logfp, "---- 00-01-02-03-04-05-06-07  08-09-0A-0B-0C-0D-0E-0F\n");
				memset(tmpstr, '\0', sizeof(tmpstr));
				for(i = 0; i < RFIFOREST(fd); i++) {
					if ((i & 15) == 0)
						fprintf(logfp, "%04X ",i);
					fprintf(logfp, "%02x ", RFIFOB(fd,i));
					if (RFIFOB(fd,i) > 0x1f)
						tmpstr[i % 16] = RFIFOB(fd,i);
					else
						tmpstr[i % 16] = '.';
					if ((i - 7) % 16 == 0) // -8 + 1
						fprintf(logfp, " ");
					else if ((i + 1) % 16 == 0) {
						fprintf(logfp, " %s\n", tmpstr);
						memset(tmpstr, '\0', sizeof(tmpstr));
					}
				}
				if (i % 16 != 0) {
					for(j = i; j % 16 != 0; j++) {
						fprintf(logfp, "   ");
						if ((j - 7) % 16 == 0) // -8 + 1
							fprintf(logfp, " ");
					}
					fprintf(logfp, " %s\n", tmpstr);
				}
				fprintf(logfp, "\n");
				fclose(logfp);
			}

			ShowError("parse_fromchar: Unknown packet 0x%x from a char-server! Disconnecting!\n", command);
			set_eof(fd);
			return 0;
		}
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

//----------------------------------------------------------------------------------------
// Default packet parsing (normal players or administation/char-server connection requests)
//----------------------------------------------------------------------------------------
int parse_login(int fd)
{	
	struct mmo_account account;
	int result, j;
	unsigned int i;
	uint32 ipl;
	char ip[16];

	if( session[fd]->flag.eof )
	{
		do_close(fd);
		return 0;
	}

	ipl = session[fd]->client_addr;
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
			if (!check_ip(ipl))
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
			memcpy(account.userid,RFIFOP(fd,6),NAME_LENGTH); account.userid[23] = '\0';
			remove_control_chars(account.userid);
			if (command != 0x01dd) {
				ShowStatus("Request for connection (non encryption mode) of %s (ip: %s).\n", account.userid, ip);
				memcpy(account.passwd, RFIFOP(fd,30), NAME_LENGTH); account.passwd[23] = '\0';
				remove_control_chars(account.passwd);
			} else {
				ShowStatus("Request for connection (encryption mode) of %s (ip: %s).\n", account.userid, ip);
				memcpy(account.passwd, RFIFOP(fd,30), 16); account.passwd[16] = '\0'; // binary data here
			}
			account.passwdenc = (command == 0x01dd) ? PASSWORDENC : 0;

			result = mmo_auth(&account, fd);
			if( result == -1 )
			{	// auth success
				int gm_level = isGM(account.account_id);
				if( login_config.min_level_to_connect > gm_level )
				{
					ShowStatus("Connection refused: the minimum GM level for connection is %d (account: %s, GM level: %d, ip: %s).\n", login_config.min_level_to_connect, account.userid, gm_level, ip);
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
						if (gm_level)
							ShowStatus("Connection of the GM (level:%d) account '%s' accepted.\n", gm_level, account.userid);
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
				WFIFOHEAD(fd,23);
				WFIFOW(fd,0) = 0x6a;
				WFIFOB(fd,2) = (uint8)result;
				if( result != 6 )
					memset(WFIFOP(fd,3), '\0', 20);
				else
				{// 6 = Your are Prohibited to log in until %s
					char tmpstr[20];
					time_t ban_until_time;
					i = search_account_index(account.userid);
					ban_until_time = (i) ? auth_dat[i].ban_until_time : 0;
					strftime(tmpstr, 20, login_config.date_format, localtime(&ban_until_time));
					safestrncpy((char*)WFIFOP(fd,3), tmpstr, 20); // ban timestamp goes here
				}
				WFIFOSET(fd,23);
			}

			RFIFOSKIP(fd,packet_len);
		}
		break;

		case 0x01db:	// Sending request of the coding key
		case 0x791a:	// Sending request of the coding key (administration packet)
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
			char* server_name;
			uint32 server_ip;
			uint16 server_port;

			memset(&account, 0, sizeof(account));
			safestrncpy(account.userid, (char*)RFIFOP(fd,2), NAME_LENGTH); remove_control_chars(account.userid);
			safestrncpy(account.passwd, (char*)RFIFOP(fd,26), NAME_LENGTH); remove_control_chars(account.passwd);
			account.passwdenc = 0;
			server_name = (char*)RFIFOP(fd,60); server_name[20] = '\0'; remove_control_chars(server_name);
			server_ip = ntohl(RFIFOL(fd, 54));
			server_port = ntohs(RFIFOW(fd, 58));

			ShowInfo("Connection request of the char-server '%s' @ %d.%d.%d.%d:%d (account: '%s', pass: '%s', ip: '%s')\n", server_name, CONVIP(server_ip), server_port, account.userid, account.passwd, ip);

			result = mmo_auth(&account, fd);
			if( result == -1 && account.sex == 2 && account.account_id < MAX_SERVERS && server[account.account_id].fd == -1 )
			{
				ShowStatus("Connection of the char-server '%s' accepted.\n", server_name);
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
				session[fd]->flag.server = 1;
				realloc_fifo(fd, FIFOSIZE_SERVERLINK, FIFOSIZE_SERVERLINK);

				send_GM_accounts(fd); // send GM account to char-server
			}
			else
			{
				ShowNotice("Connection of the char-server '%s' REFUSED.\n", server_name);
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
			return 0;

		case 0x7918:	// Request for administation login
			if ((int)RFIFOREST(fd) < 4 || (int)RFIFOREST(fd) < ((RFIFOW(fd,2) == 0) ? 28 : 20))
				return 0;
			WFIFOW(fd,0) = 0x7919;
			WFIFOB(fd,2) = 1;
			if (!check_ladminip(session[fd]->client_addr)) {
				ShowNotice("'ladmin'-login: Connection in administration mode refused: IP isn't authorised (ladmin_allow, ip: %s).\n", ip);
			} else {
				struct login_session_data *ld = (struct login_session_data*)session[fd]->session_data;
				if (RFIFOW(fd,2) == 0) {	// non encrypted password
					char password[25];
					memcpy(password, RFIFOP(fd,4), 24);
					password[24] = '\0';
					remove_control_chars(password);
					if( !admin_state )
						ShowNotice("'ladmin'-login: Connection in administration mode REFUSED - remote administration is disabled (non encrypted password: %s, ip: %s)\n", password, ip);
					else
					if( strcmp(password, admin_pass) != 0)
						ShowNotice("'ladmin'-login: Connection in administration mode REFUSED - invalid password (non encrypted password: %s, ip: %s)\n", password, ip);
					else {
						// If remote administration is enabled and password sent by client matches password read from login server configuration file
						ShowNotice("'ladmin'-login: Connection in administration mode accepted (non encrypted password: %s, ip: %s)\n", password, ip);
						WFIFOB(fd,2) = 0;
						session[fd]->func_parse = parse_admin;
					}
				} else {	// encrypted password
					if (!ld)
						ShowError("'ladmin'-login: error! MD5 key not created/requested for an administration login.\n");
					else {
						char md5str[64] = "", md5bin[32];
						if (RFIFOW(fd,2) == 1) {
							sprintf(md5str, "%s%s", ld->md5key, admin_pass); // 20 24
						} else if (RFIFOW(fd,2) == 2) {
							sprintf(md5str, "%s%s", admin_pass, ld->md5key); // 24 20
						}
						MD5_String2binary(md5str, md5bin);
						if( !admin_state )
							ShowNotice("'ladmin'-login: Connection in administration mode REFUSED - remote administration is disabled (encrypted password, ip: %s)\n", ip);
						else
						if( memcmp(md5bin, RFIFOP(fd,4), 16) != 0 )
							ShowNotice("'ladmin'-login: Connection in administration mode REFUSED - invalid password (encrypted password, ip: %s)\n", ip);
						else {
							// If remote administration is enabled and password hash sent by client matches hash of password read from login server configuration file
							ShowNotice("'ladmin'-login: Connection in administration mode accepted (encrypted password, ip: %s)\n", ip);
							ShowNotice("Connection of a remote administration accepted (encrypted password).\n");
							WFIFOB(fd,2) = 0;
							session[fd]->func_parse = parse_admin;
						}
					}
				}
			}
			WFIFOSET(fd,3);

			RFIFOSKIP(fd, (RFIFOW(fd,2) == 0) ? 28 : 20);
		break;

		default:
			if (save_unknown_packets) {
				FILE *logfp;
				char tmpstr[24];
				time_t raw_time;
				logfp = fopen(login_log_unknown_packets_filename, "a");
				if (logfp) {
					time(&raw_time);
					strftime(tmpstr, 23, login_config.date_format, localtime(&raw_time));
					fprintf(logfp, "%s: receiving of an unknown packet -> disconnection\n", tmpstr);
					fprintf(logfp, "parse_login: connection #%d (ip: %s), packet: 0x%x (with being read: %lu).\n", fd, ip, command, (unsigned long)RFIFOREST(fd));
					fprintf(logfp, "Detail (in hex):\n");
					fprintf(logfp, "---- 00-01-02-03-04-05-06-07  08-09-0A-0B-0C-0D-0E-0F\n");
					memset(tmpstr, '\0', sizeof(tmpstr));
					for(i = 0; i < RFIFOREST(fd); i++) {
						if ((i & 15) == 0)
							fprintf(logfp, "%04X ",i);
						fprintf(logfp, "%02x ", RFIFOB(fd,i));
						if (RFIFOB(fd,i) > 0x1f)
							tmpstr[i % 16] = RFIFOB(fd,i);
						else
							tmpstr[i % 16] = '.';
						if ((i - 7) % 16 == 0) // -8 + 1
							fprintf(logfp, " ");
						else if ((i + 1) % 16 == 0) {
							fprintf(logfp, " %s\n", tmpstr);
							memset(tmpstr, '\0', sizeof(tmpstr));
						}
					}
					if (i % 16 != 0) {
						for(j = i; j % 16 != 0; j++) {
							fprintf(logfp, "   ");
							if ((j - 7) % 16 == 0) // -8 + 1
								fprintf(logfp, " ");
						}
						fprintf(logfp, " %s\n", tmpstr);
					}
					fprintf(logfp, "\n");
					fclose(logfp);
				}
			}
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
		else if (strcmpi(w1, "admin_state") == 0) {
			admin_state = (bool)config_switch(w2);
		} else if (strcmpi(w1, "admin_pass") == 0) {
			memset(admin_pass, 0, sizeof(admin_pass));
			strncpy(admin_pass, w2, sizeof(admin_pass));
			admin_pass[sizeof(admin_pass)-1] = '\0';
		} else if (strcmpi(w1, "ladminallowip") == 0) {
			if (strcmpi(w2, "clear") == 0) {
				if (access_ladmin_allow)
					aFree(access_ladmin_allow);
				access_ladmin_allow = NULL;
				access_ladmin_allownum = 0;
			} else {
				if (strcmpi(w2, "all") == 0) {
					// reset all previous values
					if (access_ladmin_allow)
						aFree(access_ladmin_allow);
					// set to all
					access_ladmin_allow = (char*)aCalloc(ACO_STRSIZE, sizeof(char));
					access_ladmin_allownum = 1;
					access_ladmin_allow[0] = '\0';
				} else if (w2[0] && !(access_ladmin_allownum == 1 && access_ladmin_allow[0] == '\0')) { // don't add IP if already 'all'
					if (access_ladmin_allow)
						access_ladmin_allow = (char*)aRealloc(access_ladmin_allow, (access_ladmin_allownum+1) * ACO_STRSIZE);
					else
						access_ladmin_allow = (char*)aCalloc(ACO_STRSIZE, sizeof(char));
					strncpy(access_ladmin_allow + (access_ladmin_allownum++) * ACO_STRSIZE, w2, ACO_STRSIZE);
					access_ladmin_allow[access_ladmin_allownum * ACO_STRSIZE - 1] = '\0';
				}
			}
		} else if (strcmpi(w1, "gm_pass") == 0) {
			memset(gm_pass, 0, sizeof(gm_pass));
			strncpy(gm_pass, w2, sizeof(gm_pass));
			gm_pass[sizeof(gm_pass)-1] = '\0';
		} else if (strcmpi(w1, "level_new_gm") == 0) {
			level_new_gm = atoi(w2);
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
		else if (strcmpi(w1, "account_filename") == 0) {
			memset(account_filename, 0, sizeof(account_filename));
			strncpy(account_filename, w2, sizeof(account_filename));
			account_filename[sizeof(account_filename)-1] = '\0';
		} else if (strcmpi(w1, "gm_account_filename") == 0) {
			memset(GM_account_filename, 0, sizeof(GM_account_filename));
			strncpy(GM_account_filename, w2, sizeof(GM_account_filename));
			GM_account_filename[sizeof(GM_account_filename)-1] = '\0';
		} else if (strcmpi(w1, "gm_account_filename_check_timer") == 0) {
			gm_account_filename_check_timer = atoi(w2);
		} else if (strcmpi(w1, "log_login") == 0) {
			login_config.log_login = (bool)config_switch(w2);
		} else if (strcmpi(w1, "login_log_unknown_packets_filename") == 0) {
			memset(login_log_unknown_packets_filename, 0, sizeof(login_log_unknown_packets_filename));
			strncpy(login_log_unknown_packets_filename, w2, sizeof(login_log_unknown_packets_filename));
			login_log_unknown_packets_filename[sizeof(login_log_unknown_packets_filename)-1] = '\0';
		} else if (strcmpi(w1, "save_unknown_packets") == 0) {
			save_unknown_packets = config_switch(w2);
		} else if (strcmpi(w1, "start_limited_time") == 0) {
			start_limited_time = atoi(w2);
		} else if (strcmpi(w1, "order") == 0) {
			access_order = atoi(w2);
			if (strcmpi(w2, "deny,allow") == 0 ||
			    strcmpi(w2, "deny, allow") == 0) access_order = ACO_DENY_ALLOW;
			if (strcmpi(w2, "allow,deny") == 0 ||
			    strcmpi(w2, "allow, deny") == 0) access_order = ACO_ALLOW_DENY;
			if (strcmpi(w2, "mutual-failture") == 0 ||
			    strcmpi(w2, "mutual-failure") == 0) access_order = ACO_MUTUAL_FAILTURE;
		} else if (strcmpi(w1, "allow") == 0) {
			if (strcmpi(w2, "clear") == 0) {
				if (access_allow)
					aFree(access_allow);
				access_allow = NULL;
				access_allownum = 0;
			} else {
				if (strcmpi(w2, "all") == 0) {
					// reset all previous values
					if (access_allow)
						aFree(access_allow);
					// set to all
					access_allow = (char*)aCalloc(ACO_STRSIZE, sizeof(char));
					access_allownum = 1;
					access_allow[0] = '\0';
				} else if (w2[0] && !(access_allownum == 1 && access_allow[0] == '\0')) { // don't add IP if already 'all'
					if (access_allow)
						access_allow = (char*)aRealloc(access_allow, (access_allownum+1) * ACO_STRSIZE);
					else
						access_allow = (char*)aCalloc(ACO_STRSIZE, sizeof(char));
					strncpy(access_allow + (access_allownum++) * ACO_STRSIZE, w2, ACO_STRSIZE);
					access_allow[access_allownum * ACO_STRSIZE - 1] = '\0';
				}
			}
		} else if (strcmpi(w1, "deny") == 0) {
			if (strcmpi(w2, "clear") == 0) {
				if (access_deny)
					aFree(access_deny);
				access_deny = NULL;
				access_denynum = 0;
			} else {
				if (strcmpi(w2, "all") == 0) {
					// reset all previous values
					if (access_deny)
						aFree(access_deny);
					// set to all
					access_deny = (char*)aCalloc(ACO_STRSIZE, sizeof(char));
					access_denynum = 1;
					access_deny[0] = '\0';
				} else if (w2[0] && !(access_denynum == 1 && access_deny[0] == '\0')) { // don't add IP if already 'all'
					if (access_deny)
						access_deny = (char*)aRealloc(access_deny, (access_denynum+1) * ACO_STRSIZE);
					else
						access_deny = (char*)aCalloc(ACO_STRSIZE, sizeof(char));
					strncpy(access_deny + (access_denynum++) * ACO_STRSIZE, w2, ACO_STRSIZE);
					access_deny[access_denynum * ACO_STRSIZE - 1] = '\0';
				}
			}
		}

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
//		else if(!strcmpi(w1, "case_sensitive"))
//			login_config.case_sensitive = config_switch(w2);
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

//-------------------------------------
// Displaying of configuration warnings
//-------------------------------------
void display_conf_warnings(void)
{
	if( admin_state ) {
		if (admin_pass[0] == '\0') {
			ShowWarning("Administrator password is void (admin_pass).\n");
		} else if (strcmp(admin_pass, "admin") == 0) {
			ShowWarning("You are using the default administrator password (admin_pass).\n");
			ShowWarning("  We highly recommend that you change it.\n");
		}
	}

	if (gm_pass[0] == '\0') {
		ShowWarning("'To GM become' password is void (gm_pass).\n");
		ShowWarning("  We highly recommend that you set one password.\n");
	} else if (strcmp(gm_pass, "gm") == 0) {
		ShowWarning("You are using the default GM password (gm_pass).\n");
		ShowWarning("  We highly recommend that you change it.\n");
	}

	if (level_new_gm < 0 || level_new_gm > 99) {
		ShowWarning("Invalid value for level_new_gm parameter -> setting to 60 (default).\n");
		level_new_gm = 60;
	}

	if (gm_account_filename_check_timer < 0) {
		ShowWarning("Invalid value for gm_account_filename_check_timer parameter. Setting to 15 sec (default).\n");
		gm_account_filename_check_timer = 15;
	} else if (gm_account_filename_check_timer == 1) {
		ShowWarning("Invalid value for gm_account_filename_check_timer parameter. Setting to 2 sec (minimum value).\n");
		gm_account_filename_check_timer = 2;
	}

	if (login_config.min_level_to_connect < 0) { // 0: all players, 1-99 at least gm level x
		ShowWarning("Invalid value for min_level_to_connect (%d) parameter -> setting 0 (any player).\n", login_config.min_level_to_connect);
		login_config.min_level_to_connect = 0;
	} else if (login_config.min_level_to_connect > 99) { // 0: all players, 1-99 at least gm level x
		ShowWarning("Invalid value for min_level_to_connect (%d) parameter -> setting to 99 (only GM level 99)\n", login_config.min_level_to_connect);
		login_config.min_level_to_connect = 99;
	}

	if (start_limited_time < -1) { // -1: create unlimited account, 0 or more: additionnal sec from now to create limited time
		ShowWarning("Invalid value for start_limited_time parameter\n");
		ShowWarning("  -> setting to -1 (new accounts are created with unlimited time).\n");
		start_limited_time = -1;
	}

	if (access_order == ACO_DENY_ALLOW) {
		if (access_denynum == 1 && access_deny[0] == '\0') {
			ShowWarning("The IP security order is 'deny,allow' (allow if not deny) and you refuse ALL IP.\n");
		}
	} else if (access_order == ACO_ALLOW_DENY) {
		if (access_allownum == 0) {
			ShowWarning("The IP security order is 'allow,deny' (deny if not allow) but, NO IP IS AUTHORISED!\n");
		}
	} else { // ACO_MUTUAL_FAILTURE
		if (access_allownum == 0) {
			ShowWarning("The IP security order is 'mutual-failture'\n");
			ShowWarning("  (allow if in the allow list and not in the deny list).\n");
			ShowWarning("  But, NO IP IS AUTHORISED!\n");
		} else if (access_denynum == 1 && access_deny[0] == '\0') {
			ShowWarning("The IP security order is mutual-failture\n");
			ShowWarning("  (allow if in the allow list and not in the deny list).\n");
			ShowWarning("  But, you refuse ALL IP!\n");
		}
	}

	return;
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
//	login_config.case_sensitive = true;
	login_config.use_md5_passwds = false;
//	login_config.login_gm_read = true;
	login_config.min_level_to_connect = 0;
	login_config.online_check = true;
	login_config.check_client_version = false;
	login_config.client_version_to_connect = 20;

//	login_config.ipban = true;
//	login_config.dynamic_pass_failure_ban = true;
//	login_config.dynamic_pass_failure_ban_interval = 5;
//	login_config.dynamic_pass_failure_ban_limit = 7;
//	login_config.dynamic_pass_failure_ban_duration = 5;
	login_config.use_dnsbl = false;
	safestrncpy(login_config.dnsbl_servs, "", sizeof(login_config.dnsbl_servs));
}

//--------------------------------------
// Function called at exit of the server
//--------------------------------------
void do_final(void)
{
	int i, fd;
	ShowStatus("Terminating...\n");

	mmo_auth_sync();
	online_db->destroy(online_db, NULL);

	if(auth_dat) aFree(auth_dat);
	if(gm_account_db) aFree(gm_account_db);
	if(access_ladmin_allow) aFree(access_ladmin_allow);
	if(access_allow) aFree(access_allow);
	if(access_deny) aFree(access_deny);

	for (i = 0; i < MAX_SERVERS; i++) {
		if ((fd = server[i].fd) >= 0) {
			memset(&server[i], 0, sizeof(struct mmo_char_server));
			server[i].fd = -1;
			do_close(fd);
		}
	}
	do_close(login_fd);

	if(log_fp)
		fclose(log_fp);

	ShowStatus("Finished.\n");
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
	display_conf_warnings(); // not in login_config_read, because we can use 'import' option, and display same message twice or more
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
	mmo_auth_init();

	// Read account information.
	read_gm_account();

	// set default parser as parse_login function
	set_defaultparse(parse_login);

	add_timer_func_list(check_auth_sync, "check_auth_sync");
	add_timer_interval(gettick() + 60000, check_auth_sync, 0, 0, 60000); // every 60 sec we check if we must save accounts file (only if necessary to save)

	// every x sec we check if gm file has been changed
	if( gm_account_filename_check_timer ) {
		add_timer_func_list(check_GM_file, "check_GM_file");
		add_timer_interval(gettick() + gm_account_filename_check_timer * 1000, check_GM_file, 0, 0, gm_account_filename_check_timer * 1000); 
	}

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
