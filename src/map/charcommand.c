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
#include "status.h"
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
#include "showmsg.h"

static char command_symbol = '#';

extern char *msg_table[1000]; // Server messages (0-499 reserved for GM commands, 500-999 reserved for others)

#define CCMD_FUNC(x) int charcommand_ ## x (const int fd, struct map_session_data* sd, const char* command, const char* message)

CCMD_FUNC(jobchange);
CCMD_FUNC(petrename);
CCMD_FUNC(petfriendly);
CCMD_FUNC(stats);
CCMD_FUNC(option);
CCMD_FUNC(save);
CCMD_FUNC(stats_all);
CCMD_FUNC(reset);
CCMD_FUNC(spiritball);
CCMD_FUNC(itemlist);
CCMD_FUNC(effect);
CCMD_FUNC(storagelist);
CCMD_FUNC(item);
CCMD_FUNC(warp);
CCMD_FUNC(zeny);
CCMD_FUNC(showexp);
CCMD_FUNC(showdelay);

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
	{ CharCommandPetFriendly,			"#petfriendly",				50, charcommand_petfriendly },
	{ CharCommandStats,					"#stats",					40, charcommand_stats },
	{ CharCommandOption,				"#option",					60, charcommand_option },
	{ CharCommandReset,					"#reset",					60, charcommand_reset },
	{ CharCommandSave,					"#save",					60, charcommand_save },
	{ CharCommandStatsAll,				"#statsall",				40, charcommand_stats_all },
	{ CharCommandSpiritball,			"#spiritball",				40, charcommand_spiritball },
	{ CharCommandItemList,				"#itemlist",				40,	charcommand_itemlist },
	{ CharCommandEffect,				"#effect",					40, charcommand_effect },
	{ CharCommandStorageList,			"#storagelist",				40, charcommand_storagelist },
	{ CharCommandItem,					"#item",					60, charcommand_item },
	{ CharCommandWarp,					"#warp",					60, charcommand_warp },
	{ CharCommandWarp,					"#rura",					60, charcommand_warp },
	{ CharCommandWarp,					"#rura+",					60, charcommand_warp },
	{ CharCommandZeny,					"#zeny",					60, charcommand_zeny },
	{ CharCommandShowExp,					"#showexp", 					0, charcommand_showexp},
	{ CharCommandShowDelay,					"#showdelay",					0, charcommand_showdelay},


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
			snprintf(output, sizeof(output),msg_txt(153), command); // %s is Unknown Command.
			clif_displaymessage(fd, output);
		} else {
			if (info.proc(fd, sd, command, p) != 0) {
				// Command can not be executed
				snprintf(output, sizeof(output), msg_txt(154), command); // %s failed.
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
		printf("CharCommands configuration file not found: %s\n", cfgName);
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
		         w2[0] != '%'	// symbol of party chat speaking
			)
			command_symbol = w2[0];
	}
	fclose(fp);

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
		int j;
		if (pc_isGM(sd) >= pc_isGM(pl_sd)) { // you can change job only to lower or same level
			if ((job >= 0 && job < MAX_PC_CLASS)) {

				// fix pecopeco display
				if ((job != 13 && job != 21 && job != 4014 && job != 4022)) {
					if (pc_isriding(sd)) {
						if (pl_sd->status.class_ == 13)
							pl_sd->status.class_ = pl_sd->view_class = 7;
						if (pl_sd->status.class_ == 21)
							pl_sd->status.class_ = pl_sd->view_class = 14;
						if (pl_sd->status.class_ == 4014)
							pl_sd->status.class_ = pl_sd->view_class = 4008;
						if (pl_sd->status.class_ == 4022)
							pl_sd->status.class_ = pl_sd->view_class = 4015;
						pl_sd->status.option &= ~0x0020;
						clif_changeoption(&pl_sd->bl);
						status_calc_pc(pl_sd, 0);
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
				for (j=0; j < MAX_INVENTORY; j++) {
					if(pl_sd->status.inventory[j].nameid>0 && pl_sd->status.inventory[j].equip!=0)
						pc_unequipitem(pl_sd, j, 3);
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
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
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
	char character[100];
	struct map_session_data *pl_sd;

	memset(character, '\0', sizeof(character));
	if (!message || !*message || sscanf(message,"%d %s",&friendly,character) < 2) {
		clif_displaymessage(fd, "Please, enter a valid value (usage: "
			"#petfriendly <0-1000> <player>).");
		return -1;
	}

	if (((pl_sd = map_nick2sd(character)) != NULL) && pc_isGM(sd)>pc_isGM(pl_sd)) {
		if (pl_sd->status.pet_id > 0 && pl_sd->pd) {
			if (friendly >= 0 && friendly <= 1000) {
				if (friendly != pl_sd->pet.intimate) {
					t = pl_sd->pet.intimate;
					pl_sd->pet.intimate = friendly;
					clif_send_petstatus(pl_sd);
					clif_pet_emotion(pl_sd->pd,0);
					if (battle_config.pet_status_support) {
						if ((pl_sd->pet.intimate > 0 && t <= 0) ||
						    (pl_sd->pet.intimate <= 0 && t > 0)) {
							if (pl_sd->bl.prev != NULL)
								status_calc_pc(pl_sd, 0);
							else
								status_calc_pc(pl_sd, 2);
						}
					}
					clif_displaymessage(pl_sd->fd, msg_table[182]); // Pet friendly value changed!
					clif_displaymessage(sd->fd, msg_table[182]); // Pet friendly value changed!
				} else {
					clif_displaymessage(fd, msg_table[183]); // Pet friendly is already the good value.
					return -1;
				}
			} else {
				clif_displaymessage(fd, msg_table[37]); // An invalid number was specified.
				return -1;
			}
		} else {
			return -1;
		}
	} else {
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int charcommand_stats(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	char character[100];
	char job_jobname[100];
	char output[200];
	struct map_session_data *pl_sd;
	int i;

	memset(character, '\0', sizeof(character));
	memset(job_jobname, '\0', sizeof(job_jobname));
	memset(output, '\0', sizeof(output));

	if (!message || !*message || sscanf(message, "%99[^\n]", character) < 1) {
		clif_displaymessage(fd, "Please, enter a player name (usage: #stats <char name>).");
		return -1;
	}

	if ((pl_sd = map_nick2sd(character)) != NULL) {
		struct {
			const char* format;
			int value;
		} output_table[] = {
			{ "Base Level - %d", pl_sd->status.base_level },
			{ job_jobname, pl_sd->status.job_level },
			{ "Hp - %d",    pl_sd->status.hp },
			{ "MaxHp - %d", pl_sd->status.max_hp },
			{ "Sp - %d",    pl_sd->status.sp },
			{ "MaxSp - %d", pl_sd->status.max_sp },
			{ "Str - %3d",  pl_sd->status.str },
			{ "Agi - %3d",  pl_sd->status.agi },
			{ "Vit - %3d",  pl_sd->status.vit },
			{ "Int - %3d",  pl_sd->status.int_ },
			{ "Dex - %3d",  pl_sd->status.dex },
			{ "Luk - %3d",  pl_sd->status.luk },
			{ "Zeny - %d",  pl_sd->status.zeny },
			{ NULL, 0 }
		};
		sprintf(job_jobname, "Job - %s %s", job_name(pl_sd->status.class_), "(level %d)");
		sprintf(output, msg_table[53], pl_sd->status.name); // '%s' stats:
		clif_displaymessage(fd, output);
		for (i = 0; output_table[i].format != NULL; i++) {
			sprintf(output, output_table[i].format, output_table[i].value);
			clif_displaymessage(fd, output);
		}
	} else {
		clif_displaymessage(fd, msg_table[3]); // Character not found.
		return -1;
	}

	return 0;
}

/*==========================================
 * Character Reset
 *------------------------------------------
 */
int charcommand_reset(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	char character[100];
	char output[200];
	struct map_session_data *pl_sd;

	memset(character, '\0', sizeof(character));
	memset(output, '\0', sizeof(output));

	if (!message || !*message || sscanf(message, "%99[^\n]", character) < 1) {
		clif_displaymessage(fd, "Please, enter a player name (usage: #reset <charname>).");
		return -1;
	}

	if ((pl_sd = map_nick2sd(character)) != NULL) {
		if (pc_isGM(sd) >= pc_isGM(pl_sd)) { // you can reset a character only for lower or same GM level
			pc_resetstate(pl_sd);
			pc_resetskill(pl_sd);
			sprintf(output, msg_table[208], character); // '%s' skill and stats points reseted!
			clif_displaymessage(fd, output);
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
int charcommand_option(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	char character[100];
	int opt1 = 0, opt2 = 0, opt3 = 0;
	struct map_session_data* pl_sd;

	memset(character, '\0', sizeof(character));

	if (!message || !*message ||
		sscanf(message, "%d %d %d %99[^\n]", &opt1, &opt2, &opt3, character) < 4 ||
		opt1 < 0 || opt2 < 0 || opt3 < 0) {
		clif_displaymessage(fd, "Please, enter valid options and a player name (usage: #option <param1> <param2> <param3> <charname>).");
		return -1;
	}

	if ((pl_sd = map_nick2sd(character)) != NULL) {
		if (pc_isGM(sd) >= pc_isGM(pl_sd)) { // you can change option only to lower or same level
			pl_sd->opt1 = opt1;
			pl_sd->opt2 = opt2;
			pl_sd->status.option = opt3;
			// fix pecopeco display
			if (pl_sd->status.class_ == 13 || pl_sd->status.class_ == 21 || pl_sd->status.class_ == 4014 || pl_sd->status.class_ == 4022) {
				if (!pc_isriding(pl_sd)) { // pl_sd have the new value...
					if (pl_sd->status.class_ == 13)
						pl_sd->status.class_ = pl_sd->view_class = 7;
					else if (pl_sd->status.class_ == 21)
						pl_sd->status.class_ = pl_sd->view_class = 14;
					else if (pl_sd->status.class_ == 4014)
						pl_sd->status.class_ = pl_sd->view_class = 4008;
					else if (pl_sd->status.class_ == 4022)
						pl_sd->status.class_ = pl_sd->view_class = 4015;
				}
			} else {
				if (pc_isriding(pl_sd)) { // pl_sd have the new value...
					if (pl_sd->disguise > 0) { // temporary prevention of crash caused by peco + disguise, will look into a better solution [Valaris] (code added by [Yor])
						pl_sd->status.option &= ~0x0020;
					} else {
						if (pl_sd->status.class_ == 7)
							pl_sd->status.class_ = pl_sd->view_class = 13;
						else if (pl_sd->status.class_ == 14)
							pl_sd->status.class_ = pl_sd->view_class = 21;
						else if (pl_sd->status.class_ == 4008)
							pl_sd->status.class_ = pl_sd->view_class = 4014;
						else if (pl_sd->status.class_ == 4015)
							pl_sd->status.class_ = pl_sd->view_class = 4022;
						else
							pl_sd->status.option &= ~0x0020;
					}
				}
			}
			clif_changeoption(&pl_sd->bl);
			status_calc_pc(pl_sd, 0);
			clif_displaymessage(fd, msg_table[58]); // Character's options changed.
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
int charcommand_save(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	char map_name[100];
	char character[100];
	struct map_session_data* pl_sd;
	int x = 0, y = 0;
	int m;

	memset(map_name, '\0', sizeof(map_name));
	memset(character, '\0', sizeof(character));

	if (!message || !*message || sscanf(message, "%99s %d %d %99[^\n]", map_name, &x, &y, character) < 4 || x < 0 || y < 0) {
		clif_displaymessage(fd, "Please, enter a valid save point and a player name (usage: #save <map> <x> <y> <charname>).");
		return -1;
	}

	if (strstr(map_name, ".gat") == NULL && strstr(map_name, ".afm") == NULL && strlen(map_name) < 13) // 16 - 4 (.gat)
		strcat(map_name, ".gat");

	if ((pl_sd = map_nick2sd(character)) != NULL) {
		if (pc_isGM(sd) >= pc_isGM(pl_sd)) { // you can change save point only to lower or same gm level
			m = map_mapname2mapid(map_name);
			if (m < 0) {
				clif_displaymessage(fd, msg_table[1]); // Map not found.
				return -1;
			} else {
				if (m >= 0 && map[m].flag.nowarpto && battle_config.any_warp_GM_min_level > pc_isGM(sd)) {
					clif_displaymessage(fd, "You are not authorised to set this map as a save map.");
					return -1;
				}
				pc_setsavepoint(pl_sd, map_name, x, y);
				clif_displaymessage(fd, msg_table[57]); // Character's respawn point changed.
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
//** Character Stats All by fritz
int charcommand_stats_all(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	char output[1024], gmlevel[1024];
	int i;
	int count;
	struct map_session_data *pl_sd;

	memset(output, '\0', sizeof(output));
	memset(gmlevel, '\0', sizeof(gmlevel));

	count = 0;
	for(i = 0; i < fd_max; i++) {
		if (session[i] && (pl_sd = (struct map_session_data *) session[i]->session_data) && pl_sd->state.auth) {

			if (pc_isGM(pl_sd) > 0)
				sprintf(gmlevel, "| GM Lvl: %d", pc_isGM(pl_sd));
			else
				sprintf(gmlevel, " ");

			sprintf(output, "Name: %s | BLvl: %d | Job: %s (Lvl: %d) | HP: %d/%d | SP: %d/%d", pl_sd->status.name, pl_sd->status.base_level, job_name(pl_sd->status.class_), pl_sd->status.job_level, pl_sd->status.hp, pl_sd->status.max_hp, pl_sd->status.sp, pl_sd->status.max_sp);
			clif_displaymessage(fd, output);
			sprintf(output, "STR: %d | AGI: %d | VIT: %d | INT: %d | DEX: %d | LUK: %d | Zeny: %d %s", pl_sd->status.str, pl_sd->status.agi, pl_sd->status.vit, pl_sd->status.int_, pl_sd->status.dex, pl_sd->status.luk, pl_sd->status.zeny, gmlevel);
			clif_displaymessage(fd, output);
			clif_displaymessage(fd, "--------");
			count++;
		}
	}

	if (count == 0)
		clif_displaymessage(fd, msg_table[28]); // No player found.
	else if (count == 1)
		clif_displaymessage(fd, msg_table[29]); // 1 player found.
	else {
		sprintf(output, msg_table[30], count); // %d players found.
		clif_displaymessage(fd, output);
	}

	return 0;
}

/*==========================================
 * CharSpiritBall Function by PalasX
 *------------------------------------------
 */
int charcommand_spiritball(const int fd, struct map_session_data* sd,const char* command, const char* message)
{
	struct map_session_data *pl_sd;
	char character[100];
	int spirit = 0;

	memset(character, '\0', sizeof(character));

	if(!message || !*message || sscanf(message, "%d %99[^\n]", &spirit, character) < 2 || spirit < 0 || spirit > 1000) {
		clif_displaymessage(fd, "Usage: @spiritball <number: 0-1000>) <CHARACTER_NAME>.");
		return -1;
	}

	if((pl_sd = map_nick2sd(character)) != NULL) {
		if (spirit >= 0 && spirit <= 0x7FFF) {
			if (pl_sd->spiritball != spirit || spirit > 999) {
				if (pl_sd->spiritball > 0)
					pc_delspiritball(pl_sd, pl_sd->spiritball, 1);
				pl_sd->spiritball = spirit;
				clif_spiritball(pl_sd);
				// no message, player can look the difference
				if (spirit > 1000)
					clif_displaymessage(fd, msg_table[204]); // WARNING: more than 1000 spiritballs can CRASH your server and/or client!
			} else {
				clif_displaymessage(fd, msg_table[205]); // You already have this number of spiritballs.
				return -1;
			}
		} else {
			clif_displaymessage(fd, msg_table[37]); // An invalid number was specified.
			return -1;
		}
	} else {
		clif_displaymessage(fd, msg_table[3]); // Character not found.
		return -1;
	}
	return 0;
}

/*==========================================
 * #itemlist <character>: Displays the list of a player's items.
 *------------------------------------------
 */
int
charcommand_itemlist(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	struct map_session_data *pl_sd;
	struct item_data *item_data, *item_temp;
	int i, j, equip, count, counter, counter2;
	char character[100], output[200], equipstr[100], outputtmp[200];
	nullpo_retr(-1, sd);

	memset(character, '\0', sizeof(character));
	memset(output, '\0', sizeof(output));
	memset(equipstr, '\0', sizeof(equipstr));
	memset(outputtmp, '\0', sizeof(outputtmp));

	if (!message || !*message || sscanf(message, "%99[^\n]", character) < 1) {
		clif_displaymessage(fd, "Please, enter a player name (usage: #itemlist <char name>).");
		return -1;
	}

	if ((pl_sd = map_nick2sd(character)) != NULL) {
		if (pc_isGM(sd) >= pc_isGM(pl_sd)) { // you can look items only lower or same level
			counter = 0;
			count = 0;
			for (i = 0; i < MAX_INVENTORY; i++) {
				if (pl_sd->status.inventory[i].nameid > 0 && (item_data = itemdb_search(pl_sd->status.inventory[i].nameid)) != NULL) {
					counter = counter + pl_sd->status.inventory[i].amount;
					count++;
					if (count == 1) {
						sprintf(output, "------ Items list of '%s' ------", pl_sd->status.name);
						clif_displaymessage(fd, output);
					}
					if ((equip = pl_sd->status.inventory[i].equip)) {
						strcpy(equipstr, "| equiped: ");
						if (equip & 4)
							strcat(equipstr, "robe/gargment, ");
						if (equip & 8)
							strcat(equipstr, "left accessory, ");
						if (equip & 16)
							strcat(equipstr, "body/armor, ");
						if ((equip & 34) == 2)
							strcat(equipstr, "right hand, ");
						if ((equip & 34) == 32)
							strcat(equipstr, "left hand, ");
						if ((equip & 34) == 34)
							strcat(equipstr, "both hands, ");
						if (equip & 64)
							strcat(equipstr, "feet, ");
						if (equip & 128)
							strcat(equipstr, "right accessory, ");
						if ((equip & 769) == 1)
							strcat(equipstr, "lower head, ");
						if ((equip & 769) == 256)
							strcat(equipstr, "top head, ");
						if ((equip & 769) == 257)
							strcat(equipstr, "lower/top head, ");
						if ((equip & 769) == 512)
							strcat(equipstr, "mid head, ");
						if ((equip & 769) == 512)
							strcat(equipstr, "lower/mid head, ");
						if ((equip & 769) == 769)
							strcat(equipstr, "lower/mid/top head, ");
						// remove final ', '
						equipstr[strlen(equipstr) - 2] = '\0';
					} else
						memset(equipstr, '\0', sizeof(equipstr));
					if (sd->status.inventory[i].refine)
						sprintf(output, "%d %s %+d (%s %+d, id: %d) %s", pl_sd->status.inventory[i].amount, item_data->name, pl_sd->status.inventory[i].refine, item_data->jname, pl_sd->status.inventory[i].refine, pl_sd->status.inventory[i].nameid, equipstr);
					else
						sprintf(output, "%d %s (%s, id: %d) %s", pl_sd->status.inventory[i].amount, item_data->name, item_data->jname, pl_sd->status.inventory[i].nameid, equipstr);
					clif_displaymessage(fd, output);
					memset(output, '\0', sizeof(output));
					counter2 = 0;
					for (j = 0; j < item_data->slot; j++) {
						if (pl_sd->status.inventory[i].card[j]) {
							if ((item_temp = itemdb_search(pl_sd->status.inventory[i].card[j])) != NULL) {
								if (output[0] == '\0')
									sprintf(outputtmp, " -> (card(s): #%d %s (%s), ", ++counter2, item_temp->name, item_temp->jname);
								else
									sprintf(outputtmp, "#%d %s (%s), ", ++counter2, item_temp->name, item_temp->jname);
								strcat(output, outputtmp);
							}
						}
					}
					if (output[0] != '\0') {
						output[strlen(output) - 2] = ')';
						output[strlen(output) - 1] = '\0';
						clif_displaymessage(fd, output);
					}
				}
			}
			if (count == 0)
				clif_displaymessage(fd, "No item found on this player.");
			else {
				sprintf(output, "%d item(s) found in %d kind(s) of items.", counter, count);
				clif_displaymessage(fd, output);
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
 * #effect by [MouseJstr]
 *
 * Create a effect localized on another character
 *------------------------------------------
 */
int
charcommand_effect(const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	struct map_session_data *pl_sd = NULL;
	char target[255];
	int type = 0;
	nullpo_retr(-1, sd);

	if (!message || !*message || sscanf(message, "%d %s", &type, target) != 2) {
		clif_displaymessage(fd, "usage: #effect <type+> <target>.");
		return -1;
	}

	if((pl_sd=map_nick2sd((char *) target)) == NULL)
		return -1;

	clif_specialeffect(&pl_sd->bl, type, 0);
	clif_displaymessage(fd, msg_table[229]); // Your effect has changed.

	return 0;
}

/*==========================================
 * #storagelist <character>: Displays the items list of a player's storage.
 *------------------------------------------
 */
int
charcommand_storagelist(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	struct storage *stor;
	struct map_session_data *pl_sd;
	struct item_data *item_data, *item_temp;
	int i, j, count, counter, counter2;
	char character[100], output[200], outputtmp[200];
	nullpo_retr(-1, sd);

	memset(character, '\0', sizeof(character));
	memset(output, '\0', sizeof(output));
	memset(outputtmp, '\0', sizeof(outputtmp));

	if (!message || !*message || sscanf(message, "%99[^\n]", character) < 1) {
		clif_displaymessage(fd, "Please, enter a player name (usage: #itemlist <char name>).");
		return -1;
	}

	if ((pl_sd = map_nick2sd(character)) != NULL) {
		if (pc_isGM(sd) >= pc_isGM(pl_sd)) { // you can look items only lower or same level
			if((stor = account2storage2(pl_sd->status.account_id)) != NULL) {
				counter = 0;
				count = 0;
				for (i = 0; i < MAX_STORAGE; i++) {
					if (stor->storage_[i].nameid > 0 && (item_data = itemdb_search(stor->storage_[i].nameid)) != NULL) {
						counter = counter + stor->storage_[i].amount;
						count++;
						if (count == 1) {
							sprintf(output, "------ Storage items list of '%s' ------", pl_sd->status.name);
							clif_displaymessage(fd, output);
						}
						if (stor->storage_[i].refine)
							sprintf(output, "%d %s %+d (%s %+d, id: %d)", stor->storage_[i].amount, item_data->name, stor->storage_[i].refine, item_data->jname, stor->storage_[i].refine, stor->storage_[i].nameid);
						else
							sprintf(output, "%d %s (%s, id: %d)", stor->storage_[i].amount, item_data->name, item_data->jname, stor->storage_[i].nameid);
						clif_displaymessage(fd, output);
						memset(output, '\0', sizeof(output));
						counter2 = 0;
						for (j = 0; j < item_data->slot; j++) {
							if (stor->storage_[i].card[j]) {
								if ((item_temp = itemdb_search(stor->storage_[i].card[j])) != NULL) {
									if (output[0] == '\0')
										sprintf(outputtmp, " -> (card(s): #%d %s (%s), ", ++counter2, item_temp->name, item_temp->jname);
									else
										sprintf(outputtmp, "#%d %s (%s), ", ++counter2, item_temp->name, item_temp->jname);
									strcat(output, outputtmp);
								}
							}
						}
						if (output[0] != '\0') {
							output[strlen(output) - 2] = ')';
							output[strlen(output) - 1] = '\0';
							clif_displaymessage(fd, output);
						}
					}
				}
				if (count == 0)
					clif_displaymessage(fd, "No item found in the storage of this player.");
				else {
					sprintf(output, "%d item(s) found in %d kind(s) of items.", counter, count);
					clif_displaymessage(fd, output);
				}
			} else {
				clif_displaymessage(fd, "This player has no storage.");
				return 0;
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

static void 
charcommand_giveitem_sub(struct map_session_data *sd,struct item_data *item_data,int number)
{
	int flag = 0;
	int loop = 1, get_count = number,i;
	struct item item_tmp;

	if(sd && item_data){
		if (item_data->type == 4 || item_data->type == 5 ||
			item_data->type == 7 || item_data->type == 8) {
			loop = number;
			get_count = 1;
		}
		for (i = 0; i < loop; i++) {
			memset(&item_tmp, 0, sizeof(item_tmp));
			item_tmp.nameid = item_data->nameid;
			item_tmp.identify = 1;
			if ((flag = pc_additem((struct map_session_data*)sd,
					&item_tmp, get_count)))
				clif_additem((struct map_session_data*)sd, 0, 0, flag);
		}
	}
}
/*==========================================
 * #item command (usage: #item <name/id_of_item> <quantity> <player>)
 * by MC Cameri
 *------------------------------------------
 */
int charcommand_item(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	char item_name[100];
	char character[100];
	struct map_session_data *pl_sd;
	int number = 0, item_id, flag;
	struct item item_tmp;
	struct item_data *item_data;
	int get_count, i, pet_id;
	nullpo_retr(-1, sd);

	memset(item_name, '\0', sizeof(item_name));

	if (!message || !*message || sscanf(message, "%99s %d %99[^\n]", item_name, &number, character) < 3) {
		clif_displaymessage(fd, "Please, enter an item name/id (usage: #item <item name or ID> <quantity> <char name>).");
		return -1;
	}

	if (number <= 0)
		number = 1;

	item_id = 0;
	if ((item_data = itemdb_searchname(item_name)) != NULL ||
	    (item_data = itemdb_exists(atoi(item_name))) != NULL)
		item_id = item_data->nameid;

	if (item_id >= 500) {
		get_count = number;
		// check pet egg
		pet_id = search_petDB_index(item_id, PET_EGG);
		if (item_data->type == 4 || item_data->type == 5 ||
			item_data->type == 7 || item_data->type == 8) {
			get_count = 1;
		}
		if ((pl_sd = map_nick2sd(character)) != NULL) {
			if (pc_isGM(sd) >= pc_isGM(pl_sd)) { // you can look items only lower or same level
				for (i = 0; i < number; i += get_count) {
					// if pet egg
					if (pet_id >= 0) {
						pl_sd->catch_target_class = pet_db[pet_id].class_;
						intif_create_pet(pl_sd->status.account_id, pl_sd->status.char_id,
						                 (short)pet_db[pet_id].class_, (short)mob_db[pet_db[pet_id].class_].lv,
						                 (short)pet_db[pet_id].EggID, 0, (short)pet_db[pet_id].intimate,
						                 100, 0, 1, pet_db[pet_id].jname);
					// if not pet egg
					} else {
						memset(&item_tmp, 0, sizeof(item_tmp));
						item_tmp.nameid = item_id;
						item_tmp.identify = 1;
						if ((flag = pc_additem(pl_sd, &item_tmp, get_count)))
							clif_additem(pl_sd, 0, 0, flag);
					}
				}
				clif_displaymessage(fd, msg_table[18]); // Item created.
			} else {
				clif_displaymessage(fd, msg_table[81]); // Your GM level don't authorise you to do this action on this player.
				return -1;
			}
		} else if(/* from jA's @giveitem */strcmpi(character,"all")==0 || strcmpi(character,"everyone")==0){
			for (i = 0; i < fd_max; i++) {
				if (session[i] && (pl_sd = (struct map_session_data *) session[i]->session_data)){
					charcommand_giveitem_sub(pl_sd,item_data,number);
					snprintf(tmp_output, sizeof(tmp_output), "You got %s %d.", item_name,number);
					clif_displaymessage(pl_sd->fd, tmp_output);
				}
			}
			snprintf(tmp_output, sizeof(tmp_output), "%s received %s %d.","Everyone",item_name,number);
			clif_displaymessage(fd, tmp_output);
		} else {
			clif_displaymessage(fd, msg_table[3]); // Character not found.
			return -1;
		}
	} else {
		clif_displaymessage(fd, msg_table[19]); // Invalid item ID or name.
		return -1;
	}

	return 0;
}

/*==========================================
 * #warp/#rura/#rura+ <mapname> <x> <y> <char name>
 *------------------------------------------
 */
int charcommand_warp(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	char map_name[100];
	char character[100];
	int x = 0, y = 0;
	struct map_session_data *pl_sd;
	int m;

	nullpo_retr(-1, sd);

	memset(map_name, '\0', sizeof(map_name));
	memset(character, '\0', sizeof(character));

	if (!message || !*message || sscanf(message, "%99s %d %d %99[^\n]", map_name, &x, &y, character) < 4) {
		clif_displaymessage(fd, "Usage: #warp/#rura/#rura+ <mapname> <x> <y> <char name>");
		return -1;
	}

	if (x <= 0)
		x = rand() % 399 + 1;
	if (y <= 0)
		y = rand() % 399 + 1;
	if (strstr(map_name, ".gat") == NULL && strstr(map_name, ".afm") == NULL && strlen(map_name) < 13) // 16 - 4 (.gat)
		strcat(map_name, ".gat");

	if ((pl_sd = map_nick2sd(character)) != NULL) {
		if (pc_isGM(sd) >= pc_isGM(pl_sd)) { // you can rura+ only lower or same GM level
			if (x > 0 && x < 400 && y > 0 && y < 400) {
				m = map_mapname2mapid(map_name);
				if (m >= 0 && map[m].flag.nowarpto && battle_config.any_warp_GM_min_level > pc_isGM(sd)) {
					clif_displaymessage(fd, "You are not authorised to warp someone to this map.");
					return -1;
				}
				if (pl_sd->bl.m >= 0 && map[pl_sd->bl.m].flag.nowarp && battle_config.any_warp_GM_min_level > pc_isGM(sd)) {
					clif_displaymessage(fd, "You are not authorised to warp this player from its actual map.");
					return -1;
				}
				if (pc_setpos(pl_sd, map_name, x, y, 3) == 0) {
					clif_displaymessage(pl_sd->fd, msg_table[0]); // Warped.
					clif_displaymessage(fd, msg_table[15]); // Player warped (message sends to player too).
				} else {
					clif_displaymessage(fd, msg_table[1]); // Map not found.
					return -1;
				}
			} else {
				clif_displaymessage(fd, msg_table[2]); // Coordinates out of range.
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
 * #zeny <charname>
 *------------------------------------------
 */
int charcommand_zeny(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	struct map_session_data *pl_sd;
	char character[100];
	int zeny = 0, new_zeny;
	nullpo_retr(-1, sd);

	memset(character, '\0', sizeof(character));

	if (!message || !*message || sscanf(message, "%d %99[^\n]", &zeny, character) < 2 || zeny == 0) {
		clif_displaymessage(fd, "Please, enter a number and a player name (usage: #zeny <zeny> <name>).");
		return -1;
	}

	if ((pl_sd = map_nick2sd(character)) != NULL) {
		new_zeny = pl_sd->status.zeny + zeny;
		if (zeny > 0 && (zeny > MAX_ZENY || new_zeny > MAX_ZENY)) // fix positiv overflow
			new_zeny = MAX_ZENY;
		else if (zeny < 0 && (zeny < -MAX_ZENY || new_zeny < 0)) // fix negativ overflow
			new_zeny = 0;
		if (new_zeny != pl_sd->status.zeny) {
			pl_sd->status.zeny = new_zeny;
			clif_updatestatus(pl_sd, SP_ZENY);
			clif_displaymessage(fd, msg_table[211]); // Character's number of zenys changed!
		} else {
			if (zeny < 0)
				clif_displaymessage(fd, msg_table[41]); // Impossible to decrease the number/value.
			else
				clif_displaymessage(fd, msg_table[149]); // Impossible to increase the number/value.
			return -1;
		}
	} else {
		clif_displaymessage(fd, msg_table[3]); // Character not found.
		return -1;
	}

	return 0;
}

/*===================================
 * Remove some messages
 *-----------------------------------
 */
int charcommand_showexp(
       const int fd, struct map_session_data* sd,
       const char* command, const char* message)
{
       if (sd->noexp) {
               sd->noexp = 0;
               clif_displaymessage(fd, "Gained exp is now shown");
               return 0;
       }
       else {
               sd->noexp = 1;
               clif_displaymessage(fd, "Gained exp is now NOT shown");
               return 0;
       }
}

int charcommand_showdelay(
       const int fd, struct map_session_data* sd,
       const char* command, const char* message)
{
       if (sd->nodelay) {
               sd->nodelay = 0;
               clif_displaymessage(fd, "Skill delay failure is now shown");
               return 0;
       }
       else {
               sd->nodelay = 1;
               clif_displaymessage(fd, "Skill delay failure is NOT now shown");
               return 0;
       }
}

