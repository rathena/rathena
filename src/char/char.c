// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/cbasetypes.h"
#include "../common/mmo.h"
#include "../common/db.h"
#include "../common/lock.h"
#include "../common/malloc.h"
#include "../common/core.h"
#include "../common/socket.h"
#include "../common/strlib.h"
#include "../common/showmsg.h"
#include "../common/timer.h"
#include "../common/lock.h"
#include "../common/malloc.h"
#include "../common/mapindex.h"
#include "../common/showmsg.h"
#include "../common/utils.h"
#include "../common/version.h"
#include "inter.h"
#include "int_guild.h"
#include "int_homun.h"
#include "int_pet.h"
#include "int_party.h"
#include "int_storage.h"
#include "int_status.h"
#include "char.h"

#include <sys/types.h>
#ifdef WIN32
#include <winsock2.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif
#include <time.h>
#include <signal.h>
#include <fcntl.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

// private declarations
#define CHAR_CONF_NAME	"conf/char_athena.conf"
#define LAN_CONF_NAME	"conf/subnet_athena.conf"

char char_txt[1024] = "save/athena.txt";
char friends_txt[1024] = "save/friends.txt";
char hotkeys_txt[1024] = "save/hotkeys.txt";
char char_log_filename[1024] = "log/char.log";

int save_log = 1;	// show loading/saving messages

#ifndef TXT_SQL_CONVERT
char db_path[1024] = "db";

struct mmo_map_server {
	int fd;
	uint32 ip;
	uint16 port;
	int users;
	unsigned short map[MAX_MAP_PER_SERVER];
} server[MAX_MAP_SERVERS];

int login_fd=-1, char_fd=-1;
char userid[24];
char passwd[24];
char server_name[20];
char wisp_server_name[NAME_LENGTH] = "Server";
char login_ip_str[128];
uint32 login_ip = 0;
uint16 login_port = 6900;
char char_ip_str[128];
uint32 char_ip = 0;
char bind_ip_str[128];
uint32 bind_ip = INADDR_ANY;
uint16 char_port = 6121;
int char_maintenance = 0;
bool char_new = true;
int char_new_display = 0;

int email_creation = 0; // disabled by default

bool name_ignoring_case = false; // Allow or not identical name for characters but with a different case by [Yor]
int char_name_option = 0; // Option to know which letters/symbols are authorised in the name of a character (0: all, 1: only those in char_name_letters, 2: all EXCEPT those in char_name_letters) by [Yor]
char unknown_char_name[1024] = "Unknown"; // Name to use when the requested name cannot be determined
#define TRIM_CHARS "\032\t\x0A\x0D " //The following characters are trimmed regardless because they cause confusion and problems on the servers. [Skotlex]
char char_name_letters[1024] = ""; // list of letters/symbols authorised (or not) in a character name. by [Yor]
bool char_rename = true;

int log_char = 1;	// loggin char or not [devil]
int log_inter = 1;	// loggin inter or not [devil]

static int online_check = 1; //If one, it won't let players connect when their account is already registered online and will send the relevant map server a kick user request. [Skotlex]

// Advanced subnet check [LuzZza]
struct s_subnet {
	uint32 mask;
	uint32 char_ip;
	uint32 map_ip;
} subnet[16];
int subnet_count = 0;

struct char_session_data {
	bool auth; // whether the session is authed or not
	int account_id, login_id1, login_id2, sex;
	int found_char[MAX_CHARS]; // ids of chars on this account
	char email[40]; // e-mail (default: a@a.com) by [Yor]
	time_t connect_until_time; // # of seconds 1/1/1970 (timestamp): Validity limit of the account (0 = unlimited)
};

#define AUTH_FIFO_SIZE 256
struct {
	int account_id, char_id, login_id1, login_id2;
	uint32 ip;
	int delflag;
	int sex;
	time_t connect_until_time; // # of seconds 1/1/1970 (timestamp): Validity limit of the account (0 = unlimited)
} auth_fifo[AUTH_FIFO_SIZE];
int auth_fifo_pos = 0;

int char_id_count = START_CHAR_NUM;
struct character_data *char_dat;

int char_num, char_max;
int max_connect_user = 0;
int gm_allow_level = 99;
int autosave_interval = DEFAULT_AUTOSAVE_INTERVAL;
int start_zeny = 500;
int start_weapon = 1201;
int start_armor = 2301;
int guild_exp_rate = 100;

//Custom limits for the fame lists. [Skotlex]
int fame_list_size_chemist = MAX_FAME_LIST;
int fame_list_size_smith = MAX_FAME_LIST;
int fame_list_size_taekwon = MAX_FAME_LIST;

// Char-server-side stored fame lists [DracoRPG]
struct fame_list smith_fame_list[MAX_FAME_LIST];
struct fame_list chemist_fame_list[MAX_FAME_LIST];
struct fame_list taekwon_fame_list[MAX_FAME_LIST];

// Initial position (it's possible to set it in conf file)
struct point start_point = { 0, 53, 111 };

struct gm_account *gm_account = NULL;
int GM_num = 0;

// online players by [Yor]
char online_txt_filename[1024] = "online.txt";
char online_html_filename[1024] = "online.html";
int online_sorting_option = 0; // sorting option to display online players in online files
int online_display_option = 1; // display options: to know which columns must be displayed
int online_refresh_html = 20; // refresh time (in sec) of the html file in the explorer
int online_gm_display_min_level = 20; // minimum GM level to display 'GM' when we want to display it

//These are used to aid the map server in identifying valid clients. [Skotlex]
static int max_account_id = DEFAULT_MAX_ACCOUNT_ID, max_char_id = DEFAULT_MAX_CHAR_ID;

//Structure for holding in memory which characters are online on the map servers connected.
struct online_char_data {
	int account_id;
	int char_id;
	int fd;
	int waiting_disconnect;
	short server;
};

// Holds all online characters.
static DBMap* online_char_db; // int account_id -> struct online_char_data*

int console = 0;

//------------------------------
// Writing function of logs file
//------------------------------
int char_log(char *fmt, ...)
{
	if(log_char)
	{
		FILE *logfp;
		va_list ap;
		time_t raw_time;
		char tmpstr[2048];

		va_start(ap, fmt);

		logfp = fopen(char_log_filename, "a");
		if (logfp) {
			if (fmt[0] == '\0') // jump a line if no message
				fprintf(logfp, "\n");
			else {
				time(&raw_time);
				strftime(tmpstr, 24, "%d-%m-%Y %H:%M:%S", localtime(&raw_time));
				sprintf(tmpstr + 19, ": %s", fmt);
				vfprintf(logfp, tmpstr, ap);
			}
			fclose(logfp);
		}
		va_end(ap);
	}
	return 0;
}

//----------------------------------------------------------------------
// Determine if an account (id) is a GM account
// and returns its level (or 0 if it isn't a GM account or if not found)
//----------------------------------------------------------------------
int isGM(int account_id)
{
	int i;

	for(i = 0; i < GM_num; i++)
		if (gm_account[i].account_id == account_id)
			return gm_account[i].level;
	return 0;
}

//Search character data from the aid/cid givem
struct mmo_charstatus* search_character(int aid, int cid)
{
	int i;
	for (i = 0; i < char_num; i++) {
		if (char_dat[i].status.char_id == cid && char_dat[i].status.account_id == aid)
			return &char_dat[i].status;
	}
	return NULL;
}
	
struct mmo_charstatus* search_character_byname(char* character_name)
{
	int i = search_character_index(character_name);
	if (i == -1) return NULL;
	return &char_dat[i].status;
}

//----------------------------------------------
// Search an character id
//   (return character index or -1 (if not found))
//   If exact character name is not found,
//   the function checks without case sensitive
//   and returns index if only 1 character is found
//   and similar to the searched name.
//----------------------------------------------
int search_character_index(char* character_name)
{
	int i, quantity, index;

	quantity = 0;
	index = -1;
	for(i = 0; i < char_num; i++) {
		// Without case sensitive check (increase the number of similar character names found)
		if (stricmp(char_dat[i].status.name, character_name) == 0) {
			// Strict comparison (if found, we finish the function immediatly with correct value)
			if (strcmp(char_dat[i].status.name, character_name) == 0)
				return i;
			quantity++;
			index = i;
		}
	}
	// Here, the exact character name is not found
	// We return the found index of a similar account ONLY if there is 1 similar character
	if (quantity == 1)
		return index;

	// Exact character name is not found and 0 or more than 1 similar characters have been found ==> we say not found
	return -1;
}

//-------------------------------------
// Return character name with the index
//-------------------------------------
char * search_character_name(int index)
{
	if (index >= 0 && index < char_num)
		return char_dat[index].status.name;

	return unknown_char_name;
}

// Searches if the given character is online, and returns the fd of the
// map-server it is connected to.
int search_character_online(int aid, int cid)
{
	//Look for online character.
	struct online_char_data* character;
	character = (struct online_char_data*)idb_get(online_char_db, aid);
	if(character &&
		character->char_id == cid &&
		character->server > -1) 
		return server[character->server].fd;
	return -1;
}
static void * create_online_char_data(DBKey key, va_list args)
{
	struct online_char_data* character;
	character = (struct online_char_data*)aCalloc(1, sizeof(struct online_char_data));
	character->account_id = key.i;
	character->char_id = -1;
  	character->server = -1;
	character->fd = -1;
	character->waiting_disconnect = -1;
	return character;
}

static int chardb_waiting_disconnect(int tid, unsigned int tick, int id, int data);

//-------------------------------------------------
// Set Character online/offline [Wizputer]
//-------------------------------------------------

void set_char_online(int map_id, int char_id, int account_id)
{
	struct online_char_data* character;
	
	if ( char_id != 99 ) {
		if (max_account_id < account_id || max_char_id < char_id)
		{	//Notify map-server of the new max IDs [Skotlex]
			if (account_id > max_account_id)
				max_account_id = account_id;
			if (char_id > max_char_id)
				max_char_id = char_id;
			mapif_send_maxid(max_account_id, max_char_id);
		}
	}

	character = (struct online_char_data*)idb_ensure(online_char_db, account_id, create_online_char_data);
	if (online_check && character->char_id != -1 && character->server > -1 && character->server != map_id)
	{
		//char == 99 <- Character logging in, so someone has logged in while one
		//char is still on map-server, so kick him out, but don't print "error"
		//as this is normal behaviour. [Skotlex]
		if (char_id != 99)
			ShowNotice("set_char_online: Character %d:%d marked in map server %d, but map server %d claims to have (%d:%d) online!\n",
				character->account_id, character->char_id, character->server, map_id, account_id, char_id);
		mapif_disconnectplayer(server[character->server].fd, character->account_id, character->char_id, 2);
	}

	character->char_id = (char_id==99)?-1:char_id;
	character->server = (char_id==99)?-1:map_id;

	if( character->server > -1 )
		server[character->server].users++;

	if(character->waiting_disconnect != -1) {
		delete_timer(character->waiting_disconnect, chardb_waiting_disconnect);
		character->waiting_disconnect = -1;
	}
	if (login_fd > 0 && !session[login_fd]->flag.eof)
	{	
		WFIFOHEAD(login_fd,6);
		WFIFOW(login_fd,0) = 0x272b;
		WFIFOL(login_fd,2) = account_id;
		WFIFOSET(login_fd,6);
	}
}

void set_char_offline(int char_id, int account_id)
{
	struct online_char_data* character;

	if ((character = (struct online_char_data*)idb_get(online_char_db, account_id)) != NULL)
	{	//We don't free yet to avoid aCalloc/aFree spamming during char change. [Skotlex]
		if( character->server > -1 )
			server[character->server].users--;
		
		character->char_id = -1;
		character->server = -1;
		if(character->waiting_disconnect != -1){
			delete_timer(character->waiting_disconnect, chardb_waiting_disconnect);
			character->waiting_disconnect = -1;
		}
	}
	
	if (login_fd > 0 && !session[login_fd]->flag.eof)
	{
		WFIFOHEAD(login_fd,6);
		WFIFOW(login_fd,0) = 0x272c;
		WFIFOL(login_fd,2) = account_id;
		WFIFOSET(login_fd,6);
	}
}

static int char_db_setoffline(DBKey key, void* data, va_list ap)
{
	struct online_char_data* character = (struct online_char_data*)data;
	int server = va_arg(ap, int);
	if (server == -1) {
		character->char_id = -1;
		character->server = -1;
		if(character->waiting_disconnect != -1){
			delete_timer(character->waiting_disconnect, chardb_waiting_disconnect);
			character->waiting_disconnect = -1;
		}
	} else if (character->server == server)
		character->server = -2; //In some map server that we aren't connected to.
	return 0;
}

static int char_db_kickoffline(DBKey key, void* data, va_list ap)
{
	struct online_char_data* character = (struct online_char_data*)data;
	int server_id = va_arg(ap, int);

	if (server_id > -1 && character->server != server_id)
		return 0;

	//Kick out any connected characters, and set them offline as appropiate.
	if (character->server > -1)
		mapif_disconnectplayer(server[character->server].fd, character->account_id, character->char_id, 1);
	else if (character->waiting_disconnect == -1)
		set_char_offline(character->char_id, character->account_id);
	else
		return 0; // fail

	return 1;
}

void set_all_offline(int id)
{
	if (id < 0)
		ShowNotice("Sending all users offline.\n");
	else
		ShowNotice("Sending users of map-server %d offline.\n",id);
	online_char_db->foreach(online_char_db,char_db_kickoffline,id);

	if (id >= 0 || login_fd <= 0 || session[login_fd]->flag.eof)
		return;
	//Tell login-server to also mark all our characters as offline.
	WFIFOHEAD(login_fd,2);
	WFIFOW(login_fd,0) = 0x2737;
	WFIFOSET(login_fd,2);
}

/*---------------------------------------------------
  Make a data line for friends list
 --------------------------------------------------*/
int mmo_friends_list_data_str(char *str, struct mmo_charstatus *p)
{
	int i;
	char *str_p = str;
	str_p += sprintf(str_p, "%d", p->char_id);

	for (i=0;i<MAX_FRIENDS;i++){
		if (p->friends[i].account_id > 0 && p->friends[i].char_id > 0 && p->friends[i].name[0])
			str_p += sprintf(str_p, ",%d,%d,%s", p->friends[i].account_id, p->friends[i].char_id, p->friends[i].name);
	}

	str_p += '\0';

	return 0;
}

/*---------------------------------------------------
  Make a data line for hotkeys list
 --------------------------------------------------*/
int mmo_hotkeys_tostr(char *str, struct mmo_charstatus *p)
{
#ifdef HOTKEY_SAVING
	int i;
	char *str_p = str;
	str_p += sprintf(str_p, "%d", p->char_id);
	for (i=0;i<MAX_HOTKEYS;i++)
		str_p += sprintf(str_p, ",%d,%d,%d", p->hotkeys[i].type, p->hotkeys[i].id, p->hotkeys[i].lv);
	str_p += '\0';
#endif

	return 0;
}

