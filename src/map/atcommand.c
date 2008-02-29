// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/cbasetypes.h"
#include "../common/mmo.h"
#include "../common/timer.h"
#include "../common/nullpo.h"
#include "../common/core.h"
#include "../common/showmsg.h"
#include "../common/malloc.h"
#include "../common/socket.h"
#include "../common/strlib.h"
#include "../common/utils.h"

#include "atcommand.h"
#include "battle.h"
#include "clif.h"
#include "chrif.h"
#include "intif.h"
#include "itemdb.h"
#include "log.h"
#include "map.h"
#include "pc.h"
#include "status.h"
#include "skill.h"
#include "mob.h"
#include "npc.h"
#include "pet.h"
#include "mercenary.h"
#include "party.h"
#include "guild.h"
#include "script.h"
#include "trade.h"
#include "unit.h"

#ifndef TXT_ONLY
#include "mail.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// extern variables
char atcommand_symbol = '@'; // first char of the commands
char* msg_table[MAX_MSG]; // Server messages (0-499 reserved for GM commands, 500-999 reserved for others)

// local declarations
#define ACMD_FUNC(x) int atcommand_ ## x (const int fd, struct map_session_data* sd, const char* command, const char* message)

typedef struct AtCommandInfo
{
	const char* command;
	int level;
	AtCommandFunc func;
} AtCommandInfo;

static AtCommandInfo* get_atcommandinfo_byname(const char* name);
static AtCommandInfo* get_atcommandinfo_byfunc(const AtCommandFunc func);

int atcommand_commands(const int fd, struct map_session_data* sd, const char* command, const char* message);


/*=========================================
 * Generic variables
 *-----------------------------------------*/
char atcmd_output[200];
char atcmd_player_name[NAME_LENGTH];
char atcmd_temp[100];

// compare function for sorting high to lowest
int hightolow_compare (const void * a, const void * b)
{
	return ( *(int*)b - *(int*)a );
}

// compare function for sorting lowest to highest
int lowtohigh_compare (const void * a, const void * b)
{
	return ( *(int*)a - *(int*)b );
}

//-----------------------------------------------------------
// Return the message string of the specified number by [Yor]
//-----------------------------------------------------------
char* msg_txt(int msg_number)
{
	if (msg_number >= 0 && msg_number < MAX_MSG &&
	    msg_table[msg_number] != NULL && msg_table[msg_number][0] != '\0')
		return msg_table[msg_number];

	return "??";
}

//-----------------------------------------------------------
// Returns Players title (from msg_athena.conf) [Lupus]
//-----------------------------------------------------------
static char* player_title_txt(int level)
{
	const char* format;
	format = (level >= battle_config.title_lvl8) ? msg_txt(332)
	       : (level >= battle_config.title_lvl7) ? msg_txt(331)
	       : (level >= battle_config.title_lvl6) ? msg_txt(330)
	       : (level >= battle_config.title_lvl5) ? msg_txt(329)
	       : (level >= battle_config.title_lvl4) ? msg_txt(328)
	       : (level >= battle_config.title_lvl3) ? msg_txt(327)
	       : (level >= battle_config.title_lvl2) ? msg_txt(326)
	       : (level >= battle_config.title_lvl1) ? msg_txt(325)
	       : "";
	sprintf(atcmd_temp, format, level);
	return atcmd_temp;
}


/*==========================================
 * Read Message Data
 *------------------------------------------*/
int msg_config_read(const char* cfgName)
{
	int msg_number;
	char line[1024], w1[1024], w2[1024];
	FILE *fp;
	static int called = 1;

	if ((fp = fopen(cfgName, "r")) == NULL) {
		ShowError("Messages file not found: %s\n", cfgName);
		return 1;
	}

	if ((--called) == 0)
		memset(msg_table, 0, sizeof(msg_table[0]) * MAX_MSG);

	while(fgets(line, sizeof(line), fp))
	{
		if (line[0] == '/' && line[1] == '/')
			continue;
		if (sscanf(line, "%[^:]: %[^\r\n]", w1, w2) != 2)
			continue;

		if (strcmpi(w1, "import") == 0)
			msg_config_read(w2);
		else
		{
			msg_number = atoi(w1);
			if (msg_number >= 0 && msg_number < MAX_MSG)
			{
				if (msg_table[msg_number] != NULL)
					aFree(msg_table[msg_number]);
				msg_table[msg_number] = (char *)aMalloc((strlen(w2) + 1)*sizeof (char));
				strcpy(msg_table[msg_number],w2);
			}
		}
	}

	fclose(fp);

	return 0;
}

/*==========================================
 * Cleanup Message Data
 *------------------------------------------*/
void do_final_msg(void)
{
	int i;
	for (i = 0; i < MAX_MSG; i++)
		aFree(msg_table[i]);
}


/*==========================================
 * @send (used for testing packet sends from the client)
 *------------------------------------------*/
int atcommand_send(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	int len=0,off,end,type;
	long num;
	(void)command; // not used

	// read message type as hex number (without the 0x)
	if(!message || !*message ||
			!((sscanf(message, "len %x", &type)==1 && (len=1))
			|| sscanf(message, "%x", &type)==1) )
	{
		clif_displaymessage(fd, "Usage:");
		clif_displaymessage(fd, "	@send len <packet hex number>");
		clif_displaymessage(fd, "	@send <packet hex number> {<value>}*");
		clif_displaymessage(fd, "	Value: <type=B(default),W,L><number> or S<length>\"<string>\"");
		return -1;
	}

#define PARSE_ERROR(error,p) \
	{\
		clif_displaymessage(fd, (error));\
		sprintf(atcmd_output, ">%s", (p));\
		clif_displaymessage(fd, atcmd_output);\
	}
//define PARSE_ERROR

#define CHECK_EOS(p) \
	if(*(p) == 0){\
		clif_displaymessage(fd, "Unexpected end of string");\
		return -1;\
	}
//define CHECK_EOS

#define SKIP_VALUE(p) \
	{\
		while(*(p) && !ISSPACE(*(p))) ++(p); /* non-space */\
		while(*(p) && ISSPACE(*(p)))  ++(p); /* space */\
	}
//define SKIP_VALUE

#define GET_VALUE(p,num) \
	{\
		if(sscanf((p), "x%lx", &(num)) < 1 && sscanf((p), "%ld ", &(num)) < 1){\
			PARSE_ERROR("Invalid number in:",(p));\
			return -1;\
		}\
	}
//define GET_VALUE

	if (type > 0 && type < MAX_PACKET_DB) {

		if(len)
		{// show packet length
			sprintf(atcmd_output, "Packet 0x%x length: %d", type, packet_db[sd->packet_ver][type].len);
			clif_displaymessage(fd, atcmd_output);
			return 0;
		}

		len=packet_db[sd->packet_ver][type].len;
		off=2;
		if(len == 0)
		{// unknown packet - ERROR
			sprintf(atcmd_output, "Unknown packet: 0x%x", type);
			clif_displaymessage(fd, atcmd_output);
			return -1;
		} else if(len == -1)
		{// dynamic packet
			len=SHRT_MAX-4; // maximum length
			off=4;
		}
		WFIFOHEAD(fd, len);
		WFIFOW(fd,0)=TOW(type);

		// parse packet contents
		SKIP_VALUE(message);
		while(*message != 0 && off < len){
			if(ISDIGIT(*message) || *message == '-' || *message == '+')
			{// default (byte)
				GET_VALUE(message,num);
				WFIFOB(fd,off)=TOB(num);
				++off;
			} else if(TOUPPER(*message) == 'B')
			{// byte
				++message;
				GET_VALUE(message,num);
				WFIFOB(fd,off)=TOB(num);
				++off;
			} else if(TOUPPER(*message) == 'W')
			{// word (2 bytes)
				++message;
				GET_VALUE(message,num);
				WFIFOW(fd,off)=TOW(num);
				off+=2;
			} else if(TOUPPER(*message) == 'L')
			{// long word (4 bytes)
				++message;
				GET_VALUE(message,num);
				WFIFOL(fd,off)=TOL(num);
				off+=4;
			} else if(TOUPPER(*message) == 'S')
			{// string - escapes are valid
				// get string length - num <= 0 means not fixed length (default)
				++message;
				if(*message == '"'){
					num=0;
				} else {
					GET_VALUE(message,num);
					while(*message != '"')
					{// find start of string
						if(*message == 0 || ISSPACE(*message)){
							PARSE_ERROR("Not a string:",message);
							return -1;
						}
						++message;
					}
				}

				// parse string
				++message;
				CHECK_EOS(message);
				end=(num<=0? 0: min(off+((int)num),len));
				for(; *message != '"' && (off < end || end == 0); ++off){
					if(*message == '\\'){
						++message;
						CHECK_EOS(message);
						switch(*message){
							case 'a': num=0x07; break; // Bell
							case 'b': num=0x08; break; // Backspace
							case 't': num=0x09; break; // Horizontal tab
							case 'n': num=0x0A; break; // Line feed
							case 'v': num=0x0B; break; // Vertical tab
							case 'f': num=0x0C; break; // Form feed
							case 'r': num=0x0D; break; // Carriage return
							case 'e': num=0x1B; break; // Escape
							default:  num=*message; break;
							case 'x': // Hexadecimal
							{
								++message;
								CHECK_EOS(message);
								if(!ISXDIGIT(*message)){
									PARSE_ERROR("Not a hexadecimal digit:",message);
									return -1;
								}
								num=(ISDIGIT(*message)?*message-'0':TOLOWER(*message)-'a'+10);
								if(ISXDIGIT(*message)){
									++message;
									CHECK_EOS(message);
									num<<=8;
									num+=(ISDIGIT(*message)?*message-'0':TOLOWER(*message)-'a'+10);
								}
								WFIFOB(fd,off)=TOB(num);
								++message;
								CHECK_EOS(message);
								continue;
							}
							case '0':
							case '1':
							case '2':
							case '3':
							case '4':
							case '5':
							case '6':
							case '7': // Octal
							{
								num=*message-'0'; // 1st octal digit
								++message;
								CHECK_EOS(message);
								if(ISDIGIT(*message) && *message < '8'){
									num<<=3;
									num+=*message-'0'; // 2nd octal digit
									++message;
									CHECK_EOS(message);
									if(ISDIGIT(*message) && *message < '8'){
										num<<=3;
										num+=*message-'0'; // 3rd octal digit
										++message;
										CHECK_EOS(message);
									}
								}
								WFIFOB(fd,off)=TOB(num);
								continue;
							}
						}
					} else
						num=*message;
					WFIFOB(fd,off)=TOB(num);
					++message;
					CHECK_EOS(message);
				}//for
				while(*message != '"')
				{// ignore extra characters
					++message;
					CHECK_EOS(message);
				}

				// terminate the string
				if(off < end)
				{// fill the rest with 0's
					memset(WFIFOP(fd,off),0,end-off);
					off=end;
				}
			} else
			{// unknown
				PARSE_ERROR("Unknown type of value in:",message);
				return -1;
			}
			SKIP_VALUE(message);
		}

		if(packet_db[sd->packet_ver][type].len == -1)
		{// send dynamic packet
			WFIFOW(fd,2)=TOW(off);
			WFIFOSET(fd,off);
		} else
		{// send static packet
			if(off < len)
				memset(WFIFOP(fd,off),0,len-off);
			WFIFOSET(fd,len);
		}
	} else {
		clif_displaymessage(fd, msg_txt(259)); // Invalid packet
		return -1;
	}
	sprintf (atcmd_output, msg_txt(258), type, type); // Sent packet 0x%x (%d)
	clif_displaymessage(fd, atcmd_output);
	return 0;
#undef PARSE_ERROR
#undef CHECK_EOS
#undef SKIP_VALUE
#undef GET_VALUE
}

/*==========================================
 * @rura, @warp, @mapmove
 *------------------------------------------*/
int atcommand_mapmove(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	char map_name[MAP_NAME_LENGTH_EXT];
	unsigned short mapindex;
	int x = 0, y = 0;
	int m = -1;

	nullpo_retr(-1, sd);

	memset(map_name, '\0', sizeof(map_name));

	if (!message || !*message ||
		(sscanf(message, "%15s %d %d", map_name, &x, &y) < 3 &&
		 sscanf(message, "%15[^,],%d,%d", map_name, &x, &y) < 1)) {
		 
			clif_displaymessage(fd, "Please, enter a map (usage: @warp/@rura/@mapmove <mapname> <x> <y>).");
			return -1;
	}

	mapindex = mapindex_name2id(map_name);
	if (mapindex)
		m = map_mapindex2mapid(mapindex);
	
	if (!mapindex || m < 0) {
		clif_displaymessage(fd, msg_txt(1)); // Map not found.
		return -1;
	}

	if ((x || y) && map_getcell(m, x, y, CELL_CHKNOPASS))
  	{	//This is to prevent the pc_setpos call from printing an error.
		clif_displaymessage(fd, msg_txt(2));
		x = y = 0; //Invalid cell, use random spot.
	}
	if (map[m].flag.nowarpto && battle_config.any_warp_GM_min_level > pc_isGM(sd)) {
		clif_displaymessage(fd, msg_txt(247));
		return -1;
	}
	if (sd->bl.m >= 0 && map[sd->bl.m].flag.nowarp && battle_config.any_warp_GM_min_level > pc_isGM(sd)) {
		clif_displaymessage(fd, msg_txt(248));
		return -1;
	}
	if (pc_setpos(sd, mapindex, x, y, 3) != 0) {
		clif_displaymessage(fd, msg_txt(1)); // Map not found.
		return -1;
	}

	clif_displaymessage(fd, msg_txt(0)); // Warped.
	return 0;
}

/*==========================================
 * Displays where a character is. Corrected version by Silent. [Skotlex]
 *------------------------------------------*/
