// $Id: char.c,v 1.3 2004/09/13 16:52:16 Yor Exp $
// original : char2.c 2003/03/14 11:58:35 Rev.1.5

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#include <winsock.h>
typedef long in_addr_t;
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <unistd.h>
#endif

#include <time.h>
#include <signal.h>
#include <fcntl.h>
#include <string.h>
#include <stdarg.h>

#include "../common/strlib.h"
#include "core.h"
#include "socket.h"
#include "timer.h"
#include "mmo.h"
#include "db.h"
#include "version.h"
#include "lock.h"
#include "char.h"
#include "showmsg.h"
#include "buffer.h"

#include "inter.h"
#include "int_pet.h"
#include "int_guild.h"
#include "int_party.h"
#include "int_storage.h"

#ifdef MEMWATCH
#include "memwatch.h"
#endif

struct mmo_map_server server[MAX_MAP_SERVERS];
int server_fd[MAX_MAP_SERVERS];

int login_fd, char_fd;
char userid[24];
char passwd[24];
char server_name[20];
char wisp_server_name[24] = "Server";
int login_ip_set_ = 0;
char login_ip_str[16];
in_addr_t login_ip;
int login_port = 6900;
int char_ip_set_ = 0;
char char_ip_str[16];
int bind_ip_set_ = 0;
char bind_ip_str[16];
in_addr_t char_ip;
int char_port = 6121;
int char_maintenance;
int char_new;
int email_creation = 0; // disabled by default
char char_txt[1024]="save/athena.txt";
char backup_txt[1024]="save/backup.txt"; //By zanetheinsane
char friends_txt[1024]="save/friends.txt"; // davidsiaw
char backup_txt_flag = 0; // The backup_txt file was created because char deletion bug existed. Now it's finish and that take a lot of time to create a second file when there are a lot of characters. => option By [Yor]
char unknown_char_name[1024] = "Unknown";
char char_log_filename[1024] = "log/char.log";
char db_path[1024]="db";
//Added for lan support
char lan_map_ip[128];
int subneti[4];
int subnetmaski[4];
int name_ignoring_case = 0; // Allow or not identical name for characters but with a different case by [Yor]
int char_name_option = 0; // Option to know which letters/symbols are authorised in the name of a character (0: all, 1: only those in char_name_letters, 2: all EXCEPT those in char_name_letters) by [Yor]
char char_name_letters[1024] = ""; // list of letters/symbols authorised (or not) in a character name. by [Yor]

int log_char = 1;	// loggin char or not [devil]
int log_inter = 1;	// loggin inter or not [devil]

struct char_session_data{
	int account_id, login_id1, login_id2, sex;
	int found_char[9];
	char email[40]; // e-mail (default: a@a.com) by [Yor]
	time_t connect_until_time; // # of seconds 1/1/1970 (timestamp): Validity limit of the account (0 = unlimited)
};

//Added for Mugendai's I'm Alive mod
int imalive_on=0;
int imalive_time=60;
//Added by Mugendai for GUI
int flush_on=1;
int flush_time=100;

#define AUTH_FIFO_SIZE 256
struct {
	int account_id, char_id, login_id1, login_id2, ip, char_pos, delflag, sex;
	time_t connect_until_time; // # of seconds 1/1/1970 (timestamp): Validity limit of the account (0 = unlimited)
} auth_fifo[AUTH_FIFO_SIZE];
int auth_fifo_pos = 0;

int check_ip_flag = 1; // It's to check IP of a player between char-server and other servers (part of anti-hacking system)

int char_id_count = 150000;
struct mmo_charstatus *char_dat;
int char_num, char_max;
int max_connect_user = 0;
int gm_allow_level = 99;
int autosave_interval = DEFAULT_AUTOSAVE_INTERVAL;
int start_zeny = 500;
int start_weapon = 1201;
int start_armor = 2301;

// Initial position (it's possible to set it in conf file)
struct point start_point = {"new_1-1.gat", 53, 111};

struct gm_account *gm_account = NULL;
int GM_num = 0;

// online players by [Yor]
char online_txt_filename[1024] = "online.txt";
char online_html_filename[1024] = "online.html";
int online_sorting_option = 0; // sorting option to display online players in online files
int online_display_option = 1; // display options: to know which columns must be displayed
int online_players_max;
int online_refresh_html = 20; // refresh time (in sec) of the html file in the explorer
int online_gm_display_min_level = 20; // minimum GM level to display 'GM' when we want to display it

struct online_chars {
	int char_id;
	int server;
} *online_chars;

time_t update_online; // to update online files when we receiving information from a server (not less than 8 seconds)

int console = 0;