//-------------------------------------------------
// Function to create the character line (for save)
//-------------------------------------------------
int mmo_char_tostr(char *str, struct mmo_charstatus *p, struct global_reg *reg, int reg_num)
{
	int i,j;
	char *str_p = str;

	str_p += sprintf(str_p,
		"%d\t%d,%d\t%s\t%d,%d,%d\t%u,%u,%d" //Up to Zeny field
		"\t%d,%d,%d,%d\t%d,%d,%d,%d,%d,%d\t%d,%d" //Up to Skill Point
		"\t%d,%d,%d\t%d,%d,%d,%d" //Up to hom id
		"\t%d,%d,%d\t%d,%d,%d,%d,%d" //Up to head bottom
		"\t%d,%d,%d\t%d,%d,%d" //last point + save point
		",%d,%d,%d,%d,%d\t",	//Family info
		p->char_id, p->account_id, p->slot, p->name, //
		p->class_, p->base_level, p->job_level,
		p->base_exp, p->job_exp, p->zeny,
		p->hp, p->max_hp, p->sp, p->max_sp,
		p->str, p->agi, p->vit, p->int_, p->dex, p->luk,
		p->status_point, p->skill_point,
		p->option, p->karma, p->manner,	//
		p->party_id, p->guild_id, p->pet_id, p->hom_id,
		p->hair, p->hair_color, p->clothes_color,
		p->weapon, p->shield, p->head_top, p->head_mid, p->head_bottom,
		p->last_point.map, p->last_point.x, p->last_point.y, //
		p->save_point.map, p->save_point.x, p->save_point.y,
		p->partner_id,p->father,p->mother,p->child,p->fame);
	for(i = 0; i < MAX_MEMOPOINTS; i++)
		if (p->memo_point[i].map) {
			str_p += sprintf(str_p, "%d,%d,%d ", p->memo_point[i].map, p->memo_point[i].x, p->memo_point[i].y);
		}
	*(str_p++) = '\t';

	for(i = 0; i < MAX_INVENTORY; i++)
		if (p->inventory[i].nameid) {
			str_p += sprintf(str_p,"%d,%d,%d,%d,%d,%d,%d",
				p->inventory[i].id,p->inventory[i].nameid,p->inventory[i].amount,p->inventory[i].equip,
				p->inventory[i].identify,p->inventory[i].refine,p->inventory[i].attribute);
			for(j=0; j<MAX_SLOTS; j++)
				str_p += sprintf(str_p,",%d",p->inventory[i].card[j]);
			str_p += sprintf(str_p," ");
		}
	*(str_p++) = '\t';

	for(i = 0; i < MAX_CART; i++)
		if (p->cart[i].nameid) {
			str_p += sprintf(str_p,"%d,%d,%d,%d,%d,%d,%d",
				p->cart[i].id,p->cart[i].nameid,p->cart[i].amount,p->cart[i].equip,
				p->cart[i].identify,p->cart[i].refine,p->cart[i].attribute);
			for(j=0; j<MAX_SLOTS; j++)
				str_p += sprintf(str_p,",%d",p->cart[i].card[j]);
			str_p += sprintf(str_p," ");
		}
	*(str_p++) = '\t';

	for(i = 0; i < MAX_SKILL; i++)
		if (p->skill[i].id && p->skill[i].flag != 1) {
			str_p += sprintf(str_p, "%d,%d ", p->skill[i].id, (p->skill[i].flag == 0) ? p->skill[i].lv : p->skill[i].flag-2);
		}
	*(str_p++) = '\t';

	for(i = 0; i < reg_num; i++)
		if (reg[i].str[0])
			str_p += sprintf(str_p, "%s,%s ", reg[i].str, reg[i].value);
	*(str_p++) = '\t';

	*str_p = '\0';
	return 0;
}
#endif //TXT_SQL_CONVERT
//-------------------------------------------------------------------------
// Function to set the character from the line (at read of characters file)
//-------------------------------------------------------------------------
int mmo_char_fromstr(char *str, struct mmo_charstatus *p, struct global_reg *reg, int *reg_num)
{
	char tmp_str[3][128]; //To avoid deleting chars with too long names.
	int tmp_int[256];
	unsigned int tmp_uint[2]; //To read exp....
	int next, len, i, j;

	// initilialise character
	memset(p, '\0', sizeof(struct mmo_charstatus));
	
// Char structure of version 1500 (homun + mapindex maps)
	if (sscanf(str, "%d\t%d,%d\t%127[^\t]\t%d,%d,%d\t%u,%u,%d\t%d,%d,%d,%d\t%d,%d,%d,%d,%d,%d\t%d,%d"
		"\t%d,%d,%d\t%d,%d,%d,%d\t%d,%d,%d\t%d,%d,%d,%d,%d"
		"\t%d,%d,%d\t%d,%d,%d,%d,%d,%d,%d,%d%n",
		&tmp_int[0], &tmp_int[1], &tmp_int[2], tmp_str[0],
		&tmp_int[3], &tmp_int[4], &tmp_int[5],
		&tmp_uint[0], &tmp_uint[1], &tmp_int[8],
		&tmp_int[9], &tmp_int[10], &tmp_int[11], &tmp_int[12],
		&tmp_int[13], &tmp_int[14], &tmp_int[15], &tmp_int[16], &tmp_int[17], &tmp_int[18],
		&tmp_int[19], &tmp_int[20],
		&tmp_int[21], &tmp_int[22], &tmp_int[23], //
		&tmp_int[24], &tmp_int[25], &tmp_int[26], &tmp_int[44],
		&tmp_int[27], &tmp_int[28], &tmp_int[29],
		&tmp_int[30], &tmp_int[31], &tmp_int[32], &tmp_int[33], &tmp_int[34],
		&tmp_int[45], &tmp_int[35], &tmp_int[36],
		&tmp_int[46], &tmp_int[37], &tmp_int[38], &tmp_int[39], 
		&tmp_int[40], &tmp_int[41], &tmp_int[42], &tmp_int[43], &next) != 48)
	{
	tmp_int[44] = 0; //Hom ID.
// Char structure of version 1488 (fame field addition)
	if (sscanf(str, "%d\t%d,%d\t%127[^\t]\t%d,%d,%d\t%u,%u,%d\t%d,%d,%d,%d\t%d,%d,%d,%d,%d,%d\t%d,%d"
		"\t%d,%d,%d\t%d,%d,%d\t%d,%d,%d\t%d,%d,%d,%d,%d"
		"\t%127[^,],%d,%d\t%127[^,],%d,%d,%d,%d,%d,%d,%d%n",
		&tmp_int[0], &tmp_int[1], &tmp_int[2], tmp_str[0],
		&tmp_int[3], &tmp_int[4], &tmp_int[5],
		&tmp_uint[0], &tmp_uint[1], &tmp_int[8],
		&tmp_int[9], &tmp_int[10], &tmp_int[11], &tmp_int[12],
		&tmp_int[13], &tmp_int[14], &tmp_int[15], &tmp_int[16], &tmp_int[17], &tmp_int[18],
		&tmp_int[19], &tmp_int[20],
		&tmp_int[21], &tmp_int[22], &tmp_int[23], //
		&tmp_int[24], &tmp_int[25], &tmp_int[26],
		&tmp_int[27], &tmp_int[28], &tmp_int[29],
		&tmp_int[30], &tmp_int[31], &tmp_int[32], &tmp_int[33], &tmp_int[34],
		tmp_str[1], &tmp_int[35], &tmp_int[36],
		tmp_str[2], &tmp_int[37], &tmp_int[38], &tmp_int[39], 
		&tmp_int[40], &tmp_int[41], &tmp_int[42], &tmp_int[43], &next) != 47)
	{
	tmp_int[43] = 0; //Fame
// Char structure of version 1363 (family data addition)
	if (sscanf(str, "%d\t%d,%d\t%127[^\t]\t%d,%d,%d\t%u,%u,%d\t%d,%d,%d,%d\t%d,%d,%d,%d,%d,%d\t%d,%d"
		"\t%d,%d,%d\t%d,%d,%d\t%d,%d,%d\t%d,%d,%d,%d,%d"
		"\t%127[^,],%d,%d\t%127[^,],%d,%d,%d,%d,%d,%d%n",
		&tmp_int[0], &tmp_int[1], &tmp_int[2], tmp_str[0], //
		&tmp_int[3], &tmp_int[4], &tmp_int[5],
		&tmp_uint[0], &tmp_uint[1], &tmp_int[8],
		&tmp_int[9], &tmp_int[10], &tmp_int[11], &tmp_int[12],
		&tmp_int[13], &tmp_int[14], &tmp_int[15], &tmp_int[16], &tmp_int[17], &tmp_int[18],
		&tmp_int[19], &tmp_int[20],
		&tmp_int[21], &tmp_int[22], &tmp_int[23], //
		&tmp_int[24], &tmp_int[25], &tmp_int[26],
		&tmp_int[27], &tmp_int[28], &tmp_int[29],
		&tmp_int[30], &tmp_int[31], &tmp_int[32], &tmp_int[33], &tmp_int[34],
		tmp_str[1], &tmp_int[35], &tmp_int[36], //
		tmp_str[2], &tmp_int[37], &tmp_int[38], &tmp_int[39], 
		&tmp_int[40], &tmp_int[41], &tmp_int[42], &next) != 46)
	{
	tmp_int[40] = 0; // father
	tmp_int[41] = 0; // mother
	tmp_int[42] = 0; // child
// Char structure version 1008 (marriage partner addition)
	if (sscanf(str, "%d\t%d,%d\t%127[^\t]\t%d,%d,%d\t%u,%u,%d\t%d,%d,%d,%d\t%d,%d,%d,%d,%d,%d\t%d,%d"
		"\t%d,%d,%d\t%d,%d,%d\t%d,%d,%d\t%d,%d,%d,%d,%d"
		"\t%127[^,],%d,%d\t%127[^,],%d,%d,%d%n",
		&tmp_int[0], &tmp_int[1], &tmp_int[2], tmp_str[0], //
		&tmp_int[3], &tmp_int[4], &tmp_int[5],
		&tmp_uint[0], &tmp_uint[1], &tmp_int[8],
		&tmp_int[9], &tmp_int[10], &tmp_int[11], &tmp_int[12],
		&tmp_int[13], &tmp_int[14], &tmp_int[15], &tmp_int[16], &tmp_int[17], &tmp_int[18],
		&tmp_int[19], &tmp_int[20],
		&tmp_int[21], &tmp_int[22], &tmp_int[23], //
		&tmp_int[24], &tmp_int[25], &tmp_int[26],
		&tmp_int[27], &tmp_int[28], &tmp_int[29],
		&tmp_int[30], &tmp_int[31], &tmp_int[32], &tmp_int[33], &tmp_int[34],
		tmp_str[1], &tmp_int[35], &tmp_int[36], //
		tmp_str[2], &tmp_int[37], &tmp_int[38], &tmp_int[39], &next) != 43)
	{
	tmp_int[39] = 0; // partner id
// Char structure version 384 (pet addition)
	if (sscanf(str, "%d\t%d,%d\t%127[^\t]\t%d,%d,%d\t%u,%u,%d\t%d,%d,%d,%d\t%d,%d,%d,%d,%d,%d\t%d,%d"
		"\t%d,%d,%d\t%d,%d,%d\t%d,%d,%d\t%d,%d,%d,%d,%d"
		"\t%127[^,],%d,%d\t%127[^,],%d,%d%n",
		&tmp_int[0], &tmp_int[1], &tmp_int[2], tmp_str[0], //
		&tmp_int[3], &tmp_int[4], &tmp_int[5],
		&tmp_uint[0], &tmp_uint[1], &tmp_int[8],
		&tmp_int[9], &tmp_int[10], &tmp_int[11], &tmp_int[12],
		&tmp_int[13], &tmp_int[14], &tmp_int[15], &tmp_int[16], &tmp_int[17], &tmp_int[18],
		&tmp_int[19], &tmp_int[20],
		&tmp_int[21], &tmp_int[22], &tmp_int[23], //
		&tmp_int[24], &tmp_int[25], &tmp_int[26],
		&tmp_int[27], &tmp_int[28], &tmp_int[29],
		&tmp_int[30], &tmp_int[31], &tmp_int[32], &tmp_int[33], &tmp_int[34],
		tmp_str[1], &tmp_int[35], &tmp_int[36], //
		tmp_str[2], &tmp_int[37], &tmp_int[38], &next) != 42)
	{
	tmp_int[26] = 0; // pet id
// Char structure of a version 1 (original data structure)
	if (sscanf(str, "%d\t%d,%d\t%127[^\t]\t%d,%d,%d\t%u,%u,%d\t%d,%d,%d,%d\t%d,%d,%d,%d,%d,%d\t%d,%d"
		"\t%d,%d,%d\t%d,%d\t%d,%d,%d\t%d,%d,%d,%d,%d"
		"\t%127[^,],%d,%d\t%127[^,],%d,%d%n",
		&tmp_int[0], &tmp_int[1], &tmp_int[2], tmp_str[0], //
		&tmp_int[3], &tmp_int[4], &tmp_int[5],
		&tmp_uint[0], &tmp_uint[1], &tmp_int[8],
		&tmp_int[9], &tmp_int[10], &tmp_int[11], &tmp_int[12],
		&tmp_int[13], &tmp_int[14], &tmp_int[15], &tmp_int[16], &tmp_int[17], &tmp_int[18],
		&tmp_int[19], &tmp_int[20],
		&tmp_int[21], &tmp_int[22], &tmp_int[23], //
		&tmp_int[24], &tmp_int[25], //
		&tmp_int[27], &tmp_int[28], &tmp_int[29],
		&tmp_int[30], &tmp_int[31], &tmp_int[32], &tmp_int[33], &tmp_int[34],
		tmp_str[1], &tmp_int[35], &tmp_int[36], //
		tmp_str[2], &tmp_int[37], &tmp_int[38], &next) != 41)
	{
		ShowError("Char-loading: Unrecognized character data version, info lost!\n");
		ShowDebug("Character info: %s\n", str);
		return 0;
	}
	}	// Char structure version 384 (pet addition)
	}	// Char structure version 1008 (marriage partner addition)
	}	// Char structure of version 1363 (family data addition)
	}	// Char structure of version 1488 (fame field addition)
	//Convert save data from string to integer for older formats
		tmp_int[45] = mapindex_name2id(tmp_str[1]);
		tmp_int[46] = mapindex_name2id(tmp_str[2]);
	}	// Char structure of version 1500 (homun + mapindex maps)

	memcpy(p->name, tmp_str[0], NAME_LENGTH); //Overflow protection [Skotlex]
	p->char_id = tmp_int[0];
	p->account_id = tmp_int[1];
	p->slot = tmp_int[2];
	p->class_ = tmp_int[3];
	p->base_level = tmp_int[4];
	p->job_level = tmp_int[5];
	p->base_exp = tmp_uint[0];
	p->job_exp = tmp_uint[1];
	p->zeny = tmp_int[8];
	p->hp = tmp_int[9];
	p->max_hp = tmp_int[10];
	p->sp = tmp_int[11];
	p->max_sp = tmp_int[12];
	p->str = tmp_int[13];
	p->agi = tmp_int[14];
	p->vit = tmp_int[15];
	p->int_ = tmp_int[16];
	p->dex = tmp_int[17];
	p->luk = tmp_int[18];
	p->status_point = min(tmp_int[19], USHRT_MAX);
	p->skill_point = min(tmp_int[20], USHRT_MAX);
	p->option = tmp_int[21];
	p->karma = tmp_int[22];
	p->manner = tmp_int[23];
	p->party_id = tmp_int[24];
	p->guild_id = tmp_int[25];
	p->pet_id = tmp_int[26];
	p->hair = tmp_int[27];
	p->hair_color = tmp_int[28];
	p->clothes_color = tmp_int[29];
	p->weapon = tmp_int[30];
	p->shield = tmp_int[31];
	p->head_top = tmp_int[32];
	p->head_mid = tmp_int[33];
	p->head_bottom = tmp_int[34];
	p->last_point.x = tmp_int[35];
	p->last_point.y = tmp_int[36];
	p->save_point.x = tmp_int[37];
	p->save_point.y = tmp_int[38];
	p->partner_id = tmp_int[39];
	p->father = tmp_int[40];
	p->mother = tmp_int[41];
	p->child = tmp_int[42];
	p->fame = tmp_int[43];
	p->hom_id = tmp_int[44];
	p->last_point.map = tmp_int[45];
	p->save_point.map = tmp_int[46];

#ifndef TXT_SQL_CONVERT
	// Some checks
	for(i = 0; i < char_num; i++) {
		if (char_dat[i].status.char_id == p->char_id) {
			ShowError(CL_RED"mmmo_auth_init: a character has an identical id to another.\n");
			ShowError("               character id #%d -> new character not readed.\n", p->char_id);
			ShowError("               Character saved in log file."CL_RESET"\n");
			return -1;
		} else if (strcmp(char_dat[i].status.name, p->name) == 0) {
			ShowError(CL_RED"mmmo_auth_init: a character name already exists.\n");
			ShowError("               character name '%s' -> new character not read.\n", p->name);
			ShowError("               Character saved in log file."CL_RESET"\n");
			return -2;
		}
	}

	if (strcmpi(wisp_server_name, p->name) == 0) {
		ShowWarning("mmo_auth_init: ******WARNING: character name has wisp server name.\n");
		ShowWarning("               Character name '%s' = wisp server name '%s'.\n", p->name, wisp_server_name);
		ShowWarning("               Character readed. Suggestion: change the wisp server name.\n");
		char_log("mmo_auth_init: ******WARNING: character name has wisp server name: Character name '%s' = wisp server name '%s'.\n",
		          p->name, wisp_server_name);
	}
#endif //TXT_SQL_CONVERT
	if (str[next] == '\n' || str[next] == '\r')
		return 1;	// 新規データ

	next++;

	for(i = 0; str[next] && str[next] != '\t'; i++) {
		//mapindex memo format
		if (sscanf(str+next, "%d,%d,%d%n", &tmp_int[2], &tmp_int[0], &tmp_int[1], &len) != 3)
		{	//Old string-based memo format.
			if (sscanf(str+next, "%[^,],%d,%d%n", tmp_str[0], &tmp_int[0], &tmp_int[1], &len) != 3)
				return -3;
			tmp_int[2] = mapindex_name2id(tmp_str[0]);
		}
		if (i < MAX_MEMOPOINTS)
	  	{	//Avoid overflowing (but we must also read through all saved memos)
			p->memo_point[i].x = tmp_int[0];
			p->memo_point[i].y = tmp_int[1];
			p->memo_point[i].map = tmp_int[2];
		}
		next += len;
		if (str[next] == ' ')
			next++;
	}

	next++;

	for(i = 0; str[next] && str[next] != '\t'; i++) {
		if(sscanf(str + next, "%d,%d,%d,%d,%d,%d,%d%[0-9,-]%n",
		      &tmp_int[0], &tmp_int[1], &tmp_int[2], &tmp_int[3],
		      &tmp_int[4], &tmp_int[5], &tmp_int[6], tmp_str[0], &len) == 8)
		{
			p->inventory[i].id = tmp_int[0];
			p->inventory[i].nameid = tmp_int[1];
			p->inventory[i].amount = tmp_int[2];
			p->inventory[i].equip = tmp_int[3];
			p->inventory[i].identify = tmp_int[4];
			p->inventory[i].refine = tmp_int[5];
			p->inventory[i].attribute = tmp_int[6];

			for(j = 0; j < MAX_SLOTS && tmp_str[0] && sscanf(tmp_str[0], ",%d%[0-9,-]",&tmp_int[0], tmp_str[0]) > 0; j++)
				p->inventory[i].card[j] = tmp_int[0];

			next += len;
			if (str[next] == ' ')
				next++;
		} else // invalid structure
			return -4;
	}
	next++;

	for(i = 0; str[next] && str[next] != '\t'; i++) {
		if(sscanf(str + next, "%d,%d,%d,%d,%d,%d,%d%[0-9,-]%n",
		      &tmp_int[0], &tmp_int[1], &tmp_int[2], &tmp_int[3],
		      &tmp_int[4], &tmp_int[5], &tmp_int[6], tmp_str[0], &len) == 8)
		{
			p->cart[i].id = tmp_int[0];
			p->cart[i].nameid = tmp_int[1];
			p->cart[i].amount = tmp_int[2];
			p->cart[i].equip = tmp_int[3];
			p->cart[i].identify = tmp_int[4];
			p->cart[i].refine = tmp_int[5];
			p->cart[i].attribute = tmp_int[6];
			
			for(j = 0; j < MAX_SLOTS && tmp_str && sscanf(tmp_str[0], ",%d%[0-9,-]",&tmp_int[0], tmp_str[0]) > 0; j++)
				p->cart[i].card[j] = tmp_int[0];
			
			next += len;
			if (str[next] == ' ')
				next++;
		} else // invalid structure
			return -5;
	}

	next++;

	for(i = 0; str[next] && str[next] != '\t'; i++) {
		if (sscanf(str + next, "%d,%d%n", &tmp_int[0], &tmp_int[1], &len) != 2)
			return -6;
		p->skill[tmp_int[0]].id = tmp_int[0];
		p->skill[tmp_int[0]].lv = tmp_int[1];
		next += len;
		if (str[next] == ' ')
			next++;
	}

	next++;

	for(i = 0; str[next] && str[next] != '\t' && str[next] != '\n' && str[next] != '\r'; i++) { // global_reg実装以前のathena.txt互換のため一応'\n'チェック
		if (sscanf(str + next, "%[^,],%[^ ] %n", reg[i].str, reg[i].value, &len) != 2) { 
			// because some scripts are not correct, the str can be "". So, we must check that.
			// If it's, we must not refuse the character, but just this REG value.
			// Character line will have something like: nov_2nd_cos,9 ,9 nov_1_2_cos_c,1 (here, ,9 is not good)
			if (str[next] == ',' && sscanf(str + next, ",%[^ ] %n", reg[i].value, &len) == 1) 
				i--;
			else
				return -7;
		}
		next += len;
		if (str[next] == ' ')
			next++;
	}
	*reg_num = i;

	return 1;
}

//---------------------------------
// Function to read friend list
//---------------------------------
int parse_friend_txt(struct mmo_charstatus *p)
{
	char line[1024], temp[1024];
	int pos = 0, count = 0, next;
	int i,len;
	FILE *fp;

	// Open the file and look for the ID
	fp = fopen(friends_txt, "r");

	if(fp == NULL)
		return -1;
	
	while(fgets(line, sizeof(line), fp))
	{
		if(line[0] == '/' && line[1] == '/')
			continue;
		if (sscanf(line, "%d%n",&i, &pos) < 1 || i != p->char_id)
			continue; //Not this line...
		//Read friends
		len = strlen(line);
		next = pos;
		for (count = 0; next < len && count < MAX_FRIENDS; count++)
		{ //Read friends.
			if (sscanf(line+next, ",%d,%d,%23[^,^\n]%n",&p->friends[count].account_id,&p->friends[count].char_id, p->friends[count].name, &pos) < 3)
			{	//Invalid friend?
				memset(&p->friends[count], 0, sizeof(p->friends[count]));
				break;
			}
			next+=pos;
			//What IF the name contains a comma? while the next field is not a 
			//number, we assume it belongs to the current name. [Skotlex]
			//NOTE: Of course, this will fail if someone sets their name to something like
			//Bob,2005 but... meh, it's the problem of parsing a text file (encasing it in "
			//won't do as quotes are also valid name chars!)
			while(next < len && sscanf(line+next, ",%23[^,^\n]%n", temp, &pos) > 0)
			{
				if (atoi(temp)) //We read the next friend, just continue.
					break;
				//Append the name.
				next+=pos;
				i = strlen(p->friends[count].name);
				if (i + strlen(temp) +1 < NAME_LENGTH)
				{
					p->friends[count].name[i] = ',';
					strcpy(p->friends[count].name+i+1, temp);
				}
			} //End Guess Block
		} //Friend's for.
		break; //Found friends.
	}
	fclose(fp);
	return count;
}

//---------------------------------
// Function to read hotkey list
//---------------------------------
int parse_hotkey_txt(struct mmo_charstatus *p)
{
#ifdef HOTKEY_SAVING
	char line[1024];
	int pos = 0, count = 0, next;
	int i,len;
	int type, id, lv;
	FILE *fp;

	// Open the file and look for the ID
	fp = fopen(hotkeys_txt, "r");
	if(fp == NULL)
		return -1;
	
	while(fgets(line, sizeof(line), fp))
	{
		if(line[0] == '/' && line[1] == '/')
			continue;
		if (sscanf(line, "%d%n",&i, &pos) < 1 || i != p->char_id)
			continue; //Not this line...
		//Read hotkeys 
		len = strlen(line);
		next = pos;
		for (count = 0; next < len && count < MAX_HOTKEYS; count++)
		{
			if (sscanf(line+next, ",%d,%d,%d%n",&type,&id,&lv, &pos) < 3)
				//Invalid entry?
				break;
			p->hotkeys[count].type = type;
			p->hotkeys[count].id = id;
			p->hotkeys[count].lv = lv;
			next+=pos;
		}
		break; //Found hotkeys.
	}
	fclose(fp);
	return count;
#else
	return 0;
#endif
}



#ifndef TXT_SQL_CONVERT
//---------------------------------
// Function to read characters file
//---------------------------------
int mmo_char_init(void)
{
	char line[65536];
	int ret, line_count;
	FILE* fp;

	char_num = 0;
	char_max = 0;
	char_dat = NULL;

	fp = fopen(char_txt, "r");

	if (fp == NULL) {
		ShowError("Characters file not found: %s.\n", char_txt);
		char_log("Characters file not found: %s.\n", char_txt);
		char_log("Id for the next created character: %d.\n", char_id_count);
		return 0;
	}

	line_count = 0;
	while(fgets(line, sizeof(line), fp))
	{
		int i, j;
		line_count++;

		if (line[0] == '/' && line[1] == '/')
			continue;

		j = 0;
		if (sscanf(line, "%d\t%%newid%%%n", &i, &j) == 1 && j > 0) {
			if (char_id_count < i)
				char_id_count = i;
			continue;
		}

		if (char_num >= char_max) {
			char_max += 256;
			char_dat = (struct character_data*)aRealloc(char_dat, sizeof(struct character_data) * char_max);
			if (!char_dat) {
				ShowFatalError("Out of memory: mmo_char_init (realloc of char_dat).\n");
				char_log("Out of memory: mmo_char_init (realloc of char_dat).\n");
				exit(EXIT_FAILURE);
			}
		}

		ret = mmo_char_fromstr(line, &char_dat[char_num].status, char_dat[char_num].global, &char_dat[char_num].global_num);

		// Initialize friends list
		parse_friend_txt(&char_dat[char_num].status);  // Grab friends for the character
		// Initialize hotkey list
		parse_hotkey_txt(&char_dat[char_num].status);  // Grab hotkeys for the character
		
		if (ret > 0) { // negative value or zero for errors
			if (char_dat[char_num].status.char_id >= char_id_count)
				char_id_count = char_dat[char_num].status.char_id + 1;
			char_num++;
		} else {
			ShowError("mmo_char_init: in characters file, unable to read the line #%d.\n", line_count);
			ShowError("               -> Character saved in log file.\n");
			switch (ret) {
			case -1:
				char_log("Duplicate character id in the next character line (character not readed):\n");
				break;
			case -2:
				char_log("Duplicate character name in the next character line (character not readed):\n");
				break;
			case -3:
				char_log("Invalid memo point structure in the next character line (character not readed):\n");
				break;
			case -4:
				char_log("Invalid inventory item structure in the next character line (character not readed):\n");
				break;
			case -5:
				char_log("Invalid cart item structure in the next character line (character not readed):\n");
				break;
			case -6:
				char_log("Invalid skill structure in the next character line (character not readed):\n");
				break;
			case -7:
				char_log("Invalid register structure in the next character line (character not readed):\n");
				break;
			default: // 0
				char_log("Unabled to get a character in the next line - Basic structure of line (before inventory) is incorrect (character not readed):\n");
				break;
			}
			char_log("%s", line);
		}
	}
	fclose(fp);

	if (char_num == 0) {
		ShowNotice("mmo_char_init: No character found in %s.\n", char_txt);
		char_log("mmo_char_init: No character found in %s.\n", char_txt);
	} else if (char_num == 1) {
		ShowStatus("mmo_char_init: 1 character read in %s.\n", char_txt);
		char_log("mmo_char_init: 1 character read in %s.\n", char_txt);
	} else {
		ShowStatus("mmo_char_init: %d characters read in %s.\n", char_num, char_txt);
		char_log("mmo_char_init: %d characters read in %s.\n", char_num, char_txt);
	}

	char_log("Id for the next created character: %d.\n", char_id_count);

	return 0;
}

