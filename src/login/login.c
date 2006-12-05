// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

// new version of the login-server by [Yor]

#include <sys/types.h>
#ifdef __WIN32
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h> // for stat/lstat/fstat
#include <signal.h>
#include <fcntl.h>
#include <string.h>
#include <stdarg.h>

#include "../common/core.h"
#include "../common/socket.h"
#include "../common/timer.h"
#include "../common/mmo.h"
#include "../common/showmsg.h"
#include "../common/version.h"
#include "../common/db.h"
#include "../common/lock.h"
#include "../common/malloc.h"
#include "../common/strlib.h"
#include "../common/showmsg.h"
#include "login.h"

#ifdef PASSWORDENC
#include "md5calc.h"
#endif

int account_id_count = START_ACCOUNT_NUM;
int server_num;
int new_account_flag = 0;
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

int use_dnsbl=0; // [Zido]
char dnsbl_servs[1024]; // [Zido]

char account_filename[1024] = "save/account.txt";
char GM_account_filename[1024] = "conf/GM_account.txt";
char login_log_filename[1024] = "log/login.log";
FILE *log_fp = NULL;
char login_log_unknown_packets_filename[1024] = "log/login_unknown_packets.log";
char date_format[32] = "%Y-%m-%d %H:%M:%S";
int save_unknown_packets = 0;
long creation_time_GM_account_file;
int gm_account_filename_check_timer = 15; // Timer to check if GM_account file has been changed and reload GM account automaticaly (in seconds; default: 15)

int log_login = 1;

int display_parse_login = 0; // 0: no, 1: yes
int display_parse_admin = 0; // 0: no, 1: yes
int display_parse_fromchar = 0; // 0: no, 1: yes (without packet 0x2714), 2: all packets

struct mmo_char_server server[MAX_SERVERS];
int server_fd[MAX_SERVERS];

int login_fd;

static int online_check=1; //When set to 1, login server rejects incoming players that are already registered as online. [Skotlex]
//Account flood protection [Kevin]
unsigned int new_reg_tick=0;
int allowed_regs=1;
int num_regs=0;
int time_allowed=10; //Init this to 10 seconds. [Skotlex]

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

int min_level_to_connect = 0; // minimum level of player/GM (0: player, 1-99: gm) to connect on the server
int add_to_unlimited_account = 0; // Give possibility or not to adjust (ladmin command: timeadd) the time of an unlimited account.
int start_limited_time = -1; // Starting additional sec from now for the limited time at creation of accounts (-1: unlimited time, 0 or more: additional sec from now)
int check_ip_flag = 1; // It's to check IP of a player between login-server and char-server (part of anti-hacking system)

int check_client_version = 0; //Client version check ON/OFF .. (sirius)
int client_version_to_connect = 20; //Client version needed to connect ..(sirius)
static int ip_sync_interval = 0;


struct login_session_data {
	unsigned int md5keylen;
	char md5key[20];
};

#define AUTH_FIFO_SIZE 256
struct {
	int account_id, login_id1, login_id2;
	int ip, sex, delflag;
} auth_fifo[AUTH_FIFO_SIZE];
int auth_fifo_pos = 0;

struct online_login_data {
	int account_id;
	short char_server;
	short waiting_disconnect;
};

struct auth_dat {
	int account_id, sex;
	char userid[24], pass[33], lastlogin[24]; // 33 for 32 + NULL terminated
	int logincount;
	int state; // packet 0x006a value + 1 (0: compte OK)
	char email[40]; // e-mail (by default: a@a.com)
	char error_message[20]; // Message of error code #6 = Your are Prohibited to log in until %s (packet 0x006a)
	time_t ban_until_time; // # of seconds 1/1/1970 (timestamp): ban time limit of the account (0 = no ban)
	time_t connect_until_time; // # of seconds 1/1/1970 (timestamp): Validity limit of the account (0 = unlimited)
	char last_ip[16]; // save of last IP of connection
	char memo[255]; // a memo field
	int account_reg2_num;
	struct global_reg account_reg2[ACCOUNT_REG2_NUM];
} *auth_dat = NULL;

unsigned int auth_num = 0, auth_max = 0;

// define the number of times that some players must authentify them before to save account file.
// it's just about normal authentification. If an account is created or modified, save is immediatly done.
// An authentification just change last connected IP and date. It already save in log file.
// set minimum auth change before save:
#define AUTH_BEFORE_SAVE_FILE 10
// set divider of auth_num to found number of change before save
#define AUTH_SAVE_FILE_DIVIDER 50
int auth_before_save_file = 0; // Counter. First save when 1st char-server do connection.

int admin_state = 0;
char admin_pass[24] = "";
unsigned int GM_num;
unsigned int GM_max=256;
char gm_pass[64] = "";
int level_new_gm = 60;

struct gm_account *gm_account_db;

static struct dbt *online_db;

int dynamic_pass_failure_ban = 1;
int dynamic_pass_failure_ban_time = 5;
int dynamic_pass_failure_ban_how_many = 3;
int dynamic_pass_failure_ban_how_long = 1;

int use_md5_passwds = 0;

int console = 0;

int charif_sendallwos(int sfd, unsigned char *buf, unsigned int len);

//------------------------------
// Writing function of logs file
//------------------------------
int login_log(char *fmt, ...) {
	if (log_login) {
		va_list ap;
		time_t raw_time;
		char tmpstr[2048];

		if(!log_fp)
			log_fp = fopen(login_log_filename, "a");

		if (log_fp) {
			if (fmt[0] == '\0') // jump a line if no message
				fprintf(log_fp, RETCODE);
			else {
				va_start(ap, fmt);
				// Platform/Compiler dependant clock() for time check is removed. [Lance]
				// clock() is originally used to track processing ticks on program execution.
				time(&raw_time);
				strftime(tmpstr, 24, date_format, localtime(&raw_time));
				sprintf(tmpstr + strlen(tmpstr), ": %s", fmt);
				vfprintf(log_fp, tmpstr, ap);
				va_end(ap);
			}
			fflush(log_fp); // under cygwin or windows, if software is stopped, data are not written in the file -> fflush at every line
		}
	}

	return 0;
}

static void* create_online_user(DBKey key, va_list args) {
	struct online_login_data *p;
	p = aCalloc(1, sizeof(struct online_login_data));
	p->account_id = key.i;
	p->char_server = -1;
	return p;	
}
//-----------------------------------------------------
// Online User Database [Wizputer]
//-----------------------------------------------------

void add_online_user (int char_server, int account_id) {
	struct online_login_data *p;
	if (!online_check)
		return;
	p = idb_ensure(online_db, account_id, create_online_user);
	p->char_server = char_server;
	p->waiting_disconnect = 0;
}
int is_user_online (int account_id) {
	return (idb_get(online_db, account_id) != NULL);
}
void remove_online_user (int account_id) {
	if(!online_check)
		return;
	if (account_id == 99) {	// reset all to offline
		online_db->clear(online_db, NULL);	// purge db
		return;
	}
	idb_remove(online_db,account_id);
}

int waiting_disconnect_timer(int tid, unsigned int tick, int id, int data)
{
	struct online_login_data *p;
	if ((p= idb_get(online_db, id)) != NULL && p->waiting_disconnect)
		remove_online_user(p->account_id);
	return 0;
}

static int sync_ip_addresses(int tid, unsigned int tick, int id, int data){
	unsigned char buf[2];
	ShowInfo("IP Sync in progress...\n");
	WBUFW(buf,0) = 0x2735;
	charif_sendallwos(-1, buf, 2);
	return 0;
}

//----------------------------------------------------------------------
// Determine if an account (id) is a GM account
// and returns its level (or 0 if it isn't a GM account or if not found)
//----------------------------------------------------------------------
int isGM(int account_id) {
	unsigned int i;
	for(i=0; i < GM_num; i++)
		if(gm_account_db[i].account_id == account_id)
			return gm_account_db[i].level;
	return 0;
}

//----------------------------------------------------------------------
// Adds a new GM using acc id and level
//----------------------------------------------------------------------
void addGM(int account_id, int level) {
	unsigned int i;
	int do_add = 0;
	for(i = 0; i < auth_num; i++) {
		if (auth_dat[i].account_id==account_id) {
			do_add = 1;
			break;
		}
	}
	for(i = 0; i < GM_num; i++)
		if (gm_account_db[i].account_id == account_id) {
			if (gm_account_db[i].level == level)
				ShowWarning("addGM: GM account %d defined twice (same level: %d).\n", account_id, level);
			else {
				ShowWarning("addGM: GM account %d defined twice (levels: %d and %d).\n", account_id, gm_account_db[i].level, level);
				gm_account_db[i].level = level;
			}
			return;
		}

	// if new account
	if (i == GM_num && do_add) {
		if (GM_num >= GM_max) {
			GM_max += 256;
			gm_account_db = (struct gm_account*)aRealloc(gm_account_db, sizeof(struct gm_account) * GM_max);
			memset(gm_account_db + (GM_max - 256), 0, sizeof(struct gm_account) * 256);
		}
		gm_account_db[GM_num].account_id = account_id;
		gm_account_db[GM_num].level = level;
		GM_num++;
		if (GM_num >= 4000) {
			ShowWarning("4000 GM accounts found. Next GM accounts are not read.\n");
			login_log("***WARNING: 4000 GM accounts found. Next GM accounts are not read." RETCODE);
		}
	}
}

