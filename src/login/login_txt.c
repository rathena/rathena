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


// GM account management
struct gm_account* gm_account_db = NULL;
unsigned int GM_num = 0; // number of gm accounts
char GM_account_filename[1024] = "conf/GM_account.txt";
long creation_time_GM_account_file; // tracks the last-changed timestamp of the gm accounts file
int gm_account_filename_check_timer = 15; // Timer to check if GM_account file has been changed and reload GM account automaticaly (in seconds; default: 15)


// data handling (TXT)
char account_txt[1024] = "save/account.txt";

// account database
struct mmo_account* auth_dat = NULL;
unsigned int auth_num = 0, auth_max = 0;

int account_id_count = START_ACCOUNT_NUM;

// define the number of times that some players must authentify them before to save account file.
// it's just about normal authentication. If an account is created or modified, save is immediatly done.
// An authentication just change last connected IP and date. It already save in log file.
// set minimum auth change before save:
#define AUTH_BEFORE_SAVE_FILE 10
// set divider of auth_num to found number of change before save
#define AUTH_SAVE_FILE_DIVIDER 50
int auth_before_save_file = 0; // Counter. First save when 1st char-server do connection.


// ladmin configuration
bool admin_state = false;
char admin_pass[24] = "";
uint32 admin_allowed_ip = 0;

int parse_admin(int fd);


// temporary external imports
#define MAX_SERVERS 30
#define AUTH_TIMEOUT 30000
#define sex_num2str(num) ( (num ==  0  ) ? 'F' : (num ==  1  ) ? 'M' : 'S' )
#define sex_str2num(str) ( (str == 'F' ) ?  0  : (str == 'M' ) ?  1  :  2  )

struct online_login_data;
extern DBMap* auth_db;
extern DBMap* online_db;
extern struct Login_Config login_config;
extern struct mmo_char_server server[MAX_SERVERS];
extern int login_fd;
extern int allowed_regs;
extern int time_allowed;