//---------------------------------------------------------
// Function to save characters in files (speed up by [Yor])
//---------------------------------------------------------
void mmo_char_sync(void)
{
	char line[65536],f_line[1024];
	int i, j, k;
	int lock;
	FILE *fp,*f_fp;
	CREATE_BUFFER(id, int, char_num);

	// Sorting before save (by [Yor])
	for(i = 0; i < char_num; i++) {
		id[i] = i;
		for(j = 0; j < i; j++) {
			if ((char_dat[i].status.account_id < char_dat[id[j]].status.account_id) ||
			    // if same account id, we sort by slot.
			    (char_dat[i].status.account_id == char_dat[id[j]].status.account_id &&
			     char_dat[i].status.slot < char_dat[id[j]].status.slot)) {
				for(k = i; k > j; k--)
					id[k] = id[k-1];
				id[j] = i; // id[i]
				break;
			}
		}
	}

	// Data save
	fp = lock_fopen(char_txt, &lock);
	if (fp == NULL) {
		ShowWarning("Server cannot save characters.\n");
		char_log("WARNING: Server cannot save characters.\n");
	} else {
		for(i = 0; i < char_num; i++) {
			mmo_char_tostr(line, &char_dat[id[i]].status, char_dat[id[i]].global, char_dat[id[i]].global_num); // use of sorted index
			fprintf(fp, "%s\n", line);
		}
		fprintf(fp, "%d\t%%newid%%\n", char_id_count);
		lock_fclose(fp, char_txt, &lock);
	}

	// Friends List data save (davidsiaw)
	f_fp = lock_fopen(friends_txt, &lock);
	for(i = 0; i < char_num; i++) {
		mmo_friends_list_data_str(f_line, &char_dat[id[i]].status);
		fprintf(f_fp, "%s\n", f_line);
	}

	lock_fclose(f_fp, friends_txt, &lock);

#ifdef HOTKEY_SAVING
	// Hotkey List data save (Skotlex)
	f_fp = lock_fopen(hotkeys_txt, &lock);
	for(i = 0; i < char_num; i++) {
		mmo_hotkeys_tostr(f_line, &char_dat[id[i]].status);
		fprintf(f_fp, "%s\n", f_line);
	}

	lock_fclose(f_fp, hotkeys_txt, &lock);
#endif

	DELETE_BUFFER(id);

	return;
}

//----------------------------------------------------
// Function to save (in a periodic way) datas in files
//----------------------------------------------------
int mmo_char_sync_timer(int tid, unsigned int tick, int id, int data)
{
	if (save_log)
		ShowInfo("Saving all files...\n");
	mmo_char_sync();
	inter_save();
	return 0;
}

//-----------------------------------
// Function to create a new character
//-----------------------------------
int make_new_char(struct char_session_data* sd, char* name_, int str, int agi, int vit, int int_, int dex, int luk, int slot, int hair_color, int hair_style)
{
	char name[NAME_LENGTH];
	int i;
	
	safestrncpy(name, name_, NAME_LENGTH);
	normalize_name(name,TRIM_CHARS);

	// check length of character name
	if( name[0] == '\0' )
		return -2; // empty character name

	// check content of character name
	if( remove_control_chars(name) )
		return -2; // control chars in name

	// check for reserved names
	if( strcmpi(name, main_chat_nick) == 0 || strcmpi(name, wisp_server_name) == 0 )
		return -1; // nick reserved for internal server messages

	// Check Authorised letters/symbols in the name of the character
	if( char_name_option == 1 ) { // only letters/symbols in char_name_letters are authorised
		for( i = 0; i < NAME_LENGTH && name[i]; i++ )
			if( strchr(char_name_letters, name[i]) == NULL )
				return -2;
	} else
	if( char_name_option == 2 ) { // letters/symbols in char_name_letters are forbidden
		for( i = 0; i < NAME_LENGTH && name[i]; i++ )
			if( strchr(char_name_letters, name[i]) != NULL )
				return -2;
	} // else, all letters/symbols are authorised (except control char removed before)

	// check name (already in use?)
	ARR_FIND( 0, char_num, i,
		(name_ignoring_case && strncmp(char_dat[i].status.name, name, NAME_LENGTH) == 0) ||
		(!name_ignoring_case && strncmpi(char_dat[i].status.name, name, NAME_LENGTH) == 0) );
	if( i < char_num )
		return -1; // name already exists

	//check other inputs
	if((slot >= MAX_CHARS) // slots
	|| (hair_style >= 24) // hair style
	|| (hair_color >= 9) // hair color
	|| (str + agi + vit + int_ + dex + luk != 6*5 ) // stats
	|| (str < 1 || str > 9 || agi < 1 || agi > 9 || vit < 1 || vit > 9 || int_ < 1 || int_ > 9 || dex < 1 || dex > 9 || luk < 1 || luk > 9) // individual stat values
	|| (str + int_ != 10 || agi + luk != 10 || vit + dex != 10) ) // pairs
		return -2; // invalid input

	// check char slot
	ARR_FIND( 0, char_num, i, char_dat[i].status.account_id == sd->account_id && char_dat[i].status.slot == slot );
	if( i < char_num )
		return -2; // slot already in use

	if (char_num >= char_max) {
		char_max += 256;
		RECREATE(char_dat, struct character_data, char_max);
		if (!char_dat) {
			ShowFatalError("Out of memory: make_new_char (realloc of char_dat).\n");
			char_log("Out of memory: make_new_char (realloc of char_dat).\n");
			exit(EXIT_FAILURE);
		}
	}

	// validation success, log result
	char_log("make new char: account: %d, slot %d, name: %s, stats: %d/%d/%d/%d/%d/%d, hair: %d, hair color: %d.\n",
	         sd->account_id, slot, name, str, agi, vit, int_, dex, luk, hair_style, hair_color);

	i = char_num;
	memset(&char_dat[i], 0, sizeof(struct character_data));

	char_dat[i].status.char_id = char_id_count++;
	char_dat[i].status.account_id = sd->account_id;
	char_dat[i].status.slot = slot;
	safestrncpy(char_dat[i].status.name,name,NAME_LENGTH);
	char_dat[i].status.class_ = 0;
	char_dat[i].status.base_level = 1;
	char_dat[i].status.job_level = 1;
	char_dat[i].status.base_exp = 0;
	char_dat[i].status.job_exp = 0;
	char_dat[i].status.zeny = start_zeny;
	char_dat[i].status.str = str;
	char_dat[i].status.agi = agi;
	char_dat[i].status.vit = vit;
	char_dat[i].status.int_ = int_;
	char_dat[i].status.dex = dex;
	char_dat[i].status.luk = luk;
	char_dat[i].status.max_hp = 40 * (100 + char_dat[i].status.vit) / 100;
	char_dat[i].status.max_sp = 11 * (100 + char_dat[i].status.int_) / 100;
	char_dat[i].status.hp = char_dat[i].status.max_hp;
	char_dat[i].status.sp = char_dat[i].status.max_sp;
	char_dat[i].status.status_point = 0;
	char_dat[i].status.skill_point = 0;
	char_dat[i].status.option = 0;
	char_dat[i].status.karma = 0;
	char_dat[i].status.manner = 0;
	char_dat[i].status.party_id = 0;
	char_dat[i].status.guild_id = 0;
	char_dat[i].status.hair = hair_style;
	char_dat[i].status.hair_color = hair_color;
	char_dat[i].status.clothes_color = 0;
	char_dat[i].status.inventory[0].nameid = start_weapon; // Knife
	char_dat[i].status.inventory[0].amount = 1;
	char_dat[i].status.inventory[0].identify = 1;
	char_dat[i].status.inventory[1].nameid = start_armor; // Cotton Shirt
	char_dat[i].status.inventory[1].amount = 1;
	char_dat[i].status.inventory[1].identify = 1;
	char_dat[i].status.weapon = 0; // W_FIST
	char_dat[i].status.shield = 0;
	char_dat[i].status.head_top = 0;
	char_dat[i].status.head_mid = 0;
	char_dat[i].status.head_bottom = 0;
	memcpy(&char_dat[i].status.last_point, &start_point, sizeof(start_point));
	memcpy(&char_dat[i].status.save_point, &start_point, sizeof(start_point));
	char_num++;

	ShowInfo("Created char: account: %d, char: %d, slot: %d, name: %s\n", sd->account_id, i, slot, name);
	mmo_char_sync();
	return i;
}

//----------------------------------------------------
// This function return the name of the job (by [Yor])
//----------------------------------------------------
char * job_name(int class_)
{
	switch (class_) {
	case JOB_NOVICE:    return "Novice";
	case JOB_SWORDMAN:    return "Swordsman";
	case JOB_MAGE:    return "Mage";
	case JOB_ARCHER:    return "Archer";
	case JOB_ACOLYTE:    return "Acolyte";
	case JOB_MERCHANT:    return "Merchant";
	case JOB_THIEF:    return "Thief";
	case JOB_KNIGHT:    return "Knight";
	case JOB_PRIEST:    return "Priest";
	case JOB_WIZARD:    return "Wizard";
	case JOB_BLACKSMITH:   return "Blacksmith";
	case JOB_HUNTER:   return "Hunter";
	case JOB_ASSASSIN:   return "Assassin";
	case JOB_KNIGHT2:   return "Peco-Knight";
	case JOB_CRUSADER:   return "Crusader";
	case JOB_MONK:   return "Monk";
	case JOB_SAGE:   return "Sage";
	case JOB_ROGUE:   return "Rogue";
	case JOB_ALCHEMIST:   return "Alchemist";
	case JOB_BARD:   return "Bard";
	case JOB_DANCER:   return "Dancer";
	case JOB_CRUSADER2:   return "Peco-Crusader";
	case JOB_WEDDING:   return "Wedding";
	case JOB_SUPER_NOVICE:   return "Super Novice";
	case JOB_GUNSLINGER: return "Gunslinger";
	case JOB_NINJA: return "Ninja";
	case JOB_XMAS: return "Christmas";
	case JOB_NOVICE_HIGH: return "Novice High";
	case JOB_SWORDMAN_HIGH: return "Swordsman High";
	case JOB_MAGE_HIGH: return "Mage High";
	case JOB_ARCHER_HIGH: return "Archer High";
	case JOB_ACOLYTE_HIGH: return "Acolyte High";
	case JOB_MERCHANT_HIGH: return "Merchant High";
	case JOB_THIEF_HIGH: return "Thief High";
	case JOB_LORD_KNIGHT: return "Lord Knight";
	case JOB_HIGH_PRIEST: return "High Priest";
	case JOB_HIGH_WIZARD: return "High Wizard";
	case JOB_WHITESMITH: return "Whitesmith";
	case JOB_SNIPER: return "Sniper";
	case JOB_ASSASSIN_CROSS: return "Assassin Cross";
	case JOB_LORD_KNIGHT2: return "Peko Knight";
	case JOB_PALADIN: return "Paladin";
	case JOB_CHAMPION: return "Champion";
	case JOB_PROFESSOR: return "Professor";
	case JOB_STALKER: return "Stalker";
	case JOB_CREATOR: return "Creator";
	case JOB_CLOWN: return "Clown";
	case JOB_GYPSY: return "Gypsy";
	case JOB_PALADIN2: return "Peko Paladin";
	case JOB_BABY: return "Baby Novice";
	case JOB_BABY_SWORDMAN: return "Baby Swordsman";
	case JOB_BABY_MAGE: return "Baby Mage";
	case JOB_BABY_ARCHER: return "Baby Archer";
	case JOB_BABY_ACOLYTE: return "Baby Acolyte";
	case JOB_BABY_MERCHANT: return "Baby Merchant";
	case JOB_BABY_THIEF: return "Baby Thief";
	case JOB_BABY_KNIGHT: return "Baby Knight";
	case JOB_BABY_PRIEST: return "Baby Priest";
	case JOB_BABY_WIZARD: return "Baby Wizard";
	case JOB_BABY_BLACKSMITH: return "Baby Blacksmith";
	case JOB_BABY_HUNTER: return "Baby Hunter";
	case JOB_BABY_ASSASSIN: return "Baby Assassin";
	case JOB_BABY_KNIGHT2: return "Baby Peco Knight";
	case JOB_BABY_CRUSADER: return "Baby Crusader";
	case JOB_BABY_MONK: return "Baby Monk";
	case JOB_BABY_SAGE: return "Baby Sage";
	case JOB_BABY_ROGUE: return "Baby Rogue";
	case JOB_BABY_ALCHEMIST: return "Baby Alchemist";
	case JOB_BABY_BARD: return "Baby Bard";
	case JOB_BABY_DANCER: return "Baby Dancer";
	case JOB_BABY_CRUSADER2: return "Baby Peco Crusader";
	case JOB_SUPER_BABY: return "Super Baby";
	case JOB_TAEKWON: return "Taekwon";
	case JOB_STAR_GLADIATOR: return "Star Gladiator";
	case JOB_STAR_GLADIATOR2: return "Flying Star Gladiator";
	case JOB_SOUL_LINKER: return "Soul Linker";
	}
	return "Unknown Job";
}

static int create_online_files_sub(DBKey key, void* data, va_list va)
{
	struct online_char_data *character;
	int* players;
	int *id;
	int j,k,l;
	character = (struct online_char_data*) data;
	players = va_arg(va, int*);
	id = va_arg(va, int*);
	
	// check if map-server is online
	if (character->server == -1 || character->char_id == -1) { //Character not currently online.
		return -1;
	}
	
	j = character->server;
	if (server[j].fd < 0) {
		server[j].users = 0;
		return -1;
	}
	// search position of character in char_dat and sort online characters.
	for(j = 0; j < char_num; j++) {
		if (char_dat[j].status.char_id != character->char_id)
			continue;
		id[*players] = j;
		// use sorting option
		switch (online_sorting_option) {
		case 1: // by name (without case sensitive)
			for(k = 0; k < *players; k++)
				if (stricmp(char_dat[j].status.name, char_dat[id[k]].status.name) < 0 ||
					// if same name, we sort with case sensitive.
					(stricmp(char_dat[j].status.name, char_dat[id[k]].status.name) == 0 &&
					 strcmp(char_dat[j].status.name, char_dat[id[k]].status.name) < 0)) {
					for(l = *players; l > k; l--)
						id[l] = id[l-1];
					id[k] = j; // id[*players]
					break;
				}
			break;
		case 2: // by zeny
			for(k = 0; k < *players; k++)
				if (char_dat[j].status.zeny < char_dat[id[k]].status.zeny ||
					// if same number of zenys, we sort by name.
					(char_dat[j].status.zeny == char_dat[id[k]].status.zeny &&
					 stricmp(char_dat[j].status.name, char_dat[id[k]].status.name) < 0)) {
					for(l = *players; l > k; l--)
						id[l] = id[l-1];
					id[k] = j; // id[*players]
					break;
				}
			break;
		case 3: // by base level
			for(k = 0; k < *players; k++)
				if (char_dat[j].status.base_level < char_dat[id[k]].status.base_level ||
					// if same base level, we sort by base exp.
					(char_dat[j].status.base_level == char_dat[id[k]].status.base_level &&
					 char_dat[j].status.base_exp < char_dat[id[k]].status.base_exp)) {
					for(l = *players; l > k; l--)
						id[l] = id[l-1];
					id[k] = j; // id[*players]
					break;
				}
			break;
		case 4: // by job (and job level)
			for(k = 0; k < *players; k++)
				if (char_dat[j].status.class_ < char_dat[id[k]].status.class_ ||
					// if same job, we sort by job level.
					(char_dat[j].status.class_ == char_dat[id[k]].status.class_ &&
					 char_dat[j].status.job_level < char_dat[id[k]].status.job_level) ||
					// if same job and job level, we sort by job exp.
					(char_dat[j].status.class_ == char_dat[id[k]].status.class_ &&
					 char_dat[j].status.job_level == char_dat[id[k]].status.job_level &&
					 char_dat[j].status.job_exp < char_dat[id[k]].status.job_exp)) {
					for(l = *players; l > k; l--)
						id[l] = id[l-1];
					id[k] = j; // id[*players]
					break;
				}
			break;
		case 5: // by location map name
		{
			const char *map1, *map2;
			map1 = mapindex_id2name(char_dat[j].status.last_point.map);
			
			for(k = 0; k < *players; k++) {
				map2 = mapindex_id2name(char_dat[id[k]].status.last_point.map);
				if (!map1 || !map2 || //Avoid sorting if either one failed to resolve.
					stricmp(map1, map2) < 0 ||
					// if same map name, we sort by name.
					(stricmp(map1, map2) == 0 &&
					 stricmp(char_dat[j].status.name, char_dat[id[k]].status.name) < 0)) {
					for(l = *players; l > k; l--)
						id[l] = id[l-1];
					id[k] = j; // id[*players]
					break;
				}
			}
		}
		break;
		default: // 0 or invalid value: no sorting
			break;
		}
	(*players)++;
	break;
	}
	return 0;
}
//-------------------------------------------------------------
// Function to create the online files (txt and html). by [Yor]
//-------------------------------------------------------------
void create_online_files(void)
{
	unsigned int k, j; // for loop with strlen comparing
	int i, l; // for loops
	int players;    // count the number of players
	FILE *fp;       // for the txt file
	FILE *fp2;      // for the html file
	char temp[256];      // to prepare what we must display
	time_t time_server;  // for number of seconds
	struct tm *datetime; // variable for time in structure ->tm_mday, ->tm_sec, ...
	int id[4096];

	if (online_display_option == 0) // we display nothing, so return
		return;

	// Get number of online players, id of each online players, and verify if a server is offline
	players = 0;
	online_char_db->foreach(online_char_db, create_online_files_sub, &players, &id);

	// write files
	fp = fopen(online_txt_filename, "w");
	if (fp != NULL) {
		fp2 = fopen(online_html_filename, "w");
		if (fp2 != NULL) {
			// get time
			time(&time_server); // get time in seconds since 1/1/1970
			datetime = localtime(&time_server); // convert seconds in structure
			strftime(temp, sizeof(temp), "%d %b %Y %X", datetime); // like sprintf, but only for date/time (05 dec 2003 15:12:52)
			// write heading
			fprintf(fp2, "<HTML>\n");
			fprintf(fp2, "  <META http-equiv=\"Refresh\" content=\"%d\">\n", online_refresh_html); // update on client explorer every x seconds
			fprintf(fp2, "  <HEAD>\n");
			fprintf(fp2, "    <TITLE>Online Players on %s</TITLE>\n", server_name);
			fprintf(fp2, "  </HEAD>\n");
			fprintf(fp2, "  <BODY>\n");
			fprintf(fp2, "    <H3>Online Players on %s (%s):</H3>\n", server_name, temp);
			fprintf(fp, "Online Players on %s (%s):\n", server_name, temp);
			fprintf(fp, "\n");

			for (i = 0; i < players; i++) {
				// if it's the first player
				if (i == 0) {
					j = 0; // count the number of characters for the txt version and to set the separate line
					fprintf(fp2, "    <table border=\"1\" cellspacing=\"1\">\n");
					fprintf(fp2, "      <tr>\n");
					if ((online_display_option & 1) || (online_display_option & 64)) {
						fprintf(fp2, "        <td><b>Name</b></td>\n");
						if (online_display_option & 64) {
							fprintf(fp, "Name                          "); // 30
							j += 30;
						} else {
							fprintf(fp, "Name                     "); // 25
							j += 25;
						}
					}
					if ((online_display_option & 6) == 6) {
						fprintf(fp2, "        <td><b>Job (levels)</b></td>\n");
						fprintf(fp, "Job                 Levels "); // 27
						j += 27;
					} else if (online_display_option & 2) {
						fprintf(fp2, "        <td><b>Job</b></td>\n");
						fprintf(fp, "Job                "); // 19
						j += 19;
					} else if (online_display_option & 4) {
						fprintf(fp2, "        <td><b>Levels</b></td>\n");
						fprintf(fp, " Levels "); // 8
						j += 8;
					}
					if (online_display_option & 24) { // 8 or 16
						fprintf(fp2, "        <td><b>Location</b></td>\n");
						if (online_display_option & 16) {
							fprintf(fp, "Location     ( x , y ) "); // 23
							j += 23;
						} else {
							fprintf(fp, "Location     "); // 13
							j += 13;
						}
					}
					if (online_display_option & 32) {
						fprintf(fp2, "        <td ALIGN=CENTER><b>zenys</b></td>\n");
						fprintf(fp, "          Zenys "); // 16
						j += 16;
					}
					fprintf(fp2, "      </tr>\n");
					fprintf(fp, "\n");
					for (k = 0; k < j; k++)
						fprintf(fp, "-");
					fprintf(fp, "\n");
				}
				fprintf(fp2, "      <tr>\n");
				// get id of the character (more speed)
				j = id[i];
				// displaying the character name
				if ((online_display_option & 1) || (online_display_option & 64)) { // without/with 'GM' display
					strcpy(temp, char_dat[j].status.name);
					l = isGM(char_dat[j].status.account_id);
					if (online_display_option & 64) {
						if (l >= online_gm_display_min_level)
							fprintf(fp, "%-24s (GM) ", temp);
						else
							fprintf(fp, "%-24s      ", temp);
					} else
						fprintf(fp, "%-24s ", temp);
					// name of the character in the html (no < >, because that create problem in html code)
					fprintf(fp2, "        <td>");
					if ((online_display_option & 64) && l >= online_gm_display_min_level)
						fprintf(fp2, "<b>");
					for (k = 0; k < strlen(temp); k++) {
						switch(temp[k]) {
						case '<': // <
							fprintf(fp2, "&lt;");
							break;
						case '>': // >
							fprintf(fp2, "&gt;");
							break;
						default:
							fprintf(fp2, "%c", temp[k]);
							break;
						};
					}
					if ((online_display_option & 64) && l >= online_gm_display_min_level)
						fprintf(fp2, "</b> (GM)");
					fprintf(fp2, "</td>\n");
				}
				// displaying of the job
				if (online_display_option & 6) {
					char * jobname = job_name(char_dat[j].status.class_);
					if ((online_display_option & 6) == 6) {
						fprintf(fp2, "        <td>%s %d/%d</td>\n", jobname, char_dat[j].status.base_level, char_dat[j].status.job_level);
						fprintf(fp, "%-18s %3d/%3d ", jobname, char_dat[j].status.base_level, char_dat[j].status.job_level);
					} else if (online_display_option & 2) {
						fprintf(fp2, "        <td>%s</td>\n", jobname);
						fprintf(fp, "%-18s ", jobname);
					} else if (online_display_option & 4) {
						fprintf(fp2, "        <td>%d/%d</td>\n", char_dat[j].status.base_level, char_dat[j].status.job_level);
						fprintf(fp, "%3d/%3d ", char_dat[j].status.base_level, char_dat[j].status.job_level);
					}
				}
				// displaying of the map
				if (online_display_option & 24) { // 8 or 16
					// prepare map name
					memcpy(temp, mapindex_id2name(char_dat[j].status.last_point.map), MAP_NAME_LENGTH);
					// write map name
					if (online_display_option & 16) { // map-name AND coordinates
						fprintf(fp2, "        <td>%s (%d, %d)</td>\n", temp, char_dat[j].status.last_point.x, char_dat[j].status.last_point.y);
						fprintf(fp, "%-12s (%3d,%3d) ", temp, char_dat[j].status.last_point.x, char_dat[j].status.last_point.y);
					} else {
						fprintf(fp2, "        <td>%s</td>\n", temp);
						fprintf(fp, "%-12s ", temp);
					}
				}
				// displaying nimber of zenys
				if (online_display_option & 32) {
					// write number of zenys
					if (char_dat[j].status.zeny == 0) { // if no zeny
						fprintf(fp2, "        <td ALIGN=RIGHT>no zeny</td>\n");
						fprintf(fp, "        no zeny ");
					} else {
						fprintf(fp2, "        <td ALIGN=RIGHT>%d z</td>\n", char_dat[j].status.zeny);
						fprintf(fp, "%13d z ", char_dat[j].status.zeny);
					}
				}
				fprintf(fp, "\n");
				fprintf(fp2, "      </tr>\n");
			}
			// If we display at least 1 player
			if (players > 0) {
				fprintf(fp2, "    </table>\n");
				fprintf(fp, "\n");
			}

			// Displaying number of online players
			if (players == 0) {
				fprintf(fp2, "    <p>No user is online.</p>\n");
				fprintf(fp, "No user is online.\n");
			} else if (players == 1) {
				fprintf(fp2, "    <p>%d user is online.</p>\n", players);
				fprintf(fp, "%d user is online.\n", players);
			} else {
				fprintf(fp2, "    <p>%d users are online.</p>\n", players);
				fprintf(fp, "%d users are online.\n", players);
			}
			fprintf(fp2, "  </BODY>\n");
			fprintf(fp2, "</HTML>\n");
			fclose(fp2);
		}
		fclose(fp);
	}

	return;
}

