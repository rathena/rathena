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

CHARCOMMAND_FUNC(jobchange);
CHARCOMMAND_FUNC(petrename);

#ifdef TXT_ONLY
/* TXT_ONLY */

/* TXT_ONLY */
#else
/* SQL-only */

/* SQL Only */
#endif

/*==========================================
 *CharCommandInfo charcommand_info[]構造体の定義
 *------------------------------------------
 */

// First char of commands is configured in charcommand_athena.conf. Leave @ in this list for default value.
// to set default level, read charcommand_athena.conf first please.
static CharCommandInfo charcommand_info[] = {
	{ CharCommandJobChange,				"#job",						60,	charcommand_jobchange },
	{ CharCommandJobChange,				"#jobchange",				60,	charcommand_jobchange },
	{ CharCommandPetRename,				"#petrename",				50, charcommand_petrename },

#ifdef TXT_ONLY
/* TXT_ONLY */

/* TXT_ONLY */
#else
/* SQL-only */

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

/*==========================================
 * 対象キャラクターを転職させる upper指定で転生や養子も可能
 *------------------------------------------
 */
int charcommand_jobchange(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	char character[100];
	struct map_session_data* pl_sd;
	int job = 0, upper = -1;

	memset(character, '\0', sizeof(character));

	if (!message || !*message) {
		clif_displaymessage(fd, "Please, enter a job and a player name (usage: #job/#jobchange <job ID> <char name>).");
		return -1;
	}

	if (sscanf(message, "%d %d %99[^\n]", &job, &upper, character) < 3) { //upper指定してある
		upper = -1;
		if (sscanf(message, "%d %99[^\n]", &job, character) < 2) { //upper指定してない上に何か足りない
			clif_displaymessage(fd, "Please, enter a job and a player name (usage: #job/#jobchange <job ID> <char name>).");
			return -1;
		}
	}

	if ((pl_sd = map_nick2sd(character)) != NULL) {
		if (pc_isGM(sd) >= pc_isGM(pl_sd)) { // you can change job only to lower or same level
			if ((job >= 0 && job < MAX_PC_CLASS)) {

				// fix pecopeco display
				if ((job != 13 && job != 21 && job != 4014 && job != 4022)) {
					if (pc_isriding(sd)) {
						if (pl_sd->status.class == 13)
							pl_sd->status.class = pl_sd->view_class = 7;
						if (pl_sd->status.class == 21)
							pl_sd->status.class = pl_sd->view_class = 14;
						if (pl_sd->status.class == 4014)
							pl_sd->status.class = pl_sd->view_class = 4008;
						if (pl_sd->status.class == 4022)
							pl_sd->status.class = pl_sd->view_class = 4015;
						pl_sd->status.option &= ~0x0020;
						clif_changeoption(&pl_sd->bl);
						pc_calcstatus(pl_sd, 0);
					}
				} else {
					if (!pc_isriding(sd)) {
						if (job == 13)
							job = 7;
						if (job == 21)
							job = 14;
						if (job == 4014)
							job = 4008;
						if (job == 4022)
							job = 4015;
					}
				}

				if (pc_jobchange(pl_sd, job, upper) == 0)
					clif_displaymessage(fd, msg_table[48]); // Character's job changed.
				else {
					clif_displaymessage(fd, msg_table[192]); // Impossible to change the character's job.
					return -1;
				}
			} else {
				clif_displaymessage(fd, msg_table[49]); // Invalid job ID.
				return -1;
			}
		} else {
			clif_displaymessage(fd, msg_table[81]); // Your GM level don't authorise you to do this action on this player.
			return -1;
		}
	} else {
		clif_displaymessage(fd, msg_table[3]); // Character not found.
		return -1;
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int charcommand_petrename(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	char character[100];
	struct map_session_data *pl_sd;

	memset(character, '\0', sizeof(character));

	if (!message || !*message || sscanf(message, "%99[^\n]", character) < 1) {
		clif_displaymessage(fd, "Please, enter a player name (usage: #petrename <char name>).");
		return -1;
	}

	if ((pl_sd = map_nick2sd(character)) != NULL) {
		if (pl_sd->status.pet_id > 0 && pl_sd->pd) {
			if (pl_sd->pet.rename_flag != 0) {
				pl_sd->pet.rename_flag = 0;
				intif_save_petdata(pl_sd->status.account_id, &pl_sd->pet);
				clif_send_petstatus(pl_sd);
				clif_displaymessage(fd, msg_table[189]); // This player can now rename his/her pet.
			} else {
				clif_displaymessage(fd, msg_table[190]); // This player can already rename his/her pet.
				return -1;
			}
		} else {
			clif_displaymessage(fd, msg_table[191]); // Sorry, but this player has no pet.
			return -1;
		}
	} else {
		clif_displaymessage(fd, msg_table[3]); // Character not found.
		return -1;
	}

	return 0;
}


/*==========================================
 * 
 *------------------------------------------
 */
int charcommand_petfriendly(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	int friendly = 0;
	int t = 0;

	if (!message || !*message || (friendly = atoi(message)) < 0) {
		clif_displaymessage(fd, "Please, enter a valid value (usage: "
			"#petfriendly <0-1000> <player>).");
		return 0;
	}

	if (sd->status.pet_id > 0 && sd->pd) {
		if (friendly >= 0 && friendly <= 1000) {
			if (friendly != sd->pet.intimate) {
				t = sd->pet.intimate;
				sd->pet.intimate = friendly;
				clif_send_petstatus(sd);
				if (battle_config.pet_status_support) {
					if ((sd->pet.intimate > 0 && t <= 0) ||
					    (sd->pet.intimate <= 0 && t > 0)) {
						if (sd->bl.prev != NULL)
							pc_calcstatus(sd, 0);
						else
							pc_calcstatus(sd, 2);
					}
				}
				clif_displaymessage(fd, msg_table[182]); // Pet friendly value changed!
			} else {
				clif_displaymessage(fd, msg_table[183]); // Pet friendly is already the good value.
				return -1;
			}
		} else {
			clif_displaymessage(fd, msg_table[37]); // An invalid number was specified.
			return -1;
		}
	} else {
		clif_displaymessage(fd, msg_table[184]); // Sorry, but you have no pet.
		return -1;
	}

	return 0;
}