int atcommand_where(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	struct map_session_data* pl_sd;

	nullpo_retr(-1, sd);
	memset(atcmd_player_name, '\0', sizeof atcmd_player_name);

	if (!message || !*message || sscanf(message, "%23[^\n]", atcmd_player_name) < 1) {
		clif_displaymessage(fd, "Please, enter a player name (usage: @where <char name>).");
		return -1;
	}

	pl_sd = map_nick2sd(atcmd_player_name);
	if( pl_sd == NULL
	||  strncmp(pl_sd->status.name,atcmd_player_name,NAME_LENGTH) != 0
	||  (battle_config.hide_GM_session && pc_isGM(sd) < pc_isGM(pl_sd) && !(battle_config.who_display_aid && pc_isGM(sd) >= battle_config.who_display_aid))
	) {
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	snprintf(atcmd_output, sizeof atcmd_output, "%s %s %d %d", pl_sd->status.name, mapindex_id2name(pl_sd->mapindex), pl_sd->bl.x, pl_sd->bl.y);
	clif_displaymessage(fd, atcmd_output);

	return 0;
}

/*==========================================
 *
 *------------------------------------------*/
int atcommand_jumpto(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	struct map_session_data *pl_sd = NULL;

	nullpo_retr(-1, sd);

	if (!message || !*message) {
		clif_displaymessage(fd, "Please, enter a player name (usage: @jumpto/@warpto/@goto <player name/id>).");
		return -1;
	}

	if((pl_sd=map_nick2sd((char *)message)) == NULL && (pl_sd=map_charid2sd(atoi(message))) == NULL)
	{
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}
	
	if (pl_sd == sd)
	{
		clif_displaymessage(fd, "But you are already where you are...");
		return -1;
	}
	
	if (pl_sd->bl.m >= 0 && map[pl_sd->bl.m].flag.nowarpto && battle_config.any_warp_GM_min_level > pc_isGM(sd))
	{
		clif_displaymessage(fd, msg_txt(247));	// You are not authorized to warp to this map.
		return -1;
	}
	
	if (sd->bl.m >= 0 && map[sd->bl.m].flag.nowarp && battle_config.any_warp_GM_min_level > pc_isGM(sd))
	{
		clif_displaymessage(fd, msg_txt(248));	// You are not authorized to warp from your current map.
		return -1;
	}
	
	pc_setpos(sd, pl_sd->mapindex, pl_sd->bl.x, pl_sd->bl.y, 3);
	sprintf(atcmd_output, msg_txt(4), pl_sd->status.name); // Jumped to %s
 	clif_displaymessage(fd, atcmd_output);

	return 0;
}

/*==========================================
 *
 *------------------------------------------*/
int atcommand_jump(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	int x = 0, y = 0;

	nullpo_retr(-1, sd);

	memset(atcmd_output, '\0', sizeof(atcmd_output));

	sscanf(message, "%d %d", &x, &y);

	if (x <= 0) //If coordinates are 'wrong', random jump.
		x = -1;
	if (y <= 0)
		y = -1;
	if (sd->bl.m >= 0 && map[sd->bl.m].flag.noteleport && battle_config.any_warp_GM_min_level > pc_isGM(sd)) {
		clif_displaymessage(fd, msg_txt(248));	// You are not authorized to warp from your current map.
		return -1;
	}
	pc_setpos(sd, sd->mapindex, x, y, 3);
	sprintf(atcmd_output, msg_txt(5), sd->bl.x, sd->bl.y); // Jumped to %d %d
	clif_displaymessage(fd, atcmd_output);
	return 0;
}

/*==========================================
 * @who3 = Player name, his location
 *------------------------------------------*/
int atcommand_who3(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	char temp0[100];
	struct map_session_data *pl_sd;
	struct s_mapiterator* iter;
	int j, count;
	int pl_GM_level, GM_level;
	char match_text[100];
	char player_name[NAME_LENGTH];

	nullpo_retr(-1, sd);

	memset(atcmd_output, '\0', sizeof(atcmd_output));
	memset(match_text, '\0', sizeof(match_text));
	memset(player_name, '\0', sizeof(player_name));

	if (sscanf(message, "%99[^\n]", match_text) < 1)
		strcpy(match_text, "");
	for (j = 0; match_text[j]; j++)
		match_text[j] = TOLOWER(match_text[j]);

	count = 0;
	GM_level = pc_isGM(sd);

	iter = mapit_getallusers();
	for( pl_sd = (TBL_PC*)mapit_first(iter); mapit_exists(iter); pl_sd = (TBL_PC*)mapit_next(iter) )
	{
		pl_GM_level = pc_isGM(pl_sd);
		if(!( (battle_config.hide_GM_session || (pl_sd->sc.option & OPTION_INVISIBLE)) && pl_GM_level > GM_level ))
		{// you can look only lower or same level
			memcpy(player_name, pl_sd->status.name, NAME_LENGTH);
			for (j = 0; player_name[j]; j++)
				player_name[j] = TOLOWER(player_name[j]);
			if (strstr(player_name, match_text) != NULL) { // search with no case sensitive

				if (battle_config.who_display_aid > 0 && pc_isGM(sd) >= battle_config.who_display_aid) {
					sprintf(atcmd_output, "(CID:%d/AID:%d) ", pl_sd->status.char_id, pl_sd->status.account_id);
				} else {
					atcmd_output[0]=0;
				}
				//Player name
				sprintf(temp0, msg_txt(333), pl_sd->status.name);
				strcat(atcmd_output,temp0);
				//Player title, if exists
				if (pl_GM_level > 0) {
					//sprintf(temp0, "(%s) ", player_title_txt(pl_GM_level) );
					sprintf(temp0, msg_txt(334), player_title_txt(pl_GM_level) );
					strcat(atcmd_output,temp0);
				}
				//Players Location: map x y
				sprintf(temp0, msg_txt(338), mapindex_id2name(pl_sd->mapindex), pl_sd->bl.x, pl_sd->bl.y);
				strcat(atcmd_output,temp0);

				clif_displaymessage(fd, atcmd_output);
				count++;
			}
		}
	}
	mapit_free(iter);

	if (count == 0)
		clif_displaymessage(fd, msg_txt(28)); // No player found.
	else if (count == 1)
		clif_displaymessage(fd, msg_txt(29)); // 1 player found.
	else {
		sprintf(atcmd_output, msg_txt(30), count); // %d players found.
		clif_displaymessage(fd, atcmd_output);
	}

	return 0;
}

/*==========================================
 * Player name, BLevel, Job, 
 *------------------------------------------*/
int atcommand_who2(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	char temp0[100];
	struct map_session_data *pl_sd;
	struct s_mapiterator* iter;
	int j, count;
	int pl_GM_level, GM_level;
	char match_text[100];
	char player_name[NAME_LENGTH];

	nullpo_retr(-1, sd);

	memset(atcmd_output, '\0', sizeof(atcmd_output));
	memset(match_text, '\0', sizeof(match_text));
	memset(player_name, '\0', sizeof(player_name));

	if (sscanf(message, "%99[^\n]", match_text) < 1)
		strcpy(match_text, "");
	for (j = 0; match_text[j]; j++)
		match_text[j] = TOLOWER(match_text[j]);

	count = 0;
	GM_level = pc_isGM(sd);

	iter = mapit_getallusers();
	for( pl_sd = (TBL_PC*)mapit_first(iter); mapit_exists(iter); pl_sd = (TBL_PC*)mapit_next(iter) )
	{
		pl_GM_level = pc_isGM(pl_sd);
		if(!( (battle_config.hide_GM_session || (pl_sd->sc.option & OPTION_INVISIBLE)) && (pl_GM_level > GM_level) ))
		{// you can look only lower or same level
			memcpy(player_name, pl_sd->status.name, NAME_LENGTH);
			for (j = 0; player_name[j]; j++)
				player_name[j] = TOLOWER(player_name[j]);
			if (strstr(player_name, match_text) != NULL) { // search with no case sensitive
				//Players Name
				//sprintf(atcmd_output, "Name: %s ", pl_sd->status.name);
				sprintf(atcmd_output, msg_txt(333), pl_sd->status.name);
				//Player title, if exists
				if (pl_GM_level > 0) {
					//sprintf(temp0, "(%s) ", player_title_txt(pl_GM_level) );
					sprintf(temp0, msg_txt(334), player_title_txt(pl_GM_level) );
					strcat(atcmd_output,temp0);
				}
				//Players Base Level / Job name
				//sprintf(temp0, "| L:%d/%d | Job: %s", pl_sd->status.base_level, pl_sd->status.job_level, job_name(pl_sd->status.class_) );
				sprintf(temp0, msg_txt(337), pl_sd->status.base_level, pl_sd->status.job_level, job_name(pl_sd->status.class_) );
				strcat(atcmd_output,temp0);

				clif_displaymessage(fd, atcmd_output);
				count++;
			}
		}
	}
	mapit_free(iter);
	
	if (count == 0)
		clif_displaymessage(fd, msg_txt(28)); // No player found.
	else if (count == 1)
		clif_displaymessage(fd, msg_txt(29)); // 1 player found.
	else {
		sprintf(atcmd_output, msg_txt(30), count); // %d players found.
		clif_displaymessage(fd, atcmd_output);
	}

	return 0;
}

/*==========================================
 * Player name, Playrs Party / Guild name
 *------------------------------------------*/
int atcommand_who(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	char temp0[100];
	struct map_session_data *pl_sd;
	struct s_mapiterator* iter;
	int j, count;
	int pl_GM_level, GM_level;
	char match_text[100];
	char player_name[NAME_LENGTH];
	struct guild *g;
	struct party_data *p;

	nullpo_retr(-1, sd);

	memset(temp0, '\0', sizeof(temp0));
	memset(atcmd_output, '\0', sizeof(atcmd_output));
	memset(match_text, '\0', sizeof(match_text));
	memset(player_name, '\0', sizeof(player_name));

	if (sscanf(message, "%99[^\n]", match_text) < 1)
		strcpy(match_text, "");
	for (j = 0; match_text[j]; j++)
		match_text[j] = TOLOWER(match_text[j]);

	count = 0;
	GM_level = pc_isGM(sd);

	iter = mapit_getallusers();
	for( pl_sd = (TBL_PC*)mapit_first(iter); mapit_exists(iter); pl_sd = (TBL_PC*)mapit_next(iter) )
	{
		pl_GM_level = pc_isGM(pl_sd);
		if(!( (battle_config.hide_GM_session || (pl_sd->sc.option & OPTION_INVISIBLE)) && pl_GM_level > GM_level ))
		{// you can look only lower or same level
			memcpy(player_name, pl_sd->status.name, NAME_LENGTH);
			for (j = 0; player_name[j]; j++)
				player_name[j] = TOLOWER(player_name[j]);
			if (strstr(player_name, match_text) != NULL) { // search with no case sensitive
				g = guild_search(pl_sd->status.guild_id);
				p = party_search(pl_sd->status.party_id);
				//Players Name
				sprintf(atcmd_output, msg_txt(333), pl_sd->status.name);
				//Player title, if exists
				if (pl_GM_level > 0) {
					sprintf(temp0, msg_txt(334), player_title_txt(pl_GM_level) );
					strcat(atcmd_output,temp0);
				}
				//Players Party if exists
				if (p != NULL) {
					//sprintf(temp0," | Party: '%s'", p->name);
					sprintf(temp0, msg_txt(335), p->party.name);
					strcat(atcmd_output,temp0);
				}
				//Players Guild if exists
				if (g != NULL) {
					//sprintf(temp0," | Guild: '%s'", g->name);
					sprintf(temp0, msg_txt(336), g->name);
					strcat(atcmd_output,temp0);
				}
				clif_displaymessage(fd, atcmd_output);
				count++;
			}
		}
	}
	mapit_free(iter);

	if (count == 0)
		clif_displaymessage(fd, msg_txt(28)); // No player found.
	else if (count == 1)
		clif_displaymessage(fd, msg_txt(29)); // 1 player found.
	else {
		sprintf(atcmd_output, msg_txt(30), count); // %d players found.
		clif_displaymessage(fd, atcmd_output);
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------*/
int atcommand_whomap3(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	struct map_session_data *pl_sd;
	struct s_mapiterator* iter;
	int count;
	int pl_GM_level, GM_level;
	int map_id;
	char map_name[MAP_NAME_LENGTH_EXT];

	memset(atcmd_output, '\0', sizeof(atcmd_output));
	memset(map_name, '\0', sizeof(map_name));

	if (!message || !*message)
		map_id = sd->bl.m;
	else {
		sscanf(message, "%15s", map_name);
		if ((map_id = map_mapname2mapid(map_name)) < 0)
			map_id = sd->bl.m;
	}

	count = 0;
	GM_level = pc_isGM(sd);

	iter = mapit_getallusers();
	for( pl_sd = (TBL_PC*)mapit_first(iter); mapit_exists(iter); pl_sd = (TBL_PC*)mapit_next(iter) )
	{
		pl_GM_level = pc_isGM(pl_sd);
		if( pl_sd->bl.m != map_id )
			continue;
		if( (battle_config.hide_GM_session || (pl_sd->sc.option & OPTION_INVISIBLE)) && (pl_GM_level > GM_level) )
			continue;

		if (pl_GM_level > 0)
			sprintf(atcmd_output, "Name: %s (GM:%d) | Location: %s %d %d", pl_sd->status.name, pl_GM_level, mapindex_id2name(pl_sd->mapindex), pl_sd->bl.x, pl_sd->bl.y);
		else
			sprintf(atcmd_output, "Name: %s | Location: %s %d %d", pl_sd->status.name, mapindex_id2name(pl_sd->mapindex), pl_sd->bl.x, pl_sd->bl.y);
		clif_displaymessage(fd, atcmd_output);
		count++;
	}
	mapit_free(iter);

	if (count == 0)
		sprintf(atcmd_output, msg_txt(54), map[map_id].name); // No player found in map '%s'.
	else if (count == 1)
		sprintf(atcmd_output, msg_txt(55), map[map_id].name); // 1 player found in map '%s'.
	else {
		sprintf(atcmd_output, msg_txt(56), count, map[map_id].name); // %d players found in map '%s'.
	}
	clif_displaymessage(fd, atcmd_output);

	return 0;
}

/*==========================================
 *
 *------------------------------------------*/
int atcommand_whomap2(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	struct map_session_data *pl_sd;
	struct s_mapiterator* iter;
	int count;
	int pl_GM_level, GM_level;
	int map_id = 0;
	char map_name[MAP_NAME_LENGTH_EXT];

	nullpo_retr(-1, sd);

	memset(atcmd_output, '\0', sizeof(atcmd_output));
	memset(map_name, '\0', sizeof(map_name));

	if (!message || !*message)
		map_id = sd->bl.m;
	else {
		sscanf(message, "%15s", map_name);
		if ((map_id = map_mapname2mapid(map_name)) < 0)
			map_id = sd->bl.m;
	}

	count = 0;
	GM_level = pc_isGM(sd);

	iter = mapit_getallusers();
	for( pl_sd = (TBL_PC*)mapit_first(iter); mapit_exists(iter); pl_sd = (TBL_PC*)mapit_next(iter) )
	{
		pl_GM_level = pc_isGM(pl_sd);
		if( pl_sd->bl.m != map_id )
			continue;
		if( (battle_config.hide_GM_session || (pl_sd->sc.option & OPTION_INVISIBLE)) && (pl_GM_level > GM_level) )
			continue;

		if (pl_GM_level > 0)
			sprintf(atcmd_output, "Name: %s (GM:%d) | BLvl: %d | Job: %s (Lvl: %d)", pl_sd->status.name, pl_GM_level, pl_sd->status.base_level, job_name(pl_sd->status.class_), pl_sd->status.job_level);
		else
			sprintf(atcmd_output, "Name: %s | BLvl: %d | Job: %s (Lvl: %d)", pl_sd->status.name, pl_sd->status.base_level, job_name(pl_sd->status.class_), pl_sd->status.job_level);
		clif_displaymessage(fd, atcmd_output);
		count++;
	}
	mapit_free(iter);

	if (count == 0)
		sprintf(atcmd_output, msg_txt(54), map[map_id].name); // No player found in map '%s'.
	else if (count == 1)
		sprintf(atcmd_output, msg_txt(55), map[map_id].name); // 1 player found in map '%s'.
	else {
		sprintf(atcmd_output, msg_txt(56), count, map[map_id].name); // %d players found in map '%s'.
	}
	clif_displaymessage(fd, atcmd_output);

	return 0;
}

/*==========================================
 *
 *------------------------------------------*/
int atcommand_whomap(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	char temp0[100];
	char temp1[100];
	struct map_session_data *pl_sd;
	struct s_mapiterator* iter;
	int count;
	int pl_GM_level, GM_level;
	int map_id = 0;
	char map_name[MAP_NAME_LENGTH_EXT];
	struct guild *g;
	struct party_data *p;

	nullpo_retr(-1, sd);

	memset(temp0, '\0', sizeof(temp0));
	memset(temp1, '\0', sizeof(temp1));
	memset(atcmd_output, '\0', sizeof(atcmd_output));
	memset(map_name, '\0', sizeof(map_name));

	if (!message || !*message)
		map_id = sd->bl.m;
	else {
		sscanf(message, "%15s", map_name);
		if ((map_id = map_mapname2mapid(map_name)) < 0)
			map_id = sd->bl.m;
	}

	count = 0;
	GM_level = pc_isGM(sd);

	iter = mapit_getallusers();
	for( pl_sd = (TBL_PC*)mapit_first(iter); mapit_exists(iter); pl_sd = (TBL_PC*)mapit_next(iter) )
	{
		pl_GM_level = pc_isGM(pl_sd);
		if( pl_sd->bl.m != map_id )
			continue;
		if( (battle_config.hide_GM_session || (pl_sd->sc.option & OPTION_INVISIBLE)) && (pl_GM_level > GM_level) )
			continue;

		g = guild_search(pl_sd->status.guild_id);
		if (g == NULL)
			sprintf(temp1, "None");
		else
			sprintf(temp1, "%s", g->name);
		p = party_search(pl_sd->status.party_id);
		if (p == NULL)
			sprintf(temp0, "None");
		else
			sprintf(temp0, "%s", p->party.name);
		if (pl_GM_level > 0)
			sprintf(atcmd_output, "Name: %s (GM:%d) | Party: '%s' | Guild: '%s'", pl_sd->status.name, pl_GM_level, temp0, temp1);
		else
			sprintf(atcmd_output, "Name: %s | Party: '%s' | Guild: '%s'", pl_sd->status.name, temp0, temp1);
		clif_displaymessage(fd, atcmd_output);
		count++;
	}
	mapit_free(iter);

	if (count == 0)
		sprintf(atcmd_output, msg_txt(54), map[map_id].name); // No player found in map '%s'.
	else if (count == 1)
		sprintf(atcmd_output, msg_txt(55), map[map_id].name); // 1 player found in map '%s'.
	else {
		sprintf(atcmd_output, msg_txt(56), count, map[map_id].name); // %d players found in map '%s'.
	}
	clif_displaymessage(fd, atcmd_output);

	return 0;
}

/*==========================================
 *
 *------------------------------------------*/
int atcommand_whogm(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	struct map_session_data* pl_sd;
	struct s_mapiterator* iter;
	int j, count;
	int pl_GM_level, GM_level;
	char match_text[200];
	char player_name[NAME_LENGTH];
	struct guild *g;
	struct party_data *p;

	nullpo_retr(-1, sd);

	memset(atcmd_output, '\0', sizeof(atcmd_output));
	memset(match_text, '\0', sizeof(match_text));
	memset(player_name, '\0', sizeof(player_name));

	if (sscanf(message, "%199[^\n]", match_text) < 1)
		strcpy(match_text, "");
	for (j = 0; match_text[j]; j++)
		match_text[j] = TOLOWER(match_text[j]);

	count = 0;
	GM_level = pc_isGM(sd);

	iter = mapit_getallusers();
	for( pl_sd = (TBL_PC*)mapit_first(iter); mapit_exists(iter); pl_sd = (TBL_PC*)mapit_next(iter) )
	{
		pl_GM_level = pc_isGM(pl_sd);
		if (!pl_GM_level)
			continue;

		if (match_text[0])
		{
			memcpy(player_name, pl_sd->status.name, NAME_LENGTH);
			for (j = 0; player_name[j]; j++)
				player_name[j] = TOLOWER(player_name[j]);
		  	// search with no case sensitive
			if (strstr(player_name, match_text) == NULL)
				continue;
		}
		if (pl_GM_level > GM_level) {
			if (pl_sd->sc.option & OPTION_INVISIBLE)
				continue;
			sprintf(atcmd_output, "Name: %s (GM)", pl_sd->status.name);
			clif_displaymessage(fd, atcmd_output);
			count++;
			continue;
		}

		sprintf(atcmd_output, "Name: %s (GM:%d) | Location: %s %d %d",
			pl_sd->status.name, pl_GM_level,
			mapindex_id2name(pl_sd->mapindex), pl_sd->bl.x, pl_sd->bl.y);
		clif_displaymessage(fd, atcmd_output);

		sprintf(atcmd_output, "       BLvl: %d | Job: %s (Lvl: %d)",
			pl_sd->status.base_level,
			job_name(pl_sd->status.class_), pl_sd->status.job_level);
		clif_displaymessage(fd, atcmd_output);
		
		p = party_search(pl_sd->status.party_id);
		g = guild_search(pl_sd->status.guild_id);
	
		sprintf(atcmd_output,"       Party: '%s' | Guild: '%s'",
			p?p->party.name:"None", g?g->name:"None");

		clif_displaymessage(fd, atcmd_output);
		count++;
	}
	mapit_free(iter);

	if (count == 0)
		clif_displaymessage(fd, msg_txt(150)); // No GM found.
	else if (count == 1)
		clif_displaymessage(fd, msg_txt(151)); // 1 GM found.
	else {
		sprintf(atcmd_output, msg_txt(152), count); // %d GMs found.
		clif_displaymessage(fd, atcmd_output);
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------*/
int atcommand_save(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	nullpo_retr(-1, sd);

	pc_setsavepoint(sd, sd->mapindex, sd->bl.x, sd->bl.y);
	if (sd->status.pet_id > 0 && sd->pd)
		intif_save_petdata(sd->status.account_id, &sd->pd->pet);

	chrif_save(sd,0);
	
	clif_displaymessage(fd, msg_txt(6)); // Character data respawn point saved.

	return 0;
}

/*==========================================
 *
 *------------------------------------------*/
int atcommand_load(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	int m;

	nullpo_retr(-1, sd);

	m = map_mapindex2mapid(sd->status.save_point.map);
	if (m >= 0 && map[m].flag.nowarpto && battle_config.any_warp_GM_min_level > pc_isGM(sd)) {
		clif_displaymessage(fd, msg_txt(249));	// You are not authorized to warp to your save map.
		return -1;
	}
	if (sd->bl.m >= 0 && map[sd->bl.m].flag.nowarp && battle_config.any_warp_GM_min_level > pc_isGM(sd)) {
		clif_displaymessage(fd, msg_txt(248));	// You are not authorized to warp from your current map.
		return -1;
	}

	pc_setpos(sd, sd->status.save_point.map, sd->status.save_point.x, sd->status.save_point.y, 0);
	clif_displaymessage(fd, msg_txt(7)); // Warping to respawn point.

	return 0;
}

/*==========================================
 *
 *------------------------------------------*/
int atcommand_speed(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	int speed;

	nullpo_retr(-1, sd);

	memset(atcmd_output, '\0', sizeof(atcmd_output));

	if (!message || !*message || sscanf(message, "%d", &speed) < 1) {
		sprintf(atcmd_output, "Please, enter a speed value (usage: @speed <%d-%d>).", MIN_WALK_SPEED, MAX_WALK_SPEED);
		clif_displaymessage(fd, atcmd_output);
		return -1;
	}

	sd->base_status.speed = cap_value(speed, MIN_WALK_SPEED, MAX_WALK_SPEED);
	status_calc_bl(&sd->bl, SCB_SPEED);
	clif_displaymessage(fd, msg_txt(8)); // Speed changed.
	return 0;
}

/*==========================================
 *
 *------------------------------------------*/
int atcommand_storage(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	nullpo_retr(-1, sd);
	
	if (sd->npc_id || sd->vender_id || sd->state.trading || sd->state.storage_flag)
		return -1;

	if (storage_storageopen(sd) == 1)
	{	//Already open.
		clif_displaymessage(fd, msg_txt(250));
		return -1;
	}
	
	clif_displaymessage(fd, "Storage opened.");
	
	return 0;
}


/*==========================================
 *
 *------------------------------------------*/
int atcommand_guildstorage(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	struct storage *stor; //changes from Freya/Yor
	nullpo_retr(-1, sd);

	if (!sd->status.guild_id) {
		clif_displaymessage(fd, msg_txt(252));
		return -1;
	}

	if (sd->npc_id || sd->vender_id || sd->state.trading || sd->state.storage_flag)
		return -1;

	if (sd->state.storage_flag) {
		clif_displaymessage(fd, msg_txt(251));
		return -1;
	}

	if ((stor = account2storage2(sd->status.account_id)) != NULL && stor->storage_status == 1) {
		clif_displaymessage(fd, msg_txt(251));
		return -1;
	}
	storage_guild_storageopen(sd);
	clif_displaymessage(fd, "Guild storage opened.");
	return 0;
}

/*==========================================
 *
 *------------------------------------------*/
int atcommand_option(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	int param1 = 0, param2 = 0, param3 = 0;
	nullpo_retr(-1, sd);

	if (!message || !*message || sscanf(message, "%d %d %d", &param1, &param2, &param3) < 1 || param1 < 0 || param2 < 0 || param3 < 0) {
		clif_displaymessage(fd, "Please, enter at least a option (usage: @option <param1:0+> <param2:0+> <param3:0+>).");
		return -1;
	}

	sd->sc.opt1 = param1;
	sd->sc.opt2 = param2;
	pc_setoption(sd, param3);
	
	clif_displaymessage(fd, msg_txt(9)); // Options changed.

	return 0;
}

/*==========================================
 *
 *------------------------------------------*/
int atcommand_hide(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	nullpo_retr(-1, sd);
	if (sd->sc.option & OPTION_INVISIBLE) {
		sd->sc.option &= ~OPTION_INVISIBLE;
		if (sd->disguise)
			status_set_viewdata(&sd->bl, sd->disguise);
		else
			status_set_viewdata(&sd->bl, sd->status.class_);
		clif_displaymessage(fd, msg_txt(10)); // Invisible: Off
	} else {
		sd->sc.option |= OPTION_INVISIBLE;
		sd->vd.class_ = INVISIBLE_CLASS;
		clif_displaymessage(fd, msg_txt(11)); // Invisible: On
	}
	clif_changeoption(&sd->bl);

	return 0;
}

/*==========================================
 * Changes a character's class
 *------------------------------------------*/
int atcommand_jobchange(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	//FIXME: redundancy, potentially wrong code, should use job_name() or similar instead of hardcoding the table [ultramage]
	int job = 0, upper = 0;
	nullpo_retr(-1, sd);

	if (!message || !*message || sscanf(message, "%d %d", &job, &upper) < 1)
	{
		int i, found = 0;
		const struct { char name[16]; int id; } jobs[] = {
			{ "novice",		0 },
			{ "swordsman",	1 },
			{ "mage",		2 },
			{ "archer",		3 },
			{ "acolyte",	4 },
			{ "merchant",	5 },
			{ "thief",		6 },
			{ "knight",		7 },
			{ "priest",		8 },
			{ "priestess",	8 },
			{ "wizard",		9 },
			{ "blacksmith",	10 },
			{ "hunter",		11 },
			{ "assassin",	12 },
			{ "crusader",	14 },
			{ "monk",		15 },
			{ "sage",		16 },
			{ "rogue",		17 },
			{ "alchemist",	18 },
			{ "bard",		19 },
			{ "dancer",		20 },
			{ "super novice",	23 },
			{ "supernovice",	23 },
			{ "gunslinger",	24 },
			{ "gunner",	24 },
			{ "ninja",	25 },
			{ "high novice",	4001 },
			{ "swordsman high",	4002 },
			{ "mage high",		4003 },
			{ "archer high",	4004 },
			{ "acolyte high",	4005 },
			{ "merchant high",	4006 },
			{ "thief high",		4007 },
			{ "lord knight",	4008 },
			{ "high priest",	4009 },
			{ "high priestess",	4009 },
			{ "high wizard",	4010 },
			{ "whitesmith",		4011 },
			{ "sniper",		4012 },
			{ "assassin cross",	4013 },
			{ "paladin",	4015 },
			{ "champion",	4016 },
			{ "professor",	4017 },
			{ "stalker",	4018 },
			{ "creator",	4019 },
			{ "clown",		4020 },
			{ "gypsy",		4021 },
			{ "baby novice",	4023 },
			{ "baby swordsman",	4024 },
			{ "baby mage",		4025 },
			{ "baby archer",	4026 },
			{ "baby acolyte",	4027 },
			{ "baby merchant",	4028 },
			{ "baby thief",		4029 },
			{ "baby knight",	4030 },
			{ "baby priest",	4031 },
			{ "baby priestess",	4031 },
			{ "baby wizard",	4032 },
			{ "baby blacksmith",4033 },
			{ "baby hunter",	4034 },
			{ "baby assassin",	4035 },
			{ "baby crusader",	4037 },
			{ "baby monk",		4038 },
			{ "baby sage",		4039 },
			{ "baby rogue",		4040 },
			{ "baby alchemist",	4041 },
			{ "baby bard",		4042 },
			{ "baby dancer",	4043 },
			{ "super baby",		4045 },
			{ "taekwon",		4046 },
			{ "taekwon boy",	4046 },
			{ "taekwon girl",	4046 },
			{ "star gladiator",	4047 },
			{ "soul linker",	4049 },
		};

		for (i=0; i < ARRAYLENGTH(jobs); i++) {
			if (strncmpi(message, jobs[i].name, 16) == 0) {
				job = jobs[i].id;
				upper = 0;
				found = 1;
				break;
			}
		}

		if (!found) {
			clif_displaymessage(fd, "Please, enter job ID (usage: @job/@jobchange <job name/ID>).");
			clif_displaymessage(fd, "   0 Novice            7 Knight           14 Crusader         21 Peco Crusader");
			clif_displaymessage(fd, "   1 Swordman          8 Priest           15 Monk             22 N/A");
			clif_displaymessage(fd, "   2 Mage              9 Wizard           16 Sage             23 Super Novice");
			clif_displaymessage(fd, "   3 Archer           10 Blacksmith       17 Rogue            24 Gunslinger");
			clif_displaymessage(fd, "   4 Acolyte          11 Hunter           18 Alchemist        25 Ninja");
			clif_displaymessage(fd, "   5 Merchant         12 Assassin         19 Bard             26 N/A");
			clif_displaymessage(fd, "   6 Thief            13 Peco Knight      20 Dancer");
			clif_displaymessage(fd, "4001 Novice High    4008 Lord Knight      4015 Paladin        4022 Peco Paladin");
			clif_displaymessage(fd, "4002 Swordman High  4009 High Priest      4016 Champion");
			clif_displaymessage(fd, "4003 Mage High      4010 High Wizard      4017 Professor");
			clif_displaymessage(fd, "4004 Archer High    4011 Whitesmith       4018 Stalker");
			clif_displaymessage(fd, "4005 Acolyte High   4012 Sniper           4019 Creator");
			clif_displaymessage(fd, "4006 Merchant High  4013 Assassin Cross   4020 Clown");
			clif_displaymessage(fd, "4007 Thief High     4014 Peco Lord Knight 4021 Gypsy");
			clif_displaymessage(fd, "4023 Baby Novice    4030 Baby Knight      4037 Baby Crusader  4044 Baby Peco Crusader");
			clif_displaymessage(fd, "4024 Baby Swordsman 4031 Baby Priest      4038 Baby Monk      4045 Super Baby");
			clif_displaymessage(fd, "4025 Baby Mage      4032 Baby Wizard      4039 Baby Sage      4046 Taekwon Kid");
			clif_displaymessage(fd, "4026 Baby Archer    4033 Baby Blacksmith  4040 Baby Rogue     4047 Taekwon Master");
			clif_displaymessage(fd, "4027 Baby Acolyte   4034 Baby Hunter      4041 Baby Alchemist 4048 N/A");
			clif_displaymessage(fd, "4028 Baby Merchant  4035 Baby Assassin    4042 Baby Bard      4049 Soul Linker");
			clif_displaymessage(fd, "4029 Baby Thief     4036 Baby Peco-Knight 4043 Baby Dancer");
			clif_displaymessage(fd, "[upper]: -1 (default) to automatically determine the 'level', 0 to force normal job, 1 to force high job.");
			return -1;
		}
	}

	if (job == 37 ||job == 45)
		return 0;

	if (pcdb_checkid(job))
	{
		if (pc_jobchange(sd, job, upper) == 0)
			clif_displaymessage(fd, msg_txt(12)); // Your job has been changed.
		else {
			clif_displaymessage(fd, msg_txt(155)); // Impossible to change your job.
			return -1;
		}
	} else {
		clif_displaymessage(fd, "Please, enter a valid job ID (usage: @job/@jobchange <job name/ID>).");
		clif_displaymessage(fd, "   0 Novice            7 Knight           14 Crusader         21 Peco Crusader");
		clif_displaymessage(fd, "   1 Swordman          8 Priest           15 Monk             22 N/A");
		clif_displaymessage(fd, "   2 Mage              9 Wizard           16 Sage             23 Super Novice");
		clif_displaymessage(fd, "   3 Archer           10 Blacksmith       17 Rogue            24 Gunslinger");
		clif_displaymessage(fd, "   4 Acolyte          11 Hunter           18 Alchemist        25 Ninja");
		clif_displaymessage(fd, "   5 Merchant         12 Assassin         19 Bard             26 N/A");
		clif_displaymessage(fd, "   6 Thief            13 Peco Knight      20 Dancer");
		clif_displaymessage(fd, "4001 Novice High    4008 Lord Knight      4015 Paladin        4022 Peco Paladin");
		clif_displaymessage(fd, "4002 Swordman High  4009 High Priest      4016 Champion");
		clif_displaymessage(fd, "4003 Mage High      4010 High Wizard      4017 Professor");
		clif_displaymessage(fd, "4004 Archer High    4011 Whitesmith       4018 Stalker");
		clif_displaymessage(fd, "4005 Acolyte High   4012 Sniper           4019 Creator");
		clif_displaymessage(fd, "4006 Merchant High  4013 Assassin Cross   4020 Clown");
		clif_displaymessage(fd, "4007 Thief High     4014 Peco Lord Knight 4021 Gypsy");
		clif_displaymessage(fd, "4023 Baby Novice    4030 Baby Knight      4037 Baby Crusader  4044 Baby Peco Crusader");
		clif_displaymessage(fd, "4024 Baby Swordsman 4031 Baby Priest      4038 Baby Monk      4045 Super Baby");
		clif_displaymessage(fd, "4025 Baby Mage      4032 Baby Wizard      4039 Baby Sage      4046 Taekwon Kid");
		clif_displaymessage(fd, "4026 Baby Archer    4033 Baby Blacksmith  4040 Baby Rogue     4047 Taekwon Master");
		clif_displaymessage(fd, "4027 Baby Acolyte   4034 Baby Hunter      4041 Baby Alchemist 4048 N/A");
		clif_displaymessage(fd, "4028 Baby Merchant  4035 Baby Assassin    4042 Baby Bard      4049 Soul Linker");
		clif_displaymessage(fd, "4029 Baby Thief     4036 Baby Peco-Knight 4043 Baby Dancer");
		clif_displaymessage(fd, "[upper]: -1 (default) to automatically determine the 'level', 0 to force normal job, 1 to force high job.");
		return -1;
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------*/
int atcommand_die(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	nullpo_retr(-1, sd);
	clif_specialeffect(&sd->bl,450,SELF);
	status_kill(&sd->bl);
	clif_displaymessage(fd, msg_txt(13)); // A pity! You've died.

	return 0;
}

/*==========================================
 *
 *------------------------------------------*/
int atcommand_kill(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	struct map_session_data *pl_sd;
	nullpo_retr(-1, sd);

	memset(atcmd_player_name, '\0', sizeof(atcmd_player_name));

	if (!message || !*message) {
		clif_displaymessage(fd, "Please, enter a player name (usage: @kill <player name/id>).");
		return -1;
	}

	if((pl_sd=map_nick2sd((char *)message)) == NULL && (pl_sd=map_charid2sd(atoi(message))) == NULL)
	{
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}
	
	if (pc_isGM(sd) < pc_isGM(pl_sd))
	{ // you can kill only lower or same level
		clif_displaymessage(fd, msg_txt(81)); // Your GM level don't authorise you to do this action on this player.
		return -1;
	}
	
	status_kill(&pl_sd->bl);
	clif_displaymessage(pl_sd->fd, msg_txt(13)); // A pity! You've died.
	if (fd != pl_sd->fd)
		clif_displaymessage(fd, msg_txt(14)); // Character killed.

	return 0;
}

/*==========================================
 *
 *------------------------------------------*/
int atcommand_alive(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	nullpo_retr(-1, sd);
	if (!status_revive(&sd->bl, 100, 100))
	{
		clif_displaymessage(fd, "You're not dead.");
		return -1;
	}
	clif_skill_nodamage(&sd->bl,&sd->bl,ALL_RESURRECTION,4,1);
	clif_displaymessage(fd, msg_txt(16)); // You've been revived! It's a miracle!
	return 0;
}

/*==========================================
 * +kamic [LuzZza]
 *------------------------------------------*/
int atcommand_kami(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	unsigned long color=0;
	nullpo_retr(-1, sd);

	memset(atcmd_output, '\0', sizeof(atcmd_output));

	if(*(command + 5) != 'c' && *(command + 5) != 'C') {

		if (!message || !*message) {
			clif_displaymessage(fd, "Please, enter a message (usage: @kami <message>).");
			return -1;
		}

		sscanf(message, "%199[^\n]", atcmd_output);
		intif_GMmessage(atcmd_output, strlen(atcmd_output) + 1, (*(command + 5) == 'b' || *(command + 5) == 'B') ? 0x10 : 0);
	
	} else {
	
		if(!message || !*message || (sscanf(message, "%lx %199[^\n]", &color, atcmd_output) < 2)) {
			clif_displaymessage(fd, "Please, enter color and message (usage: @kamic <color> <message>).");
			return -1;
		}
	
		if(color > 0xFFFFFF) {
			clif_displaymessage(fd, "Invalid color.");
			return -1;
		}
	
		intif_announce(atcmd_output, strlen(atcmd_output) + 1, color, 0);
	}
	return 0;
}

/*==========================================
 *
 *------------------------------------------*/
int atcommand_heal(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	int hp = 0, sp = 0; // [Valaris] thanks to fov
	nullpo_retr(-1, sd);

	sscanf(message, "%d %d", &hp, &sp);

	if (hp == 0 && sp == 0) {
		if (!status_percent_heal(&sd->bl, 100, 100))
			clif_displaymessage(fd, msg_txt(157)); // HP and SP are already with the good value.
		else
			clif_displaymessage(fd, msg_txt(17)); // HP, SP recovered.
		return 0;
	}
	
	if(hp > 0 && sp >= 0) {
		if(!status_heal(&sd->bl, hp, sp, 0))
			clif_displaymessage(fd, msg_txt(157)); // HP and SP are already with the good value.
		else
			clif_displaymessage(fd, msg_txt(17)); // HP, SP recovered.
		return 0;
	}

	if(hp < 0 && sp <= 0) {
		status_damage(NULL, &sd->bl, -hp, -sp, 0, 0);
		clif_damage(&sd->bl,&sd->bl, gettick(), 0, 0, -hp, 0 , 4, 0);
		clif_displaymessage(fd, msg_txt(156)); // HP or/and SP modified.
		return 0;
	}

	//Opposing signs.
	if (hp) {
		if (hp > 0)
			status_heal(&sd->bl, hp, 0, 0);
		else {
			status_damage(NULL, &sd->bl, -hp, 0, 0, 0);
			clif_damage(&sd->bl,&sd->bl, gettick(), 0, 0, -hp, 0 , 4, 0);
		}
	}

	if (sp) {
		if (sp > 0)
			status_heal(&sd->bl, 0, sp, 0);
		else
			status_damage(NULL, &sd->bl, 0, -sp, 0, 0);
	}

	clif_displaymessage(fd, msg_txt(156)); // HP or/and SP modified.
	return 0;
}

/*==========================================
 * @item command (usage: @item <name/id_of_item> <quantity>) (modified by [Yor] for pet_egg)
 *------------------------------------------*/
int atcommand_item(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	char item_name[100];
	int number = 0, item_id, flag;
	struct item item_tmp;
	struct item_data *item_data;
	int get_count, i;
	nullpo_retr(-1, sd);

	memset(item_name, '\0', sizeof(item_name));

	if (!message || !*message || (
		sscanf(message, "\"%99[^\"]\" %d", item_name, &number) < 1 &&
		sscanf(message, "%99s %d", item_name, &number) < 1
	)) {
		clif_displaymessage(fd, "Please, enter an item name/id (usage: @item <item name or ID> [quantity]).");
		return -1;
	}

	if (number <= 0)
		number = 1;

	if ((item_data = itemdb_searchname(item_name)) == NULL &&
	    (item_data = itemdb_exists(atoi(item_name))) == NULL)
	{
		clif_displaymessage(fd, msg_txt(19)); // Invalid item ID or name.
		return -1;
	}

	item_id = item_data->nameid;
	get_count = number;
	//Check if it's stackable.
	if (!itemdb_isstackable2(item_data))
		get_count = 1;

	for (i = 0; i < number; i += get_count) {
		// if not pet egg
		if (!pet_create_egg(sd, item_id)) {
			memset(&item_tmp, 0, sizeof(item_tmp));
			item_tmp.nameid = item_id;
			item_tmp.identify = 1;

			if ((flag = pc_additem(sd, &item_tmp, get_count)))
				clif_additem(sd, 0, 0, flag);
		}
	}

	//Logs (A)dmins items [Lupus]
	if(log_config.enable_logs&0x400)
		log_pick_pc(sd, "A", item_id, number, NULL);

	clif_displaymessage(fd, msg_txt(18)); // Item created.
	return 0;
}

/*==========================================
 *
 *------------------------------------------*/
int atcommand_item2(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	struct item item_tmp;
	struct item_data *item_data;
	char item_name[100];
	int item_id, number = 0;
	int identify = 0, refine = 0, attr = 0;
	int c1 = 0, c2 = 0, c3 = 0, c4 = 0;
	int flag;
	int loop, get_count, i;
	nullpo_retr(-1, sd);

	memset(item_name, '\0', sizeof(item_name));

	if (!message || !*message || (
		sscanf(message, "\"%99[^\"]\" %d %d %d %d %d %d %d %d", item_name, &number, &identify, &refine, &attr, &c1, &c2, &c3, &c4) < 9 &&
		sscanf(message, "%99s %d %d %d %d %d %d %d %d", item_name, &number, &identify, &refine, &attr, &c1, &c2, &c3, &c4) < 9
	)) {
		clif_displaymessage(fd, "Please, enter all informations (usage: @item2 <item name or ID> <quantity>");
		clif_displaymessage(fd, "  <Identify_flag> <refine> <attribut> <Card1> <Card2> <Card3> <Card4>).");
		return -1;
	}

	if (number <= 0)
		number = 1;

	item_id = 0;
	if ((item_data = itemdb_searchname(item_name)) != NULL ||
	    (item_data = itemdb_exists(atoi(item_name))) != NULL)
		item_id = item_data->nameid;

	if (item_id > 500) {
		loop = 1;
		get_count = number;
		if (item_data->type == 4 || item_data->type == 5 ||
			item_data->type == 7 || item_data->type == 8) {
			loop = number;
			get_count = 1;
			if (item_data->type == 7) {
				identify = 1;
				refine = 0;
			}
			if (item_data->type == 8)
				refine = 0;
			if (refine > 10)
				refine = 10;
		} else {
			identify = 1;
			refine = attr = 0;
		}
		for (i = 0; i < loop; i++) {
			memset(&item_tmp, 0, sizeof(item_tmp));
			item_tmp.nameid = item_id;
			item_tmp.identify = identify;
			item_tmp.refine = refine;
			item_tmp.attribute = attr;
			item_tmp.card[0] = c1;
			item_tmp.card[1] = c2;
			item_tmp.card[2] = c3;
			item_tmp.card[3] = c4;
			if ((flag = pc_additem(sd, &item_tmp, get_count)))
				clif_additem(sd, 0, 0, flag);
		}

		//Logs (A)dmins items [Lupus]
		if(log_config.enable_logs&0x400)
			log_pick_pc(sd, "A", item_tmp.nameid, number, &item_tmp);

		clif_displaymessage(fd, msg_txt(18)); // Item created.
	} else {
		clif_displaymessage(fd, msg_txt(19)); // Invalid item ID or name.
		return -1;
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------*/
int atcommand_itemreset(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	int i;
	nullpo_retr(-1, sd);

	for (i = 0; i < MAX_INVENTORY; i++) {
		if (sd->status.inventory[i].amount && sd->status.inventory[i].equip == 0) {

			//Logs (A)dmins items [Lupus]
			if(log_config.enable_logs&0x400)
				log_pick_pc(sd, "A", sd->status.inventory[i].nameid, -sd->status.inventory[i].amount, &sd->status.inventory[i]);

			pc_delitem(sd, i, sd->status.inventory[i].amount, 0);
		}
	}
	clif_displaymessage(fd, msg_txt(20)); // All of your items have been removed.

	return 0;
}

/*==========================================
 * Atcommand @lvlup
 *------------------------------------------*/
int atcommand_baselevelup(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	int level=0, i=0, status_point=0;
	nullpo_retr(-1, sd);
	level = atoi(message);

	if (!message || !*message || !level) {
		clif_displaymessage(fd, "Please, enter a level adjustment (usage: @lvup/@blevel/@baselvlup <number of levels>).");
		return -1;
	}

	if (level > 0) {
		if (sd->status.base_level == pc_maxbaselv(sd)) { // check for max level by Valaris
			clif_displaymessage(fd, msg_txt(47)); // Base level can't go any higher.
			return -1;
		} // End Addition
		if ((unsigned int)level > pc_maxbaselv(sd) || (unsigned int)level > pc_maxbaselv(sd) - sd->status.base_level) // fix positiv overflow
			level = pc_maxbaselv(sd) - sd->status.base_level;
		for (i = 1; i <= level; i++)
			status_point += (sd->status.base_level + i + 14) / 5;

		if (sd->status.status_point > USHRT_MAX - status_point)
			sd->status.status_point = USHRT_MAX;
		else
			sd->status.status_point += status_point;
		sd->status.base_level += (unsigned int)level;
		status_percent_heal(&sd->bl, 100, 100);
		clif_misceffect(&sd->bl, 0);
		clif_displaymessage(fd, msg_txt(21)); // Base level raised.
	} else {
		if (sd->status.base_level == 1) {
			clif_displaymessage(fd, msg_txt(158)); // Base level can't go any lower.
			return -1;
		}
		level*=-1;
		if ((unsigned int)level >= sd->status.base_level)
			level = sd->status.base_level-1;
		for (i = 0; i > -level; i--)
			status_point += (sd->status.base_level + i + 14) / 5;
		if (sd->status.status_point < status_point)
			pc_resetstate(sd);
		if (sd->status.status_point < status_point)
			sd->status.status_point = 0;
		else
			sd->status.status_point -= status_point;
		sd->status.base_level -= (unsigned int)level;
		clif_displaymessage(fd, msg_txt(22)); // Base level lowered.
	}
	clif_updatestatus(sd, SP_STATUSPOINT);
	clif_updatestatus(sd, SP_BASELEVEL);
	clif_updatestatus(sd, SP_NEXTBASEEXP);
	status_calc_pc(sd, 0);
	if(sd->status.party_id)
		party_send_levelup(sd);
	return 0;
}

/*==========================================
 *
 *------------------------------------------*/
int atcommand_joblevelup(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	int level=0;
	nullpo_retr(-1, sd);
	
	level = atoi(message);

	if (!message || !*message || !level) {
		clif_displaymessage(fd, "Please, enter a level adjustment (usage: @joblvup/@jlevel/@joblvlup <number of levels>).");
		return -1;
	}
	if (level > 0) {
		if (sd->status.job_level == pc_maxjoblv(sd)) {
			clif_displaymessage(fd, msg_txt(23)); // Job level can't go any higher.
			return -1;
		}
		if ((unsigned int)level > pc_maxjoblv(sd) || (unsigned int)level > pc_maxjoblv(sd) - sd->status.job_level) // fix positiv overflow
			level = pc_maxjoblv(sd) - sd->status.job_level;
		sd->status.job_level += (unsigned int)level;
		clif_updatestatus(sd, SP_JOBLEVEL);
		clif_updatestatus(sd, SP_NEXTJOBEXP);
		if (sd->status.skill_point > USHRT_MAX - level)
			sd->status.skill_point = USHRT_MAX;
		else
			sd->status.skill_point += level;
		clif_updatestatus(sd, SP_SKILLPOINT);
		status_calc_pc(sd, 0);
		clif_misceffect(&sd->bl, 1);
		clif_displaymessage(fd, msg_txt(24)); // Job level raised.
	} else {
		if (sd->status.job_level == 1) {
			clif_displaymessage(fd, msg_txt(159)); // Job level can't go any lower.
			return -1;
		}
		level *=-1;
		if ((unsigned int)level >= sd->status.job_level) // fix negativ overflow
			level = sd->status.job_level-1;
		sd->status.job_level -= (unsigned int)level;
		clif_updatestatus(sd, SP_JOBLEVEL);
		clif_updatestatus(sd, SP_NEXTJOBEXP);
		if (sd->status.skill_point < level)
			pc_resetskill(sd,0);	//Reset skills since we need to substract more points.
		if (sd->status.skill_point < level)
			sd->status.skill_point = 0;
		else
			sd->status.skill_point -= level;
		clif_updatestatus(sd, SP_SKILLPOINT);
		status_calc_pc(sd, 0);
		clif_displaymessage(fd, msg_txt(25)); // Job level lowered.
	}

	return 0;
}

/*==========================================
 * @help
 *------------------------------------------*/
int atcommand_help(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	char buf[2048], w1[2048], w2[2048];
	int i, gm_level;
	FILE* fp;
	nullpo_retr(-1, sd);

	memset(buf, '\0', sizeof(buf));

	if ((fp = fopen(help_txt, "r")) != NULL) {
		clif_displaymessage(fd, msg_txt(26)); // Help commands:
		gm_level = pc_isGM(sd);
		while(fgets(buf, sizeof(buf), fp) != NULL) {
			if (buf[0] == '/' && buf[1] == '/')
				continue;
			for (i = 0; buf[i] != '\0'; i++) {
				if (buf[i] == '\r' || buf[i] == '\n') {
					buf[i] = '\0';
					break;
				}
			}
			if (sscanf(buf, "%2047[^:]:%2047[^\n]", w1, w2) < 2)
				clif_displaymessage(fd, buf);
			else if (gm_level >= atoi(w1))
				clif_displaymessage(fd, w2);
		}
		fclose(fp);
	} else {
		clif_displaymessage(fd, msg_txt(27)); // File help.txt not found.
		return -1;
	}

	return 0;
}

/*==========================================
 * @help2 - Char commands [Kayla]
 *------------------------------------------*/
int atcommand_help2(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	char buf[2048], w1[2048], w2[2048];
	int i, gm_level;
	FILE* fp;
	nullpo_retr(-1, sd);

	memset(buf, '\0', sizeof(buf));

	if ((fp = fopen(help2_txt, "r")) != NULL) {
		clif_displaymessage(fd, msg_txt(26)); // Help commands:
		gm_level = pc_isGM(sd);
		while(fgets(buf, sizeof(buf), fp) != NULL) {
			if (buf[0] == '/' && buf[1] == '/')
				continue;
			for (i = 0; buf[i] != '\0'; i++) {
				if (buf[i] == '\r' || buf[i] == '\n') {
					buf[i] = '\0';
					break;
				}
			}
			if (sscanf(buf, "%2047[^:]:%2047[^\n]", w1, w2) < 2)
				clif_displaymessage(fd, buf);
			else if (gm_level >= atoi(w1))
				clif_displaymessage(fd, w2);
		}
		fclose(fp);
	} else {
		clif_displaymessage(fd, msg_txt(27)); // File help.txt not found.
		return -1;
	}

	return 0;
}


/*==========================================
 * @gm
 *------------------------------------------*/
int atcommand_gm(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	char password[100];
	nullpo_retr(-1, sd);

	memset(password, '\0', sizeof(password));

	if (!message || !*message || sscanf(message, "%99[^\n]", password) < 1) {
		clif_displaymessage(fd, "Please, enter a password (usage: @gm <password>).");
		return -1;
	}

	if (pc_isGM(sd)) { // a GM can not use this function. only a normal player (become gm is not for gm!)
		clif_displaymessage(fd, msg_txt(50)); // You already have some GM powers.
		return -1;
	} else
		chrif_changegm(sd->status.account_id, password, strlen(password) + 1);

	return 0;
}

// helper function, used in foreach calls to stop auto-attack timers
// parameter: '0' - everyone, 'id' - only those attacking someone with that id
static int atcommand_stopattack(struct block_list *bl,va_list ap)
{
	struct unit_data *ud = unit_bl2ud(bl);
	int id = va_arg(ap, int);
	if (ud && ud->attacktimer != INVALID_TIMER && (!id || id == ud->target))
	{
		unit_stop_attack(bl);
		return 1;
	}
	return 0;
}
/*==========================================
 *
 *------------------------------------------*/
static int atcommand_pvpoff_sub(struct block_list *bl,va_list ap)
{
	TBL_PC* sd = (TBL_PC*)bl;
	clif_pvpset(sd, 0, 0, 2);
	if (sd->pvp_timer != -1) {
		delete_timer(sd->pvp_timer, pc_calc_pvprank_timer);
		sd->pvp_timer = -1;
	}
	return 0;
}

int atcommand_pvpoff(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	nullpo_retr(-1, sd);

	if (!map[sd->bl.m].flag.pvp) {
		clif_displaymessage(fd, msg_txt(160)); // PvP is already Off.
		return -1;
	}

	map[sd->bl.m].flag.pvp = 0;

	if (!battle_config.pk_mode)
		clif_send0199(sd->bl.m, 0);
	map_foreachinmap(atcommand_pvpoff_sub,sd->bl.m, BL_PC);
	map_foreachinmap(atcommand_stopattack,sd->bl.m, BL_CHAR, 0);
	clif_displaymessage(fd, msg_txt(31)); // PvP: Off.
	return 0;
}

/*==========================================
 *
 *------------------------------------------*/
static int atcommand_pvpon_sub(struct block_list *bl,va_list ap)
{
	TBL_PC* sd = (TBL_PC*)bl;
	if (sd->pvp_timer == -1) {
		sd->pvp_timer = add_timer(gettick() + 200, pc_calc_pvprank_timer, sd->bl.id, 0);
		sd->pvp_rank = 0;
		sd->pvp_lastusers = 0;
		sd->pvp_point = 5;
		sd->pvp_won = 0;
		sd->pvp_lost = 0;
	}
	return 0;
}

int atcommand_pvpon(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	nullpo_retr(-1, sd);

	if (map[sd->bl.m].flag.pvp) {
		clif_displaymessage(fd, msg_txt(161)); // PvP is already On.
		return -1;
	}

	map[sd->bl.m].flag.pvp = 1;

	if (!battle_config.pk_mode)
	{// display pvp circle and rank
		clif_send0199(sd->bl.m, 1);
		map_foreachinmap(atcommand_pvpon_sub,sd->bl.m, BL_PC);
	}

	clif_displaymessage(fd, msg_txt(32)); // PvP: On.

	return 0;
}

/*==========================================
 *
 *------------------------------------------*/
int atcommand_gvgoff(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	nullpo_retr(-1, sd);

	if (!map[sd->bl.m].flag.gvg) {
		clif_displaymessage(fd, msg_txt(162)); // GvG is already Off.
		return -1;
	}
		
	map[sd->bl.m].flag.gvg = 0;
	clif_send0199(sd->bl.m, 0);
	map_foreachinmap(atcommand_stopattack,sd->bl.m, BL_CHAR, 0);
	clif_displaymessage(fd, msg_txt(33)); // GvG: Off.

	return 0;
}

/*==========================================
 *
 *------------------------------------------*/
int atcommand_gvgon(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	nullpo_retr(-1, sd);

	if (map[sd->bl.m].flag.gvg) {
		clif_displaymessage(fd, msg_txt(163)); // GvG is already On.
		return -1;
	}
	
	map[sd->bl.m].flag.gvg = 1;
	clif_send0199(sd->bl.m, 3);
	clif_displaymessage(fd, msg_txt(34)); // GvG: On.

	return 0;
}

/*==========================================
 *
 *------------------------------------------*/
int atcommand_model(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	int hair_style = 0, hair_color = 0, cloth_color = 0;
	nullpo_retr(-1, sd);

	memset(atcmd_output, '\0', sizeof(atcmd_output));

	if (!message || !*message || sscanf(message, "%d %d %d", &hair_style, &hair_color, &cloth_color) < 1) {
		sprintf(atcmd_output, "Please, enter at least a value (usage: @model <hair ID: %d-%d> <hair color: %d-%d> <clothes color: %d-%d>).",
		        MIN_HAIR_STYLE, MAX_HAIR_STYLE, MIN_HAIR_COLOR, MAX_HAIR_COLOR, MIN_CLOTH_COLOR, MAX_CLOTH_COLOR);
		clif_displaymessage(fd, atcmd_output);
		return -1;
	}

	if (hair_style >= MIN_HAIR_STYLE && hair_style <= MAX_HAIR_STYLE &&
		hair_color >= MIN_HAIR_COLOR && hair_color <= MAX_HAIR_COLOR &&
		cloth_color >= MIN_CLOTH_COLOR && cloth_color <= MAX_CLOTH_COLOR) {
			pc_changelook(sd, LOOK_HAIR, hair_style);
			pc_changelook(sd, LOOK_HAIR_COLOR, hair_color);
			pc_changelook(sd, LOOK_CLOTHES_COLOR, cloth_color);
			clif_displaymessage(fd, msg_txt(36)); // Appearence changed.
	} else {
		clif_displaymessage(fd, msg_txt(37)); // An invalid number was specified.
		return -1;
	}

	return 0;
}

/*==========================================
 * @dye && @ccolor
 *------------------------------------------*/
int atcommand_dye(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	int cloth_color = 0;
	nullpo_retr(-1, sd);

	memset(atcmd_output, '\0', sizeof(atcmd_output));

	if (!message || !*message || sscanf(message, "%d", &cloth_color) < 1) {
		sprintf(atcmd_output, "Please, enter a clothes color (usage: @dye/@ccolor <clothes color: %d-%d>).", MIN_CLOTH_COLOR, MAX_CLOTH_COLOR);
		clif_displaymessage(fd, atcmd_output);
		return -1;
	}

	if (cloth_color >= MIN_CLOTH_COLOR && cloth_color <= MAX_CLOTH_COLOR) {
		pc_changelook(sd, LOOK_CLOTHES_COLOR, cloth_color);
		clif_displaymessage(fd, msg_txt(36)); // Appearence changed.
	} else {
		clif_displaymessage(fd, msg_txt(37)); // An invalid number was specified.
		return -1;
	}

	return 0;
}

/*==========================================
 * @hairstyle && @hstyle
 *------------------------------------------*/
int atcommand_hair_style(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	int hair_style = 0;
	nullpo_retr(-1, sd);

	memset(atcmd_output, '\0', sizeof(atcmd_output));

	if (!message || !*message || sscanf(message, "%d", &hair_style) < 1) {
		sprintf(atcmd_output, "Please, enter a hair style (usage: @hairstyle/@hstyle <hair ID: %d-%d>).", MIN_HAIR_STYLE, MAX_HAIR_STYLE);
		clif_displaymessage(fd, atcmd_output);
		return -1;
	}

	if (hair_style >= MIN_HAIR_STYLE && hair_style <= MAX_HAIR_STYLE) {
			pc_changelook(sd, LOOK_HAIR, hair_style);
			clif_displaymessage(fd, msg_txt(36)); // Appearence changed.
	} else {
		clif_displaymessage(fd, msg_txt(37)); // An invalid number was specified.
		return -1;
	}

	return 0;
}

/*==========================================
 * @haircolor && @hcolor
 *------------------------------------------*/
int atcommand_hair_color(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	int hair_color = 0;
	nullpo_retr(-1, sd);

	memset(atcmd_output, '\0', sizeof(atcmd_output));

	if (!message || !*message || sscanf(message, "%d", &hair_color) < 1) {
		sprintf(atcmd_output, "Please, enter a hair color (usage: @haircolor/@hcolor <hair color: %d-%d>).", MIN_HAIR_COLOR, MAX_HAIR_COLOR);
		clif_displaymessage(fd, atcmd_output);
		return -1;
	}

	if (hair_color >= MIN_HAIR_COLOR && hair_color <= MAX_HAIR_COLOR) {
			pc_changelook(sd, LOOK_HAIR_COLOR, hair_color);
			clif_displaymessage(fd, msg_txt(36)); // Appearence changed.
	} else {
		clif_displaymessage(fd, msg_txt(37)); // An invalid number was specified.
		return -1;
	}

	return 0;
}

/*==========================================
 * @go [city_number or city_name] - Updated by Harbin
 *------------------------------------------*/
int atcommand_go(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	int i;
	int town;
	char map_name[MAP_NAME_LENGTH];
	int m;
 
	const struct {
		char map[MAP_NAME_LENGTH];
		int x, y;
	} data[] = {
		{ MAP_PRONTERA,    156, 191 }, //  0=Prontera
		{ MAP_MORROC,      156,  93 }, //  1=Morroc
		{ MAP_GEFFEN,      119,  59 }, //  2=Geffen
		{ MAP_PAYON,       162, 233 }, //  3=Payon
		{ MAP_ALBERTA,     192, 147 }, //  4=Alberta
		{ MAP_IZLUDE,      128, 114 }, //  5=Izlude
		{ MAP_ALDEBARAN,   140, 131 }, //  6=Al de Baran
		{ MAP_LUTIE,       147, 134 }, //  7=Lutie
		{ MAP_COMODO,      209, 143 }, //  8=Comodo
		{ MAP_YUNO,        157,  51 }, //  9=Yuno
		{ MAP_AMATSU,      198,  84 }, // 10=Amatsu
		{ MAP_GONRYUN,     160, 120 }, // 11=Gonryun
		{ MAP_UMBALA,       89, 157 }, // 12=Umbala
		{ MAP_NIFLHEIM,     21, 153 }, // 13=Niflheim
		{ MAP_LOUYANG,     217,  40 }, // 14=Louyang
		{ MAP_NOVICE,       53, 111 }, // 15=Training Grounds
		{ MAP_JAIL,         23,  61 }, // 16=Prison
		{ MAP_JAWAII,      249, 127 }, // 17=Jawaii
		{ MAP_AYOTHAYA,    151, 117 }, // 18=Ayothaya
		{ MAP_EINBROCH,     64, 200 }, // 19=Einbroch
		{ MAP_LIGHTHALZEN, 158,  92 }, // 20=Lighthalzen
		{ MAP_EINBECH,      70,  95 }, // 21=Einbech
		{ MAP_HUGEL,        96, 145 }, // 22=Hugel
		{ MAP_RACHEL,      130, 110 }, // 23=Rachel
		{ MAP_VEINS,       216, 123 }, // 24=Veins
	};
 
	nullpo_retr(-1, sd);
 
	if( map[sd->bl.m].flag.nogo && battle_config.any_warp_GM_min_level > pc_isGM(sd) ) {
		clif_displaymessage(sd->fd,"You can not use @go on this map.");
		return 0;
	}
 
	memset(map_name, '\0', sizeof(map_name));
	memset(atcmd_output, '\0', sizeof(atcmd_output));
 
	// get the number
	town = atoi(message);
 
	// if no value, display all value
	if (!message || !*message || sscanf(message, "%11s", map_name) < 1 || town < 0 || town >= ARRAYLENGTH(data)) {
		clif_displaymessage(fd, msg_txt(38)); // Invalid location number or name.
		clif_displaymessage(fd, msg_txt(82)); // Please, use one of this number/name:
		clif_displaymessage(fd, " 0=Prontera         1=Morroc       2=Geffen");
		clif_displaymessage(fd, " 3=Payon            4=Alberta      5=Izlude");
		clif_displaymessage(fd, " 6=Al De Baran      7=Lutie        8=Comodo");
		clif_displaymessage(fd, " 9=Yuno             10=Amatsu      11=Gonryun");
		clif_displaymessage(fd, " 12=Umbala          13=Niflheim    14=Louyang");
		clif_displaymessage(fd, " 15=Novice Grounds  16=Prison      17=Jawaii");
		clif_displaymessage(fd, " 18=Ayothaya        19=Einbroch    20=Lighthalzen");
		clif_displaymessage(fd, " 21=Einbech         22=Hugel       23=Rachel");
		clif_displaymessage(fd, " 24=Veins");
		return -1;
	}

	// get possible name of the city
	map_name[MAP_NAME_LENGTH-1] = '\0';
	for (i = 0; map_name[i]; i++)
		map_name[i] = TOLOWER(map_name[i]);
	// try to identify the map name
	if (strncmp(map_name, "prontera", 3) == 0) {
		town = 0;
	} else if (strncmp(map_name, "morocc", 3) == 0) {
		town = 1;
	} else if (strncmp(map_name, "geffen", 3) == 0) {
		town = 2;
	} else if (strncmp(map_name, "payon", 3) == 0 ||
	           strncmp(map_name, "paion", 3) == 0) {
		town = 3;
	} else if (strncmp(map_name, "alberta", 3) == 0) {
		town = 4;
	} else if (strncmp(map_name, "izlude", 3) == 0 ||
	           strncmp(map_name, "islude", 3) == 0) {
		town = 5;
	} else if (strncmp(map_name, "aldebaran", 3) == 0 ||
	           strcmp(map_name,  "al") == 0) {
		town = 6;
	} else if (strncmp(map_name, "lutie", 3) == 0 ||
	           strcmp(map_name,  "christmas") == 0 ||
	           strncmp(map_name, "xmas", 3) == 0 ||
	           strncmp(map_name, "x-mas", 3) == 0) {
		town = 7;
	} else if (strncmp(map_name, "comodo", 3) == 0) {
		town = 8;
	} else if (strncmp(map_name, "yuno", 3) == 0) {
		town = 9;
	} else if (strncmp(map_name, "amatsu", 3) == 0) {
		town = 10;
	} else if (strncmp(map_name, "gonryun", 3) == 0) {
		town = 11;
	} else if (strncmp(map_name, "umbala", 3) == 0) {
		town = 12;
	} else if (strncmp(map_name, "niflheim", 3) == 0) {
		town = 13;
	} else if (strncmp(map_name, "louyang", 3) == 0) {
		town = 14;
	} else if (strncmp(map_name, "new_zone01", 3) == 0 ||
	           strncmp(map_name, "startpoint", 3) == 0 ||
	           strncmp(map_name, "begining", 3) == 0) {
		town = 15;
	} else if (strncmp(map_name, "sec_pri", 3) == 0 ||
	           strncmp(map_name, "prison", 3) == 0 ||
	           strncmp(map_name, "jails", 3) == 0) {
		town = 16;
	} else if (strncmp(map_name, "jawaii", 3) == 0 ||
	           strncmp(map_name, "jawai", 3) == 0) {
		town = 17;
	} else if (strncmp(map_name, "ayothaya", 3) == 0 ||
	           strncmp(map_name, "ayotaya", 3) == 0) {
		town = 18;
	} else if (strncmp(map_name, "einbroch", 5) == 0 ||
	           strncmp(map_name, "ainbroch", 5) == 0) {
		town = 19;
	} else if (strncmp(map_name, "lighthalzen", 3) == 0) {
		town = 20;
	} else if (strncmp(map_name, "einbech", 3) == 0) {
		town = 21;
	} else if (strncmp(map_name, "hugel", 3) == 0) {
		town = 22;
	} else if (strncmp(map_name, "rachel", 3) == 0) {
		town = 23;
	} else if (strncmp(map_name, "veins", 3) == 0) {
		town = 24;
	}

	if (town >= 0 && town < ARRAYLENGTH(data))
	{
		m = map_mapname2mapid(data[town].map);
		if (m >= 0 && map[m].flag.nowarpto && battle_config.any_warp_GM_min_level > pc_isGM(sd)) {
			clif_displaymessage(fd, msg_txt(247));
			return -1;
		}
		if (sd->bl.m >= 0 && map[sd->bl.m].flag.nowarp && battle_config.any_warp_GM_min_level > pc_isGM(sd)) {
			clif_displaymessage(fd, msg_txt(248));
			return -1;
		}
		if (pc_setpos(sd, mapindex_name2id(data[town].map), data[town].x, data[town].y, 3) == 0) {
			clif_displaymessage(fd, msg_txt(0)); // Warped.
		} else {
			clif_displaymessage(fd, msg_txt(1)); // Map not found.
			return -1;
		}
	} else { // if you arrive here, you have an error in town variable when reading of names
		clif_displaymessage(fd, msg_txt(38)); // Invalid location number or name.
		return -1;
	}
 
	return 0;
}

/*==========================================
 *
 *------------------------------------------*/
int atcommand_monster(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	char name[NAME_LENGTH];
	char monster[NAME_LENGTH];
	int mob_id;
	int number = 0;
	int count;
	int i, k, range;
	short mx, my;
	nullpo_retr(-1, sd);

	memset(name, '\0', sizeof(name));
	memset(monster, '\0', sizeof(monster));
	memset(atcmd_output, '\0', sizeof(atcmd_output));

	if (!message || !*message) {
			clif_displaymessage(fd, msg_txt(80)); // Give a display name and monster name/id please.
			return -1;
	}
	if (sscanf(message, "\"%23[^\"]\" %23s %d", name, monster, &number) > 1 ||
		sscanf(message, "%23s \"%23[^\"]\" %d", monster, name, &number) > 1) {
		//All data can be left as it is.
	} else if ((count=sscanf(message, "%23s %d %23s", monster, &number, name)) > 1) {
		//Here, it is possible name was not given and we are using monster for it.
		if (count < 3) //Blank mob's name.
			name[0] = '\0';
	} else if (sscanf(message, "%23s %23s %d", name, monster, &number) > 1) {
		//All data can be left as it is.
	} else if (sscanf(message, "%23s", monster) > 0) {
		//As before, name may be already filled.
		name[0] = '\0';
	} else {
		clif_displaymessage(fd, msg_txt(80)); // Give a display name and monster name/id please.
		return -1;
	}

	if ((mob_id = mobdb_searchname(monster)) == 0) // check name first (to avoid possible name begining by a number)
		mob_id = mobdb_checkid(atoi(monster));

	if (mob_id == 0) {
		clif_displaymessage(fd, msg_txt(40)); // Invalid monster ID or name.
		return -1;
	}

	if (mob_id == MOBID_EMPERIUM) {
		clif_displaymessage(fd, msg_txt(83)); // Cannot spawn emperium.
		return -1;
	}

	if (number <= 0)
		number = 1;

	if (strlen(name) < 1)
		strcpy(name, "--ja--");

	// If value of atcommand_spawn_quantity_limit directive is greater than or equal to 1 and quantity of monsters is greater than value of the directive
	if (battle_config.atc_spawn_quantity_limit && number > battle_config.atc_spawn_quantity_limit)
		number = battle_config.atc_spawn_quantity_limit;

	if (battle_config.etc_log)
		ShowInfo("%s monster='%s' name='%s' id=%d count=%d (%d,%d)\n", command, monster, name, mob_id, number, sd->bl.x, sd->bl.y);

	count = 0;
	range = (int)sqrt(number) +2; // calculation of an odd number (+ 4 area around)
	for (i = 0; i < number; i++) {
		map_search_freecell(&sd->bl, 0, &mx,  &my, range, range, 0);
		k = mob_once_spawn(sd, sd->bl.m, mx, my, name, mob_id, 1, "");
		count += (k != 0) ? 1 : 0;
	}

	if (count != 0)
		if (number == count)
			clif_displaymessage(fd, msg_txt(39)); // All monster summoned!
		else {
			sprintf(atcmd_output, msg_txt(240), count); // %d monster(s) summoned!
			clif_displaymessage(fd, atcmd_output);
		}
	else {
		clif_displaymessage(fd, msg_txt(40)); // Invalid monster ID or name.
		return -1;
	}

	return 0;
}

// small monster spawning [Valaris]
int atcommand_monstersmall(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	char name[NAME_LENGTH] = "";
	char monster[NAME_LENGTH] = "";
	int mob_id = 0;
	int number = 0;
	int x = 0;
	int y = 0;
	int count;
	int i;

	nullpo_retr(-1, sd);

	if (!message || !*message) {
		clif_displaymessage(fd, "Give a monster name/id please.");
		return -1;
	}

	if (sscanf(message, "\"%23[^\"]\" %23s %d %d %d", name, monster, &number, &x, &y) < 2 &&
	    sscanf(message, "%23s \"%23[^\"]\" %d %d %d", monster, name, &number, &x, &y) < 2 &&
	    sscanf(message, "%23s %d %23s %d %d", monster, &number, name, &x, &y) < 1) {
		clif_displaymessage(fd, "Give a monster name/id please.");
		return -1;
	}

	// If monster identifier/name argument is a name
	if ((mob_id = mobdb_searchname(monster)) == 0) // check name first (to avoid possible name begining by a number)
		mob_id = atoi(monster);

	if (mob_id == 0) {
		clif_displaymessage(fd, msg_txt(40));
		return -1;
	}

	if (mob_id == MOBID_EMPERIUM) {
		clif_displaymessage(fd, msg_txt(83));	// Cannot spawn emperium
		return -1;
	}

	if (mobdb_checkid(mob_id) == 0) {
		clif_displaymessage(fd, "Invalid monster ID"); // Invalid Monster ID.
		return -1;
	}

	if (number <= 0)
		number = 1;

	if (strlen(name) < 1)
		strcpy(name, "--ja--");

	// If value of atcommand_spawn_quantity_limit directive is greater than or equal to 1 and quantity of monsters is greater than value of the directive
	if (battle_config.atc_spawn_quantity_limit >= 1 && number > battle_config.atc_spawn_quantity_limit)
		number = battle_config.atc_spawn_quantity_limit;

	count = 0;
	for (i = 0; i < number; i++) {
		int mx, my;
		if (x <= 0)
			mx = sd->bl.x + (rand() % 11 - 5);
		else
			mx = x;
		if (y <= 0)
			my = sd->bl.y + (rand() % 11 - 5);
		else
			my = y;
		count += (mob_once_spawn(sd, sd->bl.m, mx, my, name, mob_id, 1, "2") != 0) ? 1 : 0;
	}

	if (count != 0)
		clif_displaymessage(fd, msg_txt(39)); // Monster Summoned!!
	else
		clif_displaymessage(fd, msg_txt(40)); // Invalid Monster ID.

	return 0;
}
// big monster spawning [Valaris]
int atcommand_monsterbig(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	char name[NAME_LENGTH] = "";
	char monster[NAME_LENGTH] = "";
	int mob_id = 0;
	int number = 0;
	int x = 0;
	int y = 0;
	int count;
	int i;

	nullpo_retr(-1, sd);

	if (!message || !*message) {
		clif_displaymessage(fd, "Give a monster name/id please.");
		return -1;
	}

	if (sscanf(message, "\"%23[^\"]\" %23s %d %d %d", name, monster, &number, &x, &y) < 2 &&
	    sscanf(message, "%23s \"%23[^\"]\" %d %d %d", monster, name, &number, &x, &y) < 2 &&
	    sscanf(message, "%23s %d %23s %d %d", monster, &number, name, &x, &y) < 1) {
		clif_displaymessage(fd, "Give a monster name/id please.");
		return -1;
	}

	// If monster identifier/name argument is a name
	if ((mob_id = mobdb_searchname(monster)) == 0) // check name first (to avoid possible name begining by a number)
		mob_id = atoi(monster);

	if (mob_id == 0) {
		clif_displaymessage(fd, msg_txt(40));
		return -1;
	}

	if (mob_id == MOBID_EMPERIUM) {
		clif_displaymessage(fd, msg_txt(83));	// Cannot spawn emperium
		return -1;
	}

	if (mobdb_checkid(mob_id) == 0) {
		clif_displaymessage(fd, "Invalid monster ID"); // Invalid Monster ID.
		return -1;
	}

	if (number <= 0)
		number = 1;

	if (strlen(name) < 1)
		strcpy(name, "--ja--");

	// If value of atcommand_spawn_quantity_limit directive is greater than or equal to 1 and quantity of monsters is greater than value of the directive
	if (battle_config.atc_spawn_quantity_limit >= 1 && number > battle_config.atc_spawn_quantity_limit)
		number = battle_config.atc_spawn_quantity_limit;

	count = 0;
	for (i = 0; i < number; i++) {
		int mx, my;
		if (x <= 0)
			mx = sd->bl.x + (rand() % 11 - 5);
		else
			mx = x;
		if (y <= 0)
			my = sd->bl.y + (rand() % 11 - 5);
		else
			my = y;
		count += (mob_once_spawn(sd, sd->bl.m, mx, my, name, mob_id, 1, "4") != 0) ? 1 : 0;
	}

	if (count != 0)
		clif_displaymessage(fd, msg_txt(39)); // Monster Summoned!!
	else
		clif_displaymessage(fd, msg_txt(40)); // Invalid Monster ID.

	return 0;
}

/*==========================================
 *
 *------------------------------------------*/
static int atkillmonster_sub(struct block_list *bl, va_list ap)
{
	struct mob_data *md;
	int flag;
	
	nullpo_retr(0, ap);
	nullpo_retr(0, md=(struct mob_data *)bl);
	flag = va_arg(ap, int);

	if (md->guardian_data)
		return 0; //Do not touch WoE mobs!
	
	if (flag)
		status_zap(bl,md->status.hp, 0);
	else
		status_kill(bl);
	return 1;
}

void atcommand_killmonster_sub(const int fd, struct map_session_data* sd, const char* message, const int drop)
{
	int map_id;
	char map_name[MAP_NAME_LENGTH_EXT];

	if (!sd) return;

	memset(map_name, '\0', sizeof(map_name));

	if (!message || !*message || sscanf(message, "%15s", map_name) < 1)
		map_id = sd->bl.m;
	else {
		if ((map_id = map_mapname2mapid(map_name)) < 0)
			map_id = sd->bl.m;
	}

	map_foreachinmap(atkillmonster_sub, map_id, BL_MOB, drop);

	clif_displaymessage(fd, msg_txt(165)); // All monsters killed!

	return;
}

int atcommand_killmonster(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	atcommand_killmonster_sub(fd, sd, message, 1);
	return 0;
}

/*==========================================
 *
 *------------------------------------------*/
int atcommand_killmonster2(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	atcommand_killmonster_sub(fd, sd, message, 0);
	return 0;
}

/*==========================================
 *
 *------------------------------------------*/
int atcommand_refine(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	int i,j, position = 0, refine = 0, current_position, final_refine;
	int count;
	nullpo_retr(-1, sd);

	memset(atcmd_output, '\0', sizeof(atcmd_output));

	if (!message || !*message || sscanf(message, "%d %d", &position, &refine) < 2) {
		clif_displaymessage(fd, "Please, enter a position and a amount (usage: @refine <equip position> <+/- amount>).");
		sprintf(atcmd_output, "%d: Left Accessory", EQI_ACC_L);
		clif_displaymessage(fd, atcmd_output);
		sprintf(atcmd_output, "%d: Right Accessory", EQI_ACC_R);
		clif_displaymessage(fd, atcmd_output);
		sprintf(atcmd_output, "%d: Shoes", EQI_SHOES);
		clif_displaymessage(fd, atcmd_output);
		sprintf(atcmd_output, "%d: Garment", EQI_GARMENT);
		clif_displaymessage(fd, atcmd_output);
		sprintf(atcmd_output, "%d: Lower Headgear", EQI_HEAD_LOW);
		clif_displaymessage(fd, atcmd_output);
		sprintf(atcmd_output, "%d: Mid Headgear", EQI_HEAD_MID);
		clif_displaymessage(fd, atcmd_output);
		sprintf(atcmd_output, "%d: Top Headgear", EQI_HEAD_TOP);
		clif_displaymessage(fd, atcmd_output);
		sprintf(atcmd_output, "%d: Body Armor", EQI_ARMOR);
		clif_displaymessage(fd, atcmd_output);
		sprintf(atcmd_output, "%d: Left Hand", EQI_HAND_L);
		clif_displaymessage(fd, atcmd_output);
		sprintf(atcmd_output, "%d: Right Hand", EQI_HAND_R);
		clif_displaymessage(fd, atcmd_output);
		return -1;
	}

	refine = cap_value(refine, -MAX_REFINE, MAX_REFINE);

	count = 0;
	for (j = 0; j < EQI_MAX-1; j++) {
		if ((i = sd->equip_index[j]) < 0)
			continue;
		if(j == EQI_HAND_R && sd->equip_index[EQI_HAND_L] == i)
			continue;
		if(j == EQI_HEAD_MID && sd->equip_index[EQI_HEAD_LOW] == i)
			continue;
		if(j == EQI_HEAD_TOP && (sd->equip_index[EQI_HEAD_MID] == i || sd->equip_index[EQI_HEAD_LOW] == i))
			continue;

		if(position && !(sd->status.inventory[i].equip & position))
			continue;

		final_refine = cap_value(sd->status.inventory[i].refine + refine, 0, MAX_REFINE);
		if (sd->status.inventory[i].refine != final_refine) {
			sd->status.inventory[i].refine = final_refine;
			current_position = sd->status.inventory[i].equip;
			pc_unequipitem(sd, i, 3);
			clif_refine(fd, 0, i, sd->status.inventory[i].refine);
			clif_delitem(sd, i, 1);
			clif_additem(sd, i, 1, 0);
			pc_equipitem(sd, i, current_position);
			clif_misceffect(&sd->bl, 3);
			count++;
		}
	}

	if (count == 0)
		clif_displaymessage(fd, msg_txt(166)); // No item has been refined!
	else if (count == 1)
		clif_displaymessage(fd, msg_txt(167)); // 1 item has been refined!
	else {
		sprintf(atcmd_output, msg_txt(168), count); // %d items have been refined!
		clif_displaymessage(fd, atcmd_output);
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------*/
int atcommand_produce(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	char item_name[100];
	int item_id, attribute = 0, star = 0;
	int flag = 0;
	struct item_data *item_data;
	struct item tmp_item;
	nullpo_retr(-1, sd);

	memset(atcmd_output, '\0', sizeof(atcmd_output));
	memset(item_name, '\0', sizeof(item_name));

	if (!message || !*message || (
		sscanf(message, "\"%99[^\"]\" %d %d", item_name, &attribute, &star) < 1 &&
		sscanf(message, "%99s %d %d", item_name, &attribute, &star) < 1
	)) {
		clif_displaymessage(fd, "Please, enter at least an item name/id (usage: @produce <equip name or equip ID> <element> <# of very's>).");
		return -1;
	}

	item_id = 0;
	if ((item_data = itemdb_searchname(item_name)) == NULL &&
	    (item_data = itemdb_exists(atoi(item_name))) == NULL)
	{
		sprintf(atcmd_output, msg_txt(170)); // This item is not an equipment.
		clif_displaymessage(fd, atcmd_output);
		return -1;
	}
	item_id = item_data->nameid;
	if (itemdb_isequip2(item_data)) {
		if (attribute < MIN_ATTRIBUTE || attribute > MAX_ATTRIBUTE)
			attribute = ATTRIBUTE_NORMAL;
		if (star < MIN_STAR || star > MAX_STAR)
			star = 0;
		memset(&tmp_item, 0, sizeof tmp_item);
		tmp_item.nameid = item_id;
		tmp_item.amount = 1;
		tmp_item.identify = 1;
		tmp_item.card[0] = CARD0_FORGE;
		tmp_item.card[1] = item_data->type==IT_WEAPON?
			((star*5) << 8) + attribute:0;
		tmp_item.card[2] = GetWord(sd->status.char_id, 0);
		tmp_item.card[3] = GetWord(sd->status.char_id, 1);
		clif_produceeffect(sd, 0, item_id);
		clif_misceffect(&sd->bl, 3);

		//Logs (A)dmins items [Lupus]
		if(log_config.enable_logs&0x400)
			log_pick_pc(sd, "A", tmp_item.nameid, 1, &tmp_item);

		if ((flag = pc_additem(sd, &tmp_item, 1)))
			clif_additem(sd, 0, 0, flag);
	} else {
		sprintf(atcmd_output, msg_txt(169), item_id, item_data->name); // This item (%d: '%s') is not an equipment.
		clif_displaymessage(fd, atcmd_output);
		return -1;
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------*/
int atcommand_memo(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	int position = 0;
	nullpo_retr(-1, sd);

	memset(atcmd_output, '\0', sizeof(atcmd_output));

	if( !message || !*message || sscanf(message, "%d", &position) < 1 )
	{
		int i;
		clif_displaymessage(sd->fd,  "Your actual memo positions are:");
		for( i = 0; i < MAX_MEMOPOINTS; i++ )
		{
			if( sd->status.memo_point[i].map )
				sprintf(atcmd_output, "%d - %s (%d,%d)", i, mapindex_id2name(sd->status.memo_point[i].map), sd->status.memo_point[i].x, sd->status.memo_point[i].y);
			else
				sprintf(atcmd_output, msg_txt(171), i); // %d - void
			clif_displaymessage(sd->fd, atcmd_output);
 		}
		return 0;
 	}
 
	if( position < 0 && position >= MAX_MEMOPOINTS )
	{
		sprintf(atcmd_output, "Please, enter a valid position (usage: @memo <memo_position:%d-%d>).", 0, MAX_MEMOPOINTS-1);
		clif_displaymessage(fd, atcmd_output);
		return -1;
	}

	pc_memo(sd, position);
	return 0;
}

/*==========================================
 *
 *------------------------------------------*/
int atcommand_gat(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	int y;
	nullpo_retr(-1, sd);

	memset(atcmd_output, '\0', sizeof(atcmd_output));

	for (y = 2; y >= -2; y--) {
		sprintf(atcmd_output, "%s (x= %d, y= %d) %02X %02X %02X %02X %02X",
			map[sd->bl.m].name,   sd->bl.x - 2, sd->bl.y + y,
 			map_getcell(sd->bl.m, sd->bl.x - 2, sd->bl.y + y, CELL_GETTYPE),
 			map_getcell(sd->bl.m, sd->bl.x - 1, sd->bl.y + y, CELL_GETTYPE),
 			map_getcell(sd->bl.m, sd->bl.x,     sd->bl.y + y, CELL_GETTYPE),
 			map_getcell(sd->bl.m, sd->bl.x + 1, sd->bl.y + y, CELL_GETTYPE),
 			map_getcell(sd->bl.m, sd->bl.x + 2, sd->bl.y + y, CELL_GETTYPE));

		clif_displaymessage(fd, atcmd_output);
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------*/
int atcommand_displaystatus(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	int i, type, flag;
	nullpo_retr(-1, sd);
	
	if (!message || !*message || (i = sscanf(message, "%d %d", &type, &flag)) < 1) {
		clif_displaymessage(fd, "Please, enter a status type/flag (usage: @displaystatus <status type> <flag>).");
		return -1;
	}
	if (i == 1) flag = 1;

	clif_status_change(&sd->bl, type, flag);

	return 0;
}

/*==========================================
 * @stpoint (Rewritten by [Yor])
 *------------------------------------------*/
int atcommand_statuspoint(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	int point, new_status_point;

	if (!message || !*message || (point = atoi(message)) == 0) {
		clif_displaymessage(fd, "Please, enter a number (usage: @stpoint <number of points>).");
		return -1;
	}

	if (point > 0 && sd->status.status_point > USHRT_MAX - point)
		new_status_point = USHRT_MAX;
	else
	if (point < 0 && sd->status.status_point < -point)
		new_status_point = 0;
	else
		new_status_point = sd->status.status_point + point;
	if (new_status_point != (int)sd->status.status_point) {
		sd->status.status_point = (unsigned short)new_status_point;
		clif_updatestatus(sd, SP_STATUSPOINT);
		clif_displaymessage(fd, msg_txt(174)); // Number of status points changed!
	} else {
		if (point < 0)
			clif_displaymessage(fd, msg_txt(41)); // Impossible to decrease the number/value.
		else
			clif_displaymessage(fd, msg_txt(149)); // Impossible to increase the number/value.
		return -1;
	}

	return 0;
}

/*==========================================
 * @skpoint (Rewritten by [Yor])
 *------------------------------------------*/
int atcommand_skillpoint(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	int point, new_skill_point;
	nullpo_retr(-1, sd);

	if (!message || !*message || (point = atoi(message)) == 0) {
		clif_displaymessage(fd, "Please, enter a number (usage: @skpoint <number of points>).");
		return -1;
	}

	if (point > 0 && sd->status.skill_point > USHRT_MAX - point)
		new_skill_point = USHRT_MAX;
	else if (point < 0 && sd->status.skill_point < -point)
		new_skill_point = 0;
	else
		new_skill_point = sd->status.skill_point + point;
	
	if (new_skill_point != (int)sd->status.skill_point) {
		sd->status.skill_point = (unsigned short)new_skill_point;
		clif_updatestatus(sd, SP_SKILLPOINT);
		clif_displaymessage(fd, msg_txt(175)); // Number of skill points changed!
	} else {
		if (point < 0)
			clif_displaymessage(fd, msg_txt(41)); // Impossible to decrease the number/value.
		else
			clif_displaymessage(fd, msg_txt(149)); // Impossible to increase the number/value.
		return -1;
	}

	return 0;
}

/*==========================================
 * @zeny (Rewritten by [Yor])
 *------------------------------------------*/
int atcommand_zeny(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	int zeny, new_zeny;
	nullpo_retr(-1, sd);

	if (!message || !*message || (zeny = atoi(message)) == 0) {
		clif_displaymessage(fd, "Please, enter an amount (usage: @zeny <amount>).");
		return -1;
	}

	new_zeny = sd->status.zeny + zeny;
	if (zeny > 0 && (zeny > MAX_ZENY || new_zeny > MAX_ZENY)) // fix positiv overflow
		new_zeny = MAX_ZENY;
	else if (zeny < 0 && (zeny < -MAX_ZENY || new_zeny < 0)) // fix negativ overflow
		new_zeny = 0;

	if (new_zeny != sd->status.zeny) {
		sd->status.zeny = new_zeny;
		clif_updatestatus(sd, SP_ZENY);
		clif_displaymessage(fd, msg_txt(176)); // Number of zenys changed!
	} else {
		if (zeny < 0)
			clif_displaymessage(fd, msg_txt(41)); // Impossible to decrease the number/value.
		else
			clif_displaymessage(fd, msg_txt(149)); // Impossible to increase the number/value.
		return -1;
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------*/
int atcommand_param(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	int i, value = 0, new_value, max;
	const char* param[] = { "@str", "@agi", "@vit", "@int", "@dex", "@luk", NULL };
	short* status[6];
 	//we don't use direct initialization because it isn't part of the c standard.
	nullpo_retr(-1, sd);
	
	memset(atcmd_output, '\0', sizeof(atcmd_output));

	if (!message || !*message || sscanf(message, "%d", &value) < 1 || value == 0) {
		sprintf(atcmd_output, "Please, enter a valid value (usage: @str,@agi,@vit,@int,@dex,@luk <+/-adjustment>).");
		clif_displaymessage(fd, atcmd_output);
		return -1;
	}

	for (i = 0; param[i] != NULL; i++)
		if (strcmpi(command, param[i]) == 0)
			break;

	if (param[i] == NULL || i > MAX_STATUS_TYPE) { // normally impossible...
		sprintf(atcmd_output, "Please, enter a valid value (usage: @str,@agi,@vit,@int,@dex,@luk <+/-adjustment>).");
		clif_displaymessage(fd, atcmd_output);
		return -1;
	}

	status[0] = &sd->status.str;
	status[1] = &sd->status.agi;
	status[2] = &sd->status.vit;
	status[3] = &sd->status.int_;
	status[4] = &sd->status.dex;
	status[5] = &sd->status.luk;

	max = SHRT_MAX;
	new_value = cap_value(*status[i] + value, 1, max);
	
	if (new_value != (int)*status[i]) {
		*status[i] = new_value;
		clif_updatestatus(sd, SP_STR + i);
		clif_updatestatus(sd, SP_USTR + i);
		status_calc_pc(sd, 0);
		clif_displaymessage(fd, msg_txt(42)); // Stat changed.
	} else {
		if (value < 0)
			clif_displaymessage(fd, msg_txt(41)); // Impossible to decrease the number/value.
		else
			clif_displaymessage(fd, msg_txt(149)); // Impossible to increase the number/value.
		return -1;
	}

	return 0;
}

/*==========================================
 * Stat all by fritz (rewritten by [Yor])
 *------------------------------------------*/
int atcommand_stat_all(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	int index, count, value, max, new_value;
	short* status[6];
 	//we don't use direct initialization because it isn't part of the c standard.
	nullpo_retr(-1, sd);
	
	status[0] = &sd->status.str;
	status[1] = &sd->status.agi;
	status[2] = &sd->status.vit;
	status[3] = &sd->status.int_;
	status[4] = &sd->status.dex;
	status[5] = &sd->status.luk;

	if (!message || !*message || sscanf(message, "%d", &value) < 1 || value == 0) {
		value = pc_maxparameter(sd);
		max = pc_maxparameter(sd);
	} else {
		max = SHRT_MAX;
	}

	count = 0;
	for (index = 0; index < ARRAYLENGTH(status); index++) {

		if (value > 0 && *status[index] > max - value)
			new_value = max;
		else if (value < 0 && *status[index] <= -value)
			new_value = 1;
		else
			new_value = *status[index] +value;
		
		if (new_value != (int)*status[index]) {
			*status[index] = new_value;
			clif_updatestatus(sd, SP_STR + index);
			clif_updatestatus(sd, SP_USTR + index);
			count++;
		}
	}

	if (count > 0) { // if at least 1 stat modified
		status_calc_pc(sd, 0);
		clif_displaymessage(fd, msg_txt(84)); // All stats changed!
	} else {
		if (value < 0)
			clif_displaymessage(fd, msg_txt(177)); // Impossible to decrease a stat.
		else
			clif_displaymessage(fd, msg_txt(178)); // Impossible to increase a stat.
		return -1;
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------*/
int atcommand_guildlevelup(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	int level = 0;
	short added_level;
	struct guild *guild_info;
	nullpo_retr(-1, sd);

	if (!message || !*message || sscanf(message, "%d", &level) < 1 || level == 0) {
		clif_displaymessage(fd, "Please, enter a valid level (usage: @guildlvup/@guildlvlup <# of levels>).");
		return -1;
	}

	if (sd->status.guild_id <= 0 || (guild_info = guild_search(sd->status.guild_id)) == NULL) {
		clif_displaymessage(fd, msg_txt(43)); // You're not in a guild.
		return -1;
	}
	//if (strcmp(sd->status.name, guild_info->master) != 0) {
	//	clif_displaymessage(fd, msg_txt(44)); // You're not the master of your guild.
	//	return -1;
	//}

	added_level = (short)level;
	if (level > 0 && (level > MAX_GUILDLEVEL || added_level > ((short)MAX_GUILDLEVEL - guild_info->guild_lv))) // fix positiv overflow
		added_level = (short)MAX_GUILDLEVEL - guild_info->guild_lv;
	else if (level < 0 && (level < -MAX_GUILDLEVEL || added_level < (1 - guild_info->guild_lv))) // fix negativ overflow
		added_level = 1 - guild_info->guild_lv;

	if (added_level != 0) {
		intif_guild_change_basicinfo(guild_info->guild_id, GBI_GUILDLV, &added_level, 2);
		clif_displaymessage(fd, msg_txt(179)); // Guild level changed.
	} else {
		clif_displaymessage(fd, msg_txt(45)); // Guild level change failed.
		return -1;
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------*/
int atcommand_makeegg(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	struct item_data *item_data;
	int id, pet_id;
	nullpo_retr(-1, sd);

	if (!message || !*message) {
		clif_displaymessage(fd, "Please, enter a monster/egg name/id (usage: @makeegg <pet>).");
		return -1;
	}

	if ((item_data = itemdb_searchname(message)) != NULL) // for egg name
		id = item_data->nameid;
	else
	if ((id = mobdb_searchname(message)) != 0) // for monster name
		;
	else
		id = atoi(message);

	pet_id = search_petDB_index(id, PET_CLASS);
	if (pet_id < 0)
		pet_id = search_petDB_index(id, PET_EGG);
	if (pet_id >= 0) {
		sd->catch_target_class = pet_db[pet_id].class_;
		intif_create_pet(
			sd->status.account_id, sd->status.char_id,
			(short)pet_db[pet_id].class_, (short)mob_db(pet_db[pet_id].class_)->lv,
			(short)pet_db[pet_id].EggID, 0, (short)pet_db[pet_id].intimate,
			100, 0, 1, pet_db[pet_id].jname);
	} else {
		clif_displaymessage(fd, msg_txt(180)); // The monster/egg name/id doesn't exist.
		return -1;
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------*/
int atcommand_hatch(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	nullpo_retr(-1, sd);
	if (sd->status.pet_id <= 0)
		clif_sendegg(sd);
	else {
		clif_displaymessage(fd, msg_txt(181)); // You already have a pet.
		return -1;
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------*/
int atcommand_petfriendly(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	int friendly;
	struct pet_data *pd;
	nullpo_retr(-1, sd);

	if (!message || !*message || (friendly = atoi(message)) < 0) {
		clif_displaymessage(fd, "Please, enter a valid value (usage: @petfriendly <0-1000>).");
		return -1;
	}

	pd = sd->pd;
	if (!pd) {
		clif_displaymessage(fd, msg_txt(184)); // Sorry, but you have no pet.
		return -1;
	}
	
	if (friendly < 0 || friendly > 1000)
	{
		clif_displaymessage(fd, msg_txt(37)); // An invalid number was specified.
		return -1;
	}
	
	if (friendly == pd->pet.intimate) {
		clif_displaymessage(fd, msg_txt(183)); // Pet friendly is already the good value.
		return -1;
	}
	pd->pet.intimate = friendly;
	clif_send_petstatus(sd);
	clif_displaymessage(fd, msg_txt(182)); // Pet friendly value changed!
	return 0;
}

/*==========================================
 *
 *------------------------------------------*/
int atcommand_pethungry(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	int hungry;
	struct pet_data *pd;
	nullpo_retr(-1, sd);

	if (!message || !*message || (hungry = atoi(message)) < 0) {
		clif_displaymessage(fd, "Please, enter a valid number (usage: @pethungry <0-100>).");
		return -1;
	}

	pd = sd->pd;
	if (!sd->status.pet_id || !pd) {
		clif_displaymessage(fd, msg_txt(184)); // Sorry, but you have no pet.
		return -1;
	}
	if (hungry < 0 || hungry > 100) {
		clif_displaymessage(fd, msg_txt(37)); // An invalid number was specified.
		return -1;
	}
	if (hungry == pd->pet.hungry) {
		clif_displaymessage(fd, msg_txt(186)); // Pet hungry is already the good value.
		return -1;
	}

	pd->pet.hungry = hungry;
	clif_send_petstatus(sd);
	clif_displaymessage(fd, msg_txt(185)); // Pet hungry value changed!

	return 0;
}

/*==========================================
 *
 *------------------------------------------*/
int atcommand_petrename(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	struct pet_data *pd;
	nullpo_retr(-1, sd);
	if (!sd->status.pet_id || !sd->pd) {
		clif_displaymessage(fd, msg_txt(184)); // Sorry, but you have no pet.
		return -1;
	}
	pd = sd->pd;
	if (!pd->pet.rename_flag) {
		clif_displaymessage(fd, msg_txt(188)); // You can already rename your pet.
		return -1;
	}

	pd->pet.rename_flag = 0;
	intif_save_petdata(sd->status.account_id, &pd->pet);
	clif_send_petstatus(sd);
	clif_displaymessage(fd, msg_txt(187)); // You can now rename your pet.

	return 0;
}

/*==========================================
 *
 *------------------------------------------*/
int atcommand_recall(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	struct map_session_data *pl_sd = NULL;

	nullpo_retr(-1, sd);

	if (!message || !*message) {
		clif_displaymessage(fd, "Please, enter a player name (usage: @recall <player name/id>).");
		return -1;
	}

	if((pl_sd=map_nick2sd((char *)message)) == NULL && (pl_sd=map_charid2sd(atoi(message))) == NULL)
	{
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}
		
	if (pl_sd == sd)
	{
		clif_displaymessage(fd, "You are already where you are...");
		return -1;
	}

	if ( pc_isGM(sd) < pc_isGM(pl_sd) )
	{
		clif_displaymessage(fd, msg_txt(81)); // Your GM level don't authorise you to do this action on this player.
		return -1;
	}
	
	if (sd->bl.m >= 0 && map[sd->bl.m].flag.nowarpto && battle_config.any_warp_GM_min_level > pc_isGM(sd)) {
		clif_displaymessage(fd, "You are not authorised to warp somenone to your actual map.");
		return -1;
	}
	if (pl_sd->bl.m >= 0 && map[pl_sd->bl.m].flag.nowarp && battle_config.any_warp_GM_min_level > pc_isGM(sd)) {
		clif_displaymessage(fd, "You are not authorized to warp this player from its actual map.");
		return -1;
	}
	pc_setpos(pl_sd, sd->mapindex, sd->bl.x, sd->bl.y, 2);
	sprintf(atcmd_output, msg_txt(46), pl_sd->status.name); // %s recalled!
	clif_displaymessage(fd, atcmd_output);

	return 0;
}

/*==========================================
 * charblock command (usage: charblock <player_name>)
 * This command do a definitiv ban on a player
 *------------------------------------------*/
int atcommand_char_block(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	nullpo_retr(-1, sd);

	memset(atcmd_player_name, '\0', sizeof(atcmd_player_name));

	if (!message || !*message || sscanf(message, "%23[^\n]", atcmd_player_name) < 1) {
		clif_displaymessage(fd, "Please, enter a player name (usage: @charblock/@block <name>).");
		return -1;
	}

	chrif_char_ask_name(sd->status.account_id, atcmd_player_name, 1, 0, 0, 0, 0, 0, 0); // type: 1 - block
	clif_displaymessage(fd, msg_txt(88)); // Character name sent to char-server to ask it.

	return 0;
}

/*==========================================
 * charban command (usage: charban <time> <player_name>)
 * This command do a limited ban on a player
 * Time is done as follows:
 *   Adjustment value (-1, 1, +1, etc...)
 *   Modified element:
 *     a or y: year
 *     m:  month
 *     j or d: day
 *     h:  hour
 *     mn: minute
 *     s:  second
 * <example> @ban +1m-2mn1s-6y test_player
 *           this example adds 1 month and 1 second, and substracts 2 minutes and 6 years at the same time.
 *------------------------------------------*/
int atcommand_char_ban(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	char * modif_p;
	int year, month, day, hour, minute, second, value;
	nullpo_retr(-1, sd);

	memset(atcmd_output, '\0', sizeof(atcmd_output));
	memset(atcmd_player_name, '\0', sizeof(atcmd_player_name));

	if (!message || !*message || sscanf(message, "%s %23[^\n]", atcmd_output, atcmd_player_name) < 2) {
		clif_displaymessage(fd, "Please, enter ban time and a player name (usage: @charban/@ban/@banish/@charbanish <time> <name>).");
		return -1;
	}

	atcmd_output[sizeof(atcmd_output)-1] = '\0';

	modif_p = atcmd_output;
	year = month = day = hour = minute = second = 0;
	while (modif_p[0] != '\0') {
		value = atoi(modif_p);
		if (value == 0)
			modif_p++;
		else {
			if (modif_p[0] == '-' || modif_p[0] == '+')
				modif_p++;
			while (modif_p[0] >= '0' && modif_p[0] <= '9')
				modif_p++;
			if (modif_p[0] == 's') {
				second = value;
				modif_p++;
			} else if (modif_p[0] == 'n' || (modif_p[0] == 'm' && modif_p[1] == 'n')) {
				minute = value;
				modif_p = modif_p + 2;
			} else if (modif_p[0] == 'h') {
				hour = value;
				modif_p++;
			} else if (modif_p[0] == 'd' || modif_p[0] == 'j') {
				day = value;
				modif_p++;
			} else if (modif_p[0] == 'm') {
				month = value;
				modif_p++;
			} else if (modif_p[0] == 'y' || modif_p[0] == 'a') {
				year = value;
				modif_p++;
			} else if (modif_p[0] != '\0') {
				modif_p++;
			}
		}
	}
	if (year == 0 && month == 0 && day == 0 && hour == 0 && minute == 0 && second == 0) {
		clif_displaymessage(fd, msg_txt(85)); // Invalid time for ban command.
		return -1;
	}

	chrif_char_ask_name(sd->status.account_id, atcmd_player_name, 2, year, month, day, hour, minute, second); // type: 2 - ban
	clif_displaymessage(fd, msg_txt(88)); // Character name sent to char-server to ask it.

	return 0;
}

/*==========================================
 * charunblock command (usage: charunblock <player_name>)
 *------------------------------------------*/
int atcommand_char_unblock(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	nullpo_retr(-1, sd);

	memset(atcmd_player_name, '\0', sizeof(atcmd_player_name));

	if (!message || !*message || sscanf(message, "%23[^\n]", atcmd_player_name) < 1) {
		clif_displaymessage(fd, "Please, enter a player name (usage: @charunblock <player_name>).");
		return -1;
	}

	// send answer to login server via char-server
	chrif_char_ask_name(sd->status.account_id, atcmd_player_name, 3, 0, 0, 0, 0, 0, 0); // type: 3 - unblock
	clif_displaymessage(fd, msg_txt(88)); // Character name sent to char-server to ask it.

	return 0;
}

/*==========================================
 * charunban command (usage: charunban <player_name>)
 *------------------------------------------*/
int atcommand_char_unban(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	nullpo_retr(-1, sd);

	memset(atcmd_player_name, '\0', sizeof(atcmd_player_name));

	if (!message || !*message || sscanf(message, "%23[^\n]", atcmd_player_name) < 1) {
		clif_displaymessage(fd, "Please, enter a player name (usage: @charunban <player_name>).");
		return -1;
	}

	// send answer to login server via char-server
	chrif_char_ask_name(sd->status.account_id, atcmd_player_name, 4, 0, 0, 0, 0, 0, 0); // type: 4 - unban
	clif_displaymessage(fd, msg_txt(88)); // Character name sent to char-server to ask it.

	return 0;
}

/*==========================================
 *
 *------------------------------------------*/
int atcommand_night(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	nullpo_retr(-1, sd);

	if (night_flag != 1) {
		map_night_timer(night_timer_tid, 0, 0, 1);
	} else {
		clif_displaymessage(fd, msg_txt(89)); // Sorry, it's already the night. Impossible to execute the command.
		return -1;
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------*/
int atcommand_day(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	nullpo_retr(-1, sd);

	if (night_flag != 0) {
		map_day_timer(day_timer_tid, 0, 0, 1);
	} else {
		clif_displaymessage(fd, msg_txt(90)); // Sorry, it's already the day. Impossible to execute the command.
		return -1;
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------*/
int atcommand_doom(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	struct map_session_data* pl_sd;
	struct s_mapiterator* iter;

	nullpo_retr(-1, sd);
	clif_specialeffect(&sd->bl,450,ALL_SAMEMAP);

	iter = mapit_getallusers();
	for( pl_sd = (TBL_PC*)mapit_first(iter); mapit_exists(iter); pl_sd = (TBL_PC*)mapit_next(iter) )
	{
		if (pl_sd->fd != fd && pc_isGM(sd) >= pc_isGM(pl_sd))
		{
			status_kill(&pl_sd->bl);
			clif_displaymessage(pl_sd->fd, msg_txt(61)); // The holy messenger has given judgement.
		}
	}
	mapit_free(iter);

	clif_displaymessage(fd, msg_txt(62)); // Judgement was made.

	return 0;
}

/*==========================================
 *
 *------------------------------------------*/
int atcommand_doommap(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	struct map_session_data* pl_sd;
	struct s_mapiterator* iter;

	nullpo_retr(-1, sd);
	clif_specialeffect(&sd->bl,450,ALL_CLIENT);

	iter = mapit_getallusers();
	for( pl_sd = (TBL_PC*)mapit_first(iter); mapit_exists(iter); pl_sd = (TBL_PC*)mapit_next(iter) )
	{
		if (pl_sd->fd != fd && sd->bl.m == pl_sd->bl.m && pc_isGM(sd) >= pc_isGM(pl_sd))
		{
			status_kill(&pl_sd->bl);
			clif_displaymessage(pl_sd->fd, msg_txt(61)); // The holy messenger has given judgement.
		}
	}
	mapit_free(iter);

	clif_displaymessage(fd, msg_txt(62)); // Judgement was made.

	return 0;
}

/*==========================================
 *
 *------------------------------------------*/
static void atcommand_raise_sub(struct map_session_data* sd)
{
	if (!status_isdead(&sd->bl))
		return;

	if(!status_revive(&sd->bl, 100, 100))
		return;
	clif_skill_nodamage(&sd->bl,&sd->bl,ALL_RESURRECTION,4,1);
	clif_displaymessage(sd->fd, msg_txt(63)); // Mercy has been shown.
}

/*==========================================
 *
 *------------------------------------------*/
int atcommand_raise(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	struct map_session_data* pl_sd;
	struct s_mapiterator* iter;
		
	nullpo_retr(-1, sd);

	iter = mapit_getallusers();
	for( pl_sd = (TBL_PC*)mapit_first(iter); mapit_exists(iter); pl_sd = (TBL_PC*)mapit_next(iter) )
		atcommand_raise_sub(pl_sd);
	mapit_free(iter);

	clif_displaymessage(fd, msg_txt(64)); // Mercy has been granted.

	return 0;
}

/*==========================================
 *
 *------------------------------------------*/
int atcommand_raisemap(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	struct map_session_data* pl_sd;
	struct s_mapiterator* iter;

	nullpo_retr(-1, sd);

	iter = mapit_getallusers();
	for( pl_sd = (TBL_PC*)mapit_first(iter); mapit_exists(iter); pl_sd = (TBL_PC*)mapit_next(iter) )
		if (sd->bl.m == pl_sd->bl.m)
			atcommand_raise_sub(pl_sd);
	mapit_free(iter);

	clif_displaymessage(fd, msg_txt(64)); // Mercy has been granted.

	return 0;
}

/*==========================================
 *
 *------------------------------------------*/
int atcommand_kick(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	struct map_session_data *pl_sd;
	nullpo_retr(-1, sd);

	memset(atcmd_player_name, '\0', sizeof(atcmd_player_name));

	if (!message || !*message) {
		clif_displaymessage(fd, "Please, enter a player name (usage: @kick <player name/id>).");
		return -1;
	}

	if((pl_sd=map_nick2sd((char *)message)) == NULL && (pl_sd=map_charid2sd(atoi(message))) == NULL)
	{
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	if ( pc_isGM(sd) < pc_isGM(pl_sd) )
	{
		clif_displaymessage(fd, msg_txt(81)); // Your GM level don't authorise you to do this action on this player.
		return -1;
	}
	
	clif_GM_kick(sd, pl_sd);
	
	return 0;
}

/*==========================================
 *
 *------------------------------------------*/
int atcommand_kickall(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	struct map_session_data* pl_sd;
	struct s_mapiterator* iter;
	nullpo_retr(-1, sd);

	iter = mapit_getallusers();
	for( pl_sd = (TBL_PC*)mapit_first(iter); mapit_exists(iter); pl_sd = (TBL_PC*)mapit_next(iter) )
	{
		if (pc_isGM(sd) >= pc_isGM(pl_sd)) { // you can kick only lower or same gm level
			if (sd->status.account_id != pl_sd->status.account_id)
				clif_GM_kick(NULL, pl_sd);
		}
	}
	mapit_free(iter);

	clif_displaymessage(fd, msg_txt(195)); // All players have been kicked!

	return 0;
}

/*==========================================
 *
 *------------------------------------------*/
int atcommand_allskill(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	nullpo_retr(-1, sd);
	pc_allskillup(sd); // all skills
	sd->status.skill_point = 0; // 0 skill points
	clif_updatestatus(sd, SP_SKILLPOINT); // update
	clif_displaymessage(fd, msg_txt(76)); // You have received all skills.

	return 0;
}

/*==========================================
 *
 *------------------------------------------*/
int atcommand_questskill(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	int skill_id;
	nullpo_retr(-1, sd);

	if (!message || !*message || (skill_id = atoi(message)) < 0) {
		clif_displaymessage(fd, "Please, enter a quest skill number (usage: @questskill <#:0+>).");
		return -1;
	}
	if (skill_id < 0 && skill_id >= MAX_SKILL_DB) {
		clif_displaymessage(fd, msg_txt(198)); // This skill number doesn't exist.
		return -1;
	}
	if (!(skill_get_inf2(skill_id) & INF2_QUEST_SKILL)) {
		clif_displaymessage(fd, msg_txt(197)); // This skill number doesn't exist or isn't a quest skill.
		return -1;
	}
	if (pc_checkskill(sd, skill_id) > 0) {
		clif_displaymessage(fd, msg_txt(196)); // You already have this quest skill.
		return -1;
	}

	pc_skill(sd, skill_id, 1, 0);
	clif_displaymessage(fd, msg_txt(70)); // You have learned the skill.

	return 0;
}

/*==========================================
 *
 *------------------------------------------*/
int atcommand_lostskill(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	int skill_id;
	nullpo_retr(-1, sd);

	if (!message || !*message || (skill_id = atoi(message)) < 0) {
		clif_displaymessage(fd, "Please, enter a quest skill number (usage: @lostskill <#:0+>).");
		return -1;
	}
	if (skill_id < 0 && skill_id >= MAX_SKILL) {
		clif_displaymessage(fd, msg_txt(198)); // This skill number doesn't exist.
		return -1;
	}
	if (!(skill_get_inf2(skill_id) & INF2_QUEST_SKILL)) {
		clif_displaymessage(fd, msg_txt(197)); // This skill number doesn't exist or isn't a quest skill.
		return -1;
	}
	if (pc_checkskill(sd, skill_id) == 0) {
		clif_displaymessage(fd, msg_txt(201)); // You don't have this quest skill.
		return -1;
	}

	sd->status.skill[skill_id].lv = 0;
	sd->status.skill[skill_id].flag = 0;
	clif_skillinfoblock(sd);
	clif_displaymessage(fd, msg_txt(71)); // You have forgotten the skill.

	return 0;
}

/*==========================================
 *
 *------------------------------------------*/
int atcommand_spiritball(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	int number;
	nullpo_retr(-1, sd);

	if (!message || !*message || (number = atoi(message)) < 0) {
		clif_displaymessage(fd, "Please, enter a spirit ball number (usage: @spiritball <number: 0-1000>).");
		return -1;
	}

	// set max number to avoid server/client crash (500 create big balls of several balls: no visial difference with more)
	if (number > 500)
		number = 500;

	if (number >= 0 && number <= 0x7FFF) {
		if (sd->spiritball != number || number > 499) {
			if (sd->spiritball > 0)
				pc_delspiritball(sd, sd->spiritball, 1);
			sd->spiritball = number;
			clif_spiritball(sd);
			// no message, player can look the difference
			if (number > 1000)
				clif_displaymessage(fd, msg_txt(204)); // WARNING: more than 1000 spiritballs can CRASH your server and/or client!
		} else {
			clif_displaymessage(fd, msg_txt(205)); // You already have this number of spiritballs.
			return -1;
		}
	} else {
		clif_displaymessage(fd, msg_txt(37)); // An invalid number was specified.
		return -1;
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------*/
int atcommand_party(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	char party[NAME_LENGTH];
	nullpo_retr(-1, sd);

	memset(party, '\0', sizeof(party));

	if (!message || !*message || sscanf(message, "%23[^\n]", party) < 1) {
		clif_displaymessage(fd, "Please, enter a party name (usage: @party <party_name>).");
		return -1;
	}

	party_create(sd, party, 0, 0);

	return 0;
}

/*==========================================
 *
 *------------------------------------------*/
int atcommand_guild(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	char guild[NAME_LENGTH];
	int prev;
	nullpo_retr(-1, sd);

	memset(guild, '\0', sizeof(guild));

	if (!message || !*message || sscanf(message, "%23[^\n]", guild) < 1) {
		clif_displaymessage(fd, "Please, enter a guild name (usage: @guild <guild_name>).");
		return -1;
	}

	prev = battle_config.guild_emperium_check;
	battle_config.guild_emperium_check = 0;
	guild_create(sd, guild);
	battle_config.guild_emperium_check = prev;

	return 0;
}

/*==========================================
 *
 *------------------------------------------*/
int atcommand_agitstart(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	nullpo_retr(-1, sd);
	if (agit_flag == 1) {
		clif_displaymessage(fd, msg_txt(73)); // Already it has started siege warfare.
		return -1;
	}

	agit_flag = 1;
	guild_agit_start();
	clif_displaymessage(fd, msg_txt(72)); // Guild siege warfare start!

	return 0;
}

/*==========================================
 *
 *------------------------------------------*/
int atcommand_agitend(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	nullpo_retr(-1, sd);
	if (agit_flag == 0) {
		clif_displaymessage(fd, msg_txt(75)); // Siege warfare hasn't started yet.
		return -1;
	}

	agit_flag = 0;
	guild_agit_end();
	clif_displaymessage(fd, msg_txt(74)); // Guild siege warfare end!

	return 0;
}

/*==========================================
 * @mapexit - shuts down the map server
 *------------------------------------------*/
int atcommand_mapexit(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	struct map_session_data* pl_sd;
	struct s_mapiterator* iter;

	nullpo_retr(-1, sd);

	iter = mapit_getallusers();
	for( pl_sd = (TBL_PC*)mapit_first(iter); mapit_exists(iter); pl_sd = (TBL_PC*)mapit_next(iter) )
		if (sd->status.account_id != pl_sd->status.account_id)
			clif_GM_kick(NULL, pl_sd);
	mapit_free(iter);

	clif_GM_kick(NULL, sd);
	
	flush_fifos();

	runflag = 0;

	return 0;
}

/*==========================================
 * idsearch <part_of_name>: revrited by [Yor]
 *------------------------------------------*/
int atcommand_idsearch(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	char item_name[100];
	unsigned int i, match;
	struct item_data *item_array[MAX_SEARCH];
	nullpo_retr(-1, sd);

	memset(item_name, '\0', sizeof(item_name));
	memset(atcmd_output, '\0', sizeof(atcmd_output));

	if (!message || !*message || sscanf(message, "%99s", item_name) < 0) {
		clif_displaymessage(fd, "Please, enter a part of item name (usage: @idsearch <part_of_item_name>).");
		return -1;
	}

	sprintf(atcmd_output, msg_txt(77), item_name); // The reference result of '%s' (name: id):
	clif_displaymessage(fd, atcmd_output);
	match = itemdb_searchname_array(item_array, MAX_SEARCH, item_name);
	if (match > MAX_SEARCH) {
		sprintf(atcmd_output, msg_txt(269), MAX_SEARCH, match);
		clif_displaymessage(fd, atcmd_output);
		match = MAX_SEARCH;
	}
	for(i = 0; i < match; i++) {
		sprintf(atcmd_output, msg_txt(78), item_array[i]->jname, item_array[i]->nameid); // %s: %d
		clif_displaymessage(fd, atcmd_output);
	}
	sprintf(atcmd_output, msg_txt(79), match); // It is %d affair above.
	clif_displaymessage(fd, atcmd_output);

	return 0;
}

/*==========================================
 * Recall All Characters Online To Your Location
 *------------------------------------------*/
int atcommand_recallall(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	struct map_session_data* pl_sd;
	struct s_mapiterator* iter;
	int count;
	nullpo_retr(-1, sd);

	memset(atcmd_output, '\0', sizeof(atcmd_output));

	if (sd->bl.m >= 0 && map[sd->bl.m].flag.nowarpto && battle_config.any_warp_GM_min_level > pc_isGM(sd)) {
		clif_displaymessage(fd, "You are not authorised to warp somenone to your actual map.");
		return -1;
	}

	count = 0;
	iter = mapit_getallusers();
	for( pl_sd = (TBL_PC*)mapit_first(iter); mapit_exists(iter); pl_sd = (TBL_PC*)mapit_next(iter) )
	{
		if (sd->status.account_id != pl_sd->status.account_id && pc_isGM(sd) >= pc_isGM(pl_sd))
		{
			if (pl_sd->bl.m >= 0 && map[pl_sd->bl.m].flag.nowarp && battle_config.any_warp_GM_min_level > pc_isGM(sd))
				count++;
			else {
				if (pc_isdead(pl_sd)) { //Wake them up
					pc_setstand(pl_sd);
					pc_setrestartvalue(pl_sd,1);
				}
				pc_setpos(pl_sd, sd->mapindex, sd->bl.x, sd->bl.y, 2);
			}
		}
	}
	mapit_free(iter);

	clif_displaymessage(fd, msg_txt(92)); // All characters recalled!
	if (count) {
		sprintf(atcmd_output, "Because you are not authorised to warp from some maps, %d player(s) have not been recalled.", count);
		clif_displaymessage(fd, atcmd_output);
	}

	return 0;
}

/*==========================================
 * Recall online characters of a guild to your location
 *------------------------------------------*/
int atcommand_guildrecall(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	struct map_session_data* pl_sd;
	struct s_mapiterator* iter;
	int count;
	char guild_name[NAME_LENGTH];
	struct guild *g;
	nullpo_retr(-1, sd);

	memset(guild_name, '\0', sizeof(guild_name));
	memset(atcmd_output, '\0', sizeof(atcmd_output));

	if (!message || !*message || sscanf(message, "%23[^\n]", guild_name) < 1) {
		clif_displaymessage(fd, "Please, enter a guild name/id (usage: @guildrecall <guild_name/id>).");
		return -1;
	}

	if (sd->bl.m >= 0 && map[sd->bl.m].flag.nowarpto && battle_config.any_warp_GM_min_level > pc_isGM(sd)) {
		clif_displaymessage(fd, "You are not authorised to warp somenone to your actual map.");
		return -1;
	}

	if ((g = guild_searchname(guild_name)) == NULL && // name first to avoid error when name begin with a number
	    (g = guild_search(atoi(message))) == NULL)
	{
		clif_displaymessage(fd, msg_txt(94)); // Incorrect name/ID, or no one from the guild is online.
		return -1;
	}

	count = 0;

	iter = mapit_getallusers();
	for( pl_sd = (TBL_PC*)mapit_first(iter); mapit_exists(iter); pl_sd = (TBL_PC*)mapit_next(iter) )
	{
		if (sd->status.account_id != pl_sd->status.account_id && pl_sd->status.guild_id == g->guild_id)
		{
			if (pc_isGM(pl_sd) > pc_isGM(sd))
				continue; //Skip GMs greater than you.
			if (pl_sd->bl.m >= 0 && map[pl_sd->bl.m].flag.nowarp && battle_config.any_warp_GM_min_level > pc_isGM(sd))
				count++;
			else
				pc_setpos(pl_sd, sd->mapindex, sd->bl.x, sd->bl.y, 2);
		}
	}
	mapit_free(iter);

	sprintf(atcmd_output, msg_txt(93), g->name); // All online characters of the %s guild are near you.
	clif_displaymessage(fd, atcmd_output);
	if (count) {
		sprintf(atcmd_output, "Because you are not authorised to warp from some maps, %d player(s) have not been recalled.", count);
		clif_displaymessage(fd, atcmd_output);
	}

	return 0;
}

/*==========================================
 * Recall online characters of a party to your location
 *------------------------------------------*/
int atcommand_partyrecall(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	struct map_session_data* pl_sd;
	struct s_mapiterator* iter;
	char party_name[NAME_LENGTH];
	struct party_data *p;
	int count;
	nullpo_retr(-1, sd);

	memset(party_name, '\0', sizeof(party_name));
	memset(atcmd_output, '\0', sizeof(atcmd_output));

	if (!message || !*message || sscanf(message, "%23[^\n]", party_name) < 1) {
		clif_displaymessage(fd, "Please, enter a party name/id (usage: @partyrecall <party_name/id>).");
		return -1;
	}

	if (sd->bl.m >= 0 && map[sd->bl.m].flag.nowarpto && battle_config.any_warp_GM_min_level > pc_isGM(sd)) {
		clif_displaymessage(fd, "You are not authorised to warp somenone to your actual map.");
		return -1;
	}

	if ((p = party_searchname(party_name)) == NULL && // name first to avoid error when name begin with a number
	    (p = party_search(atoi(message))) == NULL)
	{
		clif_displaymessage(fd, msg_txt(96)); // Incorrect name or ID, or no one from the party is online.
		return -1;
	}

	count = 0;

	iter = mapit_getallusers();
	for( pl_sd = (TBL_PC*)mapit_first(iter); mapit_exists(iter); pl_sd = (TBL_PC*)mapit_next(iter) )
	{
		if (sd->status.account_id != pl_sd->status.account_id && pl_sd->status.party_id == p->party.party_id)
		{
			if (pc_isGM(pl_sd) > pc_isGM(sd))
				continue; //Skip GMs greater than you.
			if (pl_sd->bl.m >= 0 && map[pl_sd->bl.m].flag.nowarp && battle_config.any_warp_GM_min_level > pc_isGM(sd))
				count++;
			else
				pc_setpos(pl_sd, sd->mapindex, sd->bl.x, sd->bl.y, 2);
		}
	}
	mapit_free(iter);

	sprintf(atcmd_output, msg_txt(95), p->party.name); // All online characters of the %s party are near you.
	clif_displaymessage(fd, atcmd_output);
	if (count) {
		sprintf(atcmd_output, "Because you are not authorised to warp from some maps, %d player(s) have not been recalled.", count);
		clif_displaymessage(fd, atcmd_output);
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------*/
int atcommand_reloaditemdb(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	nullpo_retr(-1, sd);
	itemdb_reload();
	clif_displaymessage(fd, msg_txt(97)); // Item database reloaded.

	return 0;
}

/*==========================================
 *
 *------------------------------------------*/
int atcommand_reloadmobdb(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	nullpo_retr(-1, sd);
	mob_reload();
	read_petdb();
	merc_reload();
	clif_displaymessage(fd, msg_txt(98)); // Monster database reloaded.

	return 0;
}

/*==========================================
 *
 *------------------------------------------*/
int atcommand_reloadskilldb(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	nullpo_retr(-1, sd);
	skill_reload();
	merc_skill_reload();
	clif_displaymessage(fd, msg_txt(99)); // Skill database reloaded.

	return 0;
}

/*==========================================
 * @reloadatcommand - reloads atcommand_athena.conf
 *------------------------------------------*/
int atcommand_reloadatcommand(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	atcommand_config_read(ATCOMMAND_CONF_FILENAME);
	clif_displaymessage(fd, msg_txt(254));
	return 0;
}
/*==========================================
 * @reloadbattleconf - reloads battle_athena.conf
 *------------------------------------------*/
int atcommand_reloadbattleconf(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	struct Battle_Config prev_config;
	memcpy(&prev_config, &battle_config, sizeof(prev_config));

	battle_config_read(BATTLE_CONF_FILENAME);

	if( prev_config.item_rate_mvp          != battle_config.item_rate_mvp
	||  prev_config.item_rate_common       != battle_config.item_rate_common
	||  prev_config.item_rate_common_boss  != battle_config.item_rate_common_boss
	||  prev_config.item_rate_card         != battle_config.item_rate_card
	||  prev_config.item_rate_card_boss    != battle_config.item_rate_card_boss
	||  prev_config.item_rate_equip        != battle_config.item_rate_equip
	||  prev_config.item_rate_equip_boss   != battle_config.item_rate_equip_boss
	||  prev_config.item_rate_heal         != battle_config.item_rate_heal
	||  prev_config.item_rate_heal_boss    != battle_config.item_rate_heal_boss
	||  prev_config.item_rate_use          != battle_config.item_rate_use
	||  prev_config.item_rate_use_boss     != battle_config.item_rate_use_boss
	||  prev_config.item_rate_treasure     != battle_config.item_rate_treasure
	||  prev_config.item_rate_adddrop      != battle_config.item_rate_adddrop
	||  prev_config.logarithmic_drops      != battle_config.logarithmic_drops
	||  prev_config.item_drop_common_min   != battle_config.item_drop_common_min
	||  prev_config.item_drop_common_max   != battle_config.item_drop_common_max
	||  prev_config.item_drop_card_min     != battle_config.item_drop_card_min
	||  prev_config.item_drop_card_max     != battle_config.item_drop_card_max
	||  prev_config.item_drop_equip_min    != battle_config.item_drop_equip_min
	||  prev_config.item_drop_equip_max    != battle_config.item_drop_equip_max
	||  prev_config.item_drop_mvp_min      != battle_config.item_drop_mvp_min
	||  prev_config.item_drop_mvp_max      != battle_config.item_drop_mvp_max
	||  prev_config.item_drop_heal_min     != battle_config.item_drop_heal_min
	||  prev_config.item_drop_heal_max     != battle_config.item_drop_heal_max
	||  prev_config.item_drop_use_min      != battle_config.item_drop_use_min
	||  prev_config.item_drop_use_max      != battle_config.item_drop_use_max
	||  prev_config.item_drop_treasure_min != battle_config.item_drop_treasure_min
	||  prev_config.item_drop_treasure_max != battle_config.item_drop_treasure_max
	)
  	{	//Drop rates changed.
		mob_reload(); //Needed as well so rate changes take effect.
#ifndef TXT_ONLY
		chrif_ragsrvinfo(battle_config.base_exp_rate, battle_config.job_exp_rate, battle_config.item_rate_common);
#endif
	}
	clif_displaymessage(fd, msg_txt(255));
	return 0;
}
/*==========================================
 * @reloadstatusdb - reloads job_db1.txt job_db2.txt job_db2-2.txt refine_db.txt size_fix.txt
 *------------------------------------------*/
int atcommand_reloadstatusdb(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	status_readdb();
	clif_displaymessage(fd, msg_txt(256));
	return 0;
}
/*==========================================
 * @reloadpcdb - reloads exp.txt skill_tree.txt attr_fix.txt statpoint.txt
 *------------------------------------------*/
int atcommand_reloadpcdb(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	pc_readdb();
	clif_displaymessage(fd, msg_txt(257));
	return 0;
}

/*==========================================
 * @reloadmotd - reloads motd.txt
 *------------------------------------------*/
int atcommand_reloadmotd(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	pc_read_motd();
	clif_displaymessage(fd, msg_txt(268));
	return 0;
}

/*==========================================
 * @reloadscript - reloads all scripts (npcs, warps, mob spawns, ...)
 *------------------------------------------*/
int atcommand_reloadscript(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	nullpo_retr(-1, sd);
	//atcommand_broadcast( fd, sd, "@broadcast", "eAthena Server is Rehashing..." );
	//atcommand_broadcast( fd, sd, "@broadcast", "You will feel a bit of lag at this point !" );
	//atcommand_broadcast( fd, sd, "@broadcast", "Reloading NPCs..." );

	flush_fifos();
	script_reload();
	npc_reload();

	clif_displaymessage(fd, msg_txt(100)); // Scripts reloaded.

	return 0;
}

/*==========================================
 * @reloadgmdb - reloads gm levels from where they are stored (gm_account.txt / mysql database)
 *------------------------------------------*/
int atcommand_reloadgmdb(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	nullpo_retr(-1, sd);
	chrif_reloadGMdb();

	clif_displaymessage(fd, msg_txt(101)); // Login-server asked to reload GM accounts and their level.

	return 0;
}

/*==========================================
 * @mapinfo <map name> [0-3] by MC_Cameri
 * => Shows information about the map [map name]
 * 0 = no additional information
 * 1 = Show users in that map and their location
 * 2 = Shows NPCs in that map
 * 3 = Shows the shops/chats in that map (not implemented)
 *------------------------------------------*/
int atcommand_mapinfo(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	struct map_session_data* pl_sd;
	struct s_mapiterator* iter;
	struct npc_data *nd = NULL;
	struct chat_data *cd = NULL;
	char direction[12];
	int i, m_id, chat_num, list = 0;
	unsigned short m_index;
	char mapname[24];

	nullpo_retr(-1, sd);

	memset(atcmd_output, '\0', sizeof(atcmd_output));
	memset(mapname, '\0', sizeof(mapname));
	memset(direction, '\0', sizeof(direction));

	sscanf(message, "%d %23[^\n]", &list, mapname);

	if (list < 0 || list > 3) {
		clif_displaymessage(fd, "Please, enter at least a valid list number (usage: @mapinfo <0-3> [map]).");
		return -1;
	}

	if (mapname[0] == '\0') {
		safestrncpy(mapname, mapindex_id2name(sd->mapindex), MAP_NAME_LENGTH);
		m_id =  map_mapindex2mapid(sd->mapindex);
	} else {
		m_id = map_mapname2mapid(mapname);
	}

	if (m_id < 0) {
		clif_displaymessage(fd, msg_txt(1)); // Map not found.
		return -1;
	}
	m_index = mapindex_name2id(mapname); //This one shouldn't fail since the previous seek did not.
	
	clif_displaymessage(fd, "------ Map Info ------");

	// count chats (for initial message)
	chat_num = 0;
	iter = mapit_getallusers();
	for( pl_sd = (TBL_PC*)mapit_first(iter); mapit_exists(iter); pl_sd = (TBL_PC*)mapit_next(iter) )
		if( (cd = (struct chat_data*)map_id2bl(pl_sd->chatID)) != NULL && pl_sd->mapindex == m_index && cd->usersd[0] == pl_sd )
			chat_num++;
	mapit_free(iter);

	sprintf(atcmd_output, "Map Name: %s | Players In Map: %d | NPCs In Map: %d | Chats In Map: %d", mapname, map[m_id].users, map[m_id].npc_num, chat_num);
	clif_displaymessage(fd, atcmd_output);
	clif_displaymessage(fd, "------ Map Flags ------");
	strcpy(atcmd_output,"PvP Flags: ");
	if (map[m_id].flag.pvp)
		strcat(atcmd_output, "Pvp ON | ");
	if (map[m_id].flag.pvp_noguild)
		strcat(atcmd_output, "NoGuild | ");
	if (map[m_id].flag.pvp_noparty)
		strcat(atcmd_output, "NoParty | ");
	if (map[m_id].flag.pvp_nightmaredrop)
		strcat(atcmd_output, "NightmareDrop | ");
	if (map[m_id].flag.pvp_nocalcrank)
		strcat(atcmd_output, "NoCalcRank | ");
	clif_displaymessage(fd, atcmd_output);

	strcpy(atcmd_output,"GvG Flags: ");
	if (map[m_id].flag.gvg)
		strcat(atcmd_output, "GvG ON | ");
	if (map[m_id].flag.gvg_dungeon)
		strcat(atcmd_output, "GvG Dungeon | ");
	if (map[m_id].flag.gvg_castle)
		strcat(atcmd_output, "GvG Castle | ");
	if (map[m_id].flag.gvg_noparty)
		strcat(atcmd_output, "NoParty | ");
	clif_displaymessage(fd, atcmd_output);

	strcpy(atcmd_output,"Teleport Flags: ");
	if (map[m_id].flag.noteleport)
		strcat(atcmd_output, "NoTeleport | ");
	if (map[m_id].flag.monster_noteleport)
		strcat(atcmd_output, "Monster NoTeleport | ");
	if (map[m_id].flag.nowarp)
		strcat(atcmd_output, "NoWarp | ");
	if (map[m_id].flag.nowarpto)
		strcat(atcmd_output, "NoWarpTo | ");
	if (map[m_id].flag.noreturn)
		strcat(atcmd_output, "NoReturn | ");
	if (map[m_id].flag.nogo)
		strcat(atcmd_output, "NoGo | ");
	if (map[m_id].flag.nomemo)
		strcat(atcmd_output, "NoMemo | ");
	clif_displaymessage(fd, atcmd_output);

	sprintf(atcmd_output, "No Exp Penalty: %s | No Zeny Penalty: %s", (map[m_id].flag.noexppenalty) ? "On" : "Off", (map[m_id].flag.nozenypenalty) ? "On" : "Off");
	clif_displaymessage(fd, atcmd_output);

	if (map[m_id].flag.nosave) {
		if (!map[m_id].save.map)
			sprintf(atcmd_output, "No Save (Return to last Save Point)");
		else if (map[m_id].save.x == -1 || map[m_id].save.y == -1 )
			sprintf(atcmd_output, "No Save, Save Point: %s,Random",mapindex_id2name(map[m_id].save.map));
		else
			sprintf(atcmd_output, "No Save, Save Point: %s,%d,%d",
				mapindex_id2name(map[m_id].save.map),map[m_id].save.x,map[m_id].save.y);
		clif_displaymessage(fd, atcmd_output);
	}

	strcpy(atcmd_output,"Weather Flags: ");
	if (map[m_id].flag.snow)
		strcat(atcmd_output, "Snow | ");
	if (map[m_id].flag.fog)
		strcat(atcmd_output, "Fog | ");
	if (map[m_id].flag.sakura)
		strcat(atcmd_output, "Sakura | ");
	if (map[m_id].flag.clouds)
		strcat(atcmd_output, "Clouds | ");
	if (map[m_id].flag.clouds2)
		strcat(atcmd_output, "Clouds2 | ");
	if (map[m_id].flag.fireworks)
		strcat(atcmd_output, "Fireworks | ");
	if (map[m_id].flag.leaves)
		strcat(atcmd_output, "Leaves | ");
	if (map[m_id].flag.rain)
		strcat(atcmd_output, "Rain | ");
	if (map[m_id].flag.indoors)
		strcat(atcmd_output, "Indoors | ");
	if (map[m_id].flag.nightenabled)
		strcat(atcmd_output, "Displays Night | ");
	clif_displaymessage(fd, atcmd_output);

	strcpy(atcmd_output,"Other Flags: ");
	if (map[m_id].flag.nobranch)
		strcat(atcmd_output, "NoBranch | ");
	if (map[m_id].flag.notrade)
		strcat(atcmd_output, "NoTrade | ");
	if (map[m_id].flag.novending)
		strcat(atcmd_output, "NoVending | ");
	if (map[m_id].flag.nodrop)
		strcat(atcmd_output, "NoDrop | ");
	if (map[m_id].flag.noskill)
		strcat(atcmd_output, "NoSkill | ");
	if (map[m_id].flag.noicewall)
		strcat(atcmd_output, "NoIcewall | ");
		
	clif_displaymessage(fd, atcmd_output);

	strcpy(atcmd_output,"Other Flags: ");
	if (map[m_id].nocommand)
		strcat(atcmd_output, "NoCommand | ");
	if (map[m_id].flag.nobaseexp)
		strcat(atcmd_output, "NoBaseEXP | ");
	if (map[m_id].flag.nojobexp)
		strcat(atcmd_output, "NoJobEXP | ");
	if (map[m_id].flag.nomobloot)
		strcat(atcmd_output, "NoMobLoot | ");
	if (map[m_id].flag.nomvploot)
		strcat(atcmd_output, "NoMVPLoot | ");
	if (map[m_id].flag.partylock)
		strcat(atcmd_output, "PartyLock | ");
	if (map[m_id].flag.guildlock)
		strcat(atcmd_output, "GuildLock | ");
	clif_displaymessage(fd, atcmd_output);


	switch (list) {
	case 0:
		// Do nothing. It's list 0, no additional display.
		break;
	case 1:
		clif_displaymessage(fd, "----- Players in Map -----");
		iter = mapit_getallusers();
		for( pl_sd = (TBL_PC*)mapit_first(iter); mapit_exists(iter); pl_sd = (TBL_PC*)mapit_next(iter) )
		{
			if (pl_sd->mapindex == m_index) {
				sprintf(atcmd_output, "Player '%s' (session #%d) | Location: %d,%d",
				        pl_sd->status.name, pl_sd->fd, pl_sd->bl.x, pl_sd->bl.y);
				clif_displaymessage(fd, atcmd_output);
			}
		}
		mapit_free(iter);
		break;
	case 2:
		clif_displaymessage(fd, "----- NPCs in Map -----");
		for (i = 0; i < map[m_id].npc_num;)
		{
			nd = map[m_id].npc[i];
			switch(nd->ud.dir) {
			case 0:  strcpy(direction, "North"); break;
			case 1:  strcpy(direction, "North West"); break;
			case 2:  strcpy(direction, "West"); break;
			case 3:  strcpy(direction, "South West"); break;
			case 4:  strcpy(direction, "South"); break;
			case 5:  strcpy(direction, "South East"); break;
			case 6:  strcpy(direction, "East"); break;
			case 7:  strcpy(direction, "North East"); break;
			case 9:  strcpy(direction, "North"); break;
			default: strcpy(direction, "Unknown"); break;
			}
			sprintf(atcmd_output, "NPC %d: %s | Direction: %s | Sprite: %d | Location: %d %d",
			        ++i, nd->name, direction, nd->class_, nd->bl.x, nd->bl.y);
			clif_displaymessage(fd, atcmd_output);
		}
		break;
	case 3:
		clif_displaymessage(fd, "----- Chats in Map -----");
		iter = mapit_getallusers();
		for( pl_sd = (TBL_PC*)mapit_first(iter); mapit_exists(iter); pl_sd = (TBL_PC*)mapit_next(iter) )
		{
			if ((cd = (struct chat_data*)map_id2bl(pl_sd->chatID)) != NULL &&
			    pl_sd->mapindex == m_index &&
			    cd->usersd[0] == pl_sd)
			{
				sprintf(atcmd_output, "Chat: %s | Player: %s | Location: %d %d",
				        cd->title, pl_sd->status.name, cd->bl.x, cd->bl.y);
				clif_displaymessage(fd, atcmd_output);
				sprintf(atcmd_output, "   Users: %d/%d | Password: %s | Public: %s",
				        cd->users, cd->limit, cd->pass, (cd->pub) ? "Yes" : "No");
				clif_displaymessage(fd, atcmd_output);
			}
		}
		mapit_free(iter);
		break;
	default: // normally impossible to arrive here
		clif_displaymessage(fd, "Please, enter at least a valid list number (usage: @mapinfo <0-3> [map]).");
		return -1;
		break;
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------*/
int atcommand_mount_peco(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	nullpo_retr(-1, sd);

	if (!pc_isriding(sd)) { // if actually no peco
		if (!pc_checkskill(sd, KN_RIDING))
		{
			clif_displaymessage(fd, msg_txt(213)); // You can not mount a peco with your job.
			return -1;
		}

		if (sd->disguise)
		{
			clif_displaymessage(fd, msg_txt(212)); // Cannot mount a Peco while in disguise.
			return -1;
		}

		pc_setoption(sd, sd->sc.option | OPTION_RIDING);
		clif_displaymessage(fd, msg_txt(102)); // Mounted Peco.
	} else {	//Dismount
		pc_setoption(sd, sd->sc.option & ~OPTION_RIDING);
		clif_displaymessage(fd, msg_txt(214)); // Unmounted Peco.
	}

	return 0;
}

/*==========================================
 *Spy Commands by Syrus22
 *------------------------------------------*/
int atcommand_guildspy(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	char guild_name[NAME_LENGTH];
	struct guild *g;
	nullpo_retr(-1, sd);

	memset(guild_name, '\0', sizeof(guild_name));
	memset(atcmd_output, '\0', sizeof(atcmd_output));

	if (!enable_spy)
	{
		clif_displaymessage(fd, "The mapserver has spy command support disabled.");
		return -1;
	}
	if (!message || !*message || sscanf(message, "%23[^\n]", guild_name) < 1) {
		clif_displaymessage(fd, "Please, enter a guild name/id (usage: @guildspy <guild_name/id>).");
		return -1;
	}

	if ((g = guild_searchname(guild_name)) != NULL || // name first to avoid error when name begin with a number
	    (g = guild_search(atoi(message))) != NULL) {
		if (sd->guildspy == g->guild_id) {
			sd->guildspy = 0;
			sprintf(atcmd_output, msg_txt(103), g->name); // No longer spying on the %s guild.
			clif_displaymessage(fd, atcmd_output);
		} else {
			sd->guildspy = g->guild_id;
			sprintf(atcmd_output, msg_txt(104), g->name); // Spying on the %s guild.
			clif_displaymessage(fd, atcmd_output);
		}
	} else {
		clif_displaymessage(fd, msg_txt(94)); // Incorrect name/ID, or no one from the guild is online.
		return -1;
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------*/
int atcommand_partyspy(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	char party_name[NAME_LENGTH];
	struct party_data *p;
	nullpo_retr(-1, sd);

	memset(party_name, '\0', sizeof(party_name));
	memset(atcmd_output, '\0', sizeof(atcmd_output));

	if (!enable_spy)
	{
		clif_displaymessage(fd, "The mapserver has spy command support disabled.");
		return -1;
	}

	if (!message || !*message || sscanf(message, "%23[^\n]", party_name) < 1) {
		clif_displaymessage(fd, "Please, enter a party name/id (usage: @partyspy <party_name/id>).");
		return -1;
	}

	if ((p = party_searchname(party_name)) != NULL || // name first to avoid error when name begin with a number
	    (p = party_search(atoi(message))) != NULL) {
		if (sd->partyspy == p->party.party_id) {
			sd->partyspy = 0;
			sprintf(atcmd_output, msg_txt(105), p->party.name); // No longer spying on the %s party.
			clif_displaymessage(fd, atcmd_output);
		} else {
			sd->partyspy = p->party.party_id;
			sprintf(atcmd_output, msg_txt(106), p->party.name); // Spying on the %s party.
			clif_displaymessage(fd, atcmd_output);
		}
	} else {
		clif_displaymessage(fd, msg_txt(96)); // Incorrect name or ID, or no one from the party is online.
		return -1;
	}

	return 0;
}

/*==========================================
 * @repairall [Valaris]
 *------------------------------------------*/
int atcommand_repairall(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	int count, i;
	nullpo_retr(-1, sd);

	count = 0;
	for (i = 0; i < MAX_INVENTORY; i++) {
		if (sd->status.inventory[i].nameid && sd->status.inventory[i].attribute == 1) {
			sd->status.inventory[i].attribute = 0;
			clif_produceeffect(sd, 0, sd->status.inventory[i].nameid);
			count++;
		}
	}

	if (count > 0) {
		clif_misceffect(&sd->bl, 3);
		clif_equiplist(sd);
		clif_displaymessage(fd, msg_txt(107)); // All items have been repaired.
	} else {
		clif_displaymessage(fd, msg_txt(108)); // No item need to be repaired.
		return -1;
	}

	return 0;
}

/*==========================================
 * @nuke [Valaris]
 *------------------------------------------*/
int atcommand_nuke(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	struct map_session_data *pl_sd;
	nullpo_retr(-1, sd);

	memset(atcmd_player_name, '\0', sizeof(atcmd_player_name));

	if (!message || !*message || sscanf(message, "%23[^\n]", atcmd_player_name) < 1) {
		clif_displaymessage(fd, "Please, enter a player name (usage: @nuke <char name>).");
		return -1;
	}

	if ((pl_sd = map_nick2sd(atcmd_player_name)) != NULL) {
		if (pc_isGM(sd) >= pc_isGM(pl_sd)) { // you can kill only lower or same GM level
			skill_castend_nodamage_id(&pl_sd->bl, &pl_sd->bl, NPC_SELFDESTRUCTION, 99, gettick(), 0);
			clif_displaymessage(fd, msg_txt(109)); // Player has been nuked!
		} else {
			clif_displaymessage(fd, msg_txt(81)); // Your GM level don't authorise you to do this action on this player.
			return -1;
		}
	} else {
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	return 0;
}

/*==========================================
 * @tonpc
 *------------------------------------------*/
int atcommand_tonpc(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	char npcname[NAME_LENGTH];
	struct npc_data *nd;

	nullpo_retr(-1, sd);

	memset(npcname, 0, sizeof(npcname));

	if (!message || !*message || sscanf(message, "%23[^\n]", npcname) < 1) {
		clif_displaymessage(fd, "Please, enter a NPC name (usage: @tonpc <NPC_name>).");
		return -1;
	}

	if ((nd = npc_name2id(npcname)) != NULL) {
		if (pc_setpos(sd, map_id2index(nd->bl.m), nd->bl.x, nd->bl.y, 3) == 0)
			clif_displaymessage(fd, msg_txt(0)); // Warped.
		else
			return -1;
	} else {
		clif_displaymessage(fd, msg_txt(111)); // This NPC doesn't exist.
		return -1;
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------*/
int atcommand_shownpc(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	char NPCname[NAME_LENGTH];
	nullpo_retr(-1, sd);

	memset(NPCname, '\0', sizeof(NPCname));

	if (!message || !*message || sscanf(message, "%23[^\n]", NPCname) < 1) {
		clif_displaymessage(fd, "Please, enter a NPC name (usage: @enablenpc <NPC_name>).");
		return -1;
	}

	if (npc_name2id(NPCname) != NULL) {
		npc_enable(NPCname, 1);
		clif_displaymessage(fd, msg_txt(110)); // Npc Enabled.
	} else {
		clif_displaymessage(fd, msg_txt(111)); // This NPC doesn't exist.
		return -1;
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------*/
int atcommand_hidenpc(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	char NPCname[NAME_LENGTH];
	nullpo_retr(-1, sd);

	memset(NPCname, '\0', sizeof(NPCname));

	if (!message || !*message || sscanf(message, "%23[^\n]", NPCname) < 1) {
		clif_displaymessage(fd, "Please, enter a NPC name (usage: @hidenpc <NPC_name>).");
		return -1;
	}

	if (npc_name2id(NPCname) == NULL) {
		clif_displaymessage(fd, msg_txt(111)); // This NPC doesn't exist.
		return -1;
	}

	npc_enable(NPCname, 0);
	clif_displaymessage(fd, msg_txt(112)); // Npc Disabled.
	return 0;
}

int atcommand_loadnpc(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	FILE *fp;

	if (!message || !*message) {
		clif_displaymessage(fd, "Please, enter a script file name (usage: @loadnpc <file name>).");
		return -1;
	}

	// check if script file exists
	if ((fp = fopen(message, "r")) == NULL) {
		clif_displaymessage(fd, msg_txt(261));
		return -1;
	}
	fclose(fp);

	// add to list of script sources and run it
	npc_addsrcfile(message);
	npc_parsesrcfile(message);
	npc_read_event_script();

	clif_displaymessage(fd, msg_txt(262));

	return 0;
}

int atcommand_unloadnpc(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	struct npc_data *nd;
	char NPCname[NAME_LENGTH];
	nullpo_retr(-1, sd);

	memset(NPCname, '\0', sizeof(NPCname));

	if (!message || !*message || sscanf(message, "%23[^\n]", NPCname) < 1) {
		clif_displaymessage(fd, "Please, enter a NPC name (usage: @npcoff <NPC_name>).");
		return -1;
	}

	if ((nd = npc_name2id(NPCname)) == NULL) {
		clif_displaymessage(fd, msg_txt(111)); // This NPC doesn't exist.
		return -1;
	}

	npc_unload_duplicates(nd);
	npc_unload(nd);
	npc_read_event_script();
	clif_displaymessage(fd, msg_txt(112)); // Npc Disabled.
	return 0;
}

/*==========================================
 * time in txt for time command (by [Yor])
 *------------------------------------------*/
char* txt_time(unsigned int duration)
{
	int days, hours, minutes, seconds;
	char temp[256];
	static char temp1[256];

	memset(temp, '\0', sizeof(temp));
	memset(temp1, '\0', sizeof(temp1));

	days = duration / (60 * 60 * 24);
	duration = duration - (60 * 60 * 24 * days);
	hours = duration / (60 * 60);
	duration = duration - (60 * 60 * hours);
	minutes = duration / 60;
	seconds = duration - (60 * minutes);

	if (days < 2)
		sprintf(temp, msg_txt(219), days); // %d day
	else
		sprintf(temp, msg_txt(220), days); // %d days
	if (hours < 2)
		sprintf(temp1, msg_txt(221), temp, hours); // %s %d hour
	else
		sprintf(temp1, msg_txt(222), temp, hours); // %s %d hours
	if (minutes < 2)
		sprintf(temp, msg_txt(223), temp1, minutes); // %s %d minute
	else
		sprintf(temp, msg_txt(224), temp1, minutes); // %s %d minutes
	if (seconds < 2)
		sprintf(temp1, msg_txt(225), temp, seconds); // %s and %d second
	else
		sprintf(temp1, msg_txt(226), temp, seconds); // %s and %d seconds

	return temp1;
}

/*==========================================
 * @time/@date/@serverdate/@servertime: Display the date/time of the server (by [Yor]
 * Calculation management of GM modification (@day/@night GM commands) is done
 *------------------------------------------*/
int atcommand_servertime(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	struct TimerData * timer_data;
	struct TimerData * timer_data2;
	time_t time_server;  // variable for number of seconds (used with time() function)
	struct tm *datetime; // variable for time in structure ->tm_mday, ->tm_sec, ...
	char temp[256];
	nullpo_retr(-1, sd);

	memset(temp, '\0', sizeof(temp));

	time(&time_server);  // get time in seconds since 1/1/1970
	datetime = localtime(&time_server); // convert seconds in structure
	// like sprintf, but only for date/time (Sunday, November 02 2003 15:12:52)
	strftime(temp, sizeof(temp)-1, msg_txt(230), datetime); // Server time (normal time): %A, %B %d %Y %X.
	clif_displaymessage(fd, temp);

	if (battle_config.night_duration == 0 && battle_config.day_duration == 0) {
		if (night_flag == 0)
			clif_displaymessage(fd, msg_txt(231)); // Game time: The game is in permanent daylight.
		else
			clif_displaymessage(fd, msg_txt(232)); // Game time: The game is in permanent night.
	} else if (battle_config.night_duration == 0)
		if (night_flag == 1) { // we start with night
			timer_data = get_timer(day_timer_tid);
			sprintf(temp, msg_txt(233), txt_time(DIFF_TICK(timer_data->tick,gettick())/1000)); // Game time: The game is actualy in night for %s.
			clif_displaymessage(fd, temp);
			clif_displaymessage(fd, msg_txt(234)); // Game time: After, the game will be in permanent daylight.
		} else
			clif_displaymessage(fd, msg_txt(231)); // Game time: The game is in permanent daylight.
	else if (battle_config.day_duration == 0)
		if (night_flag == 0) { // we start with day
			timer_data = get_timer(night_timer_tid);
			sprintf(temp, msg_txt(235), txt_time(DIFF_TICK(timer_data->tick,gettick())/1000)); // Game time: The game is actualy in daylight for %s.
			clif_displaymessage(fd, temp);
			clif_displaymessage(fd, msg_txt(236)); // Game time: After, the game will be in permanent night.
		} else
			clif_displaymessage(fd, msg_txt(232)); // Game time: The game is in permanent night.
	else {
		if (night_flag == 0) {
			timer_data = get_timer(night_timer_tid);
			timer_data2 = get_timer(day_timer_tid);
			sprintf(temp, msg_txt(235), txt_time(DIFF_TICK(timer_data->tick,gettick())/1000)); // Game time: The game is actualy in daylight for %s.
			clif_displaymessage(fd, temp);
			if (DIFF_TICK(timer_data->tick, timer_data2->tick) > 0)
				sprintf(temp, msg_txt(237), txt_time(DIFF_TICK(timer_data->interval,DIFF_TICK(timer_data->tick,timer_data2->tick)) / 1000)); // Game time: After, the game will be in night for %s.
			else
				sprintf(temp, msg_txt(237), txt_time(DIFF_TICK(timer_data2->tick,timer_data->tick)/1000)); // Game time: After, the game will be in night for %s.
			clif_displaymessage(fd, temp);
			sprintf(temp, msg_txt(238), txt_time(timer_data->interval / 1000)); // Game time: A day cycle has a normal duration of %s.
			clif_displaymessage(fd, temp);
		} else {
			timer_data = get_timer(day_timer_tid);
			timer_data2 = get_timer(night_timer_tid);
			sprintf(temp, msg_txt(233), txt_time(DIFF_TICK(timer_data->tick,gettick()) / 1000)); // Game time: The game is actualy in night for %s.
			clif_displaymessage(fd, temp);
			if (DIFF_TICK(timer_data->tick,timer_data2->tick) > 0)
				sprintf(temp, msg_txt(239), txt_time((timer_data->interval - DIFF_TICK(timer_data->tick, timer_data2->tick)) / 1000)); // Game time: After, the game will be in daylight for %s.
			else
				sprintf(temp, msg_txt(239), txt_time(DIFF_TICK(timer_data2->tick, timer_data->tick) / 1000)); // Game time: After, the game will be in daylight for %s.
			clif_displaymessage(fd, temp);
			sprintf(temp, msg_txt(238), txt_time(timer_data->interval / 1000)); // Game time: A day cycle has a normal duration of %s.
			clif_displaymessage(fd, temp);
		}
	}

	return 0;
}

//Added by Coltaro
//We're using this function here instead of using time_t so that it only counts player's jail time when he/she's online (and since the idea is to reduce the amount of minutes one by one in status_change_timer...).
//Well, using time_t could still work but for some reason that looks like more coding x_x
static void get_jail_time(int jailtime, int* year, int* month, int* day, int* hour, int* minute)
{
	const int factor_year = 518400; //12*30*24*60 = 518400
	const int factor_month = 43200; //30*24*60 = 43200
	const int factor_day = 1440; //24*60 = 1440
	const int factor_hour = 60;

	*year = jailtime/factor_year;
	jailtime -= *year*factor_year;
	*month = jailtime/factor_month;
	jailtime -= *month*factor_month;
	*day = jailtime/factor_day;
	jailtime -= *day*factor_day;
	*hour = jailtime/factor_hour;
	jailtime -= *hour*factor_hour;
	*minute = jailtime;

	*year = *year > 0? *year : 0;
	*month = *month > 0? *month : 0;
	*day = *day > 0? *day : 0;
	*hour = *hour > 0? *hour : 0;
	*minute = *minute > 0? *minute : 0;
	return;
}

/*==========================================
 * @jail <char_name> by [Yor]
 * Special warp! No check with nowarp and nowarpto flag
 *------------------------------------------*/
int atcommand_jail(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	struct map_session_data *pl_sd;
	int x, y;
	unsigned short m_index;
	nullpo_retr(-1, sd);

	memset(atcmd_player_name, '\0', sizeof(atcmd_player_name));

	if (!message || !*message || sscanf(message, "%23[^\n]", atcmd_player_name) < 1) {
		clif_displaymessage(fd, "Please, enter a player name (usage: @jail <char_name>).");
		return -1;
	}

	if ((pl_sd = map_nick2sd(atcmd_player_name)) == NULL) {
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	if (pc_isGM(sd) < pc_isGM(pl_sd))
  	{ // you can jail only lower or same GM
		clif_displaymessage(fd, msg_txt(81)); // Your GM level don't authorise you to do this action on this player.
		return -1;
	}

	if (pl_sd->sc.data[SC_JAILED])
	{
		clif_displaymessage(fd, msg_txt(118)); // Player warped in jails.
		return -1;
	}

	switch(rand() % 2) { //Jail Locations
	case 0:
		m_index = mapindex_name2id(MAP_JAIL);
		x = 24;
		y = 75;
		break;
	default:
		m_index = mapindex_name2id(MAP_JAIL);
		x = 49;
		y = 75;
		break;
	}

	//Duration of INT_MAX to specify infinity.
	sc_start4(&pl_sd->bl,SC_JAILED,100,INT_MAX,m_index,x,y,1000);
	clif_displaymessage(pl_sd->fd, msg_txt(117)); // GM has send you in jails.
	clif_displaymessage(fd, msg_txt(118)); // Player warped in jails.
	return 0;
}

/*==========================================
 * @unjail/@discharge <char_name> by [Yor]
 * Special warp! No check with nowarp and nowarpto flag
 *------------------------------------------*/
int atcommand_unjail(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	struct map_session_data *pl_sd;

	memset(atcmd_player_name, '\0', sizeof(atcmd_player_name));

	if (!message || !*message || sscanf(message, "%23[^\n]", atcmd_player_name) < 1) {
		clif_displaymessage(fd, "Please, enter a player name (usage: @unjail/@discharge <char_name>).");
		return -1;
	}

	if ((pl_sd = map_nick2sd(atcmd_player_name)) == NULL) {
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	if (pc_isGM(sd) < pc_isGM(pl_sd)) { // you can jail only lower or same GM

		clif_displaymessage(fd, msg_txt(81)); // Your GM level don't authorise you to do this action on this player.
		return -1;
	}

	if (!pl_sd->sc.data[SC_JAILED])
	{
		clif_displaymessage(fd, msg_txt(119)); // This player is not in jails.
		return -1;
	}

	//Reset jail time to 1 sec.
	sc_start(&pl_sd->bl,SC_JAILED,100,1,1000);
	clif_displaymessage(pl_sd->fd, msg_txt(120)); // GM has discharge you.
	clif_displaymessage(fd, msg_txt(121)); // Player unjailed.
	return 0;
}

int atcommand_jailfor(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	struct map_session_data *pl_sd = NULL;
	int year, month, day, hour, minute, value;
	char * modif_p;
	int jailtime = 0,x,y;
	short m_index = 0;
	nullpo_retr(-1, sd);

	if (!message || !*message || sscanf(message, "%s %23[^\n]",atcmd_output,atcmd_player_name) < 2) {
		clif_displaymessage(fd, msg_txt(400));	//Usage: @jailfor <time> <character name>
		return -1;
	}

	atcmd_output[sizeof(atcmd_output)-1] = '\0';

	modif_p = atcmd_output;
	year = month = day = hour = minute = 0;
	while (modif_p[0] != '\0') {
		value = atoi(modif_p);
		if (value == 0)
			modif_p++;
		else {
			if (modif_p[0] == '-' || modif_p[0] == '+')
				modif_p++;
			while (modif_p[0] >= '0' && modif_p[0] <= '9')
				modif_p++;
			if (modif_p[0] == 'n' || (modif_p[0] == 'm' && modif_p[1] == 'n')) {
				minute = value;
				modif_p = modif_p + 2;
			} else if (modif_p[0] == 'h') {
				hour = value;
				modif_p++;
			} else if (modif_p[0] == 'd' || modif_p[0] == 'j') {
				day = value;
				modif_p++;
			} else if (modif_p[0] == 'm') {
				month = value;
				modif_p++;
			} else if (modif_p[0] == 'y' || modif_p[0] == 'a') {
				year = value;
				modif_p++;
			} else if (modif_p[0] != '\0') {
				modif_p++;
			}
		}
	}

	if (year == 0 && month == 0 && day == 0 && hour == 0 && minute == 0) {
		clif_displaymessage(fd, "Invalid time for jail command.");
		return -1;
	}

	if ((pl_sd = map_nick2sd(atcmd_player_name)) == NULL) {
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	if (pc_isGM(pl_sd) > pc_isGM(sd)) {
		clif_displaymessage(fd, msg_txt(81)); // Your GM level don't authorise you to do this action on this player.
		return -1;
	}

	jailtime = year*12*30*24*60 + month*30*24*60 + day*24*60 + hour*60 + minute;	//In minutes

	if(jailtime==0) {
		clif_displaymessage(fd, "Invalid time for jail command.");
		return -1;
	}

	//Added by Coltaro
	if(pl_sd->sc.data[SC_JAILED] && 
		pl_sd->sc.data[SC_JAILED]->val1 != INT_MAX)
  	{	//Update the player's jail time
		jailtime += pl_sd->sc.data[SC_JAILED]->val1;
		if (jailtime <= 0) {
			jailtime = 0;
			clif_displaymessage(pl_sd->fd, msg_txt(120)); // GM has discharge you.
			clif_displaymessage(fd, msg_txt(121)); // Player unjailed
		} else {
			get_jail_time(jailtime,&year,&month,&day,&hour,&minute);
			sprintf(atcmd_output,msg_txt(402),"You are now",year,month,day,hour,minute); //%s in jail for %d years, %d months, %d days, %d hours and %d minutes
	 		clif_displaymessage(pl_sd->fd, atcmd_output);
			sprintf(atcmd_output,msg_txt(402),"This player is now",year,month,day,hour,minute); //This player is now in jail for %d years, %d months, %d days, %d hours and %d minutes
	 		clif_displaymessage(fd, atcmd_output);
		}
	} else if (jailtime < 0) {
		clif_displaymessage(fd, "Invalid time for jail command.");
		return -1;
	}

	//Jail locations, add more as you wish.
	switch(rand()%2)
	{
		case 1: //Jail #1
			m_index = mapindex_name2id(MAP_JAIL);
			x = 49; y = 75;
			break;
		default: //Default Jail
			m_index = mapindex_name2id(MAP_JAIL);
			x = 24; y = 75;
			break;
	}

	sc_start4(&pl_sd->bl,SC_JAILED,100,jailtime,m_index,x,y,jailtime?60000:1000); //jailtime = 0: Time was reset to 0. Wait 1 second to warp player out (since it's done in status_change_timer).
	return 0;
}


//By Coltaro
int atcommand_jailtime(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	int year, month, day, hour, minute;

	nullpo_retr(-1, sd);
	
	if (!sd->sc.data[SC_JAILED]) {
		clif_displaymessage(fd, "You are not in jail."); // You are not in jail.
		return -1;
	}

	if (sd->sc.data[SC_JAILED]->val1 == INT_MAX) {
		clif_displaymessage(fd, "You have been jailed indefinitely.");
		return 0;
	}

	if (sd->sc.data[SC_JAILED]->val1 <= 0) { // Was not jailed with @jailfor (maybe @jail? or warped there? or got recalled?)
		clif_displaymessage(fd, "You have been jailed for an unknown amount of time.");
		return -1;
	}

	//Get remaining jail time
	get_jail_time(sd->sc.data[SC_JAILED]->val1,&year,&month,&day,&hour,&minute);
	sprintf(atcmd_output,msg_txt(402),"You will remain",year,month,day,hour,minute); // You will remain in jail for %d years, %d months, %d days, %d hours and %d minutes

	clif_displaymessage(fd, atcmd_output);

	return 0;
}

/*==========================================
 * @disguise <mob_id> by [Valaris] (simplified by [Yor])
 *------------------------------------------*/
int atcommand_disguise(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	int id = 0;
	nullpo_retr(-1, sd);

	if (!message || !*message) {
		clif_displaymessage(fd, "Please, enter a Monster/NPC name/id (usage: @disguise <monster_name_or_monster_ID>).");
		return -1;
	}

	if ((id = atoi(message)) > 0)
	{	//Acquired an ID
		if (!mobdb_checkid(id) && !npcdb_checkid(id))
			id = 0; //Invalid id for either mobs or npcs.
	}	else	{ //Acquired a Name
		if ((id = mobdb_searchname(message)) == 0)
		{
			struct npc_data* nd = npc_name2id(message);
			if (nd != NULL)
				id = nd->class_;
		}
	}

	if (id == 0)
	{
		clif_displaymessage(fd, msg_txt(123));	// Monster/NPC name/id hasn't been found.
		return -1;
	}

	if(pc_isriding(sd))
	{
		//FIXME: wrong message [ultramage]
		//clif_displaymessage(fd, msg_txt(227)); // Character cannot wear disguise while riding a PecoPeco.
		return -1;
	}

	pc_disguise(sd, id);
	clif_displaymessage(fd, msg_txt(122)); // Disguise applied.

	return 0;
}

/*==========================================
 * DisguiseAll
 *------------------------------------------*/
int atcommand_disguiseall(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	int mob_id=0;
	struct map_session_data *pl_sd;
	struct s_mapiterator* iter;
	nullpo_retr(-1, sd);

	if (!message || !*message) {
		clif_displaymessage(fd, "Please, enter a Monster/NPC name/id (usage: @disguiseall <monster name or monster ID>).");
		return -1;
	}

	if ((mob_id = mobdb_searchname(message)) == 0) // check name first (to avoid possible name begining by a number)
		mob_id = atoi(message);

	if (!mobdb_checkid(mob_id) && !npcdb_checkid(mob_id)) { //if mob or npc...
		clif_displaymessage(fd, msg_txt(123)); // Monster/NPC name/id not found.
		return -1;
	}

	iter = mapit_getallusers();
	for( pl_sd = (TBL_PC*)mapit_first(iter); mapit_exists(iter); pl_sd = (TBL_PC*)mapit_next(iter) )
		pc_disguise(pl_sd, mob_id);
	mapit_free(iter);

	clif_displaymessage(fd, msg_txt(122)); // Disguise applied.
	return 0;
}

/*==========================================
 * @undisguise by [Yor]
 *------------------------------------------*/
int atcommand_undisguise(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	nullpo_retr(-1, sd);
	if (sd->disguise) {
		pc_disguise(sd, 0);
		clif_displaymessage(fd, msg_txt(124)); // Undisguise applied.
	} else {
		clif_displaymessage(fd, msg_txt(125)); // You're not disguised.
		return -1;
	}

	return 0;
}

/*==========================================
 * UndisguiseAll
 *------------------------------------------*/
int atcommand_undisguiseall(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	struct map_session_data *pl_sd;
	struct s_mapiterator* iter;
	nullpo_retr(-1, sd);

	iter = mapit_getallusers();
	for( pl_sd = (TBL_PC*)mapit_first(iter); mapit_exists(iter); pl_sd = (TBL_PC*)mapit_next(iter) )
		if( pl_sd->disguise )
			pc_disguise(pl_sd, 0);
	mapit_free(iter);

	clif_displaymessage(fd, msg_txt(124)); // Undisguise applied.

	return 0;
}

/*==========================================
 * @exp by [Skotlex]
 *------------------------------------------*/
int atcommand_exp(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	char output[200];
	double nextb, nextj;
	nullpo_retr(-1, sd);
	memset(output, '\0', sizeof(output));
	
	nextb = pc_nextbaseexp(sd);
	if (nextb)
		nextb = sd->status.base_exp*100.0/nextb;
	
	nextj = pc_nextjobexp(sd);
	if (nextj)
		nextj = sd->status.job_exp*100.0/nextj;
	
	sprintf(output, "Base Level: %d (%.3f%%) | Job Level: %d (%.3f%%)", sd->status.base_level, nextb, sd->status.job_level, nextj);
	clif_displaymessage(fd, output);
	return 0;
}


/*==========================================
 * @broadcast by [Valaris]
 *------------------------------------------*/
int atcommand_broadcast(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	nullpo_retr(-1, sd);

	memset(atcmd_output, '\0', sizeof(atcmd_output));

	if (!message || !*message) {
		clif_displaymessage(fd, "Please, enter a message (usage: @broadcast <message>).");
		return -1;
	}

	sprintf(atcmd_output, "%s: %s", sd->status.name, message);
	intif_GMmessage(atcmd_output, strlen(atcmd_output) + 1, 0);

	return 0;
}

/*==========================================
 * @localbroadcast by [Valaris]
 *------------------------------------------*/
int atcommand_localbroadcast(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	nullpo_retr(-1, sd);

	memset(atcmd_output, '\0', sizeof(atcmd_output));

	if (!message || !*message) {
		clif_displaymessage(fd, "Please, enter a message (usage: @localbroadcast <message>).");
		return -1;
	}

	sprintf(atcmd_output, "%s: %s", sd->status.name, message);

	clif_GMmessage(&sd->bl, atcmd_output, strlen(atcmd_output) + 1, 1); // 1: ALL_SAMEMAP

	return 0;
}

/*==========================================
 * @email <actual@email> <new@email> by [Yor]
 *------------------------------------------*/
int atcommand_email(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	char actual_email[100];
	char new_email[100];
	nullpo_retr(-1, sd);

	memset(actual_email, '\0', sizeof(actual_email));
	memset(new_email, '\0', sizeof(new_email));

	if (!message || !*message || sscanf(message, "%99s %99s", actual_email, new_email) < 2) {
		clif_displaymessage(fd, "Please enter 2 emails (usage: @email <actual@email> <new@email>).");
		return -1;
	}

	if (e_mail_check(actual_email) == 0) {
		clif_displaymessage(fd, msg_txt(144)); // Invalid actual email. If you have default e-mail, give a@a.com.
		return -1;
	} else if (e_mail_check(new_email) == 0) {
		clif_displaymessage(fd, msg_txt(145)); // Invalid new email. Please enter a real e-mail.
		return -1;
	} else if (strcmpi(new_email, "a@a.com") == 0) {
		clif_displaymessage(fd, msg_txt(146)); // New email must be a real e-mail.
		return -1;
	} else if (strcmpi(actual_email, new_email) == 0) {
		clif_displaymessage(fd, msg_txt(147)); // New email must be different of the actual e-mail.
		return -1;
	}

	chrif_changeemail(sd->status.account_id, actual_email, new_email);
	clif_displaymessage(fd, msg_txt(148)); // Information sended to login-server via char-server.
	return 0;
}

/*==========================================
 *@effect
 *------------------------------------------*/
int atcommand_effect(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	int type = 0, flag = 0;
	nullpo_retr(-1, sd);

	if (!message || !*message || sscanf(message, "%d", &type) < 1) {
		clif_displaymessage(fd, "Please, enter an effect number (usage: @effect <effect number>).");
		return -1;
	}

	clif_specialeffect(&sd->bl, type, flag);
	clif_displaymessage(fd, msg_txt(229)); // Your effect has changed.
	return 0;
}

/*==========================================
 * @killer by MouseJstr
 * enable killing players even when not in pvp
 *------------------------------------------*/
int atcommand_killer(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	nullpo_retr(-1, sd);
	sd->state.killer = !sd->state.killer;

	if(sd->state.killer)
		clif_displaymessage(fd, msg_txt(241));
	else {
		clif_displaymessage(fd, msg_txt(287));
		pc_stop_attack(sd);
	}
	return 0;
}

/*==========================================
 * @killable by MouseJstr
 * enable other people killing you
 *------------------------------------------*/
int atcommand_killable(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	nullpo_retr(-1, sd);
	sd->state.killable = !sd->state.killable;

	if(sd->state.killable)
		clif_displaymessage(fd, msg_txt(242));
	else {
		clif_displaymessage(fd, msg_txt(288));
		map_foreachinrange(atcommand_stopattack,&sd->bl, AREA_SIZE, BL_CHAR, sd->bl.id);
	}
	return 0;
}

/*==========================================
 * @skillon by MouseJstr
 * turn skills on for the map
 *------------------------------------------*/
int atcommand_skillon(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	nullpo_retr(-1, sd);
	map[sd->bl.m].flag.noskill = 0;
	clif_displaymessage(fd, msg_txt(244));
	return 0;
}

/*==========================================
 * @skilloff by MouseJstr
 * Turn skills off on the map
 *------------------------------------------*/
int atcommand_skilloff(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	nullpo_retr(-1, sd);
	map[sd->bl.m].flag.noskill = 1;
	clif_displaymessage(fd, msg_txt(243));
	return 0;
}

/*==========================================
 * @npcmove by MouseJstr
 * move a npc
 *------------------------------------------*/
int atcommand_npcmove(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	int x = 0, y = 0, m;
	struct npc_data *nd = 0;
	nullpo_retr(-1, sd);
	memset(atcmd_player_name, '\0', sizeof atcmd_player_name);

	if (!message || !*message || sscanf(message, "%d %d %23[^\n]", &x, &y, atcmd_player_name) < 3) {
		clif_displaymessage(fd, "Usage: @npcmove <X> <Y> <npc_name>");
		return -1;
	}

	if ((nd = npc_name2id(atcmd_player_name)) == NULL)
	{
		clif_displaymessage(fd, msg_txt(111)); // This NPC doesn't exist.
		return -1;
	}

	if ((m=nd->bl.m) < 0 || nd->bl.prev == NULL)
	{
		clif_displaymessage(fd, "NPC is not on this map.");
		return -1;	//Not on a map.
	}
	
	x = cap_value(x, 0, map[m].xs-1);
	y = cap_value(y, 0, map[m].ys-1);
	map_foreachinrange(clif_outsight, &nd->bl, AREA_SIZE, BL_PC, &nd->bl);
	map_moveblock(&nd->bl, x, y, gettick());
	map_foreachinrange(clif_insight, &nd->bl, AREA_SIZE, BL_PC, &nd->bl);
	clif_displaymessage(fd, "NPC moved.");

	return 0;
}

/*==========================================
 * @addwarp by MouseJstr
 * Create a new static warp point.
 *------------------------------------------*/
int atcommand_addwarp(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	char mapname[32];
	int x,y;
	unsigned short m;
	struct npc_data* nd;

	nullpo_retr(-1, sd);

	if (!message || !*message || sscanf(message, "%31s %d %d", mapname, &x, &y) < 3) {
		clif_displaymessage(fd, "usage: @addwarp <mapname> <X> <Y>.");
		return -1;
	}

	m = mapindex_name2id(mapname);
	if( m == 0 )
	{
		sprintf(atcmd_output, "Unknown map '%s'.", mapname);
		clif_displaymessage(fd, atcmd_output);
		return -1;
	}

	nd = npc_add_warp(sd->bl.m, sd->bl.x, sd->bl.y, 2, 2, m, x, y);
	if( nd == NULL )
		return -1;

	sprintf(atcmd_output, "New warp NPC '%s' created.", nd->exname);
	clif_displaymessage(fd, atcmd_output);
	return 0;
}

/*==========================================
 * @follow by [MouseJstr]
 * Follow a player .. staying no more then 5 spaces away
 *------------------------------------------*/
int atcommand_follow(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	struct map_session_data *pl_sd = NULL;
	nullpo_retr(-1, sd);

	if (!message || !*message) {
		if (sd->followtarget == -1)
			return -1;

		pc_stop_following (sd);
		clif_displaymessage(fd, "Follow mode OFF.");
		return 0;
	}
	
	if ( (pl_sd = map_nick2sd((char *)message)) == NULL )
	{
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	if (sd->followtarget == pl_sd->bl.id) {
		pc_stop_following (sd);
		clif_displaymessage(fd, "Follow mode OFF.");
	} else {
		pc_follow(sd, pl_sd->bl.id);
		clif_displaymessage(fd, "Follow mode ON.");
	}
	
	return 0;
}


/*==========================================
 * @dropall by [MouseJstr]
 * Drop all your possession on the ground
 *------------------------------------------*/
int atcommand_dropall(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	int i;
	nullpo_retr(-1, sd);
	for (i = 0; i < MAX_INVENTORY; i++) {
	if (sd->status.inventory[i].amount) {
		if(sd->status.inventory[i].equip != 0)
			pc_unequipitem(sd, i, 3);
			pc_dropitem(sd,  i, sd->status.inventory[i].amount);
		}
	}
	return 0;
}

/*==========================================
 * @storeall by [MouseJstr]
 * Put everything into storage
 *------------------------------------------*/
int atcommand_storeall(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	int i;
	nullpo_retr(-1, sd);

	if (sd->state.storage_flag != 1)
  	{	//Open storage.
		switch (storage_storageopen(sd)) {
		case 2: //Try again
			clif_displaymessage(fd, "run this command again..");
			return 0;
		case 1: //Failure
			clif_displaymessage(fd, "You can't open the storage currently.");
			return -1;
		}
	}
	for (i = 0; i < MAX_INVENTORY; i++) {
		if (sd->status.inventory[i].amount) {
			if(sd->status.inventory[i].equip != 0)
				pc_unequipitem(sd, i, 3);
			storage_storageadd(sd,  i, sd->status.inventory[i].amount);
		}
	}
	storage_storageclose(sd);

	clif_displaymessage(fd, "It is done");
	return 0;
}

/*==========================================
 * @skillid by [MouseJstr]
 * lookup a skill by name
 *------------------------------------------*/
int atcommand_skillid(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	int skillen, idx;
	nullpo_retr(-1, sd);

	if (!message || !*message)
	{
		clif_displaymessage(fd, "Please enter a skill name to look up (usage: @skillid <skill name>).");
		return -1;
	}

	skillen = strlen(message);

	for (idx = 0; idx < MAX_SKILL_DB; idx++) {
		if (strnicmp(skill_db[idx].name, message, skillen) == 0 || strnicmp(skill_db[idx].desc, message, skillen) == 0)
		{
			sprintf(atcmd_output, "skill %d: %s", idx, skill_db[idx].desc);
			clif_displaymessage(fd, atcmd_output);
		}
	}

	return 0;
}

/*==========================================
 * @useskill by [MouseJstr]
 * A way of using skills without having to find them in the skills menu
 *------------------------------------------*/
int atcommand_useskill(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	struct map_session_data *pl_sd = NULL;
	struct block_list *bl;
	int skillnum;
	int skilllv;
	char target[100];
	nullpo_retr(-1, sd);

	if(!message || !*message || sscanf(message, "%d %d %23[^\n]", &skillnum, &skilllv, target) != 3) {
		clif_displaymessage(fd, "Usage: @useskill <skillnum> <skillv> <target>");
		return -1;
	}

	if ( (pl_sd = map_nick2sd(target)) == NULL )
	{
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	if ( pc_isGM(sd) < pc_isGM(pl_sd) )
	{
		clif_displaymessage(fd, msg_txt(81)); // Your GM level don't authorise you to do this action on this player.
		return -1;
	}

	if (skillnum >= HM_SKILLBASE && skillnum < HM_SKILLBASE+MAX_HOMUNSKILL
		&& sd->hd && merc_is_hom_active(sd->hd)) // (If used with @useskill, put the homunc as dest)
		bl = &sd->hd->bl;
	else
		bl = &sd->bl;
	
	if (skill_get_inf(skillnum)&INF_GROUND_SKILL)
		unit_skilluse_pos(bl, pl_sd->bl.x, pl_sd->bl.y, skillnum, skilllv);
	else
		unit_skilluse_id(bl, pl_sd->bl.id, skillnum, skilllv);

	return 0;
}

/*==========================================
 * @displayskill by [Skotlex]
 *  Debug command to locate new skill IDs. It sends the
 *  three possible skill-effect packets to the area.
 *------------------------------------------*/
int atcommand_displayskill(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	struct status_data * status;
	unsigned int tick;
	int skillnum;
	int skilllv = 1;
	nullpo_retr(-1, sd);

	if (!message || !*message || sscanf(message, "%d %d", &skillnum, &skilllv) < 1)
	{
		clif_displaymessage(fd, "Usage: @displayskill <skillnum> {<skillv>}>");
		return -1;
	}
	status = status_get_status_data(&sd->bl);
	tick = gettick();
	clif_skill_damage(&sd->bl,&sd->bl, tick, status->amotion, status->dmotion, 1, 1, skillnum, skilllv, 5);
	clif_skill_nodamage(&sd->bl, &sd->bl, skillnum, skilllv, 1);
	clif_skill_poseffect(&sd->bl, skillnum, skilllv, sd->bl.x, sd->bl.y, tick);
	return 0;
}

/*==========================================
 * @skilltree by [MouseJstr]
 * prints the skill tree for a player required to get to a skill
 *------------------------------------------*/
int atcommand_skilltree(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	struct map_session_data *pl_sd = NULL;
	int skillnum, skillidx = -1;
	int meets = 1, j, c=0;
	char target[NAME_LENGTH], *tbl;
	struct skill_tree_entry *ent;
	nullpo_retr(-1, sd);

	if(!message || !*message || sscanf(message, "%d %23[^\r\n]", &skillnum, target) != 2) {
		clif_displaymessage(fd, "Usage: @skilltree <skillnum> <target>");
		return -1;
	}

	if ( (pl_sd = map_nick2sd(target)) == NULL )
	{
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	c = pc_calc_skilltree_normalize_job(pl_sd);
	c = pc_mapid2jobid(c, pl_sd->status.sex);

	tbl = job_name(c);

	sprintf(atcmd_output, "Player is using %s skill tree (%d basic points)",
	tbl, pc_checkskill(pl_sd, 1));
	clif_displaymessage(fd, atcmd_output);

	for (j = 0; skill_tree[c][j].id != 0; j++) {
		if (skill_tree[c][j].id == skillnum) {
			skillidx = j;
			break;
		}
	}

	if (skillidx == -1) {
		sprintf(atcmd_output, "I do not believe the player can use that skill");
		clif_displaymessage(fd, atcmd_output);
		return 0;
	}

	ent = &skill_tree[c][skillidx];

	for(j=0;j<5;j++)
		if( ent->need[j].id && pc_checkskill(sd,ent->need[j].id) < ent->need[j].lv)
		{
			sprintf(atcmd_output, "player requires level %d of skill %s", ent->need[j].lv, skill_db[ent->need[j].id].desc);
			clif_displaymessage(fd, atcmd_output);
			meets = 0;
		}

		if (meets == 1) {
			sprintf(atcmd_output, "I believe the player meets all the requirements for that skill");
			clif_displaymessage(fd, atcmd_output);
		}

	return 0;
}

// Hand a ring with partners name on it to this char
void getring (struct map_session_data* sd)
{
	int flag, item_id;
	struct item item_tmp;
	item_id = (sd->status.sex) ? WEDDING_RING_M : WEDDING_RING_F;

	memset(&item_tmp, 0, sizeof(item_tmp));
	item_tmp.nameid = item_id;
	item_tmp.identify = 1;
	item_tmp.card[0] = 255;
	item_tmp.card[2] = sd->status.partner_id;
	item_tmp.card[3] = sd->status.partner_id >> 16;

	//Logs (A)dmins items [Lupus]
	if(log_config.enable_logs&0x400)
		log_pick_pc(sd, "A", item_id, 1, &item_tmp);

	if((flag = pc_additem(sd,&item_tmp,1))) {
		clif_additem(sd,0,0,flag);
		map_addflooritem(&item_tmp,1,sd->bl.m,sd->bl.x,sd->bl.y,0,0,0,0);
	}
}

/*==========================================
 * @marry by [MouseJstr], fixed by Lupus
 * Marry two players
 *------------------------------------------*/
int atcommand_marry(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
  struct map_session_data *pl_sd1 = NULL;
  struct map_session_data *pl_sd2 = NULL;
  char player1[128], player2[128]; //Length used for return error msgs

  nullpo_retr(-1, sd);

  if (!message || !*message || sscanf(message, "%23[^,], %23[^\r\n]", player1, player2) != 2) {
	clif_displaymessage(fd, "Usage: @marry <player1>,<player2>");
	return -1;
  }

  if((pl_sd1=map_nick2sd((char *) player1)) == NULL) {
	sprintf(player2, "Cannot find player '%s' online", player1);
	clif_displaymessage(fd, player2);
	return -1;
  }

  if((pl_sd2=map_nick2sd((char *) player2)) == NULL) {
	sprintf(player1, "Cannot find player '%s' online", player2);
	clif_displaymessage(fd, player1);
	return -1;
  }

  if (pc_marriage(pl_sd1, pl_sd2) == 0) {
	clif_displaymessage(fd, "They are married.. wish them well");
	clif_wedding_effect(&sd->bl);	//wedding effect and music [Lupus]
	// Auto-give named rings (Aru)
	getring (pl_sd1);
	getring (pl_sd2);
	return 0;
  }

	clif_displaymessage(fd, "The two cannot wed because one of them is either a baby or is already married.");
	return -1;
}

/*==========================================
 * @divorce by [MouseJstr], fixed by [Lupus]
 * divorce two players
 *------------------------------------------*/
int atcommand_divorce(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
  struct map_session_data *pl_sd = NULL;

  nullpo_retr(-1, sd);

  if (!message || !*message || sscanf(message, "%23[^\r\n]", atcmd_player_name) != 1) {
	clif_displaymessage(fd, "Usage: @divorce <player>.");
	return -1;
  }

	if ( (pl_sd = map_nick2sd(atcmd_player_name)) == NULL )
	{
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	if (pc_divorce(pl_sd) != 0) {
		sprintf(atcmd_output, "The divorce has failed.. Cannot find player '%s' or his(her) partner online.", atcmd_player_name);
		clif_displaymessage(fd, atcmd_output);
		return -1;
	}
	
	sprintf(atcmd_output, "'%s' and his(her) partner are now divorced.", atcmd_player_name);
	clif_displaymessage(fd, atcmd_output);
	return 0;
}

/*==========================================
 * @changelook by [Celest]
 *------------------------------------------*/
int atcommand_changelook(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	int i, j = 0, k = 0;
	int pos[6] = { LOOK_HEAD_TOP,LOOK_HEAD_MID,LOOK_HEAD_BOTTOM,LOOK_WEAPON,LOOK_SHIELD,LOOK_SHOES };

	if((i = sscanf(message, "%d %d", &j, &k)) < 1) {
		clif_displaymessage(fd, "Usage: @changelook [<position>] <view id> -- [] = optional");
		clif_displaymessage(fd, "Position: 1-Top 2-Middle 3-Bottom 4-Weapon 5-Shield");
		return -1;
	} else if (i == 2) {
		if (j < 1) j = 1;
		else if (j > 6) j = 6;	// 6 = Shoes - for beta clients only perhaps
		j = pos[j - 1];
	} else if (i == 1) {	// position not defined, use HEAD_TOP as default
		k = j;	// swap
		j = LOOK_HEAD_TOP;
	}

	clif_changelook(&sd->bl,j,k);

	return 0;
}

/*==========================================
 * @autotrade by durf (changed by Lupus)
 * Turns on/off Autotrade for a specific player
 *------------------------------------------*/
int atcommand_autotrade(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	nullpo_retr(-1, sd);
	if( sd->vender_id ) //check if player's vending
	{
		if( map[sd->bl.m].flag.autotrade == battle_config.autotrade_mapflag )
		{
			sd->state.autotrade = 1;
			if( battle_config.at_timeout )
			{
				int timeout = atoi(message);
				status_change_start(&sd->bl, SC_AUTOTRADE, 10000, 0, 0, 0, 0, ((timeout > 0) ? min(timeout,battle_config.at_timeout) : battle_config.at_timeout) * 60000, 0);
			}

			clif_authfail_fd(fd, 15);
		} else
			clif_displaymessage(fd, "Autotrade is not allowed on this map.");
	}
	else
		clif_displaymessage(fd, msg_txt(549)); // You should be vending to use @Autotrade.

	return 0;
}

/*==========================================
 * @changegm by durf (changed by Lupus)
 * Changes Master of your Guild to a specified guild member
 *------------------------------------------*/
int atcommand_changegm(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	struct guild *g;
	struct map_session_data *pl_sd;
	nullpo_retr(-1, sd);

	if (sd->status.guild_id == 0 || (g = guild_search(sd->status.guild_id)) == NULL || strcmp(g->master,sd->status.name))
	{
		clif_displaymessage(fd, "You need to be a Guild Master to use this command.");
		return -1;
	}
	if (strlen(message)==0)
	{
		clif_displaymessage(fd, "Command usage: @changegm <guildmember name>");
		return -1;
	}
	
	if((pl_sd=map_nick2sd((char *) message)) == NULL || pl_sd->status.guild_id != sd->status.guild_id) {
		clif_displaymessage(fd, "Target character must be online and be a guildmate.");
		return -1;
	}

	guild_gm_change(sd->status.guild_id, pl_sd);
	return 0;
}

/*==========================================
 * @changeleader by Skotlex
 * Changes the leader of a party.
 *------------------------------------------*/
int atcommand_changeleader(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	struct party_data *p;
	struct map_session_data *pl_sd;
	int mi, pl_mi;
	nullpo_retr(-1, sd);

	if (sd->status.party_id == 0 || (p = party_search(sd->status.party_id)) == NULL)
	{	//Need to be a party leader.
		clif_displaymessage(fd, msg_txt(282));
		return -1;
	}
	
	for (mi = 0; mi < MAX_PARTY && p->data[mi].sd != sd; mi++);
	
	if (mi == MAX_PARTY)
		return -1; //Shouldn't happen

	if (!p->party.member[mi].leader)
	{	//Need to be a party leader.
		clif_displaymessage(fd, msg_txt(282));
		return -1;
	}
	
	if (strlen(message)==0)
	{
		clif_displaymessage(fd, "Command usage: @changeleader <party member name>");
		return -1;
	}
	
	if((pl_sd=map_nick2sd((char *) message)) == NULL || pl_sd->status.party_id != sd->status.party_id) {
		clif_displaymessage(fd, msg_txt(283));
		return -1;
	}

	for (pl_mi = 0; pl_mi < MAX_PARTY && p->data[pl_mi].sd != pl_sd; pl_mi++);
	
	if (pl_mi == MAX_PARTY)
		return -1; //Shouldn't happen

	//Change leadership.
	p->party.member[mi].leader = 0;
	if (p->data[mi].sd->fd)
		clif_displaymessage(p->data[mi].sd->fd, msg_txt(284));
	p->party.member[pl_mi].leader = 1;
	if (p->data[pl_mi].sd->fd)
		clif_displaymessage(p->data[pl_mi].sd->fd, msg_txt(285));

	intif_party_leaderchange(p->party.party_id,p->party.member[pl_mi].account_id,p->party.member[pl_mi].char_id);
	//Update info.
	clif_party_info(p,NULL);

	return 0;
}

/*==========================================
 * @partyoption by Skotlex
 * Used to change the item share setting of a party.
 *------------------------------------------*/
int atcommand_partyoption(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	struct party_data *p;
	int mi, option;
	char w1[16], w2[16];
	nullpo_retr(-1, sd);

	if (sd->status.party_id == 0 || (p = party_search(sd->status.party_id)) == NULL)
	{
		clif_displaymessage(fd, msg_txt(282));
		return -1;
	}

	ARR_FIND( 0, MAX_PARTY, mi, p->data[mi].sd == sd );
	if (mi == MAX_PARTY)
		return -1; //Shouldn't happen

	if (!p->party.member[mi].leader)
	{
		clif_displaymessage(fd, msg_txt(282));
		return -1;
	}

	if(!message || !*message || sscanf(message, "%15s %15s", w1, w2) < 2)
	{
		clif_displaymessage(fd, "Command usage: @partyoption <pickup share: yes/no> <item distribution: yes/no>");
		return -1;
	}
	
	option = (config_switch(w1)?1:0)|(config_switch(w2)?2:0);

	//Change item share type.
	if (option != p->party.item)
		party_changeoption(sd, p->party.exp, option);
	else
		clif_displaymessage(fd, msg_txt(286));

	return 0;
}

/*==========================================
 * @autoloot by Upa-Kun
 * Turns on/off AutoLoot for a specific player
 *------------------------------------------*/
int atcommand_autoloot(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	int rate;
	double drate;
	nullpo_retr(-1, sd);
	// autoloot command without value
	if(!message || !*message)
	{
		if (sd->state.autoloot)
			rate = 0;
		else
			rate = 10000;
	} else {
		drate = atof(message);
		rate = (int)(drate*100);
	}
	if (rate < 0) rate = 0;
	if (rate > 10000) rate = 10000;

	sd->state.autoloot = rate;
	if (sd->state.autoloot) {
		snprintf(atcmd_output, sizeof atcmd_output, "Autolooting items with drop rates of %0.02f%% and below.",((double)sd->state.autoloot)/100.);
		clif_displaymessage(fd, atcmd_output);
	}else
		clif_displaymessage(fd, "Autoloot is now off.");

	if (sd->state.autoloot && sd->state.autolootid) {
		// Autolootitem should be turned off
		sd->state.autolootid = 0;
		clif_displaymessage(fd, "Autolootitem is now off.");
	}

	return 0;  
}
/*==========================================
 * @autolootitem
 *------------------------------------------*/
int atcommand_autolootitem(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	struct item_data *item_data = NULL;

	if (!message || !*message) {
		if (sd->state.autolootid) {
			sd->state.autolootid = 0;
			clif_displaymessage(fd, "Autolootitem have been turned OFF.");
		} else
			clif_displaymessage(fd, "Please, enter Item name or its ID (usage: @autolootitem <item_name_or_ID>).");

		return -1;
	}

	if ((item_data = itemdb_exists(atoi(message))) == NULL)
		item_data = itemdb_searchname(message);

	if (!item_data) {
		// No items founds in the DB with Id or Name
		clif_displaymessage(fd, "Item not found.");
		return -1;
	}

	sd->state.autolootid = item_data->nameid; // Autoloot Activated

	sprintf(atcmd_output, "Autolooting Item: '%s'/'%s' {%d}",
		item_data->name, item_data->jname, item_data->nameid);
	clif_displaymessage(fd, atcmd_output);

	if (sd->state.autolootid && sd->state.autoloot) {
		// Autoloot should be turned off
		sd->state.autoloot = 0;
		clif_displaymessage(fd, "Autoloot is now off (cannot be actitaved together).");
	}

	return 0;
}

/*==========================================
 * It is made to rain.
 *------------------------------------------*/
int atcommand_rain(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	nullpo_retr(-1, sd);
	if (map[sd->bl.m].flag.rain) {
		map[sd->bl.m].flag.rain=0;
		clif_weather(sd->bl.m);
		clif_displaymessage(fd, "The rain has stopped.");
	} else {
		map[sd->bl.m].flag.rain=1;
		clif_weather(sd->bl.m);
		clif_displaymessage(fd, "It is made to rain.");
	}
	return 0;
}

/*==========================================
 * It is made to snow.
 *------------------------------------------*/
int atcommand_snow(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	nullpo_retr(-1, sd);
	if (map[sd->bl.m].flag.snow) {
		map[sd->bl.m].flag.snow=0;
		clif_weather(sd->bl.m);
		clif_displaymessage(fd, "Snow has stopped falling.");
	} else {
		map[sd->bl.m].flag.snow=1;
		clif_weather(sd->bl.m);
		clif_displaymessage(fd, "It is made to snow.");
	}

	return 0;
}

/*==========================================
 * Cherry tree snowstorm is made to fall. (Sakura)
 *------------------------------------------*/
int atcommand_sakura(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	nullpo_retr(-1, sd);
	if (map[sd->bl.m].flag.sakura) {
		map[sd->bl.m].flag.sakura=0;
		clif_weather(sd->bl.m);
		clif_displaymessage(fd, "Cherry tree leaves no longer fall.");
	} else {
		map[sd->bl.m].flag.sakura=1;
		clif_weather(sd->bl.m);
		clif_displaymessage(fd, "Cherry tree leaves is made to fall.");
	}
	return 0;
}

/*==========================================
 * Clouds appear.
 *------------------------------------------*/
int atcommand_clouds(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	nullpo_retr(-1, sd);
	if (map[sd->bl.m].flag.clouds) {
		map[sd->bl.m].flag.clouds=0;
		clif_weather(sd->bl.m);
		clif_displaymessage(fd, "The clouds has disappear.");
	} else {
		map[sd->bl.m].flag.clouds=1;
		clif_weather(sd->bl.m);
		clif_displaymessage(fd, "Clouds appear.");
	}

	return 0;
}

/*==========================================
 * Different type of clouds using effect 516
 *------------------------------------------*/
int atcommand_clouds2(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	nullpo_retr(-1, sd);
	if (map[sd->bl.m].flag.clouds2) {
		map[sd->bl.m].flag.clouds2=0;
		clif_weather(sd->bl.m);
		clif_displaymessage(fd, "The alternative clouds disappear.");
	} else {
		map[sd->bl.m].flag.clouds2=1;
		clif_weather(sd->bl.m);
		clif_displaymessage(fd, "Alternative clouds appear.");
	}

	return 0;
}

/*==========================================
 * Fog hangs over.
 *------------------------------------------*/
int atcommand_fog(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	nullpo_retr(-1, sd);
	if (map[sd->bl.m].flag.fog) {
		map[sd->bl.m].flag.fog=0;
		clif_weather(sd->bl.m);
		clif_displaymessage(fd, "The fog has gone.");
	} else {
		map[sd->bl.m].flag.fog=1;
		clif_weather(sd->bl.m);
		clif_displaymessage(fd, "Fog hangs over.");
	}
		return 0;
}

/*==========================================
 * Fallen leaves fall.
 *------------------------------------------*/
int atcommand_leaves(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	nullpo_retr(-1, sd);
	if (map[sd->bl.m].flag.leaves) {
		map[sd->bl.m].flag.leaves=0;
		clif_weather(sd->bl.m);
		clif_displaymessage(fd, "Leaves no longer fall.");
	} else {
		map[sd->bl.m].flag.leaves=1;
		clif_weather(sd->bl.m);
		clif_displaymessage(fd, "Fallen leaves fall.");
	}

	return 0;
}

/*==========================================
 * Fireworks appear.
 *------------------------------------------*/
int atcommand_fireworks(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	nullpo_retr(-1, sd);
	if (map[sd->bl.m].flag.fireworks) {
		map[sd->bl.m].flag.fireworks=0;
		clif_weather(sd->bl.m);
		clif_displaymessage(fd, "Fireworks are ended.");
	} else {
		map[sd->bl.m].flag.fireworks=1;
		clif_weather(sd->bl.m);
		clif_displaymessage(fd, "Fireworks are launched.");
	}

	return 0;
}

/*==========================================
 * Clearing Weather Effects by Dexity
 *------------------------------------------*/
int atcommand_clearweather(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	nullpo_retr(-1, sd);
	map[sd->bl.m].flag.rain=0;
	map[sd->bl.m].flag.snow=0;
	map[sd->bl.m].flag.sakura=0;
	map[sd->bl.m].flag.clouds=0;
	map[sd->bl.m].flag.clouds2=0;
	map[sd->bl.m].flag.fog=0;
	map[sd->bl.m].flag.fireworks=0;
	map[sd->bl.m].flag.leaves=0;
	clif_weather(sd->bl.m);
	clif_displaymessage(fd, msg_txt(291));
	
	return 0;
}

/*===============================================================
 * Sound Command - plays a sound for everyone around! [Codemaster]
 *---------------------------------------------------------------*/
int atcommand_sound(const int fd, struct map_session_data *sd, const char *command, const char *message)
{
	char sound_file[100];

	memset(sound_file, '\0', sizeof(sound_file));

		if(!message || !*message || sscanf(message, "%99[^\n]", sound_file) < 1) {
		clif_displaymessage(fd, "Please, enter a sound filename. (usage: @sound <filename>)");
		return -1;
	}

	if(strstr(sound_file, ".wav") == NULL)
		strcat(sound_file, ".wav");

	clif_soundeffectall(&sd->bl, sound_file, 0, AREA);

	return 0;
}

/*==========================================
 * 	MOB Search
 *------------------------------------------*/
static int atmobsearch_sub(struct block_list *bl,va_list ap)
{
	int mob_id,fd;
	static int number=0;
	struct mob_data *md;

	nullpo_retr(0, bl);

	if(!ap){
		number=0;
		return 0;
	}
	mob_id = va_arg(ap,int);
	fd = va_arg(ap,int);

	md = (struct mob_data *)bl;

	if(md && fd && (mob_id==-1 || (md->class_==mob_id))){
		snprintf(atcmd_output, sizeof atcmd_output, "%2d[%3d:%3d] %s",
				++number,bl->x, bl->y,md->name);
		clif_displaymessage(fd, atcmd_output);
	}
	return 0;
}

int atcommand_mobsearch(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	char mob_name[100];
	int mob_id,map_id = 0;

	nullpo_retr(-1, sd);

	if (!message || !*message || sscanf(message, "%99[^\n]", mob_name) < 1) {
		clif_displaymessage(fd, "Please, enter a monster name (usage: @mobsearch <monster name>).");
		return -1;
	}

	if ((mob_id = atoi(mob_name)) == 0)
		 mob_id = mobdb_searchname(mob_name);
	if(mob_id > 0 && mobdb_checkid(mob_id) == 0){
		snprintf(atcmd_output, sizeof atcmd_output, "Invalid mob id %s!",mob_name);
		clif_displaymessage(fd, atcmd_output);
		return -1;
	}
	if(mob_id == atoi(mob_name) && mob_db(mob_id)->jname)
				strcpy(mob_name,mob_db(mob_id)->jname);	// --ja--
//				strcpy(mob_name,mob_db(mob_id)->name);	// --en--

	map_id = sd->bl.m;

	snprintf(atcmd_output, sizeof atcmd_output, "Mob Search... %s %s", mob_name, mapindex_id2name(sd->mapindex));
	clif_displaymessage(fd, atcmd_output);

	map_foreachinmap(atmobsearch_sub, map_id, BL_MOB, mob_id, fd);

	atmobsearch_sub(&sd->bl,0); // reset the counter

	return 0;
}

/*==========================================
 * @cleanmap - cleans items on the ground
 *------------------------------------------*/
static int atcommand_cleanmap_sub(struct block_list *bl, va_list ap)
{
	nullpo_retr(0, bl);
	map_clearflooritem(bl->id);

	return 0;
}

int atcommand_cleanmap(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	map_foreachinarea(atcommand_cleanmap_sub, sd->bl.m,
		sd->bl.x-AREA_SIZE*2, sd->bl.y-AREA_SIZE*2,
		sd->bl.x+AREA_SIZE*2, sd->bl.y+AREA_SIZE*2,
		BL_ITEM);
	clif_displaymessage(fd, "All dropped items have been cleaned up.");
	return 0;
}

/*==========================================
 * make a NPC/PET talk
 *------------------------------------------*/
int atcommand_npctalk(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	char name[NAME_LENGTH],mes[100],temp[100];
	struct npc_data *nd;

	if (sd->sc.count && //no "chatting" while muted.
		(sd->sc.data[SC_BERSERK] ||
		(sd->sc.data[SC_NOCHAT] && sd->sc.data[SC_NOCHAT]->val1&MANNER_NOCHAT)))
		return -1;

	if (!message || !*message || sscanf(message, "%23[^,], %99[^\n]", name, mes) < 2) {
		clif_displaymessage(fd, "Please, enter the correct info (usage: @npctalk <npc name>, <message>).");
		return -1;
	}

	if (!(nd = npc_name2id(name))) {
		clif_displaymessage(fd, msg_txt(111)); // This NPC doesn't exist
		return -1;
	}
	
	strtok(name, "#"); // discard extra name identifier if present
	snprintf(temp, sizeof(temp), "%s : %s", name, mes);
	clif_message(&nd->bl, temp);

	return 0;
}

int atcommand_pettalk(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	char mes[100],temp[100];
	struct pet_data *pd;

	nullpo_retr(-1, sd);

	if(!sd->status.pet_id || !(pd=sd->pd))
	{
		clif_displaymessage(fd, msg_txt(184));
		return -1;
	}

	if (sd->sc.count && //no "chatting" while muted.
		(sd->sc.data[SC_BERSERK] ||
		(sd->sc.data[SC_NOCHAT] && sd->sc.data[SC_NOCHAT]->val1&MANNER_NOCHAT)))
		return -1;

	if (!message || !*message || sscanf(message, "%99[^\n]", mes) < 1) {
		clif_displaymessage(fd, "Please, enter a message (usage: @pettalk <message>");
		return -1;
	}

	if (message[0] == '/')
	{// pet emotion processing
		const char* emo[] = {
			"/!", "/?", "/ho", "/lv", "/swt", "/ic", "/an", "/ag", "/$", "/...",
			"/scissors", "/rock", "/paper", "/korea", "/lv2", "/thx", "/wah", "/sry", "/heh", "/swt2",
			"/hmm", "/no1", "/??", "/omg", "/O", "/X", "/hlp", "/go", "/sob", "/gg",
			"/kis", "/kis2", "/pif", "/ok", "-?-", "-?-", "/bzz", "/rice", "/awsm", "/meh",
			"/shy", "/pat", "/mp", "/slur", "/com", "/yawn", "/grat", "/hp", "/philippines", "/usa",
			"/indonesia", "/brazil", "/fsh", "/spin", "/sigh", "/dum", "/crwd", "/desp", "/dice"
		};
		int i;
		ARR_FIND( 0, ARRAYLENGTH(emo), i, stricmp(message, emo[i]) == 0 );
		if( i < ARRAYLENGTH(emo) )
		{
			clif_emotion(&pd->bl, i);
			return 0;
		}
	}

	snprintf(temp, sizeof temp ,"%s : %s", pd->pet.name, mes);
	clif_message(&pd->bl, temp);

	return 0;
}

/*==========================================
 * @users - displays the number of players present on each map (percentage)
 *------------------------------------------*/

static DBMap* users_db = NULL; // unsigned int mapindex -> int users
static int users_all;

static int atcommand_users_sub1(struct map_session_data* sd,va_list va)
{
	int users = (int)(uidb_get(users_db,(unsigned int)sd->mapindex)) + 1;
	users_all++;
	uidb_put(users_db,(unsigned int)sd->mapindex,(void *)users);
	return 0;
}

static int atcommand_users_sub2(DBKey key,void* val,va_list va)
{
	char buf[256];
	struct map_session_data* sd = va_arg(va,struct map_session_data*);
	sprintf(buf,"%s: %d (%d%%)",mapindex_id2name(key.i),(int)val,(int)val * 100 / users_all);
	clif_displaymessage(sd->fd,buf);
	return 0;
}

int atcommand_users(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	char buf[256];
	users_all = 0;

	users_db->clear(users_db, NULL);
	clif_foreachclient(atcommand_users_sub1);
	users_db->foreach(users_db,atcommand_users_sub2,sd);
	sprintf(buf,"all: %d",users_all);
	clif_displaymessage(fd,buf);
	return 0;
}

/*==========================================
 *
 *------------------------------------------*/
int atcommand_reset(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	pc_resetstate(sd);
	pc_resetskill(sd,1);
	sprintf(atcmd_output, msg_txt(208), sd->status.name); // '%s' skill and stats points reseted!
	clif_displaymessage(fd, atcmd_output);
	return 0;
}

/*==========================================
 *
 *------------------------------------------*/
int atcommand_summon(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	char name[NAME_LENGTH];
	int mob_id = 0;
	int duration = 0;
	struct mob_data *md;
	unsigned int tick=gettick();

	nullpo_retr(-1, sd);

	if (!message || !*message || sscanf(message, "%23s %d", name, &duration) < 1)
	{
		clif_displaymessage(fd, "Please, enter a monster name (usage: @summon <monster name> [duration]");
		return -1;
	}

	if (duration < 1)
		duration =1;
	else if (duration > 60)
		duration =60;
	
	if ((mob_id = atoi(name)) == 0)
		mob_id = mobdb_searchname(name);
	if(mob_id == 0 || mobdb_checkid(mob_id) == 0)
	{
		clif_displaymessage(fd, msg_txt(40));	// Invalid monster ID or name.
		return -1;
	}

	md = mob_once_spawn_sub(&sd->bl, sd->bl.m, -1, -1, "--ja--", mob_id, "");

	if(!md)
		return -1;
	
	md->master_id=sd->bl.id;
	md->special_state.ai=1;
	md->deletetimer=add_timer(tick+(duration*60000),mob_timer_delete,md->bl.id,0);
	clif_misceffect2(&md->bl,344);
	mob_spawn(md);
	sc_start4(&md->bl, SC_MODECHANGE, 100, 1, 0, MD_AGGRESSIVE, 0, 60000);
	clif_skill_poseffect(&sd->bl,AM_CALLHOMUN,1,md->bl.x,md->bl.y,tick);
	clif_displaymessage(fd, msg_txt(39));	// All monster summoned!
	
	return 0;
}

/*==========================================
 * @adjcmdlvl by [MouseJstr]
 *
 * Temp adjust the GM level required to use a GM command
 * Useful during beta testing to allow players to use GM commands for short periods of time
 *------------------------------------------*/
int atcommand_adjcmdlvl(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	int newlev;
	char name[100];
	AtCommandInfo* cmd;

	nullpo_retr(-1, sd);

	if (!message || !*message || sscanf(message, "%d %99s", &newlev, name) != 2)
	{
		clif_displaymessage(fd, "Usage: @adjcmdlvl <lvl> <command>.");
		return -1;
	}

	cmd = get_atcommandinfo_byname(name);
	if (cmd == NULL)
	{
		clif_displaymessage(fd, "@command not found.");
		return -1;
	}
	else if (newlev > pc_isGM(sd))
	{
		clif_displaymessage(fd, "You can't make a command require higher GM level than your own.");
		return -1;
	}
	else if (cmd->level > pc_isGM(sd))
	{
		clif_displaymessage(fd, "You can't adjust the level of a command which's level is above your own.");
		return -1;
	}
	else
	{
		cmd->level = newlev;
		clif_displaymessage(fd, "@command level changed.");
		return 0;
	}
}

/*==========================================
 * @adjgmlvl by [MouseJstr]
 * Create a temp GM
 * Useful during beta testing to allow players to use GM commands for short periods of time
 *------------------------------------------*/
int atcommand_adjgmlvl(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
    int newlev;
    char user[NAME_LENGTH];
    struct map_session_data *pl_sd;
	nullpo_retr(-1, sd);

    if (!message || !*message || sscanf(message, "%d %23[^\r\n]", &newlev, user) != 2) {
        clif_displaymessage(fd, "Usage: @adjgmlvl <lvl> <user>.");
        return -1;
    }

	if ( (pl_sd = map_nick2sd(user)) == NULL )
	{
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

    pc_set_gm_level(pl_sd->status.account_id, newlev);

    return 0;
}

/*==========================================
 * @trade by [MouseJstr]
 * Open a trade window with a remote player
 *------------------------------------------*/
int atcommand_trade(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
    struct map_session_data *pl_sd = NULL;
	nullpo_retr(-1, sd);

	if (!message || !*message) {
		clif_displaymessage(fd, "Please, enter a player name (usage: @trade <player>).");
		return -1;
	}

	if ( (pl_sd = map_nick2sd((char *)message)) == NULL )
	{
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	trade_traderequest(sd, pl_sd);
	return 0;
}

/*==========================================
 * @setbattleflag by [MouseJstr]
 * set a battle_config flag without having to reboot
 *------------------------------------------*/
int atcommand_setbattleflag(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	char flag[128], value[128];
	nullpo_retr(-1, sd);

	if (!message || !*message || sscanf(message, "%127s %127s", flag, value) != 2) {
        	clif_displaymessage(fd, "Usage: @setbattleflag <flag> <value>.");
        	return -1;
    	}

	if (battle_set_value(flag, value) == 0)
	{
		clif_displaymessage(fd, "unknown battle_config flag");
		return -1;
	}

	clif_displaymessage(fd, "battle_config set as requested");

	return 0;
}

/*==========================================
 * @unmute [Valaris]
 *------------------------------------------*/
int atcommand_unmute(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	struct map_session_data *pl_sd = NULL;
	nullpo_retr(-1, sd);

	if (!message || !*message) {
		clif_displaymessage(fd, "Please, enter a player name (usage: @unmute <player>).");
		return -1;
	}

	if ( (pl_sd = map_nick2sd((char *)message)) == NULL )
	{
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	if(!pl_sd->sc.data[SC_NOCHAT]) {
		clif_displaymessage(sd->fd,"Player is not muted");
		return -1;
	}

	pl_sd->status.manner = 0;
	status_change_end(&pl_sd->bl,SC_NOCHAT,-1);
	clif_displaymessage(sd->fd,"Player unmuted");
	
	return 0;
}

/*==========================================
 * @uptime by MC Cameri
 *------------------------------------------*/
int atcommand_uptime(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	unsigned long seconds = 0, day = 24*60*60, hour = 60*60,
		minute = 60, days = 0, hours = 0, minutes = 0;
	nullpo_retr(-1, sd);

	seconds = get_uptime();
	days = seconds/day;
	seconds -= (seconds/day>0)?(seconds/day)*day:0;
	hours = seconds/hour;
	seconds -= (seconds/hour>0)?(seconds/hour)*hour:0;
	minutes = seconds/minute;
	seconds -= (seconds/minute>0)?(seconds/minute)*minute:0;

	snprintf(atcmd_output, sizeof(atcmd_output), msg_txt(245), days, hours, minutes, seconds);
	clif_displaymessage(fd, atcmd_output);

	return 0;
}

/*==========================================
 * @changesex <sex>
 * => Changes one's sex. Argument sex can be 0 or 1, m or f, male or female.
 *------------------------------------------*/
int atcommand_changesex(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	nullpo_retr(-1, sd);
	chrif_changesex(sd);
	return 0;
}

/*================================================
 * @mute - Mutes a player for a set amount of time
 *------------------------------------------------*/
int atcommand_mute(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	struct map_session_data *pl_sd = NULL;
	int manner;
	nullpo_retr(-1, sd);

	if (!message || !*message || sscanf(message, "%d %23[^\n]", &manner, atcmd_player_name) < 1) {
		clif_displaymessage(fd, "Usage: @mute <time> <character name>.");
		return -1;
	}

	if ( (pl_sd = map_nick2sd(atcmd_player_name)) == NULL )
	{
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	if ( pc_isGM(sd) < pc_isGM(pl_sd) )
	{
		clif_displaymessage(fd, msg_txt(81)); // Your GM level don't authorise you to do this action on this player.
		return -1;
	}

	clif_manner_message(sd, 0);
	clif_manner_message(pl_sd, 5);

	if( pl_sd->status.manner < manner ) {
		pl_sd->status.manner -= manner;
		sc_start(&pl_sd->bl,SC_NOCHAT,100,0,0);
	} else {
		pl_sd->status.manner = 0;
		status_change_end(&pl_sd->bl,SC_NOCHAT,-1);
	}

	clif_GM_silence(sd, pl_sd, (manner > 0 ? 1 : 0));

	return 0;
}

/*==========================================
 * @refresh (like @jumpto <<yourself>>)
 *------------------------------------------*/
int atcommand_refresh(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	nullpo_retr(-1, sd);
	clif_refresh(sd);
	return 0;
}

/*==========================================
 * @identify
 * => GM's magnifier.
 *------------------------------------------*/
int atcommand_identify(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	int i,num;

	nullpo_retr(-1, sd);

	for(i=num=0;i<MAX_INVENTORY;i++){
		if(sd->status.inventory[i].nameid > 0 && sd->status.inventory[i].identify!=1){
			num++;
		}
	}
	if (num > 0) {
		clif_item_identify_list(sd);
	} else {
		clif_displaymessage(fd,"There are no items to appraise.");
	}
	return 0;
}

/*==========================================
 * @gmotd (Global MOTD)
 * by davidsiaw :P
 *------------------------------------------*/
int atcommand_gmotd(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
		char buf[256];
		FILE *fp;
	nullpo_retr(-1, sd);
		if((fp = fopen(motd_txt, "r"))!=NULL){
			while(fgets(buf, sizeof(buf), fp) != NULL)
			{
				int i;
				if (buf[0] == '/' && buf[1] == '/')
					continue;
				for(i=0; buf[i]; i++){
					if(buf[i]=='\r' || buf[i]=='\n'){
						buf[i]=0;
						break;
					}
				}
				intif_GMmessage(buf,strlen(buf)+1,8);
			}
			fclose(fp);
		}
		return 0;
}

int atcommand_misceffect(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	int effect = 0;
	nullpo_retr(-1, sd);
	if (!message || !*message)
		return -1;
	if (sscanf(message, "%d", &effect) < 1)
		return -1;
	clif_misceffect(&sd->bl,effect);

	return 0;
}

/*==========================================
 * MAIL SYSTEM
 *------------------------------------------*/
int atcommand_mail(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	nullpo_retr(0,sd);
#ifndef TXT_ONLY
	mail_openmail(sd);
#endif
	return 0;
}

/*==========================================
 * Show Monster DB Info   v 1.0
 * originally by [Lupus] eAthena
 *------------------------------------------*/
int atcommand_mobinfo(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	unsigned char msize[3][7] = {"Small", "Medium", "Large"};
	unsigned char mrace[12][11] = {"Formless", "Undead", "Beast", "Plant", "Insect", "Fish", "Demon", "Demi-Human", "Angel", "Dragon", "Boss", "Non-Boss"};
	unsigned char melement[10][8] = {"Neutral", "Water", "Earth", "Fire", "Wind", "Poison", "Holy", "Dark", "Ghost", "Undead"};
	char atcmd_output2[200];
	struct item_data *item_data;
	struct mob_db *mob, *mob_array[MAX_SEARCH];
	int count;
	int i, j, k;

	memset(atcmd_output, '\0', sizeof(atcmd_output));
	memset(atcmd_output2, '\0', sizeof(atcmd_output2));

	if (!message || !*message) {
		clif_displaymessage(fd, "Please, enter a Monster/ID (usage: @mobinfo <monster_name_or_monster_ID>).");
		return -1;
	}

	// If monster identifier/name argument is a name
	if ((i = mobdb_checkid(atoi(message))))
	{
		mob_array[0] = mob_db(i);
		count = 1;
	} else
		count = mobdb_searchname_array(mob_array, MAX_SEARCH, message);

	if (!count) {
		clif_displaymessage(fd, msg_txt(40)); // Invalid monster ID or name.
		return -1;
	}

	if (count > MAX_SEARCH) {
		sprintf(atcmd_output, msg_txt(269), MAX_SEARCH, count);
		clif_displaymessage(fd, atcmd_output);
		count = MAX_SEARCH;
	}
	for (k = 0; k < count; k++) {
		mob = mob_array[k];

		// stats
		if (mob->mexp)
			sprintf(atcmd_output, "MVP Monster: '%s'/'%s'/'%s' (%d)", mob->name, mob->jname, mob->sprite, mob->vd.class_);
		else
			sprintf(atcmd_output, "Monster: '%s'/'%s'/'%s' (%d)", mob->name, mob->jname, mob->sprite, mob->vd.class_);
		clif_displaymessage(fd, atcmd_output);
		sprintf(atcmd_output, " Level:%d  HP:%d  SP:%d  Base EXP:%u  Job EXP:%u", mob->lv, mob->status.max_hp, mob->status.max_sp, mob->base_exp, mob->job_exp);
		clif_displaymessage(fd, atcmd_output);
		sprintf(atcmd_output, " DEF:%d  MDEF:%d  STR:%d  AGI:%d  VIT:%d  INT:%d  DEX:%d  LUK:%d",
			mob->status.def, mob->status.mdef, mob->status.str, mob->status.agi,
			mob->status.vit, mob->status.int_, mob->status.dex, mob->status.luk);
		clif_displaymessage(fd, atcmd_output);
		
		sprintf(atcmd_output, " ATK:%d~%d  Range:%d~%d~%d  Size:%s  Race: %s  Element: %s (Lv:%d)",
			mob->status.rhw.atk, mob->status.rhw.atk2, mob->status.rhw.range,
			mob->range2 , mob->range3, msize[mob->status.size],
			mrace[mob->status.race], melement[mob->status.def_ele], mob->status.ele_lv);
		clif_displaymessage(fd, atcmd_output);
		// drops
		clif_displaymessage(fd, " Drops:");
		strcpy(atcmd_output, " ");
		j = 0;
		for (i = 0; i < MAX_MOB_DROP; i++) {
			if (mob->dropitem[i].nameid <= 0 || mob->dropitem[i].p < 1 || (item_data = itemdb_search(mob->dropitem[i].nameid)) == NULL)
				continue;
			if (item_data->slot)
				sprintf(atcmd_output2, " - %s[%d]  %02.02f%%", item_data->jname, item_data->slot, (float)mob->dropitem[i].p / 100);
			else
				sprintf(atcmd_output2, " - %s  %02.02f%%", item_data->jname, (float)mob->dropitem[i].p / 100);
			strcat(atcmd_output, atcmd_output2);
			if (++j % 3 == 0) {
				clif_displaymessage(fd, atcmd_output);
				strcpy(atcmd_output, " ");
			}
		}
		if (j == 0)
			clif_displaymessage(fd, "This monster has no drops.");
		else if (j % 3 != 0)
			clif_displaymessage(fd, atcmd_output);
		// mvp
		if (mob->mexp) {
			sprintf(atcmd_output, " MVP Bonus EXP:%d  %02.02f%%", mob->mexp, (float)mob->mexpper / 100);
			clif_displaymessage(fd, atcmd_output);
			strcpy(atcmd_output, " MVP Items:");
			j = 0;
			for (i = 0; i < 3; i++) {
				if (mob->mvpitem[i].nameid <= 0 || (item_data = itemdb_search(mob->mvpitem[i].nameid)) == NULL)
					continue;
				if (mob->mvpitem[i].p > 0) {
					j++;
					if (j == 1)
						sprintf(atcmd_output2, " %s  %02.02f%%", item_data->name, (float)mob->mvpitem[i].p / 100);
					else
						sprintf(atcmd_output2, " - %s  %02.02f%%", item_data->name, (float)mob->mvpitem[i].p / 100);
					strcat(atcmd_output, atcmd_output2);
				}
			}
			if (j == 0)
				clif_displaymessage(fd, "This monster has no MVP prizes.");
			else
				clif_displaymessage(fd, atcmd_output);
		}
	}
	return 0;
}

/*=========================================
* @showmobs by KarLaeda
* => For 5 sec displays the mobs on minimap
*------------------------------------------*/
int atshowmobs_timer(int tid, unsigned int tick, int id, int data)
{
	struct map_session_data *sd;

	if (!session[id] || (sd = session[id]->session_data) == NULL)
		return 0;

	clif_viewpoint(sd, 1, 2, 0, 0, data, 0xFFFFFF);
	return 1;
}
static int atshowmobs_sub(struct block_list *bl,va_list ap)
{
    int mob_id,fd;
    struct map_session_data* sd;
    static int number=0;
    struct mob_data *md;

    if(!ap){
        number=0;
        return 0;
    }
    mob_id = va_arg(ap,int);
    fd = va_arg(ap,int);
    sd = va_arg(ap,struct map_session_data*);

    md = (struct mob_data *)bl;

	if(md->special_state.ai || md->master_id)
		 return 0; //Hide slaves and player summoned mobs. [Skotlex]

    if(fd && (mob_id==-1 || (md->class_==mob_id))){
        clif_viewpoint(sd, 1, 1, bl->x, bl->y, ++number, 0xFFFFFF);
        add_timer(gettick()+5000, atshowmobs_timer, fd, number);
    }
    return 0;
}
int atcommand_showmobs(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
    char mob_name[100];
    int mob_id,map_id = 0;

    nullpo_retr(-1, sd);

    if (sscanf(message, "%99[^\n]", mob_name) < 0)
        return -1;

    if ((mob_id = atoi(mob_name)) == 0)
         mob_id = mobdb_searchname(mob_name);
    if(mob_id > 0 && mobdb_checkid(mob_id) == 0){
        snprintf(atcmd_output, sizeof atcmd_output, "Invalid mob id %s!",mob_name);
        clif_displaymessage(fd, atcmd_output);
        return 0;
    }
// Uncomment the following line to show mini-bosses & MVP.
//#define SHOW_MVP
#ifndef SHOW_MVP
    if(mob_db(mob_id)->status.mode&MD_BOSS){
        snprintf(atcmd_output, sizeof atcmd_output, "Can't show Boss mobs!");
        clif_displaymessage(fd, atcmd_output);
        return 0;
    }
#endif
    if(mob_id == atoi(mob_name) && mob_db(mob_id)->jname)
                strcpy(mob_name,mob_db(mob_id)->jname);    // --ja--
//                strcpy(mob_name,mob_db(mob_id)->name);    // --en--

    map_id = sd->bl.m;

    snprintf(atcmd_output, sizeof atcmd_output, "Mob Search... %s %s",
        mob_name, mapindex_id2name(sd->mapindex));
    clif_displaymessage(fd, atcmd_output);

    map_foreachinmap(atshowmobs_sub, map_id, BL_MOB, mob_id, fd, sd);

    atshowmobs_sub(&sd->bl,0);

    return 0;
}

/*==========================================
 * homunculus level up [orn]
 *------------------------------------------*/
int atcommand_homlevel(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	TBL_HOM * hd;
	int level = 0, i = 0;

	nullpo_retr(-1, sd);

	if (!message || !*message) {
		clif_displaymessage(fd, "Please, enter a level adjustment: (usage: @homlevel <+/- # of levels>.");
		return -1;
	}
		
	if ( !merc_is_hom_active(sd->hd) ) {
		clif_displaymessage(fd, "You do not have a homunculus.");
		return -1;
	}

	level = atoi(message);
	hd = sd->hd;
	
	for (i = 1; i <= level && hd->exp_next; i++){
		hd->homunculus.exp += hd->exp_next;
		merc_hom_levelup(hd);
	}
	status_calc_homunculus(hd,0);
	status_percent_heal(&hd->bl, 100, 100);
	clif_misceffect2(&hd->bl,568);
	return 0;
}

/*==========================================
 * homunculus evolution H [orn]
 *------------------------------------------*/
int atcommand_homevolution(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	nullpo_retr(-1, sd);

	if ( !merc_is_hom_active(sd->hd) ) {
		clif_displaymessage(fd, "You do not have a homunculus.");
		return -1;
	}

	if ( !merc_hom_evolution(sd->hd) ) {
		clif_displaymessage(fd, "Your homunculus doesn't evolve.");
		return -1;
	}

	return 0;
}

/*==========================================
 * call choosen homunculus [orn]
 *------------------------------------------*/
int atcommand_makehomun(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	int homunid;
	nullpo_retr(-1, sd);

	if ( merc_is_hom_active(sd->hd) ) {
		clif_displaymessage(fd, msg_txt(450));
		return -1;
	}

	if (!message || !*message) {
		clif_displaymessage(fd, "Please, enter a homunculus id: (usage: @makehomun <homunculus id>.");
		return -1;
	}

	homunid = atoi(message);
	if( homunid < HM_CLASS_BASE || homunid > HM_CLASS_BASE + MAX_HOMUNCULUS_CLASS - 1 )
	{
		clif_displaymessage(fd, "Invalid Homunculus id.");
		return -1;
	}

	merc_create_homunculus_request(sd,homunid);
	return 0;
}

/*==========================================
 * modify homunculus intimacy [orn]
 *------------------------------------------*/
int atcommand_homfriendly(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	int friendly = 0;

	nullpo_retr(-1, sd);

	if ( !merc_is_hom_active(sd->hd) ) {
		clif_displaymessage(fd, "You do not have a homunculus.");
		return -1;
	}

	if (!message || !*message) {
		clif_displaymessage(fd, "Please, enter a friendly value: (usage: @homfriendly <friendly value[0-1000]>.");
		return -1;
	}

	friendly = atoi(message);
	friendly = cap_value(friendly, 0, 1000);

	sd->hd->homunculus.intimacy = friendly * 100 ;
	clif_send_homdata(sd,SP_INTIMATE,friendly);
	return 0;
}

/*==========================================
 * modify homunculus hunger [orn]
 *------------------------------------------*/
int atcommand_homhungry(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	int hungry = 0;

	nullpo_retr(-1, sd);

	if ( !merc_is_hom_active(sd->hd) ) {
		clif_displaymessage(fd, "You do not have a homunculus.");
		return -1;
	}

	if (!message || !*message) {
		clif_displaymessage(fd, "Please, enter a hunger value: (usage: @homhungry <hunger value[0-100]>.");
		return -1;
	}

	hungry = atoi(message);
	hungry = cap_value(hungry, 0, 100);

	sd->hd->homunculus.hunger = hungry;
	clif_send_homdata(sd,SP_HUNGRY,hungry);
	return 0;
}

/*==========================================
 * make the homunculus speak [orn]
 *------------------------------------------*/
int atcommand_homtalk(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	char mes[100],temp[100];

	nullpo_retr(-1, sd);

	if (sd->sc.count && //no "chatting" while muted.
		(sd->sc.data[SC_BERSERK] ||
		(sd->sc.data[SC_NOCHAT] && sd->sc.data[SC_NOCHAT]->val1&MANNER_NOCHAT)))
		return -1;

	if ( !merc_is_hom_active(sd->hd) ) {
		clif_displaymessage(fd, "You do not have a homunculus.");
		return -1;
	}

	if (!message || !*message || sscanf(message, "%99[^\n]", mes) < 1) {
		clif_displaymessage(fd, "Please, enter a message (usage: @homtalk <message>");
		return -1;
	}

	snprintf(temp, sizeof temp ,"%s : %s", sd->hd->homunculus.name, mes);
	clif_message(&sd->hd->bl, temp);

	return 0;
}

/*==========================================
 * Show homunculus stats
 *------------------------------------------*/
int atcommand_hominfo(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	struct homun_data *hd;
	struct status_data *status;
	nullpo_retr(-1, sd);

	if ( !merc_is_hom_active(sd->hd) ) {
		clif_displaymessage(fd, "You do not have a homunculus.");
		return -1;
	}

	hd = sd->hd;
	status = status_get_status_data(&hd->bl);
	clif_displaymessage(fd, "Homunculus stats :");

	snprintf(atcmd_output, sizeof(atcmd_output) ,"HP : %d/%d - SP : %d/%d",
		status->hp, status->max_hp, status->sp, status->max_sp);
	clif_displaymessage(fd, atcmd_output);

	snprintf(atcmd_output, sizeof(atcmd_output) ,"ATK : %d - MATK : %d~%d",
		status->rhw.atk2 +status->batk, status->matk_min, status->matk_max);
	clif_displaymessage(fd, atcmd_output);

	snprintf(atcmd_output, sizeof(atcmd_output) ,"Hungry : %d - Intimacy : %u",
		hd->homunculus.hunger, hd->homunculus.intimacy/100);
	clif_displaymessage(fd, atcmd_output);

	snprintf(atcmd_output, sizeof(atcmd_output) ,
		"Stats: Str %d / Agi %d / Vit %d / Int %d / Dex %d / Luk %d",
		status->str, status->agi, status->vit,
		status->int_, status->dex, status->luk);
	clif_displaymessage(fd, atcmd_output);

	return 0;
}

int atcommand_homstats(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	struct homun_data *hd;
	struct s_homunculus_db *db;
	struct s_homunculus *hom;
	int lv, min, max, evo;

	nullpo_retr(-1, sd);

	if ( !merc_is_hom_active(sd->hd) ) {
		clif_displaymessage(fd, "You do not have a homunculus.");
		return -1;
	}

	hd = sd->hd;
	
	hom = &hd->homunculus;
	db = hd->homunculusDB;
	lv = hom->level;

	snprintf(atcmd_output, sizeof(atcmd_output) ,
		"Homunculus growth stats (Lv %d %s):", lv, db->name);
	clif_displaymessage(fd, atcmd_output);
	lv--; //Since the first increase is at level 2.
	
	evo = (hom->class_ == db->evo_class);
	min = db->base.HP +lv*db->gmin.HP +(evo?db->emin.HP:0);
	max = db->base.HP +lv*db->gmax.HP +(evo?db->emax.HP:0);;
	snprintf(atcmd_output, sizeof(atcmd_output) ,"Max HP: %d (%d~%d)", hom->max_hp, min, max);
	clif_displaymessage(fd, atcmd_output);

	min = db->base.SP +lv*db->gmin.SP +(evo?db->emin.SP:0);
	max = db->base.SP +lv*db->gmax.SP +(evo?db->emax.SP:0);;
	snprintf(atcmd_output, sizeof(atcmd_output) ,"Max SP: %d (%d~%d)", hom->max_sp, min, max);
	clif_displaymessage(fd, atcmd_output);

	min = db->base.str +lv*(db->gmin.str/10) +(evo?db->emin.str:0);
	max = db->base.str +lv*(db->gmax.str/10) +(evo?db->emax.str:0);;
	snprintf(atcmd_output, sizeof(atcmd_output) ,"Str: %d (%d~%d)", hom->str/10, min, max);
	clif_displaymessage(fd, atcmd_output);

	min = db->base.agi +lv*(db->gmin.agi/10) +(evo?db->emin.agi:0);
	max = db->base.agi +lv*(db->gmax.agi/10) +(evo?db->emax.agi:0);;
	snprintf(atcmd_output, sizeof(atcmd_output) ,"Agi: %d (%d~%d)", hom->agi/10, min, max);
	clif_displaymessage(fd, atcmd_output);

	min = db->base.vit +lv*(db->gmin.vit/10) +(evo?db->emin.vit:0);
	max = db->base.vit +lv*(db->gmax.vit/10) +(evo?db->emax.vit:0);;
	snprintf(atcmd_output, sizeof(atcmd_output) ,"Vit: %d (%d~%d)", hom->vit/10, min, max);
	clif_displaymessage(fd, atcmd_output);

	min = db->base.int_ +lv*(db->gmin.int_/10) +(evo?db->emin.int_:0);
	max = db->base.int_ +lv*(db->gmax.int_/10) +(evo?db->emax.int_:0);;
	snprintf(atcmd_output, sizeof(atcmd_output) ,"Int: %d (%d~%d)", hom->int_/10, min, max);
	clif_displaymessage(fd, atcmd_output);

	min = db->base.dex +lv*(db->gmin.dex/10) +(evo?db->emin.dex:0);
	max = db->base.dex +lv*(db->gmax.dex/10) +(evo?db->emax.dex:0);;
	snprintf(atcmd_output, sizeof(atcmd_output) ,"Dex: %d (%d~%d)", hom->dex/10, min, max);
	clif_displaymessage(fd, atcmd_output);

	min = db->base.luk +lv*(db->gmin.luk/10) +(evo?db->emin.luk:0);
	max = db->base.luk +lv*(db->gmax.luk/10) +(evo?db->emax.luk:0);;
	snprintf(atcmd_output, sizeof(atcmd_output) ,"Luk: %d (%d~%d)", hom->luk/10, min, max);
	clif_displaymessage(fd, atcmd_output);

	return 0;
}

int atcommand_homshuffle(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	TBL_PC* tsd = sd;

	nullpo_retr(-1, sd);

	if ((!message || !*message) && !sd->hd)
	{
		clif_displaymessage(fd, "usage: @homshuffle <Alchemist's name>");
		clif_displaymessage(fd, "Use this to recalculate your (or someone else's) homunculus growth data");
		return -1;
	}
	if (message && *message) {
		tsd = map_nick2sd((char*)message);
		if (!tsd) {
			clif_displaymessage(fd, msg_txt(3)); // Character not found.
			return -1;
		}
		if (pc_isGM(tsd) > pc_isGM(sd)) {
			clif_displaymessage(fd, msg_txt(81)); // Your GM level don't authorise you to do this action on this player.
			return -1;
		}
	}

	if(!merc_hom_shuffle(tsd->hd))
		return -1;

	clif_displaymessage(sd->fd, "[Homunculus Stats Altered]");
	//Print out the new stats
	//This will send the commands to the invoker since they all use this fd regardless of sd value.
	atcommand_homstats(fd, tsd, command, message);
	return 0;
}

/*==========================================
 * Show Items DB Info   v 1.0
 * originally by [Lupus] eAthena
 *------------------------------------------*/
int atcommand_iteminfo(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	char *itype[12] = {"Potion/Food", "BUG!", "Usable", "Etc", "Weapon", "Protection", "Card", "Egg", "Pet Acessory", "BUG!", "Arrow"};
	//, "Lure/Scroll"}; No need, type 11 items are converted to type 2 upon loading [Skotlex]

	struct item_data *item_data, *item_array[MAX_SEARCH];
	int i, count = 1;

	if (!message || !*message) {
		clif_displaymessage(fd, "Please, enter Item name or its ID (usage: @ii/@iteminfo <item name or ID>).");
		return -1;
	}
	if ((item_array[0] = itemdb_exists(atoi(message))) == NULL)
		count = itemdb_searchname_array(item_array, MAX_SEARCH, message);

	if (!count) {
		clif_displaymessage(fd, msg_txt(19));	// Invalid item ID or name.
		return -1;
	}

	if (count > MAX_SEARCH) {
		sprintf(atcmd_output, msg_txt(269), MAX_SEARCH, count); // Displaying first %d out of %d matches
		clif_displaymessage(fd, atcmd_output);
		count = MAX_SEARCH;
	}
	for (i = 0; i < count; i++) {
		item_data = item_array[i];
		sprintf(atcmd_output, "Item: '%s'/'%s'[%d] (%d) Type: %s | Extra Effect: %s",
			item_data->name,item_data->jname,item_data->slot,item_data->nameid,
			item_data->type < 12 ? itype[item_data->type] : "BUG!", 
			(item_data->script==NULL)? "None" : "With script"
		);
		clif_displaymessage(fd, atcmd_output);

		sprintf(atcmd_output, "NPC Buy:%dz%s, Sell:%dz%s | Weight: %.1f ", item_data->value_buy, item_data->flag.value_notdc ? "(No Discount!)":"", item_data->value_sell, item_data->flag.value_notoc ? "(No Overcharge!)":"", item_data->weight/10. );
		clif_displaymessage(fd, atcmd_output);

		if (item_data->maxchance == 10000)
			strcpy(atcmd_output, " - Available in the shops only");
		else if (item_data->maxchance)
			sprintf(atcmd_output, " - Maximal monsters drop chance: %02.02f%%", (float)item_data->maxchance / 100 );
		else
			strcpy(atcmd_output, " - Monsters don't drop this item");
		clif_displaymessage(fd, atcmd_output);

	}
	return 0;
}

/*==========================================
 * Show who drops the item.
 *------------------------------------------*/
int atcommand_whodrops(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	struct item_data *item_data, *item_array[MAX_SEARCH];
	int i,j, count = 1;

	if (!message || !*message) {
		clif_displaymessage(fd, "Please, enter Item name or its ID (usage: @whodrops <item name or ID>).");
		return -1;
	}
	if ((item_array[0] = itemdb_exists(atoi(message))) == NULL)
		count = itemdb_searchname_array(item_array, MAX_SEARCH, message);

	if (!count) {
		clif_displaymessage(fd, msg_txt(19));	// Invalid item ID or name.
		return -1;
	}

	if (count > MAX_SEARCH) {
		sprintf(atcmd_output, msg_txt(269), MAX_SEARCH, count); // Displaying first %d out of %d matches
		clif_displaymessage(fd, atcmd_output);
		count = MAX_SEARCH;
	}
	for (i = 0; i < count; i++) {
		item_data = item_array[i];
		sprintf(atcmd_output, "Item: '%s'[%d]",
			item_data->jname,item_data->slot);
		clif_displaymessage(fd, atcmd_output);

		if (item_data->mob[0].chance == 0) {
			strcpy(atcmd_output, " - Item is not dropped by mobs.");
			clif_displaymessage(fd, atcmd_output);
		} else {
			sprintf(atcmd_output, "- Common mobs with highest drop chance (only max %d are listed):", MAX_SEARCH);
			clif_displaymessage(fd, atcmd_output);
		
			for (j=0; j < MAX_SEARCH && item_data->mob[j].chance > 0; j++)
			{
				sprintf(atcmd_output, "- %s (%02.02f%%)", mob_db(item_data->mob[j].id)->jname, item_data->mob[j].chance/100.);
				clif_displaymessage(fd, atcmd_output);
			}
		}
	}
	return 0;
}

int atcommand_whereis(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	struct mob_db *mob, *mob_array[MAX_SEARCH];
	int count;
	int i, j, k;

	if (!message || !*message) {
		clif_displaymessage(fd, "Please, enter a Monster/ID (usage: @whereis<monster_name_or_monster_ID>).");
		return -1;
	}

	// If monster identifier/name argument is a name
	if ((i = mobdb_checkid(atoi(message))))
	{
		mob_array[0] = mob_db(i);
		count = 1;
	} else
		count = mobdb_searchname_array(mob_array, MAX_SEARCH, message);

	if (!count) {
		clif_displaymessage(fd, msg_txt(40)); // Invalid monster ID or name.
		return -1;
	}

	if (count > MAX_SEARCH) {
		sprintf(atcmd_output, msg_txt(269), MAX_SEARCH, count);
		clif_displaymessage(fd, atcmd_output);
		count = MAX_SEARCH;
	}
	for (k = 0; k < count; k++) {
		mob = mob_array[k];
		snprintf(atcmd_output, sizeof atcmd_output, "%s spawns in:", mob->jname);
		clif_displaymessage(fd, atcmd_output);

		for (i = 0; i < ARRAYLENGTH(mob->spawn) && mob->spawn[i].qty; i++)
		{
			j = map_mapindex2mapid(mob->spawn[i].mapindex);
			if (j < 0) continue;
			snprintf(atcmd_output, sizeof atcmd_output, "%s (%d)", map[j].name, mob->spawn[i].qty);
			clif_displaymessage(fd, atcmd_output);
		}
		if (i == 0)
			clif_displaymessage(fd, "This monster does not spawn normally.");
	}

	return 0;
}

/*==========================================
 * @adopt by [Veider]
 * adopt a novice
 *------------------------------------------*/
int atcommand_adopt(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	struct map_session_data *pl_sd1 = NULL;
	struct map_session_data *pl_sd2 = NULL;
	struct map_session_data *pl_sd3 = NULL;
	char player1[NAME_LENGTH], player2[NAME_LENGTH], player3[NAME_LENGTH];
	char output[256];

	nullpo_retr(-1, sd);

	if (!message || !*message || sscanf(message, "%23[^,],%23[^,],%23[^\r\n]", player1, player2, player3) < 3) {
		clif_displaymessage(fd, "usage: @adopt <father>,<mother>,<child>.");
		return -1;
	}

	if (battle_config.etc_log)
		ShowInfo("Adopting: --%s--%s--%s--\n",player1,player2,player3);

	if((pl_sd1=map_nick2sd((char *) player1)) == NULL) {
		sprintf(output, "Cannot find player %s online", player1);
		clif_displaymessage(fd, output);
		return -1;
	}

	if((pl_sd2=map_nick2sd((char *) player2)) == NULL) {
		sprintf(output, "Cannot find player %s online", player2);
		clif_displaymessage(fd, output);
		return -1;
	}
 
	if((pl_sd3=map_nick2sd((char *) player3)) == NULL) {
		sprintf(output, "Cannot find player %s online", player3);
		clif_displaymessage(fd, output);
		return -1;
	}

	if( pc_adoption(pl_sd1, pl_sd2, pl_sd3) != 0 ) {
		return -1;
	}
	
	clif_displaymessage(fd, "They are family... wish them luck");
	return 0;
}

int atcommand_version(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	const char * revision;

	if ((revision = get_svn_revision()) != 0) {
		sprintf(atcmd_output,"eAthena Version SVN r%s",revision);
		clif_displaymessage(fd,atcmd_output);
	} else 
		clif_displaymessage(fd,"Cannot determine SVN revision");

	return 0;
}

/*==========================================
 * @mutearea by MouseJstr
 *------------------------------------------*/
static int atcommand_mutearea_sub(struct block_list *bl,va_list ap)
{
	
	int time, id;
	struct map_session_data *pl_sd = (struct map_session_data *)bl;
	if (pl_sd == NULL)
		return 0;

	id = va_arg(ap, int);
	time = va_arg(ap, int);	

	if (id != bl->id && !pc_isGM(pl_sd)) {
		pl_sd->status.manner -= time;
		if (pl_sd->status.manner < 0)
			sc_start(&pl_sd->bl,SC_NOCHAT,100,0,0);
		else if (pl_sd->sc.data[SC_NOCHAT])
			status_change_end(&pl_sd->bl, SC_NOCHAT, -1);
	}
	return 0;
}

int atcommand_mutearea(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	int time;
	nullpo_retr(0, sd);

	if (!message || !*message) {
		clif_displaymessage(fd, "Please, enter a time in minutes (usage: @mutearea/@stfu <time in minutes>.");
		return -1;
	}
	
	time = atoi(message);

	map_foreachinarea(atcommand_mutearea_sub,sd->bl.m, 
		sd->bl.x-AREA_SIZE, sd->bl.y-AREA_SIZE, 
		sd->bl.x+AREA_SIZE, sd->bl.y+AREA_SIZE, BL_PC, sd->bl.id, time);

	return 0;
}


int atcommand_rates(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	char buf[255];
	
	nullpo_retr(0, sd);
	
	sprintf(buf, "Experience rates: Base %.1fx / Job %.1fx",
	battle_config.base_exp_rate/100., battle_config.job_exp_rate/100.);
	
	clif_displaymessage(fd, buf);
	
	return 0;
}

/*==========================================
 * @me by lordalfa
 * => Displays the OUTPUT string on top of the Visible players Heads.
 *------------------------------------------*/
int atcommand_me(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	char tempmes[200];
	nullpo_retr(-1, sd);

	memset(tempmes, '\0', sizeof(tempmes));    
	memset(atcmd_output, '\0', sizeof(atcmd_output));

	if (sd->sc.count && //no "chatting" while muted.
		(sd->sc.data[SC_BERSERK] ||
		(sd->sc.data[SC_NOCHAT] && sd->sc.data[SC_NOCHAT]->val1&MANNER_NOCHAT)))
		return -1;

	if (!message || !*message || sscanf(message, "%199[^\n]", tempmes) < 0) {
		clif_displaymessage(fd, "Please, enter a message (usage: @me <message>).");
		return -1;
	}
	
	sprintf(atcmd_output, msg_txt(270), sd->status.name, tempmes);	// *%s %s*
	clif_disp_overhead(sd, atcmd_output);
	
	return 0;
	
}

/*==========================================
 * @size
 * => Resize your character sprite. [Valaris]
 *------------------------------------------*/
int atcommand_size(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	int size=0;

	nullpo_retr(-1, sd);

	size = atoi(message);
	if(sd->state.size) {
		sd->state.size=0;
		pc_setpos(sd, sd->mapindex, sd->bl.x, sd->bl.y, 3);
	}

	if(size==1) {
		sd->state.size=1;
		clif_specialeffect(&sd->bl,420,AREA);
	} else if(size==2) {
		sd->state.size=2;
		clif_specialeffect(&sd->bl,422,AREA);
	}

	return 0;
}

/*==========================================
 * @monsterignore
 * => Makes monsters ignore you. [Valaris]
 *------------------------------------------*/ 
int atcommand_monsterignore(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	nullpo_retr(-1, sd);

	if (!sd->state.monster_ignore) {
		sd->state.monster_ignore = 1;
		clif_displaymessage(sd->fd, "You are now immune to attacks.");
	} else {
		sd->state.monster_ignore = 0;
		clif_displaymessage(sd->fd, "Returned to normal state.");
	}

	return 0;
}
/*==========================================
 * @fakename
 * => Gives your character a fake name. [Valaris]
 *------------------------------------------*/
int atcommand_fakename(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	char name[NAME_LENGTH];
	nullpo_retr(-1, sd);
	
	if((!message || !*message) && strlen(sd->fakename) > 1) {
		sd->fakename[0]='\0';
		clif_charnameack(0, &sd->bl);
		clif_displaymessage(sd->fd,"Returned to real name.");
		return 0;
	}

	if (!message || !*message || sscanf(message, "%23[^\n]", name) < 1) {
		clif_displaymessage(sd->fd,"You must enter a name.");
		return -1;
	}

	if(strlen(name) < 2) {
		clif_displaymessage(sd->fd,"Fake name must be at least two characters.");
		return -1;
	}
	
	memcpy(sd->fakename,name,NAME_LENGTH);
	sd->fakename[NAME_LENGTH-1] = '\0';
	clif_charnameack(0, &sd->bl);
	clif_displaymessage(sd->fd,"Fake name enabled.");
	
	return 0;
}

/*==========================================
 * @mapflag [flag name] [1|0|on|off] [map name] by Lupus
 * => Shows information about the map flags [map name]
 * Also set flags
 *------------------------------------------*/
int atcommand_mapflag(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
// WIP
	return 0;
}

/*===================================
 * Remove some messages
 *-----------------------------------*/
int atcommand_showexp(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	if (sd->state.showexp) {
		sd->state.showexp = 0;
		clif_displaymessage(fd, "Gained exp will not be shown.");
		return 0;
	}

	sd->state.showexp = 1;
	clif_displaymessage(fd, "Gained exp is now shown");
	return 0;
}

int atcommand_showzeny(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	if (sd->state.showzeny) {
		sd->state.showzeny = 0;
		clif_displaymessage(fd, "Gained zeny will not be shown.");
		return 0;
	}

	sd->state.showzeny = 1;
	clif_displaymessage(fd, "Gained zeny is now shown");
	return 0;
}

int atcommand_showdelay(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	if (sd->state.showdelay) {
		sd->state.showdelay = 0;
		clif_displaymessage(fd, "Skill delay failures won't be shown.");
		return 0;
	}
	
	sd->state.showdelay = 1;
	clif_displaymessage(fd, "Skill delay failures are shown now.");
	return 0;
}

/*==========================================
 * Duel organizing functions [LuzZza]
 *
 * @duel [limit|nick] - create a duel
 * @invite <nick> - invite player
 * @accept - accept invitation
 * @reject - reject invitation
 * @leave - leave duel
 *------------------------------------------*/
int atcommand_invite(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	unsigned int did = sd->duel_group;
	struct map_session_data *target_sd = map_nick2sd((char *)message);

	if(did <= 0)	{
		// "Duel: @invite without @duel."
		clif_displaymessage(fd, msg_txt(350));
		return 0;
	}
	
	if(duel_list[did].max_players_limit > 0 &&
		duel_list[did].members_count >= duel_list[did].max_players_limit) {
		
		// "Duel: Limit of players is reached."
		clif_displaymessage(fd, msg_txt(351));
		return 0;
	}
	
	if(target_sd == NULL) {
		// "Duel: Player not found."
		clif_displaymessage(fd, msg_txt(352));
		return 0;
	}
	
	if(target_sd->duel_group > 0 || target_sd->duel_invite > 0) {
		// "Duel: Player already in duel."
		clif_displaymessage(fd, msg_txt(353));
		return 0;
	}

	if(battle_config.duel_only_on_same_map && target_sd->bl.m != sd->bl.m)
	{
		sprintf(atcmd_output, msg_txt(364), message);
		clif_displaymessage(fd, atcmd_output);
		return 0;
	}
	
	duel_invite(did, sd, target_sd);
	// "Duel: Invitation has been sent."
	clif_displaymessage(fd, msg_txt(354));
	return 0;
}

int atcommand_duel(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	char output[256];
	unsigned int maxpl=0, newduel;
	struct map_session_data *target_sd;

	if(sd->duel_group > 0) {
		duel_showinfo(sd->duel_group, sd);
		return 0;
	}

	if(sd->duel_invite > 0) {
		// "Duel: @duel without @reject."
		clif_displaymessage(fd, msg_txt(355));
		return 0;
	}

	if(!duel_checktime(sd)) {
		// "Duel: You can take part in duel only one time per %d minutes."
		sprintf(output, msg_txt(356), battle_config.duel_time_interval);
		clif_displaymessage(fd, output);
		return 0;
	}

	if(strlen(message) > 0) {
		if(sscanf(message, "%d", &maxpl) >= 1) {
			if(maxpl < 2 || maxpl > 65535) {
				clif_displaymessage(fd, msg_txt(357)); // "Duel: Invalid value."
				return 0;
			}
			duel_create(sd, maxpl);
		} else {
			target_sd = map_nick2sd((char *)message);
			if(target_sd != NULL) {
				if((newduel = duel_create(sd, 2)) != -1) {
					if(target_sd->duel_group > 0 ||	target_sd->duel_invite > 0) {
						clif_displaymessage(fd, msg_txt(353)); // "Duel: Player already in duel."
						return 0;
					}
					duel_invite(newduel, sd, target_sd);
					clif_displaymessage(fd, msg_txt(354)); // "Duel: Invitation has been sent."
				}
			} else {
				// "Duel: Player not found."
				clif_displaymessage(fd, msg_txt(352));
				return 0;
			}
		}
	} else
		duel_create(sd, 0);

	return 0;
}


int atcommand_leave(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	if(sd->duel_group <= 0) {
		// "Duel: @leave without @duel."
		clif_displaymessage(fd, msg_txt(358));
		return 0;
	}

	duel_leave(sd->duel_group, sd);
	clif_displaymessage(fd, msg_txt(359)); // "Duel: You left the duel."
	return 0;
}

int atcommand_accept(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	char output[256];

	if(!duel_checktime(sd)) {
		// "Duel: You can take part in duel only one time per %d minutes."
		sprintf(output, msg_txt(356), battle_config.duel_time_interval);
		clif_displaymessage(fd, output);
		return 0;
	}

	if(sd->duel_invite <= 0) {
		// "Duel: @accept without invititation."
		clif_displaymessage(fd, msg_txt(360));
		return 0;
	}

	duel_accept(sd->duel_invite, sd);
	// "Duel: Invitation has been accepted."
	clif_displaymessage(fd, msg_txt(361));
	return 0;
}

int atcommand_reject(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	if(sd->duel_invite <= 0) {
		// "Duel: @reject without invititation."
		clif_displaymessage(fd, msg_txt(362));
		return 0;
	}

	duel_reject(sd->duel_invite, sd);
	// "Duel: Invitation has been rejected."
	clif_displaymessage(fd, msg_txt(363));
	return 0;
}

/*===================================
 * Cash Points
 *-----------------------------------*/
int atcommand_cash(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	int value;
	nullpo_retr(-1, sd);

	if( !message || !*message || (value = atoi(message)) == 0 ) {
		clif_displaymessage(fd, "Please, enter an amount.");
		return -1;
	}

	if( !strcmpi(command+1,"cash") )
	{
		if( value > 0 )
			pc_getcash(sd, value, 0);
		else
			pc_paycash(sd, value, 0);
	}
	else
	{ // @points
		if( value > 0 )
			pc_getcash(sd, 0, value);
		else
			pc_paycash(sd, value, value);
	}

	return 0;
}

/*===================================
 * Away message (@away, @aw) [LuzZza]
 *-----------------------------------*/
int atcommand_away(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	if(strlen(message) > 0) {
		if(strlen(message) > 128)
			return -1;
		strcpy(sd->away_message, message);
		//"Away automessage has been activated."
		clif_displaymessage(fd, msg_txt(546));
	} else {
		if(strlen(sd->away_message) > 0) {
			sd->away_message[0] = 0;
			//"Away automessage has been disabled."
			clif_displaymessage(fd, msg_txt(547));
			return 0;
		}
		//"Usage: @away,@aw <message>. Enter empty message for disable it."
		clif_displaymessage(fd, msg_txt(548));
	}
	return 0;
}

// @clone/@slaveclone/@evilclone <playername> [Valaris]
int atcommand_clone(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	int x=0,y=0,flag=0,master=0,i=0;
	struct map_session_data *pl_sd=NULL;

	if (!message || !*message) {
		clif_displaymessage(sd->fd,"You must enter a name or character ID.");
		return 0;
	}

	if((pl_sd=map_nick2sd((char *)message)) == NULL && (pl_sd=map_charid2sd(atoi(message))) == NULL) {
		clif_displaymessage(fd, msg_txt(3));	// Character not found.
		return 0;
	}

	if(pc_isGM(pl_sd) > pc_isGM(sd)) {
		clif_displaymessage(fd, msg_txt(126));	// Cannot clone a player of higher GM level than yourself.
		return 0;
	}

	if (strcmpi(command+1, "clone") == 0) 
		flag = 1;
	else if (strcmpi(command+1, "slaveclone") == 0) {
	  	flag = 2;
		master = sd->bl.id;
		if (battle_config.atc_slave_clone_limit
			&& mob_countslave(&sd->bl) >= battle_config.atc_slave_clone_limit) {
			clif_displaymessage(fd, msg_txt(127));	// You've reached your slave clones limit.
			return 0;
		}
	}

	do {
		x = sd->bl.x + (rand() % 10 - 5);
		y = sd->bl.y + (rand() % 10 - 5);
	} while (map_getcell(sd->bl.m,x,y,CELL_CHKNOPASS) && i++ < 10);

	if (i >= 10) {
		x = sd->bl.x;
		y = sd->bl.y;
	}
		
	if((x = mob_clone_spawn(pl_sd, sd->bl.m, x, y, "", master, 0, flag?1:0, 0)) > 0) {
		clif_displaymessage(fd, msg_txt(128+flag*2));	// Evil Clone spawned. Clone spawned. Slave clone spawned.
		return 0;
	}
	clif_displaymessage(fd, msg_txt(129+flag*2));	// Unable to spawn evil clone. Unable to spawn clone. Unable to spawn slave clone.
	return 0;
}

/*===================================
 * Main chat [LuzZza]
 * Usage: @main <on|off|message>
 *-----------------------------------*/
int atcommand_main(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	if(strlen(message) > 0) {

		if(strcmpi(message, "on") == 0) {
			if(!sd->state.mainchat) {
				sd->state.mainchat = 1;
				clif_displaymessage(fd, msg_txt(380)); // Main chat has been activated.
			} else {
				clif_displaymessage(fd, msg_txt(381)); // Main chat already activated.
			}
		} else if(strcmpi(message, "off") == 0) {
			if(sd->state.mainchat) {
				sd->state.mainchat = 0;
				clif_displaymessage(fd, msg_txt(382)); // Main chat has been disabled.
			} else {
				clif_displaymessage(fd, msg_txt(383)); // Main chat already disabled.
			}
		} else {
			if(!sd->state.mainchat) {
				sd->state.mainchat = 1;
				clif_displaymessage(fd, msg_txt(380)); // Main chat has been activated.
			}
			if (sd->sc.data[SC_NOCHAT] && sd->sc.data[SC_NOCHAT]->val1&MANNER_NOCHAT) {
				clif_displaymessage(fd, msg_txt(387));
				return -1;
			}
			sprintf(atcmd_output, msg_txt(386), sd->status.name, message);
			// I use 0xFE000000 color for signalizing that this message is
			// main chat message. 0xFE000000 is invalid color, same using
			// 0xFF000000 for simple (not colored) GM messages. [LuzZza]
			intif_announce(atcmd_output, strlen(atcmd_output) + 1, 0xFE000000, 0);

			// Chat logging type 'M' / Main Chat
			if( log_config.chat&1 || (log_config.chat&32 && !(agit_flag && log_config.chat&64)) )
				log_chat("M", 0, sd->status.char_id, sd->status.account_id, mapindex_id2name(sd->mapindex), sd->bl.x, sd->bl.y, NULL, message);
		}
		
	} else {
	
		if(sd->state.mainchat) 
			clif_displaymessage(fd, msg_txt(384)); // Main chat currently enabled. Usage: @main <on|off>, @main <message>.
		else
			clif_displaymessage(fd, msg_txt(385)); // Main chat currently disabled. Usage: @main <on|off>, @main <message>.
	}
	return 0;
}

/*=====================================
 * Autorejecting Invites/Deals [LuzZza]
 * Usage: @noask
 *-------------------------------------*/
int atcommand_noask(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	if(sd->state.noask) {
		clif_displaymessage(fd, msg_txt(391)); // Autorejecting is deactivated.
		sd->state.noask = 0;
	} else {
		clif_displaymessage(fd, msg_txt(390)); // Autorejecting is activated.
		sd->state.noask = 1;
	}
	
	return 0;
}

/*=====================================
 * Send a @request message to all GMs of lowest_gm_level.
 * Usage: @request <petition>
 *-------------------------------------*/
int atcommand_request(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	if (!message || !*message) {
		clif_displaymessage(sd->fd,msg_txt(277));	// Usage: @request <petition/message to online GMs>.
		return -1;
	}

	sprintf(atcmd_output, msg_txt(278), message);	// (@request): %s
	intif_wis_message_to_gm(sd->status.name, lowest_gm_level, atcmd_output);
	clif_disp_onlyself(sd, atcmd_output, strlen(atcmd_output));
	clif_displaymessage(sd->fd,msg_txt(279));	// @request sent.
	return 0;
}

/*==========================================
 * Feel (SG save map) Reset [HiddenDragon]
 *------------------------------------------*/
int atcommand_feelreset(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	pc_resetfeel(sd);
	clif_displaymessage(fd, "Reset 'Feeling' maps.");

	return 0;
}

/*==========================================
 * Kill Steal Protection
 *------------------------------------------*/
int atcommand_ksprotection(const int fd, struct map_session_data *sd, const char *command, const char *message)
{
	nullpo_retr(-1,sd);

	if( sd->state.noks ) {
		sd->state.noks = 0;
		sprintf(atcmd_output, "[ K.S Protection Inactive ]");
	}
	else
	{
		if( !message || !*message || !strcmpi(message, "party") )
		{ // Default is Party
			sd->state.noks = 2;
			sprintf(atcmd_output, "[ K.S Protection Active - Option: Party ]");
		}
		else if( !strcmpi(message, "self") )
		{
			sd->state.noks = 1;
			sprintf(atcmd_output, "[ K.S Protection Active - Option: Self ]");
		}
		else if( !strcmpi(message, "guild") )
		{
			sd->state.noks = 3;
			sprintf(atcmd_output, "[ K.S Protection Active - Option: Guild ]");
		}
	}

	clif_displaymessage(fd, atcmd_output);
	return 0;
}
/*==========================================
 * Map Kill Steal Protection Setting
 *------------------------------------------*/
int atcommand_allowks(const int fd, struct map_session_data *sd, const char *command, const char *message)
{
	nullpo_retr(-1,sd);

	if( map[sd->bl.m].flag.allowks ) {
		map[sd->bl.m].flag.allowks = 0;
		sprintf(atcmd_output, "[ Map K.S Protection Active ]");
	} else {
		map[sd->bl.m].flag.allowks = 1;
		sprintf(atcmd_output, "[ Map K.S Protection Inactive ]");
	}

	clif_displaymessage(fd, atcmd_output);
	return 0;
}

/*==========================================
 * atcommand_info[] structure definition
 *------------------------------------------*/

AtCommandInfo atcommand_info[] = {
	{ "rura",              40,     atcommand_mapmove },
	{ "warp",              40,     atcommand_mapmove },
	{ "mapmove",           40,     atcommand_mapmove }, // + /mm
	{ "where",              1,     atcommand_where },
	{ "jumpto",            20,     atcommand_jumpto }, // + /shift
	{ "warpto",            20,     atcommand_jumpto },
	{ "goto",              20,     atcommand_jumpto },
	{ "jump",              40,     atcommand_jump },
	{ "who",               20,     atcommand_who },
	{ "whois",             20,     atcommand_who },
	{ "who2",              20,     atcommand_who2 },
	{ "who3",              20,     atcommand_who3 },
	{ "whomap",            20,     atcommand_whomap },
	{ "whomap2",           20,     atcommand_whomap2 },
	{ "whomap3",           20,     atcommand_whomap3 },
	{ "whogm",             20,     atcommand_whogm },
	{ "save",              40,     atcommand_save },
	{ "return",            40,     atcommand_load },
	{ "load",              40,     atcommand_load },
	{ "speed",             40,     atcommand_speed },
	{ "storage",            1,     atcommand_storage },
	{ "gstorage",          50,     atcommand_guildstorage },
	{ "option",            40,     atcommand_option },
	{ "hide",              40,     atcommand_hide }, // + /hide
	{ "jobchange",         40,     atcommand_jobchange },
	{ "job",               40,     atcommand_jobchange },
	{ "die",                1,     atcommand_die },
	{ "kill",              60,     atcommand_kill },
	{ "alive",             60,     atcommand_alive },
	{ "kami",              40,     atcommand_kami },
	{ "kamib",             40,     atcommand_kami },
	{ "kamic",             40,     atcommand_kami },
	{ "heal",              40,     atcommand_heal },
	{ "item",              60,     atcommand_item },
	{ "item2",             60,     atcommand_item2 },
	{ "itemreset",         40,     atcommand_itemreset },
	{ "blvl",              60,     atcommand_baselevelup },
	{ "lvup",              60,     atcommand_baselevelup },
	{ "blevel",            60,     atcommand_baselevelup },
	{ "baselvl",           60,     atcommand_baselevelup },
	{ "baselvup",          60,     atcommand_baselevelup },
	{ "baselevel",         60,     atcommand_baselevelup },
	{ "baselvlup",         60,     atcommand_baselevelup },
	{ "jlvl",              60,     atcommand_joblevelup },
	{ "jlevel",            60,     atcommand_joblevelup },
	{ "joblvl",            60,     atcommand_joblevelup },
	{ "joblevel",          60,     atcommand_joblevelup },
	{ "joblvup",           60,     atcommand_joblevelup },
	{ "joblvlup",          60,     atcommand_joblevelup },
	{ "h",                 20,     atcommand_help },
	{ "help",              20,     atcommand_help },
	{ "h2",                20,     atcommand_help2 },
	{ "help2",             20,     atcommand_help2 },
	{ "gm",               100,     atcommand_gm },
	{ "pvpoff",            40,     atcommand_pvpoff },
	{ "pvpon",             40,     atcommand_pvpon },
	{ "gvgoff",            40,     atcommand_gvgoff },
	{ "gpvpoff",           40,     atcommand_gvgoff },
	{ "gvgon",             40,     atcommand_gvgon },
	{ "gpvpon",            40,     atcommand_gvgon },
	{ "model",             20,     atcommand_model },
	{ "go",                10,     atcommand_go },
	{ "monster",           50,     atcommand_monster },
	{ "spawn",             50,     atcommand_monster },
	{ "monstersmall",      50,     atcommand_monstersmall },
	{ "monsterbig",        50,     atcommand_monsterbig },
	{ "killmonster",       60,     atcommand_killmonster },
	{ "killmonster2",      40,     atcommand_killmonster2 },
	{ "refine",            60,     atcommand_refine },
	{ "produce",           60,     atcommand_produce },
	{ "memo",              40,     atcommand_memo },
	{ "gat",               99,     atcommand_gat },
	{ "displaystatus",     99,     atcommand_displaystatus },
	{ "stpoint",           60,     atcommand_statuspoint },
	{ "skpoint",           60,     atcommand_skillpoint },
	{ "zeny",              60,     atcommand_zeny },
	{ "str",               60,     atcommand_param },
	{ "agi",               60,     atcommand_param },
	{ "vit",               60,     atcommand_param },
	{ "int",               60,     atcommand_param },
	{ "dex",               60,     atcommand_param },
	{ "luk",               60,     atcommand_param },
	{ "glvl",              60,     atcommand_guildlevelup },
	{ "glevel",            60,     atcommand_guildlevelup },
	{ "guildlvl",          60,     atcommand_guildlevelup },
	{ "guildlvup",         60,     atcommand_guildlevelup },
	{ "guildlevel",        60,     atcommand_guildlevelup },
	{ "guildlvlup",        60,     atcommand_guildlevelup },
	{ "makeegg",           60,     atcommand_makeegg },
	{ "hatch",             60,     atcommand_hatch },
	{ "petfriendly",       40,     atcommand_petfriendly },
	{ "pethungry",         40,     atcommand_pethungry },
	{ "petrename",          1,     atcommand_petrename },
	{ "recall",            60,     atcommand_recall }, // + /recall
	{ "night",             80,     atcommand_night },
	{ "day",               80,     atcommand_day },
	{ "doom",              80,     atcommand_doom },
	{ "doommap",           80,     atcommand_doommap },
	{ "raise",             80,     atcommand_raise },
	{ "raisemap",          80,     atcommand_raisemap },
	{ "kick",              20,     atcommand_kick }, // + right click menu for GM "(name) force to quit"
	{ "kickall",           99,     atcommand_kickall },
	{ "allskill",          60,     atcommand_allskill },
	{ "allskills",         60,     atcommand_allskill },
	{ "skillall",          60,     atcommand_allskill },
	{ "skillsall",         60,     atcommand_allskill },
	{ "questskill",        40,     atcommand_questskill },
	{ "lostskill",         40,     atcommand_lostskill },
	{ "spiritball",        40,     atcommand_spiritball },
	{ "party",              1,     atcommand_party },
	{ "guild",             50,     atcommand_guild },
	{ "agitstart",         60,     atcommand_agitstart },
	{ "agitend",           60,     atcommand_agitend },
	{ "mapexit",           99,     atcommand_mapexit },
	{ "idsearch",          60,     atcommand_idsearch },
	{ "broadcast",         40,     atcommand_broadcast }, // + /b and /nb
	{ "localbroadcast",    40,     atcommand_localbroadcast }, // + /lb and /nlb
	{ "recallall",         80,     atcommand_recallall },
	{ "reloaditemdb",      99,     atcommand_reloaditemdb },
	{ "reloadmobdb",       99,     atcommand_reloadmobdb },
	{ "reloadskilldb",     99,     atcommand_reloadskilldb },
	{ "reloadscript",      99,     atcommand_reloadscript },
	{ "reloadgmdb",        99,     atcommand_reloadgmdb },
	{ "reloadatcommand",   99,     atcommand_reloadatcommand },
	{ "reloadbattleconf",  99,     atcommand_reloadbattleconf },
	{ "reloadstatusdb",    99,     atcommand_reloadstatusdb },
	{ "reloadpcdb",        99,     atcommand_reloadpcdb },
	{ "reloadmotd",        99,     atcommand_reloadmotd },
	{ "mapinfo",           99,     atcommand_mapinfo },
	{ "dye",               40,     atcommand_dye },
	{ "ccolor",            40,     atcommand_dye },
	{ "hairstyle",         40,     atcommand_hair_style },
	{ "hstyle",            40,     atcommand_hair_style },
	{ "haircolor",         40,     atcommand_hair_color },
	{ "hcolor",            40,     atcommand_hair_color },
	{ "statall",           60,     atcommand_stat_all },
	{ "statsall",          60,     atcommand_stat_all },
	{ "allstats",          60,     atcommand_stat_all },
	{ "allstat",           60,     atcommand_stat_all },
	{ "block",             60,     atcommand_char_block },
	{ "charblock",         60,     atcommand_char_block },
	{ "ban",               60,     atcommand_char_ban },
	{ "banish",            60,     atcommand_char_ban },
	{ "charban",           60,     atcommand_char_ban },
	{ "charbanish",        60,     atcommand_char_ban },
	{ "unblock",           60,     atcommand_char_unblock },
	{ "charunblock",       60,     atcommand_char_unblock },
	{ "unban",             60,     atcommand_char_unban },
	{ "unbanish",          60,     atcommand_char_unban },
	{ "charunban",         60,     atcommand_char_unban },
	{ "charunbanish",      60,     atcommand_char_unban },
	{ "mount",             20,     atcommand_mount_peco },
	{ "mountpeco",         20,     atcommand_mount_peco },
	{ "guildspy",          60,     atcommand_guildspy },
	{ "partyspy",          60,     atcommand_partyspy },
	{ "repairall",         60,     atcommand_repairall },
	{ "guildrecall",       60,     atcommand_guildrecall },
	{ "partyrecall",       60,     atcommand_partyrecall },
	{ "nuke",              60,     atcommand_nuke },
	{ "shownpc",           80,     atcommand_shownpc },
	{ "hidenpc",           80,     atcommand_hidenpc },
	{ "loadnpc",           80,     atcommand_loadnpc },
	{ "unloadnpc",         80,     atcommand_unloadnpc },
	{ "time",               1,     atcommand_servertime },
	{ "date",               1,     atcommand_servertime },
	{ "serverdate",         1,     atcommand_servertime },
	{ "servertime",         1,     atcommand_servertime },
	{ "jail",              60,     atcommand_jail },
	{ "unjail",            60,     atcommand_unjail },
	{ "discharge",         60,     atcommand_unjail },
	{ "jailfor",           60,     atcommand_jailfor },
	{ "jailtime",           1,     atcommand_jailtime },
	{ "disguise",          20,     atcommand_disguise },
	{ "undisguise",        20,     atcommand_undisguise },
	{ "email",              1,     atcommand_email },
	{ "effect",            40,     atcommand_effect },
	{ "follow",            20,     atcommand_follow },
	{ "addwarp",           60,     atcommand_addwarp },
	{ "skillon",           80,     atcommand_skillon },
	{ "skilloff",          80,     atcommand_skilloff },
	{ "killer",            60,     atcommand_killer },
	{ "npcmove",           80,     atcommand_npcmove },
	{ "killable",          40,     atcommand_killable },
	{ "dropall",           40,     atcommand_dropall },
	{ "storeall",          40,     atcommand_storeall },
	{ "skillid",           40,     atcommand_skillid },
	{ "useskill",          40,     atcommand_useskill },
	{ "displayskill",      99,     atcommand_displayskill },
	{ "snow",              99,     atcommand_snow },
	{ "sakura",            99,     atcommand_sakura },
	{ "clouds",            99,     atcommand_clouds },
	{ "clouds2",           99,     atcommand_clouds2 },
	{ "fog",               99,     atcommand_fog },
	{ "fireworks",         99,     atcommand_fireworks },
	{ "leaves",            99,     atcommand_leaves },
	{ "summon",            60,     atcommand_summon },
	{ "adjgmlvl",          99,     atcommand_adjgmlvl },
	{ "adjcmdlvl",         99,     atcommand_adjcmdlvl },
	{ "trade",             60,     atcommand_trade },
	{ "send",              99,     atcommand_send },
	{ "setbattleflag",     99,     atcommand_setbattleflag },
	{ "unmute",            80,     atcommand_unmute },
	{ "clearweather",      99,     atcommand_clearweather },
	{ "uptime",             1,     atcommand_uptime },
	{ "changesex",         60,     atcommand_changesex },
	{ "mute",              80,     atcommand_mute },
	{ "refresh",            1,     atcommand_refresh },
	{ "identify",          40,     atcommand_identify },
	{ "gmotd",             20,     atcommand_gmotd },
	{ "misceffect",        50,     atcommand_misceffect },
	{ "mobsearch",         10,     atcommand_mobsearch },
	{ "cleanmap",          40,     atcommand_cleanmap },
	{ "npctalk",           20,     atcommand_npctalk },
	{ "pettalk",           10,     atcommand_pettalk },
	{ "users",             40,     atcommand_users },
	{ "reset",             40,     atcommand_reset },
	{ "skilltree",         40,     atcommand_skilltree },
	{ "marry",             40,     atcommand_marry },
	{ "divorce",           40,     atcommand_divorce },
	{ "sound",             40,     atcommand_sound },
	{ "undisguiseall",     99,     atcommand_undisguiseall },
	{ "disguiseall",       99,     atcommand_disguiseall },
	{ "changelook",        60,     atcommand_changelook },
	{ "autoloot",          10,     atcommand_autoloot },
	{ "alootid",           10,     atcommand_autolootitem },
	{ "mobinfo",            1,     atcommand_mobinfo },
	{ "monsterinfo",        1,     atcommand_mobinfo },
	{ "mi",                 1,     atcommand_mobinfo },
	{ "exp",                1,     atcommand_exp },
	{ "adopt",             40,     atcommand_adopt },
	{ "version",            1,     atcommand_version },
	{ "mutearea",          99,     atcommand_mutearea },
	{ "stfu",              99,     atcommand_mutearea },
	{ "rates",              1,     atcommand_rates },
	{ "iteminfo",           1,     atcommand_iteminfo },
	{ "ii",                 1,     atcommand_iteminfo },
	{ "whodrops",           1,     atcommand_whodrops },
	{ "whereis",           10,     atcommand_whereis },
	{ "mapflag",           99,     atcommand_mapflag },
	{ "me",                20,     atcommand_me },
	{ "monsterignore",     99,     atcommand_monsterignore },
	{ "battleignore",      99,     atcommand_monsterignore },
	{ "fakename",          20,     atcommand_fakename },
	{ "size",              20,     atcommand_size },
	{ "showexp",           10,     atcommand_showexp},
	{ "showzeny",          10,     atcommand_showzeny},
	{ "showdelay",          1,     atcommand_showdelay},
	{ "autotrade",         10,     atcommand_autotrade },
	{ "at",                10,     atcommand_autotrade },
	{ "changegm",          10,     atcommand_changegm },
	{ "changeleader",      10,     atcommand_changeleader },
	{ "partyoption",       10,     atcommand_partyoption},
	{ "invite",             1,     atcommand_invite },
	{ "duel",               1,     atcommand_duel },
	{ "leave",              1,     atcommand_leave },
	{ "accept",             1,     atcommand_accept },
	{ "reject",             1,     atcommand_reject },
	{ "away",               1,     atcommand_away },
	{ "aw",                 1,     atcommand_away },
	{ "main",               1,     atcommand_main },
	{ "clone",             50,     atcommand_clone },
	{ "slaveclone",        50,     atcommand_clone },
	{ "evilclone",         50,     atcommand_clone },
	{ "tonpc",             40,     atcommand_tonpc },
	{ "commands",           1,     atcommand_commands },
	{ "noask",              1,     atcommand_noask },
	{ "request",           20,     atcommand_request },
	{ "hlvl",              60,     atcommand_homlevel },
	{ "hlevel",            60,     atcommand_homlevel },
	{ "homlvl",            60,     atcommand_homlevel },
	{ "homlvup",           60,     atcommand_homlevel },
	{ "homlevel",          60,     atcommand_homlevel },
	{ "homevolve",         60,     atcommand_homevolution },
	{ "homevolution",      60,     atcommand_homevolution },
	{ "makehomun",         60,     atcommand_makehomun },
	{ "homfriendly",       60,     atcommand_homfriendly },
	{ "homhungry",         60,     atcommand_homhungry },
	{ "homtalk",           10,     atcommand_homtalk },
	{ "hominfo",            1,     atcommand_hominfo },
	{ "homstats",           1,     atcommand_homstats },
	{ "homshuffle",        60,     atcommand_homshuffle },
	{ "showmobs",          10,     atcommand_showmobs },
	{ "feelreset",         10,     atcommand_feelreset },
	{ "mail",               1,     atcommand_mail },
	{ "noks",               0,     atcommand_ksprotection },
	{ "allowks",            6,     atcommand_allowks },
	{ "cash",              60,     atcommand_cash },
	{ "points",            60,     atcommand_cash },
};


/*==========================================
 * Command lookup functions
 *------------------------------------------*/
static AtCommandInfo* get_atcommandinfo_byname(const char* name)
{
	int i;
	if( *name == atcommand_symbol ) name++; // for backwards compatibility
	ARR_FIND( 0, ARRAYLENGTH(atcommand_info), i, strcmpi(atcommand_info[i].command, name) == 0 );
	return ( i != ARRAYLENGTH(atcommand_info) ) ? &atcommand_info[i] : NULL;
}

static AtCommandInfo* get_atcommandinfo_byfunc(const AtCommandFunc func)
{
	int i;
	ARR_FIND( 0, ARRAYLENGTH(atcommand_info), i, atcommand_info[i].func == func );
	return ( i != ARRAYLENGTH(atcommand_info) ) ? &atcommand_info[i] : NULL;
}


/*==========================================
 * Retrieve the command's required gm level
 *------------------------------------------*/
int get_atcommand_level(const AtCommandFunc func)
{
	AtCommandInfo* info = get_atcommandinfo_byfunc(func);
	return ( info != NULL ) ? info->level : 100; // 100: command can not be used
}


/// Executes an at-command.
/// To be called by internal server code (bypasses various restrictions).
bool is_atcommand_sub(const int fd, struct map_session_data* sd, const char* str, int gmlvl)
{
	AtCommandInfo* info;
	char command[100];
	char args[100];
	char output[200];
	
	if( !str || !*str )
		return false;

	if( *str != atcommand_symbol ) // check first char
		return false;

	if( sscanf(str, "%99s %99[^\n]", command, args) < 2 )
		args[0] = '\0';

	info = get_atcommandinfo_byname(command);
	if( info == NULL || info->func == NULL || gmlvl < info->level )
	{
		if( gmlvl == 0 )
			return false; // will just display as normal text
		else
		{
			sprintf(output, msg_txt(153), command); // "%s is Unknown Command."
			clif_displaymessage(fd, output);
			return true;
		}
	}

	if( log_config.gm && info->level >= log_config.gm )
		log_atcommand(sd, str);

	if( info->func(fd, sd, command, args) != 0 )
	{
		sprintf(output, msg_txt(154), command); // "%s failed."
		clif_displaymessage(fd, output);
	}
	
	return true;
}

/// Executes an at-command.
/// To be used by player-invoked code (restrictions will be applied).
bool is_atcommand(const int fd, struct map_session_data* sd, const char* message)
{
	int gmlvl = pc_isGM(sd);
	
	nullpo_retr(false, sd);
	
	if( !message || !*message )
		return false; // shouldn't happen
	
	if( sd->sc.data[SC_NOCHAT] && sd->sc.data[SC_NOCHAT]->val1&MANNER_NOCOMMAND )
		return true; // so that it won't display as normal message
	
	if( battle_config.atc_gmonly != 0 && gmlvl == 0 )
		return false;
	
	if( map[sd->bl.m].nocommand && gmlvl < map[sd->bl.m].nocommand )
	{
		clif_displaymessage(fd, msg_txt(143)); // "Commands are disabled on this map."
		return false;
	}
	
	// skip 10/11-langtype's codepage indicator, if detected
	if( message[0] == '|' && strlen(message) >= 4 && message[3] == atcommand_symbol )
		message += 3;
	
	return is_atcommand_sub(fd,sd,message,gmlvl);
}


/*==========================================
 *
 *------------------------------------------*/
int atcommand_config_read(const char* cfgName)
{
	char line[1024], w1[1024], w2[1024];
	AtCommandInfo* p;
	FILE* fp;
	
	if( (fp = fopen(cfgName, "r")) == NULL )
	{
		ShowError("AtCommand configuration file not found: %s\n", cfgName);
		return 1;
	}
	
	while( fgets(line, sizeof(line), fp) )
	{
		if( line[0] == '/' && line[1] == '/' )
			continue;
		
		if( sscanf(line, "%1023[^:]:%1023s", w1, w2) != 2 )
			continue;
		
		p = get_atcommandinfo_byname(w1);
		if( p != NULL )
		{
			p->level = atoi(w2);
			p->level = cap_value(p->level, 0, 100);
		}
		else
		if( strcmpi(w1, "import") == 0 )
			atcommand_config_read(w2);
		else
		if( strcmpi(w1, "command_symbol") == 0 &&
			w2[0] > 31   && // control characters
			w2[0] != '/' && // symbol of standard ragnarok GM commands
			w2[0] != '%' && // symbol of party chat speaking
			w2[0] != '$' && // symbol of guild chat speaking
			w2[0] != '#' )  // symbol of charcommand
			atcommand_symbol = w2[0];
		else
			ShowWarning("Unknown setting '%s' in file %s\n", w1, cfgName);
	}
	fclose(fp);

	return 0;
}

void do_init_atcommand()
{
	users_db = uidb_alloc(DB_OPT_BASE);
	add_timer_func_list(atshowmobs_timer, "atshowmobs_timer");
	return;
}

void do_final_atcommand()
{
	users_db->destroy(users_db,NULL);
}


// commands that need to go _after_ the commands table

/*==========================================
 * @commands Lists available @ commands to you
 *------------------------------------------*/
int atcommand_commands(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	char line_buff[CHATBOX_SIZE];
	int i, gm_lvl = pc_isGM(sd), count = 0;
	char* cur = line_buff;

	memset(line_buff,' ',CHATBOX_SIZE);
	line_buff[CHATBOX_SIZE-1] = 0;

	clif_displaymessage(fd, msg_txt(273)); // "Commands available:"

	for( i = 0; i < ARRAYLENGTH(atcommand_info); i++ )
	{
		unsigned int slen;

		if( gm_lvl < atcommand_info[i].level )
			continue;

		slen = (unsigned int)strlen(atcommand_info[i].command);

		// flush the text buffer if this command won't fit into it
		if( ((CHATBOX_SIZE-1+(int)line_buff)-(int)cur) < (int)slen )
		{
			clif_displaymessage(fd,line_buff);
			cur = line_buff;
			memset(line_buff,' ',CHATBOX_SIZE);
			line_buff[CHATBOX_SIZE-1] = 0;
		}

		memcpy(cur,atcommand_info[i].command,slen);
		cur += slen+(10-slen%10);

		count++;
	}
	clif_displaymessage(fd,line_buff);

	sprintf(atcmd_output, msg_txt(274), count); // "%d commands found."
	clif_displaymessage(fd, atcmd_output);
	
	return 0;
}