//---------------------------------------------------------------------
// This function return the number of online players in all map-servers
//---------------------------------------------------------------------
int count_users(void)
{
	int i, users;

	users = 0;
	for(i = 0; i < MAX_MAP_SERVERS; i++)
		if (server[i].fd >= 0)
			users += server[i].users;

	return users;
}

/// Writes char data to the buffer in the format used by the client.
/// Used in packets 0x6b (chars info) and 0x6d (new char info)
/// Returns the size (106 or 108)
int mmo_char_tobuf(uint8* buf, struct mmo_charstatus* p)
{
	if( buf == NULL || p == NULL )
		return 0;

	WBUFL(buf,0) = p->char_id;
	WBUFL(buf,4) = min(p->base_exp, LONG_MAX);
	WBUFL(buf,8) = p->zeny;
	WBUFL(buf,12) = min(p->job_exp, LONG_MAX);
	WBUFL(buf,16) = p->job_level;
	WBUFL(buf,20) = 0; // probably opt1
	WBUFL(buf,24) = 0; // probably opt2
	WBUFL(buf,28) = p->option;
	WBUFL(buf,32) = p->karma;
	WBUFL(buf,36) = p->manner;
	WBUFW(buf,40) = min(p->status_point, SHRT_MAX);
	WBUFW(buf,42) = min(p->hp, SHRT_MAX);
	WBUFW(buf,44) = min(p->max_hp, SHRT_MAX);
	WBUFW(buf,46) = min(p->sp, SHRT_MAX);
	WBUFW(buf,48) = min(p->max_sp, SHRT_MAX);
	WBUFW(buf,50) = DEFAULT_WALK_SPEED; // p->speed;
	WBUFW(buf,52) = p->class_;
	WBUFW(buf,54) = p->hair;
	WBUFW(buf,56) = p->option&0x20 ? 0 : p->weapon; //When the weapon is sent and your option is riding, the client crashes on login!?
	WBUFW(buf,58) = p->base_level;
	WBUFW(buf,60) = min(p->skill_point, SHRT_MAX);
	WBUFW(buf,62) = p->head_bottom;
	WBUFW(buf,64) = p->shield;
	WBUFW(buf,66) = p->head_top;
	WBUFW(buf,68) = p->head_mid;
	WBUFW(buf,70) = p->hair_color;
	WBUFW(buf,72) = p->clothes_color;
	memcpy(WBUFP(buf,74), p->name, NAME_LENGTH);
	WBUFB(buf,98) = min(p->str, UCHAR_MAX);
	WBUFB(buf,99) = min(p->agi, UCHAR_MAX);
	WBUFB(buf,100) = min(p->vit, UCHAR_MAX);
	WBUFB(buf,101) = min(p->int_, UCHAR_MAX);
	WBUFB(buf,102) = min(p->dex, UCHAR_MAX);
	WBUFB(buf,103) = min(p->luk, UCHAR_MAX);
	WBUFW(buf,104) = p->slot;
	if (char_rename) {
		WBUFW(buf,106) = 1;// Rename bit (0=rename,1=no rename)
		return 108;
	} else {
		return 106;
	}
}

//----------------------------------------
// Function to send characters to a player
//----------------------------------------
int mmo_char_send006b(int fd, struct char_session_data* sd)
{
	int i, j, found_num;

	found_num = 0;
	for(i = 0; i < char_num; i++) {
		if (char_dat[i].status.account_id == sd->account_id) {
			sd->found_char[found_num] = i;
			if( ++found_num == MAX_CHARS )
				break;
		}
	}
	for(i = found_num; i < MAX_CHARS; i++)
		sd->found_char[i] = -1;

	j = 24; // offset
	WFIFOHEAD(fd,j + found_num*108); // or 106(!)
	WFIFOW(fd,0) = 0x6b;
	memset(WFIFOP(fd,4), 0, 20); // unknown bytes
	for(i = 0; i < found_num; i++)
		j += mmo_char_tobuf(WFIFOP(fd,j), &char_dat[sd->found_char[i]].status);
	WFIFOW(fd,2) = j; // packet len
	WFIFOSET(fd,j);

	return 0;
}

// 離婚(char削除時に使用)
int char_divorce(struct mmo_charstatus *cs)
{
	if (cs == NULL)
		return 0;

	if (cs->partner_id > 0){
		int i, j;
		for(i = 0; i < char_num; i++) {
			if (char_dat[i].status.char_id == cs->partner_id && char_dat[i].status.partner_id == cs->char_id) {
				cs->partner_id = 0;
				char_dat[i].status.partner_id = 0;
				for(j = 0; j < MAX_INVENTORY; j++)
					if (char_dat[i].status.inventory[j].nameid == WEDDING_RING_M || char_dat[i].status.inventory[j].nameid == WEDDING_RING_F)
						memset(&char_dat[i].status.inventory[j], 0, sizeof(char_dat[i].status.inventory[0]));
					if (cs->inventory[j].nameid == WEDDING_RING_M || cs->inventory[j].nameid == WEDDING_RING_F)
						memset(&cs->inventory[j], 0, sizeof(cs->inventory[0]));
				return 0;
			}
		}
	}
	return 0;
}

int char_married(int pl1,int pl2)
{
	return (char_dat[pl1].status.char_id == char_dat[pl2].status.partner_id && char_dat[pl2].status.char_id == char_dat[pl1].status.partner_id);
}

int char_child(int parent_id, int child_id)
{
	return (char_dat[parent_id].status.child == char_dat[child_id].status.char_id && 
		((char_dat[parent_id].status.char_id == char_dat[child_id].status.father) || 
		(char_dat[parent_id].status.char_id == char_dat[child_id].status.mother)));		
}

int char_family(int cid1, int cid2, int cid3)
{
	int i, idx1 = -1, idx2 =-1;//, idx3 =-1;
	for(i = 0; i < char_num && (idx1 == -1 || idx2 == -1/* || idx3 == 1*/); i++)
  	{
		if (char_dat[i].status.char_id == cid1)
			idx1 = i;
		if (char_dat[i].status.char_id == cid2)
			idx2 = i;
//		if (char_dat[i].status.char_id == cid2)
//			idx3 = i;
	}
	if (idx1 == -1 || idx2 == -1/* || idx3 == -1*/)
  		return 0; //Some character not found??

	//Unless the dbs are corrupted, these 3 checks should suffice, even though 
	//we could do a lot more checks and force cross-reference integrity.
	if(char_dat[idx1].status.partner_id == cid2 &&
		char_dat[idx1].status.child == cid3)
		return cid3; //cid1/cid2 parents. cid3 child.

	if(char_dat[idx1].status.partner_id == cid3 &&
		char_dat[idx1].status.child == cid2)
		return cid2; //cid1/cid3 parents. cid2 child.

	if(char_dat[idx2].status.partner_id == cid3 &&
		char_dat[idx2].status.child == cid1)
		return cid1; //cid2/cid3 parents. cid1 child.
	return 0;
}

//Clears the given party id from all characters.
//Since sometimes the party format changes and parties must be wiped, this 
//method is required to prevent stress during the "party not found!" stages.
void char_clearparty(int party_id) 
{
	int i;
	for(i = 0; i < char_num; i++)
  	{
		if (char_dat[i].status.party_id == party_id)
			char_dat[i].status.party_id = 0;
	}
}

//----------------------------------------------------------------------
// Force disconnection of an online player (with account value) by [Yor]
//----------------------------------------------------------------------
int disconnect_player(int account_id)
{
	int i;
	struct char_session_data *sd;

	// disconnect player if online on char-server
	for(i = 0; i < fd_max; i++) {
		if (session[i] && (sd = (struct char_session_data*)session[i]->session_data)) {
			if (sd->account_id == account_id) {
				set_eof(i);
				return 1;
			}
		}
	}

	return 0;
}

// キャラ削除に伴うデータ削除
static int char_delete(struct mmo_charstatus *cs)
{
	int j;

	// ペット削除
	if (cs->pet_id)
		inter_pet_delete(cs->pet_id);
	if (cs->hom_id)
		inter_homun_delete(cs->hom_id);
	for (j = 0; j < MAX_INVENTORY; j++)
		if (cs->inventory[j].card[0] == (short)0xff00)
			inter_pet_delete(MakeDWord(cs->inventory[j].card[1],cs->inventory[j].card[2]));
	for (j = 0; j < MAX_CART; j++)
		if (cs->cart[j].card[0] == (short)0xff00)
			inter_pet_delete( MakeDWord(cs->cart[j].card[1],cs->cart[j].card[2]) );
	// ギルド脱退
	if (cs->guild_id)
		inter_guild_leave(cs->guild_id, cs->account_id, cs->char_id);
	// パーティー脱退
	if (cs->party_id)
		inter_party_leave(cs->party_id, cs->account_id, cs->char_id);
	// 離婚
	if (cs->partner_id){
		// 離婚情報をmapに通知
		unsigned char buf[10];
		WBUFW(buf,0) = 0x2b12;
		WBUFL(buf,2) = cs->char_id;
		WBUFL(buf,6) = cs->partner_id;
		mapif_sendall(buf,10);
		// 離婚
		char_divorce(cs);
	}
#ifdef ENABLE_SC_SAVING
	status_delete_scdata(cs->account_id, cs->char_id);
#endif
	return 0;
}

static void char_auth_ok(int fd, struct char_session_data *sd)
{
	struct online_char_data* character;
	if (max_connect_user && count_users() >= max_connect_user && isGM(sd->account_id) < gm_allow_level)
	{
		// refuse connection (over populated)
		WFIFOW(fd,0) = 0x6c;
		WFIFOW(fd,2) = 0;
		WFIFOSET(fd,3);
		return;
	}

	if( online_check && (character = (struct online_char_data*)idb_get(online_char_db, sd->account_id)) != NULL )
	{	// check if character is not online already. [Skotlex]
		if (character->server > -1)
		{	//Character already online. KICK KICK KICK
			mapif_disconnectplayer(server[character->server].fd, character->account_id, character->char_id, 2);
			if (character->waiting_disconnect == -1)
				character->waiting_disconnect = add_timer(gettick()+20000, chardb_waiting_disconnect, character->account_id, 0);
			WFIFOW(fd,0) = 0x81;
			WFIFOB(fd,2) = 8;
			WFIFOSET(fd,3);
			return;
		}
		if (character->fd >= 0 && character->fd != fd)
		{	//There's already a connection from this account that hasn't picked a char yet.
			WFIFOW(fd,0) = 0x81;
			WFIFOB(fd,2) = 8;
			WFIFOSET(fd,3);
			return;
		}
		character->fd = fd;
	}

	if (login_fd > 0) {
		// request to login-server to obtain e-mail/time limit
		//FIXME: isn't this part of the auth_ok packet? [ultramage]
		WFIFOHEAD(login_fd,6);
		WFIFOW(login_fd,0) = 0x2716;
		WFIFOL(login_fd,2) = sd->account_id;
		WFIFOSET(login_fd,6);
	}

	// mark session as 'authed'
	sd->auth = true;

	// set char online on charserver
	set_char_online(-1, 99, sd->account_id);

	// send characters to player
	mmo_char_send006b(fd, sd);
}

int send_accounts_tologin(int tid, unsigned int tick, int id, int data);

