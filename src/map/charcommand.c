#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#include "../common/socket.h"
#include "../common/timer.h"
#include "../common/nullpo.h"

#include "log.h"
#include "clif.h"
#include "chrif.h"
#include "intif.h"
#include "itemdb.h"
#include "map.h"
#include "pc.h"
#include "skill.h"
#include "mob.h"
#include "pet.h"
#include "battle.h"
#include "party.h"
#include "guild.h"
#include "charcommand.h"
#include "atcommand.h"
#include "script.h"
#include "npc.h"
#include "trade.h"
#include "core.h"

static char command_symbol = '#';

static char msg_table[1000][1024]; // Server messages (0-499 reserved for GM commands, 500-999 reserved for others)

#define CHARCOMMAND_FUNC(x) int charcommand_ ## x (const int fd, struct map_session_data* sd, const char* command, const char* message)
#ifdef TXT_ONLY
/* TXT_ONLY */
	CHARCOMMAND_FUNC(test);
/* TXT_ONLY */
#else
/* SQL-only */
	//CHARCOMMAND_FUNC(funcname);
/* SQL Only */
#endif

/*==========================================
 *CharCommandInfo charcommand_info[]構造体の定義
 *------------------------------------------
 */

// First char of commands is configured in charcommand_athena.conf. Leave @ in this list for default value.
// to set default level, read charcommand_athena.conf first please.
static CharCommandInfo charcommand_info[] = {
	
#ifdef TXT_ONLY
/* TXT_ONLY */
//	{ CharCommandType,				"#name",			level, charcommand_func },
	{ CharCommandTest,				"#test",			0,	charcommand_test },
/* TXT_ONLY */
#else
/* SQL-only */
//	{ CharCommandType,				"#name",			level, charcommand_func },
/* SQL Only */
#endif

// add new commands before this line
	{ CharCommand_Unknown,             NULL,                1, NULL }
};


int get_charcommand_level(const CharCommandType type) {
	int i;

	for (i = 0; charcommand_info[i].type != CharCommand_None; i++)
		if (charcommand_info[i].type == type)
			return charcommand_info[i].level;

	return 100; // 100: command can not be used
}

/*==========================================
 *is_charcommand @コマンドに存在するかどうか確認する
 *------------------------------------------
 */
CharCommandType
is_charcommand(const int fd, struct map_session_data* sd, const char* message, int gmlvl) {
	const char* str = message;
	int s_flag = 0;
	CharCommandInfo info;
	CharCommandType type;

	nullpo_retr(CharCommand_None, sd);

	if (!message || !*message)
		return CharCommand_None;

	memset(&info, 0, sizeof(info));
	str += strlen(sd->status.name);
	while (*str && (isspace(*str) || (s_flag == 0 && *str == ':'))) {
		if (*str == ':')
			s_flag = 1;
		str++;
	}
	if (!*str)
		return CharCommand_None;

	type = charcommand(gmlvl > 0 ? gmlvl : pc_isGM(sd), str, &info);
	if (type != CharCommand_None) {
		char command[100];
		char output[200];
		const char* p = str;
		memset(command, '\0', sizeof(command));
		memset(output, '\0', sizeof(output));
		while (*p && !isspace(*p))
			p++;
		if (p - str >= sizeof(command)) // too long
			return CharCommand_Unknown;
		strncpy(command, str, p - str);
		while (isspace(*p))
			p++;

		if (type == CharCommand_Unknown || info.proc == NULL) {
			sprintf(output, msg_table[153], command); // %s is Unknown Command.
			clif_displaymessage(fd, output);
		} else {
			if (info.proc(fd, sd, command, p) != 0) {
				// Command can not be executed
				sprintf(output, msg_table[154], command); // %s failed.
				clif_displaymessage(fd, output);
			}
		}

		return info.type;
	}

	return CharCommand_None;
}

/*==========================================
 *
 *------------------------------------------
 */
CharCommandType charcommand(const int level, const char* message, struct CharCommandInfo* info) {
	char* p = (char *)message; 

	if (!info)
		return CharCommand_None;
	if (battle_config.atc_gmonly != 0 && !level) // level = pc_isGM(sd)
		return CharCommand_None;
	if (!p || !*p) {
		fprintf(stderr, "char command message is empty\n");
		return CharCommand_None;
	}

	if (*p == command_symbol) { // check first char.
		char command[101];
		int i = 0;
		memset(info, 0, sizeof(CharCommandInfo));
		sscanf(p, "%100s", command);
		command[sizeof(command)-1] = '\0';

		while (charcommand_info[i].type != CharCommand_Unknown) {
			if (strcmpi(command+1, charcommand_info[i].command+1) == 0 && level >= charcommand_info[i].level) {
				p[0] = charcommand_info[i].command[0]; // set correct first symbol for after.
				break;
			}
			i++;
		}

		if (charcommand_info[i].type == CharCommand_Unknown) {
			// doesn't return Unknown if player is normal player (display the text, not display: unknown command)
			if (level == 0)
				return CharCommand_None;
			else
				return CharCommand_Unknown;
		}
		memcpy(info, &charcommand_info[i], sizeof charcommand_info[i]);
	} else {
		return CharCommand_None;
	}

	return info->type;
}


/*==========================================
 *
 *------------------------------------------
 */
static CharCommandInfo* get_charcommandinfo_byname(const char* name) {
	int i;

	for (i = 0; charcommand_info[i].type != CharCommand_Unknown; i++)
		if (strcmpi(charcommand_info[i].command + 1, name) == 0)
			return &charcommand_info[i];

	return NULL;
}

/*==========================================
 *
 *------------------------------------------
 */
int charcommand_config_read(const char *cfgName) {
	char line[1024], w1[1024], w2[1024];
	CharCommandInfo* p;
	FILE* fp;

	if ((fp = fopen(cfgName, "r")) == NULL) {
		printf("Char commands configuration file not found: %s\n", cfgName);
		return 1;
	}

	while (fgets(line, sizeof(line)-1, fp)) {
		if (line[0] == '/' && line[1] == '/')
			continue;

		if (sscanf(line, "%1023[^:]:%1023s", w1, w2) != 2)
			continue;
		p = get_charcommandinfo_byname(w1);
		if (p != NULL) {
			p->level = atoi(w2);
			if (p->level > 100)
				p->level = 100;
			else if (p->level < 0)
				p->level = 0;
		}

		if (strcmpi(w1, "import") == 0)
			charcommand_config_read(w2);
		else if (strcmpi(w1, "command_symbol") == 0 && w2[0] > 31 &&
		         w2[0] != '/' && // symbol of standard ragnarok GM commands
		         w2[0] != '%' && // symbol of party chat speaking
				 w2[0] != '@')	 // symbol for @commands
			command_symbol = w2[0];
	}
	fclose(fp);

	return 0;
}



/*==========================================
// # command processing functions
 *------------------------------------------
 */
int 
charcommand_test (const int fd, struct map_session_data* sd, 
	const char* command, const char* message) {
	clif_displaymessage(fd,"Works!");
	return 0;
}