//-------------------------------------------------------
// Reading function of GM accounts file (and their level)
//-------------------------------------------------------
int read_gm_account(void) {
	char line[512];
	FILE *fp;
	int account_id, level;
	int line_counter;
	struct stat file_stat;
	int start_range = 0, end_range = 0, is_range = 0, current_id = 0;

	if(gm_account_db) aFree(gm_account_db);
	GM_num = 0;
	if(GM_max < 0)	GM_max = 256;
	gm_account_db = (struct gm_account*)aCalloc(GM_max, sizeof(struct gm_account));

	// get last modify time/date
	if (stat(GM_account_filename, &file_stat))
		creation_time_GM_account_file = 0; // error
	else
		creation_time_GM_account_file = (long)file_stat.st_mtime;

	if ((fp = fopen(GM_account_filename, "r")) == NULL) {
		ShowError("read_gm_account: GM accounts file [%s] not found.\n", GM_account_filename);
		ShowError("                 Actually, there is no GM accounts on the server.\n");
		login_log("read_gm_account: GM accounts file [%s] not found." RETCODE, GM_account_filename);
		login_log("                 Actually, there is no GM accounts on the server." RETCODE);
		return 1;
	}

	line_counter = 0;
	// limited to 4000, because we send information to char-servers (more than 4000 GM accounts???)
	// int (id) + int (level) = 8 bytes * 4000 = 32k (limit of packets in windows)
	while(fgets(line, sizeof(line)-1, fp) && GM_num < 4000) {
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
	login_log("read_gm_account: file '%s' read (%d GM accounts found)." RETCODE, GM_account_filename, GM_num);

	return 0;
}

//--------------------------------------------------------------
// Test of the IP mask
// (ip: IP to be tested, str: mask x.x.x.x/# or x.x.x.x/y.y.y.y)
//--------------------------------------------------------------
int check_ipmask(unsigned int ip, const unsigned char *str) {
	unsigned int mask = 0, i = 0, m, ip2, a0, a1, a2, a3;
	unsigned char *p = (unsigned char *)&ip2, *p2 = (unsigned char *)&mask;

	if (sscanf((const char*)str, "%d.%d.%d.%d/%n", &a0, &a1, &a2, &a3, &i) != 4 || i == 0)
		return 0;
	p[0] = a0; p[1] = a1; p[2] = a2; p[3] = a3;

	if (sscanf((const char*)str+i, "%d.%d.%d.%d", &a0, &a1, &a2, &a3) == 4) {
		p2[0] = a0; p2[1] = a1; p2[2] = a2; p2[3] = a3;
		mask = ntohl(mask);
	} else if (sscanf((const char*)(str+i), "%d", &m) == 1 && m >= 0 && m <= 32) {
		for(i = 0; i < m && i < 32; i++)
			mask = (mask >> 1) | 0x80000000;
	} else {
		ShowError("check_ipmask: invalid mask [%s].\n", str);
		return 0;
	}

//	printf("Tested IP: %08x, network: %08x, network mask: %08x\n",
//	       (unsigned int)ntohl(ip), (unsigned int)ntohl(ip2), (unsigned int)mask);
	return ((ntohl(ip) & mask) == (ntohl(ip2) & mask));
}

//---------------------
// Access control by IP
//---------------------
int check_ip(unsigned int ip) {
	int i;
	unsigned char *p = (unsigned char *)&ip;
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
	sprintf(buf, "%d.%d.%d.%d.", p[0], p[1], p[2], p[3]);

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
int check_ladminip(unsigned int ip) {
	int i;
	unsigned char *p = (unsigned char *)&ip;
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
	sprintf(buf, "%d.%d.%d.%d.", p[0], p[1], p[2], p[3]);

	for(i = 0; i < access_ladmin_allownum; i++) {
		access_ip = access_ladmin_allow + i * ACO_STRSIZE;
		if (memcmp(access_ip, buf, strlen(access_ip)) == 0 || check_ipmask(ip, (unsigned char*)access_ip)) {
			return 1;
		}
	}

	return 0;
}

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

	for(ch = 1; ch < 32; ch++)
		if (strchr(last_arobas, ch) != NULL)
			return 0;

	if (strchr(last_arobas, ' ') != NULL ||
	    strchr(last_arobas, ';') != NULL)
		return 0;

	// all correct
	return 1;
}

//-----------------------------------------------
// Search an account id
//   (return account index or -1 (if not found))
//   If exact account name is not found,
//   the function checks without case sensitive
//   and returns index if only 1 account is found
//   and similar to the searched name.
//-----------------------------------------------
int search_account_index(char* account_name) {
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
int mmo_auth_tostr(char *str, struct auth_dat *p) {
	int i;
	char *str_p = str;

	str_p += sprintf(str_p, "%d\t%s\t%s\t%s\t%c\t%d\t%d\t"
	                 "%s\t%s\t%ld\t%s\t%s\t%ld\t",
	                 p->account_id, p->userid, p->pass, p->lastlogin,
	                 (p->sex == 2) ? 'S' : (p->sex ? 'M' : 'F'),
	                 p->logincount, p->state,
	                 p->email, p->error_message,
	                 (long)p->connect_until_time, p->last_ip, p->memo, (long)p->ban_until_time);

	for(i = 0; i < p->account_reg2_num; i++)
		if (p->account_reg2[i].str[0])
			str_p += sprintf(str_p, "%s,%s ", p->account_reg2[i].str, p->account_reg2[i].value);

	return 0;
}

//---------------------------------
// Reading of the accounts database
//---------------------------------
int mmo_auth_init(void) {
	FILE *fp;
	int account_id, logincount, state, n, i;
	unsigned int j;
	char line[2048], *p, userid[2048], pass[2048], lastlogin[2048], sex, email[2048], error_message[2048], last_ip[2048], memo[2048];
	long ban_until_time;
	long connect_until_time;
	char str[2048];
	char v[2048];
	int GM_count = 0;
	int server_count = 0;

	auth_max = 256;
	auth_dat = (struct auth_dat*)aCalloc(auth_max, sizeof(struct auth_dat));

	if ((fp = fopen(account_filename, "r")) == NULL) {
		// no account file -> no account -> no login, including char-server (ERROR)
		ShowError(CL_RED"mmmo_auth_init: Accounts file [%s] not found."CL_RESET"\n", account_filename);
		return 0;
	}

	while(fgets(line, sizeof(line)-1, fp) != NULL) {
		if (line[0] == '/' && line[1] == '/')
			continue;
		line[sizeof(line)-1] = '\0';
		// remove carriage return if exist
		while(line[0] != '\0' && (line[strlen(line)-1] == '\n' || line[strlen(line)-1] == '\r'))
			line[strlen(line)-1] = '\0';
		p = line;

		memset(userid, 0, sizeof(userid));
		memset(pass, 0, sizeof(pass));
		memset(lastlogin, 0, sizeof(lastlogin));
		memset(email, 0, sizeof(email));
		memset(error_message, 0, sizeof(error_message));
		memset(last_ip, 0, sizeof(last_ip));
		memset(memo, 0, sizeof(memo));

		// database version reading (v2)
		if (((i = sscanf(line, "%d\t%[^\t]\t%[^\t]\t%[^\t]\t%c\t%d\t%d\t"
		                 "%[^\t]\t%[^\t]\t%ld\t%[^\t]\t%[^\t]\t%ld%n",
		    &account_id, userid, pass, lastlogin, &sex, &logincount, &state,
		    email, error_message, &connect_until_time, last_ip, memo, &ban_until_time, &n)) == 13 && line[n] == '\t') ||
		    ((i = sscanf(line, "%d\t%[^\t]\t%[^\t]\t%[^\t]\t%c\t%d\t%d\t"
		                 "%[^\t]\t%[^\t]\t%ld\t%[^\t]\t%[^\t]%n",
		    &account_id, userid, pass, lastlogin, &sex, &logincount, &state,
		    email, error_message, &connect_until_time, last_ip, memo, &n)) == 12 && line[n] == '\t')) {
			n = n + 1;

			// Some checks
			if (account_id > END_ACCOUNT_NUM) {
				ShowError(CL_RED"mmmo_auth_init: an account has an id higher than %d\n", END_ACCOUNT_NUM);
				ShowError("               account id #%d -> account not read (saved in log file)."CL_RESET"\n", account_id);
				login_log("mmmo_auth_init: ******Error: an account has an id higher than %d." RETCODE, END_ACCOUNT_NUM);
				login_log("               account id #%d -> account not read (saved in next line):" RETCODE, account_id);
				login_log("%s", line);
				continue;
			}
			userid[23] = '\0';
			remove_control_chars((unsigned char *)userid);
			for(j = 0; j < auth_num; j++) {
				if (auth_dat[j].account_id == account_id) {
					ShowError(CL_RED"mmmo_auth_init: an account has an identical id to another.\n");
					ShowError("               account id #%d -> new account not read (saved in log file)."CL_RED"\n", account_id);
					login_log("mmmo_auth_init: ******Error: an account has an identical id to another." RETCODE);
					login_log("               account id #%d -> new account not read (saved in next line):" RETCODE, account_id);
					login_log("%s", line);
					break;
				} else if (strcmp(auth_dat[j].userid, userid) == 0) {
					ShowError(CL_RED"mmmo_auth_init: account name already exists.\n");
					ShowError("               account name '%s' -> new account not read (saved in log file)."CL_RESET"\n", userid); // 2 lines, account name can be long.
					login_log("mmmo_auth_init: ******Error: an account has an identical name to another." RETCODE);
					login_log("               account name '%s' -> new account not read (saved in next line):" RETCODE, userid);
					login_log("%s", line);
					break;
				}
			}
			if (j != auth_num)
				continue;

			if (auth_num >= auth_max) {
				auth_max += 256;
				auth_dat = (struct auth_dat*)aRealloc(auth_dat, sizeof(struct auth_dat) * auth_max);
			}

			memset(&auth_dat[auth_num], '\0', sizeof(struct auth_dat));

			auth_dat[auth_num].account_id = account_id;

			strncpy(auth_dat[auth_num].userid, userid, 24);

			pass[23] = '\0';
			remove_control_chars((unsigned char *)pass);
			strncpy(auth_dat[auth_num].pass, pass, 24);

			lastlogin[23] = '\0';
			remove_control_chars((unsigned char *)lastlogin);
			strncpy(auth_dat[auth_num].lastlogin, lastlogin, 24);

			auth_dat[auth_num].sex = (sex == 'S' || sex == 's') ? 2 : (sex == 'M' || sex == 'm');

			if (logincount >= 0)
				auth_dat[auth_num].logincount = logincount;
			else
				auth_dat[auth_num].logincount = 0;

			if (state > 255)
				auth_dat[auth_num].state = 100;
			else if (state < 0)
				auth_dat[auth_num].state = 0;
			else
				auth_dat[auth_num].state = state;

			if (e_mail_check(email) == 0) {
				ShowNotice("Account %s (%d): invalid e-mail (replaced par a@a.com).\n", auth_dat[auth_num].userid, auth_dat[auth_num].account_id);
				strncpy(auth_dat[auth_num].email, "a@a.com", 40);
			} else {
				remove_control_chars((unsigned char *)email);
				strncpy(auth_dat[auth_num].email, email, 40);
			}

			error_message[19] = '\0';
			remove_control_chars((unsigned char *)error_message);
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
			remove_control_chars((unsigned char *)last_ip);
			strncpy(auth_dat[auth_num].last_ip, last_ip, 16);

			memo[254] = '\0';
			remove_control_chars((unsigned char *)memo);
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
				remove_control_chars((unsigned char *)str);
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
		} else if ((i = sscanf(line, "%d\t%[^\t]\t%[^\t]\t%[^\t]\t%c\t%d\t%d\t%n",
		           &account_id, userid, pass, lastlogin, &sex, &logincount, &state, &n)) >= 5) {
			if (account_id > END_ACCOUNT_NUM) {
				ShowError(CL_RED"mmmo_auth_init: an account has an id higher than %d\n", END_ACCOUNT_NUM);
				ShowError("               account id #%d -> account not read (saved in log file)."CL_RESET"\n", account_id);
				login_log("mmmo_auth_init: ******Error: an account has an id higher than %d." RETCODE, END_ACCOUNT_NUM);
				login_log("               account id #%d -> account not read (saved in next line):" RETCODE, account_id);
				login_log("%s", line);
				continue;
			}
			userid[23] = '\0';
			remove_control_chars((unsigned char *)userid);
			for(j = 0; j < auth_num; j++) {
				if (auth_dat[j].account_id == account_id) {
					ShowError(CL_RED"mmo_auth_init: an account has an identical id to another.\n");
					ShowError("               account id #%d -> new account not read (saved in log file)."CL_RESET"\n", account_id);
					login_log("mmmo_auth_init: ******Error: an account has an identical id to another." RETCODE);
					login_log("               account id #%d -> new account not read (saved in next line):" RETCODE, account_id);
					login_log("%s", line);
					break;
				} else if (strcmp(auth_dat[j].userid, userid) == 0) {
					ShowError(CL_RED"mmo_auth_init: account name already exists.\n");
					ShowError("               account name '%s' -> new account not read (saved in log file)."CL_RESET"\n", userid);
					login_log("mmmo_auth_init: ******Error: an account has an identical id to another." RETCODE);
					login_log("               account id #%d -> new account not read (saved in next line):" RETCODE, account_id);
					login_log("%s", line);
					break;
				}
			}
			if (j != auth_num)
				continue;

			if (auth_num >= auth_max) {
				auth_max += 256;
				auth_dat = (struct auth_dat*)aRealloc(auth_dat, sizeof(struct auth_dat) * auth_max);
			}

			memset(&auth_dat[auth_num], '\0', sizeof(struct auth_dat));

			auth_dat[auth_num].account_id = account_id;

			strncpy(auth_dat[auth_num].userid, userid, 24);

			pass[23] = '\0';
			remove_control_chars((unsigned char *)pass);
			strncpy(auth_dat[auth_num].pass, pass, 24);

			lastlogin[23] = '\0';
			remove_control_chars((unsigned char *)lastlogin);
			strncpy(auth_dat[auth_num].lastlogin, lastlogin, 24);

			auth_dat[auth_num].sex = (sex == 'S' || sex == 's') ? 2 : (sex == 'M' || sex == 'm');

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
				else if (state < 0)
					auth_dat[auth_num].state = 0;
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
				remove_control_chars((unsigned char *)str);
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
			i = 0;
			if (sscanf(line, "%d\t%%newid%%\n%n", &account_id, &i) == 1 &&
			    i > 0 && account_id > account_id_count)
				account_id_count = account_id;
		}
	}
	fclose(fp);

	if (auth_num == 0) {
		ShowNotice("mmo_auth_init: No account found in %s.\n", account_filename);
		sprintf(line, "No account found in %s.", account_filename);
	} else {
		if (auth_num == 1) {
			ShowStatus("mmo_auth_init: 1 account read in %s,\n", account_filename);
			sprintf(line, "1 account read in %s,", account_filename);
		} else {
			ShowStatus("mmo_auth_init: %d accounts read in %s,\n", auth_num, account_filename);
			sprintf(line, "%d accounts read in %s,", auth_num, account_filename);
		}
		if (GM_count == 0) {
			ShowStatus("               of which is no GM account, and ");
			sprintf(str, "%s of which is no GM account and", line);
		} else if (GM_count == 1) {
			ShowStatus("               of which is 1 GM account, and ");
			sprintf(str, "%s of which is 1 GM account and", line);
		} else {
			ShowStatus("               of which is %d GM accounts, and ", GM_count);
			sprintf(str, "%s of which is %d GM accounts and", line, GM_count);
		}
		if (server_count == 0) {
			printf("no server account ('S').\n");
			sprintf(line, "%s no server account ('S').", str);
		} else if (server_count == 1) {
			printf("1 server account ('S').\n");
			sprintf(line, "%s 1 server account ('S').", str);
		} else {
			printf("%d server accounts ('S').\n", server_count);
			sprintf(line, "%s %d server accounts ('S').", str, server_count);
		}
	}
	login_log("%s" RETCODE, line);

	return 0;
}

//------------------------------------------
// Writing of the accounts database file
//   (accounts are sorted by id before save)
//------------------------------------------
void mmo_auth_sync(void) {
	FILE *fp;
	unsigned int i, j, k;
	int lock;
	int account_id;
	//int id[auth_num];
	//int *id = (int *)aCalloc(auth_num, sizeof(int));
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
		if (auth_dat[k].account_id < 0)
			continue;

		mmo_auth_tostr(line, &auth_dat[k]);
		fprintf(fp, "%s" RETCODE, line);
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
//   have do some authentifications without arrive to
//   the minimum of authentifications for the save.
// Note: all other modification of accounts (deletion,
//       change of some informations excepted lastip/
//       lastlogintime, creation) are always save
//       immediatly and set  the minimum of
//       authentifications to its initialization value.
//-----------------------------------------------------
int check_auth_sync(int tid, unsigned int tick, int id, int data) {
	// we only save if necessary:
	// we have do some authentifications without do saving
	if (auth_before_save_file < AUTH_BEFORE_SAVE_FILE ||
	    auth_before_save_file < (int)(auth_num / AUTH_SAVE_FILE_DIVIDER))
		mmo_auth_sync();

	return 0;
}

//--------------------------------------------------------------------
// Packet send to all char-servers, except one (wos: without our self)
//--------------------------------------------------------------------
int charif_sendallwos(int sfd, unsigned char *buf, unsigned int len) {
	int i, c, fd;

	for(i = 0, c = 0; i < MAX_SERVERS; i++) {
		if ((fd = server_fd[i]) >= 0 && fd != sfd) {
         WFIFOHEAD(fd, len);
			if (WFIFOSPACE(fd) < len) //Increase buffer size.
				realloc_writefifo(fd, len);
			memcpy(WFIFOP(fd,0), buf, len);
			WFIFOSET(fd, len);
			c++;
		}
	}
	return c;
}

//-----------------------------------------------------
// Send GM accounts to all char-server
//-----------------------------------------------------
void send_GM_accounts(void) {
	unsigned int i;
	unsigned char buf[32767];
	int len;

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
	charif_sendallwos(-1, buf, len);

	return;
}

//-----------------------------------------------------
// Check if GM file account have been changed
//-----------------------------------------------------
int check_GM_file(int tid, unsigned int tick, int id, int data) {
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
		send_GM_accounts();
	}

	return 0;
}

//-------------------------------------
// Account creation (with e-mail check)
//-------------------------------------
int mmo_auth_new(struct mmo_account* account, char sex, char* email) {
	time_t timestamp, timestamp_temp;
	struct tm *tmtime;
	int i = auth_num;

	if (auth_num >= auth_max) {
		auth_max += 256;
		auth_dat = (struct auth_dat*)aRealloc(auth_dat, sizeof(struct auth_dat) * auth_max);
	}

	memset(&auth_dat[i], '\0', sizeof(struct auth_dat));

	while (isGM(account_id_count) > 0)
		account_id_count++;

	auth_dat[i].account_id = account_id_count++;

	strncpy(auth_dat[i].userid, account->userid, NAME_LENGTH);
	auth_dat[i].userid[23] = '\0';

	strncpy(auth_dat[i].pass, account->passwd, NAME_LENGTH);
	auth_dat[i].pass[23] = '\0';

	memcpy(auth_dat[i].lastlogin, "-", 2);

	auth_dat[i].sex = (sex == 'M' || sex == 'm');

	auth_dat[i].logincount = 0;

	auth_dat[i].state = 0;

	if (e_mail_check(email) == 0)
		strncpy(auth_dat[i].email, "a@a.com", 40);
	else
		strncpy(auth_dat[i].email, email, 40);

	strncpy(auth_dat[i].error_message, "-", 20);

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

//---------------------------------------
// Check/authentification of a connection
//---------------------------------------
int mmo_auth(struct mmo_account* account, int fd) {
	char *dnsbl_serv;
	unsigned int i;
	time_t raw_time;
	char tmpstr[256];
	int len, newaccount = 0;
#ifdef PASSWORDENC
	struct login_session_data *ld;
#endif
	int encpasswdok;
	char md5str[64], md5bin[32];
	char ip[16];
	unsigned char *sin_addr = (unsigned char *)&session[fd]->client_addr.sin_addr;
	char user_password[256];
	char r_ip[16]; // [Zido]
	char ip_dnsbl[256]; // [Zido]

	sprintf(ip, "%d.%d.%d.%d", sin_addr[0], sin_addr[1], sin_addr[2], sin_addr[3]);

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

		while((dnsbl_serv=strtok(dnsbl_servs,","))) {
			sprintf(ip_dnsbl,"%s.%s",r_ip,dnsbl_serv);
// Using directly gethostbyname should be quicker. [Skotlex]
//			if(resolve_hostbyname(ip_dnsbl,NULL,NULL)!=0) {
			if(gethostbyname(ip_dnsbl)) {
				ShowInfo("DNSBL: (%s) Blacklisted. User Kicked.\n",ip);
				return 3;
			}
		}

	}
	// End DNS Blacklist check [Zido]


	len = strlen(account->userid) - 2;
	// Account creation with _M/_F
	if (account->passwdenc == 0 && account->userid[len] == '_' &&
		(account->userid[len+1] == 'F' || account->userid[len+1] == 'M' ||
		account->userid[len+1] == 'f' || account->userid[len+1] == 'm')
		&& new_account_flag && account_id_count <= END_ACCOUNT_NUM && len >= 4 && strlen(account->passwd) >= 4) {
						
		//only continue if amount in this time limit is allowed (account registration flood protection)[Kevin]
		if(gettick() <= new_reg_tick && num_regs >= allowed_regs) {
			ShowNotice("Account registration denied (registration limit exceeded) to %s!\n", ip);
			login_log("Notice: Account registration denied (registration limit exceeded) to %s!", ip);
			return 3;
		} else {
			num_regs=0;
		}
		
		newaccount = 1;
		account->userid[len] = '\0';
	}
	
	//EXE Version check [Sirius]
	if (check_client_version == 1 && account->version != 0 &&
		account->version != client_version_to_connect)
		return 5;
	
	// Strict account search
	for(i = 0; i < auth_num; i++) {
		if (strcmp(account->userid, auth_dat[i].userid) == 0)
			break;
	}
	// if there is no creation request and strict account search fails, we do a no sensitive case research for index
	if (!newaccount && i == auth_num) {
		i = search_account_index(account->userid);
		if (i == -1)
			i = auth_num;
		else
			memcpy(account->userid, auth_dat[i].userid, NAME_LENGTH); // for the possible tests/checks afterwards (copy correcte sensitive case).
	}

	if (i != auth_num) {
		if (newaccount) {
			login_log("Attempt of creation of an already existant account (account: %s_%c, pass: %s, received pass: %s, ip: %s)" RETCODE,
			          account->userid, account->userid[len+1], auth_dat[i].pass, account->passwd, ip);
			return 1; // 1 = Incorrect Password
		}
		if(use_md5_passwds)
			MD5_String(account->passwd, user_password);
		else
			memcpy(user_password, account->passwd, NAME_LENGTH);
		encpasswdok = 0;
#ifdef PASSWORDENC
		ld = (struct login_session_data*)session[fd]->session_data;
		if (account->passwdenc > 0) {
			int j = account->passwdenc;
			if (!ld) {
				login_log("Md5 key not created (account: %s, ip: %s)" RETCODE, account->userid, ip);
				return 1; // 1 = Incorrect Password
			}
			if (j > 2)
				j = 1;
			do {
				if (j == 1) {
					sprintf(md5str, "%s%s", ld->md5key, auth_dat[i].pass); // 20 + 24
				} else if (j == 2) {
					sprintf(md5str, "%s%s", auth_dat[i].pass, ld->md5key); // 24 + 20
				} else
					md5str[0] = '\0';
				md5str[sizeof(md5str)-1] = '\0'; // 64
				MD5_String2binary(md5str, md5bin);
				encpasswdok = (memcmp(account->passwd, md5bin, 16) == 0);
			} while (j < 2 && !encpasswdok && (j++) != account->passwdenc);
//			printf("key[%s] md5 [%s] ", md5key, md5);
//			printf("client [%s] accountpass [%s]\n", account->passwd, auth_dat[i].pass);
		}
#endif
		if ((strcmp(account->passwd, auth_dat[i].pass) && !encpasswdok)) {
			if (account->passwdenc == 0)
				login_log("Invalid password (account: %s, pass: %s, received pass: %s, ip: %s)" RETCODE, account->userid, auth_dat[i].pass, account->passwd, ip);
#ifdef PASSWORDENC
			else {
				char logbuf[512], *p = logbuf;
				unsigned int j;
				p += sprintf(p, "Invalid password (account: %s, received md5[", account->userid);
				for(j = 0; j < 16; j++)
					p += sprintf(p, "%02x", ((unsigned char *)account->passwd)[j]);
				p += sprintf(p,"] calculated md5[");
				for(j = 0; j < 16; j++)
					p += sprintf(p, "%02x", ((unsigned char *)md5bin)[j]);
				p += sprintf(p, "] md5 key[");
				for(j = 0; j < ld->md5keylen; j++)
					p += sprintf(p, "%02x", ((unsigned char *)ld->md5key)[j]);
				p += sprintf(p, "], ip: %s)" RETCODE, ip);
				login_log(logbuf);
			}
#endif
			return 1; // 1 = Incorrect Password
		}

		if (auth_dat[i].state) {
			login_log("Connection refused (account: %s, pass: %s, state: %d, ip: %s)" RETCODE,
			          account->userid, account->passwd, auth_dat[i].state, ip);
			switch(auth_dat[i].state) { // packet 0x006a value + 1
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
				return auth_dat[i].state - 1;
			default:
				return 99; // 99 = ID has been totally erased
			}
		}

		if (online_check) {
			unsigned char buf[8];
			struct online_login_data* data = idb_get(online_db,auth_dat[i].account_id);
			if (data && data->char_server > -1) {
				//Request char servers to kick this account out. [Skotlex]
				ShowWarning("User [%d] is already online - Rejected.\n",auth_dat[i].account_id);
				WBUFW(buf,0) = 0x2734;
				WBUFL(buf,2) = auth_dat[i].account_id;
				charif_sendallwos(-1, buf, 6);
				if (!data->waiting_disconnect)
					add_timer(gettick()+30000, waiting_disconnect_timer,auth_dat[i].account_id, 0);
				data->waiting_disconnect = 1;
				return 3; // Rejected
			}
		}

		if (auth_dat[i].ban_until_time != 0) { // if account is banned
			strftime(tmpstr, 20, date_format, localtime(&auth_dat[i].ban_until_time));
			tmpstr[19] = '\0';
			if (auth_dat[i].ban_until_time > time(NULL)) { // always banned
				login_log("Connection refused (account: %s, pass: %s, banned until %s, ip: %s)" RETCODE,
				          account->userid, account->passwd, tmpstr, ip);
				return 6; // 6 = Your are Prohibited to log in until %s
			} else { // ban is finished
				login_log("End of ban (account: %s, pass: %s, previously banned until %s -> not more banned, ip: %s)" RETCODE,
				          account->userid, account->passwd, tmpstr, ip);
				auth_dat[i].ban_until_time = 0; // reset the ban time
			}
		}

		if (auth_dat[i].connect_until_time != 0 && auth_dat[i].connect_until_time < time(NULL)) {
			login_log("Connection refused (account: %s, pass: %s, expired ID, ip: %s)" RETCODE,
			          account->userid, account->passwd, ip);
			return 2; // 2 = This ID is expired
		}

		login_log("Authentification accepted (account: %s (id: %d), ip: %s)" RETCODE, account->userid, auth_dat[i].account_id, ip);
	} else {
		if (!newaccount) {
			login_log("Unknown account (account: %s, received pass: %s, ip: %s)" RETCODE,
			          account->userid, account->passwd, ip);
			return 0; // 0 = Unregistered ID
		} else {
			int new_id = mmo_auth_new(account, account->userid[len+1], "a@a.com");
			login_log("Account creation and authentification accepted (account %s (id: %d), pass: %s, sex: %c, connection with _F/_M, ip: %s)" RETCODE,
			          account->userid, new_id, account->passwd, account->userid[len+1], ip);
			auth_before_save_file = 0; // Creation of an account -> save accounts file immediatly
			
			//restart ticker (account registration flood protection)[Kevin]
			if(num_regs==0) {
				new_reg_tick=gettick()+time_allowed*1000;
			}
			num_regs++;
		}
	}

	// auth start : time seed
	// Platform/Compiler dependant clock() for time check is removed. [Lance]
	// clock() is originally used to track processing ticks on program execution.
	time(&raw_time);
	strftime(tmpstr, 24, "%Y-%m-%d %H:%M:%S",localtime(&raw_time));

	account->account_id = auth_dat[i].account_id;
	account->login_id1 = rand();
	account->login_id2 = rand();
	memcpy(account->lastlogin, auth_dat[i].lastlogin, 24);
	memcpy(auth_dat[i].lastlogin, tmpstr, 24);
	account->sex = auth_dat[i].sex;
	if (account->sex != 2 && account->account_id < 700000)
		ShowWarning("Account %s has account id %d! Account IDs must be over 700000 to work properly!\n", account->userid, account->account_id);

	strncpy(auth_dat[i].last_ip, ip, 16);
	auth_dat[i].logincount++;

	// Save until for change ip/time of auth is not very useful => limited save for that
	// Save there informations isnot necessary, because they are saved in log file.
	if (--auth_before_save_file <= 0) // Reduce counter. 0 or less, we save
		mmo_auth_sync();

	return -1; // account OK
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

//--------------------------------
// Packet parsing for char-servers
//--------------------------------
int parse_fromchar(int fd) {
	unsigned int i;
	int j, id;
	unsigned char *p = (unsigned char *) &session[fd]->client_addr.sin_addr;
	char ip[16];
	int acc;
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
			login_log("Char-server '%s' has disconnected (ip: %s)." RETCODE,
			          server[id].name, ip);
			server_fd[id] = -1;
			memset(&server[id], 0, sizeof(struct mmo_char_server));
			online_db->foreach(online_db,online_db_setoffline,id); //Set all chars from this char server to offline.
		}
		do_close(fd);
		return 0;
	}

	while (RFIFOREST(fd) >= 2) {

		if (display_parse_fromchar == 2 || (display_parse_fromchar == 1 && RFIFOW(fd,0) != 0x2714)) // 0x2714 is done very often (number of players)
			ShowDebug("parse_fromchar: connection #%d, packet: 0x%x (with being read: %d bytes).\n", fd, RFIFOW(fd,0), RFIFOREST(fd));

		switch (RFIFOW(fd,0)) {
		// request from map-server via char-server to reload GM accounts (by Yor).
		case 0x2709:
			login_log("Char-server '%s': Request to re-load GM configuration file (ip: %s)." RETCODE, server[id].name, ip);
			read_gm_account();
			// send GM accounts to all char-servers
			send_GM_accounts();
			RFIFOSKIP(fd,2);
			break;

		case 0x2712: // request from char-server to authentify an account
			if (RFIFOREST(fd) < 19)
				return 0;
		  {
			int acc;
			acc = RFIFOL(fd,2); // speed up
			for(i = 0; i < AUTH_FIFO_SIZE; i++) {
				if (auth_fifo[i].account_id == acc &&
				    auth_fifo[i].login_id1 == RFIFOL(fd,6) &&
#if CMP_AUTHFIFO_LOGIN2 != 0
				    auth_fifo[i].login_id2 == RFIFOL(fd,10) && // relate to the versions higher than 18
#endif
				    auth_fifo[i].sex == RFIFOB(fd,14) &&
				    (!check_ip_flag || auth_fifo[i].ip == RFIFOL(fd,15)) &&
				    !auth_fifo[i].delflag) {
					unsigned int k;
					time_t connect_until_time = 0;
					char email[40] = "";
					WFIFOHEAD(fd,51);
					auth_fifo[i].delflag = 1;
					login_log("Char-server '%s': authentification of the account %d accepted (ip: %s)." RETCODE,
					          server[id].name, acc, ip);
//					printf("%d\n", i);
					for(k = 0; k < auth_num; k++) {
						if (auth_dat[k].account_id == acc) {
							strcpy(email, auth_dat[k].email);
							connect_until_time = auth_dat[k].connect_until_time;
							break;
						}
					}
					WFIFOW(fd,0) = 0x2713;
					WFIFOL(fd,2) = acc;
					WFIFOB(fd,6) = 0;
					memcpy(WFIFOP(fd, 7), email, 40);
					WFIFOL(fd,47) = (unsigned long)connect_until_time;
					WFIFOSET(fd,51);
					break;
				}
			}
			// authentification not found
			if (i == AUTH_FIFO_SIZE) {
				login_log("Char-server '%s': authentification of the account %d REFUSED (ip: %s)." RETCODE,
				          server[id].name, acc, ip);
                                WFIFOHEAD(fd, 51);
				WFIFOW(fd,0) = 0x2713;
				WFIFOL(fd,2) = acc;
				WFIFOB(fd,6) = 1;
				// It is unnecessary to send email
				// It is unnecessary to send validity date of the account
				WFIFOSET(fd,51);
			}
		  }
			RFIFOSKIP(fd,19);
			break;

		case 0x2714:
			if (RFIFOREST(fd) < 6)
				return 0;
			//printf("parse_fromchar: Receiving of the users number of the server '%s': %d\n", server[id].name, RFIFOL(fd,2));
			server[id].users = RFIFOL(fd,2);
			// send some answer
			WFIFOHEAD(fd, 2);
			WFIFOW(fd,0) = 0x2718;
			WFIFOSET(fd,2);

			RFIFOSKIP(fd,6);
			break;

		// we receive a e-mail creation of an account with a default e-mail (no answer)
		case 0x2715:
			if (RFIFOREST(fd) < 46)
				return 0;
		{
			char email[40];
			acc = RFIFOL(fd,2); // speed up
			memcpy(email, RFIFOP(fd,6), 40);
			email[39] = '\0';
			remove_control_chars((unsigned char *)email);
			//printf("parse_fromchar: an e-mail creation of an account with a default e-mail: server '%s', account: %d, e-mail: '%s'.\n", server[id].name, acc, RFIFOP(fd,6));
			if (e_mail_check(email) == 0)
				login_log("Char-server '%s': Attempt to create an e-mail on an account with a default e-mail REFUSED - e-mail is invalid (account: %d, ip: %s)" RETCODE,
				          server[id].name, acc, ip);
			else {
				for(i = 0; i < auth_num; i++) {
					if (auth_dat[i].account_id == acc && (strcmp(auth_dat[i].email, "a@a.com") == 0 || auth_dat[i].email[0] == '\0')) {
						memcpy(auth_dat[i].email, email, 40);
						login_log("Char-server '%s': Create an e-mail on an account with a default e-mail (account: %d, new e-mail: %s, ip: %s)." RETCODE,
						          server[id].name, acc, email, ip);
						// Save
						mmo_auth_sync();
						break;
					}
				}
				if (i == auth_num)
					login_log("Char-server '%s': Attempt to create an e-mail on an account with a default e-mail REFUSED - account doesn't exist or e-mail of account isn't default e-mail (account: %d, ip: %s)." RETCODE,
					          server[id].name, acc, ip);
			}
		}
			RFIFOSKIP(fd,46);
			break;

		// We receive an e-mail/limited time request, because a player comes back from a map-server to the char-server
		case 0x2716:
			if (RFIFOREST(fd) < 6)
				return 0;
			//printf("parse_fromchar: E-mail/limited time request from '%s' server (concerned account: %d)\n", server[id].name, RFIFOL(fd,2));
			for(i = 0; i < auth_num; i++) {
				if (auth_dat[i].account_id == RFIFOL(fd,2)) {
					login_log("Char-server '%s': e-mail of the account %d found (ip: %s)." RETCODE,
					          server[id].name, RFIFOL(fd,2), ip);
					WFIFOW(fd,0) = 0x2717;
					WFIFOL(fd,2) = RFIFOL(fd,2);
					memcpy(WFIFOP(fd, 6), auth_dat[i].email, 40);
					WFIFOL(fd,46) = (unsigned long)auth_dat[i].connect_until_time;
					WFIFOSET(fd,50);
					break;
				}
			}
			if (i == auth_num)
				login_log("Char-server '%s': e-mail of the account %d NOT found (ip: %s)." RETCODE,
				          server[id].name, RFIFOL(fd,2), ip);
			RFIFOSKIP(fd,6);
			break;

		case 0x2720:	// To become GM request
			if (RFIFOREST(fd) < 4 || RFIFOREST(fd) < RFIFOW(fd,2))
				return 0;
			  {
				unsigned char buf[10];
				FILE *fp;
				acc = RFIFOL(fd,4);
				//printf("parse_fromchar: Request to become a GM acount from %d account.\n", acc);
				WBUFW(buf,0) = 0x2721;
				WBUFL(buf,2) = acc;
				WBUFL(buf,6) = 0;
				if (strcmp((char*)RFIFOP(fd,8), gm_pass) == 0) {
					// only non-GM can become GM
					if (isGM(acc) == 0) {
						// if we autorise creation
						if (level_new_gm > 0) {
							// if we can open the file to add the new GM
							if ((fp = fopen(GM_account_filename, "a")) != NULL) {
								char tmpstr[24];
								time_t raw_time;
								time(&raw_time);
								strftime(tmpstr, 23, date_format, localtime(&raw_time));
								fprintf(fp, RETCODE "// %s: @GM command on account %d" RETCODE "%d %d" RETCODE, tmpstr, acc, acc, level_new_gm);
								fclose(fp);
								WBUFL(buf,6) = level_new_gm;
								read_gm_account();
								send_GM_accounts();
								ShowNotice("GM Change of the account %d: level 0 -> %d.\n", acc, level_new_gm);
								login_log("Char-server '%s': GM Change of the account %d: level 0 -> %d (ip: %s)." RETCODE,
								          server[id].name, acc, level_new_gm, ip);
							} else {
								ShowError("Error of GM change (suggested account: %d, correct password, unable to add a GM account in GM accounts file)\n", acc);
								login_log("Char-server '%s': Error of GM change (suggested account: %d, correct password, unable to add a GM account in GM accounts file, ip: %s)." RETCODE,
								          server[id].name, acc, ip);
							}
						} else {
							ShowError("Error of GM change (suggested account: %d, correct password, but GM creation is disable (level_new_gm = 0))\n", acc);
							login_log("Char-server '%s': Error of GM change (suggested account: %d, correct password, but GM creation is disable (level_new_gm = 0), ip: %s)." RETCODE,
							          server[id].name, acc, ip);
						}
					} else {
						ShowError("Error of GM change (suggested account: %d (already GM), correct password).\n", acc);
						login_log("Char-server '%s': Error of GM change (suggested account: %d (already GM), correct password, ip: %s)." RETCODE,
						          server[id].name, acc, ip);
					}
				} else {
					ShowError("Error of GM change (suggested account: %d, invalid password).\n", acc);
					login_log("Char-server '%s': Error of GM change (suggested account: %d, invalid password, ip: %s)." RETCODE,
					          server[id].name, acc, ip);
				}
				charif_sendallwos(-1, buf, 10);
			  }
			RFIFOSKIP(fd, RFIFOW(fd,2));
			return 0;

		// Map server send information to change an email of an account via char-server
		case 0x2722:	// 0x2722 <account_id>.L <actual_e-mail>.40B <new_e-mail>.40B
			if (RFIFOREST(fd) < 86)
				return 0;
		  {
			char actual_email[40], new_email[40];
			acc = RFIFOL(fd,2);
			memcpy(actual_email, RFIFOP(fd,6), 40);
			actual_email[39] = '\0';
			remove_control_chars((unsigned char *)actual_email);
			memcpy(new_email, RFIFOP(fd,46), 40);
			new_email[39] = '\0';
			remove_control_chars((unsigned char *)new_email);
			if (e_mail_check(actual_email) == 0)
				login_log("Char-server '%s': Attempt to modify an e-mail on an account (@email GM command), but actual email is invalid (account: %d, ip: %s)" RETCODE,
				          server[id].name, acc, ip);
			else if (e_mail_check(new_email) == 0)
				login_log("Char-server '%s': Attempt to modify an e-mail on an account (@email GM command) with a invalid new e-mail (account: %d, ip: %s)" RETCODE,
				          server[id].name, acc, ip);
			else if (strcmpi(new_email, "a@a.com") == 0)
				login_log("Char-server '%s': Attempt to modify an e-mail on an account (@email GM command) with a default e-mail (account: %d, ip: %s)" RETCODE,
				          server[id].name, acc, ip);
			else {
				for(i = 0; i < auth_num; i++) {
					if (auth_dat[i].account_id == acc) {
						if (strcmpi(auth_dat[i].email, actual_email) == 0) {
							memcpy(auth_dat[i].email, new_email, 40);
							login_log("Char-server '%s': Modify an e-mail on an account (@email GM command) (account: %d (%s), new e-mail: %s, ip: %s)." RETCODE,
							          server[id].name, acc, auth_dat[i].userid, new_email, ip);
							// Save
							mmo_auth_sync();
						} else
							login_log("Char-server '%s': Attempt to modify an e-mail on an account (@email GM command), but actual e-mail is incorrect (account: %d (%s), actual e-mail: %s, proposed e-mail: %s, ip: %s)." RETCODE,
							          server[id].name, acc, auth_dat[i].userid, auth_dat[i].email, actual_email, ip);
						break;
					}
				}
				if (i == auth_num)
					login_log("Char-server '%s': Attempt to modify an e-mail on an account (@email GM command), but account doesn't exist (account: %d, ip: %s)." RETCODE,
					          server[id].name, acc, ip);
			}
		  }
			RFIFOSKIP(fd, 86);
			break;

		// Receiving of map-server via char-server a status change resquest (by Yor)
		case 0x2724:
			if (RFIFOREST(fd) < 10)
				return 0;
			{
				int acc, statut;
				acc = RFIFOL(fd,2);
				statut = RFIFOL(fd,6);
				for(i = 0; i < auth_num; i++) {
					if (auth_dat[i].account_id == acc) {
						if (auth_dat[i].state != statut) {
							login_log("Char-server '%s': Status change (account: %d, new status %d, ip: %s)." RETCODE,
							          server[id].name, acc, statut, ip);
							if (statut != 0) {
								unsigned char buf[16];
								WBUFW(buf,0) = 0x2731;
								WBUFL(buf,2) = acc;
								WBUFB(buf,6) = 0; // 0: change of statut, 1: ban
								WBUFL(buf,7) = statut; // status or final date of a banishment
								charif_sendallwos(-1, buf, 11);
								for(j = 0; j < AUTH_FIFO_SIZE; j++)
									if (auth_fifo[j].account_id == acc)
										auth_fifo[j].login_id1++; // to avoid reconnection error when come back from map-server (char-server will ask again the authentification)
							}
							auth_dat[i].state = statut;
							// Save
							mmo_auth_sync();
						} else
							login_log("Char-server '%s':  Error of Status change - actual status is already the good status (account: %d, status %d, ip: %s)." RETCODE,
							          server[id].name, acc, statut, ip);
						break;
					}
				}
				if (i == auth_num) {
					login_log("Char-server '%s': Error of Status change (account: %d not found, suggested status %d, ip: %s)." RETCODE,
					          server[id].name, acc, statut, ip);
				}
				RFIFOSKIP(fd,10);
			}
			return 0;

		case 0x2725:	// Receiving of map-server via char-server a ban resquest (by Yor)
			if (RFIFOREST(fd) < 18)
				return 0;
			{
				acc = RFIFOL(fd,2);
				for(i = 0; i < auth_num; i++) {
					if (auth_dat[i].account_id == acc) {
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
						if (timestamp != -1) {
							if (timestamp <= time(NULL))
								timestamp = 0;
							if (auth_dat[i].ban_until_time != timestamp) {
								if (timestamp != 0) {
									unsigned char buf[16];
									char tmpstr[2048];
									strftime(tmpstr, 24, date_format, localtime(&timestamp));
									login_log("Char-server '%s': Ban request (account: %d, new final date of banishment: %d (%s), ip: %s)." RETCODE,
									          server[id].name, acc, timestamp, (timestamp == 0 ? "no banishment" : tmpstr), ip);
									WBUFW(buf,0) = 0x2731;
									WBUFL(buf,2) = auth_dat[i].account_id;
									WBUFB(buf,6) = 1; // 0: change of statut, 1: ban
									WBUFL(buf,7) = (unsigned int)timestamp; // status or final date of a banishment
									charif_sendallwos(-1, buf, 11);
									for(j = 0; j < AUTH_FIFO_SIZE; j++)
										if (auth_fifo[j].account_id == acc)
											auth_fifo[j].login_id1++; // to avoid reconnection error when come back from map-server (char-server will ask again the authentification)
								} else {
									login_log("Char-server '%s': Error of ban request (account: %d, new date unbans the account, ip: %s)." RETCODE,
									          server[id].name, acc, ip);
								}
								auth_dat[i].ban_until_time = timestamp;
								// Save
								mmo_auth_sync();
							} else {
								login_log("Char-server '%s': Error of ban request (account: %d, no change for ban date, ip: %s)." RETCODE,
								          server[id].name, acc, ip);
							}
						} else {
							login_log("Char-server '%s': Error of ban request (account: %d, invalid date, ip: %s)." RETCODE,
							          server[id].name, acc, ip);
						}
						break;
					}
				}
				if (i == auth_num)
					login_log("Char-server '%s': Error of ban request (account: %d not found, ip: %s)." RETCODE,
					          server[id].name, acc, ip);
				RFIFOSKIP(fd,18);
			}
			return 0;

		case 0x2727:	// Change of sex (sex is reversed)
			if (RFIFOREST(fd) < 6)
				return 0;
			{
				int sex;
				acc = RFIFOL(fd,2);
				for(i = 0; i < auth_num; i++) {
//					printf("%d,", auth_dat[i].account_id);
					if (auth_dat[i].account_id == acc) {
						if (auth_dat[i].sex == 2)
							login_log("Char-server '%s': Error of sex change - Server account (suggested account: %d, actual sex %d (Server), ip: %s)." RETCODE,
							          server[id].name, acc, auth_dat[i].sex, ip);
						else {
							unsigned char buf[16];
							if (auth_dat[i].sex == 0)
								sex = 1;
							else
								sex = 0;
							login_log("Char-server '%s': Sex change (account: %d, new sex %c, ip: %s)." RETCODE,
							          server[id].name, acc, (sex == 2) ? 'S' : (sex ? 'M' : 'F'), ip);
							for(j = 0; j < AUTH_FIFO_SIZE; j++)
								if (auth_fifo[j].account_id == acc)
									auth_fifo[j].login_id1++; // to avoid reconnection error when come back from map-server (char-server will ask again the authentification)
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
					login_log("Char-server '%s': Error of sex change (account: %d not found, sex would be reversed, ip: %s)." RETCODE,
					          server[id].name, acc, ip);
				RFIFOSKIP(fd,6);
			}
			return 0;

		case 0x2728:	// We receive account_reg2 from a char-server, and we send them to other map-servers.
			if (RFIFOREST(fd) < 4 || RFIFOREST(fd) < RFIFOW(fd,2))
				return 0;
			{
				int p;
				acc = RFIFOL(fd,4);
				for(i = 0; i < auth_num; i++) {
					if (auth_dat[i].account_id == acc) {
						//unsigned char buf[rfifow(fd,2)+1];
						unsigned char *buf;
						int len;
						buf = (unsigned char*)aCalloc(RFIFOW(fd,2)+1, sizeof(unsigned char));
						login_log("char-server '%s': receiving (from the char-server) of account_reg2 (account: %d, ip: %s)." RETCODE,
						          server[id].name, acc, ip);
						for(j=0,p=13;j<ACCOUNT_REG2_NUM && p<RFIFOW(fd,2);j++){
							sscanf(RFIFOP(fd,p), "%31c%n",auth_dat[i].account_reg2[j].str,&len);
							auth_dat[i].account_reg2[j].str[len]='\0';
							p +=len+1; //+1 to skip the '\0' between strings.
							sscanf(RFIFOP(fd,p), "%255c%n",auth_dat[i].account_reg2[j].value,&len);
							auth_dat[i].account_reg2[j].value[len]='\0';
							p +=len+1;
							remove_control_chars((unsigned char *)auth_dat[i].account_reg2[j].str);
							remove_control_chars((unsigned char *)auth_dat[i].account_reg2[j].value);
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
//					printf("parse_fromchar: receiving (from the char-server) of account_reg2 (unknwon account id: %d).\n", acc);
					login_log("Char-server '%s': receiving (from the char-server) of account_reg2 (account: %d not found, ip: %s)." RETCODE,
					          server[id].name, acc, ip);
				}
			}
			RFIFOSKIP(fd,RFIFOW(fd,2));
			break;

		case 0x272a:	// Receiving of map-server via char-server a unban resquest (by Yor)
			if (RFIFOREST(fd) < 6)
				return 0;
			{
				acc = RFIFOL(fd,2);
				for(i = 0; i < auth_num; i++) {
					if (auth_dat[i].account_id == acc) {
						if (auth_dat[i].ban_until_time != 0) {
							auth_dat[i].ban_until_time = 0;
							login_log("Char-server '%s': UnBan request (account: %d, ip: %s)." RETCODE,
							          server[id].name, acc, ip);
						} else {
							login_log("Char-server '%s': Error of UnBan request (account: %d, no change for unban date, ip: %s)." RETCODE,
							          server[id].name, acc, ip);
						}
						break;
					}
				}
				if (i == auth_num)
					login_log("Char-server '%s': Error of UnBan request (account: %d not found, ip: %s)." RETCODE,
					          server[id].name, acc, ip);
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
				int aid;
			  	unsigned int users;
				online_db->foreach(online_db,online_db_setoffline,id); //Set all chars from this char-server offline first
				users = RFIFOW(fd,4);
				for (i = 0; i < users; i++) {
					aid = RFIFOL(fd,6+i*4);
					p = idb_ensure(online_db, aid, create_online_user);
					p->char_server = id;
					p->waiting_disconnect = 0;
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
				RFIFOSKIP(fd,10);
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
				for(p = 13,j=0;j<auth_dat[i].account_reg2_num;j++){
					if (auth_dat[i].account_reg2[j].str[0]) {
						p+= sprintf(WFIFOP(fd,p), "%s", auth_dat[i].account_reg2[j].str)+1; //We add 1 to consider the '\0' in place.
						p+= sprintf(WFIFOP(fd,p), "%s", auth_dat[i].account_reg2[j].value)+1;
					}
				}
				WFIFOW(fd,2) = p;
				WFIFOSET(fd,WFIFOW(fd,2));
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

		case 0x3000: //change sex for chrif_changesex()
			if (RFIFOREST(fd) < 4 || RFIFOREST(fd) < RFIFOW(fd,2))
				return 0;
			{
				unsigned int sex;
				acc = RFIFOL(fd,4);
				sex = RFIFOB(fd,8);
				if (sex != 0 && sex != 1)
					sex = 0;
				for(i = 0; i < auth_num; i++) {
					if (auth_dat[i].account_id == acc) {
						unsigned char buf[16];
						login_log("Char-server '%s': Sex change (account: %d, new sex %c, ip: %s)." RETCODE,
						          server[id].name, acc, (sex == 2) ? 'S' : (sex ? 'M' : 'F'), ip);
						auth_fifo[i].login_id1++; // to avoid reconnection error when come back from map-server (char-server will ask again the authentification)
						auth_dat[i].sex = sex;
						WBUFW(buf,0) = 0x2723;
						WBUFL(buf,2) = acc;
						WBUFB(buf,6) = sex;
						charif_sendallwos(-1, buf, 7);
						break;
					}
				}
				if (i == auth_num) {
					login_log("Char-server '%s': Error of Sex change (account: %d not found, suggested sex %c, ip: %s)." RETCODE,
					          server[id].name, acc, (sex == 2) ? 'S' : (sex ? 'M' : 'F'), ip);
				}
				RFIFOSKIP(fd,RFIFOW(fd,2));
			}
			return 0;

		default:
			{
				FILE *logfp;
				char tmpstr[24];
				time_t raw_time;
				logfp = fopen(login_log_unknown_packets_filename, "a");
				if (logfp) {
					time(&raw_time);
					strftime(tmpstr, 23, date_format, localtime(&raw_time));
					fprintf(logfp, "%s: receiving of an unknown packet -> disconnection" RETCODE, tmpstr);
					fprintf(logfp, "parse_fromchar: connection #%d (ip: %s), packet: 0x%x (with being read: %d)." RETCODE, fd, ip, RFIFOW(fd,0), RFIFOREST(fd));
					fprintf(logfp, "Detail (in hex):" RETCODE);
					fprintf(logfp, "---- 00-01-02-03-04-05-06-07  08-09-0A-0B-0C-0D-0E-0F" RETCODE);
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
							fprintf(logfp, " %s" RETCODE, tmpstr);
							memset(tmpstr, '\0', sizeof(tmpstr));
						}
					}
					if (i % 16 != 0) {
						for(j = i; j % 16 != 0; j++) {
							fprintf(logfp, "   ");
							if ((j - 7) % 16 == 0) // -8 + 1
								fprintf(logfp, " ");
						}
						fprintf(logfp, " %s" RETCODE, tmpstr);
					}
					fprintf(logfp, RETCODE);
					fclose(logfp);
				}
			}
			ShowWarning("parse_fromchar: Unknown packet 0x%x (from a char-server)! -> disconnection.\n", RFIFOW(fd,0));
			session[fd]->eof = 1;
			ShowStatus("Char-server has been disconnected (unknown packet).\n");
			return 0;
		}
	}
	RFIFOSKIP(fd,RFIFOREST(fd));
	return 0;
}

//---------------------------------------
// Packet parsing for administation login
//---------------------------------------
int parse_admin(int fd) {
	unsigned int i, j;
	unsigned char *p = (unsigned char *) &session[fd]->client_addr.sin_addr;
	char* account_name;
	char ip[16];
	RFIFOHEAD(fd);

	sprintf(ip, "%d.%d.%d.%d", p[0], p[1], p[2], p[3]);

	if (session[fd]->eof) {
		do_close(fd);
		ShowInfo("Remote administration has disconnected (session #%d).\n", fd);
		return 0;
	}

	while(RFIFOREST(fd) >= 2) {
          if (display_parse_admin == 1) {

			ShowDebug("parse_admin: connection #%d, packet: 0x%x (with being read: %d).\n", fd, RFIFOW(fd,0), RFIFOREST(fd));
          }

		switch(RFIFOW(fd,0)) {
		case 0x7530:	// Request of the server version
			login_log("'ladmin': Sending of the server version (ip: %s)" RETCODE, ip);
                        WFIFOHEAD(fd, 10);
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

		case 0x7532:	// Request of end of connection
			login_log("'ladmin': End of connection (ip: %s)" RETCODE, ip);
			RFIFOSKIP(fd,2);
			session[fd]->eof = 1;
			break;

		case 0x7920:	// Request of an accounts list
			if (RFIFOREST(fd) < 10)
				return 0;
			{
				int st, ed, len;
				//int id[auth_num];
				//int *id=(int *)aCalloc(auth_num, sizeof(int));
				CREATE_BUFFER(id, int, auth_num);
				st = RFIFOL(fd,2);
				ed = RFIFOL(fd,6);
				RFIFOSKIP(fd,10);
				WFIFOW(fd,0) = 0x7921;
				if (st < 0)
					st = 0;
				if (ed > END_ACCOUNT_NUM || ed < st || ed <= 0)
					ed = END_ACCOUNT_NUM;
				login_log("'ladmin': Sending an accounts list (ask: from %d to %d, ip: %s)" RETCODE, st, ed, ip);
				// Sort before send
				for(i = 0; i < auth_num; i++) {
					unsigned int k;
					id[i] = i;
					for(j = 0; j < i; j++) {
						if (auth_dat[id[i]].account_id < auth_dat[id[j]].account_id) {
							for(k = i; k > j; k--) {
								id[k] = id[k-1];
							}
							id[j] = i; // id[i]
							break;
						}
					}
				}
				// Sending accounts information
				len = 4;
				for(i = 0; i < auth_num && len < 30000; i++) {
					int account_id = auth_dat[id[i]].account_id; // use sorted index
					if (account_id >= st && account_id <= ed) {
						j = id[i];
						WFIFOL(fd,len) = account_id;
						WFIFOB(fd,len+4) = (unsigned char)isGM(account_id);
						memcpy(WFIFOP(fd,len+5), auth_dat[j].userid, 24);
						WFIFOB(fd,len+29) = auth_dat[j].sex;
						WFIFOL(fd,len+30) = auth_dat[j].logincount;
						if (auth_dat[j].state == 0 && auth_dat[j].ban_until_time != 0) // if no state and banished
							WFIFOL(fd,len+34) = 7; // 6 = Your are Prohibited to log in until %s
						else
							WFIFOL(fd,len+34) = auth_dat[j].state;
						len += 38;
					}
				}
				WFIFOW(fd,2) = len;
				WFIFOSET(fd,len);
				//if (id) free(id);
				DELETE_BUFFER(id);
			}
			break;

		case 0x7930:	// Request for an account creation
			if (RFIFOREST(fd) < 91)
				return 0;
			{
				struct mmo_account ma;
				memcpy(ma.userid,RFIFOP(fd, 2),NAME_LENGTH);
				ma.userid[23] = '\0';
				memcpy(ma.passwd, RFIFOP(fd, 26), NAME_LENGTH);
				ma.passwd[23] = '\0';
				memcpy(ma.lastlogin, "-", 2);
				ma.sex = RFIFOB(fd,50);
				WFIFOW(fd,0) = 0x7931;
				WFIFOL(fd,2) = 0xffffffff;
				memcpy(WFIFOP(fd,6), RFIFOP(fd,2), 24);
				if (strlen(ma.userid) < 4 || strlen(ma.passwd) < 4) {
					login_log("'ladmin': Attempt to create an invalid account (account or pass is too short, ip: %s)" RETCODE,
					          ip);
				} else if (ma.sex != 'F' && ma.sex != 'M') {
					login_log("'ladmin': Attempt to create an invalid account (account: %s, received pass: %s, invalid sex, ip: %s)" RETCODE,
					          ma.userid, ma.passwd, ip);
				} else if (account_id_count > END_ACCOUNT_NUM) {
					login_log("'ladmin': Attempt to create an account, but there is no more available id number (account: %s, pass: %s, sex: %c, ip: %s)" RETCODE,
					          ma.userid, ma.passwd, ma.sex, ip);
				} else {
					remove_control_chars((unsigned char *)ma.userid);
					remove_control_chars((unsigned char *)ma.passwd);
					for(i = 0; i < auth_num; i++) {
						if (strncmp(auth_dat[i].userid, ma.userid, 24) == 0) {
							login_log("'ladmin': Attempt to create an already existing account (account: %s, pass: %s, received pass: %s, ip: %s)" RETCODE,
							          auth_dat[i].userid, auth_dat[i].pass, ma.passwd, ip);
							break;
						}
					}
					if (i == auth_num) {
						int new_id;
						char email[40];
						memcpy(email, RFIFOP(fd,51), 40);
						email[39] = '\0';
						remove_control_chars((unsigned char *)email);
						new_id = mmo_auth_new(&ma, ma.sex, email);
						login_log("'ladmin': Account creation (account: %s (id: %d), pass: %s, sex: %c, email: %s, ip: %s)" RETCODE,
						          ma.userid, new_id, ma.passwd, ma.sex, auth_dat[i].email, ip);
						WFIFOL(fd,2) = new_id;
						mmo_auth_sync();
					}
				}
				WFIFOSET(fd,30);
				RFIFOSKIP(fd,91);
			}
			break;

		case 0x7932:	// Request for an account deletion
			if (RFIFOREST(fd) < 26)
				return 0;
			WFIFOW(fd,0) = 0x7933;
			WFIFOL(fd,2) = 0xFFFFFFFF;
			account_name = (char*)RFIFOP(fd,2);
			account_name[23] = '\0';
			remove_control_chars((unsigned char *)account_name);
			i = search_account_index(account_name);
			if (i != -1) {
				// Char-server is notified of deletion (for characters deletion).
				unsigned char buf[65535];
				WBUFW(buf,0) = 0x2730;
				WBUFL(buf,2) = auth_dat[i].account_id;
				charif_sendallwos(-1, buf, 6);
				// send answer
				memcpy(WFIFOP(fd,6), auth_dat[i].userid, 24);
				WFIFOL(fd,2) = auth_dat[i].account_id;
				// save deleted account in log file
				login_log("'ladmin': Account deletion (account: %s, id: %d, ip: %s) - saved in next line:" RETCODE,
				          auth_dat[i].userid, auth_dat[i].account_id, ip);
				mmo_auth_tostr((char*)buf, &auth_dat[i]);
				login_log("%s" RETCODE, buf);
				// delete account
				memset(auth_dat[i].userid, '\0', sizeof(auth_dat[i].userid));
				auth_dat[i].account_id = -1;
				mmo_auth_sync();
			} else {
				memcpy(WFIFOP(fd,6), account_name, 24);
				login_log("'ladmin': Attempt to delete an unknown account (account: %s, ip: %s)" RETCODE,
				          account_name, ip);
			}
			WFIFOSET(fd,30);
			RFIFOSKIP(fd,26);
			break;

		case 0x7934:	// Request to change a password
			if (RFIFOREST(fd) < 50)
				return 0;
			WFIFOW(fd,0) = 0x7935;
			WFIFOL(fd,2) = 0xFFFFFFFF; /// WTF??? an unsigned being set to a -1
			account_name = (char*)RFIFOP(fd,2);
			account_name[23] = '\0';
			remove_control_chars((unsigned char *)account_name);
			i = search_account_index(account_name);
			if (i != -1) {
				memcpy(WFIFOP(fd,6), auth_dat[i].userid, 24);
				memcpy(auth_dat[i].pass, RFIFOP(fd,26), 24);
				auth_dat[i].pass[23] = '\0';
				remove_control_chars((unsigned char *)auth_dat[i].pass);
				WFIFOL(fd,2) = auth_dat[i].account_id;
				login_log("'ladmin': Modification of a password (account: %s, new password: %s, ip: %s)" RETCODE,
				          auth_dat[i].userid, auth_dat[i].pass, ip);
				mmo_auth_sync();
			} else {
				memcpy(WFIFOP(fd,6), account_name, 24);
				login_log("'ladmin': Attempt to modify the password of an unknown account (account: %s, ip: %s)" RETCODE,
				          account_name, ip);
			}
			WFIFOSET(fd,30);
			RFIFOSKIP(fd,50);
			break;

		case 0x7936:	// Request to modify a state
			if (RFIFOREST(fd) < 50)
				return 0;
			{
				char error_message[20];
				int statut;
				WFIFOW(fd,0) = 0x7937;
				WFIFOL(fd,2) = 0xFFFFFFFF; // WTF???
				account_name = (char*)RFIFOP(fd,2);
				account_name[23] = '\0';
				remove_control_chars((unsigned char *)account_name);
				statut = RFIFOL(fd,26);
				memcpy(error_message, RFIFOP(fd,30), 20);
				error_message[19] = '\0';
				remove_control_chars((unsigned char *)error_message);
				if (statut != 7 || error_message[0] == '\0') { // 7: // 6 = Your are Prohibited to log in until %s
					strcpy(error_message, "-");
				}
				i = search_account_index(account_name);
				if (i != -1) {
					memcpy(WFIFOP(fd,6), auth_dat[i].userid, 24);
					WFIFOL(fd,2) = auth_dat[i].account_id;
					if (auth_dat[i].state == statut && strcmp(auth_dat[i].error_message, error_message) == 0)
						login_log("'ladmin': Modification of a state, but the state of the account is already the good state (account: %s, received state: %d, ip: %s)" RETCODE,
						          account_name, statut, ip);
					else {
						if (statut == 7)
							login_log("'ladmin': Modification of a state (account: %s, new state: %d - prohibited to login until '%s', ip: %s)" RETCODE,
							          auth_dat[i].userid, statut, error_message, ip);
						else
							login_log("'ladmin': Modification of a state (account: %s, new state: %d, ip: %s)" RETCODE,
							          auth_dat[i].userid, statut, ip);
						if (auth_dat[i].state == 0) {
							unsigned char buf[16];
							WBUFW(buf,0) = 0x2731;
							WBUFL(buf,2) = auth_dat[i].account_id;
							WBUFB(buf,6) = 0; // 0: change of statut, 1: ban
							WBUFL(buf,7) = statut; // status or final date of a banishment
							charif_sendallwos(-1, buf, 11);
							for(j = 0; j < AUTH_FIFO_SIZE; j++)
								if (auth_fifo[j].account_id == auth_dat[i].account_id)
									auth_fifo[j].login_id1++; // to avoid reconnection error when come back from map-server (char-server will ask again the authentification)
						}
						auth_dat[i].state = statut;
						memcpy(auth_dat[i].error_message, error_message, 20);
						mmo_auth_sync();
					}
				} else {
					memcpy(WFIFOP(fd,6), account_name, 24);
					login_log("'ladmin': Attempt to modify the state of an unknown account (account: %s, received state: %d, ip: %s)" RETCODE,
					          account_name, statut, ip);
				}
				WFIFOL(fd,30) = statut;
			}
			WFIFOSET(fd,34);
			RFIFOSKIP(fd,50);
			break;

		case 0x7938:	// Request for servers list and # of online players
			login_log("'ladmin': Sending of servers list (ip: %s)" RETCODE, ip);
			server_num = 0;
			for(i = 0; i < MAX_SERVERS; i++) {
				if (server_fd[i] >= 0) {
					WFIFOL(fd,4+server_num*32) = server[i].ip;
					WFIFOW(fd,4+server_num*32+4) = server[i].port;
					memcpy(WFIFOP(fd,4+server_num*32+6), server[i].name, 20);
					WFIFOW(fd,4+server_num*32+26) = server[i].users;
					WFIFOW(fd,4+server_num*32+28) = server[i].maintenance;
					WFIFOW(fd,4+server_num*32+30) = server[i].new_;
					server_num++;
				}
			}
			WFIFOW(fd,0) = 0x7939;
			WFIFOW(fd,2) = 4 + 32 * server_num;
			WFIFOSET(fd,4+32*server_num);
			RFIFOSKIP(fd,2);
			break;

		case 0x793a:	// Request to password check
			if (RFIFOREST(fd) < 50)
				return 0;
			WFIFOW(fd,0) = 0x793b;
			WFIFOL(fd,2) = 0xFFFFFFFF; // WTF???
			account_name = (char*)RFIFOP(fd,2);
			account_name[23] = '\0';
			remove_control_chars((unsigned char *)account_name);
			i = search_account_index(account_name);
			if (i != -1) {
				char pass[25];
				memcpy(WFIFOP(fd,6), auth_dat[i].userid, 24);
				memcpy(pass, RFIFOP(fd,26), 24);
				pass[24] = '\0';
				remove_control_chars((unsigned char *)pass);
				if (strcmp(auth_dat[i].pass, pass) == 0) {
					WFIFOL(fd,2) = auth_dat[i].account_id;
					login_log("'ladmin': Check of password OK (account: %s, password: %s, ip: %s)" RETCODE,
					          auth_dat[i].userid, auth_dat[i].pass, ip);
				} else {
					login_log("'ladmin': Failure of password check (account: %s, proposed pass: %s, ip: %s)" RETCODE,
					          auth_dat[i].userid, pass, ip);
				}
			} else {
				memcpy(WFIFOP(fd,6), account_name, 24);
				login_log("'ladmin': Attempt to check the password of an unknown account (account: %s, ip: %s)" RETCODE,
				          account_name, ip);
			}
			WFIFOSET(fd,30);
			RFIFOSKIP(fd,50);
			break;

		case 0x793c:	// Request to modify sex
			if (RFIFOREST(fd) < 27)
				return 0;
			WFIFOW(fd,0) = 0x793d;
			WFIFOL(fd,2) = 0xFFFFFFFF; // WTF???
			account_name = (char*)RFIFOP(fd,2);
			account_name[23] = '\0';
			remove_control_chars((unsigned char *)account_name);
			memcpy(WFIFOP(fd,6), account_name, 24);
			{
				char sex;
				sex = RFIFOB(fd,26);
				if (sex != 'F' && sex != 'M') {
					if (sex > 31)
						login_log("'ladmin': Attempt to give an invalid sex (account: %s, received sex: %c, ip: %s)" RETCODE,
						          account_name, sex, ip);
					else
						login_log("'ladmin': Attempt to give an invalid sex (account: %s, received sex: 'control char', ip: %s)" RETCODE,
						          account_name, ip);
				} else {
					i = search_account_index(account_name);
					if (i != -1) {
						memcpy(WFIFOP(fd,6), auth_dat[i].userid, 24);
						if (auth_dat[i].sex != ((sex == 'S' || sex == 's') ? 2 : (sex == 'M' || sex == 'm'))) {
							unsigned char buf[16];
							WFIFOL(fd,2) = auth_dat[i].account_id;
							for(j = 0; j < AUTH_FIFO_SIZE; j++)
								if (auth_fifo[j].account_id == auth_dat[i].account_id)
									auth_fifo[j].login_id1++; // to avoid reconnection error when come back from map-server (char-server will ask again the authentification)
							auth_dat[i].sex = (sex == 'S' || sex == 's') ? 2 : (sex == 'M' || sex == 'm');
							login_log("'ladmin': Modification of a sex (account: %s, new sex: %c, ip: %s)" RETCODE,
							          auth_dat[i].userid, sex, ip);
							mmo_auth_sync();
							// send to all char-server the change
							WBUFW(buf,0) = 0x2723;
							WBUFL(buf,2) = auth_dat[i].account_id;
							WBUFB(buf,6) = auth_dat[i].sex;
							charif_sendallwos(-1, buf, 7);
						} else {
							login_log("'ladmin': Modification of a sex, but the sex is already the good sex (account: %s, sex: %c, ip: %s)" RETCODE,
							          auth_dat[i].userid, sex, ip);
						}
					} else {
						login_log("'ladmin': Attempt to modify the sex of an unknown account (account: %s, received sex: %c, ip: %s)" RETCODE,
						          account_name, sex, ip);
					}
				}
			}
			WFIFOSET(fd,30);
			RFIFOSKIP(fd,27);
			break;

		case 0x793e:	// Request to modify GM level
			if (RFIFOREST(fd) < 27)
				return 0;
			WFIFOW(fd,0) = 0x793f;
			WFIFOL(fd,2) = 0xFFFFFFFF; // WTF???
			account_name = (char*)RFIFOP(fd,2);
			account_name[23] = '\0';
			remove_control_chars((unsigned char *)account_name);
			memcpy(WFIFOP(fd,6), account_name, 24);
			{
				char new_gm_level;
				new_gm_level = RFIFOB(fd,26);
				if (new_gm_level < 0 || new_gm_level > 99) {
					login_log("'ladmin': Attempt to give an invalid GM level (account: %s, received GM level: %d, ip: %s)" RETCODE,
					          account_name, (int)new_gm_level, ip);
				} else {
					i = search_account_index(account_name);
					if (i != -1) {
						int acc = auth_dat[i].account_id;
						memcpy(WFIFOP(fd,6), auth_dat[i].userid, 24);
						if (isGM(acc) != new_gm_level) {
							// modification of the file
							FILE *fp, *fp2;
							int lock;
							char line[512];
							int GM_account, GM_level;
							int modify_flag;
							char tmpstr[24];
							time_t raw_time;
							if ((fp2 = lock_fopen(GM_account_filename, &lock)) != NULL) {
								if ((fp = fopen(GM_account_filename, "r")) != NULL) {
									time(&raw_time);
									strftime(tmpstr, 23, date_format, localtime(&raw_time));
									modify_flag = 0;
									// read/write GM file
									while(fgets(line, sizeof(line)-1, fp)) {
										while(line[0] != '\0' && (line[strlen(line)-1] == '\n' || line[strlen(line)-1] == '\r'))
											line[strlen(line)-1] = '\0';
										if ((line[0] == '/' && line[1] == '/') || line[0] == '\0')
											fprintf(fp2, "%s" RETCODE, line);
										else {
											if (sscanf(line, "%d %d", &GM_account, &GM_level) != 2 && sscanf(line, "%d: %d", &GM_account, &GM_level) != 2)
												fprintf(fp2, "%s" RETCODE, line);
											else if (GM_account != acc)
												fprintf(fp2, "%s" RETCODE, line);
											else if (new_gm_level < 1) {
												fprintf(fp2, "// %s: 'ladmin' GM level removed on account %d '%s' (previous level: %d)" RETCODE "//%d %d" RETCODE, tmpstr, acc, auth_dat[i].userid, GM_level, acc, new_gm_level);
												modify_flag = 1;
											} else {
												fprintf(fp2, "// %s: 'ladmin' GM level on account %d '%s' (previous level: %d)" RETCODE "%d %d" RETCODE, tmpstr, acc, auth_dat[i].userid, GM_level, acc, new_gm_level);
												modify_flag = 1;
											}
										}
									}
									if (modify_flag == 0)
										fprintf(fp2, "// %s: 'ladmin' GM level on account %d '%s' (previous level: 0)" RETCODE "%d %d" RETCODE, tmpstr, acc, auth_dat[i].userid, acc, new_gm_level);
									fclose(fp);
								} else {
									login_log("'ladmin': Attempt to modify of a GM level - impossible to read GM accounts file (account: %s (%d), received GM level: %d, ip: %s)" RETCODE,
									          auth_dat[i].userid, acc, (int)new_gm_level, ip);
								}
								if (lock_fclose(fp2, GM_account_filename, &lock) == 0) {
									WFIFOL(fd,2) = acc;
									login_log("'ladmin': Modification of a GM level (account: %s (%d), new GM level: %d, ip: %s)" RETCODE,
									          auth_dat[i].userid, acc, (int)new_gm_level, ip);
									// read and send new GM informations
									read_gm_account();
									send_GM_accounts();
								} else {
									login_log("'ladmin': Attempt to modify of a GM level - impossible to write GM accounts file (account: %s (%d), received GM level: %d, ip: %s)" RETCODE,
									          auth_dat[i].userid, acc, (int)new_gm_level, ip);
								}
							} else {
								login_log("'ladmin': Attempt to modify of a GM level - impossible to write GM accounts file (account: %s (%d), received GM level: %d, ip: %s)" RETCODE,
								          auth_dat[i].userid, acc, (int)new_gm_level, ip);
							}
						} else {
							login_log("'ladmin': Attempt to modify of a GM level, but the GM level is already the good GM level (account: %s (%d), GM level: %d, ip: %s)" RETCODE,
							          auth_dat[i].userid, acc, (int)new_gm_level, ip);
						}
					} else {
						login_log("'ladmin': Attempt to modify the GM level of an unknown account (account: %s, received GM level: %d, ip: %s)" RETCODE,
						          account_name, (int)new_gm_level, ip);
					}
				}
			}
			WFIFOSET(fd,30);
			RFIFOSKIP(fd,27);
			break;

		case 0x7940:	// Request to modify e-mail
			if (RFIFOREST(fd) < 66)
				return 0;
			WFIFOW(fd,0) = 0x7941;
			WFIFOL(fd,2) = 0xFFFFFFFF; // WTF???
			account_name = (char*)RFIFOP(fd,2);
			account_name[23] = '\0';
			remove_control_chars((unsigned char *)account_name);
			memcpy(WFIFOP(fd,6), account_name, 24);
			{
				char email[40];
				memcpy(email, RFIFOP(fd,26), 40);
				if (e_mail_check(email) == 0) {
					login_log("'ladmin': Attempt to give an invalid e-mail (account: %s, ip: %s)" RETCODE,
					          account_name, ip);
				} else {
					remove_control_chars((unsigned char *)email);
					i = search_account_index(account_name);
					if (i != -1) {
						memcpy(WFIFOP(fd,6), auth_dat[i].userid, 24);
						memcpy(auth_dat[i].email, email, 40);
						WFIFOL(fd,2) = auth_dat[i].account_id;
						login_log("'ladmin': Modification of an email (account: %s, new e-mail: %s, ip: %s)" RETCODE,
						          auth_dat[i].userid, email, ip);
						mmo_auth_sync();
					} else {
						login_log("'ladmin': Attempt to modify the e-mail of an unknown account (account: %s, received e-mail: %s, ip: %s)" RETCODE,
						          account_name, email, ip);
					}
				}
			}
			WFIFOSET(fd,30);
			RFIFOSKIP(fd,66);
			break;

		case 0x7942:	// Request to modify memo field
			if ((int)RFIFOREST(fd) < 28 || (int)RFIFOREST(fd) < (28 + RFIFOW(fd,26)))
				return 0;
			WFIFOW(fd,0) = 0x7943;
			WFIFOL(fd,2) = 0xFFFFFFFF; // WTF???
			account_name = (char*)RFIFOP(fd,2);
			account_name[23] = '\0';
			remove_control_chars((unsigned char *)account_name);
			i = search_account_index(account_name);
			if (i != -1) {
				int size_of_memo = sizeof(auth_dat[i].memo);
				memcpy(WFIFOP(fd,6), auth_dat[i].userid, 24);
				memset(auth_dat[i].memo, '\0', size_of_memo);
				if (RFIFOW(fd,26) == 0) {
					strncpy(auth_dat[i].memo, "-", size_of_memo);
				} else if (RFIFOW(fd,26) > size_of_memo - 1) {
					memcpy(auth_dat[i].memo, RFIFOP(fd,28), size_of_memo - 1);
				} else {
					memcpy(auth_dat[i].memo, RFIFOP(fd,28), RFIFOW(fd,26));
				}
				auth_dat[i].memo[size_of_memo - 1] = '\0';
				remove_control_chars((unsigned char *)auth_dat[i].memo);
				WFIFOL(fd,2) = auth_dat[i].account_id;
				login_log("'ladmin': Modification of a memo field (account: %s, new memo: %s, ip: %s)" RETCODE,
				          auth_dat[i].userid, auth_dat[i].memo, ip);
				mmo_auth_sync();
			} else {
				memcpy(WFIFOP(fd,6), account_name, 24);
				login_log("'ladmin': Attempt to modify the memo field of an unknown account (account: %s, ip: %s)" RETCODE,
				          account_name, ip);
			}
			WFIFOSET(fd,30);
			RFIFOSKIP(fd,28 + RFIFOW(fd,26));
			break;

		case 0x7944:	// Request to found an account id
			if (RFIFOREST(fd) < 26)
				return 0;
			WFIFOW(fd,0) = 0x7945;
			WFIFOL(fd,2) = 0xFFFFFFFF; // WTF???
			account_name = (char*)RFIFOP(fd,2);
			account_name[23] = '\0';
			remove_control_chars((unsigned char *)account_name);
			i = search_account_index(account_name);
			if (i != -1) {
				memcpy(WFIFOP(fd,6), auth_dat[i].userid, 24);
				WFIFOL(fd,2) = auth_dat[i].account_id;
				login_log("'ladmin': Request (by the name) of an account id (account: %s, id: %d, ip: %s)" RETCODE,
				          auth_dat[i].userid, auth_dat[i].account_id, ip);
			} else {
				memcpy(WFIFOP(fd,6), account_name, 24);
				login_log("'ladmin': ID request (by the name) of an unknown account (account: %s, ip: %s)" RETCODE,
				          account_name, ip);
			}
			WFIFOSET(fd,30);
			RFIFOSKIP(fd,26);
			break;

		case 0x7946:	// Request to found an account name
			if (RFIFOREST(fd) < 6)
				return 0;
			WFIFOW(fd,0) = 0x7947;
			WFIFOL(fd,2) = RFIFOL(fd,2);
			memset(WFIFOP(fd,6), '\0', 24);
			for(i = 0; i < auth_num; i++) {
				if (auth_dat[i].account_id == RFIFOL(fd,2)) {
					strncpy((char*)WFIFOP(fd,6), auth_dat[i].userid, 24);
					login_log("'ladmin': Request (by id) of an account name (account: %s, id: %d, ip: %s)" RETCODE,
					          auth_dat[i].userid, RFIFOL(fd,2), ip);
					break;
				}
			}
			if (i == auth_num) {
				login_log("'ladmin': Name request (by id) of an unknown account (id: %d, ip: %s)" RETCODE,
				          RFIFOL(fd,2), ip);
				strncpy((char*)WFIFOP(fd,6), "", 24);
			}
			WFIFOSET(fd,30);
			RFIFOSKIP(fd,6);
			break;

		case 0x7948:	// Request to change the validity limit (timestamp) (absolute value)
			if (RFIFOREST(fd) < 30)
				return 0;
			{
				time_t timestamp;
				char tmpstr[2048];
				WFIFOW(fd,0) = 0x7949;
				WFIFOL(fd,2) = 0xFFFFFFFF; // WTF???
				account_name = (char*)RFIFOP(fd,2);
				account_name[23] = '\0';
				remove_control_chars((unsigned char *)account_name);
				timestamp = (time_t)RFIFOL(fd,26);
				strftime(tmpstr, 24, date_format, localtime(&timestamp));
				i = search_account_index(account_name);
				if (i != -1) {
					memcpy(WFIFOP(fd,6), auth_dat[i].userid, 24);
					login_log("'ladmin': Change of a validity limit (account: %s, new validity: %d (%s), ip: %s)" RETCODE,
					          auth_dat[i].userid, timestamp, (timestamp == 0 ? "unlimited" : tmpstr), ip);
					auth_dat[i].connect_until_time = timestamp;
					WFIFOL(fd,2) = auth_dat[i].account_id;
					mmo_auth_sync();
				} else {
					memcpy(WFIFOP(fd,6), account_name, 24);
					login_log("'ladmin': Attempt to change the validity limit of an unknown account (account: %s, received validity: %d (%s), ip: %s)" RETCODE,
					          account_name, timestamp, (timestamp == 0 ? "unlimited" : tmpstr), ip);
				}
				WFIFOL(fd,30) = (unsigned int)timestamp;
			}
			WFIFOSET(fd,34);
			RFIFOSKIP(fd,30);
			break;

		case 0x794a:	// Request to change the final date of a banishment (timestamp) (absolute value)
			if (RFIFOREST(fd) < 30)
				return 0;
			{
				time_t timestamp;
				char tmpstr[2048];
				WFIFOW(fd,0) = 0x794b;
				WFIFOL(fd,2) = 0xFFFFFFFF; // WTF???
				account_name = (char*)RFIFOP(fd,2);
				account_name[23] = '\0';
				remove_control_chars((unsigned char *)account_name);
				timestamp = (time_t)RFIFOL(fd,26);
				if (timestamp <= time(NULL))
					timestamp = 0;
				strftime(tmpstr, 24, date_format, localtime(&timestamp));
				i = search_account_index(account_name);
				if (i != -1) {
					memcpy(WFIFOP(fd,6), auth_dat[i].userid, 24);
					WFIFOL(fd,2) = auth_dat[i].account_id;
					login_log("'ladmin': Change of the final date of a banishment (account: %s, new final date of banishment: %d (%s), ip: %s)" RETCODE,
					          auth_dat[i].userid, timestamp, (timestamp == 0 ? "no banishment" : tmpstr), ip);
					if (auth_dat[i].ban_until_time != timestamp) {
						if (timestamp != 0) {
							unsigned char buf[16];
							WBUFW(buf,0) = 0x2731;
							WBUFL(buf,2) = auth_dat[i].account_id;
							WBUFB(buf,6) = 1; // 0: change of statut, 1: ban
							WBUFL(buf,7) = (unsigned int)timestamp; // status or final date of a banishment
							charif_sendallwos(-1, buf, 11);
							for(j = 0; j < AUTH_FIFO_SIZE; j++)
								if (auth_fifo[j].account_id == auth_dat[i].account_id)
									auth_fifo[j].login_id1++; // to avoid reconnection error when come back from map-server (char-server will ask again the authentification)
						}
						auth_dat[i].ban_until_time = timestamp;
						mmo_auth_sync();
					}
				} else {
					memcpy(WFIFOP(fd,6), account_name, 24);
					login_log("'ladmin': Attempt to change the final date of a banishment of an unknown account (account: %s, received final date of banishment: %d (%s), ip: %s)" RETCODE,
					          account_name, timestamp, (timestamp == 0 ? "no banishment" : tmpstr), ip);
				}
				WFIFOL(fd,30) = (unsigned int)timestamp;
			}
			WFIFOSET(fd,34);
			RFIFOSKIP(fd,30);
			break;

		case 0x794c:	// Request to change the final date of a banishment (timestamp) (relative change)
			if (RFIFOREST(fd) < 38)
				return 0;
			{
				time_t timestamp;
				struct tm *tmtime;
				char tmpstr[2048];
				WFIFOW(fd,0) = 0x794d;
				WFIFOL(fd,2) = 0xFFFFFFFF; // WTF???
				account_name = (char*)RFIFOP(fd,2);
				account_name[23] = '\0';
				remove_control_chars((unsigned char *)account_name);
				i = search_account_index(account_name);
				if (i != -1) {
					WFIFOL(fd,2) = auth_dat[i].account_id;
					memcpy(WFIFOP(fd,6), auth_dat[i].userid, 24);
					if (auth_dat[i].ban_until_time == 0 || auth_dat[i].ban_until_time < time(NULL))
						timestamp = time(NULL);
					else
						timestamp = auth_dat[i].ban_until_time;
					tmtime = localtime(&timestamp);
					tmtime->tm_year = tmtime->tm_year + (short)RFIFOW(fd,26);
					tmtime->tm_mon = tmtime->tm_mon + (short)RFIFOW(fd,28);
					tmtime->tm_mday = tmtime->tm_mday + (short)RFIFOW(fd,30);
					tmtime->tm_hour = tmtime->tm_hour + (short)RFIFOW(fd,32);
					tmtime->tm_min = tmtime->tm_min + (short)RFIFOW(fd,34);
					tmtime->tm_sec = tmtime->tm_sec + (short)RFIFOW(fd,36);
					timestamp = mktime(tmtime);
					if (timestamp != -1) {
						if (timestamp <= time(NULL))
							timestamp = 0;
						strftime(tmpstr, 24, date_format, localtime(&timestamp));
						login_log("'ladmin': Adjustment of a final date of a banishment (account: %s, (%+d y %+d m %+d d %+d h %+d mn %+d s) -> new validity: %d (%s), ip: %s)" RETCODE,
						          auth_dat[i].userid, (short)RFIFOW(fd,26), (short)RFIFOW(fd,28), (short)RFIFOW(fd,30), (short)RFIFOW(fd,32), (short)RFIFOW(fd,34), (short)RFIFOW(fd,36), timestamp, (timestamp == 0 ? "no banishment" : tmpstr), ip);
						if (auth_dat[i].ban_until_time != timestamp) {
							if (timestamp != 0) {
								unsigned char buf[16];
								WBUFW(buf,0) = 0x2731;
								WBUFL(buf,2) = auth_dat[i].account_id;
								WBUFB(buf,6) = 1; // 0: change of statut, 1: ban
								WBUFL(buf,7) = (unsigned int)timestamp; // status or final date of a banishment
								charif_sendallwos(-1, buf, 11);
								for(j = 0; j < AUTH_FIFO_SIZE; j++)
									if (auth_fifo[j].account_id == auth_dat[i].account_id)
										auth_fifo[j].login_id1++; // to avoid reconnection error when come back from map-server (char-server will ask again the authentification)
							}
							auth_dat[i].ban_until_time = timestamp;
							mmo_auth_sync();
						}
					} else {
						strftime(tmpstr, 24, date_format, localtime(&auth_dat[i].ban_until_time));
						login_log("'ladmin': Impossible to adjust the final date of a banishment (account: %s, %d (%s) + (%+d y %+d m %+d d %+d h %+d mn %+d s) -> ???, ip: %s)" RETCODE,
						          auth_dat[i].userid, auth_dat[i].ban_until_time, (auth_dat[i].ban_until_time == 0 ? "no banishment" : tmpstr), (short)RFIFOW(fd,26), (short)RFIFOW(fd,28), (short)RFIFOW(fd,30), (short)RFIFOW(fd,32), (short)RFIFOW(fd,34), (short)RFIFOW(fd,36), ip);
					}
					WFIFOL(fd,30) = (unsigned long)auth_dat[i].ban_until_time;
				} else {
					memcpy(WFIFOP(fd,6), account_name, 24);
					login_log("'ladmin': Attempt to adjust the final date of a banishment of an unknown account (account: %s, ip: %s)" RETCODE,
					          account_name, ip);
					WFIFOL(fd,30) = 0;
				}
			}
			WFIFOSET(fd,34);
			RFIFOSKIP(fd,38);
			break;

		case 0x794e:	// Request to send a broadcast message
			if (RFIFOREST(fd) < 8 || RFIFOREST(fd) < (8 + RFIFOL(fd,4)))
				return 0;
			WFIFOW(fd,0) = 0x794f;
			WFIFOW(fd,2) = 0xFFFF; // WTF???
			if (RFIFOL(fd,4) < 1) {
				login_log("'ladmin': Receiving a message for broadcast, but message is void (ip: %s)" RETCODE,
				          ip);
			} else {
				// at least 1 char-server
				for(i = 0; i < MAX_SERVERS; i++)
					if (server_fd[i] >= 0)
						break;
				if (i == MAX_SERVERS) {
					login_log("'ladmin': Receiving a message for broadcast, but no char-server is online (ip: %s)" RETCODE,
					          ip);
				} else {
					unsigned char buf[32000];
					char message[32000];
					WFIFOW(fd,2) = 0;
					memset(message, '\0', sizeof(message));
					memcpy(message, RFIFOP(fd,8), RFIFOL(fd,4));
					message[sizeof(message)-1] = '\0';
					remove_control_chars((unsigned char *)message);
					if (RFIFOW(fd,2) == 0)
						login_log("'ladmin': Receiving a message for broadcast (message (in yellow): %s, ip: %s)" RETCODE,
						          message, ip);
					else
						login_log("'ladmin': Receiving a message for broadcast (message (in blue): %s, ip: %s)" RETCODE,
						          message, ip);
					// send same message to all char-servers (no answer)
					memcpy(WBUFP(buf,0), RFIFOP(fd,0), 8 + RFIFOL(fd,4));
					WBUFW(buf,0) = 0x2726;
					charif_sendallwos(-1, buf, 8 + RFIFOL(fd,4));
				}
			}
			WFIFOSET(fd,4);
			RFIFOSKIP(fd,8 + RFIFOL(fd,4));
			break;

		case 0x7950:	// Request to change the validity limite (timestamp) (relative change)
			if (RFIFOREST(fd) < 38)
				return 0;
			{
				time_t timestamp;
				struct tm *tmtime;
				char tmpstr[2048];
				char tmpstr2[2048];
				WFIFOW(fd,0) = 0x7951;
				WFIFOL(fd,2) = 0xFFFFFFFF; // WTF???
				account_name = (char*)RFIFOP(fd,2);
				account_name[23] = '\0';
				remove_control_chars((unsigned char *)account_name);
				i = search_account_index(account_name);
				if (i != -1) {
					WFIFOL(fd,2) = auth_dat[i].account_id;
					memcpy(WFIFOP(fd,6), auth_dat[i].userid, 24);
					timestamp = auth_dat[i].connect_until_time;
					if (add_to_unlimited_account == 0 && timestamp == 0) {
						login_log("'ladmin': Attempt to adjust the validity limit of an unlimited account (account: %s, ip: %s)" RETCODE,
						          auth_dat[i].userid, ip);
						WFIFOL(fd,30) = 0;
					} else {
						if (timestamp == 0 || timestamp < time(NULL))
							timestamp = time(NULL);
						tmtime = localtime(&timestamp);
						tmtime->tm_year = tmtime->tm_year + (short)RFIFOW(fd,26);
						tmtime->tm_mon = tmtime->tm_mon + (short)RFIFOW(fd,28);
						tmtime->tm_mday = tmtime->tm_mday + (short)RFIFOW(fd,30);
						tmtime->tm_hour = tmtime->tm_hour + (short)RFIFOW(fd,32);
						tmtime->tm_min = tmtime->tm_min + (short)RFIFOW(fd,34);
						tmtime->tm_sec = tmtime->tm_sec + (short)RFIFOW(fd,36);
						timestamp = mktime(tmtime);
						if (timestamp != -1) {
							strftime(tmpstr, 24, date_format, localtime(&auth_dat[i].connect_until_time));
							strftime(tmpstr2, 24, date_format, localtime(&timestamp));
							login_log("'ladmin': Adjustment of a validity limit (account: %s, %d (%s) + (%+d y %+d m %+d d %+d h %+d mn %+d s) -> new validity: %d (%s), ip: %s)" RETCODE,
							          auth_dat[i].userid, auth_dat[i].connect_until_time, (auth_dat[i].connect_until_time == 0 ? "unlimited" : tmpstr), (short)RFIFOW(fd,26), (short)RFIFOW(fd,28), (short)RFIFOW(fd,30), (short)RFIFOW(fd,32), (short)RFIFOW(fd,34), (short)RFIFOW(fd,36), timestamp, (timestamp == 0 ? "unlimited" : tmpstr2), ip);
							auth_dat[i].connect_until_time = timestamp;
							mmo_auth_sync();
							WFIFOL(fd,30) = (unsigned long)auth_dat[i].connect_until_time;
						} else {
							strftime(tmpstr, 24, date_format, localtime(&auth_dat[i].connect_until_time));
							login_log("'ladmin': Impossible to adjust a validity limit (account: %s, %d (%s) + (%+d y %+d m %+d d %+d h %+d mn %+d s) -> ???, ip: %s)" RETCODE,
							          auth_dat[i].userid, auth_dat[i].connect_until_time, (auth_dat[i].connect_until_time == 0 ? "unlimited" : tmpstr), (short)RFIFOW(fd,26), (short)RFIFOW(fd,28), (short)RFIFOW(fd,30), (short)RFIFOW(fd,32), (short)RFIFOW(fd,34), (short)RFIFOW(fd,36), ip);
							WFIFOL(fd,30) = 0;
						}
					}
				} else {
					memcpy(WFIFOP(fd,6), account_name, 24);
					login_log("'ladmin': Attempt to adjust the validity limit of an unknown account (account: %s, ip: %s)" RETCODE,
					          account_name, ip);
					WFIFOL(fd,30) = 0;
				}
			}
			WFIFOSET(fd,34);
			RFIFOSKIP(fd,38);
			break;

		case 0x7952:	// Request about informations of an account (by account name)
			if (RFIFOREST(fd) < 26)
				return 0;
			WFIFOW(fd,0) = 0x7953;
			WFIFOL(fd,2) = 0xFFFFFFFF; // WTF???
			account_name = (char*)RFIFOP(fd,2);
			account_name[23] = '\0';
			remove_control_chars((unsigned char *)account_name);
			i = search_account_index(account_name);
			if (i != -1) {
				WFIFOL(fd,2) = auth_dat[i].account_id;
				WFIFOB(fd,6) = (unsigned char)isGM(auth_dat[i].account_id);
				memcpy(WFIFOP(fd,7), auth_dat[i].userid, 24);
				WFIFOB(fd,31) = auth_dat[i].sex;
				WFIFOL(fd,32) = auth_dat[i].logincount;
				WFIFOL(fd,36) = auth_dat[i].state;
				memcpy(WFIFOP(fd,40), auth_dat[i].error_message, 20);
				memcpy(WFIFOP(fd,60), auth_dat[i].lastlogin, 24);
				memcpy(WFIFOP(fd,84), auth_dat[i].last_ip, 16);
				memcpy(WFIFOP(fd,100), auth_dat[i].email, 40);
				WFIFOL(fd,140) = (unsigned long)auth_dat[i].connect_until_time;
				WFIFOL(fd,144) = (unsigned long)auth_dat[i].ban_until_time;
				WFIFOW(fd,148) = strlen(auth_dat[i].memo);
				if (auth_dat[i].memo[0]) {
					memcpy(WFIFOP(fd,150), auth_dat[i].memo, strlen(auth_dat[i].memo));
				}
				login_log("'ladmin': Sending information of an account (request by the name; account: %s, id: %d, ip: %s)" RETCODE,
				          auth_dat[i].userid, auth_dat[i].account_id, ip);
				WFIFOSET(fd,150+strlen(auth_dat[i].memo));
			} else {
				memcpy(WFIFOP(fd,7), account_name, 24);
				WFIFOW(fd,148) = 0;
				login_log("'ladmin': Attempt to obtain information (by the name) of an unknown account (account: %s, ip: %s)" RETCODE,
				          account_name, ip);
				WFIFOSET(fd,150);
			}
			RFIFOSKIP(fd,26);
			break;

		case 0x7954:	// Request about information of an account (by account id)
			if (RFIFOREST(fd) < 6)
				return 0;
			WFIFOW(fd,0) = 0x7953;
			WFIFOL(fd,2) = RFIFOL(fd,2);
			memset(WFIFOP(fd,7), '\0', 24);
			for(i = 0; i < auth_num; i++) {
				if (auth_dat[i].account_id == RFIFOL(fd,2)) {
					login_log("'ladmin': Sending information of an account (request by the id; account: %s, id: %d, ip: %s)" RETCODE,
					          auth_dat[i].userid, RFIFOL(fd,2), ip);
					WFIFOB(fd,6) = (unsigned char)isGM(auth_dat[i].account_id);
					memcpy(WFIFOP(fd,7), auth_dat[i].userid, 24);
					WFIFOB(fd,31) = auth_dat[i].sex;
					WFIFOL(fd,32) = auth_dat[i].logincount;
					WFIFOL(fd,36) = auth_dat[i].state;
					memcpy(WFIFOP(fd,40), auth_dat[i].error_message, 20);
					memcpy(WFIFOP(fd,60), auth_dat[i].lastlogin, 24);
					memcpy(WFIFOP(fd,84), auth_dat[i].last_ip, 16);
					memcpy(WFIFOP(fd,100), auth_dat[i].email, 40);
					WFIFOL(fd,140) = (unsigned long)auth_dat[i].connect_until_time;
					WFIFOL(fd,144) = (unsigned long)auth_dat[i].ban_until_time;
					WFIFOW(fd,148) = strlen(auth_dat[i].memo);
					if (auth_dat[i].memo[0]) {
						memcpy(WFIFOP(fd,150), auth_dat[i].memo, strlen(auth_dat[i].memo));
					}
					WFIFOSET(fd,150+strlen(auth_dat[i].memo));
					break;
				}
			}
			if (i == auth_num) {
				login_log("'ladmin': Attempt to obtain information (by the id) of an unknown account (id: %d, ip: %s)" RETCODE,
				          RFIFOL(fd,2), ip);
				strncpy((char*)WFIFOP(fd,7), "", 24);
				WFIFOW(fd,148) = 0;
				WFIFOSET(fd,150);
			}
			RFIFOSKIP(fd,6);
			break;

		case 0x7955:	// Request to reload GM file (no answer)
			login_log("'ladmin': Request to re-load GM configuration file (ip: %s)." RETCODE, ip);
			read_gm_account();
			// send GM accounts to all char-servers
			send_GM_accounts();
			RFIFOSKIP(fd,2);
			break;

		default:
			{
				FILE *logfp;
				char tmpstr[24];
				time_t raw_time;
				logfp = fopen(login_log_unknown_packets_filename, "a");
				if (logfp) {
					time(&raw_time);
					strftime(tmpstr, 23, date_format, localtime(&raw_time));
					fprintf(logfp, "%s: receiving of an unknown packet -> disconnection" RETCODE, tmpstr);
					fprintf(logfp, "parse_admin: connection #%d (ip: %s), packet: 0x%x (with being read: %d)." RETCODE, fd, ip, RFIFOW(fd,0), RFIFOREST(fd));
					fprintf(logfp, "Detail (in hex):" RETCODE);
					fprintf(logfp, "---- 00-01-02-03-04-05-06-07  08-09-0A-0B-0C-0D-0E-0F" RETCODE);
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
							fprintf(logfp, " %s" RETCODE, tmpstr);
							memset(tmpstr, '\0', sizeof(tmpstr));
						}
					}
					if (i % 16 != 0) {
						for(j = i; j % 16 != 0; j++) {
							fprintf(logfp, "   ");
							if ((j - 7) % 16 == 0) // -8 + 1
								fprintf(logfp, " ");
						}
						fprintf(logfp, " %s" RETCODE, tmpstr);
					}
					fprintf(logfp, RETCODE);
					fclose(logfp);
				}
			}
			login_log("'ladmin': End of connection, unknown packet (ip: %s)" RETCODE, ip);
			session[fd]->eof = 1;
			ShowWarning("Remote administration has been disconnected (unknown packet).\n");
			return 0;
		}
		//WFIFOW(fd,0) = 0x791f;
		//WFIFOSET(fd,2);
	}
	RFIFOSKIP(fd,RFIFOREST(fd));
	return 0;
}

//--------------------------------------------
// Test to know if an IP come from LAN or WAN.
// Rewrote: Adnvanced subnet check [LuzZza]
//--------------------------------------------
int lan_subnetcheck(long *p) {

	int i;
	unsigned char *sbn, *msk, *src = (unsigned char *)p;
	
	for(i=0; i<subnet_count; i++) {
	
		if(subnet[i].subnet == (*p & subnet[i].mask)) {
			
			sbn = (char *)&subnet[i].subnet;
			msk = (char *)&subnet[i].mask;
			
			ShowInfo("Subnet check [%u.%u.%u.%u]: Matches "CL_CYAN"%u.%u.%u.%u/%u.%u.%u.%u"CL_RESET"\n",
				src[0], src[1], src[2], src[3], sbn[0], sbn[1], sbn[2], sbn[3], msk[0], msk[1], msk[2], msk[3]);
			
			return subnet[i].char_ip;
		}
	}
	
	ShowInfo("Subnet check [%u.%u.%u.%u]: "CL_CYAN"WAN"CL_RESET"\n", src[0], src[1], src[2], src[3]);
	return 0;
}

//----------------------------------------------------------------------------------------
// Default packet parsing (normal players or administation/char-server connexion requests)
//----------------------------------------------------------------------------------------
int parse_login(int fd) {
	
	struct mmo_account account;
	int result, j;
	unsigned int i;
	unsigned char *p = (unsigned char *) &session[fd]->client_addr.sin_addr;
	char ip[16];
	long subnet_char_ip;
	
	RFIFOHEAD(fd);

	sprintf(ip, "%d.%d.%d.%d", p[0], p[1], p[2], p[3]);

        memset(&account, 0, sizeof(account));

	if (session[fd]->eof) {
		do_close(fd);
		return 0;
	}

	while(RFIFOREST(fd) >= 2) {
		if (display_parse_login == 1) {
			if (RFIFOW(fd,0) == 0x64 || RFIFOW(fd,0) == 0x01dd) {
				if ((int)RFIFOREST(fd) >= ((RFIFOW(fd,0) == 0x64) ? 55 : 47))
					ShowDebug("parse_login: connection #%d, packet: 0x%x (with being read: %d), account: %s.\n", fd, RFIFOW(fd,0), RFIFOREST(fd), RFIFOP(fd,6));
			} else if (RFIFOW(fd,0) == 0x2710) {
				if (RFIFOREST(fd) >= 86)
					ShowDebug("parse_login: connection #%d, packet: 0x%x (with being read: %d), server: %s.\n", fd, RFIFOW(fd,0), RFIFOREST(fd), RFIFOP(fd,60));
			} else
				ShowDebug("parse_login: connection #%d, packet: 0x%x (with being read: %d).\n", fd, RFIFOW(fd,0), RFIFOREST(fd));
		}

		switch(RFIFOW(fd,0)) {
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
		{
			int packet_len = RFIFOREST(fd);

			switch(RFIFOW(fd, 0)){
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
			
			account.version = RFIFOL(fd, 2);	//for exe version check [Sirius]
			if (!account.version) account.version = 1; //Force some version...
			memcpy(account.userid,RFIFOP(fd,6),NAME_LENGTH);
			account.userid[23] = '\0';
			remove_control_chars((unsigned char *)account.userid);
			if (RFIFOW(fd,0) != 0x01dd) {
				login_log("Request for connection (non encryption mode) of %s (ip: %s)." RETCODE, account.userid, ip);
				memcpy(account.passwd, RFIFOP(fd,30), NAME_LENGTH);
				account.passwd[23] = '\0';
				remove_control_chars((unsigned char *)account.passwd);
			} else {
				login_log("Request for connection (encryption mode) of %s (ip: %s)." RETCODE, account.userid, ip);
				 // If remove control characters from received password encrypted by md5,
				// there would be a wrong result and failed to authentication. [End_of_exam]
				memcpy(account.passwd, RFIFOP(fd,30), 16);
				account.passwd[16] = '\0';
			}
#ifdef PASSWORDENC
			account.passwdenc = (RFIFOW(fd,0) != 0x01dd) ? 0 : PASSWORDENC;
#else
			account.passwdenc = 0;
#endif

			if (!check_ip(session[fd]->client_addr.sin_addr.s_addr)) {
				login_log("Connection refused: IP isn't authorised (deny/allow, ip: %s)." RETCODE, ip);
				WFIFOHEAD(fd, 23);
				WFIFOW(fd,0) = 0x6a;
				WFIFOB(fd,2) = 3; // 3 = Rejected from Server
				WFIFOSET(fd,23);
				RFIFOSKIP(fd,packet_len);
				break;
			}

			result = mmo_auth(&account, fd);
			if (result == -1) {
				int gm_level = isGM(account.account_id);
				if (min_level_to_connect > gm_level) {
					login_log("Connection refused: the minimum GM level for connection is %d (account: %s, GM level: %d, ip: %s)." RETCODE,
					          min_level_to_connect, account.userid, gm_level, ip);
					WFIFOHEAD(fd, 3);
					WFIFOW(fd,0) = 0x81;
					WFIFOB(fd,2) = 1; // 01 = Server closed
					WFIFOSET(fd,3);
				} else {
					if (gm_level)
						ShowInfo("Connection of the GM (level:%d) account '%s' accepted.\n", gm_level, account.userid);
					else
						ShowInfo("Connection of the account '%s' accepted.\n", account.userid);
					server_num = 0;
					WFIFOHEAD(fd, 47+32*MAX_SERVERS);
					for(i = 0; i < MAX_SERVERS; i++) {
						if (server_fd[i] >= 0) {
						    // Andvanced subnet check [LuzZza]
							if((subnet_char_ip = lan_subnetcheck((long*)p)))
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
						WFIFOW(fd,0) = 0x69;
						WFIFOW(fd,2) = 47+32*server_num;
						WFIFOL(fd,4) = account.login_id1;
						WFIFOL(fd,8) = account.account_id;
						WFIFOL(fd,12) = account.login_id2;
						WFIFOL(fd,16) = 0; // in old version, that was for ip (not more used)
						memcpy(WFIFOP(fd,20), account.lastlogin, 24); // in old version, that was for name (not more used)
						WFIFOB(fd,46) = account.sex;
						WFIFOSET(fd,47+32*server_num);
						if (auth_fifo_pos >= AUTH_FIFO_SIZE)
							auth_fifo_pos = 0;
						auth_fifo[auth_fifo_pos].account_id = account.account_id;
						auth_fifo[auth_fifo_pos].login_id1 = account.login_id1;
						auth_fifo[auth_fifo_pos].login_id2 = account.login_id2;
						auth_fifo[auth_fifo_pos].sex = account.sex;
						auth_fifo[auth_fifo_pos].delflag = 0;
						auth_fifo[auth_fifo_pos].ip = session[fd]->client_addr.sin_addr.s_addr;
						auth_fifo_pos++;
					// if no char-server, don't send void list of servers, just disconnect the player with proper message
					} else {
						login_log("Connection refused: there is no char-server online (account: %s, ip: %s)." RETCODE,
						          account.userid, ip);
						WFIFOW(fd,0) = 0x81;
						WFIFOB(fd,2) = 1; // 01 = Server closed
						WFIFOSET(fd,3);
					}
				}
			} else {
                                WFIFOHEAD(fd, 23);
				memset(WFIFOP(fd,0), '\0', 23);
				WFIFOW(fd,0) = 0x6a;
				WFIFOB(fd,2) = result;
				if (result == 6) { // 6 = Your are Prohibited to log in until %s
					i = search_account_index(account.userid);
					if (i != -1) {
						if (auth_dat[i].ban_until_time != 0) { // if account is banned, we send ban timestamp
							char tmpstr[256];
							strftime(tmpstr, 20, date_format, localtime(&auth_dat[i].ban_until_time));
							tmpstr[19] = '\0';
							memcpy(WFIFOP(fd,3), tmpstr, 20);
						} else { // we send error message
							memcpy(WFIFOP(fd,3), auth_dat[i].error_message, 20);
						}
					}
				}
				WFIFOSET(fd,23);
			}
			RFIFOSKIP(fd,packet_len);
			break;
		}
		case 0x01db:	// Sending request of the coding key
		case 0x791a:	// Sending request of the coding key (administration packet)
			{
				struct login_session_data *ld;
				if (session[fd]->session_data) {
					ShowWarning("login: abnormal request of MD5 key (already opened session).\n");
					session[fd]->eof = 1;
					return 0;
				}
				ld = (struct login_session_data*)aCalloc(1, sizeof(struct login_session_data));
                                session[fd]->session_data = ld;
				if (!ld) {
					ShowFatalError("login: Request for md5 key: memory allocation failure (malloc)!\n");
					session[fd]->eof = 1;
					return 0;
				}
				if (RFIFOW(fd,0) == 0x01db)
					login_log("Sending request of the coding key (ip: %s)" RETCODE, ip);
				else
					login_log("'ladmin': Sending request of the coding key (ip: %s)" RETCODE, ip);
				// Creation of the coding key
				memset(ld->md5key, '\0', sizeof(ld->md5key));
				ld->md5keylen = rand() % 4 + 12;
				for(i = 0; i < ld->md5keylen; i++)
					ld->md5key[i] = rand() % 255 + 1;
				RFIFOSKIP(fd,2);
                                WFIFOHEAD(fd, 4 + ld->md5keylen);
				WFIFOW(fd,0) = 0x01dc;
				WFIFOW(fd,2) = 4 + ld->md5keylen;
				memcpy(WFIFOP(fd,4), ld->md5key, ld->md5keylen);
				WFIFOSET(fd,WFIFOW(fd,2));
			}
			break;

		case 0x2710:	// Connection request of a char-server
			if (RFIFOREST(fd) < 86)
				return 0;
			{
				int GM_value, len;
				char* server_name;
 				WFIFOHEAD(fd, 3);
				memcpy(account.userid,RFIFOP(fd,2),NAME_LENGTH);
				account.userid[23] = '\0';
				remove_control_chars((unsigned char *)account.userid);
				memcpy(account.passwd, RFIFOP(fd,26), NAME_LENGTH);
				account.passwd[23] = '\0';
				remove_control_chars((unsigned char *)account.passwd);
				account.passwdenc = 0;
				server_name = (char*)RFIFOP(fd,60);
				server_name[20] = '\0';
				remove_control_chars((unsigned char *)server_name);
				login_log("Connection request of the char-server '%s' @ %d.%d.%d.%d:%d (ip: %s)" RETCODE,
				          server_name, RFIFOB(fd,54), RFIFOB(fd,55), RFIFOB(fd,56), RFIFOB(fd,57), RFIFOW(fd,58), ip);
				result = mmo_auth(&account, fd);
				if (result == -1 && account.sex == 2 && account.account_id < MAX_SERVERS && server_fd[account.account_id] == -1) {
					login_log("Connection of the char-server '%s' accepted (account: %s, pass: %s, ip: %s)" RETCODE,
					          server_name, account.userid, account.passwd, ip);
					ShowStatus("Connection of the char-server '%s' accepted.\n", server_name);
					memset(&server[account.account_id], 0, sizeof(struct mmo_char_server));
					server[account.account_id].ip = RFIFOL(fd,54);
					server[account.account_id].port = RFIFOW(fd,58);
					memcpy(server[account.account_id].name, server_name, 20);
					server[account.account_id].users = 0;
					server[account.account_id].maintenance = RFIFOW(fd,82);
					server[account.account_id].new_ = RFIFOW(fd,84);
					server_fd[account.account_id] = fd;
					WFIFOW(fd,0) = 0x2711;
					WFIFOB(fd,2) = 0;
					WFIFOSET(fd,3);
					session[fd]->func_parse = parse_fromchar;
					realloc_fifo(fd, FIFOSIZE_SERVERLINK, FIFOSIZE_SERVERLINK);
					// send GM account to char-server
					len = 4;
					WFIFOW(fd,0) = 0x2732;
					for(i = 0; i < auth_num; i++)
						// send only existing accounts. We can not create a GM account when server is online.
						if ((GM_value = isGM(auth_dat[i].account_id)) > 0) {
							WFIFOL(fd,len) = auth_dat[i].account_id;
							WFIFOB(fd,len+4) = (unsigned char)GM_value;
							len += 5;
						}
					WFIFOW(fd,2) = len;
					WFIFOSET(fd,len);
				} else {
					if (server_fd[account.account_id] != -1) {
						ShowNotice("Connection of the char-server '%s' REFUSED - already connected (account: %ld-%s, pass: %s, ip: %s)\n",
					        server_name, account.account_id, account.userid, account.passwd, ip);
						login_log("Connexion of the char-server '%s' REFUSED - already connected (account: %ld-%s, pass: %s, ip: %s)" RETCODE,
					          server_name, account.account_id, account.userid, account.passwd, ip);
					} else {
						ShowNotice("Connection of the char-server '%s' REFUSED (account: %s, pass: %s, ip: %s).\n", server_name, account.userid, account.passwd, ip);
						login_log("Connexion of the char-server '%s' REFUSED (account: %s, pass: %s, ip: %s)" RETCODE,
					          	server_name, account.userid, account.passwd, ip);
					}
                                        WFIFOHEAD(fd, 3);
					WFIFOW(fd,0) = 0x2711;
					WFIFOB(fd,2) = 3;
					WFIFOSET(fd,3);
				}
			}
			RFIFOSKIP(fd,86);
			return 0;

		case 0x7530:	// Request of the server version
			login_log("Sending of the server version (ip: %s)" RETCODE, ip);
			WFIFOHEAD(fd, 10);
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
			login_log("End of connection (ip: %s)" RETCODE, ip);
			session[fd]->eof = 1;
			return 0;

		case 0x7918:	// Request for administation login
			if ((int)RFIFOREST(fd) < 4 || (int)RFIFOREST(fd) < ((RFIFOW(fd,2) == 0) ? 28 : 20))
				return 0;
			WFIFOW(fd,0) = 0x7919;
			WFIFOB(fd,2) = 1;
			if (!check_ladminip(session[fd]->client_addr.sin_addr.s_addr)) {
				login_log("'ladmin'-login: Connection in administration mode refused: IP isn't authorised (ladmin_allow, ip: %s)." RETCODE, ip);
			} else {
				struct login_session_data *ld = (struct login_session_data*)session[fd]->session_data;
				if (RFIFOW(fd,2) == 0) {	// non encrypted password
					char password[25];
					memcpy(password, RFIFOP(fd,4), 24);
					password[24] = '\0';
					remove_control_chars((unsigned char *)password);
					// If remote administration is enabled and password sent by client matches password read from login server configuration file
					if ((admin_state == 1) && (strcmp(password, admin_pass) == 0)) {
						login_log("'ladmin'-login: Connection in administration mode accepted (non encrypted password: %s, ip: %s)" RETCODE, password, ip);
						ShowNotice("Connection of a remote administration accepted (non encrypted password).\n");
						WFIFOB(fd,2) = 0;
						session[fd]->func_parse = parse_admin;
					} else if (admin_state != 1)
						login_log("'ladmin'-login: Connection in administration mode REFUSED - remote administration is disabled (non encrypted password: %s, ip: %s)" RETCODE, password, ip);
					else
						login_log("'ladmin'-login: Connection in administration mode REFUSED - invalid password (non encrypted password: %s, ip: %s)" RETCODE, password, ip);
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
						// If remote administration is enabled and password hash sent by client matches hash of password read from login server configuration file
						if ((admin_state == 1) && (memcmp(md5bin, RFIFOP(fd,4), 16) == 0)) {
							login_log("'ladmin'-login: Connection in administration mode accepted (encrypted password, ip: %s)" RETCODE, ip);
							ShowNotice("Connection of a remote administration accepted (encrypted password).\n");
							WFIFOB(fd,2) = 0;
							session[fd]->func_parse = parse_admin;
						} else if (admin_state != 1)
							login_log("'ladmin'-login: Connection in administration mode REFUSED - remote administration is disabled (encrypted password, ip: %s)" RETCODE, ip);
						else
							login_log("'ladmin'-login: Connection in administration mode REFUSED - invalid password (encrypted password, ip: %s)" RETCODE, ip);
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
					strftime(tmpstr, 23, date_format, localtime(&raw_time));
					fprintf(logfp, "%s: receiving of an unknown packet -> disconnection" RETCODE, tmpstr);
					fprintf(logfp, "parse_login: connection #%d (ip: %s), packet: 0x%x (with being read: %d)." RETCODE, fd, ip, RFIFOW(fd,0), RFIFOREST(fd));
					fprintf(logfp, "Detail (in hex):" RETCODE);
					fprintf(logfp, "---- 00-01-02-03-04-05-06-07  08-09-0A-0B-0C-0D-0E-0F" RETCODE);
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
							fprintf(logfp, " %s" RETCODE, tmpstr);
							memset(tmpstr, '\0', sizeof(tmpstr));
						}
					}
					if (i % 16 != 0) {
						for(j = i; j % 16 != 0; j++) {
							fprintf(logfp, "   ");
							if ((j - 7) % 16 == 0) // -8 + 1
								fprintf(logfp, " ");
						}
						fprintf(logfp, " %s" RETCODE, tmpstr);
					}
					fprintf(logfp, RETCODE);
					fclose(logfp);
				}
			}
			login_log("End of connection, unknown packet (ip: %s)" RETCODE, ip);
			session[fd]->eof = 1;
			return 0;
		}
	}
	RFIFOSKIP(fd,RFIFOREST(fd));
	return 0;
}

//-----------------------
// Console Command Parser [Wizputer]
//-----------------------
int parse_console(char *buf) {
	char command[256];

	memset(command,0,sizeof(command));

	sscanf(buf, "%[^\n]", command);

	login_log("Console command :%s" RETCODE, command);

	if(strcmpi("shutdown", command) == 0 ||
		strcmpi("exit", command) == 0 ||
		strcmpi("quit", command) == 0 ||
		strcmpi("end", command) == 0)
		runflag = 0;
	else if(strcmpi("alive", command) == 0 ||
		strcmpi("status", command) == 0)
		ShowInfo(CL_CYAN"Console: "CL_BOLD"I'm Alive."CL_RESET"\n");
	else if(strcmpi("help", command) == 0) {
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

//-----------------------------------
// Reading general configuration file
//-----------------------------------
int login_config_read(const char *cfgName) {
	char line[1024], w1[1024], w2[1024];
	FILE *fp;

	if ((fp = fopen(cfgName, "r")) == NULL) {
		ShowError("Configuration file (%s) not found.\n", cfgName);
		return 1;
	}

	ShowInfo("Reading configuration file %s...\n", cfgName);
	while(fgets(line, sizeof(line)-1, fp)) {
		if (line[0] == '/' && line[1] == '/')
			continue;

		line[sizeof(line)-1] = '\0';
		memset(w2, 0, sizeof(w2));
		if (sscanf(line, "%[^:]: %[^\r\n]", w1, w2) == 2) {
			remove_control_chars((unsigned char *)w1);
			remove_control_chars((unsigned char *)w2);

			if(strcmpi(w1,"timestamp_format") == 0) {
				strncpy(timestamp_format, w2, 20);
			} else if(strcmpi(w1,"stdout_with_ansisequence")==0){
				stdout_with_ansisequence = config_switch(w2);
			} else if(strcmpi(w1,"console_silent")==0){
				msg_silent = 0; //To always allow the next line to show up.
				ShowInfo("Console Silent Setting: %d\n", atoi(w2));
				msg_silent = atoi(w2);
			} else if (strcmpi(w1, "admin_state") == 0) {
				admin_state = config_switch(w2);
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
			} else if (strcmpi(w1, "new_account") == 0) {
				new_account_flag = config_switch(w2);
			} else if (strcmpi(w1, "bind_ip") == 0) {
				bind_ip = resolve_hostbyname(w2, NULL, bind_ip_str);
				if (bind_ip) 
					ShowStatus("Login server binding IP address : %s -> %s\n", w2, bind_ip_str);
			} else if (strcmpi(w1, "login_port") == 0) {
				login_port = atoi(w2);
			} else if (strcmpi(w1, "account_filename") == 0) {
				memset(account_filename, 0, sizeof(account_filename));
				strncpy(account_filename, w2, sizeof(account_filename));
				account_filename[sizeof(account_filename)-1] = '\0';
			} else if (strcmpi(w1, "gm_account_filename") == 0) {
				memset(GM_account_filename, 0, sizeof(GM_account_filename));
				strncpy(GM_account_filename, w2, sizeof(GM_account_filename));
				GM_account_filename[sizeof(GM_account_filename)-1] = '\0';
			} else if (strcmpi(w1, "gm_account_filename_check_timer") == 0) {
				gm_account_filename_check_timer = atoi(w2);
			} else if (strcmpi(w1, "use_MD5_passwords") == 0) {
				use_md5_passwds = config_switch(w2);
			} else if (strcmpi(w1, "login_log_filename") == 0) {
				memset(login_log_filename, 0, sizeof(login_log_filename));
				strncpy(login_log_filename, w2, sizeof(login_log_filename));
				login_log_filename[sizeof(login_log_filename)-1] = '\0';
			} else if (strcmpi(w1, "log_login") == 0) {
				log_login = atoi(w2);
			} else if (strcmpi(w1, "login_log_unknown_packets_filename") == 0) {
				memset(login_log_unknown_packets_filename, 0, sizeof(login_log_unknown_packets_filename));
				strncpy(login_log_unknown_packets_filename, w2, sizeof(login_log_unknown_packets_filename));
				login_log_unknown_packets_filename[sizeof(login_log_unknown_packets_filename)-1] = '\0';
			} else if (strcmpi(w1, "save_unknown_packets") == 0) {
				save_unknown_packets = config_switch(w2);
			} else if (strcmpi(w1, "display_parse_login") == 0) {
				display_parse_login = config_switch(w2); // 0: no, 1: yes
			} else if (strcmpi(w1, "display_parse_admin") == 0) {
				display_parse_admin = config_switch(w2); // 0: no, 1: yes
			} else if (strcmpi(w1, "display_parse_fromchar") == 0) {
				display_parse_fromchar = config_switch(w2); // 0: no, 1: yes (without packet 0x2714), 2: all packets
			} else if (strcmpi(w1, "date_format") == 0) { // note: never have more than 19 char for the date!
				memset(date_format, 0, sizeof(date_format));
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
			} else if (strcmpi(w1, "min_level_to_connect") == 0) {
				min_level_to_connect = atoi(w2);
			} else if (strcmpi(w1, "add_to_unlimited_account") == 0) {
				add_to_unlimited_account = config_switch(w2);
			} else if (strcmpi(w1, "start_limited_time") == 0) {
				start_limited_time = atoi(w2);
			} else if (strcmpi(w1, "check_ip_flag") == 0) {
				check_ip_flag = config_switch(w2);
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
			// dynamic password error ban
			} else if (strcmpi(w1, "dynamic_pass_failure_ban") == 0) {
				dynamic_pass_failure_ban = config_switch(w2);
			} else if (strcmpi(w1, "dynamic_pass_failure_ban_time") == 0) {
				dynamic_pass_failure_ban_time = atoi(w2);
			} else if (strcmpi(w1, "dynamic_pass_failure_ban_how_many") == 0) {
				dynamic_pass_failure_ban_how_many = atoi(w2);
			} else if (strcmpi(w1, "dynamic_pass_failure_ban_how_long") == 0) {
				dynamic_pass_failure_ban_how_long = atoi(w2);
			} else if(strcmpi(w1, "check_client_version") == 0){		//Added by Sirius for client version check
				if(strcmpi(w2,"on") == 0 || strcmpi(w2,"yes") == 0 ){
					check_client_version = 1;
				}
				if(strcmpi(w2,"off") == 0 || strcmpi(w2,"no") == 0 ){
					check_client_version = 0;
				}
			}else if(strcmpi(w1, "client_version_to_connect") == 0){	//Added by Sirius for client version check
				client_version_to_connect = atoi(w2);			//Added by Sirius for client version check
			} else if (strcmpi(w1, "console") == 0) {
				if(strcmpi(w2,"on") == 0 || strcmpi(w2,"yes") == 0 )
					console = 1;
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
	}
	fclose(fp);

	ShowInfo("Finished reading %s.\n", cfgName);

	return 0;
}

//-------------------------------------
// Displaying of configuration warnings
//-------------------------------------
void display_conf_warnings(void) {
	if (admin_state != 0 && admin_state != 1) {
		ShowWarning("Invalid value for admin_state parameter -> setting to 0 (no remote admin).\n");
		admin_state = 0;
	}

	if (admin_state == 1) {
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

	if (new_account_flag != 0 && new_account_flag != 1) {
		ShowWarning("Invalid value for new_account parameter -> setting to 0 (no new account).\n");
		new_account_flag = 0;
	}

	if (login_port < 1024 || login_port > 65535) {
		ShowWarning("Invalid value for login_port parameter -> setting to 6900 (default).\n");
		login_port = 6900;
	}

	if (gm_account_filename_check_timer < 0) {
		ShowWarning("Invalid value for gm_account_filename_check_timer parameter. Setting to 15 sec (default).\n");
		gm_account_filename_check_timer = 15;
	} else if (gm_account_filename_check_timer == 1) {
		ShowWarning("Invalid value for gm_account_filename_check_timer parameter. Setting to 2 sec (minimum value).\n");
		gm_account_filename_check_timer = 2;
	}

	if (save_unknown_packets != 0 && save_unknown_packets != 1) {
		ShowWarning("Invalid value for save_unknown_packets parameter -> setting to 0-no save.\n");
		save_unknown_packets = 0;
	}

	if (display_parse_login != 0 && display_parse_login != 1) { // 0: no, 1: yes
		ShowWarning("Invalid value for display_parse_login parameter -> setting to 0 (no display).\n");
		display_parse_login = 0;
	}

	if (display_parse_admin != 0 && display_parse_admin != 1) { // 0: no, 1: yes
		ShowWarning("Invalid value for display_parse_admin parameter -> setting to 0 (no display).\n");
		display_parse_admin = 0;
	}

	if (display_parse_fromchar < 0 || display_parse_fromchar > 2) { // 0: no, 1: yes (without packet 0x2714), 2: all packets
		ShowWarning("Invalid value for display_parse_fromchar parameter -> setting to 0 (no display).\n");
		display_parse_fromchar = 0;
	}

	if (min_level_to_connect < 0) { // 0: all players, 1-99 at least gm level x
		ShowWarning("Invalid value for min_level_to_connect (%d) parameter -> setting 0 (any player).\n", min_level_to_connect);
		min_level_to_connect = 0;
	} else if (min_level_to_connect > 99) { // 0: all players, 1-99 at least gm level x
		ShowWarning("Invalid value for min_level_to_connect (%d) parameter -> setting to 99 (only GM level 99)\n", min_level_to_connect);
		min_level_to_connect = 99;
	}

	if (add_to_unlimited_account != 0 && add_to_unlimited_account != 1) { // 0: no, 1: yes
		ShowWarning("Invalid value for add_to_unlimited_account parameter\n");
		ShowWarning("  -> setting to 0 (impossible to add a time to an unlimited account).\n");
		add_to_unlimited_account = 0;
	}

	if (start_limited_time < -1) { // -1: create unlimited account, 0 or more: additionnal sec from now to create limited time
		ShowWarning("Invalid value for start_limited_time parameter\n");
		ShowWarning("  -> setting to -1 (new accounts are created with unlimited time).\n");
		start_limited_time = -1;
	}

	if (check_ip_flag != 0 && check_ip_flag != 1) { // 0: no, 1: yes
		ShowWarning("Invalid value for check_ip_flag parameter\n");
		ShowWarning("  -> setting to 1 (check players ip between login-server & char-server).\n");
		check_ip_flag = 1;
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

	if (dynamic_pass_failure_ban != 0) {
		if (dynamic_pass_failure_ban_time < 1) {
			ShowWarning("Invalid value for dynamic_pass_failure_ban_time (%d) parameter\n", dynamic_pass_failure_ban_time);
			ShowWarning("  -> setting to 5 (5 minutes to look number of invalid passwords.\n");
			dynamic_pass_failure_ban_time = 5;
		}
		if (dynamic_pass_failure_ban_how_many < 1) {
			ShowWarning("Invalid value for dynamic_pass_failure_ban_how_many (%d) parameter\n", dynamic_pass_failure_ban_how_many);
			ShowWarning("  -> setting to 3 (3 invalid passwords before to temporarily ban.\n");
			dynamic_pass_failure_ban_how_many = 3;
		}
		if (dynamic_pass_failure_ban_how_long < 1) {
			ShowWarning("Invalid value for dynamic_pass_failure_ban_how_long (%d) parameter\n", dynamic_pass_failure_ban_how_long);
			ShowWarning("  -> setting to 1 (1 minute of temporarily ban.\n");
			dynamic_pass_failure_ban_how_long = 1;
		}
	}

	return;
}

//-------------------------------
// Save configuration in log file
//-------------------------------
void save_config_in_log(void) {
	int i;

	// a newline in the log...
	login_log("");
	login_log("The login-server starting..." RETCODE);

	// save configuration in log file
	login_log("The configuration of the server is set:" RETCODE);

	if (admin_state != 1)
		login_log("- with no remote administration." RETCODE);
	else if (admin_pass[0] == '\0')
		login_log("- with a remote administration with a VOID password." RETCODE);
	else if (strcmp(admin_pass, "admin") == 0)
		login_log("- with a remote administration with the DEFAULT password." RETCODE);
	else
		login_log("- with a remote administration with the password of %d character(s)." RETCODE, strlen(admin_pass));
	if (access_ladmin_allownum == 0 || (access_ladmin_allownum == 1 && access_ladmin_allow[0] == '\0')) {
		login_log("- to accept any IP for remote administration" RETCODE);
	} else {
		login_log("- to accept following IP for remote administration:" RETCODE);
		for(i = 0; i < access_ladmin_allownum; i++)
			login_log("  %s" RETCODE, (char *)(access_ladmin_allow + i * ACO_STRSIZE));
	}

	if (gm_pass[0] == '\0')
		login_log("- with a VOID 'To GM become' password (gm_pass)." RETCODE);
	else if (strcmp(gm_pass, "gm") == 0)
		login_log("- with the DEFAULT 'To GM become' password (gm_pass)." RETCODE);
	else
		login_log("- with a 'To GM become' password (gm_pass) of %d character(s)." RETCODE, strlen(gm_pass));
	if (level_new_gm == 0)
		login_log("- to refuse any creation of GM with @gm." RETCODE);
	else
		login_log("- to create GM with level '%d' when @gm is used." RETCODE, level_new_gm);

	if (new_account_flag == 1)
		login_log("- to ALLOW new users (with _F/_M)." RETCODE);
	else
		login_log("- to NOT ALLOW new users (with _F/_M)." RETCODE);
	login_log("- with port: %d." RETCODE, login_port);
	login_log("- with the accounts file name: '%s'." RETCODE, account_filename);
	login_log("- with the GM accounts file name: '%s'." RETCODE, GM_account_filename);
	if (gm_account_filename_check_timer == 0)
		login_log("- to NOT check GM accounts file modifications." RETCODE);
	else
		login_log("- to check GM accounts file modifications every %d seconds." RETCODE, gm_account_filename_check_timer);

	if (use_md5_passwds == 0)
		login_log("- to save password in plain text." RETCODE);
	else
		login_log("- to save password with MD5 encrypting." RETCODE);

	// not necessary to log the 'login_log_filename', we are inside :)

	login_log("- with the unknown packets file name: '%s'." RETCODE, login_log_unknown_packets_filename);
	if (save_unknown_packets)
		login_log("- to SAVE all unkown packets." RETCODE);
	else
		login_log("- to SAVE only unkown packets sending by a char-server or a remote administration." RETCODE);
	if (display_parse_login)
		login_log("- to display normal parse packets on console." RETCODE);
	else
		login_log("- to NOT display normal parse packets on console." RETCODE);
	if (display_parse_admin)
		login_log("- to display administration parse packets on console." RETCODE);
	else
		login_log("- to NOT display administration parse packets on console." RETCODE);
	if (display_parse_fromchar)
		login_log("- to display char-server parse packets on console." RETCODE);
	else
		login_log("- to NOT display char-server parse packets on console." RETCODE);

	if (min_level_to_connect == 0) // 0: all players, 1-99 at least gm level x
		login_log("- with no minimum level for connection." RETCODE);
	else if (min_level_to_connect == 99)
		login_log("- to accept only GM with level 99." RETCODE);
	else
		login_log("- to accept only GM with level %d or more." RETCODE, min_level_to_connect);

	if (add_to_unlimited_account)
		login_log("- to authorize adjustment (with timeadd ladmin) on an unlimited account." RETCODE);
	else
		login_log("- to refuse adjustment (with timeadd ladmin) on an unlimited account. You must use timeset (ladmin command) before." RETCODE);

	if (start_limited_time < 0)
		login_log("- to create new accounts with an unlimited time." RETCODE);
	else if (start_limited_time == 0)
		login_log("- to create new accounts with a limited time: time of creation." RETCODE);
	else
		login_log("- to create new accounts with a limited time: time of creation + %d second(s)." RETCODE, start_limited_time);

	if (check_ip_flag)
		login_log("- with control of players IP between login-server and char-server." RETCODE);
	else
		login_log("- to not check players IP between login-server and char-server." RETCODE);

	if (access_order == ACO_DENY_ALLOW) {
		if (access_denynum == 0) {
			login_log("- with the IP security order: 'deny,allow' (allow if not deny). You refuse no IP." RETCODE);
		} else if (access_denynum == 1 && access_deny[0] == '\0') {
			login_log("- with the IP security order: 'deny,allow' (allow if not deny). You refuse ALL IP." RETCODE);
		} else {
			login_log("- with the IP security order: 'deny,allow' (allow if not deny). Refused IP are:" RETCODE);
			for(i = 0; i < access_denynum; i++)
				login_log("  %s" RETCODE, (char *)(access_deny + i * ACO_STRSIZE));
		}
	} else if (access_order == ACO_ALLOW_DENY) {
		if (access_allownum == 0) {
			login_log("- with the IP security order: 'allow,deny' (deny if not allow). But, NO IP IS AUTHORISED!" RETCODE);
		} else if (access_allownum == 1 && access_allow[0] == '\0') {
			login_log("- with the IP security order: 'allow,deny' (deny if not allow). You authorise ALL IP." RETCODE);
		} else {
			login_log("- with the IP security order: 'allow,deny' (deny if not allow). Authorised IP are:" RETCODE);
			for(i = 0; i < access_allownum; i++)
				login_log("  %s" RETCODE, (char *)(access_allow + i * ACO_STRSIZE));
		}
	} else { // ACO_MUTUAL_FAILTURE
		login_log("- with the IP security order: 'mutual-failture' (allow if in the allow list and not in the deny list)." RETCODE);
		if (access_allownum == 0) {
			login_log("  But, NO IP IS AUTHORISED!" RETCODE);
		} else if (access_denynum == 1 && access_deny[0] == '\0') {
			login_log("  But, you refuse ALL IP!" RETCODE);
		} else {
			if (access_allownum == 1 && access_allow[0] == '\0') {
				login_log("  You authorise ALL IP." RETCODE);
			} else {
				login_log("  Authorised IP are:" RETCODE);
				for(i = 0; i < access_allownum; i++)
					login_log("    %s" RETCODE, (char *)(access_allow + i * ACO_STRSIZE));
			}
			login_log("  Refused IP are:" RETCODE);
			for(i = 0; i < access_denynum; i++)
				login_log("    %s" RETCODE, (char *)(access_deny + i * ACO_STRSIZE));
		}

		// dynamic password error ban
		if (dynamic_pass_failure_ban == 0)
			login_log("- with NO dynamic password error ban." RETCODE);
		else {
			login_log("- with a dynamic password error ban:" RETCODE);
			login_log("  After %d invalid password in %d minutes" RETCODE, dynamic_pass_failure_ban_how_many, dynamic_pass_failure_ban_time);
			login_log("  IP is banned for %d minutes" RETCODE, dynamic_pass_failure_ban_how_long);
		}
	}
}

//--------------------------------------
// Function called at exit of the server
//--------------------------------------
void do_final(void) {
	int i, fd;
	ShowInfo("Terminating...\n");
	fflush(stdout);
	mmo_auth_sync();
	online_db->destroy(online_db, NULL);

	if(auth_dat) aFree(auth_dat);
	if(gm_account_db) aFree(gm_account_db);
	if(access_ladmin_allow) aFree(access_ladmin_allow);
	if(access_allow) aFree(access_allow);
	if(access_deny) aFree(access_deny);
	for (i = 0; i < MAX_SERVERS; i++) {
		if ((fd = server_fd[i]) >= 0) {
			server_fd[i] = -1;
			memset(&server[i], 0, sizeof(struct mmo_char_server));
			do_close(fd);
		}
	}
	do_close(login_fd);

	login_log("----End of login-server (normal end with closing of all files)." RETCODE);

	if(log_fp)
		fclose(log_fp);
	ShowStatus("Finished.\n");
}

//------------------------------
// Main function of login-server
//------------------------------
void set_server_type(void)
{
	SERVER_TYPE = ATHENA_SERVER_LOGIN;
}
int do_init(int argc, char **argv) {
	int i, j;

	// read login-server configuration
	login_config_read((argc > 1) ? argv[1] : LOGIN_CONF_NAME);
	display_conf_warnings(); // not in login_config_read, because we can use 'import' option, and display same message twice or more
	save_config_in_log(); // not before, because log file name can be changed
	login_lan_config_read((argc > 2) ? argv[2] : LAN_CONF_NAME);

	srand((unsigned int)time(NULL));

	for(i = 0; i< AUTH_FIFO_SIZE; i++)
		auth_fifo[i].delflag = 1;
	for(i = 0; i < MAX_SERVERS; i++)
		server_fd[i] = -1;

	gm_account_db = NULL;
	GM_num = 0;
	GM_max = 0;
	mmo_auth_init();
	read_gm_account();
	set_defaultparse(parse_login);
	// Online user database init
	online_db = db_alloc(__FILE__,__LINE__,DB_INT,DB_OPT_RELEASE_DATA,sizeof(int));	// reinitialise
	add_timer_func_list(waiting_disconnect_timer, "waiting_disconnect_timer");

	login_fd = make_listen_bind(bind_ip?bind_ip:INADDR_ANY,login_port);

	add_timer_func_list(check_auth_sync, "check_auth_sync");
	add_timer_interval(gettick() + 60000, check_auth_sync, 0, 0, 60000); // every 60 sec we check if we must save accounts file (only if necessary to save)

	// add timer to check GM accounts file modification
	j = gm_account_filename_check_timer;
	if (j == 0) // if we would not to check, we check every 60 sec, just to have timer (if we change timer, is was not necessary to check if timer already exists)
		j = 60;

	add_timer_func_list(check_GM_file, "check_GM_file");
	add_timer_interval(gettick() + j * 1000, check_GM_file, 0, 0, j * 1000); // every x sec we check if gm file has been changed

	
	add_timer_func_list(online_data_cleanup, "online_data_cleanup");
	add_timer_interval(gettick() + 600*1000, online_data_cleanup, 0, 0, 600*1000); // every 10 minutes cleanup online account db.
	
	if (ip_sync_interval) {
		add_timer_func_list(sync_ip_addresses, "sync_ip_addresses");
		add_timer_interval(gettick() + ip_sync_interval, sync_ip_addresses, 0, 0, ip_sync_interval);
	}
	if(console) {
		set_defaultconsoleparse(parse_console);
	   	start_console();
	}

	login_log("The login-server is ready (Server is listening on the port %d)." RETCODE, login_port);
	ShowStatus("The login-server is "CL_GREEN"ready"CL_RESET" (Server is listening on the port %d).\n\n", login_port);

	return 0;
}