int parse_fromlogin(int fd)
{
	int i;
	struct char_session_data *sd;

	// only login-server can have an access to here.
	// so, if it isn't the login-server, we disconnect the session.
	if( fd != login_fd )
		set_eof(fd);

	if(session[fd]->flag.eof) {
		if (fd == login_fd) {
			ShowWarning("Connection to login-server lost (connection #%d).\n", fd);
			login_fd = -1;
		}
		do_close(fd);
		return 0;
	}

	sd = (struct char_session_data*)session[fd]->session_data;

	while(RFIFOREST(fd) >= 2)
	{
		uint16 command = RFIFOW(fd,0);

		switch( command )
		{

		// acknowledgement of connect-to-loginserver request
		case 0x2711:
			if (RFIFOREST(fd) < 3)
				return 0;

			if (RFIFOB(fd,2)) {
				//printf("connect login server error : %d\n", RFIFOB(fd,2));
				ShowError("Can not connect to login-server.\n");
				ShowError("The server communication passwords (default s1/p1) are probably invalid.\n");
				ShowInfo("Also, please make sure your accounts file (default: accounts.txt) has those values present.\n");
				ShowInfo("The communication passwords can be changed in map_athena.conf and char_athena.conf\n");
			} else {
				ShowStatus("Connected to login-server (connection #%d).\n", fd);
				
				//Send online accounts to login server.
				send_accounts_tologin(-1, gettick(), 0, 0);

				// if no map-server already connected, display a message...
				ARR_FIND( 0, MAX_MAP_SERVERS, i, server[i].fd > 0 && server[i].map[0] );
				if( i == MAX_MAP_SERVERS )
					ShowStatus("Awaiting maps from map-server.\n");
			}
			RFIFOSKIP(fd,3);
		break;

		// acknowledgement of account authentication request
		case 0x2713:
			if (RFIFOREST(fd) < 51)
				return 0;

			// find the session with this account id
			ARR_FIND( 0, fd_max, i, session[i] && (sd = (struct char_session_data*)session[i]->session_data) && sd->account_id == RFIFOL(fd,2) );
			if( i < fd_max )
			{
				if( RFIFOB(fd,6) != 0 ) { // failure
					WFIFOHEAD(i,3);
					WFIFOW(i,0) = 0x6c;
					WFIFOB(i,2) = 0x42;
					WFIFOSET(i,3);
				} else { // success
					memcpy(sd->email, RFIFOP(fd,7), 40);
					if (e_mail_check(sd->email) == 0)
						strncpy(sd->email, "a@a.com", 40); // default e-mail
					sd->connect_until_time = (time_t)RFIFOL(fd,47);
					char_auth_ok(i, sd);
				}
			}
			RFIFOSKIP(fd,51);
		break;

		// Receiving of an e-mail/time limit from the login-server (answer of a request because a player comes back from map-server to char-server) by [Yor]
		case 0x2717:
			if (RFIFOREST(fd) < 50)
				return 0;
			for(i = 0; i < fd_max; i++) {
				if (session[i] && (sd = (struct char_session_data*)session[i]->session_data)) {
					if (sd->account_id == RFIFOL(fd,2)) {
						memcpy(sd->email, RFIFOP(fd,6), 40);
						if (e_mail_check(sd->email) == 0)
							strncpy(sd->email, "a@a.com", 40); // default e-mail
						sd->connect_until_time = (time_t)RFIFOL(fd,46);
						break;
					}
				}
			}
			RFIFOSKIP(fd,50);
		break;

		// login-server alive packet
		case 0x2718:
			if (RFIFOREST(fd) < 2)
				return 0;
			RFIFOSKIP(fd,2);
		break;

		case 0x2721:	// gm reply
			if (RFIFOREST(fd) < 10)
				return 0;
		{
			unsigned char buf[10];
			WBUFW(buf,0) = 0x2b0b;
			WBUFL(buf,2) = RFIFOL(fd,2); // account
			WBUFL(buf,6) = RFIFOL(fd,6); // GM level
			mapif_sendall(buf,10);

			RFIFOSKIP(fd,10);
		}
		break;

		case 0x2723:	// changesex reply (modified by [Yor])
			if (RFIFOREST(fd) < 7)
				return 0;
		{
			int acc, sex, i, j;
			unsigned char buf[7];
			acc = RFIFOL(fd,2);
			sex = RFIFOB(fd,6);
			if (acc > 0) {
				for(i = 0; i < AUTH_FIFO_SIZE; i++) {
					if (auth_fifo[i].account_id == acc)	
						auth_fifo[i].sex = sex;
				}
				for (i = 0; i < char_num; i++) {
					if (char_dat[i].status.account_id == acc) {
						int jobclass = char_dat[i].status.class_;
						char_dat[i].status.sex = sex;
						if (jobclass == JOB_BARD || jobclass == JOB_DANCER ||
						    jobclass == JOB_CLOWN || jobclass == JOB_GYPSY ||
						    jobclass == JOB_BABY_BARD || jobclass == JOB_BABY_DANCER) {
							// job modification
							if (jobclass == JOB_BARD || jobclass == JOB_DANCER) {
								char_dat[i].status.class_ = (sex) ? JOB_BARD : JOB_DANCER;
							} else if (jobclass == JOB_CLOWN || jobclass == JOB_GYPSY) {
								char_dat[i].status.class_ = (sex) ? JOB_CLOWN : JOB_GYPSY;
							} else if (jobclass == JOB_BABY_BARD || jobclass == JOB_BABY_DANCER) {
								char_dat[i].status.class_ = (sex) ? JOB_BABY_BARD : JOB_BABY_DANCER;
							}
							// remove specifical skills of classes 19, 4020 and 4042
							for(j = 315; j <= 322; j++) {
								if (char_dat[i].status.skill[j].id > 0 && !char_dat[i].status.skill[j].flag) {
									if (char_dat[i].status.skill_point > USHRT_MAX - char_dat[i].status.skill[j].lv)
										char_dat[i].status.skill_point = USHRT_MAX;
									else
										char_dat[i].status.skill_point += char_dat[i].status.skill[j].lv;
									char_dat[i].status.skill[j].id = 0;
									char_dat[i].status.skill[j].lv = 0;
								}
							}
							// remove specifical skills of classes 20, 4021 and 4043
							for(j = 323; j <= 330; j++) {
								if (char_dat[i].status.skill[j].id > 0 && !char_dat[i].status.skill[j].flag) {
									if (char_dat[i].status.skill_point > USHRT_MAX - char_dat[i].status.skill[j].lv)
										char_dat[i].status.skill_point = USHRT_MAX;
									else
										char_dat[i].status.skill_point += char_dat[i].status.skill[j].lv;

									char_dat[i].status.skill[j].id = 0;
									char_dat[i].status.skill[j].lv = 0;
								}
							}
						}
						// to avoid any problem with equipment and invalid sex, equipment is unequiped.
						for (j = 0; j < MAX_INVENTORY; j++) {
							if (char_dat[i].status.inventory[j].nameid && char_dat[i].status.inventory[j].equip)
								char_dat[i].status.inventory[j].equip = 0;
						}
						char_dat[i].status.weapon = 0;
						char_dat[i].status.shield = 0;
						char_dat[i].status.head_top = 0;
						char_dat[i].status.head_mid = 0;
						char_dat[i].status.head_bottom = 0;

						if (char_dat[i].status.guild_id)	//If there is a guild, update the guild_member data [Skotlex]
							inter_guild_sex_changed(char_dat[i].status.guild_id, acc, char_dat[i].status.char_id, sex);
					}
				}
				// disconnect player if online on char-server
				disconnect_player(acc);
			}
			WBUFW(buf,0) = 0x2b0d;
			WBUFL(buf,2) = acc;
			WBUFB(buf,6) = sex;
			mapif_sendall(buf, 7);

			RFIFOSKIP(fd,7);
		}
		break;

		case 0x2726:	// Request to send a broadcast message (no answer)
			if (RFIFOREST(fd) < 8 || RFIFOREST(fd) < (8 + RFIFOL(fd,4)))
				return 0;
			if (RFIFOL(fd,4) < 1)
				char_log("Receiving a message for broadcast, but message is void.\n");
			else
			{
				// at least 1 map-server
				ARR_FIND( 0, MAX_MAP_SERVERS, i, server[i].fd >= 0 );
				if (i == MAX_MAP_SERVERS)
					char_log("'ladmin': Receiving a message for broadcast, but no map-server is online.\n");
				else {
					unsigned char buf[128];
					char message[4096]; // +1 to add a null terminated if not exist in the packet
					int lp;
					char *p;
					memset(message, '\0', sizeof(message));
					memcpy(message, RFIFOP(fd,8), RFIFOL(fd,4));
					message[sizeof(message)-1] = '\0';
					remove_control_chars(message);
					// remove all first spaces
					p = message;
					while(p[0] == ' ')
						p++;
					// if message is only composed of spaces
					if (p[0] == '\0')
						char_log("Receiving a message for broadcast, but message is only a lot of spaces.\n");
					// else send message to all map-servers
					else {
						if (RFIFOW(fd,2) == 0) {
							char_log("'ladmin': Receiving a message for broadcast (message (in yellow): %s)\n",
							         message);
							lp = 4;
						} else {
							char_log("'ladmin': Receiving a message for broadcast (message (in blue): %s)\n",
							         message);
							lp = 8;
						}
						// split message to max 80 char
						while(p[0] != '\0') { // if not finish
							if (p[0] == ' ') // jump if first char is a space
								p++;
							else {
								char split[80];
								char* last_space;
								sscanf(p, "%79[^\t]", split); // max 79 char, any char (\t is control char and control char was removed before)
								split[sizeof(split)-1] = '\0'; // last char always \0
								if ((last_space = strrchr(split, ' ')) != NULL) { // searching space from end of the string
									last_space[0] = '\0'; // replace it by NULL to have correct length of split
									p++; // to jump the new NULL
								}
								p += strlen(split);
								// send broadcast to all map-servers
								WBUFW(buf,0) = 0x3800;
								WBUFW(buf,2) = lp + strlen(split) + 1;
								WBUFL(buf,4) = 0x65756c62; // only write if in blue (lp = 8)
								memcpy(WBUFP(buf,lp), split, strlen(split) + 1);
								mapif_sendall(buf, WBUFW(buf,2));
							}
						}
					}
				}
			}
			RFIFOSKIP(fd,8 + RFIFOL(fd,4));
		break;

		// account_reg2変更通知
		case 0x2729:
			if (RFIFOREST(fd) < 4 || RFIFOREST(fd) < RFIFOW(fd,2))
				return 0;
		{	//Receive account_reg2 registry, forward to map servers.
			unsigned char buf[ACCOUNT_REG2_NUM*(256+32+2)+16];
			memcpy(buf,RFIFOP(fd,0), RFIFOW(fd,2));
//			WBUFW(buf,0) = 0x2b11;
			WBUFW(buf,0) = 0x3804; //Map server can now receive all kinds of reg values with the same packet. [Skotlex]
			mapif_sendall(buf, WBUFW(buf,2));
			RFIFOSKIP(fd, RFIFOW(fd,2));
		}
		break;

		// Account deletion notification (from login-server)
		case 0x2730:
			if (RFIFOREST(fd) < 6)
				return 0;
			// Deletion of all characters of the account
			for(i = 0; i < char_num; i++) {
				if (char_dat[i].status.account_id == RFIFOL(fd,2)) {
					char_delete(&char_dat[i].status);
					if (i < char_num - 1) {
						memcpy(&char_dat[i], &char_dat[char_num-1], sizeof(struct character_data));
						// if moved character owns to deleted account, check again it's character
						if (char_dat[i].status.account_id == RFIFOL(fd,2)) {
							i--;
						// Correct moved character reference in the character's owner by [Yor]
						} else {
							int j, k;
							struct char_session_data *sd2;
							for (j = 0; j < fd_max; j++) {
								if (session[j] && (sd2 = (struct char_session_data*)session[j]->session_data) &&
									sd2->account_id == char_dat[char_num-1].status.account_id) {
									for (k = 0; k < MAX_CHARS; k++) {
										if (sd2->found_char[k] == char_num-1) {
											sd2->found_char[k] = i;
											break;
										}
									}
									break;
								}
							}
						}
					}
					char_num--;
				}
			}
			// Deletion of the storage
			inter_storage_delete(RFIFOL(fd,2));
			// send to all map-servers to disconnect the player
			{
				unsigned char buf[6];
				WBUFW(buf,0) = 0x2b13;
				WBUFL(buf,2) = RFIFOL(fd,2);
				mapif_sendall(buf, 6);
			}
			// disconnect player if online on char-server
			disconnect_player(RFIFOL(fd,2));
			RFIFOSKIP(fd,6);
		break;

		// State change of account/ban notification (from login-server) by [Yor]
		case 0x2731:
			if (RFIFOREST(fd) < 11)
				return 0;
			// send to all map-servers to disconnect the player
		{
			unsigned char buf[11];
			WBUFW(buf,0) = 0x2b14;
			WBUFL(buf,2) = RFIFOL(fd,2);
			WBUFB(buf,6) = RFIFOB(fd,6); // 0: change of statut, 1: ban
			WBUFL(buf,7) = RFIFOL(fd,7); // status or final date of a banishment
			mapif_sendall(buf, 11);
		}
			// disconnect player if online on char-server
			disconnect_player(RFIFOL(fd,2));
			RFIFOSKIP(fd,11);
		break;

		// Receiving GM acounts info from login-server (by [Yor])
		case 0x2732:
			if (RFIFOREST(fd) < 4 || RFIFOREST(fd) < RFIFOW(fd,2))
				return 0;
		{
			unsigned char buf[32000]; //FIXME: this will crash
			if (gm_account != NULL)
				aFree(gm_account);
			CREATE(gm_account, struct gm_account, (RFIFOW(fd,2) - 4)/5);
			GM_num = 0;
			for (i = 4; i < RFIFOW(fd,2); i = i + 5) {
				gm_account[GM_num].account_id = RFIFOL(fd,i);
				gm_account[GM_num].level = (int)RFIFOB(fd,i+4);
				//printf("GM account: %d -> level %d\n", gm_account[GM_num].account_id, gm_account[GM_num].level);
				GM_num++;
			}
			ShowStatus("From login-server: receiving information of %d GM accounts.\n", GM_num);
			char_log("From login-server: receiving information of %d GM accounts.\n", GM_num);
			// send new gm acccounts level to map-servers
			memcpy(buf, RFIFOP(fd,0), RFIFOW(fd,2));
			WBUFW(buf,0) = 0x2b15;
			mapif_sendall(buf, RFIFOW(fd,2));

			RFIFOSKIP(fd,RFIFOW(fd,2));
		}
		break;

		//Login server request to kick a character out. [Skotlex]
		case 0x2734:
			if (RFIFOREST(fd) < 6)
				return 0;
		{
			struct online_char_data* character;
			int aid = RFIFOL(fd,2);
			if ((character = (struct online_char_data*)idb_get(online_char_db, aid)) != NULL)
			{	//Kick out this player.
				if (character->server > -1)
				{	//Kick it from the map server it is on.
					mapif_disconnectplayer(server[character->server].fd, character->account_id, character->char_id, 2);
					if (character->waiting_disconnect == -1)
						character->waiting_disconnect = add_timer(gettick()+15000, chardb_waiting_disconnect, character->account_id, 0);
				} else { //Manual kick from char server.
					struct char_session_data *tsd;
					int i;
					for(i = 0; i < fd_max; i++) {
						if (session[i] && (tsd = (struct char_session_data*)session[i]->session_data) && tsd->account_id == aid)
						{
							WFIFOHEAD(i,3);
							WFIFOW(i,0) = 0x81;
							WFIFOB(i,2) = 2; // "Someone has already logged in with this id"
							WFIFOSET(i,3);
							break;
						}
					}
					if (i == fd_max) //Shouldn't happen, but just in case.
						set_char_offline(99, aid);
				}
			}
			RFIFOSKIP(fd,6);
		}
		break;
		
		case 0x2735:
		{
			unsigned char buf[2];
			uint32 new_ip = 0;

			WBUFW(buf,0) = 0x2b1e;
			mapif_sendall(buf, 2);

			new_ip = host2ip(login_ip_str);
			if (new_ip && new_ip != login_ip)
				login_ip = new_ip; //Update login up.

			new_ip = host2ip(char_ip_str);
			if (new_ip && new_ip != char_ip)
			{	//Update ip.
				char_ip = new_ip;
				ShowInfo("Updating IP for [%s].\n", char_ip_str);
				WFIFOHEAD(fd,6);
				WFIFOW(fd,0) = 0x2736;
				WFIFOL(fd,2) = htonl(char_ip);
				WFIFOSET(fd,6);
			}

			RFIFOSKIP(fd,2);
		}
		break;

		default:
			ShowError("Unknown packet 0x%04x received from login-server, disconnecting.\n", RFIFOW(fd,0));
			set_eof(fd);
			return 0;
		}
	}

	RFIFOFLUSH(fd);
	return 0;
}

int request_accreg2(int account_id, int char_id)
{
	if (login_fd > 0) {
		WFIFOHEAD(login_fd,10);
		WFIFOW(login_fd,0) = 0x272e;
		WFIFOL(login_fd,2) = account_id;
		WFIFOL(login_fd,6) = char_id;
		WFIFOSET(login_fd,10);
		return 1;
	}
	return 0;
}

//Send packet forward to login-server for account saving
int save_accreg2(unsigned char* buf, int len)
{
	if (login_fd > 0) {
		WFIFOHEAD(login_fd,len+4);
		memcpy(WFIFOP(login_fd,4), buf, len);
		WFIFOW(login_fd,0) = 0x2728;
		WFIFOW(login_fd,2) = len+4;
		WFIFOSET(login_fd,len+4);
		return 1;
	}
	return 0;
}

//Receive Registry information for a character.
int char_parse_Registry(int account_id, int char_id, unsigned char *buf, int buf_len)
{
	int i,j,p,len;
	for (i = 0; i < char_num; i++) {
		if (char_dat[i].status.account_id == account_id && char_dat[i].status.char_id == char_id)
			break;
	}
	if(i >= char_num) //Character not found?
		return 1;
	for(j=0,p=0;j<GLOBAL_REG_NUM && p<buf_len;j++){
		sscanf((char*)WBUFP(buf,p), "%31c%n",char_dat[i].global[j].str,&len);
		char_dat[i].global[j].str[len]='\0';
		p +=len+1; //+1 to skip the '\0' between strings.
		sscanf((char*)WBUFP(buf,p), "%255c%n",char_dat[i].global[j].value,&len);
		char_dat[i].global[j].value[len]='\0';
		p +=len+1;
	}
	char_dat[i].global_num = j;
	return 0;
}

//Reply to map server with acc reg values.
int char_account_reg_reply(int fd,int account_id,int char_id)
{
	int i,j,p;
	WFIFOHEAD(fd, GLOBAL_REG_NUM*288 + 13);
	WFIFOW(fd,0)=0x3804;
	WFIFOL(fd,4)=account_id;
	WFIFOL(fd,8)=char_id;
	WFIFOB(fd,12)=3; //Type 3: char acc reg.
	for (i = 0;i < char_num; i++) {
		if (char_dat[i].status.account_id == account_id && char_dat[i].status.char_id	== char_id)
			break;
	}
	if(i >= char_num){ //Character not found? Sent empty packet.
		WFIFOW(fd,2)=13;
	}else{
		for (p=13,j = 0; j < char_dat[i].global_num; j++) {
			if (char_dat[i].global[j].str[0]) {
				p+= sprintf((char*)WFIFOP(fd,p), "%s", char_dat[i].global[j].str)+1; //We add 1 to consider the '\0' in place.
				p+= sprintf((char*)WFIFOP(fd,p), "%s", char_dat[i].global[j].value)+1;
			}
		}
		WFIFOW(fd,2)=p;
	}
	WFIFOSET(fd,WFIFOW(fd,2));
	return 0;
}

void char_read_fame_list(void)
{
	int i, j, k;
	struct fame_list fame_item;
	CREATE_BUFFER(id, int, char_num);

	for(i = 0; i < char_num; i++) {
		id[i] = i;
		for(j = 0; j < i; j++) {
			if (char_dat[i].status.fame > char_dat[id[j]].status.fame) {
				for(k = i; k > j; k--)
					id[k] = id[k-1];
				id[j] = i; // id[i]
				break;
			}
		}
	}

	// Empty ranking lists
	memset(smith_fame_list, 0, sizeof(smith_fame_list));
	memset(chemist_fame_list, 0, sizeof(chemist_fame_list));
	memset(taekwon_fame_list, 0, sizeof(taekwon_fame_list));
	// Build Blacksmith ranking list
	for (i = 0, j = 0; i < char_num && j < fame_list_size_smith; i++) {
		if (char_dat[id[i]].status.fame && (
			char_dat[id[i]].status.class_ == JOB_BLACKSMITH ||
			char_dat[id[i]].status.class_ == JOB_WHITESMITH ||
			char_dat[id[i]].status.class_ == JOB_BABY_BLACKSMITH))
		{
			fame_item.id = char_dat[id[i]].status.char_id;
			fame_item.fame = char_dat[id[i]].status.fame;
			strncpy(fame_item.name, char_dat[id[i]].status.name, NAME_LENGTH);

			memcpy(&smith_fame_list[j],&fame_item,sizeof(struct fame_list));
			j++;
		}
	}
	// Build Alchemist ranking list
	for (i = 0, j = 0; i < char_num && j < fame_list_size_chemist; i++) {
		if (char_dat[id[i]].status.fame && (
			char_dat[id[i]].status.class_ == JOB_ALCHEMIST ||
			char_dat[id[i]].status.class_ == JOB_CREATOR ||
			char_dat[id[i]].status.class_ == JOB_BABY_ALCHEMIST))
		{
			fame_item.id = char_dat[id[i]].status.char_id;
			fame_item.fame = char_dat[id[i]].status.fame;
			strncpy(fame_item.name, char_dat[id[i]].status.name, NAME_LENGTH);

			memcpy(&chemist_fame_list[j],&fame_item,sizeof(struct fame_list));

			j++;
		}
	}
	// Build Taekwon ranking list
	for (i = 0, j = 0; i < char_num && j < fame_list_size_taekwon; i++) {
		if (char_dat[id[i]].status.fame &&
			char_dat[id[i]].status.class_ == JOB_TAEKWON)
		{
			fame_item.id = char_dat[id[i]].status.char_id;
			fame_item.fame = char_dat[id[i]].status.fame;
			strncpy(fame_item.name, char_dat[id[i]].status.name, NAME_LENGTH);

			memcpy(&taekwon_fame_list[j],&fame_item,sizeof(struct fame_list));

			j++;
		}
	}
	DELETE_BUFFER(id);
}
// Send map-servers the fame ranking lists
int char_send_fame_list(int fd)
{
	int i, len = 8;
	unsigned char buf[32000];
	
	WBUFW(buf,0) = 0x2b1b;

	for(i = 0; i < fame_list_size_smith && smith_fame_list[i].id; i++) {
		memcpy(WBUFP(buf, len), &smith_fame_list[i], sizeof(struct fame_list));
		len += sizeof(struct fame_list);
	}
	// add blacksmith's block length
	WBUFW(buf, 6) = len;

	for(i = 0; i < fame_list_size_chemist && chemist_fame_list[i].id; i++) {
		memcpy(WBUFP(buf, len), &chemist_fame_list[i], sizeof(struct fame_list));
		len += sizeof(struct fame_list);
	}
	// add alchemist's block length
	WBUFW(buf, 4) = len;

	for(i = 0; i < fame_list_size_taekwon && taekwon_fame_list[i].id; i++) {
		memcpy(WBUFP(buf, len), &taekwon_fame_list[i], sizeof(struct fame_list));
		len += sizeof(struct fame_list);
	}
	// add total packet length
	WBUFW(buf, 2) = len;

	if(fd!=-1)
		mapif_send(fd, buf, len);
	else
		mapif_sendall(buf, len);

	return 0;
}

void char_update_fame_list(int type, int index, int fame)
{
	unsigned char buf[9];
	WBUFW(buf,0) = 0x2b22;
	WBUFB(buf,2) = type;
	WBUFB(buf,3) = index;
	WBUFL(buf,4) = fame;
	mapif_sendall(buf, 8);
}

//Loads a character's name and stores it in the buffer given (must be NAME_LENGTH in size)
//Returns 1 on found, 0 on not found (buffer is filled with Unknown char name)
int char_loadName(int char_id, char* name)
{
	int j;
	for( j = 0; j < char_num && char_dat[j].status.char_id != char_id; ++j )
		;// find char
	if( j < char_num )
		strncpy(name, char_dat[j].status.name, NAME_LENGTH);
	else
		strncpy(name, unknown_char_name, NAME_LENGTH);

	return (j < char_num) ? 1 : 0;
}

int search_mapserver(unsigned short map, uint32 ip, uint16 port);