//------------------------------
// Writing function of logs file
//------------------------------
int char_log(char *fmt, ...) {
	if(log_char)
	{
		FILE *logfp;
		va_list ap;
		struct timeval tv;
		char tmpstr[2048];

		va_start(ap, fmt);

		logfp = fopen(char_log_filename, "a");
		if (logfp) {
			if (fmt[0] == '\0') // jump a line if no message
				fprintf(logfp, RETCODE);
			else {
				gettimeofday(&tv, NULL);
				strftime(tmpstr, 24, "%d-%m-%Y %H:%M:%S", localtime((const time_t*)&(tv.tv_sec)));
				sprintf(tmpstr + 19, ".%03d: %s", (int)tv.tv_usec / 1000, fmt);
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
int isGM(int account_id) {
	int i;

	for(i = 0; i < GM_num; i++)
		if (gm_account[i].account_id == account_id)
			return gm_account[i].level;
	return 0;
}

//----------------------------------------------
// Search an character id
//   (return character index or -1 (if not found))
//   If exact character name is not found,
//   the function checks without case sensitive
//   and returns index if only 1 character is found
//   and similar to the searched name.
//----------------------------------------------
int search_character_index(char* character_name) {
	int i, quantity, index;

	quantity = 0;
	index = -1;
	for(i = 0; i < char_num; i++) {
		// Without case sensitive check (increase the number of similar character names found)
		if (stricmp(char_dat[i].name, character_name) == 0) {
			// Strict comparison (if found, we finish the function immediatly with correct value)
			if (strcmp(char_dat[i].name, character_name) == 0)
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
char * search_character_name(int index) {

	if (index >= 0 && index < char_num)
		return char_dat[index].name;

	return unknown_char_name;
}

//-------------------------------------------------
// Set Character online/offline [Wizputer]
//-------------------------------------------------

void set_char_online(int char_id, int account_id) {
	if (login_fd <= 0 || session[login_fd]->eof)
		return;
	WFIFOW(login_fd,0) = 0x272b;
	WFIFOL(login_fd,2) = account_id;
	WFIFOSET(login_fd,6);
}
void set_char_offline(int char_id, int account_id) {
    if (login_fd <= 0 || session[login_fd]->eof)
		return;
	WFIFOW(login_fd,0) = 0x272c;
	WFIFOL(login_fd,2) = account_id;
	WFIFOSET(login_fd,6);
}

/*---------------------------------------------------
  Make a data line for friends list
 --------------------------------------------------*/

int mmo_friends_list_data_str(char *str, struct mmo_charstatus *p) {
	int i;
	char *str_p = str;
	str_p += sprintf(str_p, "%d", p->char_id);

	for (i=0;i<20;i++)
	{
		str_p += sprintf(str_p, ",%d,%s", p->friend_id[i],p->friend_name[i]);
	}
	return 0;
}

//-------------------------------------------------
// Function to create the character line (for save)
//-------------------------------------------------
int mmo_char_tostr(char *str, struct mmo_charstatus *p) {
	int i;
	char *str_p = str;

	// on multi-map server, sometimes it's posssible that last_point become void. (reason???) We check that to not lost character at restart.
	if (p->last_point.map[0] == '\0') {
		memcpy(p->last_point.map, "prontera.gat", 16);
		p->last_point.x = 273;
		p->last_point.y = 354;
	}

	str_p += sprintf(str_p, "%d\t%d,%d\t%s\t%d,%d,%d\t%d,%d,%d\t%d,%d,%d,%d\t%d,%d,%d,%d,%d,%d\t%d,%d"
	  "\t%d,%d,%d\t%d,%d,%d\t%d,%d,%d\t%d,%d,%d,%d,%d"
	  "\t%s,%d,%d\t%s,%d,%d,%d,%d,%d,%d\t",
	  p->char_id, p->account_id, p->char_num, p->name, //
	  p->class_, p->base_level, p->job_level,
	  p->base_exp, p->job_exp, p->zeny,
	  p->hp, p->max_hp, p->sp, p->max_sp,
	  p->str, p->agi, p->vit, p->int_, p->dex, p->luk,
	  p->status_point, p->skill_point,
	  p->option, p->karma, p->manner,	//
	  p->party_id, p->guild_id, p->pet_id,
	  p->hair, p->hair_color, p->clothes_color,
	  p->weapon, p->shield, p->head_top, p->head_mid, p->head_bottom,
	  p->last_point.map, p->last_point.x, p->last_point.y, //
	  p->save_point.map, p->save_point.x, p->save_point.y,
	  p->partner_id,p->father,p->mother,p->child);
	for(i = 0; i < 10; i++)
		if (p->memo_point[i].map[0]) {
			str_p += sprintf(str_p, "%s,%d,%d", p->memo_point[i].map, p->memo_point[i].x, p->memo_point[i].y);
		}
	*(str_p++) = '\t';

	for(i = 0; i < MAX_INVENTORY; i++)
		if (p->inventory[i].nameid) {
			str_p += sprintf(str_p, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d ",
			         p->inventory[i].id, p->inventory[i].nameid, p->inventory[i].amount, p->inventory[i].equip,
			         p->inventory[i].identify, p->inventory[i].refine, p->inventory[i].attribute,
			         p->inventory[i].card[0], p->inventory[i].card[1], p->inventory[i].card[2], p->inventory[i].card[3]);
		}
	*(str_p++) = '\t';

	for(i = 0; i < MAX_CART; i++)
		if (p->cart[i].nameid) {
			str_p += sprintf(str_p, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d ",
			         p->cart[i].id, p->cart[i].nameid, p->cart[i].amount, p->cart[i].equip,
			         p->cart[i].identify, p->cart[i].refine, p->cart[i].attribute,
			         p->cart[i].card[0], p->cart[i].card[1], p->cart[i].card[2], p->cart[i].card[3]);
		}
	*(str_p++) = '\t';

	for(i = 0; i < MAX_SKILL; i++)
		if (p->skill[i].id && p->skill[i].flag != 1) {
			str_p += sprintf(str_p, "%d,%d ", p->skill[i].id, (p->skill[i].flag == 0) ? p->skill[i].lv : p->skill[i].flag-2);
		}
	*(str_p++) = '\t';

	for(i = 0; i < p->global_reg_num; i++)
		if (p->global_reg[i].str[0])
			str_p += sprintf(str_p, "%s,%d ", p->global_reg[i].str, p->global_reg[i].value);
	*(str_p++) = '\t';

	*str_p = '\0';
	return 0;
}

//-------------------------------------------------------------------------
// Function to set the character from the line (at read of characters file)
//-------------------------------------------------------------------------
int mmo_char_fromstr(char *str, struct mmo_charstatus *p) {
	int tmp_int[256];
	int set, next, len, i;

	// initilialise character
	memset(p, '\0', sizeof(struct mmo_charstatus));
	
	// If it's not char structure of version 1363 and after
	if ((set = sscanf(str, "%d\t%d,%d\t%[^\t]\t%d,%d,%d\t%d,%d,%d\t%d,%d,%d,%d\t%d,%d,%d,%d,%d,%d\t%d,%d"
	   "\t%d,%d,%d\t%d,%d,%d\t%d,%d,%d\t%d,%d,%d,%d,%d"
	   "\t%[^,],%d,%d\t%[^,],%d,%d,%d,%d,%d,%d%n",
	   &tmp_int[0], &tmp_int[1], &tmp_int[2], p->name, //
	   &tmp_int[3], &tmp_int[4], &tmp_int[5],
           &tmp_int[6], &tmp_int[7], &tmp_int[8],
           &tmp_int[9], &tmp_int[10], &tmp_int[11], &tmp_int[12],
           &tmp_int[13], &tmp_int[14], &tmp_int[15], &tmp_int[16], &tmp_int[17], &tmp_int[18],
           &tmp_int[19], &tmp_int[20],
           &tmp_int[21], &tmp_int[22], &tmp_int[23], //
           &tmp_int[24], &tmp_int[25], &tmp_int[26],
           &tmp_int[27], &tmp_int[28], &tmp_int[29],
           &tmp_int[30], &tmp_int[31], &tmp_int[32], &tmp_int[33], &tmp_int[34],
           p->last_point.map, &tmp_int[35], &tmp_int[36], //
           p->save_point.map, &tmp_int[37], &tmp_int[38], &tmp_int[39], 
	   &tmp_int[40], &tmp_int[41], &tmp_int[42], &next)) != 46) {
		tmp_int[40] = 0; // father
		tmp_int[41] = 0; // mother
		tmp_int[42] = 0; // child
	// If it's not char structure of version 1008 and before 1363
	if ((set = sscanf(str, "%d\t%d,%d\t%[^\t]\t%d,%d,%d\t%d,%d,%d\t%d,%d,%d,%d\t%d,%d,%d,%d,%d,%d\t%d,%d"
	   "\t%d,%d,%d\t%d,%d,%d\t%d,%d,%d\t%d,%d,%d,%d,%d"
	   "\t%[^,],%d,%d\t%[^,],%d,%d,%d%n",
	   &tmp_int[0], &tmp_int[1], &tmp_int[2], p->name, //
	   &tmp_int[3], &tmp_int[4], &tmp_int[5],
	   &tmp_int[6], &tmp_int[7], &tmp_int[8],
	   &tmp_int[9], &tmp_int[10], &tmp_int[11], &tmp_int[12],
	   &tmp_int[13], &tmp_int[14], &tmp_int[15], &tmp_int[16], &tmp_int[17], &tmp_int[18],
	   &tmp_int[19], &tmp_int[20],
	   &tmp_int[21], &tmp_int[22], &tmp_int[23], //
	   &tmp_int[24], &tmp_int[25], &tmp_int[26],
	   &tmp_int[27], &tmp_int[28], &tmp_int[29],
	   &tmp_int[30], &tmp_int[31], &tmp_int[32], &tmp_int[33], &tmp_int[34],
	   p->last_point.map, &tmp_int[35], &tmp_int[36], //
	   p->save_point.map, &tmp_int[37], &tmp_int[38], &tmp_int[39], &next)) != 43) {
		tmp_int[39] = 0; // partner id
		// If not char structure from version 384 to 1007
		if ((set = sscanf(str, "%d\t%d,%d\t%[^\t]\t%d,%d,%d\t%d,%d,%d\t%d,%d,%d,%d\t%d,%d,%d,%d,%d,%d\t%d,%d"
		   "\t%d,%d,%d\t%d,%d,%d\t%d,%d,%d\t%d,%d,%d,%d,%d"
		   "\t%[^,],%d,%d\t%[^,],%d,%d%n",
		   &tmp_int[0], &tmp_int[1], &tmp_int[2], p->name, //
		   &tmp_int[3], &tmp_int[4], &tmp_int[5],
		   &tmp_int[6], &tmp_int[7], &tmp_int[8],
		   &tmp_int[9], &tmp_int[10], &tmp_int[11], &tmp_int[12],
		   &tmp_int[13], &tmp_int[14], &tmp_int[15], &tmp_int[16], &tmp_int[17], &tmp_int[18],
		   &tmp_int[19], &tmp_int[20],
		   &tmp_int[21], &tmp_int[22], &tmp_int[23], //
		   &tmp_int[24], &tmp_int[25], &tmp_int[26],
		   &tmp_int[27], &tmp_int[28], &tmp_int[29],
		   &tmp_int[30], &tmp_int[31], &tmp_int[32], &tmp_int[33], &tmp_int[34],
		   p->last_point.map, &tmp_int[35], &tmp_int[36], //
		   p->save_point.map, &tmp_int[37], &tmp_int[38], &next)) != 42) {
			// It's char structure of a version before 384
			tmp_int[26] = 0; // pet id
			set = sscanf(str, "%d\t%d,%d\t%[^\t]\t%d,%d,%d\t%d,%d,%d\t%d,%d,%d,%d\t%d,%d,%d,%d,%d,%d\t%d,%d"
			      "\t%d,%d,%d\t%d,%d\t%d,%d,%d\t%d,%d,%d,%d,%d"
			      "\t%[^,],%d,%d\t%[^,],%d,%d%n",
			      &tmp_int[0], &tmp_int[1], &tmp_int[2], p->name, //
			      &tmp_int[3], &tmp_int[4], &tmp_int[5],
			      &tmp_int[6], &tmp_int[7], &tmp_int[8],
			      &tmp_int[9], &tmp_int[10], &tmp_int[11], &tmp_int[12],
			      &tmp_int[13], &tmp_int[14], &tmp_int[15], &tmp_int[16], &tmp_int[17], &tmp_int[18],
			      &tmp_int[19], &tmp_int[20],
			      &tmp_int[21], &tmp_int[22], &tmp_int[23], //
			      &tmp_int[24], &tmp_int[25], //
			      &tmp_int[27], &tmp_int[28], &tmp_int[29],
			      &tmp_int[30], &tmp_int[31], &tmp_int[32], &tmp_int[33], &tmp_int[34],
			      p->last_point.map, &tmp_int[35], &tmp_int[36], //
			      p->save_point.map, &tmp_int[37], &tmp_int[38], &next);
			set += 2;
			//printf("char: old char data ver.1\n");
		// Char structure of version 1007 or older
		} else {
			set++;
			//printf("char: old char data ver.2\n");
		}
	// Char structure of version 1008+
	} else {
		set+=3;
		//printf("char: new char data ver.3\n");
	}
	// Char structture of version 1363+
	} else {
		//printf("char: new char data ver.4\n");
	}
	if (set != 46)
		return 0;

	p->char_id = tmp_int[0];
	p->account_id = tmp_int[1];
	p->char_num = tmp_int[2];
	p->class_ = tmp_int[3];
	p->base_level = tmp_int[4];
	p->job_level = tmp_int[5];
	p->base_exp = tmp_int[6];
	p->job_exp = tmp_int[7];
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
	p->status_point = tmp_int[19];
	p->skill_point = tmp_int[20];
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

	// Some checks
	for(i = 0; i < char_num; i++) {
		if (char_dat[i].char_id == p->char_id) {
			printf("\033[1;31mmmo_auth_init: ******Error: a character has an identical id to another.\n");
			printf("               character id #%d -> new character not readed.\n", p->char_id);
			printf("               Character saved in log file.\033[0m\n");
			return -1;
		} else if (strcmp(char_dat[i].name, p->name) == 0) {
			printf("\033[1;31mmmo_auth_init: ******Error: character name already exists.\n");
			printf("               character name '%s' -> new character not readed.\n", p->name);
			printf("               Character saved in log file.\033[0m\n");
			return -2;
		}
	}

	if (strcmpi(wisp_server_name, p->name) == 0) {
		printf("mmo_auth_init: ******WARNING: character name has wisp server name.\n");
		printf("               Character name '%s' = wisp server name '%s'.\n", p->name, wisp_server_name);
		printf("               Character readed. Suggestion: change the wisp server name.\n");
		char_log("mmo_auth_init: ******WARNING: character name has wisp server name: Character name '%s' = wisp server name '%s'." RETCODE,
		          p->name, wisp_server_name);
	}

	if (str[next] == '\n' || str[next] == '\r')
		return 1;	// êVãKÉfÅ[É^

	next++;

	for(i = 0; str[next] && str[next] != '\t'; i++) {
		if (sscanf(str+next, "%[^,],%d,%d%n", p->memo_point[i].map, &tmp_int[0], &tmp_int[1], &len) != 3)
			return -3;
		p->memo_point[i].x = tmp_int[0];
		p->memo_point[i].y = tmp_int[1];
		next += len;
		if (str[next] == ' ')
			next++;
	}

	next++;

	for(i = 0; str[next] && str[next] != '\t'; i++) {
		if (sscanf(str + next, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d%n",
		    &tmp_int[0], &tmp_int[1], &tmp_int[2], &tmp_int[3],
		    &tmp_int[4], &tmp_int[5], &tmp_int[6],
		    &tmp_int[7], &tmp_int[8], &tmp_int[9], &tmp_int[10], &tmp_int[10], &len) == 12) {
			// do nothing, it's ok
		} else if (sscanf(str + next, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d%n",
		          &tmp_int[0], &tmp_int[1], &tmp_int[2], &tmp_int[3],
		          &tmp_int[4], &tmp_int[5], &tmp_int[6],
		          &tmp_int[7], &tmp_int[8], &tmp_int[9], &tmp_int[10], &len) == 11) {
		} else // invalid structure
			return -4;
		p->inventory[i].id = tmp_int[0];
		p->inventory[i].nameid = tmp_int[1];
		p->inventory[i].amount = tmp_int[2];
		p->inventory[i].equip = tmp_int[3];
		p->inventory[i].identify = tmp_int[4];
		p->inventory[i].refine = tmp_int[5];
		p->inventory[i].attribute = tmp_int[6];
		p->inventory[i].card[0] = tmp_int[7];
		p->inventory[i].card[1] = tmp_int[8];
		p->inventory[i].card[2] = tmp_int[9];
		p->inventory[i].card[3] = tmp_int[10];
		next += len;
		if (str[next] == ' ')
			next++;
	}

	next++;

	for(i = 0; str[next] && str[next] != '\t'; i++) {
		if (sscanf(str + next, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d%n",
		    &tmp_int[0], &tmp_int[1], &tmp_int[2], &tmp_int[3],
		    &tmp_int[4], &tmp_int[5], &tmp_int[6],
		    &tmp_int[7], &tmp_int[8], &tmp_int[9], &tmp_int[10], &tmp_int[10], &len) == 12) {
			// do nothing, it's ok
		} else if (sscanf(str + next, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d%n",
		           &tmp_int[0], &tmp_int[1], &tmp_int[2], &tmp_int[3],
		           &tmp_int[4], &tmp_int[5], &tmp_int[6],
		           &tmp_int[7], &tmp_int[8], &tmp_int[9], &tmp_int[10], &len) == 11) {
		} else // invalid structure
			return -5;
		p->cart[i].id = tmp_int[0];
		p->cart[i].nameid = tmp_int[1];
		p->cart[i].amount = tmp_int[2];
		p->cart[i].equip = tmp_int[3];
		p->cart[i].identify = tmp_int[4];
		p->cart[i].refine = tmp_int[5];
		p->cart[i].attribute = tmp_int[6];
		p->cart[i].card[0] = tmp_int[7];
		p->cart[i].card[1] = tmp_int[8];
		p->cart[i].card[2] = tmp_int[9];
		p->cart[i].card[3] = tmp_int[10];
		next += len;
		if (str[next] == ' ')
			next++;
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

	for(i = 0; str[next] && str[next] != '\t' && str[next] != '\n' && str[next] != '\r'; i++) { // global_regé¿ëïà»ëOÇÃathena.txtå›ä∑ÇÃÇΩÇﬂàÍâû'\n'É`ÉFÉbÉN
		if (sscanf(str + next, "%[^,],%d%n", p->global_reg[i].str, &p->global_reg[i].value, &len) != 2) {
			// because some scripts are not correct, the str can be "". So, we must check that.
			// If it's, we must not refuse the character, but just this REG value.
			// Character line will have something like: nov_2nd_cos,9 ,9 nov_1_2_cos_c,1 (here, ,9 is not good)
			if (str[next] == ',' && sscanf(str + next, ",%d%n", &p->global_reg[i].value, &len) == 1)
				i--;
			else
				return -7;
		}
		next += len;
		if (str[next] == ' ')
			next++;
	}
	p->global_reg_num = i;

	return 1;
}
//---------------------------------
// Function to read friend list
//---------------------------------

int parse_friend_txt(struct mmo_charstatus *p)
{
	char line[1024];
	int i,cid=0,temp[20];
	FILE *fp;

	// Open the file and look for the ID
	fp = fopen(friends_txt, "r");

	if(fp == NULL)
		return 1;


	while(fgets(line, sizeof(line)-1, fp)) {

		if(line[0] == '/' && line[1] == '/')
			continue;

		sscanf(line, "%d,%d,%[^,],%d,%[^,],%d,%[^,],%d,%[^,],%d,%[^,],%d,%[^,],%d,%[^,],%d,%[^,],%d,%[^,],%d,%[^,],%d,%[^,],%d,%[^,],%d,%[^,],%d,%[^,],%d,%[^,],%d,%[^,],%d,%[^,],%d,%[^,],%d,%[^,],%d,%s",&cid,
		&temp[0],p->friend_name[0],
		&temp[1],p->friend_name[1],
		&temp[2],p->friend_name[2],
		&temp[3],p->friend_name[3],
		&temp[4],p->friend_name[4],
		&temp[5],p->friend_name[5],
		&temp[6],p->friend_name[6],
		&temp[7],p->friend_name[7],
		&temp[8],p->friend_name[8],
		&temp[9],p->friend_name[9],
		&temp[10],p->friend_name[10],
		&temp[11],p->friend_name[11],
		&temp[12],p->friend_name[12],
		&temp[13],p->friend_name[13],
		&temp[14],p->friend_name[14],
		&temp[15],p->friend_name[15],
		&temp[16],p->friend_name[16],
		&temp[17],p->friend_name[17],
		&temp[18],p->friend_name[18],
		&temp[19],p->friend_name[19]);
		if (cid == p->char_id)
			break;
	}

	// No register of friends list
	if (cid == 0) {
		fclose(fp);
		return 0;
	}

	// Fill in the list

	for (i=0; i<20; i++)
		p->friend_id[i] = temp[i];

	fclose(fp);
	return 0;
}

//---------------------------------
// Function to read characters file
//---------------------------------
int mmo_char_init(void) {
	char line[65536];
	int i;
	int ret, line_count;
	FILE *fp;

	char_max = 256;
	char_dat = (struct mmo_charstatus*)aCalloc(sizeof(struct mmo_charstatus) * 256, 1);
	if (!char_dat) {
		printf("out of memory: mmo_char_init (calloc of char_dat).\n");
		exit(1);
	}
	for(i = 0; i < char_max; i++)
	{
		online_chars[i].char_id = -1;
		online_chars[i].server = -1;
	}

	char_num = 0;

	fp = fopen(char_txt, "r");

	if (fp == NULL) {
		printf("Characters file not found: %s.\n", char_txt);
		char_log("Characters file not found: %s." RETCODE, char_txt);
		char_log("Id for the next created character: %d." RETCODE, char_id_count);
		return 0;
	}

	line_count = 0;
	while(fgets(line, sizeof(line)-1, fp)) {
		int i, j;
		line_count++;

		if (line[0] == '/' && line[1] == '/')
			continue;
		line[sizeof(line)-1] = '\0';

		j = 0;
		if (sscanf(line, "%d\t%%newid%%%n", &i, &j) == 1 && j > 0) {
			if (char_id_count < i)
				char_id_count = i;
			continue;
		}

		if (char_num >= char_max) {
			char_max += 256;
			char_dat = (struct mmo_charstatus*)aRealloc(char_dat, sizeof(struct mmo_charstatus) * char_max);
			if (!char_dat) {
				printf("Out of memory: mmo_char_init (realloc of char_dat).\n");
				char_log("Out of memory: mmo_char_init (realloc of char_dat)." RETCODE);
				exit(1);
			}
			online_chars = (struct online_chars*)aRealloc(online_chars, sizeof(struct online_chars) * char_max);
			if (!online_chars) {
				printf("Out of memory: mmo_char_init (realloc of online_chars).\n");
				char_log("Out of memory: mmo_char_init (realloc of online_chars)." RETCODE);
				exit(1);
			}
			for(i = char_max - 256; i < char_max; i++)
			{
				online_chars[i].char_id = -1;
				online_chars[i].server = -1;
			}
		}

		ret = mmo_char_fromstr(line, &char_dat[char_num]);

		// Initialize friends list
		parse_friend_txt(&char_dat[char_num]);  // Grab friends for the character

		if (ret > 0) { // negative value or zero for errors
			if (char_dat[char_num].char_id >= char_id_count)
				char_id_count = char_dat[char_num].char_id + 1;
			char_num++;
		} else {
			printf("mmo_char_init: in characters file, unable to read the line #%d.\n", line_count);
			printf("               -> Character saved in log file.\n");
			switch (ret) {
			case -1:
				char_log("Duplicate character id in the next character line (character not readed):" RETCODE);
				break;
			case -2:
				char_log("Duplicate character name in the next character line (character not readed):" RETCODE);
				break;
			case -3:
				char_log("Invalid memo point structure in the next character line (character not readed):" RETCODE);
				break;
			case -4:
				char_log("Invalid inventory item structure in the next character line (character not readed):" RETCODE);
				break;
			case -5:
				char_log("Invalid cart item structure in the next character line (character not readed):" RETCODE);
				break;
			case -6:
				char_log("Invalid skill structure in the next character line (character not readed):" RETCODE);
				break;
			case -7:
				char_log("Invalid register structure in the next character line (character not readed):" RETCODE);
				break;
			default: // 0
				char_log("Unabled to get a character in the next line - Basic structure of line (before inventory) is incorrect (character not readed):" RETCODE);
				break;
			}
			char_log("%s", line);
		}
	}
	fclose(fp);

	if (char_num == 0) {
		printf("mmo_char_init: No character found in %s.\n", char_txt);
		char_log("mmo_char_init: No character found in %s." RETCODE, char_txt);
	} else if (char_num == 1) {
		printf("mmo_char_init: 1 character read in %s.\n", char_txt);
		char_log("mmo_char_init: 1 character read in %s." RETCODE, char_txt);
	} else {
		printf("mmo_char_init: %d characters read in %s.\n", char_num, char_txt);
		char_log("mmo_char_init: %d characters read in %s." RETCODE, char_num, char_txt);
	}

	char_log("Id for the next created character: %d." RETCODE, char_id_count);

	return 0;
}

//---------------------------------------------------------
// Function to save characters in files (speed up by [Yor])
//---------------------------------------------------------
void mmo_char_sync(void) {
	char line[65536],f_line[1024];
	int i, j, k;
	int lock;
	FILE *fp,*f_fp;
	//int *id = (int *) aMalloc(sizeof(int) * char_num);
	CREATE_BUFFER(id, int, char_num);

	// Sorting before save (by [Yor])
	for(i = 0; i < char_num; i++) {
		id[i] = i;
		for(j = 0; j < i; j++) {
			if ((char_dat[i].account_id < char_dat[id[j]].account_id) ||
			    // if same account id, we sort by slot.
			    (char_dat[i].account_id == char_dat[id[j]].account_id &&
			     char_dat[i].char_num < char_dat[id[j]].char_num)) {
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
		printf("WARNING: Server can't not save characters.\n");
		char_log("WARNING: Server can't not save characters." RETCODE);
	} else {
		for(i = 0; i < char_num; i++) {
			// create only once the line, and save it in the 2 files (it's speeder than repeat twice the loop and create twice the line)
			mmo_char_tostr(line, &char_dat[id[i]]); // use of sorted index
				fprintf(fp, "%s" RETCODE, line);
		}
		fprintf(fp, "%d\t%%newid%%" RETCODE, char_id_count);
		lock_fclose(fp, char_txt, &lock);
	}

	// Data save (backup)
	if (backup_txt_flag) { // The backup_txt file was created because char deletion bug existed. Now it's finish and that take a lot of time to create a second file when there are a lot of characters. => option By [Yor]
		fp = lock_fopen(backup_txt, &lock);
		if (fp == NULL) {
			printf("WARNING: Server can't not create backup of characters file.\n");
			char_log("WARNING: Server can't not create backup of characters file." RETCODE);
			//aFree(id); // free up the memory before leaving -.- [Ajarn]
			DELETE_BUFFER(id);
			return;
		}
		for(i = 0; i < char_num; i++) {
			// create only once the line, and save it in the 2 files (it's speeder than repeat twice the loop and create twice the line)
			mmo_char_tostr(line, &char_dat[id[i]]); // use of sorted index
				fprintf(fp, "%s" RETCODE, line);
		}
		fprintf(fp, "%d\t%%newid%%" RETCODE, char_id_count);
		lock_fclose(fp, backup_txt, &lock);
	}

	// Friends List data save (davidsiaw)
	f_fp = lock_fopen(friends_txt, &lock);
	for(i = 0; i < char_num; i++) {
		mmo_friends_list_data_str(f_line, &char_dat[id[i]]);
		fprintf(f_fp, "%s" RETCODE, f_line);
	}

	lock_fclose(f_fp, friends_txt, &lock);

	//aFree(id);
	DELETE_BUFFER(id);

	return;
}

//----------------------------------------------------
// Function to save (in a periodic way) datas in files
//----------------------------------------------------
int mmo_char_sync_timer(int tid, unsigned int tick, int id, int data) {
	mmo_char_sync();
	inter_save();
	return 0;
}

//-----------------------------------
// Function to create a new character
//-----------------------------------
int make_new_char(int fd, unsigned char *dat) {
	int i, j;
	struct char_session_data *sd;

	sd = (struct char_session_data*)session[fd]->session_data;

	// remove control characters from the name
	dat[23] = '\0';
	if (remove_control_chars((unsigned char *)(char*)dat)) {
		char_log("Make new char error (control char received in the name): (connection #%d, account: %d)." RETCODE,
		         fd, sd->account_id);
		return -1;
	}

	// check lenght of character name
	if (strlen((const char*)dat) < 4) {
		char_log("Make new char error (character name too small): (connection #%d, account: %d, name: '%s')." RETCODE,
		         fd, sd->account_id, dat);
		return -1;
	}

	// Check Authorised letters/symbols in the name of the character
	if (char_name_option == 1) { // only letters/symbols in char_name_letters are authorised
		for (i = 0; dat[i]; i++)
			if (strchr(char_name_letters, dat[i]) == NULL) {
				char_log("Make new char error (invalid letter in the name): (connection #%d, account: %d), name: %s, invalid letter: %c." RETCODE,
				         fd, sd->account_id, dat, dat[i]);
				return -1;
			}
	} else if (char_name_option == 2) { // letters/symbols in char_name_letters are forbidden
		for (i = 0; dat[i]; i++)
			if (strchr(char_name_letters, dat[i]) != NULL) {
				char_log("Make new char error (invalid letter in the name): (connection #%d, account: %d), name: %s, invalid letter: %c." RETCODE,
				         fd, sd->account_id, dat, dat[i]);
				return -1;
			}
	} // else, all letters/symbols are authorised (except control char removed before)

	if (dat[24] + dat[25] + dat[26] + dat[27] + dat[28] + dat[29] != 5*6 || // stats
	    dat[30] >= 9 || // slots (dat[30] can not be negativ)
	    dat[33] <= 0 || dat[33] >= 20 || // hair style
	    dat[31] >= 9) { // hair color (dat[31] can not be negativ)
		char_log("Make new char error (invalid values): (connection #%d, account: %d) slot %d, name: %s, stats: %d+%d+%d+%d+%d+%d=%d, hair: %d, hair color: %d" RETCODE,
		         fd, sd->account_id, dat[30], dat, dat[24], dat[25], dat[26], dat[27], dat[28], dat[29], dat[24] + dat[25] + dat[26] + dat[27] + dat[28] + dat[29], dat[33], dat[31]);
		return -1;
	}

	// check individual stat value
	for(i = 24; i <= 29; i++) {
		if (dat[i] < 1 || dat[i] > 9) {
			char_log("Make new char error (invalid stat value: not between 1 to 9): (connection #%d, account: %d) slot %d, name: %s, stats: %d+%d+%d+%d+%d+%d=%d, hair: %d, hair color: %d" RETCODE,
			         fd, sd->account_id, dat[30], dat, dat[24], dat[25], dat[26], dat[27], dat[28], dat[29], dat[24] + dat[25] + dat[26] + dat[27] + dat[28] + dat[29], dat[33], dat[31]);
			return -1;
		}
	}

	for(i = 0; i < char_num; i++) {
		if ((name_ignoring_case != 0 && strcmp(char_dat[i].name, (const char*)dat) == 0) ||
			(name_ignoring_case == 0 && strcmpi(char_dat[i].name, (const char*)dat) == 0)) {
			char_log("Make new char error (name already exists): (connection #%d, account: %d) slot %d, name: %s (actual name of other char: %d), stats: %d+%d+%d+%d+%d+%d=%d, hair: %d, hair color: %d." RETCODE,
			         fd, sd->account_id, dat[30], dat, char_dat[i].name, dat[24], dat[25], dat[26], dat[27], dat[28], dat[29], dat[24] + dat[25] + dat[26] + dat[27] + dat[28] + dat[29], dat[33], dat[31]);
			return -1;
		}
		if (char_dat[i].account_id == sd->account_id && char_dat[i].char_num == dat[30]) {
			char_log("Make new char error (slot already used): (connection #%d, account: %d) slot %d, name: %s (actual name of other char: %d), stats: %d+%d+%d+%d+%d+%d=%d, hair: %d, hair color: %d." RETCODE,
			         fd, sd->account_id, dat[30], dat, char_dat[i].name, dat[24], dat[25], dat[26], dat[27], dat[28], dat[29], dat[24] + dat[25] + dat[26] + dat[27] + dat[28] + dat[29], dat[33], dat[31]);
			return -1;
		}
	}

	if (strcmp(wisp_server_name, (const char*)dat) == 0) {
		char_log("Make new char error (name used is wisp name for server): (connection #%d, account: %d) slot %d, name: %s (actual name of other char: %d), stats: %d+%d+%d+%d+%d+%d=%d, hair: %d, hair color: %d." RETCODE,
		         fd, sd->account_id, dat[30], dat, char_dat[i].name, dat[24], dat[25], dat[26], dat[27], dat[28], dat[29], dat[24] + dat[25] + dat[26] + dat[27] + dat[28] + dat[29], dat[33], dat[31]);
		return -1;
	}

	if (char_num >= char_max) {
		char_max += 256;
		char_dat = (struct mmo_charstatus*)aRealloc(char_dat, sizeof(struct mmo_charstatus) * char_max);
		if (!char_dat) {
			printf("Out of memory: make_new_char (realloc of char_dat).\n");
			char_log("Out of memory: make_new_char (realloc of char_dat)." RETCODE);
			exit(1);
		}
		online_chars = (struct online_chars*)aRealloc(online_chars, sizeof(struct online_chars) * char_max);
		if (!online_chars) {
			printf("Out of memory: make_new_char (realloc of online_chars).\n");
			char_log("Out of memory: make_new_char (realloc of online_chars)." RETCODE);
			exit(1);
		}
		for(j = char_max - 256; j < char_max; j++) {
			online_chars[j].char_id = -1;
			online_chars[j].server = -1;
		}
	}

	char_log("Creation of New Character: (connection #%d, account: %d) slot %d, character Name: %s, stats: %d+%d+%d+%d+%d+%d=%d, hair: %d, hair color: %d." RETCODE,
	         fd, sd->account_id, dat[30], dat, dat[24], dat[25], dat[26], dat[27], dat[28], dat[29], dat[24] + dat[25] + dat[26] + dat[27] + dat[28] + dat[29], dat[33], dat[31]);

	memset(&char_dat[i], 0, sizeof(struct mmo_charstatus));

	char_dat[i].char_id = char_id_count++;
	char_dat[i].account_id = sd->account_id;
	char_dat[i].char_num = dat[30];
	strcpy(char_dat[i].name, (const char*)dat);
	char_dat[i].class_ = 0;
	char_dat[i].base_level = 1;
	char_dat[i].job_level = 1;
	char_dat[i].base_exp = 0;
	char_dat[i].job_exp = 0;
	char_dat[i].zeny = start_zeny;
	char_dat[i].str = dat[24];
	char_dat[i].agi = dat[25];
	char_dat[i].vit = dat[26];
	char_dat[i].int_ = dat[27];
	char_dat[i].dex = dat[28];
	char_dat[i].luk = dat[29];
	char_dat[i].max_hp = 40 * (100 + char_dat[i].vit) / 100;
	char_dat[i].max_sp = 11 * (100 + char_dat[i].int_) / 100;
	char_dat[i].hp = char_dat[i].max_hp;
	char_dat[i].sp = char_dat[i].max_sp;
	char_dat[i].status_point = 0;
	char_dat[i].skill_point = 0;
	char_dat[i].option = 0;
	char_dat[i].karma = 0;
	char_dat[i].manner = 0;
	char_dat[i].party_id = 0;
	char_dat[i].guild_id = 0;
	char_dat[i].hair = dat[33];
	char_dat[i].hair_color = dat[31];
	char_dat[i].clothes_color = 0;
	char_dat[i].inventory[0].nameid = start_weapon; // Knife
	char_dat[i].inventory[0].amount = 1;
	char_dat[i].inventory[0].equip = 0x02;
	char_dat[i].inventory[0].identify = 1;
	char_dat[i].inventory[1].nameid = start_armor; // Cotton Shirt
	char_dat[i].inventory[1].amount = 1;
	char_dat[i].inventory[1].equip = 0x10;
	char_dat[i].inventory[1].identify = 1;
	char_dat[i].weapon = 1;
	char_dat[i].shield = 0;
	char_dat[i].head_top = 0;
	char_dat[i].head_mid = 0;
	char_dat[i].head_bottom = 0;
	memcpy(&char_dat[i].last_point, &start_point, sizeof(start_point));
	memcpy(&char_dat[i].save_point, &start_point, sizeof(start_point));
	char_num++;

	mmo_char_sync();
	return i;
}

//----------------------------------------------------
// This function return the name of the job (by [Yor])
//----------------------------------------------------
char * job_name(int class_) {
	switch (class_) {
	case 0:    return "Novice";
	case 1:    return "Swordsman";
	case 2:    return "Mage";
	case 3:    return "Archer";
	case 4:    return "Acolyte";
	case 5:    return "Merchant";
	case 6:    return "Thief";
	case 7:    return "Knight";
	case 8:    return "Priest";
	case 9:    return "Wizard";
	case 10:   return "Blacksmith";
	case 11:   return "Hunter";
	case 12:   return "Assassin";
	case 13:   return "Knight 2";
	case 14:   return "Crusader";
	case 15:   return "Monk";
	case 16:   return "Sage";
	case 17:   return "Rogue";
	case 18:   return "Alchemist";
	case 19:   return "Bard";
	case 20:   return "Dancer";
	case 21:   return "Crusader 2";
	case 22:   return "Wedding";
	case 23:   return "Super Novice";
	case 4001: return "Novice High";
	case 4002: return "Swordsman High";
	case 4003: return "Mage High";
	case 4004: return "Archer High";
	case 4005: return "Acolyte High";
	case 4006: return "Merchant High";
	case 4007: return "Thief High";
	case 4008: return "Lord Knight";
	case 4009: return "High Priest";
	case 4010: return "High Wizard";
	case 4011: return "Whitesmith";
	case 4012: return "Sniper";
	case 4013: return "Assassin Cross";
	case 4014: return "Peko Knight";
	case 4015: return "Paladin";
	case 4016: return "Champion";
	case 4017: return "Professor";
	case 4018: return "Stalker";
	case 4019: return "Creator";
	case 4020: return "Clown";
	case 4021: return "Gypsy";
	case 4022: return "Peko Paladin";
	case 4023: return "Baby Novice";
	case 4024: return "Baby Swordsman";
	case 4025: return "Baby Mage";
	case 4026: return "Baby Archer";
	case 4027: return "Baby Acolyte";
	case 4028: return "Baby Merchant";
	case 4029: return "Baby Thief";
	case 4030: return "Baby Knight";
	case 4031: return "Baby Priest";
	case 4032: return "Baby Wizard";
	case 4033: return "Baby Blacksmith";
	case 4034: return "Baby Hunter";
	case 4035: return "Baby Assassin";
	case 4036: return "Baby Peco Knight";
	case 4037: return "Baby Crusader";
	case 4038: return "Baby Monk";
	case 4039: return "Baby Sage";
	case 4040: return "Baby Rogue";
	case 4041: return "Baby Alchemist";
	case 4042: return "Baby Bard";
	case 4043: return "Baby Dancer";
	case 4044: return "Baby Peco Crusader";
	case 4045: return "Super Baby";
	}
	return "Unknown Job";
}

//-------------------------------------------------------------
// Function to create the online files (txt and html). by [Yor]
//-------------------------------------------------------------
void create_online_files(void) {
	int i, j, k, l; // for loops
	int players;    // count the number of players
	FILE *fp;       // for the txt file
	FILE *fp2;      // for the html file
	char temp[256];      // to prepare what we must display
	time_t time_server;  // for number of seconds
	struct tm *datetime; // variable for time in structure ->tm_mday, ->tm_sec, ...
	int id[4096];

	// don't return here if we display nothing, because server[j].users is updated in the first loop.

	// Get number of online players, id of each online players, and verify if a server is offline
	players = 0;
	for (i = 0; i < online_players_max; i++) {
		if (online_chars[i].char_id != -1) {
			// check if map-server is online
			j = online_chars[i].server;
			if (j == -1) {
				online_chars[i].char_id = -1;
				continue;
			} else if (server_fd[j] < 0) {
				server[j].users = 0;
				online_chars[i].char_id = -1;
				online_chars[i].server = -1;
				continue;
			}
			// check if the character is twice or more in the list
			// (multiple map-servers and player have successfully connected twice!)
			for(j = i + 1; j < online_players_max; j++)
				if (online_chars[i].char_id == online_chars[j].char_id) {
					k = online_chars[j].server;
					if (k != -1 && server_fd[k] >= 0 && server[k].users > 0)
						server[k].users--;
					online_chars[j].char_id = -1;
					online_chars[j].server = -1;
				}
			// search position of character in char_dat and sort online characters.
			for(j = 0; j < char_num; j++) {
				if (char_dat[j].char_id == online_chars[i].char_id) {
					id[players] = j;
					// use sorting option
					switch (online_sorting_option) {
					case 1: // by name (without case sensitive)
						for(k = 0; k < players; k++)
							if (stricmp(char_dat[j].name, char_dat[id[k]].name) < 0 ||
							   // if same name, we sort with case sensitive.
							   (stricmp(char_dat[j].name, char_dat[id[k]].name) == 0 &&
							    strcmp(char_dat[j].name, char_dat[id[k]].name) < 0)) {
								for(l = players; l > k; l--)
									id[l] = id[l-1];
								id[k] = j; // id[players]
								break;
							}
						break;
					case 2: // by zeny
						for(k = 0; k < players; k++)
							if (char_dat[j].zeny < char_dat[id[k]].zeny ||
							   // if same number of zenys, we sort by name.
							   (char_dat[j].zeny == char_dat[id[k]].zeny &&
							    stricmp(char_dat[j].name, char_dat[id[k]].name) < 0)) {
								for(l = players; l > k; l--)
									id[l] = id[l-1];
								id[k] = j; // id[players]
								break;
							}
						break;
					case 3: // by base level
						for(k = 0; k < players; k++)
							if (char_dat[j].base_level < char_dat[id[k]].base_level ||
							   // if same base level, we sort by base exp.
							   (char_dat[j].base_level == char_dat[id[k]].base_level &&
							    char_dat[j].base_exp < char_dat[id[k]].base_exp)) {
								for(l = players; l > k; l--)
									id[l] = id[l-1];
								id[k] = j; // id[players]
								break;
							}
						break;
					case 4: // by job (and job level)
						for(k = 0; k < players; k++)
							if (char_dat[j].class_ < char_dat[id[k]].class_ ||
							   // if same job, we sort by job level.
							   (char_dat[j].class_ == char_dat[id[k]].class_ &&
							    char_dat[j].job_level < char_dat[id[k]].job_level) ||
							   // if same job and job level, we sort by job exp.
							   (char_dat[j].class_ == char_dat[id[k]].class_ &&
							    char_dat[j].job_level == char_dat[id[k]].job_level &&
							    char_dat[j].job_exp < char_dat[id[k]].job_exp)) {
								for(l = players; l > k; l--)
									id[l] = id[l-1];
								id[k] = j; // id[players]
								break;
							}
						break;
					case 5: // by location map name
						for(k = 0; k < players; k++)
							if (stricmp(char_dat[j].last_point.map, char_dat[id[k]].last_point.map) < 0 ||
							   // if same map name, we sort by name.
							   (stricmp(char_dat[j].last_point.map, char_dat[id[k]].last_point.map) == 0 &&
							    stricmp(char_dat[j].name, char_dat[id[k]].name) < 0)) {
								for(l = players; l > k; l--)
									id[l] = id[l-1];
								id[k] = j; // id[players]
								break;
							}
						break;
					default: // 0 or invalid value: no sorting
						break;
					}
					players++;
					break;
				}
			}
		}
	}

	if (online_display_option == 0) // we display nothing, so return
		return;

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
					strcpy(temp, char_dat[j].name);
					l = isGM(char_dat[j].account_id);
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
					char * jobname = job_name(char_dat[j].class_);
					if ((online_display_option & 6) == 6) {
						fprintf(fp2, "        <td>%s %d/%d</td>\n", jobname, char_dat[j].base_level, char_dat[j].job_level);
						fprintf(fp, "%-18s %3d/%3d ", jobname, char_dat[j].base_level, char_dat[j].job_level);
					} else if (online_display_option & 2) {
						fprintf(fp2, "        <td>%s</td>\n", jobname);
						fprintf(fp, "%-18s ", jobname);
					} else if (online_display_option & 4) {
						fprintf(fp2, "        <td>%d/%d</td>\n", char_dat[j].base_level, char_dat[j].job_level);
						fprintf(fp, "%3d/%3d ", char_dat[j].base_level, char_dat[j].job_level);
					}
				}
				// displaying of the map
				if (online_display_option & 24) { // 8 or 16
					// prepare map name
					memset(temp, 0, 17);
					strncpy(temp, char_dat[j].last_point.map, 16);
					if (strstr(temp, ".gat") != NULL) {
						temp[strstr(temp, ".gat") - temp] = 0; // suppress the '.gat'
					}
					// write map name
					if (online_display_option & 16) { // map-name AND coordonates
						fprintf(fp2, "        <td>%s (%d, %d)</td>\n", temp, char_dat[j].last_point.x, char_dat[j].last_point.y);
						fprintf(fp, "%-12s (%3d,%3d) ", temp, char_dat[j].last_point.x, char_dat[j].last_point.y);
					} else {
						fprintf(fp2, "        <td>%s</td>\n", temp);
						fprintf(fp, "%-12s ", temp);
					}
				}
				// displaying nimber of zenys
				if (online_display_option & 32) {
					// write number of zenys
					if (char_dat[j].zeny == 0) { // if no zeny
						fprintf(fp2, "        <td ALIGN=RIGHT>no zeny</td>\n");
						fprintf(fp, "        no zeny ");
					} else {
						fprintf(fp2, "        <td ALIGN=RIGHT>%d z</td>\n", char_dat[j].zeny);
						fprintf(fp, "%13d z ", char_dat[j].zeny);
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
int count_users(void) {
	int i, users;

	users = 0;
	for(i = 0; i < MAX_MAP_SERVERS; i++)
		if (server_fd[i] >= 0)
			users += server[i].users;

	return users;
}

//----------------------------------------
// Function to send characters to a player
//----------------------------------------
int mmo_char_send006b(int fd, struct char_session_data *sd) {
	int i, j, found_num;
	struct mmo_charstatus *p;
#ifdef NEW_006b
	const int offset = 24;
#else
	const int offset = 4;
#endif

	found_num = 0;
	for(i = 0; i < char_num; i++) {
		if (char_dat[i].account_id == sd->account_id) {
			sd->found_char[found_num] = i;
			found_num++;
			if (found_num == 9)
				break;
		}
	}
	for(i = found_num; i < 9; i++)
		sd->found_char[i] = -1;

	memset(WFIFOP(fd,0), 0, offset + found_num * 106);
	WFIFOW(fd,0) = 0x6b;
	WFIFOW(fd,2) = offset + found_num * 106;

	for(i = 0; i < found_num; i++) {
		p = &char_dat[sd->found_char[i]];
		j = offset + (i * 106); // increase speed of code

		WFIFOL(fd,j) = p->char_id;
		WFIFOL(fd,j+4) = p->base_exp;
		WFIFOL(fd,j+8) = p->zeny;
		WFIFOL(fd,j+12) = p->job_exp;
		WFIFOL(fd,j+16) = p->job_level;

		WFIFOL(fd,j+20) = 0;
		WFIFOL(fd,j+24) = 0;
		WFIFOL(fd,j+28) = p->option;

		WFIFOL(fd,j+32) = p->karma;
		WFIFOL(fd,j+36) = p->manner;

		WFIFOW(fd,j+40) = p->status_point;
		WFIFOW(fd,j+42) = (p->hp > 0x7fff) ? 0x7fff : p->hp;
		WFIFOW(fd,j+44) = (p->max_hp > 0x7fff) ? 0x7fff : p->max_hp;
		WFIFOW(fd,j+46) = (p->sp > 0x7fff) ? 0x7fff : p->sp;
		WFIFOW(fd,j+48) = (p->max_sp > 0x7fff) ? 0x7fff : p->max_sp;
		WFIFOW(fd,j+50) = DEFAULT_WALK_SPEED; // p->speed;
		WFIFOW(fd,j+52) = p->class_;
		WFIFOW(fd,j+54) = p->hair;

		// pecopeco knights/crusaders crash fix
		if (p->class_ == 13 || p->class_ == 21 ||
			p->class_ == 4014 || p->class_ == 4022)
			WFIFOW(fd,j+56) = 0;
		else WFIFOW(fd,j+56) = p->weapon;

		WFIFOW(fd,j+58) = p->base_level;
		WFIFOW(fd,j+60) = p->skill_point;
		WFIFOW(fd,j+62) = p->head_bottom;
		WFIFOW(fd,j+64) = p->shield;
		WFIFOW(fd,j+66) = p->head_top;
		WFIFOW(fd,j+68) = p->head_mid;
		WFIFOW(fd,j+70) = p->hair_color;
		WFIFOW(fd,j+72) = p->clothes_color;

		memcpy(WFIFOP(fd,j+74), p->name, 24);

		WFIFOB(fd,j+98) = (p->str > 255) ? 255 : p->str;
		WFIFOB(fd,j+99) = (p->agi > 255) ? 255 : p->agi;
		WFIFOB(fd,j+100) = (p->vit > 255) ? 255 : p->vit;
		WFIFOB(fd,j+101) = (p->int_ > 255) ? 255 : p->int_;
		WFIFOB(fd,j+102) = (p->dex > 255) ? 255 : p->dex;
		WFIFOB(fd,j+103) = (p->luk > 255) ? 255 : p->luk;
		WFIFOB(fd,j+104) = p->char_num;
	}

	WFIFOSET(fd,WFIFOW(fd,2));

	return 0;
}

int set_account_reg2(int acc, int num, struct global_reg *reg) {
	int i, c;

	c = 0;
	for(i = 0; i < char_num; i++) {
		if (char_dat[i].account_id == acc) {
			memcpy(char_dat[i].account_reg2, reg, sizeof(char_dat[i].account_reg2));
			char_dat[i].account_reg2_num = num;
			c++;
		}
	}
	return c;
}

// ó£ç•(charçÌèúéûÇ…égóp)
int char_divorce(struct mmo_charstatus *cs) {
	if (cs == NULL)
		return 0;

	if (cs->partner_id > 0){
		int i, j;
		for(i = 0; i < char_num; i++) {
			if (char_dat[i].char_id == cs->partner_id && char_dat[i].partner_id == cs->char_id) {
				cs->partner_id = 0;
				char_dat[i].partner_id = 0;
				for(j = 0; j < MAX_INVENTORY; j++)
					if (char_dat[i].inventory[j].nameid == WEDDING_RING_M || char_dat[i].inventory[j].nameid == WEDDING_RING_F)
						memset(&char_dat[i].inventory[j], 0, sizeof(char_dat[i].inventory[0]));
					if (cs->inventory[j].nameid == WEDDING_RING_M || cs->inventory[j].nameid == WEDDING_RING_F)
						memset(&cs->inventory[j], 0, sizeof(cs->inventory[0]));
				return 0;
			}
		}
	}
	return 0;
}

//------------------------------------------------------------
// E-mail check: return 0 (not correct) or 1 (valid). by [Yor]
//------------------------------------------------------------
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

//----------------------------------------------------------------------
// Force disconnection of an online player (with account value) by [Yor]
//----------------------------------------------------------------------
int disconnect_player(int accound_id) {
	int i;
	struct char_session_data *sd;

	// disconnect player if online on char-server
	for(i = 0; i < fd_max; i++) {
		if (session[i] && (sd = (struct char_session_data*)session[i]->session_data)) {
			if (sd->account_id == accound_id) {
				session[i]->eof = 1;
				return 1;
			}
		}
	}

	return 0;
}

// ÉLÉÉÉâçÌèúÇ…î∫Ç§ÉfÅ[É^çÌèú
static int char_delete(struct mmo_charstatus *cs) {
	int j;

	// ÉyÉbÉgçÌèú
	if (cs->pet_id)
		inter_pet_delete(cs->pet_id);
	for (j = 0; j < MAX_INVENTORY; j++)
		if (cs->inventory[j].card[0] == (short)0xff00)
			inter_pet_delete(*((long *)(&cs->inventory[j].card[2])));
	for (j = 0; j < MAX_CART; j++)
		if (cs->cart[j].card[0] == (short)0xff00)
			inter_pet_delete(*((long *)(&cs->cart[j].card[2])));
	// ÉMÉãÉhíEëﬁ
	if (cs->guild_id)
		inter_guild_leave(cs->guild_id, cs->account_id, cs->char_id);
	// ÉpÅ[ÉeÉBÅ[íEëﬁ
	if (cs->party_id)
		inter_party_leave(cs->party_id, cs->account_id);
	// ó£ç•
	if (cs->partner_id){
		// ó£ç•èÓïÒÇmapÇ…í ím
		unsigned char buf[10];
		WBUFW(buf,0) = 0x2b12;
		WBUFL(buf,2) = cs->char_id;
		WBUFL(buf,6) = cs->partner_id;
		mapif_sendall(buf,10);
		// ó£ç•
		char_divorce(cs);
	}
	return 0;
}

int parse_tologin(int fd) {
	int i;
	struct char_session_data *sd;

	// only login-server can have an access to here.
	// so, if it isn't the login-server, we disconnect the session (fd != login_fd).
	if (fd != login_fd)
		session[fd]->eof = 1;
	if(session[fd]->eof) {
		if (fd == login_fd) {
			printf("Char-server can't connect to login-server (connection #%d).\n", fd);
			login_fd = -1;
		}
		close(fd);
		delete_session(fd);
		return 0;
	}

	sd = (struct char_session_data*)session[fd]->session_data;

	while(RFIFOREST(fd) >= 2) {
//		printf("parse_tologin: connection #%d, packet: 0x%x (with being read: %d bytes).\n", fd, RFIFOW(fd,0), RFIFOREST(fd));

		switch(RFIFOW(fd,0)) {
		case 0x2711:
			if (RFIFOREST(fd) < 3)
				return 0;
			if (RFIFOB(fd,2)) {
//				printf("connect login server error : %d\n", RFIFOB(fd,2));
				printf("Can not connect to login-server.\n");
				printf("The server communication passwords (default s1/p1) is probably invalid.\n");
				printf("Also, please make sure your accounts file (default: accounts.txt) has those values present.\n");
				printf("If you changed the communication passwords, change them back at map_athena.conf and char_athena.conf\n");
				exit(1);
			} else {
				printf("Connected to login-server (connection #%d).\n", fd);
				// if no map-server already connected, display a message...
				for(i = 0; i < MAX_MAP_SERVERS; i++)
					if (server_fd[i] >= 0 && server[i].map[0][0]) // if map-server online and at least 1 map
						break;
				if (i == MAX_MAP_SERVERS)
					printf("Awaiting maps from map-server.\n");
			}
			RFIFOSKIP(fd,3);
			break;

		case 0x2713:
			if (RFIFOREST(fd) < 51)
				return 0;
//			printf("parse_tologin 2713 : %d\n", RFIFOB(fd,6));
			for(i = 0; i < fd_max; i++) {
				if (session[i] && (sd = (struct char_session_data*)session[i]->session_data) && sd->account_id == RFIFOL(fd,2)) {
					if (RFIFOB(fd,6) != 0) {
						WFIFOW(i,0) = 0x6c;
						WFIFOB(i,2) = 0x42;
						WFIFOSET(i,3);
					} else if (max_connect_user == 0 || count_users() < max_connect_user) {
//						if (max_connect_user == 0)
//							printf("max_connect_user (unlimited) -> accepted.\n");
//						else
//							printf("count_users(): %d < max_connect_user (%d) -> accepted.\n", count_users(), max_connect_user);
						memcpy(sd->email, RFIFOP(fd, 7), 40);
						if (e_mail_check(sd->email) == 0)
							strncpy(sd->email, "a@a.com", 40); // default e-mail
						sd->connect_until_time = (time_t)RFIFOL(fd,47);
						// send characters to player
						mmo_char_send006b(i, sd);
					} else if(isGM(sd->account_id) >= gm_allow_level) {
						sd->connect_until_time = (time_t)RFIFOL(fd,47);
						// send characters to player
						mmo_char_send006b(i, sd);
					} else {
						// refuse connection: too much online players
//						printf("count_users(): %d < max_connect_use (%d) -> fail...\n", count_users(), max_connect_user);
						WFIFOW(i,0) = 0x6c;
						WFIFOW(i,2) = 0;
						WFIFOSET(i,3);
					}
					break;
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

		// Receiving authentification from Freya-type login server (to avoid char->login->char)
		case 0x2719:
			if (RFIFOREST(fd) < 18)
				return 0;
			// to conserv a maximum of authentification, search if account is already authentified and replace it
			// that will reduce multiple connection too
			for(i = 0; i < AUTH_FIFO_SIZE; i++)
				if (auth_fifo[i].account_id == RFIFOL(fd,2))
					break;
			// if not found, use next value
			if (i == AUTH_FIFO_SIZE) {
				if (auth_fifo_pos >= AUTH_FIFO_SIZE)
					auth_fifo_pos = 0;
				i = auth_fifo_pos;
				auth_fifo_pos++;
			}
			//printf("auth_fifo set (auth #%d) - account: %d, secure: %08x-%08x\n", i, RFIFOL(fd,2), RFIFOL(fd,6), RFIFOL(fd,10));
			auth_fifo[i].account_id = RFIFOL(fd,2);
			auth_fifo[i].char_id = 0;
			auth_fifo[i].login_id1 = RFIFOL(fd,6);
			auth_fifo[i].login_id2 = RFIFOL(fd,10);
			auth_fifo[i].delflag = 2; // 0: auth_fifo canceled/void, 2: auth_fifo received from login/map server in memory, 1: connection authentified
			auth_fifo[i].char_pos = 0;
			auth_fifo[i].connect_until_time = 0; // unlimited/unknown time by default (not display in map-server)
			auth_fifo[i].ip = RFIFOL(fd,14);
			//auth_fifo[i].map_auth = 0;
			RFIFOSKIP(fd,18);
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
//			printf("parse_tologin: To become GM answer: char -> map.\n");
		  }
			RFIFOSKIP(fd,10);
			break;

		case 0x2723:	// changesex reply (modified by [Yor])
			if (RFIFOREST(fd) < 7)
				return 0;
		  {
			int acc, sex, i, j;
			unsigned char buf[7];
			acc = RFIFOL(fd,2);
			sex = RFIFOB(fd,6);
			RFIFOSKIP(fd, 7);
			if (acc > 0) {
				for (i = 0; i < char_num; i++) {
					if (char_dat[i].account_id == acc) {
						int jobclass = char_dat[i].class_;
						char_dat[i].sex = sex;
						auth_fifo[i].sex = sex;
						if (jobclass == 19 || jobclass == 20 ||
						    jobclass == 4020 || jobclass == 4021 ||
						    jobclass == 4042 || jobclass == 4043) {
							// job modification
							if (jobclass == 19 || jobclass == 20) {
								char_dat[i].class_ = (sex) ? 19 : 20;
							} else if (jobclass == 4020 || jobclass == 4021) {
								char_dat[i].class_ = (sex) ? 4020 : 4021;
							} else if (jobclass == 4042 || jobclass == 4043) {
								char_dat[i].class_ = (sex) ? 4042 : 4043;
							}
							// remove specifical skills of classes 19, 4020 and 4042
							for(j = 315; j <= 322; j++) {
								if (char_dat[i].skill[j].id > 0 && !char_dat[i].skill[j].flag) {
									char_dat[i].skill_point += char_dat[i].skill[j].lv;
									char_dat[i].skill[j].id = 0;
									char_dat[i].skill[j].lv = 0;
								}
							}
							// remove specifical skills of classes 20, 4021 and 4043
							for(j = 323; j <= 330; j++) {
								if (char_dat[i].skill[j].id > 0 && !char_dat[i].skill[j].flag) {
									char_dat[i].skill_point += char_dat[i].skill[j].lv;
									char_dat[i].skill[j].id = 0;
									char_dat[i].skill[j].lv = 0;
								}
							}
						}
						// to avoid any problem with equipment and invalid sex, equipment is unequiped.
						for (j = 0; j < MAX_INVENTORY; j++) {
							if (char_dat[i].inventory[j].nameid && char_dat[i].inventory[j].equip)
								char_dat[i].inventory[j].equip = 0;
						}
						char_dat[i].weapon = 0;
						char_dat[i].shield = 0;
						char_dat[i].head_top = 0;
						char_dat[i].head_mid = 0;
						char_dat[i].head_bottom = 0;
					}
				}
				// disconnect player if online on char-server
				disconnect_player(acc);
			}
			WBUFW(buf,0) = 0x2b0d;
			WBUFL(buf,2) = acc;
			WBUFB(buf,6) = sex;
			mapif_sendall(buf, 7);
		  }
			break;

		case 0x2726:	// Request to send a broadcast message (no answer)
			if (RFIFOREST(fd) < 8 || RFIFOREST(fd) < (8 + RFIFOL(fd,4)))
				return 0;
			if (RFIFOL(fd,4) < 1)
				char_log("Receiving a message for broadcast, but message is void." RETCODE);
			else {
				// at least 1 map-server
				for(i = 0; i < MAX_MAP_SERVERS; i++)
					if (server_fd[i] >= 0)
						break;
				if (i == MAX_MAP_SERVERS)
					char_log("'ladmin': Receiving a message for broadcast, but no map-server is online." RETCODE);
				else {
					unsigned char buf[128];
					char message[4096]; // +1 to add a null terminated if not exist in the packet
					int lp;
					char *p;
					memset(message, '\0', sizeof(message));
					memcpy(message, RFIFOP(fd,8), RFIFOL(fd,4));
					message[sizeof(message)-1] = '\0';
					remove_control_chars((unsigned char *)message);
					// remove all first spaces
					p = message;
					while(p[0] == ' ')
						p++;
					// if message is only composed of spaces
					if (p[0] == '\0')
						char_log("Receiving a message for broadcast, but message is only a lot of spaces." RETCODE);
					// else send message to all map-servers
					else {
						if (RFIFOW(fd,2) == 0) {
							char_log("'ladmin': Receiving a message for broadcast (message (in yellow): %s)" RETCODE,
							         message);
							lp = 4;
						} else {
							char_log("'ladmin': Receiving a message for broadcast (message (in blue): %s)" RETCODE,
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

		// account_reg2ïœçXí ím
		case 0x2729:
			if (RFIFOREST(fd) < 4 || RFIFOREST(fd) < RFIFOW(fd,2))
				return 0;
		  {
			struct global_reg reg[ACCOUNT_REG2_NUM];
			unsigned char buf[4096];
			int j, p, acc;
			acc = RFIFOL(fd,4);
			for (p = 8, j = 0; p < RFIFOW(fd,2) && j < ACCOUNT_REG2_NUM; p += 36, j++) {
				memcpy(reg[j].str, RFIFOP(fd,p), 32);
				reg[j].value = RFIFOL(fd,p+32);
			}
			set_account_reg2(acc, j, reg);
			// ìØçCÉçÉOÉCÉìÇã÷é~ÇµÇƒÇ¢ÇÍÇŒëóÇÈïKóvÇÕñ≥Ç¢
			memcpy(buf, RFIFOP(fd,0), RFIFOW(fd,2));
			WBUFW(buf,0) = 0x2b11;
			mapif_sendall(buf, WBUFW(buf,2));
			RFIFOSKIP(fd,RFIFOW(fd,2));
//			printf("char: save_account_reg_reply\n");
		  }
			break;

		// Account deletion notification (from login-server)
		case 0x2730:
			if (RFIFOREST(fd) < 6)
				return 0;
			// Deletion of all characters of the account
			for(i = 0; i < char_num; i++) {
				if (char_dat[i].account_id == RFIFOL(fd,2)) {
					char_delete(&char_dat[i]);
					if (i < char_num - 1) {
						memcpy(&char_dat[i], &char_dat[char_num-1], sizeof(struct mmo_charstatus));
						// if moved character owns to deleted account, check again it's character
						if (char_dat[i].account_id == RFIFOL(fd,2)) {
							i--;
						// Correct moved character reference in the character's owner by [Yor]
						} else {
							int j, k;
							struct char_session_data *sd2;
							for (j = 0; j < fd_max; j++) {
								if (session[j] && (sd2 = (struct char_session_data*)session[j]->session_data) &&
									sd2->account_id == char_dat[char_num-1].account_id) {
									for (k = 0; k < 9; k++) {
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
			unsigned char buf[32000];
			if (gm_account != NULL)
				aFree(gm_account);
			gm_account = (struct gm_account*)aCalloc(sizeof(struct gm_account) * ((RFIFOW(fd,2) - 4) / 5), 1);
			GM_num = 0;
			for (i = 4; i < RFIFOW(fd,2); i = i + 5) {
				gm_account[GM_num].account_id = RFIFOL(fd,i);
				gm_account[GM_num].level = (int)RFIFOB(fd,i+4);
				//printf("GM account: %d -> level %d\n", gm_account[GM_num].account_id, gm_account[GM_num].level);
				GM_num++;
			}
			printf("From login-server: receiving of %d GM accounts information.\n", GM_num);
			char_log("From login-server: receiving of %d GM accounts information." RETCODE, GM_num);
			create_online_files(); // update online players files (perhaps some online players change of GM level)
			// send new gm acccounts level to map-servers
			memcpy(buf, RFIFOP(fd,0), RFIFOW(fd,2));
			WBUFW(buf,0) = 0x2b15;
			mapif_sendall(buf, RFIFOW(fd,2));
		  }
			RFIFOSKIP(fd,RFIFOW(fd,2));
			break;

		// Receive GM accounts [Freya login server packet by Yor]
		case 0x2733:
		// add test here to remember that the login-server is Freya-type
		// sprintf (login_server_type, "Freya");
			if (RFIFOREST(fd) < 7)
				return 0;
			{
				unsigned char buf[32000];
				int new_level = 0;
				for(i = 0; i < GM_num; i++)
					if (gm_account[i].account_id == RFIFOL(fd,2)) {
						if (gm_account[i].level != (int)RFIFOB(fd,6)) {
							gm_account[i].level = (int)RFIFOB(fd,6);
							new_level = 1;
						}
						break;
					}
				// if not found, add it
				if (i == GM_num) {
					// limited to 4000, because we send information to char-servers (more than 4000 GM accounts???)
					// int (id) + int (level) = 8 bytes * 4000 = 32k (limit of packets in windows)
					if (((int)RFIFOB(fd,6)) > 0 && GM_num < 4000) {
						if (GM_num == 0) {
							gm_account = (struct gm_account*)aMalloc(sizeof(struct gm_account));
						} else {
							gm_account = (struct gm_account*)aRealloc(gm_account, sizeof(struct gm_account) * (GM_num + 1));						
						}
						gm_account[GM_num].account_id = RFIFOL(fd,2);
						gm_account[GM_num].level = (int)RFIFOB(fd,6);
						new_level = 1;
						GM_num++;
						if (GM_num >= 4000) {
							printf("***WARNING: 4000 GM accounts found. Next GM accounts are not readed.\n");
							char_log("***WARNING: 4000 GM accounts found. Next GM accounts are not readed." RETCODE);
						}
					}
				}
				if (new_level == 1) {
					int len;
					printf("From login-server: receiving a GM account information (%d: level %d).\n", RFIFOL(fd,2), (int)RFIFOB(fd,6));
					char_log("From login-server: receiving a GM account information (%d: level %d)." RETCODE, RFIFOL(fd,2), (int)RFIFOB(fd,6));
					//create_online_files(); // not change online file for only 1 player (in next timer, that will be done
					// send gm acccounts level to map-servers
					len = 4;
					WBUFW(buf,0) = 0x2b15;
				
					for(i = 0; i < GM_num; i++) {
						WBUFL(buf, len) = gm_account[i].account_id;
						WBUFB(buf, len+4) = (unsigned char)gm_account[i].level;
						len += 5;
					}
					WBUFW(buf, 2) = len;
					mapif_sendall(buf, len);
				}
			}
			RFIFOSKIP(fd,7);
			break;

		default:
			printf("parse_tologin: unknown packet %x! \n", RFIFOW(fd,0));
			session[fd]->eof = 1;
			return 0;
		}
	}
	RFIFOFLUSH(fd);

	return 0;
}

int parse_frommap(int fd) {
	int i, j;
	int id;

	for(id = 0; id < MAX_MAP_SERVERS; id++)
		if (server_fd[id] == fd)
			break;
	if(id==MAX_MAP_SERVERS)
		session[fd]->eof=1;
	if(session[fd]->eof){
		for(i = 0; i < MAX_MAP_SERVERS; i++)
			if (server_fd[i] == fd) {
				printf("Map-server %d has disconnected.\n", i);
				server_fd[i] = -1;
			}
		close(fd);
		delete_session(fd);
		create_online_files();
		return 0;
	}

	while(RFIFOREST(fd) >= 2) {
//		printf("parse_frommap: connection #%d, packet: 0x%x (with being read: %d bytes).\n", fd, RFIFOW(fd,0), RFIFOREST(fd));

		switch(RFIFOW(fd,0)) {

		// map-server alive packet
		case 0x2718:
			if (RFIFOREST(fd) < 2)
				return 0;
			RFIFOSKIP(fd,2);
			break;

		// request from map-server to reload GM accounts. Transmission to login-server (by Yor)
		case 0x2af7:
			if (login_fd > 0) { // don't send request if no login-server
				WFIFOW(login_fd,0) = 0x2709;
				WFIFOSET(login_fd, 2);
//				printf("char : request from map-server to reload GM accounts -> login-server.\n");
			}
			RFIFOSKIP(fd,2);
			break;

		// Receiving map names list from the map-server
		case 0x2afa:
			if (RFIFOREST(fd) < 4 || RFIFOREST(fd) < RFIFOW(fd,2))
				return 0;
			memset(server[id].map, 0, sizeof(server[id].map));
			j = 0;
			for(i = 4; i < RFIFOW(fd,2); i += 16) {
				memcpy(server[id].map[j], RFIFOP(fd,i), 16);
//				printf("set map %d.%d : %s\n", id, j, server[id].map[j]);
				j++;
			}
			{
				unsigned char *p = (unsigned char *)&server[id].ip;
				printf("Map-Server %d connected: %d maps, from IP %d.%d.%d.%d port %d.\n",
				       id, j, p[0], p[1], p[2], p[3], server[id].port);
				printf("Map-server %d loading complete.\n", id);
				char_log("Map-Server %d connected: %d maps, from IP %d.%d.%d.%d port %d. Map-server %d loading complete." RETCODE,
				         id, j, p[0], p[1], p[2], p[3], server[id].port, id);
			}
			WFIFOW(fd,0) = 0x2afb;
			WFIFOB(fd,2) = 0;
			memcpy(WFIFOP(fd,3), wisp_server_name, 24); // name for wisp to player
			WFIFOSET(fd,27);
			{
				unsigned char buf[16384];
				int x;
				if (j == 0) {
					printf("WARNING: Map-Server %d have NO map.\n", id);
					char_log("WARNING: Map-Server %d have NO map." RETCODE, id);
				// Transmitting maps information to the other map-servers
				} else {
					WBUFW(buf,0) = 0x2b04;
					WBUFW(buf,2) = j * 16 + 10;
					WBUFL(buf,4) = server[id].ip;
					WBUFW(buf,8) = server[id].port;
					memcpy(WBUFP(buf,10), RFIFOP(fd,4), j * 16);
					mapif_sendallwos(fd, buf, WBUFW(buf,2));
				}
				// Transmitting the maps of the other map-servers to the new map-server
				for(x = 0; x < MAX_MAP_SERVERS; x++) {
					if (server_fd[x] >= 0 && x != id) {
						WFIFOW(fd,0) = 0x2b04;
						WFIFOL(fd,4) = server[x].ip;
						WFIFOW(fd,8) = server[x].port;
						j = 0;
						for(i = 0; i < MAX_MAP_PER_SERVER; i++)
							if (server[x].map[i][0])
								memcpy(WFIFOP(fd,10+(j++)*16), server[x].map[i], 16);
						if (j > 0) {
							WFIFOW(fd,2) = j * 16 + 10;
							WFIFOSET(fd,WFIFOW(fd,2));
						}
					}
				}
			}
			RFIFOSKIP(fd,RFIFOW(fd,2));
			break;

		// îFèÿóvã
		// Send character data to map-serverÅ
		case 0x2afc:
			if (RFIFOREST(fd) < 22)
				return 0;
			//printf("auth_fifo search: account: %d, char: %d, secure: %08x-%08x\n", RFIFOL(fd,2), RFIFOL(fd,6), RFIFOL(fd,10), RFIFOL(fd,14));
			for(i = 0; i < AUTH_FIFO_SIZE; i++) {
				if (auth_fifo[i].account_id == RFIFOL(fd,2) &&
				    auth_fifo[i].char_id == RFIFOL(fd,6) &&
				    auth_fifo[i].login_id1 == RFIFOL(fd,10) &&
#if CMP_AUTHFIFO_LOGIN2 != 0
				// here, it's the only area where it's possible that we doesn't know login_id2 (map-server asks just after 0x72 packet, that doesn't given the value)
				    (auth_fifo[i].login_id2 == RFIFOL(fd,14) || RFIFOL(fd,14) == 0) && // relate to the versions higher than 18
#endif
				    (!check_ip_flag || auth_fifo[i].ip == RFIFOL(fd,18)) &&
				    !auth_fifo[i].delflag) {
					auth_fifo[i].delflag = 1;
					WFIFOW(fd,0) = 0x2afd;
					WFIFOW(fd,2) = 16 + sizeof(struct mmo_charstatus);
					WFIFOL(fd,4) = RFIFOL(fd,2);
					WFIFOL(fd,8) = auth_fifo[i].login_id2;
					WFIFOL(fd,12) = (unsigned long)auth_fifo[i].connect_until_time;
					char_dat[auth_fifo[i].char_pos].sex = auth_fifo[i].sex;
					memcpy(WFIFOP(fd,16), &char_dat[auth_fifo[i].char_pos], sizeof(struct mmo_charstatus));
					WFIFOSET(fd, WFIFOW(fd,2));
					//printf("auth_fifo search success (auth #%d, account %d, character: %d).\n", i, RFIFOL(fd,2), RFIFOL(fd,6));
					break;
				}
			}
			if (i == AUTH_FIFO_SIZE) {
				WFIFOW(fd,0) = 0x2afe;
				WFIFOL(fd,2) = RFIFOL(fd,2);
				WFIFOSET(fd,6);
				printf("auth_fifo search error! account %d not authentified.\n", RFIFOL(fd,2));
			}
			RFIFOSKIP(fd,22);
			break;

		// MAPÉTÅ[ÉoÅ[è„ÇÃÉÜÅ[ÉUÅ[êîéÛêM
		// Recieve alive message from map-server
		case 0x2aff:
			if (RFIFOREST(fd) < 6 || RFIFOREST(fd) < RFIFOW(fd,2))
				return 0;
			server[id].users = RFIFOW(fd,4);
			// remove all previously online players of the server
			for(i = 0; i < online_players_max; i++)
				if (online_chars[i].server == id) {
					online_chars[i].char_id = -1;
					online_chars[i].server = -1;
				}
			// add online players in the list by [Yor]
			j = 0;
			for(i = 0; i < server[id].users; i++) {
				for(; j < online_players_max; j++)
					if (online_chars[j].char_id == -1) {
						online_chars[j].char_id = RFIFOL(fd,6+i*4);
						online_chars[j].server = id;
						//printf("%d\n", online_chars[j].char_id);
						break;
					}
				// no available slots...
				if (j == online_players_max) {
					// create 256 new slots
					online_players_max += 256;
					online_chars = (struct online_chars*)aRealloc(online_chars, sizeof(struct online_chars) * online_players_max);
					if (!online_chars) {
						printf("out of memory: parse_frommap - online_chars (realloc).\n");
						exit(1);
					}
					for( ; j < online_players_max; j++) {
						online_chars[j].char_id = -1;
						online_chars[j].server = -1;
					}
					// save data
					j = online_players_max - 256;
					online_chars[j].char_id = RFIFOL(fd,6+i*4);
					online_chars[j].server = id;
				}
			}
			if (update_online < time(NULL)) { // Time is done
				update_online = time(NULL) + 8;
				create_online_files(); // only every 8 sec. (normally, 1 server send users every 5 sec.) Don't update every time, because that takes time, but only every 2 connection.
				                       // it set to 8 sec because is more than 5 (sec) and if we have more than 1 map-server, informations can be received in shifted.
			}
			RFIFOSKIP(fd,6+i*4);
			break;

		// ÉLÉÉÉâÉfÅ[É^ï€ë∂
		// Recieve character data from map-server
		case 0x2b01:
			if (RFIFOREST(fd) < 4 || RFIFOREST(fd) < RFIFOW(fd,2))
				return 0;
			for(i = 0; i < char_num; i++) {
				if (char_dat[i].account_id == RFIFOL(fd,4) &&
				    char_dat[i].char_id == RFIFOL(fd,8))
					break;
			}
			if (i != char_num)
				memcpy(&char_dat[i], RFIFOP(fd,12), sizeof(struct mmo_charstatus));
			RFIFOSKIP(fd,RFIFOW(fd,2));
			break;

		// ÉLÉÉÉâÉZÉåóvãÅ
		case 0x2b02:
			if (RFIFOREST(fd) < 18)
				return 0;
			if (auth_fifo_pos >= AUTH_FIFO_SIZE)
				auth_fifo_pos = 0;
			//printf("auth_fifo set (auth #%d) - account: %d, secure: %08x-%08x\n", auth_fifo_pos, RFIFOL(fd,2), RFIFOL(fd,6), RFIFOL(fd,10));
			auth_fifo[auth_fifo_pos].account_id = RFIFOL(fd,2);
			auth_fifo[auth_fifo_pos].char_id = 0;
			auth_fifo[auth_fifo_pos].login_id1 = RFIFOL(fd,6);
			auth_fifo[auth_fifo_pos].login_id2 = RFIFOL(fd,10);
			auth_fifo[auth_fifo_pos].delflag = 2;
			auth_fifo[auth_fifo_pos].char_pos = 0;
			auth_fifo[auth_fifo_pos].connect_until_time = 0; // unlimited/unknown time by default (not display in map-server)
			auth_fifo[auth_fifo_pos].ip = RFIFOL(fd,14);
			auth_fifo_pos++;
			WFIFOW(fd,0) = 0x2b03;
			WFIFOL(fd,2) = RFIFOL(fd,2);
			WFIFOB(fd,6) = 0;
			WFIFOSET(fd,7);
			RFIFOSKIP(fd,18);
			break;

		// É}ÉbÉvÉTÅ[ÉoÅ[ä‘à⁄ìÆóvãÅ
		case 0x2b05:
			if (RFIFOREST(fd) < 49)
				return 0;
			if (auth_fifo_pos >= AUTH_FIFO_SIZE)
				auth_fifo_pos = 0;
			WFIFOW(fd,0) = 0x2b06;
			memcpy(WFIFOP(fd,2), RFIFOP(fd,2), 42);
			//printf("auth_fifo set (auth#%d) - account: %d, secure: 0x%08x-0x%08x\n", auth_fifo_pos, RFIFOL(fd,2), RFIFOL(fd,6), RFIFOL(fd,10));
			auth_fifo[auth_fifo_pos].account_id = RFIFOL(fd,2);
			auth_fifo[auth_fifo_pos].char_id = RFIFOL(fd,14);
			auth_fifo[auth_fifo_pos].login_id1 = RFIFOL(fd,6);
			auth_fifo[auth_fifo_pos].login_id2 = RFIFOL(fd,10);
			auth_fifo[auth_fifo_pos].delflag = 0;
			auth_fifo[auth_fifo_pos].sex = RFIFOB(fd,44);
			auth_fifo[auth_fifo_pos].connect_until_time = 0; // unlimited/unknown time by default (not display in map-server)
			auth_fifo[auth_fifo_pos].ip = RFIFOL(fd,45);
			for(i = 0; i < char_num; i++)
				if (char_dat[i].account_id == RFIFOL(fd,2) &&
				    char_dat[i].char_id == RFIFOL(fd,14)) {
					auth_fifo[auth_fifo_pos].char_pos = i;
					auth_fifo_pos++;
					WFIFOL(fd,6) = 0;
					break;
				}
			if (i == char_num)
				WFIFOW(fd,6) = 1;
			WFIFOSET(fd,44);
			RFIFOSKIP(fd,49);
			break;

		// ÉLÉÉÉâñºåüçı
		case 0x2b08:
			if (RFIFOREST(fd) < 6)
				return 0;
			for(i = 0; i < char_num; i++) {
				if (char_dat[i].char_id == RFIFOL(fd,2))
					break;
			}
			WFIFOW(fd,0) = 0x2b09;
			WFIFOL(fd,2) = RFIFOL(fd,2);
			if (i != char_num)
				memcpy(WFIFOP(fd,6), char_dat[i].name, 24);
			else
				memcpy(WFIFOP(fd,6), unknown_char_name, 24);
			WFIFOSET(fd,30);
			RFIFOSKIP(fd,6);
			break;

		// it is a request to become GM
		case 0x2b0a:
			if (RFIFOREST(fd) < 4 || RFIFOREST(fd) < RFIFOW(fd,2))
				return 0;
//			printf("parse_frommap: change gm -> login, account: %d, pass: '%s'.\n", RFIFOL(fd,4), RFIFOP(fd,8));
			if (login_fd > 0) { // don't send request if no login-server
				WFIFOW(login_fd,0) = 0x2720;
				memcpy(WFIFOP(login_fd,2), RFIFOP(fd,2), RFIFOW(fd,2)-2);
				WFIFOSET(login_fd, RFIFOW(fd,2));
			} else {
				WFIFOW(fd,0) = 0x2b0b;
				WFIFOL(fd,2) = RFIFOL(fd,4);
				WFIFOL(fd,6) = 0;
				WFIFOSET(fd, 10);
			}
			RFIFOSKIP(fd, RFIFOW(fd,2));
			break;

		// Map server send information to change an email of an account -> login-server
		case 0x2b0c:
			if (RFIFOREST(fd) < 86)
				return 0;
			if (login_fd > 0) { // don't send request if no login-server
				memcpy(WFIFOP(login_fd,0), RFIFOP(fd,0), 86); // 0x2722 <account_id>.L <actual_e-mail>.40B <new_e-mail>.40B
				WFIFOW(login_fd,0) = 0x2722;
				WFIFOSET(login_fd, 86);
			}
			RFIFOSKIP(fd, 86);
			break;

		// Map server ask char-server about a character name to do some operations (all operations are transmitted to login-server)
		case 0x2b0e:
			if (RFIFOREST(fd) < 44)
				return 0;
		  {
			char character_name[24];
			int acc = RFIFOL(fd,2); // account_id of who ask (-1 if nobody)
			memcpy(character_name, RFIFOP(fd,6), 24);
			character_name[sizeof(character_name) -1] = '\0';
			// prepare answer
			WFIFOW(fd,0) = 0x2b0f; // answer
			WFIFOL(fd,2) = acc; // who want do operation
			WFIFOW(fd,30) = RFIFOW(fd, 30); // type of operation: 1-block, 2-ban, 3-unblock, 4-unban, 5-changesex
			// search character
			i = search_character_index(character_name);
			if (i >= 0) {
				memcpy(WFIFOP(fd,6), search_character_name(i), 24); // put correct name if found
				WFIFOW(fd,32) = 0; // answer: 0-login-server resquest done, 1-player not found, 2-gm level too low, 3-login-server offline
				switch(RFIFOW(fd, 30)) {
				case 1: // block
					if (acc == -1 || isGM(acc) >= isGM(char_dat[i].account_id)) {
						if (login_fd > 0) { // don't send request if no login-server
							WFIFOW(login_fd,0) = 0x2724;
							WFIFOL(login_fd,2) = char_dat[i].account_id; // account value
							WFIFOL(login_fd,6) = 5; // status of the account
							WFIFOSET(login_fd, 10);
//							printf("char : status -> login: account %d, status: %d \n", char_dat[i].account_id, 5);
						} else
							WFIFOW(fd,32) = 3; // answer: 0-login-server resquest done, 1-player not found, 2-gm level too low, 3-login-server offline
					} else
						WFIFOW(fd,32) = 2; // answer: 0-login-server resquest done, 1-player not found, 2-gm level too low, 3-login-server offline
					break;
				case 2: // ban
					if (acc == -1 || isGM(acc) >= isGM(char_dat[i].account_id)) {
						if (login_fd > 0) { // don't send request if no login-server
							WFIFOW(login_fd, 0) = 0x2725;
							WFIFOL(login_fd, 2) = char_dat[i].account_id; // account value
							WFIFOW(login_fd, 6) = RFIFOW(fd,32); // year
							WFIFOW(login_fd, 8) = RFIFOW(fd,34); // month
							WFIFOW(login_fd,10) = RFIFOW(fd,36); // day
							WFIFOW(login_fd,12) = RFIFOW(fd,38); // hour
							WFIFOW(login_fd,14) = RFIFOW(fd,40); // minute
							WFIFOW(login_fd,16) = RFIFOW(fd,42); // second
							WFIFOSET(login_fd,18);
//							printf("char : status -> login: account %d, ban: %dy %dm %dd %dh %dmn %ds\n",
//							       char_dat[i].account_id, (short)RFIFOW(fd,32), (short)RFIFOW(fd,34), (short)RFIFOW(fd,36), (short)RFIFOW(fd,38), (short)RFIFOW(fd,40), (short)RFIFOW(fd,42));
						} else
							WFIFOW(fd,32) = 3; // answer: 0-login-server resquest done, 1-player not found, 2-gm level too low, 3-login-server offline
					} else
						WFIFOW(fd,32) = 2; // answer: 0-login-server resquest done, 1-player not found, 2-gm level too low, 3-login-server offline
					break;
				case 3: // unblock
					if (acc == -1 || isGM(acc) >= isGM(char_dat[i].account_id)) {
						if (login_fd > 0) { // don't send request if no login-server
							WFIFOW(login_fd,0) = 0x2724;
							WFIFOL(login_fd,2) = char_dat[i].account_id; // account value
							WFIFOL(login_fd,6) = 0; // status of the account
							WFIFOSET(login_fd, 10);
//							printf("char : status -> login: account %d, status: %d \n", char_dat[i].account_id, 0);
						} else
							WFIFOW(fd,32) = 3; // answer: 0-login-server resquest done, 1-player not found, 2-gm level too low, 3-login-server offline
					} else
						WFIFOW(fd,32) = 2; // answer: 0-login-server resquest done, 1-player not found, 2-gm level too low, 3-login-server offline
					break;
				case 4: // unban
					if (acc == -1 || isGM(acc) >= isGM(char_dat[i].account_id)) {
						if (login_fd > 0) { // don't send request if no login-server
							WFIFOW(login_fd, 0) = 0x272a;
							WFIFOL(login_fd, 2) = char_dat[i].account_id; // account value
							WFIFOSET(login_fd, 6);
//							printf("char : status -> login: account %d, unban request\n", char_dat[i].account_id);
						} else
							WFIFOW(fd,32) = 3; // answer: 0-login-server resquest done, 1-player not found, 2-gm level too low, 3-login-server offline
					} else
						WFIFOW(fd,32) = 2; // answer: 0-login-server resquest done, 1-player not found, 2-gm level too low, 3-login-server offline
					break;
				case 5: // changesex
					if (acc == -1 || isGM(acc) >= isGM(char_dat[i].account_id)) {
						if (login_fd > 0) { // don't send request if no login-server
							WFIFOW(login_fd, 0) = 0x2727;
							WFIFOL(login_fd, 2) = char_dat[i].account_id; // account value
							WFIFOSET(login_fd, 6);
//							printf("char : status -> login: account %d, change sex request\n", char_dat[i].account_id);
						} else
							WFIFOW(fd,32) = 3; // answer: 0-login-server resquest done, 1-player not found, 2-gm level too low, 3-login-server offline
					} else
						WFIFOW(fd,32) = 2; // answer: 0-login-server resquest done, 1-player not found, 2-gm level too low, 3-login-server offline
					break;
				}
			} else {
				// character name not found
				memcpy(WFIFOP(fd,6), character_name, 24);
				WFIFOW(fd,32) = 1; // answer: 0-login-server resquest done, 1-player not found, 2-gm level too low, 3-login-server offline
			}
			// send answer if a player ask, not if the server ask
			if (acc != -1) {
				WFIFOSET(fd, 34);
			}
			RFIFOSKIP(fd, 44);
			break;
		  }

//		case 0x2b0f: not more used (available for futur usage)

		// account_regï€ë∂óvãÅ
		case 0x2b10:
			if (RFIFOREST(fd) < 4 || RFIFOREST(fd) < RFIFOW(fd,2))
				return 0;
		  {
			struct global_reg reg[ACCOUNT_REG2_NUM];
			int p, acc;
			acc = RFIFOL(fd,4);
			for(p = 8, j = 0; p < RFIFOW(fd,2) && j < ACCOUNT_REG2_NUM; p += 36, j++) {
				memcpy(reg[j].str, RFIFOP(fd,p), 32);
				reg[j].value = RFIFOL(fd, p+32);
			}
			set_account_reg2(acc, j, reg);
			// loginÉTÅ[ÉoÅ[Ç÷ëóÇÈ
			if (login_fd > 0) { // don't send request if no login-server
				WFIFOW(login_fd, 0) = 0x2728;
				memcpy(WFIFOP(login_fd,0), RFIFOP(fd,0), RFIFOW(fd,2));
				WFIFOSET(login_fd, WFIFOW(login_fd,2));
			}
			// ÉèÅ[ÉãÉhÇ÷ÇÃìØçCÉçÉOÉCÉìÇ™Ç»ÇØÇÍÇŒmapÉTÅ[ÉoÅ[Ç…ëóÇÈïKóvÇÕÇ»Ç¢
			//memcpy(buf, RFIFOP(fd,0), RFIFOW(fd,2));
			//WBUFW(buf,0) = 0x2b11;
			//mapif_sendall(buf, WBUFW(buf,2));
			RFIFOSKIP(fd, RFIFOW(fd,2));
//			printf("char: save_account_reg (from map)\n");
			break;
		}
		// Character disconnected set online 0 [Wizputer]
		case 0x2b17:
			if (RFIFOREST(fd) < 6)
				return 0;
			//printf("Setting %d char offline\n",RFIFOL(fd,2));
			set_char_offline(RFIFOL(fd,2),RFIFOL(fd,6));
			RFIFOSKIP(fd,10);
			break;

		// Character set online [Wizputer]
		case 0x2b19:
			if (RFIFOREST(fd) < 6)
				return 0;
			//printf("Setting %d char online\n",RFIFOL(fd,2));
			set_char_online(RFIFOL(fd,2),RFIFOL(fd,6));
			RFIFOSKIP(fd,10);
			break;

		default:
			// inter serverèàóùÇ…ìnÇ∑
			{
				int r = inter_parse_frommap(fd);
				if (r == 1) // èàóùÇ≈Ç´ÇΩ
					break;
				if (r == 2) // ÉpÉPÉbÉgí∑Ç™ë´ÇËÇ»Ç¢
					return 0;
			}
			// inter serverèàóùÇ≈Ç‡Ç»Ç¢èÍçáÇÕêÿíf
			printf("char: unknown packet 0x%04x (%d bytes to read in buffer)! (from map).\n", RFIFOW(fd,0), RFIFOREST(fd));
			session[fd]->eof = 1;
			return 0;
		}
	}
	return 0;
}

int search_mapserver(char *map) {
	int i, j;
	char temp_map[16];
	int temp_map_len;

//	printf("Searching the map-server for map '%s'... ", map);
	strncpy(temp_map, map, sizeof(temp_map));
	temp_map[sizeof(temp_map)-1] = '\0';
	if (strchr(temp_map, '.') != NULL)
		temp_map[strchr(temp_map, '.') - temp_map + 1] = '\0'; // suppress the '.gat', but conserve the '.' to be sure of the name of the map

	temp_map_len = strlen(temp_map);
	for(i = 0; i < MAX_MAP_SERVERS; i++)
		if (server_fd[i] >= 0)
			for (j = 0; server[i].map[j][0]; j++)
				//printf("%s : %s = %d\n", server[i].map[j], map, strncmp(server[i].map[j], temp_map, temp_map_len));
				if (strncmp(server[i].map[j], temp_map, temp_map_len) == 0) {
//					printf("found -> server #%d.\n", i);
					return i;
				}

//	printf("not found.\n");
	return -1;
}

// char_mapifÇÃèâä˙âªèàóùÅiåªç›ÇÕinter_mapifèâä˙âªÇÃÇ›Åj
static int char_mapif_init(int fd) {
	return inter_mapif_init(fd);
}

//-----------------------------------------------------
// Test to know if an IP come from LAN or WAN. by [Yor]
//-----------------------------------------------------
int lan_ip_check(unsigned char *p){
	int i;
	int lancheck = 1;

//	printf("lan_ip_check: to compare: %d.%d.%d.%d, network: %d.%d.%d.%d/%d.%d.%d.%d\n",
//	       p[0], p[1], p[2], p[3],
//	       subneti[0], subneti[1], subneti[2], subneti[3],
//	       subnetmaski[0], subnetmaski[1], subnetmaski[2], subnetmaski[3]);
	for(i = 0; i < 4; i++) {
		if ((subneti[i] & subnetmaski[i]) != (p[i] & subnetmaski[i])) {
			lancheck = 0;
			break;
		}
	}
	printf("LAN test (result): %s source\033[0m.\n", (lancheck) ? "\033[1;36mLAN" : "\033[1;32mWAN");
	return lancheck;
}

int parse_char(int fd) {
	int i, ch;
	unsigned short cmd;
	char email[40];
	struct char_session_data *sd;
	unsigned char *p = (unsigned char *) &session[fd]->client_addr.sin_addr;

	if (login_fd < 0)
		session[fd]->eof = 1;
	if(session[fd]->eof) { // disconnect any player (already connected to char-server or coming back from map-server) if login-server is diconnected.
		if (fd == login_fd)
			login_fd = -1;
		close(fd);
		delete_session(fd);
		return 0;
	}

	sd = (struct char_session_data*)session[fd]->session_data;

	while (RFIFOREST(fd) >= 2) {
		cmd = RFIFOW(fd,0);
		// crc32ÇÃÉXÉLÉbÉvóp
		if(	sd==NULL			&&	// ñ¢ÉçÉOÉCÉìorä«óùÉpÉPÉbÉg
			RFIFOREST(fd)>=4	&&	// ç≈í·ÉoÉCÉgêîêßå¿ Åï 0x7530,0x7532ä«óùÉpÉPèúãé
			RFIFOREST(fd)<=21	&&	// ç≈ëÂÉoÉCÉgêîêßå¿ Åï ÉTÅ[ÉoÅ[ÉçÉOÉCÉìèúãé
			cmd!=0x20b	&&	// md5í ímÉpÉPÉbÉgèúãé
			(RFIFOREST(fd)<6 || RFIFOW(fd,4)==0x65)	){	// éüÇ…âΩÇ©ÉpÉPÉbÉgÇ™óàÇƒÇÈÇ»ÇÁÅAê⁄ë±Ç≈Ç»Ç¢Ç∆ÇæÇﬂ
			RFIFOSKIP(fd,4);
			cmd = RFIFOW(fd,0);
			printf("parse_char : %d crc32 skipped\n",fd);
			if(RFIFOREST(fd)==0)
				return 0;
		}

//		if(cmd<30000 && cmd!=0x187)
//			printf("parse_char : %d %d %d\n",fd,RFIFOREST(fd),cmd);

		// ïsê≥ÉpÉPÉbÉgÇÃèàóù
//		if (sd == NULL && cmd != 0x65 && cmd != 0x20b && cmd != 0x187 &&
//					 cmd != 0x2af8 && cmd != 0x7530 && cmd != 0x7532)
//			cmd = 0xffff;	// ÉpÉPÉbÉgÉ_ÉìÉvÇï\é¶Ç≥ÇπÇÈ

		switch(cmd){
		case 0x20b:	//20040622à√çÜâªragexeëŒâû
			if (RFIFOREST(fd) < 19)
				return 0;
			RFIFOSKIP(fd,19);
			break;

		case 0x65:	// ê⁄ë±óvãÅ
			if (RFIFOREST(fd) < 17)
				return 0;
		  {
			int GM_value;
			if ((GM_value = isGM(RFIFOL(fd,2))))
				printf("Account Logged On; Account ID: %d (GM level %d).\n", RFIFOL(fd,2), GM_value);
			else
				printf("Account Logged On; Account ID: %d.\n", RFIFOL(fd,2));
			if (sd == NULL) {
				sd = (struct char_session_data*)aCalloc(sizeof(struct char_session_data), 1);
				session[fd]->session_data = sd;

				memset(sd, 0, sizeof(struct char_session_data));
				memcpy(sd->email, "no mail", 40); // put here a mail without '@' to refuse deletion if we don't receive the e-mail
				sd->connect_until_time = 0; // unknow or illimited (not displaying on map-server)
			}
			sd->account_id = RFIFOL(fd,2);
			sd->login_id1 = RFIFOL(fd,6);
			sd->login_id2 = RFIFOL(fd,10);
			sd->sex = RFIFOB(fd,16);
			// send back account_id
			WFIFOL(fd,0) = RFIFOL(fd,2);
			WFIFOSET(fd,4);
			// search authentification
			for(i = 0; i < AUTH_FIFO_SIZE; i++) {
				if (auth_fifo[i].account_id == sd->account_id &&
				    auth_fifo[i].login_id1 == sd->login_id1 &&
#if CMP_AUTHFIFO_LOGIN2 != 0
				    auth_fifo[i].login_id2 == sd->login_id2 && // relate to the versions higher than 18
#endif
				    (!check_ip_flag || auth_fifo[i].ip == session[fd]->client_addr.sin_addr.s_addr) &&
				    auth_fifo[i].delflag == 2) {
					auth_fifo[i].delflag = 1;
					if (max_connect_user == 0 || count_users() < max_connect_user) {
						if (login_fd > 0) { // don't send request if no login-server
							// request to login-server to obtain e-mail/time limit
							WFIFOW(login_fd,0) = 0x2716;
							WFIFOL(login_fd,2) = sd->account_id;
							WFIFOSET(login_fd,6);
						}
						// send characters to player
						mmo_char_send006b(fd, sd);
					} else {
						// refuse connection (over populated)
						WFIFOW(fd,0) = 0x6c;
						WFIFOW(fd,2) = 0;
						WFIFOSET(fd,3);
					}
					break;
				}
			}
			// authentification not found
			if (i == AUTH_FIFO_SIZE) {
				if (login_fd > 0) { // don't send request if no login-server
					WFIFOW(login_fd,0) = 0x2712; // ask login-server to authentify an account
					WFIFOL(login_fd,2) = sd->account_id;
					WFIFOL(login_fd,6) = sd->login_id1;
					WFIFOL(login_fd,10) = sd->login_id2; // relate to the versions higher than 18
					WFIFOB(login_fd,14) = sd->sex;
					WFIFOL(login_fd,15) = session[fd]->client_addr.sin_addr.s_addr;
					WFIFOSET(login_fd,19);
				} else { // if no login-server, we must refuse connection
					WFIFOW(fd,0) = 0x6c;
					WFIFOW(fd,2) = 0;
					WFIFOSET(fd,3);
				}
			}
		  }
			RFIFOSKIP(fd,17);
			break;

		case 0x66:	// ÉLÉÉÉâëIë
			if (RFIFOREST(fd) < 3)
				return 0;

			// if we activated email creation and email is default email
			if (email_creation != 0 && strcmp(sd->email, "a@a.com") == 0 && login_fd > 0) { // to modify an e-mail, login-server must be online
				WFIFOW(fd, 0) = 0x70;
				WFIFOB(fd, 2) = 0; // 00 = Incorrect Email address
				WFIFOSET(fd, 3);

			// otherwise, load the character
			} else {
				for (ch = 0; ch < 9; ch++)
					if (sd->found_char[ch] >= 0 && char_dat[sd->found_char[ch]].char_num == RFIFOB(fd,2))
						break;
				if (ch != 9) {
					char_log("Character Selected, Account ID: %d, Character Slot: %d, Character Name: %s." RETCODE,
					         sd->account_id, RFIFOB(fd,2), char_dat[sd->found_char[ch]].name);
					// searching map server
					i = search_mapserver(char_dat[sd->found_char[ch]].last_point.map);
					// if map is not found, we check major cities
					if (i < 0) {
						if ((i = search_mapserver("prontera.gat")) >= 0) { // check is done without 'gat'.
							memcpy(char_dat[sd->found_char[ch]].last_point.map, "prontera.gat", 16);
							char_dat[sd->found_char[ch]].last_point.x = 273; // savepoint coordonates
							char_dat[sd->found_char[ch]].last_point.y = 354;
						} else if ((i = search_mapserver("geffen.gat")) >= 0) { // check is done without 'gat'.
							memcpy(char_dat[sd->found_char[ch]].last_point.map, "geffen.gat", 16);
							char_dat[sd->found_char[ch]].last_point.x = 120; // savepoint coordonates
							char_dat[sd->found_char[ch]].last_point.y = 100;
						} else if ((i = search_mapserver("morocc.gat")) >= 0) { // check is done without 'gat'.
							memcpy(char_dat[sd->found_char[ch]].last_point.map, "morocc.gat", 16);
							char_dat[sd->found_char[ch]].last_point.x = 160; // savepoint coordonates
							char_dat[sd->found_char[ch]].last_point.y = 94;
						} else if ((i = search_mapserver("alberta.gat")) >= 0) { // check is done without 'gat'.
							memcpy(char_dat[sd->found_char[ch]].last_point.map, "alberta.gat", 16);
							char_dat[sd->found_char[ch]].last_point.x = 116; // savepoint coordonates
							char_dat[sd->found_char[ch]].last_point.y = 57;
						} else if ((i = search_mapserver("payon.gat")) >= 0) { // check is done without 'gat'.
							memcpy(char_dat[sd->found_char[ch]].last_point.map, "payon.gat", 16);
							char_dat[sd->found_char[ch]].last_point.x = 87; // savepoint coordonates
							char_dat[sd->found_char[ch]].last_point.y = 117;
						} else if ((i = search_mapserver("izlude.gat")) >= 0) { // check is done without 'gat'.
							memcpy(char_dat[sd->found_char[ch]].last_point.map, "izlude.gat", 16);
							char_dat[sd->found_char[ch]].last_point.x = 94; // savepoint coordonates
							char_dat[sd->found_char[ch]].last_point.y = 103;
						} else {
							int j;
							// get first online server (with a map)
							i = 0;
							for(j = 0; j < MAX_MAP_SERVERS; j++)
								if (server_fd[j] >= 0 && server[j].map[0][0]) { // change save point to one of map found on the server (the first)
									i = j;
									memcpy(char_dat[sd->found_char[ch]].last_point.map, server[j].map[0], 16);
									printf("Map-server #%d found with a map: '%s'.\n", j, server[j].map[0]);
									// coordonates are unknown
									break;
								}
							// if no map-server is connected, we send: server closed
							if (j == MAX_MAP_SERVERS) {
								WFIFOW(fd,0) = 0x81;
								WFIFOL(fd,2) = 1; // 01 = Server closed
								WFIFOSET(fd,3);
								RFIFOSKIP(fd,3);
								break;
							}
						}
					}
					WFIFOW(fd,0) = 0x71;
					WFIFOL(fd,2) = char_dat[sd->found_char[ch]].char_id;
					memcpy(WFIFOP(fd,6), char_dat[sd->found_char[ch]].last_point.map, 16);
					printf("Character selection '%s' (account: %d, slot: %d).\n", char_dat[sd->found_char[ch]].name, sd->account_id, ch);
					printf("--Send IP of map-server. ");
					if (lan_ip_check(p))
						WFIFOL(fd, 22) = inet_addr(lan_map_ip);
					else
						WFIFOL(fd, 22) = server[i].ip;
					WFIFOW(fd,26) = server[i].port;
					WFIFOSET(fd,28);
					if (auth_fifo_pos >= AUTH_FIFO_SIZE)
						auth_fifo_pos = 0;
					//printf("auth_fifo set #%d - account %d, char: %d, secure: %08x-%08x\n", auth_fifo_pos, sd->account_id, char_dat[sd->found_char[ch]].char_id, sd->login_id1, sd->login_id2);
					auth_fifo[auth_fifo_pos].account_id = sd->account_id;
					auth_fifo[auth_fifo_pos].char_id = char_dat[sd->found_char[ch]].char_id;
					auth_fifo[auth_fifo_pos].login_id1 = sd->login_id1;
					auth_fifo[auth_fifo_pos].login_id2 = sd->login_id2;
					auth_fifo[auth_fifo_pos].delflag = 0;
					auth_fifo[auth_fifo_pos].char_pos = sd->found_char[ch];
					auth_fifo[auth_fifo_pos].sex = sd->sex;
					auth_fifo[auth_fifo_pos].connect_until_time = sd->connect_until_time;
					auth_fifo[auth_fifo_pos].ip = session[fd]->client_addr.sin_addr.s_addr;
					auth_fifo_pos++;
				}
			}
			RFIFOSKIP(fd,3);
			break;

		case 0x67:	// çÏê¨
			if (RFIFOREST(fd) < 37)
				return 0;
			i = make_new_char(fd, RFIFOP(fd,2));
			if (i < 0) {
				WFIFOW(fd,0) = 0x6e;
				WFIFOB(fd,2) = 0x00;
				WFIFOSET(fd,3);
				RFIFOSKIP(fd,37);
				break;
			}

			WFIFOW(fd,0) = 0x6d;
			memset(WFIFOP(fd,2), 0, 106);

			WFIFOL(fd,2) = char_dat[i].char_id;
			WFIFOL(fd,2+4) = char_dat[i].base_exp;
			WFIFOL(fd,2+8) = char_dat[i].zeny;
			WFIFOL(fd,2+12) = char_dat[i].job_exp;
			WFIFOL(fd,2+16) = char_dat[i].job_level;

			WFIFOL(fd,2+28) = char_dat[i].karma;
			WFIFOL(fd,2+32) = char_dat[i].manner;

			WFIFOW(fd,2+40) = 0x30;
			WFIFOW(fd,2+42) = (char_dat[i].hp > 0x7fff) ? 0x7fff : char_dat[i].hp;
			WFIFOW(fd,2+44) = (char_dat[i].max_hp > 0x7fff) ? 0x7fff : char_dat[i].max_hp;
			WFIFOW(fd,2+46) = (char_dat[i].sp > 0x7fff) ? 0x7fff : char_dat[i].sp;
			WFIFOW(fd,2+48) = (char_dat[i].max_sp > 0x7fff) ? 0x7fff : char_dat[i].max_sp;
			WFIFOW(fd,2+50) = DEFAULT_WALK_SPEED; // char_dat[i].speed;
			WFIFOW(fd,2+52) = char_dat[i].class_;
			WFIFOW(fd,2+54) = char_dat[i].hair;

			WFIFOW(fd,2+58) = char_dat[i].base_level;
			WFIFOW(fd,2+60) = char_dat[i].skill_point;

			WFIFOW(fd,2+64) = char_dat[i].shield;
			WFIFOW(fd,2+66) = char_dat[i].head_top;
			WFIFOW(fd,2+68) = char_dat[i].head_mid;
			WFIFOW(fd,2+70) = char_dat[i].hair_color;

			memcpy(WFIFOP(fd,2+74), char_dat[i].name, 24);

			WFIFOB(fd,2+98) = (char_dat[i].str > 255) ? 255 : char_dat[i].str;
			WFIFOB(fd,2+99) = (char_dat[i].agi > 255) ? 255 : char_dat[i].agi;
			WFIFOB(fd,2+100) = (char_dat[i].vit > 255) ? 255 : char_dat[i].vit;
			WFIFOB(fd,2+101) = (char_dat[i].int_ > 255) ? 255 : char_dat[i].int_;
			WFIFOB(fd,2+102) = (char_dat[i].dex > 255) ? 255 : char_dat[i].dex;
			WFIFOB(fd,2+103) = (char_dat[i].luk > 255) ? 255 : char_dat[i].luk;
			WFIFOB(fd,2+104) = char_dat[i].char_num;

			WFIFOSET(fd,108);
			RFIFOSKIP(fd,37);
			for(ch = 0; ch < 9; ch++) {
				if (sd->found_char[ch] == -1) {
					sd->found_char[ch] = i;
					break;
				}
			}

		case 0x68:	// delete char //Yor's Fix
			if (RFIFOREST(fd) < 46)
				return 0;
			memcpy(email, RFIFOP(fd,6), 40);
			if (e_mail_check(email) == 0)
				strncpy(email, "a@a.com", 40); // default e-mail

			// if we activated email creation and email is default email
			if (email_creation != 0 && strcmp(sd->email, "a@a.com") == 0 && login_fd > 0) { // to modify an e-mail, login-server must be online
				// if sended email is incorrect e-mail
				if (strcmp(email, "a@a.com") == 0) {
					WFIFOW(fd, 0) = 0x70;
					WFIFOB(fd, 2) = 0; // 00 = Incorrect Email address
					WFIFOSET(fd, 3);
					RFIFOSKIP(fd,46);
				// we act like we have selected a character
				} else {
					// we change the packet to set it like selection.
					for (i = 0; i < 9; i++)
						if (char_dat[sd->found_char[i]].char_id == RFIFOL(fd,2)) {
							// we save new e-mail
							memcpy(sd->email, email, 40);
							// we send new e-mail to login-server ('online' login-server is checked before)
							WFIFOW(login_fd,0) = 0x2715;
							WFIFOL(login_fd,2) = sd->account_id;
							memcpy(WFIFOP(login_fd, 6), email, 40);
							WFIFOSET(login_fd,46);
							// skip part of the packet! (46, but leave the size of select packet: 3)
							RFIFOSKIP(fd,43);
							// change value to put new packet (char selection)
							RFIFOW(fd, 0) = 0x66;
							RFIFOB(fd, 2) = char_dat[sd->found_char[i]].char_num;
							// not send packet, it's modify of actual packet
							break;
						}
					if (i == 9) {
						WFIFOW(fd, 0) = 0x70;
						WFIFOB(fd, 2) = 0; // 00 = Incorrect Email address
						WFIFOSET(fd, 3);
						RFIFOSKIP(fd,46);
					}
				}

			// otherwise, we delete the character
			} else {
				if (strcmpi(email, sd->email) != 0) { // if it's an invalid email
					WFIFOW(fd, 0) = 0x70;
					WFIFOB(fd, 2) = 0; // 00 = Incorrect Email address
					WFIFOSET(fd, 3);
				// if mail is correct
				} else {
					for (i = 0; i < 9; i++) {
						struct mmo_charstatus *cs = NULL;
						if ((cs = &char_dat[sd->found_char[i]])->char_id == RFIFOL(fd,2)) {
							char_delete(cs); // deletion process

							if (sd->found_char[i] != char_num - 1) {
								memcpy(&char_dat[sd->found_char[i]], &char_dat[char_num-1], sizeof(struct mmo_charstatus));
								// Correct moved character reference in the character's owner
								{
									int j, k;
									struct char_session_data *sd2;
									for (j = 0; j < fd_max; j++) {
										if (session[j] && (sd2 = (struct char_session_data*)session[j]->session_data) &&
											sd2->account_id == char_dat[char_num-1].account_id) {
											for (k = 0; k < 9; k++) {
												if (sd2->found_char[k] == char_num-1) {
													sd2->found_char[k] = sd->found_char[i];
													break;
												}
											}
											break;
										}
									}
								}
							}

							char_num--;
							for(ch = i; ch < 9-1; ch++)
								sd->found_char[ch] = sd->found_char[ch+1];
							sd->found_char[8] = -1;
							WFIFOW(fd,0) = 0x6f;
							WFIFOSET(fd,2);
							break;
						}
					}

					if (i == 9) {
						WFIFOW(fd,0) = 0x70;
						WFIFOB(fd,2) = 0;
						WFIFOSET(fd,3);
					}
				}
				RFIFOSKIP(fd,46);
			}
			break;

		case 0x2af8:	// É}ÉbÉvÉTÅ[ÉoÅ[ÉçÉOÉCÉì
			if (RFIFOREST(fd) < 60)
				return 0;
			WFIFOW(fd,0) = 0x2af9;
			for(i = 0; i < MAX_MAP_SERVERS; i++) {
				if (server_fd[i] < 0)
					break;
			}
			if (i == MAX_MAP_SERVERS || strcmp((char*)RFIFOP(fd,2), userid) || strcmp((char*)RFIFOP(fd,26), passwd)){
				WFIFOB(fd,2) = 3;
				WFIFOSET(fd,3);
				RFIFOSKIP(fd,60);
			} else {
				int len;
				WFIFOB(fd,2) = 0;
				session[fd]->func_parse = parse_frommap;
				server_fd[i] = fd;
				server[i].ip = RFIFOL(fd,54);
				server[i].port = RFIFOW(fd,58);
				server[i].users = 0;
				memset(server[i].map, 0, sizeof(server[i].map));
				WFIFOSET(fd,3);
				RFIFOSKIP(fd,60);
				realloc_fifo(fd, FIFOSIZE_SERVERLINK, FIFOSIZE_SERVERLINK);
				char_mapif_init(fd);
				// send gm acccounts level to map-servers
				len = 4;
				WFIFOW(fd,0) = 0x2b15;
				for(i = 0; i < GM_num; i++) {
					WFIFOL(fd,len) = gm_account[i].account_id;
					WFIFOB(fd,len+4) = (unsigned char)gm_account[i].level;
					len += 5;
				}
				WFIFOW(fd,2) = len;
				WFIFOSET(fd,len);
				return 0;
			}
			break;

		case 0x187:	// AliveêMçÜÅH
			if (RFIFOREST(fd) < 6)
				return 0;
			RFIFOSKIP(fd, 6);
			break;

		case 0x7530:	// AthenaèÓïÒèäìæ
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
			return 0;

		case 0x7532:	// ê⁄ë±ÇÃêÿíf(defaultÇ∆èàóùÇÕàÍèèÇæÇ™ñæé¶ìIÇ…Ç∑ÇÈÇΩÇﬂ)
			session[fd]->eof = 1;
			return 0;

		default:
			session[fd]->eof = 1;
			return 0;
		}
	}
	RFIFOFLUSH(fd);
	return 0;
}

// Console Command Parser [Wizputer]
int parse_console(char *buf) {
    char *type,*command;

    type = (char *)aMalloc(64);
    command = (char *)aMalloc(64);

    memset(type,0,64);
    memset(command,0,64);

    printf("Console: %s\n",buf);

    if ( sscanf(buf, "%[^:]:%[^\n]", type , command ) < 2 )
        sscanf(buf,"%[^\n]",type);

    printf("Type of command: %s || Command: %s \n",type,command);

    if(buf) aFree(buf);
    if(type) aFree(type);
    if(command) aFree(command);

    return 0;
}

// ëSÇƒÇÃMAPÉTÅ[ÉoÅ[Ç…ÉfÅ[É^ëóêMÅiëóêMÇµÇΩmapéIÇÃêîÇï‘Ç∑Åj
int mapif_sendall(unsigned char *buf, unsigned int len) {
	int i, c;

	c = 0;
	for(i = 0; i < MAX_MAP_SERVERS; i++) {
		int fd;
		if ((fd = server_fd[i]) >= 0) {
			memcpy(WFIFOP(fd,0), buf, len);
			WFIFOSET(fd,len);
			c++;
		}
	}
	return c;
}

// é©ï™à»äOÇÃëSÇƒÇÃMAPÉTÅ[ÉoÅ[Ç…ÉfÅ[É^ëóêMÅiëóêMÇµÇΩmapéIÇÃêîÇï‘Ç∑Åj
int mapif_sendallwos(int sfd, unsigned char *buf, unsigned int len) {
	int i, c;

	c = 0;
	for(i = 0; i < MAX_MAP_SERVERS; i++) {
		int fd;
		if ((fd = server_fd[i]) >= 0 && fd != sfd) {
			memcpy(WFIFOP(fd,0), buf, len);
			WFIFOSET(fd, len);
			c++;
		}
	}
	return c;
}
// MAPÉTÅ[ÉoÅ[Ç…ÉfÅ[É^ëóêMÅimapéIê∂ë∂ämîFóLÇËÅj
int mapif_send(int fd, unsigned char *buf, unsigned int len) {
	int i;

	if (fd >= 0) {
		for(i = 0; i < MAX_MAP_SERVERS; i++) {
			if (fd == server_fd[i]) {
				memcpy(WFIFOP(fd,0), buf, len);
				WFIFOSET(fd,len);
				return 1;
			}
		}
	}
	return 0;
}

int send_users_tologin(int tid, unsigned int tick, int id, int data) {
	int users = count_users();
	unsigned char buf[16];

	if (login_fd > 0 && session[login_fd]) {
		// send number of user to login server
		WFIFOW(login_fd,0) = 0x2714;
		WFIFOL(login_fd,2) = users;
		WFIFOSET(login_fd,6);
	}
	// send number of players to all map-servers
	WBUFW(buf,0) = 0x2b00;
	WBUFL(buf,2) = users;
	mapif_sendall(buf, 6);

	return 0;
}

int check_connect_login_server(int tid, unsigned int tick, int id, int data) {
	if (login_fd <= 0 || session[login_fd] == NULL) {
		printf("Attempt to connect to login-server...\n");
		login_fd = make_connection(login_ip, login_port);
		session[login_fd]->func_parse = parse_tologin;
		realloc_fifo(login_fd, FIFOSIZE_SERVERLINK, FIFOSIZE_SERVERLINK);
		WFIFOW(login_fd,0) = 0x2710;
		memset(WFIFOP(login_fd,2), 0, 24);
		memcpy(WFIFOP(login_fd,2), userid, strlen(userid) < 24 ? strlen(userid) : 24);
		memset(WFIFOP(login_fd,26), 0, 24);
		memcpy(WFIFOP(login_fd,26), passwd, strlen(passwd) < 24 ? strlen(passwd) : 24);
		WFIFOL(login_fd,50) = 0;
		WFIFOL(login_fd,54) = char_ip;
		WFIFOL(login_fd,58) = char_port;
		memset(WFIFOP(login_fd,60), 0, 20);
		memcpy(WFIFOP(login_fd,60), server_name, strlen(server_name) < 20 ? strlen(server_name) : 20);
		WFIFOW(login_fd,80) = 0;
		WFIFOW(login_fd,82) = char_maintenance;
		WFIFOW(login_fd,84) = char_new;
		WFIFOSET(login_fd,86);
	}
	return 0;
}

//----------------------------------------------------------
// Return numerical value of a switch configuration by [Yor]
// on/off, english, franÁais, deutsch, espaÒol
//----------------------------------------------------------
int config_switch(const char *str) {
	if (strcmpi(str, "on") == 0 || strcmpi(str, "yes") == 0 || strcmpi(str, "oui") == 0 || strcmpi(str, "ja") == 0 || strcmpi(str, "si") == 0)
		return 1;
	if (strcmpi(str, "off") == 0 || strcmpi(str, "no") == 0 || strcmpi(str, "non") == 0 || strcmpi(str, "nein") == 0)
		return 0;

	return atoi(str);
}

//-------------------------------------------
// Reading Lan Support configuration by [Yor]
//-------------------------------------------
int lan_config_read(const char *lancfgName) {
	int j;
	struct hostent * h = NULL;
	char line[1024], w1[1024], w2[1024];
	FILE *fp;

	// set default configuration
	strncpy(lan_map_ip, "127.0.0.1", sizeof(lan_map_ip));
	subneti[0] = 127;
	subneti[1] = 0;
	subneti[2] = 0;
	subneti[3] = 1;
	for(j = 0; j < 4; j++)
		subnetmaski[j] = 255;

	fp = fopen(lancfgName, "r");

	if (fp == NULL) {
		printf("LAN support configuration file not found: %s\n", lancfgName);
		return 1;
	}

	printf ("---start reading of Lan Support configuration...\n");

	while(fgets(line, sizeof(line)-1, fp)) {
		if (line[0] == '/' && line[1] == '/')
			continue;

		line[sizeof(line)-1] = '\0';
		if (sscanf(line, "%[^:]: %[^\r\n]", w1, w2) != 2)
			continue;

		remove_control_chars((unsigned char *)w1);
		remove_control_chars((unsigned char *)w2);
		if (strcmpi(w1, "lan_map_ip") == 0) { // Read map-server Lan IP Address
			h = gethostbyname(w2);
			if (h != NULL) {
				sprintf(lan_map_ip, "%d.%d.%d.%d", (unsigned char)h->h_addr[0], (unsigned char)h->h_addr[1], (unsigned char)h->h_addr[2], (unsigned char)h->h_addr[3]);
			} else {
				strncpy(lan_map_ip, w2, sizeof(lan_map_ip));
				lan_map_ip[sizeof(lan_map_ip)-1] = 0;
			}
			printf("LAN IP of map-server: %s.\n", lan_map_ip);
		} else if (strcmpi(w1, "subnet") == 0) { // Read Subnetwork
			for(j = 0; j < 4; j++)
				subneti[j] = 0;
			h = gethostbyname(w2);
			if (h != NULL) {
				for(j = 0; j < 4; j++)
					subneti[j] = (unsigned char)h->h_addr[j];
			} else {
				sscanf(w2, "%d.%d.%d.%d", &subneti[0], &subneti[1], &subneti[2], &subneti[3]);
			}
			printf("Sub-network of the map-server: %d.%d.%d.%d.\n", subneti[0], subneti[1], subneti[2], subneti[3]);
		} else if (strcmpi(w1, "subnetmask") == 0){ // Read Subnetwork Mask
			for(j = 0; j < 4; j++)
				subnetmaski[j] = 255;
			h = gethostbyname(w2);
			if (h != NULL) {
				for(j = 0; j < 4; j++)
					subnetmaski[j] = (unsigned char)h->h_addr[j];
			} else {
				sscanf(w2, "%d.%d.%d.%d", &subnetmaski[0], &subnetmaski[1], &subnetmaski[2], &subnetmaski[3]);
			}
			printf("Sub-network mask of the map-server: %d.%d.%d.%d.\n", subnetmaski[0], subnetmaski[1], subnetmaski[2], subnetmaski[3]);
		}
	}
	fclose(fp);

	// sub-network check of the map-server
	{
		unsigned int a0, a1, a2, a3;
		unsigned char p[4];
		sscanf(lan_map_ip, "%d.%d.%d.%d", &a0, &a1, &a2, &a3);
		p[0] = a0; p[1] = a1; p[2] = a2; p[3] = a3;
		printf("LAN test of LAN IP of the map-server: ");
		if (lan_ip_check(p) == 0) {
			printf("\033[1;31m***ERROR: LAN IP of the map-server doesn't belong to the specified Sub-network.\033[0m\n");
		}
	}

	printf("---End reading of Lan Support configuration...\n");

	return 0;
}

int char_config_read(const char *cfgName) {
	struct hostent *h = NULL;
	char line[1024], w1[1024], w2[1024];
	FILE *fp = fopen(cfgName, "r");

	if (fp == NULL) {
		printf("Configuration file not found: %s.\n", cfgName);
		exit(1);
	}

	while(fgets(line, sizeof(line)-1, fp)) {
		if (line[0] == '/' && line[1] == '/')
			continue;

		line[sizeof(line)-1] = '\0';
		if (sscanf(line, "%[^:]: %[^\r\n]", w1, w2) != 2)
			continue;

		remove_control_chars((unsigned char *)w1);
		remove_control_chars((unsigned char *)w2);
		if (strcmpi(w1, "userid") == 0) {
			memcpy(userid, w2, 24);
		} else if (strcmpi(w1, "passwd") == 0) {
			memcpy(passwd, w2, 24);
		} else if (strcmpi(w1, "server_name") == 0) {
			memcpy(server_name, w2, sizeof(server_name));
			server_name[sizeof(server_name) - 1] = '\0';
			printf("%s server has been initialized\n", w2);
		} else if (strcmpi(w1, "wisp_server_name") == 0) {
			if (strlen(w2) >= 4) {
				memcpy(wisp_server_name, w2, sizeof(wisp_server_name));
				wisp_server_name[sizeof(wisp_server_name) - 1] = '\0';
			}
		} else if (strcmpi(w1, "login_ip") == 0) {
			login_ip_set_ = 1;
			h = gethostbyname(w2);
			if (h != NULL) {
				printf("Login server IP address : %s -> %d.%d.%d.%d\n", w2, (unsigned char)h->h_addr[0], (unsigned char)h->h_addr[1], (unsigned char)h->h_addr[2], (unsigned char)h->h_addr[3]);
				sprintf(login_ip_str, "%d.%d.%d.%d", (unsigned char)h->h_addr[0], (unsigned char)h->h_addr[1], (unsigned char)h->h_addr[2], (unsigned char)h->h_addr[3]);
			} else
				memcpy(login_ip_str, w2, 16);
		} else if (strcmpi(w1, "login_port") == 0) {
			login_port = atoi(w2);
		} else if (strcmpi(w1, "char_ip") == 0) {
			char_ip_set_ = 1;
			h = gethostbyname(w2);
			if (h != NULL) {
				printf("Character server IP address : %s -> %d.%d.%d.%d\n", w2, (unsigned char)h->h_addr[0], (unsigned char)h->h_addr[1], (unsigned char)h->h_addr[2], (unsigned char)h->h_addr[3]);
				sprintf(char_ip_str, "%d.%d.%d.%d", (unsigned char)h->h_addr[0], (unsigned char)h->h_addr[1], (unsigned char)h->h_addr[2], (unsigned char)h->h_addr[3]);
			} else
				memcpy(char_ip_str, w2, 16);
		} else if (strcmpi(w1, "bind_ip") == 0) {
			bind_ip_set_ = 1;
			h = gethostbyname(w2);
			if (h != NULL) {
				printf("Character server binding IP address : %s -> %d.%d.%d.%d\n", w2, (unsigned char)h->h_addr[0], (unsigned char)h->h_addr[1], (unsigned char)h->h_addr[2], (unsigned char)h->h_addr[3]);
				sprintf(bind_ip_str, "%d.%d.%d.%d", (unsigned char)h->h_addr[0], (unsigned char)h->h_addr[1], (unsigned char)h->h_addr[2], (unsigned char)h->h_addr[3]);
			} else
				memcpy(bind_ip_str, w2, 16);
		} else if (strcmpi(w1, "char_port") == 0) {
			char_port = atoi(w2);
		} else if (strcmpi(w1, "char_maintenance") == 0) {
			char_maintenance = atoi(w2);
		} else if (strcmpi(w1, "char_new") == 0) {
			char_new = atoi(w2);
		} else if (strcmpi(w1, "email_creation") == 0) {
			email_creation = config_switch(w2);
		} else if (strcmpi(w1, "char_txt") == 0) {
			strcpy(char_txt, w2);
		} else if (strcmpi(w1, "backup_txt") == 0) { //By zanetheinsane
			strcpy(backup_txt, w2);
		} else if (strcmpi(w1, "friends_txt") == 0) { //By davidsiaw
			strcpy(friends_txt, w2);
		} else if (strcmpi(w1, "backup_txt_flag") == 0) { // The backup_txt file was created because char deletion bug existed. Now it's finish and that take a lot of time to create a second file when there are a lot of characters. By [Yor]
			backup_txt_flag = config_switch(w2);
		} else if (strcmpi(w1, "max_connect_user") == 0) {
			max_connect_user = atoi(w2);
			if (max_connect_user < 0)
				max_connect_user = 0; // unlimited online players
		} else if(strcmpi(w1, "gm_allow_level") == 0) {
			gm_allow_level = atoi(w2);
			if(gm_allow_level < 0)
				gm_allow_level = 99;
		} else if (strcmpi(w1, "check_ip_flag") == 0) {
			check_ip_flag = config_switch(w2);
		} else if (strcmpi(w1, "autosave_time") == 0) {
			autosave_interval = atoi(w2)*1000;
			if (autosave_interval <= 0)
				autosave_interval = DEFAULT_AUTOSAVE_INTERVAL;
		} else if (strcmpi(w1, "start_point") == 0) {
			char map[32];
			int x, y;
			if (sscanf(w2, "%[^,],%d,%d", map, &x, &y) < 3)
				continue;
			if (strstr(map, ".gat") != NULL) { // Verify at least if '.gat' is in the map name
				memcpy(start_point.map, map, 16);
				start_point.x = x;
				start_point.y = y;
			}
		} else if(strcmpi(w1,"imalive_on")==0) {	//Added by Mugendai for I'm Alive mod
			imalive_on = atoi(w2);			//Added by Mugendai for I'm Alive mod
		} else if(strcmpi(w1,"imalive_time")==0) {	//Added by Mugendai for I'm Alive mod
			imalive_time = atoi(w2);		//Added by Mugendai for I'm Alive mod
		} else if(strcmpi(w1,"flush_on")==0) {		//Added by Mugendai for GUI
			flush_on = atoi(w2);			//Added by Mugendai for GUI
		} else if(strcmpi(w1,"flush_time")==0) {	//Added by Mugendai for GUI
			flush_time = atoi(w2);			//Added by Mugendai for GUI
		} else if(strcmpi(w1,"log_char")==0) {		//log char or not [devil]
			log_char = atoi(w2);
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
		} else if (strcmpi(w1, "unknown_char_name") == 0) {
			strcpy(unknown_char_name, w2);
			unknown_char_name[24] = 0;
		} else if (strcmpi(w1, "char_log_filename") == 0) {
			strcpy(char_log_filename, w2);
		} else if (strcmpi(w1, "name_ignoring_case") == 0) {
			name_ignoring_case = config_switch(w2);
		} else if (strcmpi(w1, "char_name_option") == 0) {
			char_name_option = atoi(w2);
		} else if (strcmpi(w1, "char_name_letters") == 0) {
			strcpy(char_name_letters, w2);
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
		} else if (strcmpi(w1, "import") == 0) {
			char_config_read(w2);
		} else if (strcmpi(w1, "console") == 0) {
			    if(strcmpi(w2,"on") == 0 || strcmpi(w2,"yes") == 0 )
			        console = 1;
        }
	}
	fclose(fp);

	return 0;
}

//-----------------------------------------------------
//I'm Alive Alert
//Used to output 'I'm Alive' every few seconds
//Intended to let frontends know if the app froze
//-----------------------------------------------------
int imalive_timer(int tid, unsigned int tick, int id, int data){
	printf("I'm Alive\n");
	return 0;
}

//-----------------------------------------------------
//Flush stdout
//stdout buffer needs flushed to be seen in GUI
//-----------------------------------------------------
int flush_timer(int tid, unsigned int tick, int id, int data){
	fflush(stdout);
	return 0;
}

void do_final(void) {
	int i;
	printf("Terminating server.\n");
	// write online players files with no player
	for(i = 0; i < online_players_max; i++) {
		online_chars[i].char_id = -1;
		online_chars[i].server = -1;
	}

	create_online_files();
	if(online_chars) aFree(online_chars);

	mmo_char_sync();
	inter_save();

	if(gm_account) aFree(gm_account);
	if(char_dat) aFree(char_dat);

	delete_session(login_fd);
	delete_session(char_fd);

	inter_final();
	exit_dbn();
	timer_final();

	char_log("----End of char-server (normal end with closing of all files)." RETCODE);
}

int do_init(int argc, char **argv) {
	int i;

	SERVER_TYPE = SERVER_CHAR;
	char_config_read((argc < 2) ? CHAR_CONF_NAME : argv[1]);
	lan_config_read((argc > 1) ? argv[1] : LOGIN_LAN_CONF_NAME);

		// a newline in the log...
		char_log("");
		// moved behind char_config_read in case we changed the filename [celest]
		char_log("The char-server starting..." RETCODE);

        if ((naddr_ != 0) && (login_ip_set_ == 0 || char_ip_set_ == 0)) {
          // The char server should know what IP address it is running on
          //   - MouseJstr
          int localaddr = ntohl(addr_[0]);
          unsigned char *ptr = (unsigned char *) &localaddr;
          char buf[16];
          sprintf(buf, "%d.%d.%d.%d", ptr[0], ptr[1], ptr[2], ptr[3]);;
          if (naddr_ != 1)
            printf("Multiple interfaces detected..  using %s as our IP address\n", buf);
          else
            printf("Defaulting to %s as our IP address\n", buf);
          if (login_ip_set_ == 0)
          	strcpy(login_ip_str, buf);
          if (char_ip_set_ == 0)
          	strcpy(char_ip_str, buf);

          if (ptr[0] == 192 && ptr[1] == 168)
            printf("Firewall detected.. edit lan_support.conf and char_athena.conf\n");
        }

	login_ip = inet_addr(login_ip_str);
	char_ip = inet_addr(char_ip_str);

	for(i = 0; i < MAX_MAP_SERVERS; i++) {
		memset(&server[i], 0, sizeof(struct mmo_map_server));
		server_fd[i] = -1;
	}

	online_players_max = 256;
	online_chars = (struct online_chars*)aCalloc(sizeof(struct online_chars) * 256, 1);
	if (!online_chars) {
		printf("out of memory: do_init (calloc).\n");
		exit(1);
	}
	for(i = 0; i < online_players_max; i++) {
		online_chars[i].char_id = -1;
		online_chars[i].server = -1;
	}

	mmo_char_init();

	update_online = time(NULL);
	create_online_files(); // update online players files at start of the server

	inter_init((argc > 2) ? argv[2] : inter_cfgName);	// inter server èâä˙âª

	set_termfunc(do_final);
	set_defaultparse(parse_char);

        if (bind_ip_set_)
            char_fd = make_listen_bind(inet_addr(bind_ip_str),char_port);
        else
            char_fd = make_listen_bind(INADDR_ANY,char_port);

	add_timer_func_list(check_connect_login_server, "check_connect_login_server");
	add_timer_func_list(send_users_tologin, "send_users_tologin");
	add_timer_func_list(mmo_char_sync_timer, "mmo_char_sync_timer");

	i = add_timer_interval(gettick() + 1000, check_connect_login_server, 0, 0, 10 * 1000);
	i = add_timer_interval(gettick() + 1000, send_users_tologin, 0, 0, 5 * 1000);
	i = add_timer_interval(gettick() + autosave_interval, mmo_char_sync_timer, 0, 0, autosave_interval);

	//Added for Mugendais I'm Alive mod
	if (imalive_on)
		add_timer_interval(gettick()+10, imalive_timer,0,0,imalive_time*1000);

	//Added by Mugendai for GUI support
	if (flush_on)
		add_timer_interval(gettick()+10, flush_timer,0,0,flush_time);

	if(console) {
	    set_defaultconsoleparse(parse_console);
	   	start_console();
	}

	char_log("The char-server is ready (Server is listening on the port %d)." RETCODE, char_port);

	printf("The char-server is \033[1;32mready\033[0m (Server is listening on the port %d).\n\n", char_port);

	return 0;
}

int char_married(int pl1,int pl2) {
	if (char_dat[pl1].char_id == char_dat[pl2].partner_id && char_dat[pl2].char_id == char_dat[pl1].partner_id)
		return 1;
	else 
		return 0;
}

int char_child(int parent_id, int child_id) {
	if (char_dat[parent_id].child == char_dat[child_id].char_id && 
	((char_dat[parent_id].char_id == char_dat[child_id].father) || 
	(char_dat[parent_id].char_id == char_dat[child_id].mother)))
		return 1;
	else
		return 0;
}