extern int charif_sendallwos(int sfd, uint8* buf, size_t len);
extern bool check_password(struct login_session_data* sd, int passwdenc, const char* passwd, const char* refpass);
extern int online_db_setoffline(DBKey key, void* data, va_list ap);
extern struct online_login_data* add_online_user(int char_server, int account_id);
extern void remove_online_user(int account_id);
extern void* create_online_user(DBKey key, va_list args);
extern int waiting_disconnect_timer(int tid, unsigned int tick, int id, int data);
extern int lan_subnetcheck(uint32 ip);


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
	static unsigned int GM_max = 0;
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
int mmo_auth_tostr(char* str, struct mmo_account* p)
{
	int i;
	char *str_p = str;

	str_p += sprintf(str_p, "%d\t%s\t%s\t%s\t%c\t%d\t%u\t%s\t%s\t%ld\t%s\t%s\t%ld\t",
	                 p->account_id, p->userid, p->pass, p->lastlogin, p->sex,
	                 p->logincount, p->state, p->email, p->error_message,
	                 (long)p->expiration_time, p->last_ip, p->memo, (long)p->unban_time);

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
	long unban_time;
	long expiration_time;
	char str[2048];
	char v[2048];
	int GM_count = 0;
	int server_count = 0;

	auth_max = 256;
	CREATE(auth_dat, struct mmo_account, auth_max);

	if ((fp = fopen(account_txt, "r")) == NULL) {
		// no account file -> no account -> no login, including char-server (ERROR)
		ShowError(CL_RED"mmmo_auth_init: Accounts file [%s] not found."CL_RESET"\n", account_txt);
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
		    email, error_message, &expiration_time, last_ip, memo, &unban_time, &n)) == 13 && line[n] == '\t') ||
		    ((i = sscanf(line, "%d\t%[^\t]\t%[^\t]\t%[^\t]\t%c\t%d\t%u\t"
		                 "%[^\t]\t%[^\t]\t%ld\t%[^\t]\t%[^\t]%n",
		    &account_id, userid, pass, lastlogin, &sex, &logincount, &state,
		    email, error_message, &expiration_time, last_ip, memo, &n)) == 12 && line[n] == '\t')) {
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
				auth_dat = (struct mmo_account*)aRealloc(auth_dat, sizeof(struct mmo_account) * auth_max);
			}

			memset(&auth_dat[auth_num], '\0', sizeof(struct mmo_account));

			auth_dat[auth_num].account_id = account_id;

			strncpy(auth_dat[auth_num].userid, userid, 24);

			pass[32] = '\0';
			remove_control_chars(pass);
			strncpy(auth_dat[auth_num].pass, pass, 32);

			lastlogin[23] = '\0';
			remove_control_chars(lastlogin);
			strncpy(auth_dat[auth_num].lastlogin, lastlogin, 24);

			auth_dat[auth_num].sex = sex;

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
				auth_dat[auth_num].unban_time = (time_t)unban_time;
			else
				auth_dat[auth_num].unban_time = 0;

			auth_dat[auth_num].expiration_time = (time_t)expiration_time;

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
			if (auth_dat[auth_num].sex == 'S')
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
				RECREATE(auth_dat, struct mmo_account, auth_max);
			}

			memset(&auth_dat[auth_num], '\0', sizeof(struct mmo_account));

			auth_dat[auth_num].account_id = account_id;

			strncpy(auth_dat[auth_num].userid, userid, 24);

			pass[23] = '\0';
			remove_control_chars(pass);
			strncpy(auth_dat[auth_num].pass, pass, 24);

			lastlogin[23] = '\0';
			remove_control_chars(lastlogin);
			strncpy(auth_dat[auth_num].lastlogin, lastlogin, 24);

			auth_dat[auth_num].sex = sex;

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
			auth_dat[auth_num].unban_time = 0;
			auth_dat[auth_num].expiration_time = 0;
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
			if (auth_dat[auth_num].sex == 'S')
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
		ShowNotice("mmo_auth_init: No account found in %s.\n", account_txt);
	else
	if( auth_num == 1 )
		ShowStatus("mmo_auth_init: 1 account read in %s,\n", account_txt);
	else
		ShowStatus("mmo_auth_init: %d accounts read in %s,\n", auth_num, account_txt);

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
	if ((fp = lock_fopen(account_txt, &lock)) == NULL) {
		//if (id) aFree(id);
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

	lock_fclose(fp, account_txt, &lock);

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


//-------------------------------------
// Make new account
//-------------------------------------
int mmo_auth_new(struct mmo_account* account)
{
	static int num_regs = 0; // registration counter
	static unsigned int new_reg_tick = 0;
	unsigned int tick = gettick();

	time_t expiration_time = 0;
	unsigned int i = auth_num;

	// check if the account doesn't exist already
	ARR_FIND( 0, auth_num, i, strcmp(account->userid, auth_dat[i].userid) == 0 );
	if( i < auth_num )
	{
		ShowNotice("Attempt of creation of an already existant account (account: %s_%c, pass: %s, received pass: %s)\n", account->userid, account->sex, auth_dat[i].pass, account->pass);
		return 1; // 1 = Incorrect Password
	}

	//Account Registration Flood Protection by [Kevin]
	if( new_reg_tick == 0 )
		new_reg_tick = gettick();
	if( DIFF_TICK(tick, new_reg_tick) < 0 && num_regs >= allowed_regs )
	{
		ShowNotice("Account registration denied (registration limit exceeded)\n");
		return 3;
	}

	if (auth_num >= auth_max) {
		auth_max += 256;
		auth_dat = (struct mmo_account*)aRealloc(auth_dat, sizeof(struct mmo_account) * auth_max);
	}

	memset(&auth_dat[i], '\0', sizeof(struct mmo_account));

	// find a suitable non-gm account id
	while (isGM(account_id_count) > 0)
		account_id_count++;

	auth_dat[i].account_id = account_id_count++;
	safestrncpy(auth_dat[i].userid, account->userid, NAME_LENGTH);
	if( login_config.use_md5_passwds )
		MD5_String(account->pass, auth_dat[i].pass);
	else
		safestrncpy(auth_dat[i].pass, account->pass, NAME_LENGTH);
	safestrncpy(auth_dat[i].lastlogin, "-", sizeof(auth_dat[i].lastlogin));
	auth_dat[i].sex = account->sex;
	auth_dat[i].logincount = 0;
	auth_dat[i].state = 0;
	safestrncpy(auth_dat[i].email, e_mail_check(account->email) ? account->email : "a@a.com", sizeof(auth_dat[i].email));
	safestrncpy(auth_dat[i].error_message, "-", sizeof(auth_dat[i].error_message));
	auth_dat[i].unban_time = 0;
	if( login_config.start_limited_time != -1 )
		expiration_time = time(NULL) + login_config.start_limited_time;
	auth_dat[i].expiration_time = expiration_time;
	strncpy(auth_dat[i].last_ip, "-", 16);
	strncpy(auth_dat[i].memo, "-", 255);
	auth_dat[i].account_reg2_num = 0;

	ShowNotice("Account creation (account %s, id: %d, pass: %s, sex: %c)\n", account->userid, auth_num, account->pass, account->sex);
	auth_num++;

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
int mmo_auth(struct login_session_data* sd)
{
	unsigned int i;
	time_t raw_time;
	char tmpstr[256];
	int len;
	char user_password[32+1]; // reserve for md5-ed pw

	char ip[16];
	ip2str(session[sd->fd]->client_addr, ip);

	// DNS Blacklist check
	if( login_config.use_dnsbl )
	{
		char r_ip[16];
		char ip_dnsbl[256];
		char* dnsbl_serv;
		bool matched = false;
		uint8* sin_addr = (uint8*)&session[sd->fd]->client_addr;

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
	if( login_config.check_client_version && sd->version != login_config.client_version_to_connect )
		return 5;

	len = strnlen(sd->userid, NAME_LENGTH);

	// Account creation with _M/_F
	if( login_config.new_account_flag )
	{
		if( len > 2 && strnlen(sd->passwd, NAME_LENGTH) > 0 && // valid user and password lengths
			sd->passwdenc == 0 && // unencoded password
			sd->userid[len-2] == '_' && memchr("FfMm", sd->userid[len-1], 4) && // _M/_F suffix
			account_id_count <= END_ACCOUNT_NUM )
		{
			struct mmo_account acc;
			int result;

			len -= 2;
			sd->userid[len] = '\0';

			memset(&acc, '\0', sizeof(acc));
			safestrncpy(acc.userid, sd->userid, NAME_LENGTH);
			safestrncpy(acc.pass, sd->passwd, NAME_LENGTH);
			safestrncpy(acc.email, "a@a.com", sizeof(acc.email));
			acc.sex = TOUPPER(sd->userid[len+1]);

			result = mmo_auth_new(&acc);
			if( result )
				return result;// Failed to make account. [Skotlex].

			auth_before_save_file = 0; // Creation of an account -> save accounts file immediatly
		}
	}
	
	// Strict account search
	ARR_FIND( 0, auth_num, i, strcmp(sd->userid, auth_dat[i].userid) == 0 );

	// if strict account search fails, we do a no sensitive case research for index
	if( i < auth_num )
	{
		i = search_account_index(sd->userid);
		if( i == -1 )
			i = auth_num;
		else
			memcpy(sd->userid, auth_dat[i].userid, NAME_LENGTH); // for the possible tests/checks afterwards (copy correcte sensitive case).
	}

	if( i == auth_num )
	{
		ShowNotice("Unknown account (account: %s, received pass: %s, ip: %s)\n", sd->userid, sd->passwd, ip);
		return 0; // 0 = Unregistered ID
	}

	if( login_config.use_md5_passwds )
		MD5_String(sd->passwd, user_password);
	else
		safestrncpy(user_password, sd->passwd, NAME_LENGTH);

	if( !check_password(sd, sd->passwdenc, user_password, auth_dat[i].pass) )
	{
		ShowNotice("Invalid password (account: %s, pass: %s, received pass: %s, ip: %s)\n", sd->userid, auth_dat[i].pass, (sd->passwdenc) ? "[MD5]" : sd->passwd, ip);
		return 1; // 1 = Incorrect Password
	}

	if( auth_dat[i].expiration_time != 0 && auth_dat[i].expiration_time < time(NULL) )
	{
		ShowNotice("Connection refused (account: %s, pass: %s, expired ID, ip: %s)\n", sd->userid, sd->passwd, ip);
		return 2; // 2 = This ID is expired
	}

	if( auth_dat[i].unban_time != 0 && auth_dat[i].unban_time > time(NULL) )
	{
		strftime(tmpstr, 20, login_config.date_format, localtime(&auth_dat[i].unban_time));
		tmpstr[19] = '\0';
		ShowNotice("Connection refused (account: %s, pass: %s, banned until %s, ip: %s)\n", sd->userid, sd->passwd, tmpstr, ip);
		return 6; // 6 = Your are Prohibited to log in until %s
	}

	if( auth_dat[i].state )
	{
		ShowNotice("Connection refused (account: %s, pass: %s, state: %d, ip: %s)\n", sd->userid, sd->passwd, auth_dat[i].state, ip);
		return auth_dat[i].state - 1;
	}

	ShowNotice("Authentication accepted (account: %s, id: %d, ip: %s)\n", sd->userid, auth_dat[i].account_id, ip);

	// auth start : time seed
	time(&raw_time);
	strftime(tmpstr, 24, "%Y-%m-%d %H:%M:%S",localtime(&raw_time));

	sd->account_id = auth_dat[i].account_id;
	sd->login_id1 = rand();
	sd->login_id2 = rand();
	safestrncpy(sd->lastlogin, auth_dat[i].lastlogin, 24);
	sd->sex = auth_dat[i].sex;

	if( sd->sex != 'S' && sd->account_id < START_ACCOUNT_NUM )
		ShowWarning("Account %s has account id %d! Account IDs must be over %d to work properly!\n", sd->userid, sd->account_id, START_ACCOUNT_NUM);

	safestrncpy(auth_dat[i].lastlogin, tmpstr, sizeof(auth_dat[i].lastlogin));
	safestrncpy(auth_dat[i].last_ip, ip, sizeof(auth_dat[i].last_ip));
	auth_dat[i].unban_time = 0;
	auth_dat[i].logincount++;

	// Save until for change ip/time of auth is not very useful => limited save for that
	// Save there informations isnot necessary, because they are saved in log file.
	if (--auth_before_save_file <= 0) // Reduce counter. 0 or less, we save
		mmo_auth_sync();

	return -1; // account OK
}

//--------------------------------
// Packet parsing for char-servers
//--------------------------------
int parse_fromchar(int fd)
{
	unsigned int i;
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
			RFIFOSKIP(fd,2);
			ShowStatus("Char-server '%s': Request to re-load GM configuration file (ip: %s).\n", server[id].name, ip);
			read_gm_account();
			// send GM accounts to all char-servers
			send_GM_accounts(-1);
		break;

		case 0x2712: // request from char-server to authenticate an account
			if( RFIFOREST(fd) < 19 )
				return 0;
		{
			struct auth_node* node;

			int account_id = RFIFOL(fd,2);
			int login_id1 = RFIFOL(fd,6);
			int login_id2 = RFIFOL(fd,10);
			char sex = sex_num2str(RFIFOB(fd,14));
			uint32 ip_ = ntohl(RFIFOL(fd,15));
			RFIFOSKIP(fd,19);

			node = (struct auth_node*)idb_get(auth_db, account_id);
			if( node != NULL &&
			    node->account_id == account_id &&
				node->login_id1  == login_id1 &&
				node->login_id2  == login_id2 &&
				node->sex        == sex &&
				node->ip         == ip_ )
			{// found
				uint32 expiration_time;
				char email[40];
				unsigned int k;

				//ShowStatus("Char-server '%s': authentication of the account %d accepted (ip: %s).\n", server[id].name, account_id, ip);

				// each auth entry can only be used once
				idb_remove(auth_db, account_id);

				// retrieve email and account expiration time
				ARR_FIND( 0, auth_num, k, auth_dat[k].account_id == account_id );
				if( k < auth_num )
				{
					strcpy(email, auth_dat[k].email);
					expiration_time = (uint32)auth_dat[k].expiration_time;
				}
				else
				{
					memset(email, 0, sizeof(email));
					expiration_time = 0;
				}

				// send ack
				WFIFOHEAD(fd,59);
				WFIFOW(fd,0) = 0x2713;
				WFIFOL(fd,2) = account_id;
				WFIFOL(fd,6) = login_id1;
				WFIFOL(fd,10) = login_id2;
				WFIFOB(fd,14) = 0;
				memcpy(WFIFOP(fd,15), email, 40);
				WFIFOL(fd,55) = expiration_time;
				WFIFOSET(fd,59);
			}
			else
			{// authentication not found
				ShowStatus("Char-server '%s': authentication of the account %d REFUSED (ip: %s).\n", server[id].name, account_id, ip);
				WFIFOHEAD(fd,59);
				WFIFOW(fd,0) = 0x2713;
				WFIFOL(fd,2) = account_id;
				WFIFOL(fd,6) = login_id1;
				WFIFOL(fd,10) = login_id2;
				WFIFOB(fd,14) = 1;
				// It is unnecessary to send email
				// It is unnecessary to send validity date of the account
				WFIFOSET(fd,59);
			}
		}
		break;

		case 0x2714:
			if( RFIFOREST(fd) < 6 )
				return 0;
		{
			int users = RFIFOL(fd,2);
			RFIFOSKIP(fd,6);

			// how many users on world? (update)
			if( server[id].users != users )
			{
				ShowStatus("set users %s : %d\n", server[id].name, users);

				server[id].users = users;
			}
		}
		break;

		case 0x2715: // request from char server to change e-email from default "a@a.com"
			if (RFIFOREST(fd) < 46)
				return 0;
		{
			char email[40];
			int acc = RFIFOL(fd,2);
			safestrncpy(email, (char*)RFIFOP(fd,6), 40); remove_control_chars(email);
			RFIFOSKIP(fd,46);

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
		}
		break;

		case 0x2716: // received an e-mail/limited time request, because a player comes back from a map-server to the char-server
			if( RFIFOREST(fd) < 6 )
				return 0;
		{
			uint32 expiration_time = 0;
			char email[40] = "";

			int account_id = RFIFOL(fd,2);
			RFIFOSKIP(fd,6);

			ARR_FIND( 0, auth_num, i, auth_dat[i].account_id == account_id );
			if( i == auth_num )
				ShowNotice("Char-server '%s': e-mail of the account %d NOT found (ip: %s).\n", server[id].name, RFIFOL(fd,2), ip);
			else
			{
				safestrncpy(email, auth_dat[i].email, sizeof(email));
				expiration_time = (uint32)auth_dat[i].expiration_time;
			}

			WFIFOHEAD(fd,50);
			WFIFOW(fd,0) = 0x2717;
			WFIFOL(fd,2) = account_id;
			safestrncpy((char*)WFIFOP(fd,6), email, 40);
			WFIFOL(fd,46) = expiration_time;
			WFIFOSET(fd,50);
		}
		break;

		case 0x2719: // ping request from charserver
			if( RFIFOREST(fd) < 2 )
				return 0;
			RFIFOSKIP(fd,2);

			WFIFOHEAD(fd,2);
			WFIFOW(fd,0) = 0x2718;
			WFIFOSET(fd,2);
		break;

		// Map server send information to change an email of an account via char-server
		case 0x2722:	// 0x2722 <account_id>.L <actual_e-mail>.40B <new_e-mail>.40B
			if (RFIFOREST(fd) < 86)
				return 0;
		{
			char actual_email[40];
			char new_email[40];
			int account_id = RFIFOL(fd,2);
			safestrncpy(actual_email, (char*)RFIFOP(fd,6), 40); remove_control_chars(actual_email);
			safestrncpy(new_email, (char*)RFIFOP(fd,46), 40); remove_control_chars(new_email);
			RFIFOSKIP(fd, 86);

			if( e_mail_check(actual_email) == 0 )
				ShowNotice("Char-server '%s': Attempt to modify an e-mail on an account (@email GM command), but actual email is invalid (account: %d, ip: %s)\n", server[id].name, account_id, ip);
			else if( e_mail_check(new_email) == 0 )
				ShowNotice("Char-server '%s': Attempt to modify an e-mail on an account (@email GM command) with a invalid new e-mail (account: %d, ip: %s)\n", server[id].name, account_id, ip);
			else if( strcmpi(new_email, "a@a.com") == 0 )
				ShowNotice("Char-server '%s': Attempt to modify an e-mail on an account (@email GM command) with a default e-mail (account: %d, ip: %s)\n", server[id].name, account_id, ip);
			else {
				ARR_FIND( 0, auth_num, i, auth_dat[i].account_id == account_id );
				if( i == auth_num )
					ShowNotice("Char-server '%s': Attempt to modify an e-mail on an account (@email GM command), but account doesn't exist (account: %d, ip: %s).\n", server[id].name, account_id, ip);
				else
				if( strcmpi(auth_dat[i].email, actual_email) != 0 )
					ShowNotice("Char-server '%s': Attempt to modify an e-mail on an account (@email GM command), but actual e-mail is incorrect (account: %d (%s), actual e-mail: %s, proposed e-mail: %s, ip: %s).\n", server[id].name, account_id, auth_dat[i].userid, auth_dat[i].email, actual_email, ip);
				else {
					safestrncpy(auth_dat[i].email, new_email, 40);
					ShowNotice("Char-server '%s': Modify an e-mail on an account (@email GM command) (account: %d (%s), new e-mail: %s, ip: %s).\n", server[id].name, account_id, auth_dat[i].userid, new_email, ip);
					// Save
					mmo_auth_sync();
				}
			}
		}
		break;

		case 0x2724: // Receiving an account state update request from a map-server (relayed via char-server)
			if (RFIFOREST(fd) < 10)
				return 0;
		{
			int account_id = RFIFOL(fd,2);
			uint32 state = RFIFOL(fd,6);
			RFIFOSKIP(fd,10);

			ARR_FIND( 0, auth_num, i, auth_dat[i].account_id == account_id );
			if( i == auth_num )
				ShowNotice("Char-server '%s': Error of Status change (account: %d not found, suggested status %d, ip: %s).\n", server[id].name, account_id, state, ip);
			else
			if( auth_dat[i].state == state )
				ShowNotice("Char-server '%s':  Error of Status change - actual status is already the good status (account: %d, status %d, ip: %s).\n", server[id].name, account_id, state, ip);
			else {
				ShowNotice("Char-server '%s': Status change (account: %d, new status %d, ip: %s).\n", server[id].name, account_id, state, ip);
				if (state != 0) {
					uint8 buf[11];
					WBUFW(buf,0) = 0x2731;
					WBUFL(buf,2) = account_id;
					WBUFB(buf,6) = 0; // 0: change of state, 1: ban
					WBUFL(buf,7) = state; // status or final date of a banishment
					charif_sendallwos(-1, buf, 11);
				}
				auth_dat[i].state = state;
				// Save
				mmo_auth_sync();
			}
		}
		break;

		case 0x2725: // Receiving of map-server via char-server a ban request
			if (RFIFOREST(fd) < 18)
				return 0;
		{
			int account_id = RFIFOL(fd,2);
			int year = (short)RFIFOW(fd,6);
			int month = (short)RFIFOW(fd,8);
			int mday = (short)RFIFOW(fd,10);
			int hour = (short)RFIFOW(fd,12);
			int min = (short)RFIFOW(fd,14);
			int sec = (short)RFIFOW(fd,16);
			RFIFOSKIP(fd,18);

			ARR_FIND( 0, auth_num, i, auth_dat[i].account_id == account_id );
			if( i == auth_num )
				ShowNotice("Char-server '%s': Error of ban request (account: %d not found, ip: %s).\n", server[id].name, account_id, ip);
			else
			{
				time_t timestamp;
				struct tm *tmtime;
				if (auth_dat[i].unban_time == 0 || auth_dat[i].unban_time < time(NULL))
					timestamp = time(NULL);
				else
					timestamp = auth_dat[i].unban_time;
				tmtime = localtime(&timestamp);
				tmtime->tm_year = tmtime->tm_year + year;
				tmtime->tm_mon = tmtime->tm_mon + month;
				tmtime->tm_mday = tmtime->tm_mday + mday;
				tmtime->tm_hour = tmtime->tm_hour + hour;
				tmtime->tm_min = tmtime->tm_min + min;
				tmtime->tm_sec = tmtime->tm_sec + sec;
				timestamp = mktime(tmtime);
				if (timestamp == -1)
					ShowNotice("Char-server '%s': Error of ban request (account: %d, invalid date, ip: %s).\n", server[id].name, account_id, ip);
				else
				if( timestamp <= time(NULL) || timestamp == 0 )
					ShowNotice("Char-server '%s': Error of ban request (account: %d, new date unbans the account, ip: %s).\n", server[id].name, account_id, ip);
				else
				{
					unsigned char buf[16];
					char tmpstr[2048];
					strftime(tmpstr, 24, login_config.date_format, localtime(&timestamp));
					ShowNotice("Char-server '%s': Ban request (account: %d, new final date of banishment: %d (%s), ip: %s).\n", server[id].name, account_id, timestamp, tmpstr, ip);
					WBUFW(buf,0) = 0x2731;
					WBUFL(buf,2) = auth_dat[i].account_id;
					WBUFB(buf,6) = 1; // 0: change of status, 1: ban
					WBUFL(buf,7) = (unsigned int)timestamp; // status or final date of a banishment
					charif_sendallwos(-1, buf, 11);
					auth_dat[i].unban_time = timestamp;
					// Save
					mmo_auth_sync();
				}
			}
		}
		break;

		case 0x2727: // Change of sex (sex is reversed)
			if( RFIFOREST(fd) < 6 )
				return 0;
		{
			int account_id = RFIFOL(fd,2);
			RFIFOSKIP(fd,6);

			ARR_FIND( 0, auth_num, i, auth_dat[i].account_id == account_id );
			if( i == auth_num )
				ShowNotice("Char-server '%s': Error of sex change (account: %d not found, ip: %s).\n", server[id].name, account_id, ip);
			else
			if( auth_dat[i].sex == 'S' )
				ShowNotice("Char-server '%s': Error of sex change - account to change is a Server account (account: %d, ip: %s).\n", server[id].name, account_id, ip);
			else
			{
				unsigned char buf[7];
				char sex = ( auth_dat[i].sex == 'M' ) ? 'F' : 'M'; //Change gender

				auth_dat[i].sex = sex;

				ShowNotice("Char-server '%s': Sex change (account: %d, new sex %c, ip: %s).\n", server[id].name, account_id, sex, ip);
				WBUFW(buf,0) = 0x2723;
				WBUFL(buf,2) = account_id;
				WBUFB(buf,6) = sex_str2num(sex);
				charif_sendallwos(-1, buf, 7);
				// Save
				mmo_auth_sync();
			}
		}
		break;

		case 0x2728:	// We receive account_reg2 from a char-server, and we send them to other map-servers.
			if( RFIFOREST(fd) < 4 || RFIFOREST(fd) < RFIFOW(fd,2) )
				return 0;
		{
			int acc = RFIFOL(fd,4);

			ARR_FIND( 0, auth_num, i, auth_dat[i].account_id == acc );
			if( i == auth_num )
				ShowStatus("Char-server '%s': receiving (from the char-server) of account_reg2 (account: %d not found, ip: %s).\n", server[id].name, acc, ip);
			else
			{
				int len;
				int p;
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
				RFIFOW(fd,0) = 0x2729;// reusing read buffer
				charif_sendallwos(fd, RFIFOP(fd,0), RFIFOW(fd,2));
				RFIFOSKIP(fd,RFIFOW(fd,2));

				// Save
				mmo_auth_sync();
			}

		}
		break;

		case 0x272a:	// Receiving of map-server via char-server an unban request
			if( RFIFOREST(fd) < 6 )
				return 0;
		{
			int account_id = RFIFOL(fd,2);
			RFIFOSKIP(fd,6);

			ARR_FIND( 0, auth_num, i, auth_dat[i].account_id == account_id );
			if( i == auth_num )
				ShowNotice("Char-server '%s': Error of UnBan request (account: %d not found, ip: %s).\n", server[id].name, account_id, ip);
			else
			if( auth_dat[i].unban_time == 0 )
				ShowNotice("Char-server '%s': Error of UnBan request (account: %d, no change for unban date, ip: %s).\n", server[id].name, account_id, ip);
			else
			{
				auth_dat[i].unban_time = 0;
				ShowNotice("Char-server '%s': UnBan request (account: %d, ip: %s).\n", server[id].name, account_id, ip);
			}
		}
		break;

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
					p = (struct online_login_data*)idb_ensure(online_db, aid, create_online_user);
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
			size_t off;

			int account_id = RFIFOL(fd,2);
			int char_id = RFIFOL(fd,6);
			RFIFOSKIP(fd,10);

			WFIFOHEAD(fd,10000);
			WFIFOW(fd,0) = 0x2729;
			WFIFOL(fd,4) = account_id;
			WFIFOL(fd,8) = char_id;
			WFIFOB(fd,12) = 1; //Type 1 for Account2 registry

			ARR_FIND( 0, auth_num, i, auth_dat[i].account_id == account_id );
			if( i == auth_num )
			{
				//Account not found? Send at least empty data, map servers need a reply!
				WFIFOW(fd,2) = 13;
				WFIFOSET(fd,WFIFOW(fd,2));
				break;
			}

			for( off = 13, j = 0; j < auth_dat[i].account_reg2_num && off < 9000; j++ )
			{
				if( auth_dat[i].account_reg2[j].str[0] != '\0' )
				{
					off += sprintf((char*)WFIFOP(fd,off), "%s", auth_dat[i].account_reg2[j].str)+1; //We add 1 to consider the '\0' in place.
					off += sprintf((char*)WFIFOP(fd,off), "%s", auth_dat[i].account_reg2[j].value)+1;
				}
			}

			if( off >= 9000 )
				ShowWarning("Too many account2 registries for AID %d. Some registries were not sent.\n", account_id);

			WFIFOW(fd,2) = (uint16)off;
			WFIFOSET(fd,WFIFOW(fd,2));
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

void login_auth_ok(struct login_session_data* sd)
{
	int fd = sd->fd;
	uint32 ip = session[fd]->client_addr;

	uint8 server_num, n;
	uint32 subnet_char_ip;
	struct auth_node* node;
	int i;

	sd->level = isGM(sd->account_id);

	if( sd->level < login_config.min_level_to_connect )
	{
		ShowStatus("Connection refused: the minimum GM level for connection is %d (account: %s, GM level: %d).\n", login_config.min_level_to_connect, sd->userid, sd->level);
		WFIFOHEAD(fd,3);
		WFIFOW(fd,0) = 0x81;
		WFIFOB(fd,2) = 1; // 01 = Server closed
		WFIFOSET(fd,3);
		return;
	}

	server_num = 0;
	for( i = 0; i < MAX_SERVERS; ++i )
		if( session_isValid(server[i].fd) )
			server_num++;

	if( server_num == 0 )
	{// if no char-server, don't send void list of servers, just disconnect the player with proper message
		ShowStatus("Connection refused: there is no char-server online (account: %s).\n", sd->userid);
		WFIFOHEAD(fd,3);
		WFIFOW(fd,0) = 0x81;
		WFIFOB(fd,2) = 1; // 01 = Server closed
		WFIFOSET(fd,3);
		return;
	}

	if( login_config.online_check )
	{
		struct online_login_data* data = (struct online_login_data*)idb_get(online_db, sd->account_id);
		if( data )
		{// account is already marked as online!
			if( data->char_server > -1 )
			{// Request char servers to kick this account out. [Skotlex]
				uint8 buf[6];
				ShowNotice("User '%s' is already online - Rejected.\n", sd->userid);
				WBUFW(buf,0) = 0x2734;
				WBUFL(buf,2) = sd->account_id;
				charif_sendallwos(-1, buf, 6);
				if( data->waiting_disconnect == -1 )
					data->waiting_disconnect = add_timer(gettick()+AUTH_TIMEOUT, waiting_disconnect_timer, sd->account_id, 0);

				WFIFOHEAD(fd,3);
				WFIFOW(fd,0) = 0x81;
				WFIFOB(fd,2) = 8; // 08 = Server still recognizes your last login
				WFIFOSET(fd,3);
				return;
			}
			else
			if( data->char_server == -1 )
			{// client has authed but did not access char-server yet
				// wipe previous session
				idb_remove(auth_db, sd->account_id);
				remove_online_user(sd->account_id);
				data = NULL;
			}
		}
	}

	if( sd->level > 0 )
		ShowStatus("Connection of the GM (level:%d) account '%s' accepted.\n", sd->level, sd->userid);
	else
		ShowStatus("Connection of the account '%s' accepted.\n", sd->userid);

	WFIFOHEAD(fd,47+32*server_num);
	WFIFOW(fd,0) = 0x69;
	WFIFOW(fd,2) = 47+32*server_num;
	WFIFOL(fd,4) = sd->login_id1;
	WFIFOL(fd,8) = sd->account_id;
	WFIFOL(fd,12) = sd->login_id2;
	WFIFOL(fd,16) = 0; // in old version, that was for ip (not more used)
	//memcpy(WFIFOP(fd,20), sd->lastlogin, 24); // in old version, that was for name (not more used)
	WFIFOW(fd,44) = 0; // unknown
	WFIFOB(fd,46) = sex_str2num(sd->sex);
	for( i = 0, n = 0; i < MAX_SERVERS; ++i )
	{
		if( !session_isValid(server[i].fd) )
			continue;

		subnet_char_ip = lan_subnetcheck(ip); // Advanced subnet check [LuzZza]
		WFIFOL(fd,47+n*32) = htonl((subnet_char_ip) ? subnet_char_ip : server[i].ip);
		WFIFOW(fd,47+n*32+4) = ntows(htons(server[i].port)); // [!] LE byte order here [!]
		memcpy(WFIFOP(fd,47+n*32+6), server[i].name, 20);
		WFIFOW(fd,47+n*32+26) = server[i].users;
		WFIFOW(fd,47+n*32+28) = server[i].maintenance;
		WFIFOW(fd,47+n*32+30) = server[i].new_;
		n++;
	}
	WFIFOSET(fd,47+32*server_num);

	// create temporary auth entry
	CREATE(node, struct auth_node, 1);
	node->account_id = sd->account_id;
	node->login_id1 = sd->login_id1;
	node->login_id2 = sd->login_id2;
	node->sex = sd->sex;
	node->ip = ip;
	idb_put(auth_db, sd->account_id, node);

	if( login_config.online_check )
	{
		struct online_login_data* data;

		// mark client as 'online'
		data = add_online_user(-1, sd->account_id);

		// schedule deletion of this node
		data->waiting_disconnect = add_timer(gettick()+AUTH_TIMEOUT, waiting_disconnect_timer, sd->account_id, 0);
	}
}

void login_auth_failed(struct login_session_data* sd, int result)
{
	int fd = sd->fd;

	WFIFOHEAD(fd,23);
	WFIFOW(fd,0) = 0x6a;
	WFIFOB(fd,2) = (uint8)result;
	if( result != 6 )
		memset(WFIFOP(fd,3), '\0', 20);
	else
	{// 6 = Your are Prohibited to log in until %s
		char tmpstr[20];
		int i = search_account_index(sd->userid);
		time_t unban_time = ( i >= 0 ) ? auth_dat[i].unban_time : 0;
		strftime(tmpstr, 20, login_config.date_format, localtime(&unban_time));
		safestrncpy((char*)WFIFOP(fd,3), tmpstr, 20); // ban timestamp goes here
	}
	WFIFOSET(fd,23);
}

//----------------------------------------------------------------------------------------
// Default packet parsing (normal players or administation/char-server connection requests)
//----------------------------------------------------------------------------------------
int parse_login(int fd)
{
	struct login_session_data* sd = session[fd]->session_data;
	int result;
	uint32 ipl;
	char ip[16];

	if( session[fd]->flag.eof )
	{
		do_close(fd);
		return 0;
	}

	if( sd == NULL ) {
		sd = CREATE(session[fd]->session_data, struct login_session_data, 1);
		sd->fd = fd;
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

			if( (command == 0x0064 && packet_len < 55)
			||  (command == 0x01dd && packet_len < 47)
			||  (command == 0x0277 && packet_len < 84)
			||  (command == 0x02b0 && packet_len < 85) )
				return 0;

			// S 0064 <version>.l <account name>.24B <password>.24B <version2>.B
			// S 01dd <version>.l <account name>.24B <md5 binary>.16B <version2>.B
			// S 0277 <version>.l <account name>.24B <password>.24B <junk?>.29B <version2>.B
			// S 02b0 <version>.l <account name>.24B <password>.24B <junk?>.30B <version2>.B

			sd->version = RFIFOL(fd,2);
			safestrncpy(sd->userid, (char*)RFIFOP(fd,6), NAME_LENGTH); remove_control_chars(sd->userid);
			if (command != 0x01dd) {
				ShowStatus("Request for connection of %s (ip: %s).\n", sd->userid, ip);
				safestrncpy(sd->passwd, (char*)RFIFOP(fd,30), NAME_LENGTH); remove_control_chars(sd->passwd);
				sd->passwdenc = 0;
			} else {
				ShowStatus("Request for connection (encryption mode) of %s (ip: %s).\n", sd->userid, ip);
				memcpy(sd->passwd, RFIFOP(fd,30), 16); sd->passwd[16] = '\0'; // raw binary data here!
				sd->passwdenc = PASSWORDENC;
			}
			RFIFOSKIP(fd,packet_len);

			result = mmo_auth(sd);

			if( result == -1 )
				login_auth_ok(sd);
			else
				login_auth_failed(sd, result);
		}
		break;

		case 0x01db:	// Sending request of the coding key
		case 0x791a:	// Sending request of the coding key (administration packet)
			RFIFOSKIP(fd,2);
		{
			unsigned int i;

			memset(sd->md5key, '\0', sizeof(sd->md5key));
			sd->md5keylen = (uint16)(12 + rand() % 4);
			for( i = 0; i < sd->md5keylen; ++i )
				sd->md5key[i] = (char)(1 + rand() % 255);

			WFIFOHEAD(fd,4 + sd->md5keylen);
			WFIFOW(fd,0) = 0x01dc;
			WFIFOW(fd,2) = 4 + sd->md5keylen;
			memcpy(WFIFOP(fd,4), sd->md5key, sd->md5keylen);
			WFIFOSET(fd,WFIFOW(fd,2));
		}
		break;

		case 0x2710:	// Connection request of a char-server
			if (RFIFOREST(fd) < 86)
				return 0;
		{
			char server_name[20];
			uint32 server_ip;
			uint16 server_port;
			uint16 maintenance;
			uint16 new_;

			safestrncpy(sd->userid, (char*)RFIFOP(fd,2), NAME_LENGTH); //remove_control_chars(account.userid);
			safestrncpy(sd->passwd, (char*)RFIFOP(fd,26), NAME_LENGTH); //remove_control_chars(account.passwd);
			sd->passwdenc = 0;
			sd->version = login_config.client_version_to_connect; // hack to skip version check

			server_ip = ntohl(RFIFOL(fd,54));
			server_port = ntohs(RFIFOW(fd,58));

			safestrncpy(server_name, (char*)RFIFOP(fd,60), 20); remove_control_chars(server_name);
			maintenance = RFIFOW(fd,82);
			new_ = RFIFOW(fd,84);
			RFIFOSKIP(fd,86);

			ShowInfo("Connection request of the char-server '%s' @ %d.%d.%d.%d:%d (account: '%s', pass: '%s', ip: '%s')\n", server_name, CONVIP(server_ip), server_port, sd->userid, sd->passwd, ip);

			result = mmo_auth(sd);
			if( result == -1 && sd->sex == 'S' && sd->account_id < MAX_SERVERS && server[sd->account_id].fd == -1 )
			{
				ShowStatus("Connection of the char-server '%s' accepted.\n", server_name);
				safestrncpy(server[sd->account_id].name, server_name, sizeof(server[sd->account_id].name));
				server[sd->account_id].fd = fd;
				server[sd->account_id].ip = server_ip;
				server[sd->account_id].port = server_port;
				server[sd->account_id].users = 0;
				server[sd->account_id].maintenance = maintenance;
				server[sd->account_id].new_ = new_;

				session[fd]->func_parse = parse_fromchar;
				session[fd]->flag.server = 1;
				realloc_fifo(fd, FIFOSIZE_SERVERLINK, FIFOSIZE_SERVERLINK);

				// send connection success
				WFIFOHEAD(fd,3);
				WFIFOW(fd,0) = 0x2711;
				WFIFOB(fd,2) = 0;
				WFIFOSET(fd,3);

				// send GM account to char-server
				send_GM_accounts(fd);
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
		return 0; // processing will continue elsewhere

		case 0x7530:	// Server version information request
			ShowStatus("Sending server version information to ip: %s\n", ip);
			RFIFOSKIP(fd,2);
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
		break;

		case 0x7918:	// Request for administation login
			if ((int)RFIFOREST(fd) < 4 || (int)RFIFOREST(fd) < ((RFIFOW(fd,2) == 0) ? 28 : 20))
				return 0;
			WFIFOW(fd,0) = 0x7919;
			WFIFOB(fd,2) = 1;
			if( session[fd]->client_addr != admin_allowed_ip ) {
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
			ShowNotice("Abnormal end of connection (ip: %s): Unknown packet 0x%x\n", ip, command);
			set_eof(fd);
			return 0;
		}
	}

	RFIFOSKIP(fd,RFIFOREST(fd));
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

	if (login_config.start_limited_time < -1) { // -1: create unlimited account, 0 or more: additionnal sec from now to create limited time
		ShowWarning("Invalid value for start_limited_time parameter\n");
		ShowWarning("  -> setting to -1 (new accounts are created with unlimited time).\n");
		login_config.start_limited_time = -1;
	}

	return;
}