int parse_frommap(int fd)
{
	int i, j;
	int id;

	ARR_FIND( 0, MAX_MAP_SERVERS, id, server[id].fd == fd );
	if(id == MAX_MAP_SERVERS)
		set_eof(fd);
	if(session[fd]->flag.eof) {
		if (id < MAX_MAP_SERVERS) {
			unsigned char buf[16384];
			ShowStatus("Map-server %d (session #%d) has disconnected.\n", id, fd);
			//Notify other map servers that this one is gone. [Skotlex]
			WBUFW(buf,0) = 0x2b20;
			WBUFL(buf,4) = htonl(server[id].ip);
			WBUFW(buf,8) = htons(server[id].port);
			j = 0;
			for(i = 0; i < MAX_MAP_PER_SERVER; i++)
				if (server[id].map[i])
					WBUFW(buf,10+(j++)*4) = server[id].map[i];
			if (j > 0) {
				WBUFW(buf,2) = j * 4 + 10;
				mapif_sendallwos(fd, buf, WBUFW(buf,2));
			}
			server[id].fd = -1;
			online_char_db->foreach(online_char_db,char_db_setoffline,i); //Tag relevant chars as 'in disconnected' server.
		}
		do_close(fd);
		create_online_files();
		return 0;
	}

	while(RFIFOREST(fd) >= 2)
	{
		//ShowDebug("Received packet 0x%4x (%d bytes) from map-server (connection %d)\n", RFIFOW(fd, 0), RFIFOREST(fd), fd);

		switch(RFIFOW(fd,0))
		{

		case 0x2af7: // request from map-server to reload GM accounts. Transmission to login-server
			if (login_fd > 0) { // don't send request if no login-server
				WFIFOHEAD(login_fd,2);
				WFIFOW(login_fd,0) = 0x2709;
				WFIFOSET(login_fd,2);
			}
			RFIFOSKIP(fd,2);
		break;

		case 0x2afa: // Receiving map names list from the map-server
			if (RFIFOREST(fd) < 4 || RFIFOREST(fd) < RFIFOW(fd,2))
				return 0;

			memset(server[id].map, 0, sizeof(server[id].map));
			j = 0;
			for(i = 4; i < RFIFOW(fd,2); i += 4) {
				server[id].map[j] = RFIFOW(fd,i);
				j++;
			}

			ShowStatus("Map-Server %d connected: %d maps, from IP %d.%d.%d.%d port %d.\n",
						id, j, CONVIP(server[id].ip), server[id].port);
			ShowStatus("Map-server %d loading complete.\n", id);
			char_log("Map-Server %d connected: %d maps, from IP %d.%d.%d.%d port %d. Map-server %d loading complete.\n",
						id, j, CONVIP(server[id].ip), server[id].port, id);

			if (max_account_id != DEFAULT_MAX_ACCOUNT_ID || max_char_id != DEFAULT_MAX_CHAR_ID)
				mapif_send_maxid(max_account_id, max_char_id); //Send the current max ids to the server to keep in sync [Skotlex]

			// send name for wisp to player
			WFIFOHEAD(fd, 3 + NAME_LENGTH);
			WFIFOW(fd,0) = 0x2afb;
			WFIFOB(fd,2) = 0;
			memcpy(WFIFOP(fd,3), wisp_server_name, NAME_LENGTH);
			WFIFOSET(fd,3+NAME_LENGTH);

			char_send_fame_list(fd); //Send fame list.

			{
			unsigned char buf[16384];
			int x;
			if (j == 0) {
				ShowWarning("Map-server %d has NO maps.\n", id);
				char_log("WARNING: Map-server %d has NO maps.\n", id);
			} else {
				// Transmitting maps information to the other map-servers
				WBUFW(buf,0) = 0x2b04;
				WBUFW(buf,2) = j * 4 + 10;
				WBUFL(buf,4) = htonl(server[id].ip);
				WBUFW(buf,8) = htons(server[id].port);
				memcpy(WBUFP(buf,10), RFIFOP(fd,4), j * 4);
				mapif_sendallwos(fd, buf, WBUFW(buf,2));
			}
			// Transmitting the maps of the other map-servers to the new map-server
			for(x = 0; x < MAX_MAP_SERVERS; x++) {
				if (server[x].fd > 0 && x != id) {
					WFIFOHEAD(fd,10 +4*MAX_MAP_PER_SERVER);
					WFIFOW(fd,0) = 0x2b04;
					WFIFOL(fd,4) = htonl(server[x].ip);
					WFIFOW(fd,8) = htons(server[x].port);
					j = 0;
					for(i = 0; i < MAX_MAP_PER_SERVER; i++)
						if (server[x].map[i])
							WFIFOW(fd,10+(j++)*4) = server[x].map[i];
					if (j > 0) {
						WFIFOW(fd,2) = j * 4 + 10;
						WFIFOSET(fd,WFIFOW(fd,2));
					}
				}
			}
			}
			RFIFOSKIP(fd,RFIFOW(fd,2));
		break;

		case 0x2afc: //Packet command is now used for sc_data request. [Skotlex]
			if (RFIFOREST(fd) < 10)
				return 0;
		{
#ifdef ENABLE_SC_SAVING
			int aid, cid;
			struct scdata *data;
			aid = RFIFOL(fd,2);
			cid = RFIFOL(fd,6);
			data = status_search_scdata(aid, cid);
			if (data->count > 0)
			{	//Deliver status change data.
				WFIFOW(fd,0) = 0x2b1d;
				WFIFOW(fd,2) = 14 + data->count*sizeof(struct status_change_data);
				WFIFOL(fd,4) = aid;
				WFIFOL(fd,8) = cid;
				WFIFOW(fd,12) = data->count;
				for (i = 0; i < data->count; i++)
					memcpy(WFIFOP(fd,14+i*sizeof(struct status_change_data)), &data->data[i], sizeof(struct status_change_data));
				WFIFOSET(fd, WFIFOW(fd,2));
				status_delete_scdata(aid, cid); //Data sent, so it needs be discarded now.
			}
#endif
			RFIFOSKIP(fd, 10);
		}
		break;
		
		case 0x2afe: //set MAP user count
			if (RFIFOREST(fd) < 4)
				return 0;
			if (RFIFOW(fd,2) != server[id].users) {
				server[id].users = RFIFOW(fd,2);
				ShowInfo("User Count: %d (Server: %d)\n", server[id].users, id);
			}
			RFIFOSKIP(fd, 4);
			break;

		case 0x2aff: //set MAP users
			if (RFIFOREST(fd) < 6 || RFIFOREST(fd) < RFIFOW(fd,2))
				return 0;
			//TODO: When data mismatches memory, update guild/party online/offline states.
			server[id].users = RFIFOW(fd,4);
			online_char_db->foreach(online_char_db,char_db_setoffline,id); //Set all chars from this server as 'unknown'
			for(i = 0; i < server[id].users; i++) {
				int aid, cid;
				struct online_char_data* character;
				aid = RFIFOL(fd,6+i*8);
				cid = RFIFOL(fd,6+i*8+4);
				character = (struct online_char_data*)idb_ensure(online_char_db, aid, create_online_char_data);
				if (online_check && character->server > -1 && character->server != id)
				{
					ShowNotice("Set map user: Character (%d:%d) marked on map server %d, but map server %d claims to have (%d:%d) online!\n",
						character->account_id, character->char_id, character->server, id, aid, cid);
					mapif_disconnectplayer(server[character->server].fd, character->account_id, character->char_id, 2);
				}
				character->char_id = cid;
				character->server = id;
			}
			//If any chars remain in -2, they will be cleaned in the cleanup timer.
			RFIFOSKIP(fd,6+i*8);
		break;

		case 0x2b01: // Receive character data from map-server for saving
			if (RFIFOREST(fd) < 4 || RFIFOREST(fd) < RFIFOW(fd,2))
				return 0;
			for(i = 0; i < char_num; i++) {
				if (char_dat[i].status.account_id == RFIFOL(fd,4) &&
				    char_dat[i].status.char_id == RFIFOL(fd,8))
					break;
			}
			if (i != char_num)
				memcpy(&char_dat[i].status, RFIFOP(fd,13), sizeof(struct mmo_charstatus));
			if (RFIFOB(fd,12))
			{	//Flag, set character offline. [Skotlex]
				set_char_offline(RFIFOL(fd,8),RFIFOL(fd,4));
				WFIFOW(fd,0) = 0x2b21; //Save ack only needed on final save.
				WFIFOL(fd,2) = RFIFOL(fd,4);
				WFIFOL(fd,6) = RFIFOL(fd,8);
				WFIFOSET(fd,10);
			}
			RFIFOSKIP(fd,RFIFOW(fd,2));
		break;

		case 0x2b02: // req char selection
			if (RFIFOREST(fd) < 18)
				return 0;

			if (auth_fifo_pos >= AUTH_FIFO_SIZE)
				auth_fifo_pos = 0;

			auth_fifo[auth_fifo_pos].account_id = RFIFOL(fd,2);
			auth_fifo[auth_fifo_pos].char_id = 0;
			auth_fifo[auth_fifo_pos].login_id1 = RFIFOL(fd,6);
			auth_fifo[auth_fifo_pos].login_id2 = RFIFOL(fd,10);
			auth_fifo[auth_fifo_pos].delflag = 2;
			auth_fifo[auth_fifo_pos].connect_until_time = 0; // unlimited/unknown time by default (not display in map-server)
			auth_fifo[auth_fifo_pos].ip = ntohl(RFIFOL(fd,14));
			auth_fifo_pos++;
			WFIFOHEAD(fd,7);
			WFIFOW(fd,0) = 0x2b03;
			WFIFOL(fd,2) = RFIFOL(fd,2);
			WFIFOB(fd,6) = 0;
			WFIFOSET(fd,7);
			RFIFOSKIP(fd,18);
		break;

		case 0x2b05: // request "change map server"
			if (RFIFOREST(fd) < 35)
				return 0;
		{
			unsigned short name;
			int map_id, map_fd = -1;
			struct online_char_data* data;
			struct mmo_charstatus* char_data;

			name = RFIFOW(fd,18);
			map_id = search_mapserver(name, ntohl(RFIFOL(fd,24)), ntohs(RFIFOW(fd,28))); //Locate mapserver by ip and port.
			if (map_id >= 0)
				map_fd = server[map_id].fd;
			for(i = 0; i < char_num; i++) {
			if (char_dat[i].status.account_id == RFIFOL(fd,2) &&
			    char_dat[i].status.char_id == RFIFOL(fd,14))
				break;
			}
			char_data = i < char_num ? &char_dat[i].status : NULL;
			//Tell the new map server about this player using Kevin's new auth packet. [Skotlex]
			if (map_fd >= 0 && session[map_fd] && char_data) 
			{	//Send the map server the auth of this player.
				//Update the "last map" as this is where the player must be spawned on the new map server.
				char_data->last_point.map = RFIFOW(fd,18);
				char_data->last_point.x = RFIFOW(fd,20);
				char_data->last_point.y = RFIFOW(fd,22);
				char_data->sex = RFIFOB(fd,30);

				WFIFOHEAD(map_fd, 20 + sizeof(struct mmo_charstatus));
				WFIFOW(map_fd,0) = 0x2afd;
				WFIFOW(map_fd,2) = 20 + sizeof(struct mmo_charstatus);
				WFIFOL(map_fd,4) = RFIFOL(fd,2); //Account ID
				WFIFOL(map_fd,8) = RFIFOL(fd,6); //Login1
				WFIFOL(map_fd,16) = RFIFOL(fd,10); //Login2
				WFIFOL(map_fd,12) = (unsigned long)0; //TODO: connect_until_time, how do I figure it out right now?
				memcpy(WFIFOP(map_fd,20), char_data, sizeof(struct mmo_charstatus));
				WFIFOSET(map_fd, WFIFOW(map_fd,2));

				data = (struct online_char_data*)idb_ensure(online_char_db, RFIFOL(fd,2), create_online_char_data);
				data->char_id = char_data->char_id;
				data->server = map_id; //Update server where char is.

				//Reply with an ack.
				WFIFOHEAD(fd,30);
				WFIFOW(fd,0) = 0x2b06;
				memcpy(WFIFOP(fd,2), RFIFOP(fd,2), 28);
				WFIFOSET(fd,30);
			} else { //Reply with nak
				WFIFOHEAD(fd,30);
				WFIFOW(fd,0) = 0x2b06;
				memcpy(WFIFOP(fd,2), RFIFOP(fd,2), 28);
				WFIFOL(fd,6) = 0; //Set login1 to 0.
				WFIFOSET(fd,30);
			}
			RFIFOSKIP(fd,35);
		}
		break;

		case 0x2b08: // char name request
			if (RFIFOREST(fd) < 6)
				return 0;

			WFIFOHEAD(fd,30);
			WFIFOW(fd,0) = 0x2b09;
			WFIFOL(fd,2) = RFIFOL(fd,2);
			char_loadName((int)RFIFOL(fd,2), (char*)WFIFOP(fd,6));
			WFIFOSET(fd,30);

			RFIFOSKIP(fd,6);
		break;

		case 0x2b0a: // request to become GM
			if (RFIFOREST(fd) < 4 || RFIFOREST(fd) < RFIFOW(fd,2))
				return 0;
			if (login_fd > 0) { // don't send request if no login-server
				WFIFOHEAD(login_fd, RFIFOW(fd,2));
				WFIFOW(login_fd,0) = 0x2720;
				memcpy(WFIFOP(login_fd,2), RFIFOP(fd,2), RFIFOW(fd,2)-2);
				WFIFOSET(login_fd, RFIFOW(fd,2));
			} else {
				WFIFOHEAD(fd,10);
				WFIFOW(fd,0) = 0x2b0b;
				WFIFOL(fd,2) = RFIFOL(fd,4);
				WFIFOL(fd,6) = 0;
				WFIFOSET(fd,10);
			}
			RFIFOSKIP(fd, RFIFOW(fd,2));
		break;

		case 0x2b0c: // Map server send information to change an email of an account -> login-server
			if (RFIFOREST(fd) < 86)
				return 0;
			if (login_fd > 0) { // don't send request if no login-server
				WFIFOHEAD(login_fd,86);
				memcpy(WFIFOP(login_fd,0), RFIFOP(fd,0),86); // 0x2722 <account_id>.L <actual_e-mail>.40B <new_e-mail>.40B
				WFIFOW(login_fd,0) = 0x2722;
				WFIFOSET(login_fd,86);
			}
			RFIFOSKIP(fd, 86);
		break;

		case 0x2b0e: // Request from map-server to change an account's status (will just be forwarded to login server)
			if (RFIFOREST(fd) < 44)
				return 0;
		{
			int result = 0; // 0-login-server request done, 1-player not found, 2-gm level too low, 3-login-server offline
			char character_name[NAME_LENGTH];

			int acc = RFIFOL(fd,2); // account_id of who ask (-1 if server itself made this request)
			const char* name = (char*)RFIFOP(fd,6); // name of the target character
			int type = RFIFOW(fd,30); // type of operation: 1-block, 2-ban, 3-unblock, 4-unban
			short year = RFIFOW(fd,32);
			short month = RFIFOW(fd,34);
			short day = RFIFOW(fd,36);
			short hour = RFIFOW(fd,38);
			short minute = RFIFOW(fd,40);
			short second = RFIFOW(fd,42);
			RFIFOSKIP(fd,44);

			safestrncpy(character_name, name, NAME_LENGTH);
			i = search_character_index(character_name);
			if( i < 0 )
			{
				result = 1; // 1-player not found
			}
			else
			{
				char name[NAME_LENGTH];
				int account_id;

				account_id = char_dat[i].status.account_id;
				safestrncpy(name, char_dat[i].status.name, NAME_LENGTH);

				if( login_fd <= 0 )
					result = 3; // 3-login-server offline
				else
				if( acc != -1 && isGM(acc) < isGM(account_id) )
					result = 2; // 2-gm level too low
				else
				switch( type ) {
				case 1: // block
						WFIFOHEAD(login_fd,10);
						WFIFOW(login_fd,0) = 0x2724;
						WFIFOL(login_fd,2) = account_id;
						WFIFOL(login_fd,6) = 5; // new account status
						WFIFOSET(login_fd,10);
				break;
				case 2: // ban
						WFIFOHEAD(login_fd,18);
						WFIFOW(login_fd, 0) = 0x2725;
						WFIFOL(login_fd, 2) = account_id;
						WFIFOW(login_fd, 6) = year;
						WFIFOW(login_fd, 8) = month;
						WFIFOW(login_fd,10) = day;
						WFIFOW(login_fd,12) = hour;
						WFIFOW(login_fd,14) = minute;
						WFIFOW(login_fd,16) = second;
						WFIFOSET(login_fd,18);
				break;
				case 3: // unblock
						WFIFOHEAD(login_fd,10);
						WFIFOW(login_fd,0) = 0x2724;
						WFIFOL(login_fd,2) = account_id;
						WFIFOL(login_fd,6) = 0; // new account status
						WFIFOSET(login_fd,10);
				break;
				case 4: // unban
						WFIFOHEAD(login_fd,6);
						WFIFOW(login_fd,0) = 0x272a;
						WFIFOL(login_fd,2) = account_id;
						WFIFOSET(login_fd,6);
				break;
				case 5: // changesex
						WFIFOHEAD(login_fd,6);
						WFIFOW(login_fd,0) = 0x2727;
						WFIFOL(login_fd,2) = account_id;
						WFIFOSET(login_fd,6);
				break;
				}
			}

			// send answer if a player ask, not if the server ask
			if( acc != -1 ) {
				WFIFOHEAD(fd,34);
				WFIFOW(fd, 0) = 0x2b0f;
				WFIFOL(fd, 2) = acc;
				safestrncpy((char*)WFIFOP(fd,6), name, NAME_LENGTH);
				WFIFOW(fd,30) = type;
				WFIFOW(fd,32) = result;
				WFIFOSET(fd,34);
			}
		}
		break;

		case 0x2b10: // Update and send fame ranking list
			if (RFIFOREST(fd) < 11)
				return 0;
		{
			int cid = RFIFOL(fd, 2);
			int fame = RFIFOL(fd, 6);
			char type = RFIFOB(fd, 10);
			int size;
			struct fame_list* list;
			int player_pos;
			int fame_pos;

			switch(type)
			{
				case 1:  size = fame_list_size_smith;   list = smith_fame_list;   break;
				case 2:  size = fame_list_size_chemist; list = chemist_fame_list; break;
				case 3:  size = fame_list_size_taekwon; list = taekwon_fame_list; break;
				default: size = 0;                      list = NULL;              break;
			}

			ARR_FIND(0, size, player_pos, list[player_pos].id == cid);// position of the player
			ARR_FIND(0, size, fame_pos, list[fame_pos].fame <= fame);// where the player should be

			if( player_pos == size && fame_pos == size )
				;// not on list and not enough fame to get on it
			else if( fame_pos == player_pos )
			{// same position
				list[player_pos].fame = fame;
				char_update_fame_list(type, player_pos, fame);
			}
			else
			{// move in the list
				if( player_pos == size )
				{// new ranker - not in the list
					ARR_MOVE(size - 1, fame_pos, list, struct fame_list);
					list[fame_pos].id = cid;
					list[fame_pos].fame = fame;
					char_loadName(cid, list[fame_pos].name);
				}
				else
				{// already in the list
					if( fame_pos == size )
						--fame_pos;// move to the end of the list
					ARR_MOVE(player_pos, fame_pos, list, struct fame_list);
					list[fame_pos].fame = fame;
				}
				char_send_fame_list(-1);
			}

			RFIFOSKIP(fd,11);
		}
		break;

		case 0x2b16: // Receive rates [Wizputer]
			if (RFIFOREST(fd) < 6 || RFIFOREST(fd) < RFIFOW(fd,8))
				return 0;
			// Txt doesn't need this packet, so just skip it
			RFIFOSKIP(fd,RFIFOW(fd,8));
		break;

		case 0x2b17: // Character disconnected set online 0 [Wizputer]
			if (RFIFOREST(fd) < 6)
				return 0;
			set_char_offline(RFIFOL(fd,2),RFIFOL(fd,6));
			RFIFOSKIP(fd,10);
		break;

		case 0x2b18: // Reset all chars to offline [Wizputer]
			set_all_offline(id);
			RFIFOSKIP(fd,2);
		break;
		
		case 0x2b19: // Character set online [Wizputer]
			if (RFIFOREST(fd) < 10)
				return 0;
			set_char_online(id, RFIFOL(fd,2),RFIFOL(fd,6));
			RFIFOSKIP(fd,10);
		break;

		case 0x2b1a: // Build and send fame ranking lists [DracoRPG]
			if (RFIFOREST(fd) < 2)
				return 0;
			char_read_fame_list();
			char_send_fame_list(-1);
			RFIFOSKIP(fd,2);
		break;

		case 0x2b1c: //Request to save status change data. [Skotlex]
			if (RFIFOREST(fd) < 4 || RFIFOREST(fd) < RFIFOW(fd,2))
				return 0;
		{
#ifdef ENABLE_SC_SAVING
			int count, aid, cid;
			struct scdata *data;

			aid = RFIFOL(fd, 4);
			cid = RFIFOL(fd, 8);
			count = RFIFOW(fd, 12);

			data = status_search_scdata(aid, cid);
			if (data->count != count)
			{
				data->count = count;
				data->data = (struct status_change_data*)aRealloc(data->data, count*sizeof(struct status_change_data));
			}
			for (i = 0; i < count; i++)
				memcpy (&data->data[i], RFIFOP(fd, 14+i*sizeof(struct status_change_data)), sizeof(struct status_change_data));
#endif
			RFIFOSKIP(fd, RFIFOW(fd, 2));
		}
		break;

		case 0x2b23: // map-server alive packet
			WFIFOHEAD(fd,2);
			WFIFOW(fd,0) = 0x2b24;
			WFIFOSET(fd,2);
			RFIFOSKIP(fd,2);
		break;

		case 0x2736: // ip address update
			if (RFIFOREST(fd) < 6) return 0;
			server[id].ip = ntohl(RFIFOL(fd, 2));
			ShowInfo("Updated IP address of map-server #%d to %d.%d.%d.%d.\n", id, CONVIP(server[id].ip));
			RFIFOSKIP(fd,6);
		break;

		default:
		{
			// inter server - packet
			int r = inter_parse_frommap(fd);
			if (r == 1) break;		// processed
			if (r == 2) return 0;	// need more packet

			// no inter server packet. no char server packet -> disconnect
			ShowError("Unknown packet 0x%04x from map server, disconnecting.\n", RFIFOW(fd,0));
			set_eof(fd);
			return 0;
		}
		} // switch
	} // while
	
	return 0;
}

// Searches for the mapserver that has a given map (and optionally ip/port, if not -1).
// If found, returns the server's index in the 'server' array (otherwise returns -1).
int search_mapserver(unsigned short map, uint32 ip, uint16 port)
{
	int i, j;
	
	for(i = 0; i < MAX_MAP_SERVERS; i++)
	{
		if (server[i].fd > 0
		&& (ip == (uint32)-1 || server[i].ip == ip)
		&& (port == (uint16)-1 || server[i].port == port))
		{
			for (j = 0; server[i].map[j]; j++)
				if (server[i].map[j] == map)
					return i;
		}
	}

	return -1;
}

// char_mapifの初期化処理（現在はinter_mapif初期化のみ）
static int char_mapif_init(int fd)
{
	return inter_mapif_init(fd);
}

//--------------------------------------------
// Test to know if an IP come from LAN or WAN.
//--------------------------------------------
int lan_subnetcheck(uint32 ip)
{
	int i;
	ARR_FIND( 0, subnet_count, i, (subnet[i].char_ip & subnet[i].mask) == (ip & subnet[i].mask) );
	if ( i < subnet_count ) {
		ShowInfo("Subnet check [%u.%u.%u.%u]: Matches "CL_CYAN"%u.%u.%u.%u/%u.%u.%u.%u"CL_RESET"\n", CONVIP(ip), CONVIP(subnet[i].char_ip & subnet[i].mask), CONVIP(subnet[i].mask));
		return subnet[i].char_ip;
	} else {
		ShowInfo("Subnet check [%u.%u.%u.%u]: "CL_CYAN"WAN"CL_RESET"\n", CONVIP(ip));
		return 0;
	}
}

int parse_char(int fd)
{
	int i, ch;
	char email[40];
	unsigned short cmd;
	int map_fd;
	struct char_session_data* sd;
	uint32 ipl = session[fd]->client_addr;
	
	sd = (struct char_session_data*)session[fd]->session_data;

	// disconnect any player if no login-server.
	if(login_fd < 0)
		set_eof(fd);

	if(session[fd]->flag.eof)
	{
		if (sd != NULL)
		{
			struct online_char_data* data = (struct online_char_data*)idb_get(online_char_db, sd->account_id);
			if (!data || data->server == -1) //If it is not in any server, send it offline. [Skotlex]
				set_char_offline(99,sd->account_id);
			if (data && data->fd == fd)
				data->fd = -1;
		}
		do_close(fd);
		return 0;
	}

	while( RFIFOREST(fd) >= 2 )
	{
		//For use in packets that depend on an sd being present [Skotlex]
		#define FIFOSD_CHECK(rest) { if(RFIFOREST(fd) < rest) return 0; if (sd==NULL || !sd->auth) { RFIFOSKIP(fd,rest); return 0; } }

		cmd = RFIFOW(fd,0);
		switch( cmd )
		{

		// request to connect
		// 0065 <account id>.L <login id1>.L <login id2>.L <???>.W <sex>.B
		case 0x65:
			if (RFIFOREST(fd) < 17)
				return 0;
		{
			int account_id = RFIFOL(fd,2);
			int login_id1 = RFIFOL(fd,6);
			int login_id2 = RFIFOL(fd,10);
			int sex = RFIFOB(fd,16);

			int GM_value;
			ShowInfo("request connect - account_id:%d/login_id1:%d/login_id2:%d\n", account_id, login_id1, login_id2);

			if (sd) {
				//Received again auth packet for already authentified account?? Discard it.
				//TODO: Perhaps log this as a hack attempt?
				//TODO: and perhaps send back a reply?
				RFIFOSKIP(fd,17);
				break;
			}
			if( (GM_value = isGM(account_id)) != 0 )
				ShowInfo("Account Logged On; Account ID: %d (GM level %d).\n", account_id, GM_value);
			else
				ShowInfo("Account Logged On; Account ID: %d.\n", account_id);
			
			CREATE(session[fd]->session_data, struct char_session_data, 1);
			sd = (struct char_session_data*)session[fd]->session_data;
			strncpy(sd->email, "no mail", 40); // put here a mail without '@' to refuse deletion if we don't receive the e-mail
			sd->connect_until_time = 0; // unknown or unlimited (not displaying on map-server)
			sd->account_id = account_id;
			sd->login_id1 = login_id1;
			sd->login_id2 = login_id2;
			sd->sex = sex;
			sd->auth = false; // not authed yet

			// send back account_id
			WFIFOHEAD(fd,4);
			WFIFOL(fd,0) = account_id;
			WFIFOSET(fd,4);

			// search authentification
			ARR_FIND( 0, AUTH_FIFO_SIZE, i,
				auth_fifo[i].account_id == sd->account_id &&
				auth_fifo[i].login_id1 == sd->login_id1 &&
				auth_fifo[i].login_id2 == sd->login_id2 &&
				auth_fifo[i].ip == session[fd]->client_addr &&
				auth_fifo[i].delflag == 2 );

			if( i < AUTH_FIFO_SIZE ) {
				auth_fifo[i].delflag = 1;
				char_auth_ok(fd, sd);
			} else { // authentication not found
				if (login_fd > 0) { // don't send request if no login-server
					WFIFOHEAD(login_fd,19);
					WFIFOW(login_fd,0) = 0x2712; // ask login-server to authentify an account
					WFIFOL(login_fd,2) = sd->account_id;
					WFIFOL(login_fd,6) = sd->login_id1;
					WFIFOL(login_fd,10) = sd->login_id2;
					WFIFOB(login_fd,14) = sd->sex;
					WFIFOL(login_fd,15) = htonl(session[fd]->client_addr);
					WFIFOSET(login_fd,19);
				} else { // if no login-server, we must refuse connection
					WFIFOHEAD(fd,3);
					WFIFOW(fd,0) = 0x6c;
					WFIFOB(fd,2) = 0;
					WFIFOSET(fd,3);
				}
			}
		}

			RFIFOSKIP(fd,17);
		break;

		// char select
		case 0x66:
			FIFOSD_CHECK(3);
		{
			int slot = RFIFOB(fd,2);
			struct mmo_charstatus *cd;

			RFIFOSKIP(fd,3);

			// if we activated email creation and email is default email
			if (email_creation != 0 && strcmp(sd->email, "a@a.com") == 0 && login_fd > 0) { // to modify an e-mail, login-server must be online
				WFIFOHEAD(fd,3);
				WFIFOW(fd,0) = 0x70;
				WFIFOB(fd,2) = 0; // 00 = Incorrect Email address
				WFIFOSET(fd,3);
				break;
			}
			// otherwise, load the character
			ARR_FIND( 0, MAX_CHARS, ch, sd->found_char[ch] >= 0 && char_dat[sd->found_char[ch]].status.slot == slot );
			if (ch == MAX_CHARS)
			{	//Not found?? May be forged packet.
				break;
			}
			cd = &char_dat[sd->found_char[ch]].status;
			char_log("Character Selected, Account ID: %d, Character Slot: %d, Character Name: %s.\n", sd->account_id, slot, cd->name);

			cd->sex = sd->sex;
			
			// searching map server
			i = search_mapserver(cd->last_point.map,-1,-1);

			// if map is not found, we check major cities
			if (i < 0) {
				unsigned short j;
				//First check that there's actually a map server online.
				ARR_FIND( 0, MAX_MAP_SERVERS, j, server[j].fd >= 0 && server[j].map[0] );
				if (j == MAX_MAP_SERVERS) {
					ShowInfo("Connection Closed. No map servers available.\n");
					WFIFOHEAD(fd,3);
					WFIFOW(fd,0) = 0x81;
					WFIFOB(fd,2) = 1; // 01 = Server closed
					WFIFOSET(fd,3);
					break;
				}
				if ((i = search_mapserver((j=mapindex_name2id(MAP_PRONTERA)),-1,-1)) >= 0) {
					cd->last_point.x = 273;
					cd->last_point.y = 354;
				} else if ((i = search_mapserver((j=mapindex_name2id(MAP_GEFFEN)),-1,-1)) >= 0) {
					cd->last_point.x = 120;
					cd->last_point.y = 100;
				} else if ((i = search_mapserver((j=mapindex_name2id(MAP_MORROC)),-1,-1)) >= 0) {
					cd->last_point.x = 160;
					cd->last_point.y = 94;
				} else if ((i = search_mapserver((j=mapindex_name2id(MAP_ALBERTA)),-1,-1)) >= 0) {
					cd->last_point.x = 116;
					cd->last_point.y = 57;
				} else if ((i = search_mapserver((j=mapindex_name2id(MAP_PAYON)),-1,-1)) >= 0) {
					cd->last_point.x = 87;
					cd->last_point.y = 117;
				} else if ((i = search_mapserver((j=mapindex_name2id(MAP_IZLUDE)),-1,-1)) >= 0) {
					cd->last_point.x = 94;
					cd->last_point.y = 103;
				} else {
					ShowInfo("Connection Closed. No map server available that has a major city, and unable to find map-server for '%s'.\n", mapindex_id2name(cd->last_point.map));
					WFIFOHEAD(fd,3);
					WFIFOW(fd,0) = 0x81;
					WFIFOB(fd,2) = 1; // 01 = Server closed
					WFIFOSET(fd,3);
					break;
				}
				ShowWarning("Unable to find map-server for '%s', sending to major city '%s'.\n", mapindex_id2name(cd->last_point.map), mapindex_id2name(j));
				cd->last_point.map = j;
			}

			//Send player to map
			WFIFOHEAD(fd,28);
			WFIFOW(fd,0) = 0x71;
			WFIFOL(fd,2) = cd->char_id;
			mapindex_getmapname_ext(mapindex_id2name(cd->last_point.map), (char*)WFIFOP(fd,6));
		{
			// Advanced subnet check [LuzZza]
			uint32 subnet_map_ip;
			subnet_map_ip = lan_subnetcheck(ipl);
			WFIFOL(fd,22) = htonl((subnet_map_ip) ? subnet_map_ip : server[i].ip);
			WFIFOW(fd,26) = ntows(htons(server[i].port)); // [!] LE byte order here [!]
			WFIFOSET(fd,28);
		}
			ShowInfo("Character selection '%s' (account: %d, slot: %d).\n", cd->name, sd->account_id, ch);

			if (auth_fifo_pos >= AUTH_FIFO_SIZE)
				auth_fifo_pos = 0;
			auth_fifo[auth_fifo_pos].account_id = sd->account_id;
			auth_fifo[auth_fifo_pos].char_id = cd->char_id;
			auth_fifo[auth_fifo_pos].login_id1 = sd->login_id1;
			auth_fifo[auth_fifo_pos].login_id2 = sd->login_id2;
			auth_fifo[auth_fifo_pos].delflag = 0;
			auth_fifo[auth_fifo_pos].sex = sd->sex;
			auth_fifo[auth_fifo_pos].connect_until_time = sd->connect_until_time;
			auth_fifo[auth_fifo_pos].ip = session[fd]->client_addr;

			//Send NEW auth packet [Kevin]
			if ((map_fd = server[i].fd) < 1 || session[map_fd] == NULL)
			{
				ShowError("parse_char: Attempting to write to invalid session %d! Map Server #%d disconnected.\n", map_fd, i);
				server[i].fd = -1;
				memset(&server[i], 0, sizeof(struct mmo_map_server));
				//Send server closed.
				WFIFOHEAD(fd,3);
				WFIFOW(fd,0) = 0x81;
				WFIFOB(fd,2) = 1; // 01 = Server closed
				WFIFOSET(fd,3);
				break;
			}

			//Send auth to server.
			WFIFOHEAD(map_fd, 20 + sizeof(struct mmo_charstatus));
			WFIFOW(map_fd,0) = 0x2afd;
			WFIFOW(map_fd,2) = 20 + sizeof(struct mmo_charstatus);
			WFIFOL(map_fd,4) = auth_fifo[auth_fifo_pos].account_id;
			WFIFOL(map_fd,8) = auth_fifo[auth_fifo_pos].login_id1;
			WFIFOL(map_fd,16) = auth_fifo[auth_fifo_pos].login_id2;
			WFIFOL(map_fd,12) = (unsigned long)auth_fifo[auth_fifo_pos].connect_until_time;
			memcpy(WFIFOP(map_fd,20), cd, sizeof(struct mmo_charstatus));
			WFIFOSET(map_fd, WFIFOW(map_fd,2));

			set_char_online(i, cd->char_id, cd->account_id);
			auth_fifo_pos++;
		}
		break;

		// create new char
		// S 0067 <name>.24B <str>.B <agi>.B <vit>.B <int>.B <dex>.B <luk>.B <slot>.B <hair color>.W <hair style>.W
		case 0x67:
			FIFOSD_CHECK(37);

			if( !char_new ) //turn character creation on/off [Kevin]
				i = -2;
			else
				i = make_new_char(sd, (char*)RFIFOP(fd,2),RFIFOB(fd,26),RFIFOB(fd,27),RFIFOB(fd,28),RFIFOB(fd,29),RFIFOB(fd,30),RFIFOB(fd,31),RFIFOB(fd,32),RFIFOW(fd,33),RFIFOW(fd,35));

			//'Charname already exists' (-1), 'Char creation denied' (-2) and 'You are underaged' (-3)
			if (i < 0)
			{
				WFIFOHEAD(fd,3);
				WFIFOW(fd,0) = 0x6e;
				switch (i) {
				case -1: WFIFOB(fd,2) = 0x00; break;
				case -2: WFIFOB(fd,2) = 0x02; break;
				case -3: WFIFOB(fd,2) = 0x01; break;
				}
				WFIFOSET(fd,3);
			}
			else
			{
				int len;
				// send to player
				WFIFOHEAD(fd,110);
				WFIFOW(fd,0) = 0x6d;
				len = 2 + mmo_char_tobuf(WFIFOP(fd,2), &char_dat[i].status);
				WFIFOSET(fd,len);

				// add new entry to the chars list
				ARR_FIND( 0, MAX_CHARS, ch, sd->found_char[ch] == -1 );
				if( ch < MAX_CHARS )
						sd->found_char[ch] = i; // position of the new char in the char_dat[] array
			}

			RFIFOSKIP(fd,37);
		break;

		// delete char
		case 0x68:
		// 2004-04-19aSakexe+ langtype 12 char deletion packet
		case 0x1fb:
			if (cmd == 0x68) FIFOSD_CHECK(46);
			if (cmd == 0x1fb) FIFOSD_CHECK(56);
		{
			int cid = RFIFOL(fd,2);
			struct mmo_charstatus* cs = NULL;
			ShowInfo(CL_RED"Request Char Deletion: "CL_GREEN"%d (%d)"CL_RESET"\n", sd->account_id, cid);
			memcpy(email, RFIFOP(fd,6), 40);
			RFIFOSKIP(fd,RFIFOREST(fd)); // hack to make the other deletion packet work

			if (e_mail_check(email) == 0)
				strncpy(email, "a@a.com", 40); // default e-mail

			// BEGIN HACK: "change email using the char deletion 'confirm email' menu"
			// if we activated email creation and email is default email
			if (email_creation != 0 && strcmp(sd->email, "a@a.com") == 0 && login_fd > 0) { // to modify an e-mail, login-server must be online
				// if sended email is incorrect e-mail
				if (strcmp(email, "a@a.com") == 0) {
					WFIFOHEAD(fd,3);
					WFIFOW(fd,0) = 0x70;
					WFIFOB(fd,2) = 0; // 00 = Incorrect Email address
					WFIFOSET(fd,3);
					break;
				}
				// we change the packet to set it like selection.
				ARR_FIND( 0, MAX_CHARS, i, sd->found_char[i] != -1 && char_dat[sd->found_char[i]].status.char_id == cid );
				if( i < MAX_CHARS )
				{
					// we save new e-mail
					memcpy(sd->email, email, 40);
					// we send new e-mail to login-server ('online' login-server is checked before)
					WFIFOHEAD(login_fd,46);
					WFIFOW(login_fd,0) = 0x2715;
					WFIFOL(login_fd,2) = sd->account_id;
					memcpy(WFIFOP(login_fd, 6), email, 40);
					WFIFOSET(login_fd,46);

					// change value to put new packet (char selection)
					RFIFOSKIP(fd,-3); //FIXME: Will this work? Messing with the received buffer is ugly anyway... 
					RFIFOW(fd,0) = 0x66;
					RFIFOB(fd,2) = char_dat[sd->found_char[i]].status.slot;
					// not send packet, it's modify of actual packet
				} else {
					WFIFOHEAD(fd,3);
					WFIFOW(fd,0) = 0x70;
					WFIFOB(fd,2) = 0; // 00 = Incorrect Email address
					WFIFOSET(fd,3);
				}
				break;
			}
			// END HACK

			// otherwise, we delete the character
			if (strcmpi(email, sd->email) != 0) { // if it's an invalid email
				WFIFOHEAD(fd,3);
				WFIFOW(fd,0) = 0x70;
				WFIFOB(fd,2) = 0; // 00 = Incorrect Email address
				WFIFOSET(fd,3);
				break;
			}

			// check if this char exists
			ARR_FIND( 0, MAX_CHARS, i, sd->found_char[i] != -1 && char_dat[sd->found_char[i]].status.char_id == cid );
			if( i == MAX_CHARS )
			{ // Such a character does not exist in the account
				WFIFOHEAD(fd,3);
				WFIFOW(fd,0) = 0x70;
				WFIFOB(fd,2) = 0;
				WFIFOSET(fd,3);
				break;
			}

			// deletion process
			cs = &char_dat[sd->found_char[i]].status;
			char_delete(cs);
			if (sd->found_char[i] != char_num - 1) {
				int j, k;
				struct char_session_data *sd2;
				memcpy(&char_dat[sd->found_char[i]], &char_dat[char_num-1], sizeof(struct mmo_charstatus));
				// Correct moved character reference in the character's owner
				for (j = 0; j < fd_max; j++) {
					if (session[j] && (sd2 = (struct char_session_data*)session[j]->session_data) &&
						sd2->account_id == char_dat[char_num-1].status.account_id) {
						for (k = 0; k < MAX_CHARS; k++) {
							if (sd2->found_char[k] == char_num-1) {
								sd2->found_char[k] = sd->found_char[i];
								break;
							}
						}
						break;
					}
				}
			}
			char_num--;

			// remove char from list and compact it
			for(ch = i; ch < MAX_CHARS-1; ch++)
				sd->found_char[ch] = sd->found_char[ch+1];
			sd->found_char[MAX_CHARS-1] = -1;

			/* Char successfully deleted.*/
			WFIFOHEAD(fd,2);
			WFIFOW(fd,0) = 0x6f;
			WFIFOSET(fd,2);
		}
		break;

		// client keep-alive packet (every 12 seconds)
		// R 0187 <account ID>.l
		case 0x187:
			if (RFIFOREST(fd) < 6)
				return 0;
			RFIFOSKIP(fd,6);
		break;

		// char rename request
		// R 028d <account ID>.l <char ID>.l <new name>.24B
		case 0x28d:
			FIFOSD_CHECK(34);
			//not implemented
			RFIFOSKIP(fd,34);
		break;

		// login as map-server
		case 0x2af8:
			if (RFIFOREST(fd) < 60)
				return 0;
		{
			char* l_user = (char*)RFIFOP(fd,2);
			char* l_pass = (char*)RFIFOP(fd,26);
			l_user[23] = '\0';
			l_pass[23] = '\0';
			ARR_FIND( 0, MAX_MAP_SERVERS, i, server[i].fd <= 0 );
			if (i == MAX_MAP_SERVERS || strcmp(l_user, userid) || strcmp(l_pass, passwd)) {
				WFIFOHEAD(fd,3);
				WFIFOW(fd,0) = 0x2af9;
				WFIFOB(fd,2) = 3;
				WFIFOSET(fd,3);
			} else {
				WFIFOHEAD(fd,3);
				WFIFOW(fd,0) = 0x2af9;
				WFIFOB(fd,2) = 0;
				WFIFOSET(fd,3);

				server[i].fd = fd;
				server[i].ip = ntohl(RFIFOL(fd,54));
				server[i].port = ntohs(RFIFOW(fd,58));
				server[i].users = 0;
				memset(server[i].map, 0, sizeof(server[i].map));
				session[fd]->func_parse = parse_frommap;
				session[fd]->flag.server = 1;
				realloc_fifo(fd, FIFOSIZE_SERVERLINK, FIFOSIZE_SERVERLINK);
				char_mapif_init(fd);
				// send gm acccounts level to map-servers
				WFIFOHEAD(fd,4+5*GM_num);
				WFIFOW(fd,0) = 0x2b15;
				for(i = 0; i < GM_num; i++) {
					WFIFOL(fd,4+5*i) = gm_account[i].account_id;
					WFIFOB(fd,4+5*i+4) = (unsigned char)gm_account[i].level;
				}
				WFIFOW(fd,2) = 4+5*GM_num;
				WFIFOSET(fd,WFIFOW(fd,2));
			}

			RFIFOSKIP(fd,60);
		}
		return 0; // avoid processing of followup packets here

		// Athena info get
		case 0x7530:
			WFIFOHEAD(fd,10);
			WFIFOW(fd,0) = 0x7531;
			WFIFOB(fd,2) = ATHENA_MAJOR_VERSION;
			WFIFOB(fd,3) = ATHENA_MINOR_VERSION;
			WFIFOB(fd,4) = ATHENA_REVISION;
			WFIFOB(fd,5) = ATHENA_RELEASE_FLAG;
			WFIFOB(fd,6) = ATHENA_OFFICIAL_FLAG;
			WFIFOB(fd,7) = ATHENA_SERVER_INTER | ATHENA_SERVER_CHAR;
			WFIFOW(fd,8) = ATHENA_MOD_VERSION;
			WFIFOSET(fd,10);

			RFIFOSKIP(fd,2);
		break;

		// unknown packet received
		default:
			ShowError("parse_char: Received unknown packet "CL_WHITE"0x%x"CL_RESET" from ip '"CL_WHITE"%s"CL_RESET"'! Disconnecting!\n", RFIFOW(fd,0), ip2str(ipl, NULL));
			set_eof(fd);
			return 0;
		}
	}

	RFIFOFLUSH(fd);
	return 0;
}

// Console Command Parser [Wizputer]
int parse_console(char* buf)
{
	char command[256];

	memset(command, 0, sizeof(command));

	sscanf(buf, "%[^\n]", command);

	//login_log("Console command :%s\n", command);

	if( strcmpi("shutdown", command) == 0 ||
	    strcmpi("exit", command) == 0 ||
	    strcmpi("quit", command) == 0 ||
	    strcmpi("end", command) == 0 )
		runflag = 0;
	else if( strcmpi("alive", command) == 0 ||
	         strcmpi("status", command) == 0 )
		ShowInfo(CL_CYAN"Console: "CL_BOLD"I'm Alive."CL_RESET"\n");
	else if( strcmpi("help", command) == 0 ){
		ShowInfo(CL_BOLD"Help of commands:"CL_RESET"\n");
		ShowInfo("  To shutdown the server:\n");
		ShowInfo("  'shutdown|exit|qui|end'\n");
		ShowInfo("  To know if server is alive:\n");
		ShowInfo("  'alive|status'\n");
	}

	return 0;
}

// 全てのMAPサーバーにデータ送信（送信したmap鯖の数を返す）
int mapif_sendall(unsigned char *buf, unsigned int len)
{
	int i, c;

	c = 0;
	for(i = 0; i < MAX_MAP_SERVERS; i++) {
		int fd;
		if ((fd = server[i].fd) > 0) {
			WFIFOHEAD(fd,len);
			memcpy(WFIFOP(fd,0), buf, len);
			WFIFOSET(fd,len);
			c++;
		}
	}

	return c;
}

// 自分以外の全てのMAPサーバーにデータ送信（送信したmap鯖の数を返す）
int mapif_sendallwos(int sfd, unsigned char *buf, unsigned int len)
{
	int i, c;

	c = 0;
	for(i = 0; i < MAX_MAP_SERVERS; i++) {
		int fd;
		if ((fd = server[i].fd) > 0 && fd != sfd) {
			WFIFOHEAD(fd,len);
			memcpy(WFIFOP(fd,0), buf, len);
			WFIFOSET(fd,len);
			c++;
		}
	}

	return c;
}
// MAPサーバーにデータ送信（map鯖生存確認有り）
int mapif_send(int fd, unsigned char *buf, unsigned int len)
{
	int i;

	if (fd >= 0) {
		for(i = 0; i < MAX_MAP_SERVERS; i++) {
			if (fd == server[i].fd) {
				WFIFOHEAD(fd,len);
				memcpy(WFIFOP(fd,0), buf, len);
				WFIFOSET(fd,len);
				return 1;
			}
		}
	}
	return 0;
}

int broadcast_user_count(int tid, unsigned int tick, int id, int data)
{
	uint8 buf[6];
	int users = count_users();

	// only send an update when needed
	static int prev_users = 0;
	if( prev_users == users )
		return 0;
	prev_users = users;

	if( login_fd > 0 && session[login_fd] )
	{
		// send number of user to login server
		WFIFOHEAD(login_fd,6);
		WFIFOW(login_fd,0) = 0x2714;
		WFIFOL(login_fd,2) = users;
		WFIFOSET(login_fd,6);
	}

	// send number of players to all map-servers
	WBUFW(buf,0) = 0x2b00;
	WBUFL(buf,2) = users;
	mapif_sendall(buf,6);

	// refresh online files (txt and html)
	create_online_files();

	return 0;
}

/// load this char's account id into the 'online accounts' packet
static int send_accounts_tologin_sub(DBKey key, void* data, va_list ap)
{
	struct online_char_data* character = (struct online_char_data*)data;
	int* i = va_arg(ap, int*);

	if(character->server > -1)
	{
		WFIFOL(login_fd,8+(*i)*4) = character->account_id;
		(*i)++;
		return 1;
	}
	return 0;
}

int send_accounts_tologin(int tid, unsigned int tick, int id, int data)
{
	if (login_fd > 0 && session[login_fd])
	{
		// send account list to login server
		int users = online_char_db->size(online_char_db);
		int i = 0;

		WFIFOHEAD(login_fd,8+users*4);
		WFIFOW(login_fd,0) = 0x272d;
		online_char_db->foreach(online_char_db, send_accounts_tologin_sub, &i, users);
		WFIFOW(login_fd,2) = 8+ i*4;
		WFIFOL(login_fd,4) = i;
		WFIFOSET(login_fd,WFIFOW(login_fd,2));
	}
	return 0;
}

int check_connect_login_server(int tid, unsigned int tick, int id, int data)
{
	if (login_fd > 0 && session[login_fd] != NULL)
		return 0;

	ShowInfo("Attempt to connect to login-server...\n");
	login_fd = make_connection(login_ip, login_port);
	if (login_fd == -1)
	{	//Try again later. [Skotlex]
		login_fd = 0;
		return 0;
	}
	session[login_fd]->func_parse = parse_fromlogin;
	session[login_fd]->flag.server = 1;
	realloc_fifo(login_fd, FIFOSIZE_SERVERLINK, FIFOSIZE_SERVERLINK);
	
	WFIFOHEAD(login_fd,86);
	WFIFOW(login_fd,0) = 0x2710;
	memcpy(WFIFOP(login_fd,2), userid, 24);
	memcpy(WFIFOP(login_fd,26), passwd, 24);
	WFIFOL(login_fd,50) = 0;
	WFIFOL(login_fd,54) = htonl(char_ip);
	WFIFOL(login_fd,58) = htons(char_port);
	memcpy(WFIFOP(login_fd,60), server_name, 20);
	WFIFOW(login_fd,80) = 0;
	WFIFOW(login_fd,82) = char_maintenance;
	WFIFOW(login_fd,84) = char_new_display; //only display (New) if they want to [Kevin]
	WFIFOSET(login_fd,86);
	
	return 1;
}

// sends a ping packet to login server (will receive pong 0x2718)
int ping_login_server(int tid, unsigned int tick, int id, int data)
{
	if (login_fd > 0 && session[login_fd] != NULL)
	{
		WFIFOHEAD(login_fd,2);
		WFIFOW(login_fd,0) = 0x2719;
		WFIFOSET(login_fd,2);
	}
	return 0;
}

//------------------------------------------------
//Invoked 15 seconds after mapif_disconnectplayer in case the map server doesn't
//replies/disconnect the player we tried to kick. [Skotlex]
//------------------------------------------------
static int chardb_waiting_disconnect(int tid, unsigned int tick, int id, int data)
{
	struct online_char_data* character;
	if ((character = (struct online_char_data*)idb_get(online_char_db, id)) != NULL && character->waiting_disconnect == tid)
	{	//Mark it offline due to timeout.
		character->waiting_disconnect = -1;
		set_char_offline(character->char_id, character->account_id);
	}
	return 0;
}

static int online_data_cleanup_sub(DBKey key, void *data, va_list ap)
{
	struct online_char_data *character= (struct online_char_data*)data;
	if (character->fd != -1)
		return 0; //Character still connected
	if (character->server == -2) //Unknown server.. set them offline
		set_char_offline(character->char_id, character->account_id);
	if (character->server < 0)
		//Free data from players that have not been online for a while.
		db_remove(online_char_db, key);
	return 0;
}

static int online_data_cleanup(int tid, unsigned int tick, int id, int data)
{
	online_char_db->foreach(online_char_db, online_data_cleanup_sub);
	return 0;
}

//----------------------------------
// Reading Lan Support configuration
// Rewrote: Anvanced subnet check [LuzZza]
//----------------------------------
int char_lan_config_read(const char *lancfgName)
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

		if(sscanf(line,"%[^:]: %[^:]:%[^:]:%[^\r\n]", w1, w2, w3, w4) != 4) {
	
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
#endif //TXT_SQL_CONVERT

int char_config_read(const char *cfgName)
{
	char line[1024], w1[1024], w2[1024];
	FILE* fp = fopen(cfgName, "r");

	if (fp == NULL) {
		ShowError("Configuration file not found: %s.\n", cfgName);
		return 1;
	}

	ShowInfo("Reading configuration file %s...\n", cfgName);
	while(fgets(line, sizeof(line), fp))
	{
		if (line[0] == '/' && line[1] == '/')
			continue;

		if (sscanf(line, "%[^:]: %[^\r\n]", w1, w2) != 2)
			continue;

		remove_control_chars(w1);
		remove_control_chars(w2);
		if(strcmpi(w1,"timestamp_format") == 0) {
			strncpy(timestamp_format, w2, 20);
		} else if(strcmpi(w1,"console_silent")==0){
			ShowInfo("Console Silent Setting: %d\n", atoi(w2));
			msg_silent = atoi(w2);
#ifndef TXT_SQL_CONVERT
		} else if(strcmpi(w1,"stdout_with_ansisequence")==0){
			stdout_with_ansisequence = config_switch(w2);
		} else if (strcmpi(w1, "userid") == 0) {
			strncpy(userid, w2, 24);
		} else if (strcmpi(w1, "passwd") == 0) {
			strncpy(passwd, w2, 24);
		} else if (strcmpi(w1, "server_name") == 0) {
			strncpy(server_name, w2, 20);
			server_name[sizeof(server_name) - 1] = '\0';
			ShowStatus("%s server has been initialized\n", w2);
		} else if (strcmpi(w1, "wisp_server_name") == 0) {
			if (strlen(w2) >= 4) {
				memcpy(wisp_server_name, w2, sizeof(wisp_server_name));
				wisp_server_name[sizeof(wisp_server_name) - 1] = '\0';
			}
		} else if (strcmpi(w1, "login_ip") == 0) {
			char ip_str[16];
			login_ip = host2ip(w2);
			if (login_ip) {
				strncpy(login_ip_str, w2, sizeof(login_ip_str));
				ShowStatus("Login server IP address : %s -> %s\n", w2, ip2str(login_ip, ip_str));
			}
		} else if (strcmpi(w1, "login_port") == 0) {
			login_port = atoi(w2);
		} else if (strcmpi(w1, "char_ip") == 0) {
			char ip_str[16];
			char_ip = host2ip(w2);
			if (char_ip){
				strncpy(char_ip_str, w2, sizeof(char_ip_str));
				ShowStatus("Character server IP address : %s -> %s\n", w2, ip2str(char_ip, ip_str));
			}
		} else if (strcmpi(w1, "bind_ip") == 0) {
			char ip_str[16];
			bind_ip = host2ip(w2);
			if (bind_ip) {
				strncpy(bind_ip_str, w2, sizeof(bind_ip_str));
				ShowStatus("Character server binding IP address : %s -> %s\n", w2, ip2str(bind_ip, ip_str));
			}
		} else if (strcmpi(w1, "char_port") == 0) {
			char_port = atoi(w2);
		} else if (strcmpi(w1, "char_maintenance") == 0) {
			char_maintenance = atoi(w2);
		} else if (strcmpi(w1, "char_new") == 0) {
			char_new = (bool)atoi(w2);
		} else if (strcmpi(w1, "char_new_display") == 0) {
			char_new_display = atoi(w2);
		} else if (strcmpi(w1, "email_creation") == 0) {
			email_creation = config_switch(w2);
		} else if (strcmpi(w1, "scdata_txt") == 0) { //By Skotlex
			strcpy(scdata_txt, w2);
#endif
		} else if (strcmpi(w1, "char_txt") == 0) {
			strcpy(char_txt, w2);
		} else if (strcmpi(w1, "friends_txt") == 0) { //By davidsiaw
			strcpy(friends_txt, w2);
		} else if (strcmpi(w1, "hotkeys_txt") == 0) { //By davidsiaw
			strcpy(hotkeys_txt, w2);
#ifndef TXT_SQL_CONVERT
		} else if (strcmpi(w1, "max_connect_user") == 0) {
			max_connect_user = atoi(w2);
			if (max_connect_user < 0)
				max_connect_user = 0; // unlimited online players
		} else if(strcmpi(w1, "gm_allow_level") == 0) {
			gm_allow_level = atoi(w2);
			if(gm_allow_level < 0)
				gm_allow_level = 99;
		} else if (strcmpi(w1, "online_check") == 0) {
			online_check = config_switch(w2);
		} else if (strcmpi(w1, "autosave_time") == 0) {
			autosave_interval = atoi(w2)*1000;
			if (autosave_interval <= 0)
				autosave_interval = DEFAULT_AUTOSAVE_INTERVAL;
		} else if (strcmpi(w1, "save_log") == 0) {
			save_log = config_switch(w2);
		} else if (strcmpi(w1, "start_point") == 0) {
			char map[MAP_NAME_LENGTH_EXT];
			int x, y;
			if (sscanf(w2, "%15[^,],%d,%d", map, &x, &y) < 3)
				continue;
			start_point.map = mapindex_name2id(map);
			if (!start_point.map) {
				ShowError("Specified start_point %s not found in map-index cache.\n", map);
				start_point.map = 0;
			}
			start_point.x = x;
			start_point.y = y;
		} else if (strcmpi(w1, "start_zeny") == 0) {
			start_zeny = atoi(w2);
			if (start_zeny < 0)
				start_zeny = 0;
		} else if (strcmpi(w1, "start_weapon") == 0) {
			start_weapon = atoi(w2);
			if (start_weapon < 0)
				start_weapon = 0;
		} else if (strcmpi(w1, "start_armor") == 0) {
			start_armor = atoi(w2);
			if (start_armor < 0)
				start_armor = 0;
		} else if(strcmpi(w1,"log_char")==0) {		//log char or not [devil]
			log_char = atoi(w2);
		} else if (strcmpi(w1, "unknown_char_name") == 0) {
			strcpy(unknown_char_name, w2);
			unknown_char_name[NAME_LENGTH-1] = '\0';
		} else if (strcmpi(w1, "char_log_filename") == 0) {
			strcpy(char_log_filename, w2);
		} else if (strcmpi(w1, "name_ignoring_case") == 0) {
			name_ignoring_case = (bool)config_switch(w2);
		} else if (strcmpi(w1, "char_name_option") == 0) {
			char_name_option = atoi(w2);
		} else if (strcmpi(w1, "char_name_letters") == 0) {
			strcpy(char_name_letters, w2);
		} else if (strcmpi(w1, "char_rename") == 0) {
			char_rename = config_switch(w2);
// online files options
		} else if (strcmpi(w1, "online_txt_filename") == 0) {
			strcpy(online_txt_filename, w2);
		} else if (strcmpi(w1, "online_html_filename") == 0) {
			strcpy(online_html_filename, w2);
		} else if (strcmpi(w1, "online_sorting_option") == 0) {
			online_sorting_option = atoi(w2);
		} else if (strcmpi(w1, "online_display_option") == 0) {
			online_display_option = atoi(w2);
		} else if (strcmpi(w1, "online_gm_display_min_level") == 0) { // minimum GM level to display 'GM' when we want to display it
			online_gm_display_min_level = atoi(w2);
			if (online_gm_display_min_level < 5) // send online file every 5 seconds to player is enough
				online_gm_display_min_level = 5;
		} else if (strcmpi(w1, "online_refresh_html") == 0) {
			online_refresh_html = atoi(w2);
			if (online_refresh_html < 1)
				online_refresh_html = 1;
		} else if(strcmpi(w1,"db_path")==0) {
			strcpy(db_path,w2);
		} else if (strcmpi(w1, "console") == 0) {
			console = config_switch(w2);
		} else if (strcmpi(w1, "fame_list_alchemist") == 0) {
			fame_list_size_chemist = atoi(w2);
			if (fame_list_size_chemist > MAX_FAME_LIST) {
				ShowWarning("Max fame list size is %d (fame_list_alchemist)\n", MAX_FAME_LIST);
				fame_list_size_chemist = MAX_FAME_LIST;
			}
		} else if (strcmpi(w1, "fame_list_blacksmith") == 0) {
			fame_list_size_smith = atoi(w2);
			if (fame_list_size_smith > MAX_FAME_LIST) {
				ShowWarning("Max fame list size is %d (fame_list_blacksmith)\n", MAX_FAME_LIST);
				fame_list_size_smith = MAX_FAME_LIST;
			}
		} else if (strcmpi(w1, "fame_list_taekwon") == 0) {
			fame_list_size_taekwon = atoi(w2);
			if (fame_list_size_taekwon > MAX_FAME_LIST) {
				ShowWarning("Max fame list size is %d (fame_list_taekwon)\n", MAX_FAME_LIST);
				fame_list_size_taekwon = MAX_FAME_LIST;
			}
		} else if (strcmpi(w1, "guild_exp_rate") == 0) {
			guild_exp_rate = atoi(w2);
#endif //TXT_SQL_CONVERT
		} else if (strcmpi(w1, "import") == 0) {
			char_config_read(w2);
		}
	}
	fclose(fp);

	ShowInfo("Done reading %s.\n", cfgName);
	return 0;
}

#ifndef TXT_SQL_CONVERT
int chardb_final(int key, void* data, va_list va)
{
	aFree(data);
	return 0;
}
void do_final(void)
{
	ShowStatus("Terminating server.\n");

	mmo_char_sync();
	inter_save();
	set_all_offline(-1);
	flush_fifos();
	// write online players files with no player
	online_char_db->clear(online_char_db, NULL); //clean the db...
	create_online_files();
	online_char_db->destroy(online_char_db, NULL); //dispose the db...
	
	if(gm_account) aFree(gm_account);
	if(char_dat) aFree(char_dat);

	if (login_fd > 0)
		do_close(login_fd);
	if (char_fd > 0)
		do_close(char_fd);

#ifdef ENABLE_SC_SAVING
	status_final();
#endif
	inter_final();
	mapindex_final();

	char_log("----End of char-server (normal end with closing of all files).\n");
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
	SERVER_TYPE = ATHENA_SERVER_CHAR;
}

int do_init(int argc, char **argv)
{
	int i;

	for(i = 0; i < MAX_MAP_SERVERS; i++) {
		memset(&server[i], 0, sizeof(struct mmo_map_server));
		server[i].fd = -1;
	}

	//Read map indexes
	mapindex_init();
	start_point.map = mapindex_name2id("new_zone01");

	char_config_read((argc < 2) ? CHAR_CONF_NAME : argv[1]);
	char_lan_config_read((argc > 3) ? argv[3] : LAN_CONF_NAME);

	if (strcmp(userid, "s1")==0 && strcmp(passwd, "p1")==0) {
		ShowError("Using the default user/password s1/p1 is NOT RECOMMENDED.\n");
		ShowNotice("Please edit your save/account.txt file to create a proper inter-server user/password (gender 'S')\n");
		ShowNotice("And then change the user/password to use in conf/char_athena.conf (or conf/import/char_conf.txt)\n");
	}

	ShowInfo("Finished reading the char-server configuration.\n");

	// a newline in the log...
	char_log("");
	// moved behind char_config_read in case we changed the filename [celest]
	char_log("The char-server starting...\n");

	ShowInfo("Initializing char server.\n");
	online_char_db = idb_alloc(DB_OPT_RELEASE_DATA);
	mmo_char_init();
	char_read_fame_list(); //Read fame lists.
#ifdef ENABLE_SC_SAVING
	status_init();
#endif
	inter_init_txt((argc > 2) ? argv[2] : inter_cfgName);	// inter server 初期化
	ShowInfo("char server initialized.\n");

	set_defaultparse(parse_char);

	if ((naddr_ != 0) && (!login_ip || !char_ip))
	{
		char ip_str[16];
		ip2str(addr_[0], ip_str);

		if (naddr_ > 1)
			ShowStatus("Multiple interfaces detected..  using %s as our IP address\n", ip_str);
		else
			ShowStatus("Defaulting to %s as our IP address\n", ip_str);
		if (!login_ip) {
			strcpy(login_ip_str, ip_str);
			login_ip = str2ip(login_ip_str);
		}
		if (!char_ip) {
			strcpy(char_ip_str, ip_str);
			char_ip = str2ip(char_ip_str);
		}
	}

	// establish char-login connection if not present
	add_timer_func_list(check_connect_login_server, "check_connect_login_server");
	add_timer_interval(gettick() + 1000, check_connect_login_server, 0, 0, 10 * 1000);

	// keep the char-login connection alive
	add_timer_func_list(ping_login_server, "ping_login_server");
	add_timer_interval(gettick() + 1000, ping_login_server, 0, 0, ((int)stall_time-2) * 1000);

	// periodically update the overall user count on all mapservers + login server
	add_timer_func_list(broadcast_user_count, "broadcast_user_count");
	add_timer_interval(gettick() + 1000, broadcast_user_count, 0, 0, 5 * 1000);

	// send a list of all online account IDs to login server
	add_timer_func_list(send_accounts_tologin, "send_accounts_tologin");
	add_timer_interval(gettick() + 1000, send_accounts_tologin, 0, 0, 3600 * 1000); //Sync online accounts every hour

	// ???
	add_timer_func_list(chardb_waiting_disconnect, "chardb_waiting_disconnect");

	// ???
	add_timer_func_list(online_data_cleanup, "online_data_cleanup");
	add_timer_interval(gettick() + 1000, online_data_cleanup, 0, 0, 600 * 1000);

	// periodic flush of all saved data to disk
	add_timer_func_list(mmo_char_sync_timer, "mmo_char_sync_timer");
	add_timer_interval(gettick() + 1000, mmo_char_sync_timer, 0, 0, autosave_interval);

	if( console )
	{
		//##TODO invoke a CONSOLE_START plugin event
	}

	char_fd = make_listen_bind(bind_ip, char_port);
	char_log("The char-server is ready (Server is listening on the port %d).\n", char_port);
	ShowStatus("The char-server is "CL_GREEN"ready"CL_RESET" (Server is listening on the port %d).\n\n", char_port);

	return 0;
}

#endif //TXT_SQL_CONVERT
